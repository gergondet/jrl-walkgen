/*
 * Copyright 2011,
 *
 * Medhi    Benallegue
 * Andrei   Herdt
 * Francois Keith
 * Olivier  Stasse
 *
 * JRL, CNRS/AIST
 *
 * This file is part of walkGenJrl.
 * walkGenJrl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * walkGenJrl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with walkGenJrl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Research carried out within the scope of the
 *  Joint Japanese-French Robotics Laboratory (JRL)
 */
#ifndef _QP_PROBLEM_H_
#define _QP_PROBLEM_H_



#include <jrl/mal/matrixabstractlayer.hh>
#include <Mathematics/qld.h>
#include <privatepgtypes.h>
#include <PreviewControl/rigid-body-system.hh>
#include <PreviewControl/rigid-body.hh>
#include <Mathematics/intermediate-qp-matrices.hh>

namespace PatternGeneratorJRL
{

  /// \brief Final optimization problem.
  /// Store and solve a quadratic problem with linear constraints.
  ///
  struct QPProblem_s
  {

    //
    // Public types
    //
  public:

    enum QPElement
    {
      MATRIX_Q,
      MATRIX_DU,
      VECTOR_D,
      VECTOR_DS,
      VECTOR_XL,
      VECTOR_XU
    };

    enum Solver
    {
      QLD,
      LSSOL
    };

    enum Tests
    {
	NONE,
	ALL,
	ITT,
	CTR
    };
    //
    //Public methods
    //
  public:

    /// \brief Initialize by default an empty problem.
    QPProblem_s();

    /// \brief Release the memory at the end only.
    ~QPProblem_s();

    /// \brief Reallocate array
    ///
    /// \param[in] Array
    /// \param[in] SizeOld Old array size
    /// \param[in] SizeNew New array size
    template <typename type>
    int resize( type * & Array, unsigned int SizeOld, unsigned int SizeNew );

    /// \brief Add a matrix to the final optimization problem in array form
    ///
    /// \param[in] Mat Added matrix
    /// \param[in] Type Target matrix type
    /// \param[in] Row First row inside the target
    /// \param[in] Col First column inside the target
    void add_term_to( QPElement Type, const boost_ublas::matrix<double> & Mat,
        unsigned int Row, unsigned int Col );

    /// \brief Add a vector to the final optimization problem in array form
    ///
    /// \param Mat Added vector
    /// \param ype Target vector type
    /// \param row First row inside the target
    void add_term_to( QPElement Type, const boost_ublas::vector<double> & Vec,
        unsigned int Row );

    /// \brief Dump current problem on disk.
    void dump( const char * Filename );
    void dump( double Time );

    /// \brief Dump on disk an array.
    ///
    /// \param Type
    /// \param Filename
    void dump( QPElement Type, const char * Filename);
    /// \}

    /// \brief Initialize array
    ///
    /// \param[in] type
    void clear( QPElement Type );

    /// \brief Set matrices to zero
    void reset();

    /// \brief Solve the optimization problem
    ///
    /// \param[in] Solver
    /// \param[out] Result
    /// \param[in] Tests
    void solve( Solver Solver, solution_t & Result, const Tests & Tests=NONE );

    /// \name Accessors and mutators
    /// \{
    inline void NbVariables( unsigned int NbVariables )
    { NbVariables_ = NbVariables;};
    inline unsigned int NbVariables( )
    { return NbVariables_;};

    inline void NbEqConstraints( unsigned int NbEqConstraints )
    { NbEqConstraints_ = NbEqConstraints;};
    inline unsigned int NbEqConstraints(  )
    { return NbEqConstraints_;};

    inline void NbConstraints( unsigned int NbConstraints )
    { NbConstraints_ = NbConstraints;};
    inline unsigned int NbConstraints()
    { return NbConstraints_;};
    /// \}

    //
    // Private methods
    //
  private:
	  
    /// \brief Release memory.
    void release_memory();

    /// \brief Allocate memory.
    /// Called when setting the dimensions of the problem.
    ///
    void resize_all();

    /// \name Dumping functions
    /// \{
    /// \brief Print_ on disk the parameters that are passed to the solver
    void dump_solver_parameters( std::ostream & aos );
    /// \brief Print_ array
    void dump( QPElement Type, std::ostream & aos );
    /// \brief Print_ problem
    void dump_problem( std::ostream & );
    /// \}

    //
    //Private types
    //
  private:

    /// \brief Handle matrices/vectors in array form
    template<typename type>
    struct array_s
    {
      type * Array_;

      int Id_;
      unsigned int NbRows_, NbCols_;
      unsigned int SizeMem_;

      void fill( type Value )
      { std::fill_n(Array_, NbRows_*NbCols_, Value); }

      void fill( type * Array, int Size, type Value )
      { std::fill_n(Array, Size, Value); }


      /// \brief Make a contiguous array
      ///
      /// \param[in] FinalArray New array
      /// \param[in] NbRows Size of the new array
      /// \param[in] NbCols Size of the new array
      /// \return 0

      int stick_together( struct array_s<type> & FinalArray,
          unsigned int NbRows, unsigned int NbCols )
      {

        try {
	  type * NewArray = 0;
	  if ((FinalArray.SizeMem_<NbRows*NbCols) ||
	      (FinalArray.Array_==0))
	    {
	      FinalArray.Array_ = new type[NbRows*NbCols];
	      FinalArray.SizeMem_ = NbRows*NbCols;
	    }
	  NewArray = FinalArray.Array_;
	  
          fill(NewArray, NbRows*NbCols, (type)0);
          for(unsigned int i = 0; i < NbRows; i++)
            for(unsigned int j = 0; j < NbCols; j++)
              NewArray[i+NbRows*j] = Array_[i+NbRows_*j];

          FinalArray.NbRows_ = NbRows;
          FinalArray.NbCols_ = NbCols;
        }
        catch (std::bad_alloc& ba)
        {std::cerr << "bad_alloc caught: " << ba.what() << std::endl; }

        return 0;

      }

      /// \brief Resize array
      ///
      /// \param[in] NbRows Size of the new array
      /// \param[in] NbCols Size of the new array
      /// \param[in] preserve Preserve old values
      /// \return 0
      int resize( unsigned int NbRows, unsigned int NbCols, bool Preserve )
      {

        try {
	  bool Reallocate = false;
	  type * NewArray = 0;
	  if (NbRows*NbCols>SizeMem_)
	    {
	      NewArray = new type[NbRows*NbCols];
	      SizeMem_ = NbRows*NbCols;
	      Reallocate = true;
	    }
	  else NewArray = Array_;

	  fill(NewArray, NbRows*NbCols, (type)0);
	  if ((Preserve) && 
	      (Array_!=0) ) {
	    for(unsigned int i = 0; i < NbRows_; i++)
	      for(unsigned int j = 0; j < NbCols_; j++)
		NewArray[i+NbRows*j] = Array_[i+NbRows_*j]; }

	  if ((Array_!=0) && Reallocate)
	    {
	      delete [] Array_;
	    }
	  Array_ = NewArray;
	  
          NbRows_ = NbRows;
          NbCols_ = NbCols;
        }
        catch (std::bad_alloc& ba)
        {std::cerr << "bad_alloc caught: " << ba.what() << std::endl; }

        return 0;

      }

      array_s():
        Array_(0),Id_(0),NbRows_(0),NbCols_(0), SizeMem_(0){
      };
      ~array_s()
      {

        if (Array_!=0) delete [] Array_;

      };
    };

    //
    //Private members
    //
  private:

    /// \name lssol parameters
    /// \{
    int *istate_;
    int *kx_ ;

    double *b_;

    int inform_;
    int iter_;
    double obj_;
    double *clamda_;
    /// \}
    /// \name ql-parameters
    /// \{
    int m_, me_, mmax_, n_, nmax_, mnn_;
    array_s<double> Q_, Q_dense_, D_, DU_, DU_dense_, DS_, XL_, XU_, X_, U_, war_;
    array_s<int> iwar_;
    int iout_, ifail_, iprint_, lwar_, liwar_;
    double eps_;

    RigidBodySystem * Robot_;

    boost_ublas::vector<double> last_solution_;

    /// \}

    /// \brief Number of optimization parameters
    unsigned int NbVariables_;

    /// \brief Total number of constraints
    unsigned int NbConstraints_;

    /// \brief Number of equality constraints
    unsigned int NbEqConstraints_;
  };
  typedef struct QPProblem_s QPProblem;
}
#include <ZMPRefTrajectoryGeneration/qp-problem.hxx>
#endif /* _QP_PROBLEM_H_ */
