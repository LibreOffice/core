/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


//







//



//

//




//


//






#include "solver.h"

void NaturalSpline (int N, double* x, double* a, double*& b, double*& c,
    double*& d)
{
  const double oneThird = 1.0/3.0;

  int i;
  double* h = new double[N];
  double* hdiff = new double[N];
  double* alpha = new double[N];

  for (i = 0; i < N; i++){
    h[i] = x[i+1]-x[i];
  }

  for (i = 1; i < N; i++)
    hdiff[i] = x[i+1]-x[i-1];

  for (i = 1; i < N; i++)
  {
    double numer = 3.0*(a[i+1]*h[i-1]-a[i]*hdiff[i]+a[i-1]*h[i]);
    double denom = h[i-1]*h[i];
    alpha[i] = numer/denom;
  }

  double* ell = new double[N+1];
  double* mu = new double[N];
  double* z = new double[N+1];
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

  delete[] h;
  delete[] hdiff;
  delete[] alpha;
  delete[] ell;
  delete[] mu;
  delete[] z;
}

void PeriodicSpline (int N, double* x, double* a, double*& b, double*& c,
    double*& d)
{
  double* h = new double[N];
  int i;
  for (i = 0; i < N; i++)
    h[i] = x[i+1]-x[i];

  mgcLinearSystemD sys;
  double** mat = sys.NewMatrix(N+1);  
  c = sys.NewVector(N+1);   

  
  mat[0][0] = +1.0f;
  mat[0][N] = -1.0f;

  
  
  for (i = 1; i <= N-1; i++)
  {
    mat[i][i-1] = h[i-1];
    mat[i][i  ] = 2.0f*(h[i-1]+h[i]);
    mat[i][i+1] = h[i];
    c[i] = 3.0f*((a[i+1]-a[i])/h[i] - (a[i]-a[i-1])/h[i-1]);
  }

  
  
  
  mat[N][N-1] = h[N-1];
  mat[N][0  ] = 2.0f*(h[N-1]+h[0]);
  mat[N][1  ] = h[0];
  c[N] = 3.0f*((a[1]-a[0])/h[0] - (a[0]-a[N-1])/h[N-1]);

  
  sys.Solve(N+1,mat,c);

  const double oneThird = 1.0/3.0;
  b = new double[N];
  d = new double[N];
  for (i = 0; i < N; i++)
  {
    b[i] = (a[i+1]-a[i])/h[i] - oneThird*(c[i+1]+2.0f*c[i])*h[i];
    d[i] = oneThird*(c[i+1]-c[i])/h[i];
  }

  delete[] h;
  sys.DeleteMatrix(N+1,mat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
