/*************************************************************************
 *
 *  $RCSfile: sgvspln.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef MAC
#include <mac_start.h>
#include <math.h>
#include <mac_end.h>
#else
#include <math.h>
#endif


#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "SVTOOLS_FILTER2", "SVTOOLS_CODE" )
#pragma optimize( "", off )
#endif

#if defined( PM2 ) && defined( __BORLANDC__ )
#pragma option -Od
#endif

extern "C" {

/*.pn 277 */
/*.hlAnhang: C - Programme*/
/*.hrKonstanten- und Macro-Definitionen*/
/*.fe Die Include-Datei u_const.h ist in das Verzeichnis zu stellen,  */
/*.fe wo der Compiler nach Include-Dateien sucht.                     */


/*-----------------------  FILE u_const.h  ---------------------------*/

#ifndef S390
#define IEEE
#endif

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

#define EPSQUAD   4.930380657631324e-032
#define EPSROOT   1.490116119384766e-008

#define POSMAX    8.98846567431158e+307    /* groesste positive Zahl  */
#define POSMIN    5.56268464626800e-309    /* kleinste positive Zahl  */
#define MAXROOT   9.48075190810918e+153

#define BASIS     2                        /* Basis der Zahlendarst.  */
#ifndef PI
#define PI        3.141592653589793e+000
#endif
#define EXP_1     2.718281828459045e+000

#else               /*------------------ sonst -----------------------*/

#ifndef S390
double exp  (double);
double atan (double);
double pow  (double,double);
double sqrt (double);
#endif

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

#ifndef S390
#define MAXEXPON  1023.0                   /* groesster Exponent      */
#define MINEXPON -1024.0                   /* kleinster Exponent      */
#else
#include <float.h>
#define MAXEXPON  75.0
#define MINEXPON  -79.0
#endif


#define MACH_EPS  masch()
#define EPSQUAD   MACH_EPS * MACH_EPS
#define EPSROOT   sqrt(MACH_EPS)

#define POSMAX    pow ((double) BASIS, MAXEXPON)
#define POSMIN    pow ((double) BASIS, MINEXPON)
#define MAXROOT   sqrt(POSMAX)

#define PI        4.0 * atan (1.0)
#define EXP_1     exp(1.0)

#endif              /*-------------- ENDE ifdef ----------------------*/


#define NEGMAX   -POSMIN                   /* groesste negative Zahl  */
#define NEGMIN   -POSMAX                   /* kleinste negative Zahl  */

#define TRUE      1
#define FALSE     0


/* Definition von Funktionsmakros:
   */

// #define min(X, Y) ((X) < (Y)  ?  (X) : (Y))    /* Minimum von X,Y     */
// #define max(X, Y) ((X) > (Y)  ?  (X) : (Y))    /* Maximum von X,Y     */
#define abs(X) ((X) >= 0  ?  (X) : -(X))       /* Absolutbetrag von X */
#define sign(X, Y) (Y < 0 ? -abs(X) : abs(X))  /* Vorzeichen von      */
                                               /* Y mal abs(X)        */
#define sqr(X) ((X) * (X))                     /* Quadrat von X       */

/*-------------------  ENDE FILE u_const.h  --------------------------*/









/*.HL Anhang: C - Programme*/
/*.HRGleichungssysteme fÅr Tridiagonalmatrizen*/

/*.FE  P 3.7 TRIDIAGONALE GLEICHUNGSSYSTEME*/


/*----------------------   MODUL TRIDIAGONAL  ------------------------*/

USHORT TriDiagGS(BOOL rep, USHORT n, double* lower,
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
/*      n        Dimension der Matrix ( > 1 )  USHORT n               */
/*                                                                    */
/*      lower    untere Nebendiagonale         double lower[n]        */
/*      diag     Hauptdiagonale                double diag[n]         */
/*      upper    obere Nebendiagonale          double upper[n]        */
/*                                                                    */
/*               bei rep != 0 enthalten lower, diag und upper die     */
/*               Dreieckzerlegung der Ausgangsmatrix.                 */
/*                                                                    */
/*      b        rechte Seite des Systems      double b[n]            */
/*      rep      = 0  erstmaliger Aufruf       BOOL rep               */
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
 USHORT i;
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


USHORT ZyklTriDiagGS(BOOL rep, USHORT n, double* lower, double* diag,
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
/*      n        Dimension der Matrix ( > 2 )  USHORT n               */
/*      lower    untere Nebendiagonale         double lower[n]        */
/*      diag     Hauptdiagonale                double diag[n]         */
/*      upper    obere Nebendiagonale          double upper[n]        */
/*      b        rechte Seite des Systems      double b[n]            */
/*      rep      = 0  erstmaliger Aufruf       BOOL rep               */
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
 USHORT i;
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
|*    Beschreibung      Berechnet die Koeffizienten eines natÅrlichen
|*                      kubischen Polynomsplines mit n StÅtzstellen.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/

USHORT NaturalSpline(USHORT n, double* x, double* y,
                     double Marg0, double MargN,
                     BYTE MargCond,
                     double* b, double* c, double* d)
{
    USHORT  i;
    double* a;
    double* h;
    USHORT  error;

    if (n<2) return 1;
    if (MargCond<0 || MargCond>3) return 2;
    a=new double[n+1];
    h=new double[n+1];
    for (i=0;i<n;i++) {
        h[i]=x[i+1]-x[i];
        if (h[i]<=0.0) { delete a; delete h; return 1; }
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
        error=TriDiagGS(FALSE,n-1,b,d,c,a);
        if (error!=0) { delete a; delete h; return error+2; }
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
    delete a;
    delete h;
    return 0;
}


/*************************************************************************
|*
|*    PeriodicSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines periodischen
|*                      kubischen Polynomsplines mit n StÅtzstellen.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/


USHORT PeriodicSpline(USHORT n, double* x, double* y,
                      double* b, double* c, double* d)
{                     // Arrays mÅssen von [0..n] dimensioniert sein!
    USHORT  Error;
    USHORT  i,im1,nm1; //integer
    double  hr,hl;
    double* a;
    double* lowrow;
    double* ricol;

    if (n<2) return 4;
    nm1=n-1;
    for (i=0;i<=nm1;i++) if (x[i+1]<=x[i]) return 2; // mu· streng nonoton fallend sein!
    if (y[n]!=y[0]) return 3; // Anfang mu· gleich Ende sein!

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
        Error=ZyklTriDiagGS(FALSE,n,b,d,c,lowrow,ricol,a);
        if (Error!=0) { delete a,lowrow,ricol; return(Error+4); }
        for (i=0;i<=nm1;i++) c[i+1]=a[i];
    }
    c[0]=c[n];
    for (i=0;i<=nm1;i++) {
        hl=x[i+1]-x[i];
        b[i]=(y[i+1]-y[i])/hl;
        b[i]=b[i]-hl*(c[i+1]+2.0*c[i])/3.0;
        d[i]=(c[i+1]-c[i])/hl/3.0;
    }
    delete a,lowrow,ricol;
    return 0;
}



/*************************************************************************
|*
|*    ParaSpline()
|*
|*    Beschreibung      Berechnet die Koeffizienten eines parametrischen
|*                      natÅrlichen oder periodischen kubischen
|*                      Polynomsplines mit n StÅtzstellen.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/

USHORT ParaSpline(USHORT n, double* x, double* y, BYTE MargCond,
                  double Marg01, double Marg02,
                  double MargN1, double MargN2,
                  BOOL CondT, double* T,
                  double* bx, double* cx, double* dx,
                  double* by, double* cy, double* dy)
{
    USHORT Error,Marg;
    USHORT i;
    double deltX,deltY,delt,
           alphX,alphY,
           betX,betY;

    if (n<2) return 1;
    if (MargCond<0 || MargCond>4) return 2; // ungÅltige Randbedingung
    if (CondT==FALSE) {
        T[0]=0.0;
        for (i=0;i<n;i++) {
            deltX=x[i+1]-x[i]; deltY=y[i+1]-y[i];
            delt =deltX*deltX+deltY*deltY;
            if (delt<=0.0) return 3;            // zwei identische Punkte nacheinander!
            T[i+1]=T[i]+sqrt(delt);
        }
    }
    switch (MargCond) {
        case 0: Marg=0; break;
        case 1: case 2: {
            Marg=MargCond;
            alphX=Marg01; betX=MargN1;
            alphY=Marg02; betY=MargN2;
        } break;
        case 3: {
            if (x[n]!=x[0]) return 3;
            if (y[n]!=y[0]) return 4;
        } break;
        case 4: {
            Marg=1;
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
|*                      natÅrlichen oder periodischen kubischen
|*                      Polynomsplines. Die Eckpunkte des öbergebenen
|*                      Polygons werden als StÅtzstellen angenommen.
|*                      n liefert die Anzahl der Teilpolynome.
|*                      Ist die Berechnung fehlerfrei verlaufen, so
|*                      liefert die Funktion TRUE. Nur in diesem Fall
|*                      ist Speicher fÅr die Koeffizientenarrays
|*                      allokiert, der dann spÑter vom Aufrufer mittels
|*                      delete freizugeben ist.
|*    Ersterstellung    JOE 17-08.93
|*    Letzte Aenderung  JOE 17-08.93
|*
*************************************************************************/

BOOL CalcSpline(Polygon& rPoly, BOOL Periodic, USHORT& n,
                double*& ax, double*& ay, double*& bx, double*& by,
                double*& cx, double*& cy, double*& dx, double*& dy, double*& T)
{
    BYTE   Marg;
    double Marg01,Marg02;
    double MargN1,MargN2;
    USHORT i;
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

    Marg01=0.0; Marg02=0.0;
    MargN1=0.0; MargN2=0.0;
    if (n>0) n--; // n Korregieren (Anzahl der Teilpolynome)

    if ((Marg==3 && n>=3) || (Marg==2 && n>=2)) {
        i=ParaSpline(n,ax,ay,Marg,Marg01,Marg01,MargN1,MargN2,FALSE,T,bx,cx,dx,by,cy,dy);
        if (i==0) {
            return TRUE;
        } else {
            delete ax,ay,bx,by,cx,cy,dx,dy,T;
            n=0;
            return FALSE;
        }
    } else {
        delete ax,ay,bx,by,cx,cy,dx,dy,T;
        n=0;
        return FALSE;
    }
}


/*************************************************************************
|*
|*    Spline2Poly()
|*
|*    Beschreibung      Konvertiert einen parametrichen kubischen
|*                      Polynomspline Spline (natÅrlich oder periodisch)
|*                      in ein angenÑhertes Polygon.
|*                      Die Funktion liefert FALSE, wenn ein Fehler bei
|*                      der Koeffizientenberechnung aufgetreten ist oder
|*                      das Polygon zu gro· wird (>PolyMax=16380). Im 1.
|*                      Fall hat das Polygon 0, im 2. Fall PolyMax Punkte.
|*                      Um KoordinatenÅberlÑufe zu vermeiden werden diese
|*                      auf +/-32000 begrenzt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
BOOL Spline2Poly(Polygon& rSpln, BOOL Periodic, Polygon& rPoly)
{
    short  MinKoord=-32000; // zur Vermeidung
    short  MaxKoord=32000;  // von öberlÑufen

    double* ax;          // ø
    double* ay;          // ≥ Koeffizienten
    double* bx;          // ≥ der Polynome
    double* by;          // ≥
    double* cx;          // ≥
    double* cy;          // ≥
    double* dx;          // ≥
    double* dy;          // ≥
    double* tv;          // Ÿ

    double  Step;        // Schrittweite fÅr t
    double  dt1,dt2,dt3; // Delta t, ˝, ^3
    double  t;
    BOOL    bEnde;       // Teilpolynom zu Ende?
    USHORT  n;           // Anzahl der zu zeichnenden Teilpolynome
    USHORT  i;           // aktuelles Teilpolynom
    BOOL    bOk;         // noch alles ok?
    USHORT  PolyMax=16380;// Maximale Anzahl von Polygonpunkten
    long    x,y;

    bOk=CalcSpline(rSpln,Periodic,n,ax,ay,bx,by,cx,cy,dx,dy,tv);
    if (bOk) {
        Step =10;

        rPoly.SetSize(1);
        rPoly.SetPoint(Point(short(ax[0]),short(ay[0])),0); // erster Punkt
        i=0;
        while (i<n) {       // n Teilpolynome malen
            t=tv[i]+Step;
            bEnde=FALSE;
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
                    bOk=FALSE; // Fehler: Polygon wird zu gro·
                }
                t=t+Step;
            } // Ende von Teilpolynom
            i++; // nÑchstes Teilpolynom
        }
        delete ax,ay,bx,by,cx,cy,dx,dy,tv;
        return bOk;
    } // Ende von if (bOk)
    rPoly.SetSize(0);
    return FALSE;
}









/*************************************************************************
|*
|*    CalcLine()
|*
|*    Beschreibung      Routine fÅr DrawSpline (s.u.).
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
/*void CalcLine(short x1, short y1,
              short x2, short y2,
              short hb, Polygon& rPoly)
{
    short  dx,dy,dxp,dyp;
    double l;

    rPoly.SetSize(4);
    dx=x2-x1;
    dy=y2-y1;
    l=sqrt(dx*dx+dy*dy);
    if (l>0) {
        dxp=short((long(dy)*long(hb))/l);
        dyp=short((long(dx)*long(hb))/l);
        rPoly.SetPoint(Point(x1-dxp,y1+dyp),0);
        rPoly.SetPoint(Point(x1+dxp,y1-dyp),1);
        rPoly.SetPoint(Point(x2+dxp,y2-dyp),2);
        rPoly.SetPoint(Point(x2-dxp,y2+dyp),3);
    } else {
        rPoly.SetPoint(Point(x1,y1+hb),0);
        rPoly.SetPoint(Point(x1,y1-hb),1);
        rPoly.SetPoint(Point(x2,y2-hb),2);
        rPoly.SetPoint(Point(x2,y2+hb),3);
    }
} */


/*************************************************************************
|*
|*    DrawSpline()
|*
|*    Beschreibung      Alte Routine. Dicke Linien werden mit Polys gemalt.
|*    Ersterstellung    JOE 23.06.93
|*    Letzte Aenderung  JOE 23.06.93
|*
*************************************************************************/
/*
BOOL DrawSpline(OutputDevice& rOut, Polygon& rPoly, BOOL Periodic)
{
    short  MinKoord=-12000; // StarDraw-Definitionen
    short  MaxKoord=32000;  // zur Vermeidung von öberlÑufen

    double* ax;          // ø
    double* ay;          // ≥ Koeffizienten
    double* bx;          // ≥ der Polynome
    double* by;          // ≥
    double* cx;          // ≥
    double* cy;          // ≥
    double* dx;          // ≥
    double* dy;          // ≥
    double* tv;          // Ÿ

    double  Step;        // Schrittweite fÅr t
    double  dt1,dt2,dt3; // Delta t, ˝, ^3
    double  t;
    BOOL    bEnde;       // Teilpolynom zu Ende?
    USHORT  FHB;         // Halbe Linienbreite in SGV
    Point   P10,P20;     // letzter Punkt (Screen) fÅr Anschlu·
    Point   P100,P200;   // erster Punkt (Screen) zum Schlie·en des pSpline
    BOOL    bFirst;      // erster Schrit des ersten Teilpolynoms?
    BOOL    bThin;       // StrichstÑrke 1 Pixel?
    Polygon aFP(0);      // fÅr gefÅllten Spline
    Polygon aLP(4);      // fÅr dicke Umrandung
    BOOL    bFill;       // gefÅllter Spline?
    BOOL    bOutl;       // umrandeter Spline?
    USHORT  n;           // Anzahl der zu zeichnenden Teilpolynome
    USHORT  i;           // aktuelles Teilpolynom
    BOOL    bOk;         // noch alles ok?
    USHORT  StatA,StatB;
    USHORT  PolyMax=16384;// Maximale Anzahl von Polygonpunkten
    Pen     aPenMerk;
    Brush   aBrushMerk;
    short   x0,y0;
    long    x,y;

    bOk=CalcSpline(rPoly,Periodic,n,ax,ay,bx,by,cx,cy,dx,dy,tv);
    if (bOk) {
        aPenMerk=rOut.GetPen();
        aBrushMerk=rOut.GetFillInBrush();
        bFill=rOut.GetFillInBrush().GetStyle()!=BRUSH_NULL && Periodic;
        bOutl=rOut.GetPen().GetStyle()!=PEN_NULL;
        FHB  =rOut.GetPen().GetWidth() /2;
        bThin=rOut.GetPen().GetWidth()<=1;
        Step =10;

        StatA=1; if (!bFill) StatA++;
        StatB=2; if (!bOutl) StatB--;

        while (StatA<=StatB) {
            if (StatA==1) {
                rOut.SetFillInBrush(aBrushMerk);
                rOut.SetPen(Pen(PEN_NULL));
            } else {
                if (!bThin) {
                    rOut.SetFillInBrush(Brush(rOut.GetPen().GetColor()));
                    rOut.SetPen(Pen(PEN_NULL));
                } else {
                    rOut.SetPen(aPenMerk);
                }
            }
            bFirst=TRUE;

            x0=short(ax[0]); y0=short(ay[0]); // erster Punkt
            i=0;
            while (i<n) {       // n Teilpolynome malen
                t=tv[i]+Step;
                bEnde=FALSE;
                while (!bEnde) {  // ein Teilpolynom interpolieren
                    bEnde=t>=tv[i+1];
                    if (bEnde) t=tv[i+1];
                    dt1=t-tv[i]; dt2=dt1*dt1; dt3=dt2*dt1;
                    x=long(ax[i]+bx[i]*dt1+cx[i]*dt2+dx[i]*dt3);
                    y=long(ay[i]+by[i]*dt1+cy[i]*dt2+dy[i]*dt3);
                    if (x-FHB<MinKoord) x=MinKoord+FHB; if (x+FHB>MaxKoord) x=MaxKoord-FHB;
                    if (y-FHB<MinKoord) y=MinKoord+FHB; if (y+FHB>MaxKoord) y=MaxKoord-FHB;
                    if (StatA==1) {  // FlÑche
                        if (aFP.GetSize()<PolyMax) {
                            aFP.SetSize(aFP.GetSize()+1);
                            aFP.SetPoint(Point(short(x),short(y)),aFP.GetSize()-1);
                        } else {
                            bOk=FALSE; // Fehler: Polygon wird zu gro·
                        }
                    } else {        // Umrandung
                        if (bThin) {
                            rOut.DrawLine(Point(x0,y0),Point(short(x),short(y)));
                        } else {
                            CalcLine(x0,y0,short(x),short(y),FHB,aLP);
                            rOut.DrawPolygon(aLP);
                            if (bFirst) {
                                P100=aLP.GetPoint(0); // fÅr pSpline merken
                                P200=aLP.GetPoint(1);
                            } else {
                                Polygon aMP(4);
                                aMP.SetPoint(P10,0);
                                aMP.SetPoint(P20,1);
                                aMP.SetPoint(aLP.GetPoint(1),2);
                                aMP.SetPoint(aLP.GetPoint(0),3);
                                rOut.DrawPolygon(aMP);
                            }
                            P10=aLP.GetPoint(3); // zum FÅllen der MÑuseecken merken
                            P20=aLP.GetPoint(2);
                        } // Ende von Umrandung
                    } // Ende von if (StatA==1)
                    x0=short(x); y0=short(y);  // die letzten Koordinaten merken
                    bFirst=FALSE;
                    t=t+Step;
                } // Ende von Teilpolynom
                i++; // nÑchstes Teilpolynom
            }
            if (StatA==1) {
                rOut.DrawPolygon(aFP);
            } else {
                if (!bThin && Periodic) {      // nun die letzte MÑuseecke fÅllen
                    Polygon aMP(4);
                    aMP.SetPoint(aLP[3],1);
                    aMP.SetPoint(aLP[4],2);
                    aMP.SetPoint(P100,3);
                    aMP.SetPoint(P200,4);
                    rOut.DrawPolygon(aMP);
                }
            }
            StatA++;
        } // Ende von while (StatA<=StatB)
        rOut.SetPen(aPenMerk);
        rOut.SetFillInBrush(aBrushMerk);
        delete ax,ay,bx,by,cx,cy,dx,dy,tv;
        return TRUE;
    } // Ende von if (bOk)
    return FALSE;
} */




