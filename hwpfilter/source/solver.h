/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: solver.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:45:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SOLVER_H_
#define _SOLVER_H_

class mgcLinearSystem
{
public:
  mgcLinearSystem() {;}

  float** NewMatrix (int N);
  void DeleteMatrix (int N, float** A);
  float* NewVector (int N);
  void DeleteVector (int N, float* B);

  int Inverse (int N, float** A);
  // Input:
  //     A[N][N], entries are A[row][col]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N], inverse matrix

  int Solve (int N, float** A, float* b);
  // Input:
  //     A[N][N] coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N] is inverse matrix
  //     b[N] is solution x to Ax = b

  int SolveTri (int N, float* a, float* b, float* c, float* r, float* u);
  // Input:
  //     Matrix is tridiagonal.
  //     Lower diagonal a[N-1]
  //     Main  diagonal b[N]
  //     Upper diagonal c[N-1]
  //     Right-hand side r[N]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     u[N] is solution

  int SolveConstTri (int N, float a, float b, float c, float* r, float* u);
  // Input:
  //     Matrix is tridiagonal.
  //     Lower diagonal is constant, a
  //     Main  diagonal is constant, b
  //     Upper diagonal is constant, c
  //     Right-hand side r[N]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     u[N] is solution

  int SolveSymmetric (int N, float** A, float* b);
  // Input:
  //     A[N][N] symmetric coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if (nearly) singular
  //     decomposition A = L D L^t (diagonal terms of L are all 1)
  //         A[i][i] = entries of diagonal D
  //         A[i][j] for i > j = lower triangular part of L
  //     b[N] is solution to x to Ax = b

  int SymmetricInverse (int N, float** A, float** Ainv);
  // Input:
  //     A[N][N], entries are A[row][col]
  // Output:
  //     return value is TRUE if successful, FALSE if algorithm failed
  //     Ainv[N][N], inverse matrix
};

class mgcLinearSystemD
{
public:
  mgcLinearSystemD() {;}

  double** NewMatrix (int N);
  void DeleteMatrix (int N, double** A);
  double* NewVector (int N);
  void DeleteVector (int N, double* B);

  int Inverse (int N, double** A);
  // Input:
  //     A[N][N], entries are A[row][col]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N], inverse matrix

  int Solve (int N, double** A, double* b);
  // Input:
  //     A[N][N] coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N] is inverse matrix
  //     b[N] is solution x to Ax = b

  int SolveTri (int N, double* a, double* b, double* c, double* r,
      double* u);
  // Input:
  //     Matrix is tridiagonal.
  //     Lower diagonal a[N-1]
  //     Main  diagonal b[N]
  //     Upper diagonal c[N-1]
  //     Right-hand side r[N]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     u[N] is solution

  int SolveConstTri (int N, double a, double b, double c, double* r,
      double* u);
  // Input:
  //     Matrix is tridiagonal.
  //     Lower diagonal is constant, a
  //     Main  diagonal is constant, b
  //     Upper diagonal is constant, c
  //     Right-hand side r[N]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     u[N] is solution

  int SolveSymmetric (int N, double** A, double* b);
  // Input:
  //     A[N][N] symmetric coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if (nearly) singular
  //     decomposition A = L D L^t (diagonal terms of L are all 1)
  //         A[i][i] = entries of diagonal D
  //         A[i][j] for i > j = lower triangular part of L
  //     b[N] is solution to x to Ax = b

  int SymmetricInverse (int N, double** A, double** Ainv);
  // Input:
  //     A[N][N], entries are A[row][col]
  // Output:
  //     return value is TRUE if successful, FALSE if algorithm failed
  //     Ainv[N][N], inverse matrix
};

#endif
