/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <math.h>
#include "solver.h"

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

  int i, j, k;
  int irow = 0;
  int icol = 0;
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
