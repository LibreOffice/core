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

#include <tools/poly.hxx>
#include <memory>
#include <rtl/math.hxx>

#include <sgvspln.hxx>
#include <cmath>

#if defined(ANDROID) || ( defined(__APPLE__) && (defined(__POWERPC__) || defined(__ppc__)) )
namespace std
{
template<typename T>
T copysign(T x, T y)
{
    return ::copysign(x, y);
}
}
#endif

extern "C" {

/*.pn 277 */
/*.hlAppendix: C - programs*/
/*.hrConstants- and macrodefinitions*/
/*.fe The include file u_const.h should be stored in the directory,   */
/*.fe where the compiler searches for include files.                  */

/*-----------------------  FILE u_const.h  ---------------------------*/

#define IEEE

/* IEEE - standard for representation of floating-point numbers:

     8 byte long floating point numbers with

    53 bit mantissa  ==> mantissa range:     2^52 different numbers
                                              with 0.1 <= number < 1.0,
                                              1 sign-bit
    11 bit exponent  ==> exponent range:  -1024...+1023

  The first line (#define IEEE) should be deleted if the machine
  or the compiler does not use floating-point numbers according
  to the IEEE standard. In which case also MAXEXPON, MINEXPON (see
  below) should be adapted.
*/

#ifdef IEEE         /*-------------- if IEEE norm --------------------*/

#define MACH_EPS  2.220446049250313e-016      /* machine precision    */
                                              /* IBM-AT:  = 2^-52    */
/* MACH_EPS is the smallest positive,  by the machine representable
   number x, which fulfills the equation: 1.0 + x > 1.0               */

#define MAXROOT   9.48075190810918e+153

#else               /*------------------ otherwise--------------------*/

double exp  (double);
double atan (double);
double pow  (double,double);
double sqrt (double);

double masch()            /* calculate MACH_EPS machine independence   */
{
  double eps = 1.0, x = 2.0, y = 1.0;
  while ( y < x )
    { eps *= 0.5;
      x = 1.0 + eps;
    }
  eps *= 2.0;
  return eps;
}

short basis()              /* calculate BASE machine independence     */
{
  double x = 1.0, one = 1.0, b = 1.0;

  while ( (x + one) - x == one ) x *= 2.0;
  while ( (x + b) == x ) b *= 2.0;

  return (short) ((x + b) - x);
}

#define BASIS     basis()           /* base of number representation  */

/* If the machine (the compiler) does not use the IEEE-representation
   for floating-point numbers, the next 2 constants should be adapted.
   */

#define MAXEXPON  1023.0                    /* largest exponent       */
#define MINEXPON -1024.0                    /* smallest exponent      */

#define MACH_EPS  masch()

#define POSMAX    pow ((double) BASIS, MAXEXPON)
#define MAXROOT   sqrt(POSMAX)

#endif              /*-------------- END of ifdef --------------------*/

/*--------------------  END of FILE u_const.h  -----------------------*/

/*.HL appendix: C - programs*/
/*.HR Systems of equations for tridiagonal matrices*/

/*.FE  P 3.7 tridiagonal systems of equations */

/*----------------------   MODULE tridiagonal  -----------------------*/

sal_uInt16 TriDiagGS(sal_uInt16 n, double* lower,
                 double* diag, double* upper, double* b)
                                             /*************************/
                                             /* Gaussian methods for  */
                                             /* tridiagonal matrices  */
                                             /*************************/

/*====================================================================*/
/*                                                                    */
/*  trdiag determines solution x of the system of linear equations    */
/*  A * x = b with tridiagonal n x n coefficient matrix A, which is   */
/*  stored in 3 vectors lower, upper and diag as per below:           */
/*                                                                    */
/*       ( diag[0]  upper[0]    0        0  .   .     .   0      )    */
/*       ( lower[1] diag[1]   upper[1]   0      .     .   .      )    */
/*       (   0      lower[2]  diag[2]  upper[2]   0       .      )    */
/*  A =  (   .        0       lower[3]  .     .       .          )    */
/*       (   .          .           .        .     .      0      )    */
/*       (   .             .            .        .      .        )    */
/*       (                   .             .        . upper[n-2] )    */
/*       (   0 .   .    .       0        lower[n-1]   diag[n-1]  )    */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Usage:                                                           */
/*   ======                                                           */
/*      Mainly for diagonal determinant triangular matrix, as they    */
/*      occur in Spline-interpolations.                               */
/*      For diagonal dominant matrices always a left-upper row        */
/*      reduction exists; for non diagonal dominant triangular        */
/*      matrices we should pull forward the function band, as this    */
/*      works with row pivot searches, which is numerical more stable.*/
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Input parameters:                                                */
/*   ================                                                 */
/*      n        dimension of the matrix ( > 1 )  sal_uInt16 n        */
/*                                                                    */
/*      lower    lower antidiagonal               double lower[n]     */
/*      diag     main diagonal                    double diag[n]      */
/*      upper    upper antidiagonal               double upper[n]     */
/*                                                                    */
/*               for rep = true lower, diag and upper contain the     */
/*               triangulation of the start matrix.                   */
/*                                                                    */
/*      b        right side of equation     double b[n]               */
/*      rep      = false first call         bool rep                  */
/*               = true  next call                                    */
/*                    for the same matrix,                            */
/*                    but different b.                                */
/*                                                                    */
/*   Output parameters:                                               */
/*   =================                                                */
/*      b        solution vector of the system;   double b[n]         */
/*               the original right side is overwritten               */
/*                                                                    */
/*      lower    ) contain for rep = false the decomposition of the   */
/*      diag     ) matrix; the original values of the lower and       */
/*      upper    ) diagonals are overwritten                          */
/*                                                                    */
/*   The determinant of the matrix is for rep = false defined by      */
/*      determinant A = diag[0] * ... * diag[n-1]                     */
/*                                                                    */
/*   Return value:                                                    */
/*   =============                                                    */
/*      = 0      all ok                                               */
/*      = 1      n < 2 chosen                                         */
/*      = 2      triangular decomposition of matrix does not exist    */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Functions used:                                                  */
/*   ===============                                                  */
/*                                                                    */
/*   From the C library: fabs()                                       */
/*                                                                    */
/*====================================================================*/

/*.cp 5 */
{
 sal_uInt16 i;
 short  j;

// double fabs(double);

 if ( n < 2 ) return 1;                    /*  n at least 2          */

                                            /*  determine the         */
                                            /*  triangular            */
                                            /*  decomposition of      */
                                           /*  matrix and determinant*/
   {
     for (i = 1; i < n; i++)
       { if ( fabs(diag[i-1]) < MACH_EPS )  /*  do not decompose      */
           return 2;                        /*  if one diag[i] = 0    */
         lower[i] /= diag[i-1];
         diag[i] -= lower[i] * upper[i-1];
       }
    }

 if ( fabs(diag[n-1]) < MACH_EPS ) return 2;

 for (i = 1; i < n; i++)                    /* forward elimination    */
    b[i] -= lower[i] * b[i-1];

 b[n-1] /= diag[n-1];                       /* reverse elimination    */
 for (j = n-2; j >= 0; j--) {
    i=j;
    b[i] = ( b[i] - upper[i] * b[i+1] ) / diag[i];
 }
 return 0;
}

/*-----------------------  END OF TRIDIAGONAL  ------------------------*/

/*.HL Appendix: C - Programs*/
/*.HRSystems of equations with cyclic tridiagonal matrices*/

/*.FE  P 3.8  Systems with cyclic tridiagonal matrices    */

/*----------------  Module cyclic tridiagonal  -----------------------*/

sal_uInt16 ZyklTriDiagGS(sal_uInt16 n, double* lower, double* diag,
                     double* upper, double* lowrow, double* ricol, double* b)
                                        /******************************/
                                        /* Systems with cyclic        */
                                        /* tridiagonal matrices       */
                                        /******************************/

/*====================================================================*/
/*                                                                    */
/*  tzdiag determines the solution x of the linear equation system    */
/*  A * x = b with cyclic tridiagonal  n x n coefficient-             */
/*  matrix A, which is stored in the 5 vectors: lower, upper, diag,   */
/*  lowrow and ricol as per below:                                    */
/*                                                                    */
/*       ( diag[0]  upper[0]    0        0  .   . 0   ricol[0]   )    */
/*       ( lower[1] diag[1]   upper[1]   0      .     .   0      )    */
/*       (   0      lower[2]  diag[2]  upper[2]   0       .      )    */
/*  A =  (   .        0       lower[3]  .     .       .   .      )    */
/*       (   .          .           .        .     .      0      )    */
/*       (   .             .            .        .      .        )    */
/*       (   0               .             .        . upper[n-2] )    */
/*       ( lowrow[0]  0 .  .    0        lower[n-1]   diag[n-1]  )    */
/*                                                                    */
/*  Memory for lowrow[1],..,lowrow[n-3] und ricol[1],...,ricol[n-3]   */
/*  should be provided separately, as this should be available to     */
/*  store the decomposition matrix, which is overwriting              */
/*  the 5 vectors mentioned.                                          */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Usage:                                                           */
/*   ======                                                           */
/*      Predominantly for diagonal dominant cyclic tridiagonal-       */
/*      matrices as they occur in spline-interpolations.              */
/*      For diagonal dominant matrices only a LU-decomposition exists.*/
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Input parameters:                                                */
/*   =================                                                */
/*      n        Dimension of the matrix ( > 2 )  sal_uInt16 n        */
/*      lower    lower antidiagonal               double lower[n]     */
/*      diag     main diagonal                    double diag[n]      */
/*      upper    upper antidiagonal               double upper[n]     */
/*      b        right side of the system         double b[n]         */
/*      rep      = FALSE first call               bool rep            */
/*               = TRUE  repeated call                                */
/*                    for equal matrix,                               */
/*                    but different b.                                */
/*                                                                    */
/*   Output parameters:                                               */
/*   ==================                                               */
/*      b        solution vector of the system,   double b[n]         */
/*               the original right side is overwritten               */
/*                                                                    */
/*      lower    ) contain for rep = false the solution of the matrix;*/
/*      diag     ) the original values of lower and diagonal will be  */
/*      upper    ) overwritten                                        */
/*      lowrow   )                             double lowrow[n-2]     */
/*      ricol    )                             double ricol[n-2]      */
/*                                                                    */
/*   The determinant of the matrix is for rep = false                 */
/*      det A = diag[0] * ... * diag[n-1]     defined .               */
/*                                                                    */
/*   Return value:                                                    */
/*   =============                                                    */
/*      = 0      all ok                                               */
/*      = 1      n < 3 chosen                                         */
/*      = 2      Decomposition matrix does not exist                  */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Used functions:                                                  */
/*   ===============                                                  */
/*                                                                    */
/*   from the C library: fabs()                                       */
/*                                                                    */
/*====================================================================*/

/*.cp 5 */
{
 double temp;  // fabs(double);
 sal_uInt16 i;
 short  j;

 if ( n < 3 ) return 1;

   {                                     /*  calculate decomposition  */
     lower[0] = upper[n-1] = 0.0;        /*  of the matrix.           */

     if ( fabs (diag[0]) < MACH_EPS ) return 2;
                                         /* Do not decompose if the   */
     temp = 1.0 / diag[0];               /* value of a diagonal       */
     upper[0] *= temp;                   /* element is smaller then   */
     ricol[0] *= temp;                   /* MACH_EPS                  */

     for (i = 1; i < n-2; i++)
       { diag[i] -= lower[i] * upper[i-1];
         if ( fabs(diag[i]) < MACH_EPS ) return 2;
         temp = 1.0 / diag[i];
         upper[i] *= temp;
         ricol[i] = -lower[i] * ricol[i-1] * temp;
       }

     diag[n-2] -= lower[n-2] * upper[n-3];
     if ( fabs(diag[n-2]) < MACH_EPS ) return 2;

     for (i = 1; i < n-2; i++)
       lowrow[i] = -lowrow[i-1] * upper[i-1];

     lower[n-1] -= lowrow[n-3] * upper[n-3];
     upper[n-2] = ( upper[n-2] - lower[n-2] * ricol[n-3] ) / diag[n-2];

     for (temp = 0.0, i = 0; i < n-2; i++)
       temp -= lowrow[i] * ricol[i];
     diag[n-1] += temp - lower[n-1] * upper[n-2];

     if ( fabs(diag[n-1]) < MACH_EPS ) return 2;
   }

 b[0] /= diag[0];                          /* forward elimination    */
 for (i = 1; i < n-1; i++)
   b[i] = ( b[i] - b[i-1] * lower[i] ) / diag[i];

 for (temp = 0.0, i = 0; i < n-2; i++)
   temp -= lowrow[i] * b[i];

 b[n-1] = ( b[n-1] + temp - lower[n-1] * b[n-2] ) / diag[n-1];

 b[n-2] -= b[n-1] * upper[n-2];            /* backward elimination   */
 for (j = n-3; j >= 0; j--) {
   i=j;
   b[i] -= upper[i] * b[i+1] + ricol[i] * b[n-1];
   }
 return 0;
}

/*------------------  END of CYCLIC TRIDIAGONAL  ---------------------*/

} // extern "C"

// Calculates the coefficients of natural cubic splines with n intervals.
sal_uInt16 NaturalSpline(sal_uInt16 n, double* x, double* y,
                     double Marg0, double MargN,
                     sal_uInt8 MargCond,
                     double* b, double* c, double* d)
{
    sal_uInt16  i;
    std::unique_ptr<double[]> a;
    std::unique_ptr<double[]> h;
    sal_uInt16  error;

    if (n<2) return 1;
    if ( (MargCond & ~3) ) return 2;
    a.reset(new double[n+1]);
    h.reset(new double[n+1]);
    for (i=0;i<n;i++) {
        h[i]=x[i+1]-x[i];
        if (h[i]<=0.0) return 1;
    }
    for (i=0;i<n-1;i++) {
        a[i]=3.0*((y[i+2]-y[i+1])/h[i+1]-(y[i+1]-y[i])/h[i]);
        b[i]=h[i];
        c[i]=h[i+1];
        d[i]=2.0*(h[i]+h[i+1]);
    }
    switch (MargCond) {
        case 0: {
            if (n==2) {
                a[0]=a[0]/3.0;
                d[0]=d[0]*0.5;
            } else {
                a[0]  =a[0]*h[1]/(h[0]+h[1]);
                a[n-2]=a[n-2]*h[n-2]/(h[n-1]+h[n-2]);
                d[0]  =d[0]-h[0];
                d[n-2]=d[n-2]-h[n-1];
                c[0]  =c[0]-h[0];
                b[n-2]=b[n-2]-h[n-1];
            }
            SAL_FALLTHROUGH;
        }
        case 1: {
            a[0]  =a[0]-1.5*((y[1]-y[0])/h[0]-Marg0);
            a[n-2]=a[n-2]-1.5*(MargN-(y[n]-y[n-1])/h[n-1]);
            d[0]  =d[0]-h[0]*0.5;
            d[n-2]=d[n-2]-h[n-1]*0.5;
            SAL_FALLTHROUGH;
        }
        case 2: {
            a[0]  =a[0]-h[0]*Marg0*0.5;
            a[n-2]=a[n-2]-h[n-1]*MargN*0.5;
            SAL_FALLTHROUGH;
        }
        case 3: {
            a[0]  =a[0]+Marg0*h[0]*h[0]*0.5;
            a[n-2]=a[n-2]-MargN*h[n-1]*h[n-1]*0.5;
            d[0]  =d[0]+h[0];
            d[n-2]=d[n-2]+h[n-1];
        }
    } // switch MargCond
    if (n==2) {
        c[1]=a[0]/d[0];
    } else {
        error=TriDiagGS(n-1,b,d,c,a.get());
        if (error!=0) return error+2;
        for (i=0;i<n-1;i++) c[i+1]=a[i];
    }
    switch (MargCond) {
        case 0: {
            if (n==2) {
                c[2]=c[1];
                c[0]=c[1];
            } else {
                c[0]=c[1]+h[0]*(c[1]-c[2])/h[1];
                c[n]=c[n-1]+h[n-1]*(c[n-1]-c[n-2])/h[n-2];
            }
            SAL_FALLTHROUGH;
        }
        case 1: {
            c[0]=1.5*((y[1]-y[0])/h[0]-Marg0);
            c[0]=(c[0]-c[1]*h[0]*0.5)/h[0];
            c[n]=1.5*((y[n]-y[n-1])/h[n-1]-MargN);
            c[n]=(c[n]-c[n-1]*h[n-1]*0.5)/h[n-1];
            SAL_FALLTHROUGH;
        }
        case 2: {
            c[0]=Marg0*0.5;
            c[n]=MargN*0.5;
            SAL_FALLTHROUGH;
        }
        case 3: {
            c[0]=c[1]-Marg0*h[0]*0.5;
            c[n]=c[n-1]+MargN*h[n-1]*0.5;
        }
    } // switch MargCond
    for (i=0;i<n;i++) {
        b[i]=(y[i+1]-y[i])/h[i]-h[i]*(c[i+1]+2.0*c[i])/3.0;
        d[i]=(c[i+1]-c[i])/(3.0*h[i]);
    }
    return 0;
}

// calculates the coefficients of periodical cubic splines with n intervals.
sal_uInt16 PeriodicSpline(sal_uInt16 n, double* x, double* y,
                      double* b, double* c, double* d)
{                     // array dimensions should range from [0..n]!
    sal_uInt16  Error;
    sal_uInt16  i,im1,nm1; //integer
    double  hl;
    std::unique_ptr<double[]> a;
    std::unique_ptr<double[]> lowrow;
    std::unique_ptr<double[]> ricol;

    if (n<2) return 4;
    nm1=n-1;
    for (i=0;i<=nm1;i++) if (x[i+1]<=x[i]) return 2; // should be strictly monotonically decreasing!
    if (!rtl::math::approxEqual(y[n],y[0])) return 3; // begin and end should be equal!

    a.reset(new double[n+1]);
    lowrow.reset(new double[n+1]);
    ricol.reset(new double[n+1]);

    if (n==2) {
        c[1]=3.0*((y[2]-y[1])/(x[2]-x[1]));
        c[1]=c[1]-3.0*((y[i]-y[0])/(x[1]-x[0]));
        c[1]=c[1]/(x[2]-x[0]);
        c[2]=-c[1];
    } else {
        double hr;
        for (i=1;i<=nm1;i++) {
            im1=i-1;
            hl=x[i]-x[im1];
            hr=x[i+1]-x[i];
            b[im1]=hl;
            d[im1]=2.0*(hl+hr);
            c[im1]=hr;
            a[im1]=3.0*((y[i+1]-y[i])/hr-(y[i]-y[im1])/hl);
        }
        hl=x[n]-x[nm1];
        hr=x[1]-x[0];
        b[nm1]=hl;
        d[nm1]=2.0*(hl+hr);
        lowrow[0]=hr;
        ricol[0]=hr;
        a[nm1]=3.0*((y[1]-y[0])/hr-(y[n]-y[nm1])/hl);
        Error=ZyklTriDiagGS(n,b,d,c,lowrow.get(),ricol.get(),a.get());
        if ( Error != 0 )
        {
            return Error+4;
        }
        for (i=0;i<=nm1;i++) c[i+1]=a[i];
    }
    c[0]=c[n];
    for (i=0;i<=nm1;i++) {
        hl=x[i+1]-x[i];
        b[i]=(y[i+1]-y[i])/hl;
        b[i]=b[i]-hl*(c[i+1]+2.0*c[i])/3.0;
        d[i]=(c[i+1]-c[i])/hl/3.0;
    }
    return 0;
}

// calculate the coefficients of parametric natural of periodical cubic splines
// with n intervals
sal_uInt16 ParaSpline(sal_uInt16 n, double* x, double* y, sal_uInt8 MargCond,
                  double Marg01, double Marg02,
                  double MargN1, double MargN2,
                  bool CondT, double* T,
                  double* bx, double* cx, double* dx,
                  double* by, double* cy, double* dy)
{
    sal_uInt16 Error;
    sal_uInt16 i;
    double alphX = 0,alphY = 0,
           betX = 0,betY = 0;

    if (n<2) return 1;
    if ((MargCond & ~3) && (MargCond != 4)) return 2; // invalid boundary condition
    if (!CondT) {
        T[0]=0.0;
        for (i=0;i<n;i++) {
            double deltX,deltY,delt;
            deltX=x[i+1]-x[i]; deltY=y[i+1]-y[i];
            delt =deltX*deltX+deltY*deltY;
            if (delt<=0.0) return 3;            // two identical adjacent points!
            T[i+1]=T[i]+sqrt(delt);
        }
    }
    switch (MargCond) {
        case 0: break;
        case 1: case 2: {
            alphX=Marg01; betX=MargN1;
            alphY=Marg02; betY=MargN2;
        } break;
        case 3: {
            if (!rtl::math::approxEqual(x[n],x[0])) return 3;
            if (!rtl::math::approxEqual(y[n],y[0])) return 4;
        } break;
        case 4: {
            if (std::abs(Marg01)>=MAXROOT) {
                alphX=0.0;
                alphY=std::copysign(1.0,y[1]-y[0]);
            } else {
                alphX=std::copysign(sqrt(1.0/(1.0+Marg01*Marg01)),x[1]-x[0]);
                alphY=alphX*Marg01;
            }
            if (std::abs(MargN1)>=MAXROOT) {
                betX=0.0;
                betY=std::copysign(1.0,y[n]-y[n-1]);
            } else {
                betX=std::copysign(sqrt(1.0/(1.0+MargN1*MargN1)),x[n]-x[n-1]);
                betY=betX*MargN1;
            }
        }
    } // switch MargCond
    if (MargCond==3) {
        Error=PeriodicSpline(n,T,x,bx,cx,dx);
        if (Error!=0) return Error+4;
        Error=PeriodicSpline(n,T,y,by,cy,dy);
        if (Error!=0) return Error+10;
    } else {
        Error=NaturalSpline(n,T,x,alphX,betX,MargCond,bx,cx,dx);
        if (Error!=0) return Error+4;
        Error=NaturalSpline(n,T,y,alphY,betY,MargCond,by,cy,dy);
        if (Error!=0) return Error+9;
    }
    return 0;
}

bool CalcSpline(tools::Polygon& rPoly, bool Periodic, sal_uInt16& n,
                double*& ax, double*& ay, double*& bx, double*& by,
                double*& cx, double*& cy, double*& dx, double*& dy, double*& T)
{
    sal_uInt8   Marg;
    double Marg01;
    double MargN1,MargN2;
    sal_uInt16 i;
    Point  P0(-32768,-32768);
    Point  Pt;

    n=rPoly.GetSize();
    ax=new double[rPoly.GetSize()+2];
    ay=new double[rPoly.GetSize()+2];

    n=0;
    for (i=0;i<rPoly.GetSize();i++) {
        Pt=rPoly.GetPoint(i);
        if (i==0 || Pt!=P0) {
            ax[n]=Pt.X();
            ay[n]=Pt.Y();
            n++;
            P0=Pt;
        }
    }

    if (Periodic) {
        Marg=3;
        ax[n]=ax[0];
        ay[n]=ay[0];
        n++;
    } else {
        Marg=2;
    }

    bx=new double[n+1];
    by=new double[n+1];
    cx=new double[n+1];
    cy=new double[n+1];
    dx=new double[n+1];
    dy=new double[n+1];
    T =new double[n+1];

    Marg01=0.0;
    MargN1=0.0;
    MargN2=0.0;
    if (n>0) n--; // correct n (number of partial polynoms)

    bool bRet = false;
    if ( ( Marg == 3 && n >= 3 ) || ( Marg == 2 && n >= 2 ) )
    {
        bRet = ParaSpline(n,ax,ay,Marg,Marg01,Marg01,MargN1,MargN2,false,T,bx,cx,dx,by,cy,dy) == 0;
    }
    if ( !bRet )
    {
        delete[] ax;
        delete[] ay;
        delete[] bx;
        delete[] by;
        delete[] cx;
        delete[] cy;
        delete[] dx;
        delete[] dy;
        delete[] T;
        n=0;
    }
    return bRet;
}

bool Spline2Poly(tools::Polygon& rSpln, bool Periodic, tools::Polygon& rPoly)
{
    const short MinKoord = -32000;    // to prevent
    const short MaxKoord = 32000;     // overflows

    double* ax;                // coefficients of the polynoms
    double* ay;
    double* bx;
    double* by;
    double* cx;
    double* cy;
    double* dx;
    double* dy;
    double* tv;

    sal_uInt16  n;             // number of partial polynoms to draw
    sal_uInt16  i;             // actual partial polynom
    bool        bOk;           // all still ok?
    const sal_uInt16  PolyMax=16380; // max number of polygon points

    bOk=CalcSpline(rSpln,Periodic,n,ax,ay,bx,by,cx,cy,dx,dy,tv);
    if (bOk) {
        const double Step = 10;          // stepsize for t

        rPoly.SetSize(1);
        rPoly.SetPoint(Point(short(ax[0]),short(ay[0])),0); // first point
        i=0;
        while (i<n) {       // draw n partial polynoms
            double t=tv[i]+Step;
            bool bEnd=false; // partial polynom ended?
            while (!bEnd) {  // extrapolate one partial polynom
                double      dt1,dt2,dt3;   // delta t, y, ^3
                bEnd=t>=tv[i+1];
                if (bEnd) t=tv[i+1];
                dt1=t-tv[i]; dt2=dt1*dt1; dt3=dt2*dt1;
                long x=long(ax[i]+bx[i]*dt1+cx[i]*dt2+dx[i]*dt3);
                long y=long(ay[i]+by[i]*dt1+cy[i]*dt2+dy[i]*dt3);
                if (x<MinKoord) x=MinKoord;
                if (x>MaxKoord) x=MaxKoord;
                if (y<MinKoord) y=MinKoord;
                if (y>MaxKoord) y=MaxKoord;
                if (rPoly.GetSize()<PolyMax) {
                    rPoly.SetSize(rPoly.GetSize()+1);
                    rPoly.SetPoint(Point(short(x),short(y)),rPoly.GetSize()-1);
                } else {
                    bOk=false; // error: polygon becomes to large
                }
                t=t+Step;
            } // end of partial polynom
            i++; // next partial polynom
        }
        delete[] ax;
        delete[] ay;
        delete[] bx;
        delete[] by;
        delete[] cx;
        delete[] cy;
        delete[] dx;
        delete[] dy;
        delete[] tv;
        return bOk;
    } // end of if (bOk)
    rPoly.SetSize(0);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
