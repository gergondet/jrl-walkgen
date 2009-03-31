#include <walkGenJrl/Mathematics/OptCholesky.h>

#if 0
#define RESETDEBUG4(y) { ofstream DebugFile; \
                         DebugFile.open(y,ofstream::out); \
                         DebugFile.close();}
#define ODEBUG4(x,y) { ofstream DebugFile; \
                       DebugFile.open(y,ofstream::app); \
                       DebugFile <<  x << endl; DebugFile.close();}
#else
#define RESETDEBUG4(y) 
#define ODEBUG4(x,y) 
#endif

#define RESETDEBUG6(y) 
#define ODEBUG6(x,y) 

#define RESETDEBUG5(y) { ofstream DebugFile; \
                         DebugFile.open(y,ofstream::out);\
                         DebugFile.close();}
#define ODEBUG5(x,y) { ofstream DebugFile; \
                       DebugFile.open(y,ofstream::app); \
                       DebugFile << x << endl; \
                       DebugFile.close();}
#define ODEBUG5NOE(x,y) { ofstream DebugFile; \
                          DebugFile.open(y,ofstream::app); \
                          DebugFile << x ; DebugFile.close();}
#if 1
#define ODEBUG(x)
#else
#define ODEBUG(x)  std::cout << "OptCholesky " << x << endl;
#endif

#define ODEBUG3(x)  std::cout << "OptCholesky " << x << endl;

using namespace PatternGeneratorJRL;

OptCholesky::OptCholesky(unsigned int lNbMaxOfConstraints,
			 unsigned int lCardU,
			 unsigned int lUpdateMode):
  m_NbMaxOfConstraints(lNbMaxOfConstraints),
  m_CardU(lCardU),
  m_A(0),
  m_L(0),
  m_iL(0),
  m_UpdateMode(lUpdateMode),
  m_NbOfConstraints(0)
{
  InitializeInternalVariables();
}

OptCholesky::~OptCholesky()
{
}


void OptCholesky::InitializeInternalVariables()
{
  if (m_NbMaxOfConstraints!=0)
    {
      m_L = new double [m_NbMaxOfConstraints * m_NbMaxOfConstraints];
      for(unsigned int i=0;i<m_NbMaxOfConstraints * m_NbMaxOfConstraints;i++)
	m_L[i]=0.0;
    }
}

void OptCholesky::SetToZero()
{
#if 0
  if (m_NbMaxOfConstraints!=0)
    {
      if (m_L!=0)
	for(unsigned int i=0;i<m_NbMaxOfConstraints * m_NbMaxOfConstraints;i++)
	  m_L[i]=0.0;

      
    }
#endif
  m_SetActiveConstraints.clear();

}

void OptCholesky::SetA(double *aA,
		       unsigned int lNbOfConstraints)
{
  m_A = aA;
  m_NbOfConstraints = lNbOfConstraints;
}

int OptCholesky::AddActiveConstraints(vector<unsigned int> & lConstraints)
{
  int r=0;
  for(unsigned int li=0;li<lConstraints.size();li++)
    {
      r=AddActiveConstraint(lConstraints[li]);
      if (r<0)
	return -li;
    }
  return r;
}

int OptCholesky::AddActiveConstraint(unsigned int aConstraint)
{
  /* Update set of active constraints */
  m_SetActiveConstraints.push_back(aConstraint);

  int r = 0;
  if (m_UpdateMode==MODE_NORMAL)
    UpdateCholeskyMatrixNormal();
  else if (m_UpdateMode==MODE_FORTRAN)
    UpdateCholeskyMatrixFortran();
    
  return r;
}

int OptCholesky::CurrentNumberOfRows()
{
  return m_SetActiveConstraints.size();
}

void OptCholesky::SetL(double *aL)
{
  m_L = aL;
}

void OptCholesky::SetiL(double *aiL)
{
  m_iL = aiL;
}

int OptCholesky::UpdateCholeskyMatrixNormal()
{

  if ((m_A==0) | (m_L==0))
    return -1;

  double Mij=0.0;
  unsigned int IndexNewRowAKAi = 0;
  if (m_SetActiveConstraints.size()>0)
    IndexNewRowAKAi = m_SetActiveConstraints.size()-1;
  
  double *PointerArow_i = m_A +  m_CardU * 
    m_SetActiveConstraints[IndexNewRowAKAi];

  /* Compute Li,j */
  for(int lj=0;lj<(int)m_SetActiveConstraints.size();lj++)
    {

      /* A value M(i,j) is computed once,
	 directly from the matrix A */      
      double *Arow_i = PointerArow_i;
      double *Arow_j = m_A + m_CardU* m_SetActiveConstraints[lj];
      Mij=0.0;
      for(int lk=0;lk<(int)m_CardU;lk++)
	{
	  Mij+= (*Arow_i++) * (*Arow_j++);
	}

      /* */
      double r = Mij;
      double * ptLik =m_L + IndexNewRowAKAi*m_NbMaxOfConstraints;
      double * ptLjk =m_L + lj*m_NbMaxOfConstraints;

      for(int lk=0;lk<lj;lk++)
	{
	  r = r - (*ptLik++)  * (*ptLjk++);
	}
      if (lj!=(int)m_SetActiveConstraints.size()-1)
	m_L[IndexNewRowAKAi*m_NbMaxOfConstraints+lj]=r/m_L[lj*m_NbMaxOfConstraints+lj];
      else
	m_L[IndexNewRowAKAi*m_NbMaxOfConstraints+lj] = sqrt(r);
      
    }
  
  return 0;
  
}

int OptCholesky::UpdateCholeskyMatrixFortran()
{

  if ((m_A==0) | (m_L==0))
    return -1;

  double Mij=0.0;
  unsigned int IndexNewRowAKAi = 0;
  if (m_SetActiveConstraints.size()>0)
    IndexNewRowAKAi = m_SetActiveConstraints.size()-1;
  
  double *PointerArow_i = m_A +  
    m_SetActiveConstraints[IndexNewRowAKAi];

  /* Compute Li,j */
  for(int lj=0;lj<(int)m_SetActiveConstraints.size();lj++)
    {

      /* A value M(i,j) is computed once,
	 directly from the matrix A */      
      double *Arow_i = PointerArow_i;
      double *Arow_j = m_A + m_SetActiveConstraints[lj];
      Mij=0.0;
      for(int lk=0;lk<(int)m_CardU;lk++)
	{
	  Mij+= (*Arow_i) * (*Arow_j);
	  Arow_i+= m_NbOfConstraints+1;
	  Arow_j+= m_NbOfConstraints+1;
	}

      /* */
      double r = Mij;
      ODEBUG("r: M("<< m_SetActiveConstraints[IndexNewRowAKAi] << "," 
	      << m_SetActiveConstraints[lj] <<")="<< r);
      double * ptLik =m_L + IndexNewRowAKAi*m_NbMaxOfConstraints;
      double * ptLjk =m_L + lj*m_NbMaxOfConstraints;

      for(int lk=0;lk<lj;lk++)
	{
	  r = r - (*ptLik++)  * (*ptLjk++);
	}
      if (lj!=(int)m_SetActiveConstraints.size()-1)
	m_L[IndexNewRowAKAi*m_NbMaxOfConstraints+lj]=r/m_L[lj*m_NbMaxOfConstraints+lj];
      else
	m_L[IndexNewRowAKAi*m_NbMaxOfConstraints+lj] = sqrt(r);
      
      ODEBUG("m_L(" << IndexNewRowAKAi << "," << lj << ")="
	      << m_L[IndexNewRowAKAi*m_NbMaxOfConstraints+lj] );
    }
  
  return 0;
  
}

int OptCholesky::ComputeNormalCholeskyOnANormal()
{
  if ((m_A==0) | (m_L==0))
    return -1;
  if (m_NbMaxOfConstraints!=m_CardU)
    return -2;
  

  double *pA = m_A;
  for(int li=0;li<(int)m_NbMaxOfConstraints;li++)
    {
      for(int lj=0;lj<=li;lj++)
	{
	  
	  /* Compute Li,j */
	  
	  double r = pA[lj];
	  double * ptLik =m_L + li*m_NbMaxOfConstraints;
	  double * ptLjk =m_L + lj*m_NbMaxOfConstraints;
	  
	  for(int lk=0;lk<lj;lk++)
	    {
	      r = r - (*ptLik++)  * (*ptLjk++);
	    }
	  if (lj!=li)
	    m_L[li*m_NbMaxOfConstraints+lj]=r/m_L[lj*m_NbMaxOfConstraints+lj];
	  else
	    m_L[li*m_NbMaxOfConstraints+lj] = sqrt(r);

	}
      pA+=m_NbMaxOfConstraints;
    }
  return 0;
  
}

int OptCholesky::ComputeInverseCholeskyNormal(int mode)
{
  if (m_iL==0)
    {
      cout << "no mem for iL" << endl;
      return -1;
    }

  int LocalSize =0;
  if (mode==0)
    LocalSize = m_SetActiveConstraints.size();
  else
    LocalSize = m_NbMaxOfConstraints;
    
  for(int lj=LocalSize-1;lj>=0;lj--)
    {
      double iLljlj=0.0;
      m_iL[lj*m_NbMaxOfConstraints+lj] = 
	iLljlj = 1/m_L[lj*m_NbMaxOfConstraints+lj];

      for(int li=lj+1;li<LocalSize;li++)
	{
	  
	  /* Compute Li,j */
	  double r = 0.0;
	  double * ptiLik = m_iL + li*m_NbMaxOfConstraints + lj + 1;
	  double * ptLjk  = m_L  + (lj+1)*m_NbMaxOfConstraints + lj ;
	  
	  for(int lk=lj+1;lk<LocalSize;lk++)
	    {
	      r = r + (*ptiLik++)  * (*ptLjk);
	      ptLjk+=m_NbMaxOfConstraints;
	    }
		      
	  m_iL[li*m_NbMaxOfConstraints+lj]= -iLljlj*r;
		      

	}
    }
  return 0;
  
}