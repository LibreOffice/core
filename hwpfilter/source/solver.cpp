/*************************************************************************
 *
 *  $RCSfile: solver.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:42:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <math.h>
#include "solver.h"

//---------------------------------------------------------------------------
float** mgcLinearSystem::NewMatrix (int N)
{
  float** A = new float*[N];
  if ( !A )
    return 0;

  for (int row = 0; row < N; row++)
  {
    A[row] = new float[N];
    if ( !A[row] )
    {
      for (int i = 0; i < row; i++)
    delete[] A[i];
      return 0;
    }
    for (int col = 0; col < N; col++)
      A[row][col] = 0;
  }
  return A;
}
//---------------------------------------------------------------------------
void mgcLinearSystem::DeleteMatrix (int N, float** A)
{
  for (int row = 0; row < N; row++)
    delete[] A[row];
  delete[] A;
}
//---------------------------------------------------------------------------
float* mgcLinearSystem::NewVector (int N)
{
  float* B = new float[N];
  if ( !B )
    return 0;

  for (int row = 0; row < N; row++)
    B[row] = 0;
  return B;
}
//---------------------------------------------------------------------------
void mgcLinearSystem::DeleteVector (int , float* B)
{
  delete[] B;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::Inverse (int n, float** a)
{
  int* indxc = new int[n];
  int* indxr = new int[n];
  int* ipiv  = new int[n];

  int i, j, k, irow, icol;
  float big, pivinv, save;

  for (j = 0; j < n; j++)
    ipiv[j] = 0;

  for (i = 0; i < n; i++)
  {
    big = 0;
    for (j = 0; j < n; j++)
    {
      if ( ipiv[j] != 1 )
      {
    for (k = 0; k < n; k++)
    {
      if ( ipiv[k] == 0 )
      {
        if ( fabs(a[j][k]) >= big )
        {
          big = (float)fabs(a[j][k]);
          irow = j;
          icol = k;
        }
      }
      else if ( ipiv[k] > 1 )
      {
        delete[] ipiv;
        delete[] indxr;
        delete[] indxc;
        return 0;
      }
    }
      }
    }
    ipiv[icol]++;

    if ( irow != icol )
    {
      float* rowptr = a[irow];
      a[irow] = a[icol];
      a[icol] = rowptr;
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if ( a[icol][icol] == 0 )
    {
      delete[] ipiv;
      delete[] indxr;
      delete[] indxc;
      return 0;
    }

    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1;
    for (k = 0; k < n; k++)
      a[icol][k] *= pivinv;

    for (j = 0; j < n; j++)
    {
      if ( j != icol )
      {
    save = a[j][icol];
    a[j][icol] = 0;
    for (k = 0; k < n; k++)
      a[j][k] -= a[icol][k]*save;
      }
    }
  }

  for (j = n-1; j >= 0; j--)
  {
    if ( indxr[j] != indxc[j] )
    {
      for (k = 0; k < n; k++)
      {
    save = a[k][indxr[j]];
    a[k][indxr[j]] = a[k][indxc[j]];
    a[k][indxc[j]] = save;
      }
    }
  }

  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::Solve (int n, float** a, float* b)
{
  int* indxc = new int[n];
  if ( !indxc )
    return 0;
  int* indxr = new int[n];
  if ( !indxr )
  {
    delete[] indxc;
    return 0;
  }
  int* ipiv  = new int[n];
  if ( !ipiv ) {
    delete[] indxc;
    delete[] indxr;
    return 0;
  }

  int i, j, k, irow, icol;
  float big, pivinv, save;

  for (j = 0; j < n; j++)
    ipiv[j] = 0;

  for (i = 0; i < n; i++)
  {
    big = 0;
    for (j = 0; j < n; j++)
    {
      if ( ipiv[j] != 1 )
      {
    for (k = 0; k < n; k++)
    {
      if ( ipiv[k] == 0 )
      {
        if ( (float)fabs(a[j][k]) >= big )
        {
          big = (float)fabs(a[j][k]);
          irow = j;
          icol = k;
        }
      }
      else if ( ipiv[k] > 1 )
      {
        delete[] ipiv;
        delete[] indxr;
        delete[] indxc;
        return 0;
      }
    }
      }
    }
    ipiv[icol]++;

    if ( irow != icol )
    {
      float* rowptr = a[irow];
      a[irow] = a[icol];
      a[icol] = rowptr;

      save = b[irow];
      b[irow] = b[icol];
      b[icol] = save;
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if ( a[icol][icol] == 0 )
    {
      delete[] ipiv;
      delete[] indxr;
      delete[] indxc;
      return 0;
    }

    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1;
    for (k = 0; k < n; k++)
      a[icol][k] *= pivinv;
    b[icol] *= pivinv;

    for (j = 0; j < n; j++)
    {
      if ( j != icol )
      {
    save = a[j][icol];
    a[j][icol] = 0;
    for (k = 0; k < n; k++)
      a[j][k] -= a[icol][k]*save;
    b[j] -= b[icol]*save;
      }
    }
  }

  for (j = n-1; j >= 0; j--)
  {
    if ( indxr[j] != indxc[j] )
    {
      for (k = 0; k < n; k++)
      {
    save = a[k][indxr[j]];
    a[k][indxr[j]] = a[k][indxc[j]];
    a[k][indxc[j]] = save;
      }
    }
  }

  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::SolveTri (int n, float* a, float* b, float* c,
    float* r, float* u)
{
  if ( b[0] == 0 )
    return 0;

  float* gam = new float[n-1];
  if ( !gam )
    return 0;

  float bet = b[0];
  u[0] = r[0]/bet;
  int i, j;
  for (i = 0, j = 1; j < n; i++, j++)
  {
    gam[i] = c[i]/bet;
    bet = b[j]-a[i]*gam[i];
    if ( bet == 0 )
    {
      delete[] gam;
      return 0;
    }
    u[j] = (r[j]-a[i]*u[i])/bet;
  }
  for (i = n-1, j = n-2; j >= 0; i--, j--)
    u[j] -= gam[j]*u[i];

  delete[] gam;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::SolveConstTri (int n, float a, float b, float c,
    float* r, float* u)
{
  if ( b == 0 )
    return 0;

  float* gam = new float[n-1];
  if ( !gam )
    return 0;

  float bet = b;
  u[0] = r[0]/bet;
  int i, j;
  for (i = 0, j = 1; j < n; i++, j++)
  {
    gam[i] = c/bet;
    bet = b-a*gam[i];
    if ( bet == 0 )
    {
      delete[] gam;
      return 0;
    }
    u[j] = (r[j]-a*u[i])/bet;
  }
  for (i = n-1, j = n-2; j >= 0; i--, j--)
    u[j] -= gam[j]*u[i];

  delete[] gam;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::SolveSymmetric (int n, float** A, float* b)
{
  // A = L D L^t decomposition with diagonal terms of L equal to 1
  // Algorithm stores D terms in A[i][i] and off-diagonal L terms in
  // A[i][j] for i > j.  (G. Golub and C. Van Loan, Matrix Computations)

  const float tolerance = 1e-06f;

  int i, j, k;
  float* v = new float[n];
  if ( !v )
    return 0;

  for (j = 0; j < n; j++)
  {
    for (i = 0; i < j; i++)
      v[i] = A[j][i]*A[i][i];

    v[j] = A[j][j];
    for (i = 0; i < j; i++)
      v[j] -= A[j][i]*v[i];

    A[j][j] = v[j];
    if ( fabs(v[j]) <= tolerance )
    {
      delete[] v;
      return 0;
    }
    for (i = j+1; i < n; i++)
    {
      for (k = 0; k < j; k++)
    A[i][j] -= A[i][k]*v[k];
      A[i][j] /= v[j];
    }
  }
  delete[] v;

    // Solve Ax = b.

    // Forward substitution:  Let z = DL^t x, then Lz = b.  Algorithm
    // stores z terms in b vector.
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < i; j++)
      b[i] -= A[i][j]*b[j];

  }

  // Diagonal division:  Let y = L^t x, then Dy = z.  Algorithm stores
  // y terms in b vector.
  for (i = 0; i < n; i++)
  {
    if ( fabs(A[i][i]) <= tolerance )
      return 0;
    b[i] /= A[i][i];
  }

  // Back substitution:  Solve L^t x = y.  Algorithm stores x terms in
  // b vector.
  for (i = n-2; i >= 0; i--)
  {
    for (j = i+1; j < n; j++)
      b[i] -= A[j][i]*b[j];
  }

  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystem::SymmetricInverse (int n, float** A, float** Ainv)
{
  // Same algorithm as SolveSymmetric, but applied simultaneously to
  // columns of identity matrix.

  int i, j, k;
  float* v = new float[n];
  if ( !v )
    return 0;

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
      Ainv[i][j] = ( i != j ? 0.0f : 1.0f );
  }

  for (j = 0; j < n; j++)
  {
    for (i = 0; i < j; i++)
      v[i] = A[j][i]*A[i][i];

    v[j] = A[j][j];
    for (i = 0; i < j; i++)
      v[j] -= A[j][i]*v[i];

    A[j][j] = v[j];
    for (i = j+1; i < n; i++)
    {
      for (k = 0; k < j; k++)
    A[i][j] -= A[i][k]*v[k];
      A[i][j] /= v[j];
    }
  }
  delete[] v;

  for (int col = 0; col < n; col++)
  {
    // forward substitution
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < i; j++)
    Ainv[i][col] -= A[i][j]*Ainv[j][col];
    }

    // diagonal division
    const float tolerance = 1e-06f;
    for (i = 0; i < n; i++)
    {
      if ( fabs(A[i][i]) <= tolerance )
    return 0;
      Ainv[i][col] /= A[i][i];
    }

    // back substitution
    for (i = n-2; i >= 0; i--)
    {
      for (j = i+1; j < n; j++)
    Ainv[i][col] -= A[j][i]*Ainv[j][col];
    }
  }

  return 1;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
double** mgcLinearSystemD::NewMatrix (int N)
{
  double** A = new double*[N];
  if ( !A )
    return 0;

  for (int row = 0; row < N; row++)
  {
    A[row] = new double[N];
    if ( !A[row] )
    {
      for (int i = 0; i < row; i++)
    delete[] A[i];
      return 0;
    }
    for (int col = 0; col < N; col++)
      A[row][col] = 0;
  }
  return A;
}
//---------------------------------------------------------------------------
void mgcLinearSystemD::DeleteMatrix (int N, double** A)
{
  for (int row = 0; row < N; row++)
    delete[] A[row];
  delete[] A;
}
//---------------------------------------------------------------------------
double* mgcLinearSystemD::NewVector (int N)
{
  double* B = new double[N];
  if ( !B )
    return 0;

  for (int row = 0; row < N; row++)
    B[row] = 0;
  return B;
}
//---------------------------------------------------------------------------
void mgcLinearSystemD::DeleteVector (int , double* B)
{
  delete[] B;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::Inverse (int n, double** a)
{
  int* indxc = new int[n];
  int* indxr = new int[n];
  int* ipiv  = new int[n];

  int i, j, k, irow, icol;
  double big, pivinv, save;

  for (j = 0; j < n; j++)
    ipiv[j] = 0;

  for (i = 0; i < n; i++)
  {
    big = 0;
    for (j = 0; j < n; j++)
    {
      if ( ipiv[j] != 1 )
      {
    for (k = 0; k < n; k++)
    {
      if ( ipiv[k] == 0 )
      {
        if ( fabs(a[j][k]) >= big )
        {
          big = fabs(a[j][k]);
          irow = j;
          icol = k;
        }
      }
      else if ( ipiv[k] > 1 )
      {
        delete[] ipiv;
        delete[] indxr;
        delete[] indxc;
        return 0;
      }
    }
      }
    }
    ipiv[icol]++;

    if ( irow != icol )
    {
      double* rowptr = a[irow];
      a[irow] = a[icol];
      a[icol] = rowptr;
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if ( a[icol][icol] == 0 )
    {
      delete[] ipiv;
      delete[] indxr;
      delete[] indxc;
      return 0;
    }

    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1;
    for (k = 0; k < n; k++)
      a[icol][k] *= pivinv;

    for (j = 0; j < n; j++)
    {
      if ( j != icol )
      {
    save = a[j][icol];
    a[j][icol] = 0;
    for (k = 0; k < n; k++)
      a[j][k] -= a[icol][k]*save;
      }
    }
  }

  for (j = n-1; j >= 0; j--)
  {
    if ( indxr[j] != indxc[j] )
    {
      for (k = 0; k < n; k++)
      {
    save = a[k][indxr[j]];
    a[k][indxr[j]] = a[k][indxc[j]];
    a[k][indxc[j]] = save;
      }
    }
  }

  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::Solve (int n, double** a, double* b)
{
  int* indxc = new int[n];
  if ( !indxc )
    return 0;
  int* indxr = new int[n];
  if ( !indxr ) {
    delete[] indxc;
    return 0;
  }
  int* ipiv  = new int[n];
  if ( !ipiv ) {
    delete[] indxc;
    delete[] indxr;
    return 0;
  }

  int i, j, k, irow, icol;
  double big, pivinv, save;

  for (j = 0; j < n; j++)
    ipiv[j] = 0;

  for (i = 0; i < n; i++)
  {
    big = 0;
    for (j = 0; j < n; j++)
    {
      if ( ipiv[j] != 1 )
      {
    for (k = 0; k < n; k++)
    {
      if ( ipiv[k] == 0 )
      {
        if ( fabs(a[j][k]) >= big )
        {
          big = fabs(a[j][k]);
          irow = j;
          icol = k;
        }
      }
      else if ( ipiv[k] > 1 )
      {
        delete[] ipiv;
        delete[] indxr;
        delete[] indxc;
        return 0;
      }
    }
      }
    }
    ipiv[icol]++;

    if ( irow != icol )
    {
      double* rowptr = a[irow];
      a[irow] = a[icol];
      a[icol] = rowptr;

      save = b[irow];
      b[irow] = b[icol];
      b[icol] = save;
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if ( a[icol][icol] == 0 )
    {
      delete[] ipiv;
      delete[] indxr;
      delete[] indxc;
      return 0;
    }

    pivinv = 1/a[icol][icol];
    a[icol][icol] = 1;
    for (k = 0; k < n; k++)
      a[icol][k] *= pivinv;
    b[icol] *= pivinv;

    for (j = 0; j < n; j++)
    {
      if ( j != icol )
      {
    save = a[j][icol];
    a[j][icol] = 0;
    for (k = 0; k < n; k++)
      a[j][k] -= a[icol][k]*save;
    b[j] -= b[icol]*save;
      }
    }
  }

  for (j = n-1; j >= 0; j--)
  {
    if ( indxr[j] != indxc[j] )
    {
      for (k = 0; k < n; k++)
      {
    save = a[k][indxr[j]];
    a[k][indxr[j]] = a[k][indxc[j]];
    a[k][indxc[j]] = save;
      }
    }
  }

  delete[] ipiv;
  delete[] indxr;
  delete[] indxc;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::SolveTri (int n, double* a, double* b, double* c,
    double* r, double* u)
{
  if ( b[0] == 0 )
    return 0;

  double* gam = new double[n-1];
  if ( !gam )
    return 0;

  double bet = b[0];
  u[0] = r[0]/bet;
  int i, j;
  for (i = 0, j = 1; j < n; i++, j++)
  {
    gam[i] = c[i]/bet;
    bet = b[j]-a[i]*gam[i];
    if ( bet == 0 )
    {
      delete[] gam;
      return 0;
    }
    u[j] = (r[j]-a[i]*u[i])/bet;
  }
  for (i = n-1, j = n-2; j >= 0; i--, j--)
    u[j] -= gam[j]*u[i];

  delete[] gam;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::SolveConstTri (int n, double a, double b, double c,
    double* r, double* u)
{
  if ( b == 0 )
    return 0;

  double* gam = new double[n-1];
  if ( !gam )
    return 0;

  double bet = b;
  u[0] = r[0]/bet;
  int i, j;
  for (i = 0, j = 1; j < n; i++, j++)
  {
    gam[i] = c/bet;
    bet = b-a*gam[i];
    if ( bet == 0 )
    {
      delete[] gam;
      return 0;
    }
    u[j] = (r[j]-a*u[i])/bet;
  }
  for (i = n-1, j = n-2; j >= 0; i--, j--)
    u[j] -= gam[j]*u[i];

  delete[] gam;
  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::SolveSymmetric (int n, double** A, double* b)
{
  // A = L D L^t decomposition with diagonal terms of L equal to 1
  // Algorithm stores D terms in A[i][i] and off-diagonal L terms in
  // A[i][j] for i > j.  (G. Golub and C. Van Loan, Matrix Computations)

  const double tolerance = 1e-06;

  int i, j, k;
  double* v = new double[n];
  if ( !v )
    return 0;

  for (j = 0; j < n; j++)
  {
    for (i = 0; i < j; i++)
      v[i] = A[j][i]*A[i][i];

    v[j] = A[j][j];
    for (i = 0; i < j; i++)
      v[j] -= A[j][i]*v[i];

    A[j][j] = v[j];
    if ( fabs(v[j]) <= tolerance )
    {
      delete[] v;
      return 0;
    }
    for (i = j+1; i < n; i++)
    {
      for (k = 0; k < j; k++)
    A[i][j] -= A[i][k]*v[k];
      A[i][j] /= v[j];
    }
  }
  delete[] v;

    // Solve Ax = b.

    // Forward substitution:  Let z = DL^t x, then Lz = b.  Algorithm
    // stores z terms in b vector.
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < i; j++)
      b[i] -= A[i][j]*b[j];

  }

  // Diagonal division:  Let y = L^t x, then Dy = z.  Algorithm stores
  // y terms in b vector.
  for (i = 0; i < n; i++)
  {
    if ( fabs(A[i][i]) <= tolerance )
      return 0;
    b[i] /= A[i][i];
  }

  // Back substitution:  Solve L^t x = y.  Algorithm stores x terms in
  // b vector.
  for (i = n-2; i >= 0; i--)
  {
    for (j = i+1; j < n; j++)
      b[i] -= A[j][i]*b[j];
  }

  return 1;
}
//---------------------------------------------------------------------------
int mgcLinearSystemD::SymmetricInverse (int n, double** A, double** Ainv)
{
  // Same algorithm as SolveSymmetric, but applied simultaneously to
  // columns of identity matrix.

  int i, j, k;
  double* v = new double[n];
  if ( !v )
    return 0;

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
      Ainv[i][j] = ( i != j ? 0 : 1 );
  }

  for (j = 0; j < n; j++)
  {
    for (i = 0; i < j; i++)
      v[i] = A[j][i]*A[i][i];

    v[j] = A[j][j];
    for (i = 0; i < j; i++)
      v[j] -= A[j][i]*v[i];

    A[j][j] = v[j];
    for (i = j+1; i < n; i++)
    {
      for (k = 0; k < j; k++)
    A[i][j] -= A[i][k]*v[k];
      A[i][j] /= v[j];
    }
  }
  delete[] v;

  for (int col = 0; col < n; col++)
  {
    // forward substitution
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < i; j++)
    Ainv[i][col] -= A[i][j]*Ainv[j][col];
    }

    // diagonal division
    const double tolerance = 1e-06;
    for (i = 0; i < n; i++)
    {
      if ( fabs(A[i][i]) <= tolerance )
    return 0;
      Ainv[i][col] /= A[i][i];
    }

    // back substitution
    for (i = n-2; i >= 0; i--)
    {
      for (j = i+1; j < n; j++)
    Ainv[i][col] -= A[j][i]*Ainv[j][col];
    }
  }

  return 1;
}
//---------------------------------------------------------------------------
