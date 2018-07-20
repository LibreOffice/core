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

// Natural, Clamped, or Periodic Cubic Splines
//
// Input:  A list of N+1 points (x_i,a_i), 0 <= i <= N, which are sampled
// from a function, a_i = f(x_i).  The function f is unknown.  Boundary
// conditions are
//   (1) Natural splines:  f"(x_0) = f"(x_N) = 0
//   (2) Clamped splines:  f'(x_0) and f'(x_N) are user-specified.
//   (3) Periodic splines:  f(x_0) = f(x_N) [in which case a_N = a_0 is
//       required in the input], f'(x_0) = f'(x_N), and f"(x_0) = f"(x_N).
//
// Output: b_i, c_i, d_i, 0 <= i <= N-1, which are coefficients for the cubic
// spline S_i(x) = a_i + b_i(x-x_i) + c_i(x-x_i)^2 + d_i(x-x_i)^3 for
// x_i <= x < x_{i+1}.
//
// The natural and clamped algorithms were implemented from
//
//    Numerical Analysis, 3rd edition
//    Richard L. Burden and J. Douglas Faires
//    Prindle, Weber & Schmidt
//    Boston, 1985, pp. 122-124.
//
// The algorithm sets up a tridiagonal linear system of equations in the
// c_i values.  This can be solved in O(N) time.
//
// The periodic spline algorithm was implemented from my own derivation.  The
// linear system of equations is not tridiagonal.  For now I use a standard
// linear solver that does not take advantage of the sparseness of the
// matrix.  Therefore for very large N, you may have to worry about memory
// usage.

#include <sal/config.h>
#include <memory>

#include "cspline.h"
#include "solver.h"

void NaturalSpline (int N, const double* x, const double* a, double*& b, double*& c,
    double*& d)
{
  const double oneThird = 1.0/3.0;

  int i;
  std::unique_ptr<double[]> h(new double[N]);
  std::unique_ptr<double[]> hdiff(new double[N]);
  std::unique_ptr<double[]> alpha(new double[N]);

  for (i = 0; i < N; i++){
    h[i] = x[i+1]-x[i];
  }

  for (i = 1; i < N; i++)
    hdiff[i] = x[i+1]-x[i-1];

  for (i = 1; i < N; i++)
  {
    double number = 3.0*(a[i+1]*h[i-1]-a[i]*hdiff[i]+a[i-1]*h[i]);
    double denom = h[i-1]*h[i];
    alpha[i] = number/denom;
  }

  std::unique_ptr<double[]> ell(new double[N+1]);
  std::unique_ptr<double[]> mu(new double[N]);
  std::unique_ptr<double[]> z(new double[N+1]);
  double recip;

  ell[0] = 1.0;
  mu[0] = 0.0;
  z[0] = 0.0;

  for (i = 1; i < N; i++)
  {
    ell[i] = 2.0*hdiff[i]-h[i-1]*mu[i-1];
    recip = 1.0/ell[i];
    mu[i] = recip*h[i];
    z[i] = recip*(alpha[i]-h[i-1]*z[i-1]);
  }
  ell[N] = 1.0;
  z[N] = 0.0;

  b = new double[N];
  c = new double[N+1];
  d = new double[N];

  c[N] = 0.0;

  for (i = N-1; i >= 0; i--)
  {
    c[i] = z[i]-mu[i]*c[i+1];
    recip = 1.0/h[i];
    b[i] = recip*(a[i+1]-a[i])-h[i]*(c[i+1]+2.0*c[i])*oneThird;
    d[i] = oneThird*recip*(c[i+1]-c[i]);
  }
}

void PeriodicSpline (int N, const double* x, const double* a, double*& b, double*& c,
    double*& d)
{
  std::unique_ptr<double[]> h(new double[N]);
  int i;
  for (i = 0; i < N; i++)
    h[i] = x[i+1]-x[i];

  double** mat = mgcLinearSystemD::NewMatrix(N+1);  // guaranteed to be zeroed memory
  c = mgcLinearSystemD::NewVector(N+1);   // guaranteed to be zeroed memory

  // c[0] - c[N] = 0
  mat[0][0] = +1.0f;
  mat[0][N] = -1.0f;

  // h[i-1]*c[i-1]+2*(h[i-1]+h[i])*c[i]+h[i]*c[i+1] =
  //   3*{(a[i+1]-a[i])/h[i] - (a[i]-a[i-1])/h[i-1]}
  for (i = 1; i <= N-1; i++)
  {
    mat[i][i-1] = h[i-1];
    mat[i][i  ] = 2.0f*(h[i-1]+h[i]);
    mat[i][i+1] = h[i];
    c[i] = 3.0f*((a[i+1]-a[i])/h[i] - (a[i]-a[i-1])/h[i-1]);
  }

  // "wrap around equation" for periodicity
  // h[N-1]*c[N-1]+2*(h[N-1]+h[0])*c[0]+h[0]*c[1] =
  //   3*{(a[1]-a[0])/h[0] - (a[0]-a[N-1])/h[N-1]}
  mat[N][N-1] = h[N-1];
  mat[N][0  ] = 2.0f*(h[N-1]+h[0]);
  mat[N][1  ] = h[0];
  c[N] = 3.0f*((a[1]-a[0])/h[0] - (a[0]-a[N-1])/h[N-1]);

  // solve for c[0] through c[N]
  mgcLinearSystemD::Solve(N+1,mat,c);

  const double oneThird = 1.0/3.0;
  b = new double[N];
  d = new double[N];
  for (i = 0; i < N; i++)
  {
    b[i] = (a[i+1]-a[i])/h[i] - oneThird*(c[i+1]+2.0f*c[i])*h[i];
    d[i] = oneThird*(c[i+1]-c[i])/h[i];
  }

  mgcLinearSystemD::DeleteMatrix(N+1,mat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
