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


#include <tools/poly.hxx>

extern "C" {

/*.pn 277 */
/*.hlAnhang: C - Programme*/
/*.hrKonstanten- und Macro-Definitionen*/
/*.fe Die Include-Datei u_const.h ist in das Verzeichnis zu stellen,  */
/*.fe wo der Compiler nach Include-Dateien sucht.                     */


/*-----------------------  FILE u_const.h  ---------------------------*/

#define IEEE

/* IEEE - Norm fuer die Darstellung von Gleitkommazahlen:

      8 Byte lange Gleitkommazahlen, mit

     53 Bit Mantisse  ==> Mantissenbereich:    2 hoch 52 versch. Zahlen
                                               mit 0.1 <= Zahl < 1.0,
                                               1 Vorzeichen-Bit
     11 Bit Exponent  ==> Exponentenbereich:  -1024...+1023

   Die 1. Zeile ( #define IEEE ) ist zu loeschen, falls die Maschine
   bzw. der Compiler keine Gleitpunktzahlen gemaess der IEEE-Norm
   benutzt. Zusaetzlich muessen die Zahlen  MAXEXPON, MINEXPON
   (s.u.) angepasst werden.
   */

#ifdef IEEE         /*----------- Falls IEEE Norm --------------------*/

#define MACH_EPS  2.220446049250313e-016   /* Maschinengenauigkeit    */
                                           /* IBM-AT:  = 2 hoch -52   */
/* MACH_EPS ist die kleinste positive, auf der Maschine darstellbare
   Zahl x, die der Bedingung genuegt: 1.0 + x > 1.0                   */

#define MAXROOT   9.48075190810918e+153

#else               /*------------------ sonst -----------------------*/

double exp  (double);
double atan (double);
double pow  (double,double);
double sqrt (double);

double masch()            /* MACH_EPS maschinenunabhaengig bestimmen  */
{
  double eps = 1.0, x = 2.0, y = 1.0;
  while ( y < x )
    { eps *= 0.5;
      x = 1.0 + eps;
    }
  eps *= 2.0; return (eps);
}

short basis()             /* BASIS maschinenunabhaengig bestimmen     */
{
  double x = 1.0, one = 1.0, b = 1.0;

  while ( (x + one) - x == one ) x *= 2.0;
  while ( (x + b) == x ) b *= 2.0;

  return ( (short) ((x + b) - x) );
}

#define BASIS     basis()                  /* Basis der Zahlendarst.  */

/* Falls die Maschine (der Compiler) keine IEEE-Darstellung fuer
   Gleitkommazahlen nutzt, muessen die folgenden 2 Konstanten an-
   gepasst werden.
   */

#define MAXEXPON  1023.0                   /* groesster Exponent      */
#define MINEXPON -1024.0                   /* kleinster Exponent      */


#define MACH_EPS  masch()

#define POSMAX    pow ((double) BASIS, MAXEXPON)
#define MAXROOT   sqrt(POSMAX)

#endif              /*-------------- ENDE ifdef ----------------------*/

/* Definition von Funktionsmakros:
   */

#define abs(X) ((X) >= 0  ?  (X) : -(X))       /* Absolutbetrag von X */
#define sign(X, Y) (Y < 0 ? -abs(X) : abs(X))  /* Vorzeichen von      */
                                               /* Y mal abs(X)        */

/*-------------------  ENDE FILE u_const.h  --------------------------*/









/*.HL Anhang: C - Programme*/
/*.HRGleichungssysteme fuer Tridiagonalmatrizen*/

/*.FE  P 3.7 TRIDIAGONALE GLEICHUNGSSYSTEME*/


/*----------------------   MODUL TRIDIAGONAL  ------------------------*/

sal_uInt16 TriDiagGS(sal_Bool rep, sal_uInt16 n, double* lower,
                 double* diag, double* upper, double* b)
                                              /************************/
                                              /* GAUSS-Verfahren fuer */
                                              /* Tridiagonalmatrizen  */
                                              /************************/

/*====================================================================*/
/*                                                                    */
/*  trdiag bestimmt die Loesung x des linearen Gleichungssystems      */
/*  A * x = b mit tridiagonaler n x n Koeffizientenmatrix A, die in   */
/*  den 3 Vektoren lower, upper und diag wie folgt abgespeichert ist: */
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
/*   Anwendung:                                                       */
/*   =========                                                        */
/*      Vorwiegend fuer diagonaldominante Tridiagonalmatrizen, wie    */
/*      sie bei der Spline-Interpolation auftreten.                   */
/*      Fuer diagonaldominante Matrizen existiert immer eine LU-      */
/*      Zerlegung; fuer nicht diagonaldominante Tridiagonalmatrizen   */
/*      sollte die Funktion band vorgezogen werden, da diese mit      */
/*      Spaltenpivotsuche arbeitet und daher numerisch stabiler ist.  */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Eingabeparameter:                                                */
/*   ================                                                 */
/*      n        Dimension der Matrix ( > 1 )  sal_uInt16 n               */
/*                                                                    */
/*      lower    untere Nebendiagonale         double lower[n]        */
/*      diag     Hauptdiagonale                double diag[n]         */
/*      upper    obere Nebendiagonale          double upper[n]        */
/*                                                                    */
/*               bei rep != 0 enthalten lower, diag und upper die     */
/*               Dreieckzerlegung der Ausgangsmatrix.                 */
/*                                                                    */
/*      b        rechte Seite des Systems      double b[n]            */
/*      rep      = 0  erstmaliger Aufruf       sal_Bool rep               */
/*               !=0  wiederholter Aufruf                             */
/*                    fuer gleiche Matrix,                            */
/*                    aber verschiedenes b.                           */
/*                                                                    */
/*   Ausgabeparameter:                                                */
/*   ================                                                 */
/*      b        Loesungsvektor des Systems;   double b[n]            */
/*               die urspruengliche rechte Seite wird ueberspeichert  */
/*                                                                    */
/*      lower    ) enthalten bei rep = 0 die Zerlegung der Matrix;    */
/*      diag     ) die urspruenglichen Werte von lower u. diag werden */
/*      upper    ) ueberschrieben                                     */
/*                                                                    */
/*   Die Determinante der Matrix ist bei rep = 0 durch                */
/*      det A = diag[0] * ... * diag[n-1] bestimmt.                   */
/*                                                                    */
/*   Rueckgabewert:                                                   */
/*   =============                                                    */
/*      = 0      alles ok                                             */
/*      = 1      n < 2 gewaehlt                                       */
/*      = 2      Die Dreieckzerlegung der Matrix existiert nicht      */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Benutzte Funktionen:                                             */
/*   ===================                                              */
/*                                                                    */
/*   Aus der C Bibliothek: fabs()                                     */
/*                                                                    */
/*====================================================================*/

/*.cp 5 */
{
 sal_uInt16 i;
 short  j;

// double fabs(double);

 if ( n < 2 ) return(1);                    /*  n mindestens 2        */

                                            /*  Wenn rep = 0 ist,     */
                                            /*  Dreieckzerlegung der  */
 if (rep == 0)                              /*  Matrix u. det be-     */
   {                                        /*  stimmen               */
     for (i = 1; i < n; i++)
       { if ( fabs(diag[i-1]) < MACH_EPS )  /*  Wenn ein diag[i] = 0  */
           return(2);                       /*  ist, ex. keine Zerle- */
         lower[i] /= diag[i-1];             /*  gung.                 */
         diag[i] -= lower[i] * upper[i-1];
       }
    }

 if ( fabs(diag[n-1]) < MACH_EPS ) return(2);

 for (i = 1; i < n; i++)                   /*  Vorwaertselimination  */
    b[i] -= lower[i] * b[i-1];

 b[n-1] /= diag[n-1];                      /* Rueckwaertselimination */
 for (j = n-2; j >= 0; j--) {
    i=j;
    b[i] = ( b[i] - upper[i] * b[i+1] ) / diag[i];
 }
 return(0);
}

/*-----------------------  ENDE TRIDIAGONAL  -------------------------*/









/*.HL Anhang: C - Programme*/
/*.HRGleichungssysteme mit zyklisch tridiagonalen Matrizen*/

/*.FE  P 3.8  SYSTEME MIT ZYKLISCHEN TRIDIAGONALMATRIZEN */


/*----------------  MODUL ZYKLISCH TRIDIAGONAL  ----------------------*/


sal_uInt16 ZyklTriDiagGS(sal_Bool rep, sal_uInt16 n, double* lower, double* diag,
                     double* upper, double* lowrow, double* ricol, double* b)
                                        /******************************/
                                        /* Systeme mit zyklisch tri-  */
                                        /* diagonalen Matrizen        */
                                        /******************************/

/*====================================================================*/
/*                                                                    */
/*  tzdiag bestimmt die Loesung x des linearen Gleichungssystems      */
/*  A * x = b mit zyklisch tridiagonaler n x n Koeffizienten-         */
/*  matrix A, die in den 5 Vektoren lower, upper, diag, lowrow und    */
/*  ricol wie folgt abgespeichert ist:                                */
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
/*  Speicherplatz fuer lowrow[1],..,lowrow[n-3] und ricol[1],...,     */
/*  ricol[n-3] muss zusaetzlich bereitgestellt werden, da dieser      */
/*  fuer die Aufnahme der Zerlegungsmatrix verfuegbar sein muss, die  */
/*  auf die 5 genannten Vektoren ueberspeichert wird.                 */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Anwendung:                                                       */
/*   =========                                                        */
/*      Vorwiegend fuer diagonaldominante zyklische Tridiagonalmatri- */
/*      zen wie sie bei der Spline-Interpolation auftreten.           */
/*      Fuer diagonaldominante Matrizen existiert immer eine LU-      */
/*      Zerlegung.                                                    */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Eingabeparameter:                                                */
/*   ================                                                 */
/*      n        Dimension der Matrix ( > 2 )  sal_uInt16 n               */
/*      lower    untere Nebendiagonale         double lower[n]        */
/*      diag     Hauptdiagonale                double diag[n]         */
/*      upper    obere Nebendiagonale          double upper[n]        */
/*      b        rechte Seite des Systems      double b[n]            */
/*      rep      = 0  erstmaliger Aufruf       sal_Bool rep               */
/*               !=0  wiederholter Aufruf                             */
/*                    fuer gleiche Matrix,                            */
/*                    aber verschiedenes b.                           */
/*                                                                    */
/*   Ausgabeparameter:                                                */
/*   ================                                                 */
/*      b        Loesungsvektor des Systems,   double b[n]            */
/*               die urspruengliche rechte Seite wird ueberspeichert  */
/*                                                                    */
/*      lower    ) enthalten bei rep = 0 die Zerlegung der Matrix;    */
/*      diag     ) die urspruenglichen Werte von lower u. diag werden */
/*      upper    ) ueberschrieben                                     */
/*      lowrow   )                             double lowrow[n-2]     */
/*      ricol    )                             double ricol[n-2]      */
/*                                                                    */
/*   Die Determinante der Matrix ist bei rep = 0 durch                */
/*      det A = diag[0] * ... * diag[n-1]     bestimmt.               */
/*                                                                    */
/*   Rueckgabewert:                                                   */
/*   =============                                                    */
/*      = 0      alles ok                                             */
/*      = 1      n < 3 gewaehlt                                       */
/*      = 2      Die Zerlegungsmatrix existiert nicht                 */
/*                                                                    */
/*====================================================================*/
/*                                                                    */
/*   Benutzte Funktionen:                                             */
/*   ===================                                              */
/*                                                                    */
/*   Aus der C Bibliothek: fabs()                                     */
/*                                                                    */
/*====================================================================*/

/*.cp 5 */
{
 double temp;  // fabs(double);
 sal_uInt16 i;
 short  j;

 if ( n < 3 ) return(1);

 if (rep == 0)                              /*  Wenn rep = 0 ist,     */
   {                                        /*  Zerlegung der         */
     lower[0] = upper[n-1] = 0.0;           /*  Matrix berechnen.     */

     if ( fabs (diag[0]) < MACH_EPS ) return(2);
                                          /* Ist ein Diagonalelement  */
     temp = 1.0 / diag[0];                /* betragsmaessig kleiner   */
     upper[0] *= temp;                    /* MACH_EPS, so ex. keine   */
     ricol[0] *= temp;                    /* Zerlegung.               */

     for (i = 1; i < n-2; i++)
       { diag[i] -= lower[i] * upper[i-1];
         if ( fabs(diag[i]) < MACH_EPS ) return(2);
         temp = 1.0 / diag[i];
         upper[i] *= temp;
         ricol[i] = -lower[i] * ricol[i-1] * temp;
       }

     diag[n-2] -= lower[n-2] * upper[n-3];
     if ( fabs(diag[n-2]) < MACH_EPS ) return(2);

     for (i = 1; i < n-2; i++)
       lowrow[i] = -lowrow[i-1] * upper[i-1];

     lower[n-1] -= lowrow[n-3] * upper[n-3];
     upper[n-2] = ( upper[n-2] - lower[n-2] * ricol[n-3] ) / diag[n-2];

     for (temp = 0.0, i = 0; i < n-2; i++)
       temp -= lowrow[i] * ricol[i];
     diag[n-1] += temp - lower[n-1] * upper[n-2];

     if ( fabs(diag[n-1]) < MACH_EPS ) return(2);
   }  /* end if ( rep == 0 ) */

 b[0] /= diag[0];                          /* Vorwaertselemination    */
 for (i = 1; i < n-1; i++)
   b[i] = ( b[i] - b[i-1] * lower[i] ) / diag[i];

 for (temp = 0.0, i = 0; i < n-2; i++)
   temp -= lowrow[i] * b[i];

 b[n-1] = ( b[n-1] + temp - lower[n-1] * b[n-2] ) / diag[n-1];

 b[n-2] -= b[n-1] * upper[n-2];            /* Rueckwaertselimination  */
 for (j = n-3; j >= 0; j--) {
   i=j;
   b[i] -= upper[i] * b[i+1] + ricol[i] * b[n-1];
   }
 return(0);
}

/*------------------  ENDE ZYKLISCH TRIDIAGONAL  ---------------------*/


} // extern "C"


/*************************************************************************
|*
|*    NaturalSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines natuerlichen
|*                      kubischen Polynomsplines mit n Stuetzstellen.
|*
*************************************************************************/

sal_uInt16 NaturalSpline(sal_uInt16 n, double* x, double* y,
                     double Marg0, double MargN,
                     sal_uInt8 MargCond,
                     double* b, double* c, double* d)
{
    sal_uInt16  i;
    double* a;
    double* h;
    sal_uInt16  error;

    if (n<2) return 1;
    if ( (MargCond & ~3) ) return 2;
    a=new double[n+1];
    h=new double[n+1];
    for (i=0;i<n;i++) {
        h[i]=x[i+1]-x[i];
        if (h[i]<=0.0) { delete[] a; delete[] h; return 1; }
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
        }
        case 1: {
            a[0]  =a[0]-1.5*((y[1]-y[0])/h[0]-Marg0);
            a[n-2]=a[n-2]-1.5*(MargN-(y[n]-y[n-1])/h[n-1]);
            d[0]  =d[0]-h[0]*0.5;
            d[n-2]=d[n-2]-h[n-1]*0.5;
        }
        case 2: {
            a[0]  =a[0]-h[0]*Marg0*0.5;
            a[n-2]=a[n-2]-h[n-1]*MargN*0.5;
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
        error=TriDiagGS(sal_False,n-1,b,d,c,a);
        if (error!=0) { delete[] a; delete[] h; return error+2; }
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
        }
        case 1: {
            c[0]=1.5*((y[1]-y[0])/h[0]-Marg0);
            c[0]=(c[0]-c[1]*h[0]*0.5)/h[0];
            c[n]=1.5*((y[n]-y[n-1])/h[n-1]-MargN);
            c[n]=(c[n]-c[n-1]*h[n-1]*0.5)/h[n-1];
        }
        case 2: {
            c[0]=Marg0*0.5;
            c[n]=MargN*0.5;
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
    delete[] a;
    delete[] h;
    return 0;
}


/*************************************************************************
|*
|*    PeriodicSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines periodischen
|*                      kubischen Polynomsplines mit n Stuetzstellen.
|*
*************************************************************************/


sal_uInt16 PeriodicSpline(sal_uInt16 n, double* x, double* y,
                      double* b, double* c, double* d)
{                     // Arrays muessen von [0..n] dimensioniert sein!
    sal_uInt16  Error;
    sal_uInt16  i,im1,nm1; //integer
    double  hr,hl;
    double* a;
    double* lowrow;
    double* ricol;

    if (n<2) return 4;
    nm1=n-1;
    for (i=0;i<=nm1;i++) if (x[i+1]<=x[i]) return 2; // muss streng nonoton fallend sein!
    if (y[n]!=y[0]) return 3; // Anfang muss gleich Ende sein!

    a     =new double[n+1];
    lowrow=new double[n+1];
    ricol =new double[n+1];

    if (n==2) {
        c[1]=3.0*((y[2]-y[1])/(x[2]-x[1]));
        c[1]=c[1]-3.0*((y[i]-y[0])/(x[1]-x[0]));
        c[1]=c[1]/(x[2]-x[0]);
        c[2]=-c[1];
    } else {
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
        Error=ZyklTriDiagGS(sal_False,n,b,d,c,lowrow,ricol,a);
        if ( Error != 0 )
        {
            delete[] a;
            delete[] lowrow;
            delete[] ricol;
            return(Error+4);
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
    delete[] a;
    delete[] lowrow;
    delete[] ricol;
    return 0;
}



/*************************************************************************
|*
|*    ParaSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines parametrischen
|*                      natuerlichen oder periodischen kubischen
|*                      Polynomsplines mit n Stuetzstellen.
|*
*************************************************************************/

sal_uInt16 ParaSpline(sal_uInt16 n, double* x, double* y, sal_uInt8 MargCond,
                  double Marg01, double Marg02,
                  double MargN1, double MargN2,
                  sal_Bool CondT, double* T,
                  double* bx, double* cx, double* dx,
                  double* by, double* cy, double* dy)
{
    sal_uInt16 Error;
    sal_uInt16 i;
    double deltX,deltY,delt,
           alphX = 0,alphY = 0,
           betX = 0,betY = 0;

    if (n<2) return 1;
    if ((MargCond & ~3) && (MargCond != 4)) return 2; // ungueltige Randbedingung
    if (CondT==sal_False) {
        T[0]=0.0;
        for (i=0;i<n;i++) {
            deltX=x[i+1]-x[i]; deltY=y[i+1]-y[i];
            delt =deltX*deltX+deltY*deltY;
            if (delt<=0.0) return 3;            // zwei identische Punkte nacheinander!
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
            if (x[n]!=x[0]) return 3;
            if (y[n]!=y[0]) return 4;
        } break;
        case 4: {
            if (abs(Marg01)>=MAXROOT) {
                alphX=0.0;
                alphY=sign(1.0,y[1]-y[0]);
            } else {
                alphX=sign(sqrt(1.0/(1.0+Marg01*Marg01)),x[1]-x[0]);
                alphY=alphX*Marg01;
            }
            if (abs(MargN1)>=MAXROOT) {
                betX=0.0;
                betY=sign(1.0,y[n]-y[n-1]);
            } else {
                betX=sign(sqrt(1.0/(1.0+MargN1*MargN1)),x[n]-x[n-1]);
                betY=betX*MargN1;
            }
        }
    } // switch MargCond
    if (MargCond==3) {
        Error=PeriodicSpline(n,T,x,bx,cx,dx);
        if (Error!=0) return(Error+4);
        Error=PeriodicSpline(n,T,y,by,cy,dy);
        if (Error!=0) return(Error+10);
    } else {
        Error=NaturalSpline(n,T,x,alphX,betX,MargCond,bx,cx,dx);
        if (Error!=0) return(Error+4);
        Error=NaturalSpline(n,T,y,alphY,betY,MargCond,by,cy,dy);
        if (Error!=0) return(Error+9);
    }
    return 0;
}



/*************************************************************************
|*
|*    CalcSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines parametrischen
|*                      natuerlichen oder periodischen kubischen
|*                      Polynomsplines. Die Eckpunkte des uebergebenen
|*                      Polygons werden als Stuetzstellen angenommen.
|*                      n liefert die Anzahl der Teilpolynome.
|*                      Ist die Berechnung fehlerfrei verlaufen, so
|*                      liefert die Funktion sal_True. Nur in diesem Fall
|*                      ist Speicher fuer die Koeffizientenarrays
|*                      allokiert, der dann spaeter vom Aufrufer mittels
|*                      delete freizugeben ist.
|*
*************************************************************************/

sal_Bool CalcSpline(Polygon& rPoly, sal_Bool Periodic, sal_uInt16& n,
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
    if (n>0) n--; // n Korregieren (Anzahl der Teilpolynome)

    sal_Bool bRet = sal_False;
    if ( ( Marg == 3 && n >= 3 ) || ( Marg == 2 && n >= 2 ) )
    {
        bRet = ParaSpline(n,ax,ay,Marg,Marg01,Marg01,MargN1,MargN2,sal_False,T,bx,cx,dx,by,cy,dy) == 0;
    }
    if ( bRet == sal_False )
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


/*************************************************************************
|*
|*    Spline2Poly()
|*
|*    Beschreibung      Konvertiert einen parametrichen kubischen
|*                      Polynomspline Spline (natuerlich oder periodisch)
|*                      in ein angenaehertes Polygon.
|*                      Die Funktion liefert sal_False, wenn ein Fehler bei
|*                      der Koeffizientenberechnung aufgetreten ist oder
|*                      das Polygon zu gross wird (>PolyMax=16380). Im 1.
|*                      Fall hat das Polygon 0, im 2. Fall PolyMax Punkte.
|*                      Um Koordinatenueberlaeufe zu vermeiden werden diese
|*                      auf +/-32000 begrenzt.
|*
*************************************************************************/
sal_Bool Spline2Poly(Polygon& rSpln, sal_Bool Periodic, Polygon& rPoly)
{
    short  MinKoord=-32000; // zur Vermeidung
    short  MaxKoord=32000;  // von Ueberlaeufen

    double* ax;          // Koeffizienten der Polynome
    double* ay;
    double* bx;
    double* by;
    double* cx;
    double* cy;
    double* dx;
    double* dy;
    double* tv;

    double  Step;        // Schrittweite fuer t
    double  dt1,dt2,dt3; // Delta t, y, ^3
    double  t;
    sal_Bool    bEnde;       // Teilpolynom zu Ende?
    sal_uInt16  n;           // Anzahl der zu zeichnenden Teilpolynome
    sal_uInt16  i;           // aktuelles Teilpolynom
    sal_Bool    bOk;         // noch alles ok?
    sal_uInt16  PolyMax=16380;// Maximale Anzahl von Polygonpunkten
    long    x,y;

    bOk=CalcSpline(rSpln,Periodic,n,ax,ay,bx,by,cx,cy,dx,dy,tv);
    if (bOk) {
        Step =10;

        rPoly.SetSize(1);
        rPoly.SetPoint(Point(short(ax[0]),short(ay[0])),0); // erster Punkt
        i=0;
        while (i<n) {       // n Teilpolynome malen
            t=tv[i]+Step;
            bEnde=sal_False;
            while (!bEnde) {  // ein Teilpolynom interpolieren
                bEnde=t>=tv[i+1];
                if (bEnde) t=tv[i+1];
                dt1=t-tv[i]; dt2=dt1*dt1; dt3=dt2*dt1;
                x=long(ax[i]+bx[i]*dt1+cx[i]*dt2+dx[i]*dt3);
                y=long(ay[i]+by[i]*dt1+cy[i]*dt2+dy[i]*dt3);
                if (x<MinKoord) x=MinKoord; if (x>MaxKoord) x=MaxKoord;
                if (y<MinKoord) y=MinKoord; if (y>MaxKoord) y=MaxKoord;
                if (rPoly.GetSize()<PolyMax) {
                    rPoly.SetSize(rPoly.GetSize()+1);
                    rPoly.SetPoint(Point(short(x),short(y)),rPoly.GetSize()-1);
                } else {
                    bOk=sal_False; // Fehler: Polygon wird zu gross
                }
                t=t+Step;
            } // Ende von Teilpolynom
            i++; // naechstes Teilpolynom
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
    } // Ende von if (bOk)
    rPoly.SetSize(0);
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
