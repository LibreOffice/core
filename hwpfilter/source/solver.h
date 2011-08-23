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

#ifndef _SOLVER_H_
#define _SOLVER_H_

class mgcLinearSystemD
{
public:
  mgcLinearSystemD() {;}

  double** NewMatrix (int N);
  void DeleteMatrix (int N, double** A);
  double* NewVector (int N);

  int Solve (int N, double** A, double* b);
  // Input:
  //     A[N][N] coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N] is inverse matrix
  //     b[N] is solution x to Ax = b
};

#endif
