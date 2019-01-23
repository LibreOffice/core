/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <math.h>
#include <memory>
#include "solver.h"


std::unique_ptr<std::unique_ptr<double[]>[]> mgcLinearSystemD::NewMatrix (int N)
{
  std::unique_ptr<std::unique_ptr<double[]>[]> A(new std::unique_ptr<double[]>);

  for (int row = 0; row < N; row++)
  {
    A[row].reset(new double[N]);
    for (int col = 0; col < N; col++)
      A[row][col] = 0;
  }
  return A;
}

std::unique_ptr<double[]> mgcLinearSystemD::NewVector (int N)
{
  std::unique_ptr<double[]> B(new double[N]);

  for (int row = 0; row < N; row++)
    B[row] = 0;
  return B;
}

bool mgcLinearSystemD::Solve (int n, std::unique_ptr<std::unique_ptr<double[]>[]> const & a, double* b)
{
  std::unique_ptr<int[]> indxc( new int[n] );
  if ( !indxc )
    return false;
  std::unique_ptr<int[]> indxr( new int[n] );
  if ( !indxr ) {
    return false;
  }
  std::unique_ptr<int[]> ipiv( new int[n] );
  if ( !ipiv ) {
    return false;
  }

  int i, j, k;
  int irow = 0;
  int icol = 0;
  double save;

  for (j = 0; j < n; j++)
    ipiv[j] = 0;

  for (i = 0; i < n; i++)
  {
    double big = 0;
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
            return false;
          }
        }
      }
    }
    ipiv[icol]++;

    if ( irow != icol )
    {
      std::swap(a[irow], a[icol]);

      save = b[irow];
      b[irow] = b[icol];
      b[icol] = save;
    }

    indxr[i] = irow;
    indxc[i] = icol;
    if ( a[icol][icol] == 0 )
    {
      return false;
    }

    double pivinv = 1/a[icol][icol];
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

  return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
