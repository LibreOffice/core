/*************************************************************************
 *
 *  $RCSfile: interpr3.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2001-02-28 14:29:23 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/solar.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interpre.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "scmatrix.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

#define SCdEpsilon                1.0E-7
#define SC_MAX_ITERATION_COUNT    20
#define MAX_ANZ_DOUBLE_FOR_SORT 100000
// PI jetzt als F_PI aus solar.h
//#define   PI            3.1415926535897932

//-----------------------------------------------------------------------------

class ScDistFunc
{
public:
    virtual double GetValue(double x) const = 0;
};

//  iteration for inverse distributions

//template< class T > double lcl_IterateInverse( const T& rFunction, double x0, double x1, BOOL& rConvError )
double lcl_IterateInverse( const ScDistFunc& rFunction, double x0, double x1, BOOL& rConvError )
{
    rConvError = FALSE;
    double fEps = 1.0E-7;

    DBG_ASSERT(x0<x1, "IterateInverse: wrong interval");

    //  find enclosing interval

    double f0 = rFunction.GetValue(x0);
    double f1 = rFunction.GetValue(x1);
    double xs;
    USHORT i;
    for (i = 0; i < 1000 && f0*f1 > 0.0; i++)
    {
        if (fabs(f0) <= fabs(f1))
        {
            xs = x0;
            x0 += 2.0 * (x0 - x1);
            if (x0 < 0.0)
                x0 = 0.0;
            x1 = xs;
            f1 = f0;
            f0 = rFunction.GetValue(x0);
        }
        else
        {
            xs = x1;
            x1 += 2.0 * (x1 - x0);
            x0 = xs;
            f0 = f1;
            f1 = rFunction.GetValue(x1);
        }
    }

    if (f0 == 0.0)
        return x0;
    if (f1 == 0.0)
        return x1;

    //  simple iteration

    double x00 = x0;
    double x11 = x1;
    double fs;
    for (i = 0; i < 100; i++)
    {
        xs = 0.5*(x0+x1);
        if (fabs(f1-f0) >= fEps)
        {
            fs = rFunction.GetValue(xs);
            if (f0*fs <= 0.0)
            {
                x1 = xs;
                f1 = fs;
            }
            else
            {
                x0 = xs;
                f0 = fs;
            }
        }
        else
        {
            //  add one step of regula falsi to improve precision

            if ( x0 != x1 )
            {
                double regxs = (f1-f0)/(x1-x0);
                if ( regxs != 0.0)
                {
                    double regx = x1 - f1/regxs;
                    if (regx >= x00 && regx <= x11)
                    {
                        double regfs = rFunction.GetValue(regx);
                        if ( fabs(regfs) < fabs(fs) )
                            xs = regx;
                    }
                }
            }

            return xs;
        }
    }

    rConvError = TRUE;
    return 0.0;
}

//-----------------------------------------------------------------------------
// Allgemeine Funktionen
//-----------------------------------------------------------------------------

void ScInterpreter::ScNoName()
{
    SetError(errNoName);
}

double ScInterpreter::phi(double x)
{
    return  0.39894228040143268 * exp(-(x * x) / 2.0);
}

double ScInterpreter::taylor(double* pPolynom, USHORT nMax, double x)
{
    double nVal = pPolynom[nMax];
    for (short i = nMax-1; i >= 0; i--)
    {
        nVal = pPolynom[i] + (nVal * x);
    }
    return nVal;
}

double ScInterpreter::gauss(double x)
{
    double t0[] =
    { 0.39894228040143268, -0.06649038006690545,  0.00997355701003582,
     -0.00118732821548045,  0.00011543468761616, -0.00000944465625950,
      0.00000066596935163, -0.00000004122667415,  0.00000000227352982,
      0.00000000011301172,  0.00000000000511243, -0.00000000000021218 };
    double t2[] =
    { 0.47724986805182079,  0.05399096651318805, -0.05399096651318805,
      0.02699548325659403, -0.00449924720943234, -0.00224962360471617,
      0.00134977416282970, -0.00011783742691370, -0.00011515930357476,
      0.00003704737285544,  0.00000282690796889, -0.00000354513195524,
      0.00000037669563126,  0.00000019202407921, -0.00000005226908590,
     -0.00000000491799345,  0.00000000366377919, -0.00000000015981997,
     -0.00000000017381238,  0.00000000002624031,  0.00000000000560919,
     -0.00000000000172127, -0.00000000000008634,  0.00000000000007894 };
    double t4[] =
    { 0.49996832875816688,  0.00013383022576489, -0.00026766045152977,
      0.00033457556441221, -0.00028996548915725,  0.00018178605666397,
     -0.00008252863922168,  0.00002551802519049, -0.00000391665839292,
     -0.00000074018205222,  0.00000064422023359, -0.00000017370155340,
      0.00000000909595465,  0.00000000944943118, -0.00000000329957075,
      0.00000000029492075,  0.00000000011874477, -0.00000000004420396,
      0.00000000000361422,  0.00000000000143638, -0.00000000000045848 };
    double asympt[] = { -1.0, 1.0, -3.0, 15.0, -105.0 };

    double xAbs = fabs(x);
    USHORT xShort = (USHORT)SolarMath::ApproxFloor(xAbs);
    double nVal = 0.0;
    if (xShort == 0)
        nVal = taylor(t0, 11, (xAbs * xAbs)) * xAbs;
    else if ((xShort >= 1) && (xShort <= 2))
        nVal = taylor(t2, 23, (xAbs - 2.0));
    else if ((xShort >= 3) && (xShort <= 4))
        nVal = taylor(t4, 20, (xAbs - 4.0));
    else
        nVal = 0.5 + phi(xAbs) * taylor(asympt, 4, 1.0 / (xAbs * xAbs)) / xAbs;
    if (x < 0.0)
        return -nVal;
    else
        return nVal;
}

#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif

double ScInterpreter::gaussinv(double x)
{
    double c0, c1, c2, d1, d2, d3, q, t, z;
    c0 = 2.515517;
    c1 = 0.802853;
    c2 = 0.010328;
    d1 = 1.432788;
    d2 = 0.189269;
    d3 = 0.001308;
    if (x < 0.5)
        q = x;
    else
        q = 1.0-x;
    t = sqrt(-log(q*q));
    z = t - (c0 + t*(c1 + t*c2)) / (1.0 + t*(d1 + t*(d2 + t*d3)));
    if (x < 0.5)
        z *= -1.0;
    return z;
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif

double ScInterpreter::Fakultaet(double x)
{
    x = SolarMath::ApproxFloor(x);
    if (x < 0.0)
        return 0.0;
    else if (x == 0.0)
        return 1.0;
    else if (x <= 170.0)
    {
        double fTemp = x;
        while (fTemp > 2.0)
        {
            fTemp--;
            x *= fTemp;
        }
    }
    else
        SetError(errNoValue);
/*                                           // Stirlingsche Naeherung zu ungenau
    else
        x = pow(x/exp(1), x) * sqrt(x) * SQRT_2_PI * (1.0 + 1.0 / (12.0 * x));
*/
    return x;
}

double ScInterpreter::BinomKoeff(double n, double k)
{
    double nVal = 0.0;
    k = SolarMath::ApproxFloor(k);
    if (n < k)
        nVal = 0.0;
    else if (k == 0.0)
        nVal = 1.0;
    else
    {
        nVal = n/k;
        n--;
        k--;
        while (k > 0.0)
        {
            nVal *= n/k;
            k--;
            n--;
        }
/*
        double f1 = n;                      // Zaehler
        double f2 = k;                      // Nenner
        n--;
        k--;
        while (k > 0.0)
        {
            f2 *= k;
            f1 *= n;
            k--;
            n--;
        }
        nVal = f1 / f2;
*/
    }
    return nVal;
}

double ScInterpreter::GammaHelp(double& x, BOOL& bReflect)
{
    double c[6] = {76.18009173, -86.50532033, 24.01409822,
                   -1.231739516, 0.120858003E-2, -0.536382E-5};
    if (x >= 1.0)
    {
        bReflect = FALSE;
        x -= 1.0;
    }
    else
    {
        bReflect = TRUE;
        x = 1.0 - x;
    }
    double s, anum;
    s = 1.0;
    anum = x;
    for (USHORT i = 0; i < 6; i++)
    {
        anum += 1.0;
        s += c[i]/anum;
    }
    s *= 2.506628275;                   // sqrt(2*PI)
    return s;
}

double ScInterpreter::GetGamma(double x)
{
    BOOL bReflect;
    double G = GammaHelp(x, bReflect);
    G = pow(x+5.5,x+0.5)*G/exp(x+5.5);
    if (bReflect)
        G = F_PI*x/(G*sin(F_PI*x));
    return G;
}

double ScInterpreter::GetLogGamma(double x)
{
    BOOL bReflect;
    double G = GammaHelp(x, bReflect);
    G = (x+0.5)*log(x+5.5)+log(G)-(x+5.5);
    if (bReflect)
        G = log(F_PI*x)-G-log(sin(F_PI*x));
    return G;
}

double ScInterpreter::GetBetaDist(double x, double alpha, double beta)
{
    if (beta == 1.0)
        return pow(x, alpha);
    else if (alpha == 1.0)
        return 1.0 - pow(1.0-x,beta);
    double fEps = 1.0E-8;
    BOOL bReflect;
    double cf, fA, fB;
    if (x < (alpha+1.0)/(alpha+beta+1.0))
    {
        bReflect = FALSE;
        fA = alpha;
        fB = beta;
    }
    else
    {
        bReflect = TRUE;
        fA = beta;
        fB = alpha;
        x = 1.0 - x;
    }
    if (x < fEps)
        cf = 0.0;
    else
    {
        double a1, b1, a2, b2, fnorm, rm, apl2m, d2m, d2m1, cfnew;
        a1 = 1.0; b1 = 1.0;
        b2 = 1.0 - (fA+fB)*x/(fA+1.0);
        if (b2 == 0.0)
        {
            a2 = b2;
            fnorm = 1.0;
            cf = 1.0;
        }
        else
        {
            a2 = 1.0;
            fnorm = 1.0/b2;
            cf = a2*fnorm;
        }
        cfnew = 1.0;
        for (USHORT j = 1; j <= 100; j++)
        {
            rm = (double) j;
            apl2m = fA + 2.0*rm;
            d2m = rm*(fB-rm)*x/((apl2m-1.0)*apl2m);
            d2m1 = -(fA+rm)*(fA+fB+rm)*x/(apl2m*(apl2m+1.0));
            a1 = (a2+d2m*a1)*fnorm;
            b1 = (b2+d2m*b1)*fnorm;
            a2 = a1 + d2m1*a2*fnorm;
            b2 = b1 + d2m1*b2*fnorm;
            if (b2 != 0.0)
            {
                fnorm = 1.0/b2;
                cfnew = a2*fnorm;
                if (fabs(cf-cfnew)/cf < fEps)
                    j = 101;
                else
                    cf = cfnew;
            }
        }
        if (fB < fEps)
            b1 = 1.0E30;
        else
            b1 = exp(GetLogGamma(fA)+GetLogGamma(fB)-GetLogGamma(fA+fB));

        cf *= pow(x, fA)*pow(1.0-x,fB)/(fA*b1);
    }
    if (bReflect)
        return 1.0-cf;
    else
        return cf;
}

double ScInterpreter::GetFDist(double x, double fF1, double fF2)
{
    double arg = fF2/(fF2+fF1*x);
    double alpha = fF2/2.0;
    double beta = fF1/2.0;
    return (GetBetaDist(arg, alpha, beta));
/*
    double Z = (pow(fF,1.0/3.0)*(1.0-2.0/(9.0*fF2)) - (1.0-2.0/(9.0*fF1))) /
               sqrt(2.0/(9.0*fF1) + pow(fF,2.0/3.0)*2.0/(9.0*fF2));
    return (0.5-gauss(Z));
*/
}

double ScInterpreter::GetTDist(double T, double fDF)
{
    return 0.5 * GetBetaDist(fDF/(fDF+T*T), fDF/2.0, 0.5);
/*
    USHORT DF = (USHORT) fDF;
    double A = T / sqrt(DF);
    double B = 1.0 + A*A;
    double R;
    if (DF == 1)
        R = 0.5 + atan(A)/F_PI;
    else if (DF % 2 == 0)
    {
        double S0 = A/(2.0 * sqrt(B));
        double C0 = S0;
        for (USHORT i = 2; i <= DF-2; i+=2)
        {
            C0 *= (1.0 - 1.0/(double)i)/B;
            S0 += C0;
        }
        R = 0.5 + S0;
    }
    else
    {
        double S1 = A / (B * F_PI);
        double C1 = S1;
        for (USHORT i = 3; i <= DF-2; i+=2)
        {
            C1 *= (1.0 - 1.0/(double)i)/B;
            S1 += C1;
        }
        R = 0.5 + atan(A)/F_PI + S1;
    }
    return 1.0 - R;
*/
}

double ScInterpreter::GetChiDist(double fChi, double fDF)
{
    return 1.0 - GetGammaDist(fChi/2.0, fDF/2.0, 1.0);
/*
    double x = 1.0;
    for (double i = fDF; i >= 2.0; i -= 2.0)
        x *= fChi/i;
    x *= exp(-fChi/2.0);
    if (fmod(fDF, 2.0) != 0.0)
        x *= sqrt(2.0*fChi/F_PI);
    double S = 1.0;
    double T = 1.0;
    double G = fDF;
    BOOL bStop = FALSE;
    while (!bStop)
    {
        G += 2.0;
        T *= fChi/G;
        if (T < 1.0E-7)
            bStop = TRUE;
        else
            S += T;
    }
    return 1.0 - x*S;
*/
}

void ScInterpreter::ScLogGamma()
{
    double x = GetDouble();
    if (x > 0.0)
        PushDouble(GetLogGamma(x));
    else
        SetIllegalArgument();
}

void ScInterpreter::ScBetaDist()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    double fA, fB, alpha, beta, x;
    if (nParamCount == 5)
        fB = GetDouble();
    else
        fB = 1.0;
    if (nParamCount >= 4)
        fA = GetDouble();
    else
        fA = 0.0;
    beta = GetDouble();
    alpha = GetDouble();
    x = GetDouble();
    if (x < fA || x > fB || fA == fB || alpha <= 0.0 || beta <= 0.0)
    {
        SetIllegalArgument();
        return;
    }
    x = (x-fA)/(fB-fA);                 // Skalierung auf (0,1)
    PushDouble(GetBetaDist(x, alpha, beta));
}

void ScInterpreter::ScPhi()
{
    PushDouble(phi(GetDouble()));
}

void ScInterpreter::ScGauss()
{
    PushDouble(gauss(GetDouble()));
}

void ScInterpreter::ScFisher()
{
    double fVal = GetDouble();
    if (fabs(fVal) >= 1.0)
        SetIllegalArgument();
    else
        PushDouble(0.5*log((1.0+fVal)/(1.0-fVal)));
}

void ScInterpreter::ScFisherInv()
{
    double fVal = GetDouble();
    PushDouble((exp(2.0*fVal)-1.0)/(exp(2.0*fVal)+1.0));
}

void ScInterpreter::ScFact()
{
    double nVal = GetDouble();
    if (nVal < 0.0)
        SetIllegalArgument();
    else
        PushDouble(Fakultaet(nVal));
}

void ScInterpreter::ScKombin()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = SolarMath::ApproxFloor(GetDouble());
        double n = SolarMath::ApproxFloor(GetDouble());
        if (k < 0.0 || n < 0.0 || k > n)
            SetIllegalArgument();
        else
            PushDouble(BinomKoeff(n, k));
    }
}

void ScInterpreter::ScKombin2()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = SolarMath::ApproxFloor(GetDouble());
        double n = SolarMath::ApproxFloor(GetDouble());
        if (k < 0.0 || n < 0.0 || k > n)
            SetIllegalArgument();
        else
            PushDouble(BinomKoeff(n + k - 1, k));
    }
}

void ScInterpreter::ScVariationen()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = SolarMath::ApproxFloor(GetDouble());
        double n = SolarMath::ApproxFloor(GetDouble());
        if (n < 0.0 || k < 0.0 || k > n)
            SetIllegalArgument();
        else if (k == 0.0)
            PushInt(1);     // (n! / (n - 0)!) == 1
        else
        {
            double nVal = n;
            for (ULONG i = (ULONG)k-1; i >= 1; i--)
                nVal *= n-(double)i;
            PushDouble(nVal);
        }
    }
}

void ScInterpreter::ScVariationen2()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double k = SolarMath::ApproxFloor(GetDouble());
        double n = SolarMath::ApproxFloor(GetDouble());
        if (n < 0.0 || k < 0.0 || k > n)
            SetIllegalArgument();
        else
            PushDouble(pow(n,k));
    }
}

void ScInterpreter::ScB()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return ;
    if (nParamCount == 3)
    {
        double x = SolarMath::ApproxFloor(GetDouble());
        double p = GetDouble();
        double n = SolarMath::ApproxFloor(GetDouble());
        if (n < 0.0 || x < 0.0 || x > n || p < 0.0 || p > 1.0)
            SetIllegalArgument();
        else
        {
            double q = 1.0 - p;
            double fFactor = pow(q, n);
            if (fFactor == 0.0)
            {
                fFactor = pow(p, n);
                if (fFactor == 0.0)
                    SetNoValue();
                else
                {
                    ULONG max = (ULONG) (n - x);
                    for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                        fFactor *= (n-i)/(i+1)*q/p;
                    PushDouble(fFactor);
                }
            }
            else
            {
                ULONG max = (ULONG) x;
                for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                    fFactor *= (n-i)/(i+1)*p/q;
                PushDouble(fFactor);
            }
        }
    }
    else if (nParamCount == 4)
    {
        double xe = GetDouble();
        double xs = GetDouble();
        double p = GetDouble();
        double n = GetDouble();
//                                          alter Stand 300-SC
//      if ((xs < n) && (xe < n) && (p < 1.0))
//      {
//          double Varianz = sqrt(n * p * (1.0 - p));
//          xs = fabs(xs - (n * p /* / 2.0 STE */ ));
//          xe = fabs(xe - (n * p /* / 2.0 STE */ ));
//// STE        double nVal = gauss((xs + 0.5) / Varianz) + gauss((xe + 0.5) / Varianz);
//          double nVal = fabs(gauss(xs / Varianz) - gauss(xe / Varianz));
//          PushDouble(nVal);
//      }
        if (xe <= n && xs <= xe &&
            p < 1.0 && p > 0.0 && n >= 0.0 && xs >= 0.0 )
        {
            double q = 1.0 - p;
            double fFactor = pow(q, n);
            if (fFactor == 0.0)
            {
                fFactor = pow(p, n);
                if (fFactor == 0.0)
                    SetNoValue();
                else
                {
                    double fSum = 0.0;
                    ULONG max;
                    if (xe < (ULONG) n)
                        max = (ULONG) (n-xe)-1;
                    else
                        max = 0;
                    ULONG i;
                    for (i = 0; i < max && fFactor > 0.0; i++)
                        fFactor *= (n-i)/(i+1)*q/p;
                    if (xs < (ULONG) n)
                        max = (ULONG) (n-xs);
                    else
                        fSum = fFactor;
                    for (; i < max && fFactor > 0.0; i++)
                    {
                        fFactor *= (n-i)/(i+1)*q/p;
                        fSum += fFactor;
                    }
                    PushDouble(fSum);
                }
            }
            else
            {
                ULONG max;
                double fSum;
                if ( (ULONG) xs == 0)
                {
                    fSum = fFactor;
                    max = 0;
                }
                else
                {
                    max = (ULONG) xs-1;
                    fSum = 0.0;
                }
                ULONG i;
                for (i = 0; i < max && fFactor > 0.0; i++)
                    fFactor *= (n-i)/(i+1)*p/q;
                if ((ULONG)xe == 0)                     // beide 0
                    fSum = fFactor;
                else
                    max = (ULONG) xe;
                for (; i < max && fFactor > 0.0; i++)
                {
                    fFactor *= (n-i)/(i+1)*p/q;
                    fSum += fFactor;
                }
                PushDouble(fSum);
            }
        }
        else
            SetIllegalArgument();
    }
}

void ScInterpreter::ScBinomDist()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double kum    = GetDouble();                    // 0 oder 1
        double p      = GetDouble();                    // p
        double n      = SolarMath::ApproxFloor(GetDouble());                // n
        double x      = SolarMath::ApproxFloor(GetDouble());                // x
        double fFactor, q, fSum;
        if (n < 0.0 || x < 0.0 || x > n || p < 0.0 || p > 1.0)
            SetIllegalArgument();
        else if (kum == 0.0)                        // Dichte
        {
            q = 1.0 - p;
            fFactor = pow(q, n);
            if (fFactor == 0.0)
            {
                fFactor = pow(p, n);
                if (fFactor == 0.0)
                    SetNoValue();
                else
                {
                    ULONG max = (ULONG) (n - x);
                    for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                        fFactor *= (n-i)/(i+1)*q/p;
                    PushDouble(fFactor);
                }
            }
            else
            {
                ULONG max = (ULONG) x;
                for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                    fFactor *= (n-i)/(i+1)*p/q;
                PushDouble(fFactor);
            }
        }
        else                                        // Verteilung
        {
            if (n == x)
                PushDouble(1.0);
            else
            {
                q = 1.0 - p;
                fFactor = pow(q, n);
                if (fFactor == 0.0)
                {
                    fFactor = pow(p, n);
                    if (fFactor == 0.0)
                        SetNoValue();
                    else
                    {
                        fSum = 1.0 - fFactor;
                        ULONG max = (ULONG) (n - x) - 1;
                        for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                        {
                            fFactor *= (n-i)/(i+1)*q/p;
                            fSum -= fFactor;
                        }
                        if (fSum < 0.0)
                            PushDouble(0.0);
                        else
                            PushDouble(fSum);
                    }
                }
                else
                {
                    double fSum = fFactor;
                    ULONG max = (ULONG) x;
                    for (ULONG i = 0; i < max && fFactor > 0.0; i++)
                    {
                        fFactor *= (n-i)/(i+1)*p/q;
                        fSum += fFactor;
                    }
                    PushDouble(fSum);
                }
            }
        }
    }
}

void ScInterpreter::ScCritBinom()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double alpha  = GetDouble();                    // alpha
        double p      = GetDouble();                    // p
        double n      = SolarMath::ApproxFloor(GetDouble());
        if (n < 0.0 || alpha <= 0.0 || alpha >= 1.0 || p < 0.0 || p > 1.0)
            SetIllegalArgument();
        else
        {
            double q = 1.0 - p;
            double fFactor = pow(q,n);
            if (fFactor == 0.0)
            {
                fFactor = pow(p, n);
                if (fFactor == 0.0)
                    SetNoValue();
                else
                {
                    double fSum = 1.0 - fFactor; ULONG max = (ULONG) n;
                    for (ULONG i = 0; i < max && fSum >= alpha; i++)
                    {
                        fFactor *= (n-i)/(i+1)*q/p;
                        fSum -= fFactor;
                    }
                    PushDouble(n-i);
                }
            }
            else
            {
                double fSum = fFactor; ULONG max = (ULONG) n;
                for (ULONG i = 0; i < max && fSum < alpha; i++)
                {
                    fFactor *= (n-i)/(i+1)*p/q;
                    fSum += fFactor;
                }
                PushDouble(i);
            }
        }
    }
}

void ScInterpreter::ScNegBinomDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double p      = GetDouble();                    // p
        double r      = GetDouble();                    // r
        double x      = GetDouble();                    // x
        if (r < 0.0 || x < 0.0 || p < 0.0 || p > 1.0)
            SetIllegalArgument();
        else
        {
            double q = 1.0 - p;
            double fFactor = pow(p,r);
            for (double i = 0.0; i < x; i++)
                fFactor *= (i+r)/(i+1.0)*q;
            PushDouble(fFactor);
        }
    }
}

void ScInterpreter::ScNormDist()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double kum = GetDouble();                   // 0 oder 1
        double sigma = GetDouble();                 // Stdabw
        double mue = GetDouble();                   // Mittelwert
        double x = GetDouble();                     // x
        if (sigma <= 0.0)
            SetIllegalArgument();
        else if (kum == 0.0)                        // Dichte
            PushDouble(phi((x-mue)/sigma)/sigma);
        else                                        // Verteilung
            PushDouble(0.5 + gauss((x-mue)/sigma));
    }
}

void ScInterpreter::ScLogNormDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();                 // Stdabw
        double mue = GetDouble();                   // Mittelwert
        double x = GetDouble();                     // x
        if (sigma <= 0.0 || x <= 0.0)
            SetIllegalArgument();
        else
            PushDouble(0.5 + gauss((log(x)-mue)/sigma));
    }
}

void ScInterpreter::ScStdNormDist()
{
    PushDouble(0.5 + gauss(GetDouble()));
}

void ScInterpreter::ScExpDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double kum    = GetDouble();                    // 0 oder 1
        double lambda = GetDouble();                    // lambda
        double x      = GetDouble();                    // x
        if (lambda <= 0.0)
            SetIllegalArgument();
        else if (kum == 0.0)                        // Dichte
        {
            if (x >= 0.0)
                PushDouble(lambda * exp(-lambda*x));
            else
                PushInt(0);
        }
        else                                        // Verteilung
        {
            if (x > 0.0)
                PushDouble(1.0 - exp(-lambda*x));
            else
                PushInt(0);
        }
    }
}

void ScInterpreter::ScTDist()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fFlag = SolarMath::ApproxFloor(GetDouble());
    double fDF   = SolarMath::ApproxFloor(GetDouble());
    double T     = GetDouble();
    if (fDF < 1.0 || T < 0.0 || (fFlag != 1.0 && fFlag != 2.0) )
    {
        SetIllegalArgument();
        return;
    }
    double R = GetTDist(T, fDF);
    if (fFlag == 1.0)
        PushDouble(R);
    else
        PushDouble(2.0*R);
}

void ScInterpreter::ScFDist()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fF2 = SolarMath::ApproxFloor(GetDouble());
    double fF1 = SolarMath::ApproxFloor(GetDouble());
    double fF  = GetDouble();
    if (fF < 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10)
    {
        SetIllegalArgument();
        return;
    }
    PushDouble(GetFDist(fF, fF1, fF2));
}

void ScInterpreter::ScChiDist()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = SolarMath::ApproxFloor(GetDouble());
    double fChi = GetDouble();
    if (fDF < 1.0 || fDF >= 1.0E5 || fChi < 0.0 )
    {
        SetIllegalArgument();
        return;
    }
    PushDouble(GetChiDist(fChi, fDF));
}

void ScInterpreter::ScWeibull()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double kum   = GetDouble();                 // 0 oder 1
        double beta  = GetDouble();                 // beta
        double alpha = GetDouble();                 // alpha
        double x     = GetDouble();                 // x
        if (alpha <= 0.0 || beta <= 0.0 || x < 0.0)
            SetIllegalArgument();
        else if (kum == 0.0)                        // Dichte
            PushDouble(alpha/pow(beta,alpha)*pow(x,alpha-1.0)*
                       exp(-pow(x/beta,alpha)));
        else                                        // Verteilung
            PushDouble(1.0 - exp(-pow(x/beta,alpha)));
    }
}

void ScInterpreter::ScPoissonDist()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double kum    = GetDouble();                    // 0 oder 1
        double lambda = GetDouble();                    // Mittelwert
        double x      = SolarMath::ApproxFloor(GetDouble());                // x
        if (lambda < 0.0 || x < 0.0)
            SetIllegalArgument();
        else if (kum == 0.0)                            // Dichte
        {
            if (lambda == 0.0)
                PushInt(0);
            else
                PushDouble(exp(-lambda)*pow(lambda,x)/Fakultaet(x));
        }
        else                                            // Verteilung
        {
            if (lambda == 0.0)
                PushInt(1);
            else
            {
                double sum = 1.0;
                double fFak = 1.0;
                ULONG nEnd = (ULONG) x;
                for (ULONG i = 1; i <= nEnd; i++)
                {
                    fFak *= (double)i;
                    sum += pow( lambda, (double)i ) / fFak;
                }
                sum *= exp(-lambda);
                PushDouble(sum);
            }
        }
    }
}

void ScInterpreter::ScHypGeomDist()
{
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double N = SolarMath::ApproxFloor(GetDouble());
        double M = SolarMath::ApproxFloor(GetDouble());
        double n = SolarMath::ApproxFloor(GetDouble());
        double x = SolarMath::ApproxFloor(GetDouble());

        if( (x < 0.0) || (n < x) || (M < x) || (N < n) || (N < M) || (x < n - N + M) )
        {
            SetIllegalArgument();
            return;
        }
        double fFactor =
            BinomKoeff( n, x ) / BinomKoeff( N, M ) * BinomKoeff( N - n, M - x );

/*
        double fFactor;
        if (x == n - N + M)
            fFactor = BinomKoeff(M,x)/BinomKoeff(N,n);
        else
        {
            double fIndex = N - M - n;
            if (fIndex >= 0.0)
            {
                fFactor = BinomKoeff(N-M,n)/BinomKoeff(N,n);
                for (double i = 0; i < x; i++)
                    fFactor *= (M-i)*(n-i)/((i+1.0)*(N-M-n+i+1.0));
            }
            else
            {
                fFactor = BinomKoeff(M,-fIndex)/BinomKoeff(N,n);
                for (double i = -fIndex + 1.0; i < x; i++)
                    fFactor *= (M-i)*(n-i)/((i+1)*(N-M-n+i+1.0));
            }
        }
*/
        PushDouble(fFactor);
    }
}

void ScInterpreter::ScGammaDist()
{
    if ( !MustHaveParamCount( GetByte(), 4 ) )
        return;
    double kum = GetDouble();                   // 0 oder 1
    double beta = GetDouble();
    double alpha = GetDouble();
    double x = GetDouble();                     // x
    if (x < 0.0  || alpha <= 0.0 || beta <= 0.0)
        SetIllegalArgument();
    else if (kum == 0.0)                        // Dichte
    {
        double G = GetGamma(alpha);
        PushDouble(pow(x,alpha-1.0)/exp(x/beta)/pow(beta,alpha)/G);
    }
    else                                        // Verteilung
        PushDouble(GetGammaDist(x, alpha, beta));
}

void ScInterpreter::ScNormInv()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();
        double mue   = GetDouble();
        double x     = GetDouble();
        if (sigma <= 0.0 || x < 0.0 || x > 1.0)
            SetIllegalArgument();
        else if (x == 0.0 || x == 1.0)
            SetNoValue();
        else
            PushDouble(gaussinv(x)*sigma + mue);
    }
}

void ScInterpreter::ScSNormInv()
{
    double x = GetDouble();
    if (x < 0.0 || x > 1.0)
        SetIllegalArgument();
    else if (x == 0.0 || x == 1.0)
        SetNoValue();
    else
        PushDouble(gaussinv(x));
}

void ScInterpreter::ScLogNormInv()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();                 // Stdabw
        double mue = GetDouble();                   // Mittelwert
        double y = GetDouble();                     // y
        if (sigma <= 0.0 || y <= 0.0 || y >= 1.0)
            SetIllegalArgument();
        else
            PushDouble(exp(mue+sigma*gaussinv(y)));
    }
}

class ScGammaDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fAlpha, fBeta;

public:
            ScGammaDistFunction( ScInterpreter& rI, double fpVal, double fAlphaVal, double fBetaVal ) :
                rInt(rI), fp(fpVal), fAlpha(fAlphaVal), fBeta(fBetaVal) {}

    double  GetValue( double x ) const  { return fp - rInt.GetGammaDist(x, fAlpha, fBeta); }
};

void ScInterpreter::ScGammaInv()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fBeta  = GetDouble();
    double fAlpha = GetDouble();
    double fP = GetDouble();
    if (fAlpha <= 0.0 || fBeta <= 0.0 || fP < 0.0 || fP >= 1.0 )
    {
        SetIllegalArgument();
        return;
    }
    if (fP == 0.0)
        PushInt(0);
    else
    {
        BOOL bConvError;
        ScGammaDistFunction aFunc( *this, fP, fAlpha, fBeta );
        double fStart = fAlpha * fBeta;
        double fVal = lcl_IterateInverse( aFunc, fStart*0.5, fStart, bConvError );
        if (bConvError)
            SetError(errNoConvergence);
        PushDouble(fVal);
    }
}

class ScBetaDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fAlpha, fBeta;

public:
            ScBetaDistFunction( ScInterpreter& rI, double fpVal, double fAlphaVal, double fBetaVal ) :
                rInt(rI), fp(fpVal), fAlpha(fAlphaVal), fBeta(fBetaVal) {}

    double  GetValue( double x ) const  { return fp - rInt.GetBetaDist(x, fAlpha, fBeta); }
};

void ScInterpreter::ScBetaInv()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    double fP, fA, fB, fAlpha, fBeta;
    if (nParamCount == 5)
        fB = GetDouble();
    else
        fB = 1.0;
    if (nParamCount >= 4)
        fA = GetDouble();
    else
        fA = 0.0;
    fBeta  = GetDouble();
    fAlpha = GetDouble();
    fP     = GetDouble();
    if (fP < 0.0 || fP >= 1.0 || fA == fB || fAlpha <= 0.0 || fBeta <= 0.0)
    {
        SetIllegalArgument();
        return;
    }
    if (fP == 0.0)
        PushInt(0);
    else
    {
        BOOL bConvError;
        ScBetaDistFunction aFunc( *this, fP, fAlpha, fBeta );
        // 0..1 as range for iteration so it isn't extended beyond the valid range
        double fVal = lcl_IterateInverse( aFunc, 0.0, 1.0, bConvError );
        if (bConvError)
        {
            SetError(errNoConvergence);
            PushInt(0);
        }
        else
            PushDouble(fA + fVal*(fB-fA));                  // scale to (A,B)
    }
}

                                                            // Achtung: T, F und Chi
                                                            // sind monoton fallend,
                                                            // deshalb 1-Dist als Funktion

class ScTDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fDF;

public:
            ScTDistFunction( ScInterpreter& rI, double fpVal, double fDFVal ) :
                rInt(rI), fp(fpVal), fDF(fDFVal) {}

    double  GetValue( double x ) const  { return fp - 2 * rInt.GetTDist(x, fDF); }
};

void ScInterpreter::ScTInv()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = SolarMath::ApproxFloor(GetDouble());
    double fP = GetDouble();
    if (fDF < 1.0 || fDF >= 1.0E5 || fP <= 0.0 || fP > 1.0 )
    {
        SetIllegalArgument();
        return;
    }

    BOOL bConvError;
    ScTDistFunction aFunc( *this, fP, fDF );
    double fVal = lcl_IterateInverse( aFunc, fDF*0.5, fDF, bConvError );
    if (bConvError)
        SetError(errNoConvergence);
    PushDouble(fVal);
}

class ScFDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fF1, fF2;

public:
            ScFDistFunction( ScInterpreter& rI, double fpVal, double fF1Val, double fF2Val ) :
                rInt(rI), fp(fpVal), fF1(fF1Val), fF2(fF2Val) {}

    double  GetValue( double x ) const  { return fp - rInt.GetFDist(x, fF1, fF2); }
};

void ScInterpreter::ScFInv()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    double fF2 = SolarMath::ApproxFloor(GetDouble());
    double fF1 = SolarMath::ApproxFloor(GetDouble());
    double fP  = GetDouble();
    if (fP <= 0.0 || fF1 < 1.0 || fF2 < 1.0 || fF1 >= 1.0E10 || fF2 >= 1.0E10 || fP > 1.0)
    {
        SetIllegalArgument();
        return;
    }

    BOOL bConvError;
    ScFDistFunction aFunc( *this, fP, fF1, fF2 );
    double fVal = lcl_IterateInverse( aFunc, fF1*0.5, fF1, bConvError );
    if (bConvError)
        SetError(errNoConvergence);
    PushDouble(fVal);
}

class ScChiDistFunction : public ScDistFunc
{
    ScInterpreter&  rInt;
    double          fp, fDF;

public:
            ScChiDistFunction( ScInterpreter& rI, double fpVal, double fDFVal ) :
                rInt(rI), fp(fpVal), fDF(fDFVal) {}

    double  GetValue( double x ) const  { return fp - rInt.GetChiDist(x, fDF); }
};

void ScInterpreter::ScChiInv()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fDF  = SolarMath::ApproxFloor(GetDouble());
    double fP = GetDouble();
    if (fDF < 1.0 || fDF >= 1.0E5 || fP <= 0.0 || fP > 1.0 )
    {
        SetIllegalArgument();
        return;
    }

    BOOL bConvError;
    ScChiDistFunction aFunc( *this, fP, fDF );
    double fVal = lcl_IterateInverse( aFunc, fDF*0.5, fDF, bConvError );
    if (bConvError)
        SetError(errNoConvergence);
    PushDouble(fVal);
}

/***********************************************/

void ScInterpreter::ScConfidence()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double n     = SolarMath::ApproxFloor(GetDouble());
        double sigma = GetDouble();
        double alpha = GetDouble();
        if (sigma <= 0.0 || alpha <= 0.0 || alpha >= 1.0 || n < 1.0)
            SetIllegalArgument();
        else
            PushDouble( gaussinv(1.0-alpha/2.0) * sigma/sqrt(n) );
    }
}

void ScInterpreter::ScZTest()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    double sigma, mue, x;
    if (nParamCount == 3)
    {
        sigma = GetDouble();
        if (sigma <= 0.0)
        {
            SetIllegalArgument();
            return;
        }
    }
    x = GetDouble();

    double fSum    = 0.0;
    double fSumSqr = 0.0;
    double fVal;
    double rValCount = 0.0;
    switch (GetStackType())
    {
        case svDouble :
        {
            fVal = GetDouble();
            fSum    += fVal;
            fSumSqr += fVal*fVal;
            rValCount++;
        }
        break;
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            ScBaseCell* pCell = GetCell( aAdr );
            if (HasCellValueData(pCell))
            {
                fVal = GetCellValue( aAdr, pCell );
                fSum += fVal;
                fSumSqr += fVal*fVal;
                rValCount++;
            }
        }
        break;
        case svDoubleRef :
        {
            ScRange aRange;
            USHORT nErr = 0;
            PopDoubleRef( aRange );
            ScValueIterator aValIter(pDok, aRange, glSubTotal);
            if (aValIter.GetFirst(fVal, nErr))
            {
                fSum += fVal;
                fSumSqr += fVal*fVal;
                rValCount++;
                while ((nErr == 0) && aValIter.GetNext(fVal, nErr))
                {
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    rValCount++;
                }
                SetError(nErr);
            }
        }
        break;
        case svMatrix :
        {
            ScMatrix* pMat = PopMatrix();
            if (pMat)
            {
                USHORT nC, nR;
                pMat->GetDimensions(nC, nR);
                if (pMat->IsNumeric())
                {
                    ULONG nCount = (ULONG) nC * nR;
                    for ( ULONG i = 0; i < nCount; i++ )
                    {
                        fVal= pMat->GetDouble(i);
                        fSum += fVal;
                        fSumSqr += fVal * fVal;
                        rValCount++;
                    }
                }
                else
                {
                    for (USHORT i = 0; i < nC; i++)
                        for (USHORT j = 0; j < nR; j++)
                            if (!pMat->IsString(i,j))
                            {
                                fVal= pMat->GetDouble(i,j);
                                fSum += fVal;
                                fSumSqr += fVal * fVal;
                                rValCount++;
                            }
                }
            }
        }
        break;
        default : SetError(errIllegalParameter); break;
    }
    if (rValCount <= 1.0)
        SetNoValue();
    else
    {
        mue = fSum/rValCount;
        if (nParamCount != 3)
            sigma = (fSumSqr - fSum*fSum/rValCount)/(rValCount-1.0);

        PushDouble(0.5 - gauss((mue-x)/sqrt(sigma/rValCount)));
    }
}

void ScInterpreter::ScTTest()
{
    if ( !MustHaveParamCount( GetByte(), 4 ) )
        return;
    double fTyp = SolarMath::ApproxFloor(GetDouble());
    double fAnz = SolarMath::ApproxFloor(GetDouble());
    if (fAnz != 1.0 && fAnz != 2.0)
    {
        SetIllegalArgument();
        return;
    }

    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat2 = GetMatrix(nMatInd2);
    ScMatrix* pMat1 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    double fT, fF;
    USHORT nC1, nR1, nC2, nR2, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (fTyp == 1.0)
    {
        if (nC1 != nC2 || nR1 != nR2)
        {
            SetIllegalParameter();
            return;
        }
        double fCount   = 0.0;
        double fSum1    = 0.0;
        double fSum2    = 0.0;
        double fSumSqrD = 0.0;
        double fVal1, fVal2;
        for (i = 0; i < nC1; i++)
            for (j = 0; j < nR1; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                {
                    fVal1 = pMat1->GetDouble(i,j);
                    fVal2 = pMat2->GetDouble(i,j);
                    fSum1    += fVal1;
                    fSum2    += fVal2;
                    fSumSqrD += (fVal1 - fVal2)*(fVal1 - fVal2);
                    fCount++;
                }
            }
        if (fCount < 1.0)
        {
            SetNoValue();
            return;
        }
        fT = sqrt(fCount-1.0) * fabs(fSum1 - fSum2) /
             sqrt(fCount * fSumSqrD - (fSum1-fSum2)*(fSum1-fSum2));
        fF = fCount - 1.0;
    }
    else if (fTyp == 2.0)
    {
        double fCount1  = 0.0;
        double fCount2  = 0.0;
        double fSum1    = 0.0;
        double fSumSqr1 = 0.0;
        double fSum2    = 0.0;
        double fSumSqr2 = 0.0;
        double fVal;
        for (i = 0; i < nC1; i++)
            for (j = 0; j < nR1; j++)
            {
                if (!pMat1->IsString(i,j))
                {
                    fVal = pMat1->GetDouble(i,j);
                    fSum1    += fVal;
                    fSumSqr1 += fVal * fVal;
                    fCount1++;
                }
            }
        for (i = 0; i < nC2; i++)
            for (j = 0; j < nR2; j++)
            {
                if (!pMat2->IsString(i,j))
                {
                    fVal = pMat2->GetDouble(i,j);
                    fSum2    += fVal;
                    fSumSqr2 += fVal * fVal;
                    fCount2++;
                }
            }
        if (fCount1 < 2.0 || fCount2 < 2.0)
        {
            SetNoValue();
            return;
        }
#if 0
        //  alter Templin-Code
        double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)/(fCount1-1.0)/fCount1;
        double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)/(fCount2-1.0)/fCount2;
        if (fS1 + fS2 == 0.0)
        {
            SetNoValue();
            return;
        }
        fT = fabs(fSum1/fCount1 - fSum2/fCount2)/sqrt(fS1+fS2);
        fF = fCount1 + fCount2 - 2;
#else
        //  laut Bronstein-Semendjajew
        double fS1 = (fSumSqr1 - fSum1*fSum1/fCount1) / (fCount1 - 1.0);    // Varianz
        double fS2 = (fSumSqr2 - fSum2*fSum2/fCount2) / (fCount2 - 1.0);
        fT = fabs( fSum1/fCount1 - fSum2/fCount2 ) /
             sqrt( (fCount1-1.0)*fS1 + (fCount2-1.0)*fS2 ) *
             sqrt( fCount1*fCount2*(fCount1+fCount2-2)/(fCount1+fCount2) );
        fF = fCount1 + fCount2 - 2;
#endif
    }
    else if (fTyp == 3.0)
    {
        double fCount1  = 0.0;
        double fCount2  = 0.0;
        double fSum1    = 0.0;
        double fSumSqr1 = 0.0;
        double fSum2    = 0.0;
        double fSumSqr2 = 0.0;
        double fVal;
        for (i = 0; i < nC1; i++)
            for (j = 0; j < nR1; j++)
            {
                if (!pMat1->IsString(i,j))
                {
                    fVal = pMat1->GetDouble(i,j);
                    fSum1    += fVal;
                    fSumSqr1 += fVal * fVal;
                    fCount1++;
                }
            }
        for (i = 0; i < nC2; i++)
            for (j = 0; j < nR2; j++)
            {
                if (!pMat2->IsString(i,j))
                {
                    fVal = pMat2->GetDouble(i,j);
                    fSum2    += fVal;
                    fSumSqr2 += fVal * fVal;
                    fCount2++;
                }
            }
        if (fCount1 < 2.0 || fCount2 < 2.0)
        {
            SetNoValue();
            return;
        }
        double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)/(fCount1-1.0)/fCount1;
        double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)/(fCount2-1.0)/fCount2;
        if (fS1 + fS2 == 0.0)
        {
            SetNoValue();
            return;
        }
        fT = fabs(fSum1/fCount1 - fSum2/fCount2)/sqrt(fS1+fS2);
        double c = fS1/(fS1+fS2);
// s.u. fF = SolarMath::ApproxFloor(1.0/(c*c/(fCount1-1.0)+(1.0-c)*(1.0-c)/(fCount2-1.0)));
//      fF = SolarMath::ApproxFloor((fS1+fS2)*(fS1+fS2)/(fS1*fS1/(fCount1-1.0) + fS2*fS2/(fCount2-1.0)));

        //  GetTDist wird mit GetBetaDist berechnet und kommt auch mit nicht ganzzahligen
        //  Freiheitsgraden klar. Dann stimmt das Ergebnis auch mit Excel ueberein (#52406#):
        fF = 1.0/(c*c/(fCount1-1.0)+(1.0-c)*(1.0-c)/(fCount2-1.0));
    }

    else
    {
        SetIllegalArgument();
        return;
    }
    if (fAnz == 1.0)
        PushDouble(GetTDist(fT, fF));
    else
        PushDouble(2.0*GetTDist(fT, fF));
}

void ScInterpreter::ScFTest()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat2 = GetMatrix(nMatInd2);
    ScMatrix* pMat1 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    double fCount1  = 0.0;
    double fCount2  = 0.0;
    double fSum1    = 0.0;
    double fSumSqr1 = 0.0;
    double fSum2    = 0.0;
    double fSumSqr2 = 0.0;
    double fVal;
    for (i = 0; i < nC1; i++)
        for (j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j))
            {
                fVal = pMat1->GetDouble(i,j);
                fSum1    += fVal;
                fSumSqr1 += fVal * fVal;
                fCount1++;
            }
        }
    for (i = 0; i < nC2; i++)
        for (j = 0; j < nR2; j++)
        {
            if (!pMat2->IsString(i,j))
            {
                fVal = pMat2->GetDouble(i,j);
                fSum2    += fVal;
                fSumSqr2 += fVal * fVal;
                fCount2++;
            }
        }
    if (fCount1 < 2.0 || fCount2 < 2.0)
    {
        SetNoValue();
        return;
    }
    double fS1 = (fSumSqr1-fSum1*fSum1/fCount1)/(fCount1-1.0);
    double fS2 = (fSumSqr2-fSum2*fSum2/fCount2)/(fCount2-1.0);
    if (fS1 == 0.0 || fS2 == 0.0)
    {
        SetNoValue();
        return;
    }
    double fF, fF1, fF2;
    if (fS1 > fS2)
    {
        fF = fS1/fS2;
        fF1 = fCount1-1.0;
        fF2 = fCount2-1.0;
    }
    else
    {
        fF = fS2/fS1;
        fF1 = fCount2-1.0;
        fF2 = fCount1-1.0;
    }
    PushDouble(2.0*GetFDist(fF, fF1, fF2));
/*
    double Z = (pow(fF,1.0/3.0)*(1.0-2.0/(9.0*fF2)) - (1.0-2.0/(9.0*fF1))) /
               sqrt(2.0/(9.0*fF1) + pow(fF,2.0/3.0)*2.0/(9.0*fF2));
    PushDouble(1.0-2.0*gauss(Z));
*/
}

void ScInterpreter::ScChiTest()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat2 = GetMatrix(nMatInd2);
    ScMatrix* pMat1 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fChi = 0.0;
    USHORT i, j;
    double fValX, fValE;
    for (i = 0; i < nC1; i++)
        for (j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValE = pMat2->GetDouble(i,j);
                fChi += (fValX-fValE)*(fValX-fValE)/fValE;
            }
            else
            {
                SetIllegalArgument();
                return;
            }
        }
    double fDF;
    if (nC1 == 1 || nR1 == 1)
    {
        fDF = (double)(nC1*nR1 - 1);
        if (fDF == 0.0)
        {
            SetNoValue();
            return;
        }
    }
    else
        fDF = (double)(nC1-1)*(double)(nR1-1);
    PushDouble(GetChiDist(fChi, fDF));
/*
    double fX, fS, fT, fG;
    fX = 1.0;
    for (double fi = fDF; fi >= 2.0; fi -= 2.0)
        fX *= fChi/fi;
    fX *= exp(-fChi/2.0);
    if (fmod(fDF, 2.0) != 0.0)
        fX *= sqrt(2.0*fChi/F_PI);
    fS = 1.0;
    fT = 1.0;
    fG = fDF;
    while (fT >= 1.0E-7)
    {
        fG += 2.0;
        fT *= fChi/fG;
        fS += fT;
    }
    PushDouble(1.0 - fX*fS);
*/
}

void ScInterpreter::ScKurt()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    USHORT SaveSP = sp;
    USHORT i;
    double fSum    = 0.0;
    double fSumSqr = 0.0;
    double fCount  = 0.0;
    double fVal;
    ScAddress aAdr;
    ScRange aRange;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                fSum += fVal;
                fSumSqr += fVal*fVal;
                fCount++;
            }
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    fVal = GetCellValue( aAdr, pCell );
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    fCount++;
                }
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr = 0;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    fCount++;
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(fVal, nErr))
                    {
                        fSum += fVal;
                        fSumSqr += fVal*fVal;
                        fCount++;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                fVal = pMat->GetDouble(i,j);
                                fSum += fVal;
                                fSumSqr += fVal*fVal;
                                fCount++;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                {
                                    fVal = pMat->GetDouble(i,j);
                                    fSum += fVal;
                                    fSumSqr += fVal*fVal;
                                    fCount++;
                                }
                    }
                }
            }
            break;
            default :
                SetError(errIllegalParameter);
            break;
        }
    }
    if (nGlobalError)
    {
        PushInt(0);
        return;
    }
    double fMean = fSum / fCount;
    double fSSqr = (fSumSqr - fSum*fSum/fCount)/(fCount-1.0);
    sp = SaveSP;
    fSum = 0.0;
    // #55733# GCC Optimierungsfehler, GPF wenn die 4.0 als Konstante an pow()
    // uebergeben wird, auch ein "const double fPow = 4.0;" GPF't,
    double fPow = 4.0;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                fSum += pow(GetDouble()-fMean,fPow);
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                    fSum += pow(GetCellValue( aAdr, pCell ) - fMean, fPow);
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr = 0;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += pow(fVal - fMean, fPow);
                    while (aValIter.GetNext(fVal, nErr))
                         fSum += pow(fVal - fMean, fPow);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                fSum += pow(pMat->GetDouble(i,j) - fMean, fPow);
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                    fSum += pow(pMat->GetDouble(i,j) - fMean, fPow);
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    PushDouble(fCount*(fCount+1.0)/((fCount-1.0)*(fCount-2.0)*(fCount-3.0))
               *fSum/(fSSqr*fSSqr)
               - 3.0*(fCount-1.0)*(fCount-1.0)/((fCount-2.0)*(fCount-3.0)));
}

void ScInterpreter::ScHarMean()
{
    BYTE nParamCount = GetByte();
    double nVal = 0.0;
    ULONG nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    for (short i = 0; i < nParamCount && (nGlobalError == 0); i++)
    {
        switch (GetStackType())
        {
            case svDouble    :
            {
                double x = GetDouble();
                if (x > 0.0)
                {
                    nVal += 1.0/x;
                    nCount++;
                }
                else
                    SetIllegalArgument();
                break;
            }
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    double x = GetCellValue( aAdr, pCell );
                    if (x > 0.0)
                    {
                        nVal += 1.0/x;
                        nCount++;
                    }
                    else
                        SetIllegalArgument();
                }
                break;
            }
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange, glSubTotal);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    if (nCellVal > 0.0)
                    {
                        nVal += 1.0/nCellVal;
                        nCount++;
                    }
                    else
                        SetIllegalArgument();
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                    {
                        if (nCellVal > 0.0)
                        {
                            nVal += 1.0/nCellVal;
                            nCount++;
                        }
                        else
                            SetIllegalArgument();
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                double x = pMat->GetDouble(i,j);
                                if (x > 0.0)
                                {
                                    nVal += 1.0/x;
                                    nCount++;
                                }
                                else
                                    SetIllegalArgument();
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                {
                                    double x = pMat->GetDouble(i,j);
                                    if (x > 0.0)
                                    {
                                        nVal += 1.0/x;
                                        nCount++;
                                    }
                                    else
                                        SetIllegalArgument();
                                }
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    if (nGlobalError == 0)
        PushDouble((double)nCount/nVal);
}

void ScInterpreter::ScGeoMean()
{
    BYTE nParamCount = GetByte();
    double nVal = 0.0;
    ULONG nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    for (short i = 0; i < nParamCount && (nGlobalError == 0); i++)
    {
        switch (GetStackType())
        {
            case svDouble    :
            {
                double x = GetDouble();
                if (x > 0.0)
                {
                    nVal += log(x);
                    nCount++;
                }
                else
                    SetIllegalArgument();
                break;
            }
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    double x = GetCellValue( aAdr, pCell );
                    if (x > 0.0)
                    {
                        nVal += log(x);
                        nCount++;
                    }
                    else
                        SetIllegalArgument();
                }
                break;
            }
            case svDoubleRef :
            {
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange, glSubTotal);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    if (nCellVal > 0.0)
                    {
                        nVal += log(nCellVal);
                        nCount++;
                    }
                    else
                        SetIllegalArgument();
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                    {
                        if (nCellVal > 0.0)
                        {
                            nVal += log(nCellVal);
                            nCount++;
                        }
                        else
                            SetIllegalArgument();
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                double x = pMat->GetDouble(i,j);
                                if (x > 0.0)
                                {
                                    nVal += log(x);
                                    nCount++;
                                }
                                else
                                    SetIllegalArgument();
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                {
                                    double x = pMat->GetDouble(i,j);
                                    if (x > 0.0)
                                    {
                                        nVal += log(x);
                                        nCount++;
                                    }
                                    else
                                        SetIllegalArgument();
                                }
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    if (nGlobalError == 0)
        PushDouble(exp(nVal/(double)nCount));
}

void ScInterpreter::ScStandard()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double sigma = GetDouble();
        double mue   = GetDouble();
        double x     = GetDouble();
        if (sigma <= 0.0)
            SetIllegalArgument();
        else
            PushDouble((x-mue)/sigma);
    }
}

void ScInterpreter::ScSkew()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 )  )
        return;
    USHORT SaveSP = sp;
    USHORT i;
    double fSum    = 0.0;
    double fSumSqr = 0.0;
    double fCount  = 0.0;
    double fVal;
    ScAddress aAdr;
    ScRange aRange;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
            {
                fVal = GetDouble();
                fSum += fVal;
                fSumSqr += fVal*fVal;
                fCount++;
            }
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    fVal = GetCellValue( aAdr, pCell );
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    fCount++;
                }
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr = 0;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += fVal;
                    fSumSqr += fVal*fVal;
                    fCount++;
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(fVal, nErr))
                    {
                        fSum += fVal;
                        fSumSqr += fVal*fVal;
                        fCount++;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                fVal = pMat->GetDouble(i,j);
                                fSum += fVal;
                                fSumSqr += fVal*fVal;
                                fCount++;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                {
                                    fVal = pMat->GetDouble(i,j);
                                    fSum += fVal;
                                    fSumSqr += fVal*fVal;
                                    fCount++;
                                }
                    }
                }
            }
            break;
            default :
                SetError(errIllegalParameter);
            break;
        }
    }
    if (nGlobalError)
    {
        PushInt(0);
        return;
    }
    double fMean = fSum / fCount;
    double fSSqr = (fSumSqr - fSum*fSum/fCount)/(fCount-1.0);
    sp = SaveSP;
    fSum = 0.0;
    double fPow = 3.0;      // vorsichtshalber wg. #55733#, siehe ScKurt()
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                fSum += pow(GetDouble()-fMean,fPow);
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                    fSum += pow(GetCellValue( aAdr, pCell ) - fMean, fPow);
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr = 0;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(fVal, nErr))
                {
                    fSum += pow(fVal - fMean, fPow);
                    while (aValIter.GetNext(fVal, nErr))
                         fSum += pow(fVal - fMean, fPow);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                fSum += pow(pMat->GetDouble(i,j) - fMean, fPow);
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                    fSum += pow(pMat->GetDouble(i,j) - fMean, fPow);
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    PushDouble(fCount/((fCount-1.0)*(fCount-2.0))*fSum/(fSSqr*sqrt(fSSqr)));
}

void ScInterpreter::ScMedian()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 )  )
        return;
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(nParamCount, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        if (nSize % 2 == 0)
            PushDouble((pSArray[nSize/2-1]+pSArray[nSize/2])/2.0);
        else
            PushDouble(pSArray[(nSize-1)/2]);
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }

}

void ScInterpreter::ScPercentile()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double alpha = GetDouble();
    if (alpha < 0.0 || alpha > 1.0)
    {
        SetIllegalArgument();
        return;
    }
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        if (nSize == 1)
            PushDouble(pSArray[0]);
        else
        {
            ULONG nIndex = (ULONG)SolarMath::ApproxFloor(alpha*(nSize-1));
            double fDiff = alpha*(nSize-1) - SolarMath::ApproxFloor(alpha*(nSize-1));
            DBG_ASSERT(nIndex >= 0 && nIndex < nSize, "ScPercentile: falscher Index (1)");
            if (fDiff == 0.0)
                PushDouble(pSArray[nIndex]);
            else
            {
                DBG_ASSERT(nIndex < nSize-1, "ScPercentile: falscher Index(2)");
                PushDouble(pSArray[nIndex] +
                           fDiff*(pSArray[nIndex+1]-pSArray[nIndex]));
            }
        }
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScQuartile()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double fFlag = SolarMath::ApproxFloor(GetDouble());
    if (fFlag < 0.0 || fFlag > 4.0)
    {
        SetIllegalArgument();
        return;
    }
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        if (nSize == 1)
            PushDouble(pSArray[0]);
        else
        {
            if (fFlag == 0.0)
                PushDouble(pSArray[0]);
            else if (fFlag == 1.0)
            {
                ULONG nIndex = (ULONG)SolarMath::ApproxFloor(0.25*(nSize-1));
                double fDiff = 0.25*(nSize-1) - SolarMath::ApproxFloor(0.25*(nSize-1));
                DBG_ASSERT(nIndex >= 0 && nIndex < nSize, "ScQuartile: falscher Index (1)");
                if (fDiff == 0.0)
                    PushDouble(pSArray[nIndex]);
                else
                {
                    DBG_ASSERT(nIndex < nSize-1, "ScQuartile: falscher Index(2)");
                    PushDouble(pSArray[nIndex] +
                               fDiff*(pSArray[nIndex+1]-pSArray[nIndex]));
                }
            }
            else if (fFlag == 2.0)
            {
                if (nSize % 2 == 0)
                    PushDouble((pSArray[nSize/2-1]+pSArray[nSize/2])/2.0);
                else
                    PushDouble(pSArray[(nSize-1)/2]);
            }
            else if (fFlag == 3.0)
            {
                ULONG nIndex = (ULONG)SolarMath::ApproxFloor(0.75*(nSize-1));
                double fDiff = 0.75*(nSize-1) - SolarMath::ApproxFloor(0.75*(nSize-1));
                DBG_ASSERT(nIndex >= 0 && nIndex < nSize, "ScQuartile: falscher Index (3)");
                if (fDiff == 0.0)
                    PushDouble(pSArray[nIndex]);
                else
                {
                    DBG_ASSERT(nIndex < nSize-1, "ScQuartile: falscher Index(4)");
                    PushDouble(pSArray[nIndex] +
                               fDiff*(pSArray[nIndex+1]-pSArray[nIndex]));
                }
            }
            else
                PushDouble(pSArray[nSize-1]);
        }
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScModalValue()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(nParamCount, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        ULONG nMaxIndex, nMax = 1, nCount = 1;
        double nOldVal = pSArray[0];
        for (ULONG i = 1; i < nSize; i++)
        {
            if (pSArray[i] == nOldVal)
                nCount++;
            else
            {
                nOldVal = pSArray[i];
                if (nCount > nMax)
                {
                    nMax = nCount;
                    nMaxIndex = i-1;
                }
                nCount = 1;
            }
        }
        if (nCount > nMax)
        {
            nMax = nCount;
            nMaxIndex = i-1;
        }
        if (nMax == 1 && nCount == 1)
            SetNoValue();
        else if (nMax == 1)
            PushDouble(nOldVal);
        else
            PushDouble(pSArray[nMaxIndex]);
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScLarge()
{
    if ( !MustHaveParamCount( GetByte(), 2 )  )
        return;
    double* pSortArray = NULL;
    ULONG nSize = 0;
    ULONG k = (ULONG) SolarMath::ApproxFloor(GetDouble());
    if (k <= 0)
    {
        SetIllegalArgument();
        return;
    }
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError || nSize < k)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
/*
        ULONG nCount = 1;
        double nOldVal = pSArray[nSize-1];
        for (long i = nSize-2; i >= 0 && nCount < k; i--)
        {
            if (pSArray[i] != nOldVal)
            {
                nCount++;
                nOldVal = pSArray[i];
            }
        }
        if (nCount < k)
            SetNoValue();
        else
            PushDouble(nOldVal);
*/
        PushDouble( pSArray[ nSize-k ] );
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScSmall()
{
    if ( !MustHaveParamCount( GetByte(), 2 )  )
        return;
    double* pSortArray = NULL;
    ULONG nSize = 0;
    ULONG k = (ULONG) SolarMath::ApproxFloor(GetDouble());
    if (k <= 0)
    {
        SetIllegalArgument();
        return;
    }
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError || nSize < k)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
/*
        ULONG nCount = 1;
        double nOldVal = pSArray[0];
        for (ULONG i = 1; i < nSize && nCount < k; i++)
        {
            if (pSArray[i] != nOldVal)
            {
                nCount++;
                nOldVal = pSArray[i];
            }
        }
        if (nCount < k)
            SetNoValue();
        else
            PushDouble(nOldVal);
*/
        PushDouble( pSArray[ k-1 ] );
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScPercentrank()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2 ) )
        return;
/*                          wird nicht unterstuetzt
    double fPrec;
    if (nParamCount == 3)
    {
        fPrec = SolarMath::ApproxFloor(GetDouble());
        if (fPrec < 1.0)
        {
            SetIllegalArgument();
            return;
        }
    }
    else
        fPrec = 3.0;
*/
    double fNum = GetDouble();
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        if (fNum < pSArray[0] || fNum > pSArray[nSize-1])
            SetNoValue();
        else if ( nSize == 1 )
            PushDouble(1.0);            // fNum == pSArray[0], see test above
        else
        {
            double fRes;
            ULONG nOldCount = 0;
            double fOldVal = pSArray[0];
            ULONG i;
            for (i = 1; i < nSize && pSArray[i] < fNum; i++)
            {
                if (pSArray[i] != fOldVal)
                {
                    nOldCount = i;
                    fOldVal = pSArray[i];
                }
            }
            if (pSArray[i] != fOldVal)
                nOldCount = i;
            if (fNum == pSArray[i])
                fRes = (double)nOldCount/(double)(nSize-1);
            else
            {
                //  #75312# nOldCount is the count of smaller entries
                //  fNum is between pSArray[nOldCount-1] and pSArray[nOldCount]
                //  use linear interpolation to find a position between the entries

                if ( nOldCount == 0 )
                {
                    DBG_ERROR("should not happen");
                    fRes = 0.0;
                }
                else
                {
                    double fFract = ( fNum - pSArray[nOldCount-1] ) /
                                    ( pSArray[nOldCount] - pSArray[nOldCount-1] );
                    fRes = ( (double)(nOldCount-1)+fFract )/(double)(nSize-1);
                }
            }
            PushDouble(fRes);
        }
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
    }
}

void ScInterpreter::ScTrimMean()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    double alpha = GetDouble();
    if (alpha < 0.0 || alpha >= 1.0)
    {
        SetIllegalArgument();
        return;
    }
    double* pSortArray = NULL;
    ULONG nSize = 0;
    GetSortArray(1, &pSortArray, nSize);
    if (!pSortArray || nSize == 0 || nGlobalError)
        SetNoValue();
    else
    {
        ppGlobSortArray = &pSortArray;
#ifdef WIN
        double huge* pSArray = (double huge*) pSortArray;
#else
        double* pSArray = pSortArray;
#endif
        ULONG nIndex = (ULONG) SolarMath::ApproxFloor(alpha*(double)nSize);
        if (nIndex % 2 != 0)
            nIndex--;
        nIndex /= 2;
        DBG_ASSERT(nIndex >= 0 && nIndex < nSize, "ScTrimMean: falscher Index");
        double fSum = 0.0;
        for (ULONG i = nIndex; i < nSize-nIndex; i++)
            fSum += pSArray[i];
        PushDouble(fSum/(double)(nSize-2*nIndex));
    }
    if (pSortArray)
    {
#ifdef WIN
        SvMemFree(pSortArray);
#else
        delete [] pSortArray;
#endif
        ppGlobSortArray = NULL;
    }
}

void ScInterpreter::GetSortArray(BYTE nParamCount, double** ppSortArray, ULONG& nSize)
{
    *ppSortArray = NULL;
    nSize = 0;

    USHORT SaveSP = sp;
    USHORT i;
    ULONG rValCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                rValCount++;
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                    rValCount++;
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr = 0;
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    rValCount++;
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                        rValCount++;
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                rValCount++;
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                    rValCount++;
                    }
                }
            }
            break;
            default :
                SetError(errIllegalParameter);
            break;
        }
    }
    if (rValCount > MAX_ANZ_DOUBLE_FOR_SORT || nGlobalError)
    {
        SetError(errStackOverflow);
        return;
    }
    else if (rValCount == 0)
    {
        SetNoValue();
        return;
    }
#ifdef WIN
    *ppSortArray = (double*) SvMemAlloc( rValCount * sizeof(double));
    double huge* pSArray = (double huge*) (*ppSortArray);
#else
    *ppSortArray = new double[rValCount];
    double* pSArray = *ppSortArray;
#endif
    if (!*ppSortArray)
    {
        rValCount = 0;
        SetError(errStackOverflow);
        return;
    }
    sp = SaveSP;
    ULONG nIndex = 0;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                pSArray[nIndex] = GetDouble();
                nIndex++;
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    pSArray[nIndex] = GetCellValue( aAdr, pCell );
                    nIndex++;
                }
            }
            break;
            case svDoubleRef :
            {
                PopDoubleRef( aRange );
                USHORT nErr;
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    pSArray[nIndex] = nCellVal;
                    nIndex++;
                    while (aValIter.GetNext(nCellVal, nErr))
                    {
                        pSArray[nIndex] = nCellVal;
                        nIndex++;
                    }
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                pSArray[nIndex] = pMat->GetDouble(i,j);
                                nIndex++;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                            {
                                pSArray[nIndex] = pMat->GetDouble(i,j);
                                nIndex++;
                            }
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    DBG_ASSERT(nIndex == rValCount,"nIndex != rValCount");
    if (nGlobalError == 0)
    {
        double fVal;
        USHORT nInd;
        for (ULONG i = 0; (i + 4) <= rValCount-1; i += 4)
        {
            nInd = rand() % (int) (rValCount-1);
            fVal = pSArray[i];
            pSArray[i] = pSArray[nInd];
            pSArray[nInd] = fVal;
        }
        QuickSort(0, rValCount-1, pSArray);
    }
    nSize = rValCount;
}

void ScInterpreter::QuickSort(long nLo, long nHi, double* pSortArray)
{
#ifdef WIN
    double huge* pSArray = (double huge*) pSortArray;
#else
    double* pSArray = pSortArray;
#endif
    if (nHi - nLo == 1)
    {
        if (pSArray[nLo] > pSArray[nHi])
        {
            double fVal;
            fVal = pSArray[nLo];
            pSArray[nLo] = pSArray[nHi];
            pSArray[nHi] = fVal;
        }
    }
    else
    {
        long ni = nLo;
        long nj = nHi;
        do
        {
            while (ni <= nHi && pSArray[ni]  < pSArray[nLo]) ni++;
            while (nj >= nLo && pSArray[nLo] < pSArray[nj])  nj--;
            if (ni <= nj)
            {
                if (ni != nj)
                {
                    double fVal;
                    fVal = pSArray[ni];
                    pSArray[ni] = pSArray[nj];
                    pSArray[nj] = fVal;
                }
                ni++;
                nj--;
            }
        }
        while (ni < nj);
        if ((nj - nLo) < (nHi - ni))
        {
            if (nLo < nj) QuickSort(nLo, nj, pSortArray);
            if (ni < nHi) QuickSort(ni, nHi, pSortArray);
        }
        else
        {
            if (ni < nHi) QuickSort(ni, nHi, pSortArray);
            if (nLo < nj) QuickSort(nLo, nj, pSortArray);
        }
    }
}

void ScInterpreter::ScRank()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 2, 3 ) )
        return;
    BOOL bDescending;
    if (nParamCount == 3)
        bDescending = GetBool();
    else
        bDescending = FALSE;
    double fCount = 1.0;
    BOOL bValid = FALSE;
    switch (GetStackType())
    {
        case svDouble    :
        {
            double x = GetDouble();
            double fVal = GetDouble();
            if (x == fVal)
                bValid = TRUE;
            break;
        }
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            double fVal = GetDouble();
            ScBaseCell* pCell = GetCell( aAdr );
            if (HasCellValueData(pCell))
            {
                double x = GetCellValue( aAdr, pCell );
                if (x == fVal)
                    bValid = TRUE;
            }
            break;
        }
        case svDoubleRef :
        {
            ScRange aRange;
            USHORT nErr = 0;
            PopDoubleRef( aRange );
            double fVal = GetDouble();
            double nCellVal;
            ScValueIterator aValIter(pDok, aRange, glSubTotal);
            if (aValIter.GetFirst(nCellVal, nErr))
            {
                if (nCellVal == fVal)
                    bValid = TRUE;
                else if ((!bDescending && nCellVal > fVal) ||
                         (bDescending && nCellVal < fVal) )
                    fCount++;
                SetError(nErr);
                while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                {
                    if (nCellVal == fVal)
                        bValid = TRUE;
                    else if ((!bDescending && nCellVal > fVal) ||
                             (bDescending && nCellVal < fVal) )
                        fCount++;
                }
            }
            SetError(nErr);
        }
        break;
        case svMatrix :
        {
            ScMatrix* pMat = PopMatrix();
            double fVal = GetDouble();
            if (pMat)
            {
                USHORT nC, nR;
                pMat->GetDimensions(nC, nR);
                if (pMat->IsNumeric())
                {
                    for (USHORT i = 0; i < nC; i++)
                        for (USHORT j = 0; j < nR; j++)
                        {
                            double x = pMat->GetDouble(i,j);
                            if (x == fVal)
                                bValid = TRUE;
                            else if ((!bDescending && x > fVal) ||
                                     (bDescending && x < fVal) )
                                fCount++;
                        }
                }
                else
                {
                    for (USHORT i = 0; i < nC; i++)
                        for (USHORT j = 0; j < nR; j++)
                            if (!pMat->IsString(i,j))
                            {
                                double x = pMat->GetDouble(i,j);
                                if (x == fVal)
                                    bValid = TRUE;
                                else if ((!bDescending && x > fVal) ||
                                         (bDescending && x < fVal) )
                                    fCount++;
                            }
                }
            }
        }
        break;
        default : SetError(errIllegalParameter); break;
    }
    if (bValid)
        PushDouble(fCount);
    else
        SetNoValue();
}

void ScInterpreter::ScAveDev()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1 ) )
        return;
    USHORT SaveSP = sp;
    USHORT i;
    double nMiddle = 0.0;
    double rVal = 0.0;
    double rValCount = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                rVal += GetDouble();
                rValCount++;
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                {
                    rVal += GetCellValue( aAdr, pCell );
                    rValCount++;
                }
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                double nCellVal;
                PopDoubleRef( aRange );
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    rVal += nCellVal;
                    rValCount++;
                    SetError(nErr);
                    while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                    {
                        rVal += nCellVal;
                        rValCount++;
                    }
                    SetError(nErr);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                            {
                                rVal += pMat->GetDouble(i,j);
                                rValCount++;
                            }
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                {
                                    rVal += pMat->GetDouble(i,j);
                                    rValCount++;
                                }
                    }
                }
            }
            break;
            default :
                SetError(errIllegalParameter);
            break;
        }
    }
    if (nGlobalError)
    {
        PushInt(0);
        return;
    }
    nMiddle = rVal / rValCount;
    sp = SaveSP;
    rVal = 0.0;
    for (i = 0; i < nParamCount; i++)
    {
        switch (GetStackType())
        {
            case svDouble :
                rVal += fabs(GetDouble() - nMiddle);
                break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellValueData(pCell))
                    rVal += fabs(GetCellValue( aAdr, pCell ) - nMiddle);
            }
            break;
            case svDoubleRef :
            {
                USHORT nErr = 0;
                double nCellVal;
                PopDoubleRef( aRange );
                ScValueIterator aValIter(pDok, aRange);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    rVal += (fabs(nCellVal - nMiddle));
                    while (aValIter.GetNext(nCellVal, nErr))
                         rVal += fabs(nCellVal - nMiddle);
                }
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (pMat->IsNumeric())
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                rVal += fabs(pMat->GetDouble(i,j) - nMiddle);
                    }
                    else
                    {
                        for (USHORT i = 0; i < nC; i++)
                            for (USHORT j = 0; j < nR; j++)
                                if (!pMat->IsString(i,j))
                                    rVal += fabs(pMat->GetDouble(i,j) - nMiddle);
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
    }
    PushDouble(rVal / rValCount);
}

void ScInterpreter::ScDevSq()
{
    double nVal;
    double nValCount;
    GetStVarParams(nVal, nValCount);
    PushDouble(nVal);
}

void ScInterpreter::ScProbability()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 4 ) )
        return;
    double fUp, fLo;
    fUp = GetDouble();
    if (nParamCount == 4)
        fLo = GetDouble();
    else
        fLo = fUp;
    if (fLo > fUp)
    {
        double fTemp = fLo;
        fLo = fUp;
        fUp = fTemp;
    }
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMatP = GetMatrix(nMatInd1);
    ScMatrix* pMatW = GetMatrix(nMatInd2);
    if (!pMatP || !pMatW)
        SetIllegalParameter();
    else
    {
        USHORT nC1, nR1, nC2, nR2;
        pMatP->GetDimensions(nC1, nR1);
        pMatW->GetDimensions(nC2, nR2);
        if (nC1 != nC2 || nR1 != nR2 || nC1 == 0 || nR1 == 0 ||
            nC2 == 0 || nR2 == 0)
            SetNV();
        else
        {
            double fSum = 0.0;
            double fRes = 0.0;
            BOOL bStop = FALSE;
            double fP, fW;
            ULONG nCount1 = (ULONG) nC1 * nR1;
            for ( ULONG i = 0; i < nCount1 && !bStop; i++ )
            {
                if (pMatP->IsValue(i) && pMatW->IsValue(i))
                {
                    fP = pMatP->GetDouble(i);
                    fW = pMatW->GetDouble(i);
                    if (fP < 0.0 || fP > 1.0)
                        bStop = TRUE;
                    else
                    {
                        fSum += fP;
                        if (fW >= fLo && fW <= fUp)
                            fRes += fP;
                    }
                }
                else
                    SetIllegalArgument();
            }
            if (bStop || fabs(fSum -1.0) > 1.0E-7)
                SetNoValue();
            else
                PushDouble(fRes);
        }
    }
}

void ScInterpreter::ScCorrel()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumSqrY = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 2.0)
        SetNoValue();
    else
        PushDouble( (fSumXY-fSumX*fSumY/fCount)/
                     sqrt((fSumSqrX-fSumX*fSumX/fCount)*
                          (fSumSqrY-fSumY*fSumY/fCount)));
}

void ScInterpreter::ScCovar()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumY    = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumY    += fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 1.0)
        SetNoValue();
    else
        PushDouble( (fSumXY-fSumX*fSumY/fCount)/fCount);
}

void ScInterpreter::ScPearson()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumSqrY = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 2.0)
        SetNoValue();
    else
        PushDouble( (fCount*fSumXY-fSumX*fSumY)/
                     sqrt((fCount*fSumSqrX-fSumX*fSumX)*
                          (fCount*fSumSqrY-fSumY*fSumY)));
}

void ScInterpreter::ScRSQ()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumSqrY = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 2.0)
        SetNoValue();
    else
        PushDouble( (fCount*fSumXY-fSumX*fSumY)*(fCount*fSumXY-fSumX*fSumY)/
                     (fCount*fSumSqrX-fSumX*fSumX)/(fCount*fSumSqrY-fSumY*fSumY));
}

void ScInterpreter::ScSTEXY()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumSqrY = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 3.0)
        SetNoValue();
    else
        PushDouble(sqrt((fCount*fSumSqrY - fSumY*fSumY -
        (fCount*fSumXY -fSumX*fSumY)*(fCount*fSumXY -fSumX*fSumY)/
        (fCount*fSumSqrX-fSumX*fSumX) )/(fCount*(fCount-2.0))));
}

void ScInterpreter::ScSlope()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 1.0)
        SetNoValue();
    else
        PushDouble( (fCount*fSumXY-fSumX*fSumY)/
                    (fCount*fSumSqrX-fSumX*fSumX) );
}

void ScInterpreter::ScIntercept()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 1.0)
        SetNoValue();
    else
        PushDouble( fSumY/fCount - (fCount*fSumXY-fSumX*fSumY)/
                    (fCount*fSumSqrX-fSumX*fSumX)*fSumX/fCount );

}

void ScInterpreter::ScForecast()
{
    if ( !MustHaveParamCount( GetByte(), 3 ) )
        return;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMat1 = GetMatrix(nMatInd2);
    ScMatrix* pMat2 = GetMatrix(nMatInd1);
    if (!pMat1 || !pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC1, nR1, nC2, nR2;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nR1 != nR2 || nC1 != nC2)
    {
        SetIllegalParameter();
        return;
    }
    double fVal = GetDouble();
    double fCount   = 0.0;
    double fSumX    = 0.0;
    double fSumSqrX = 0.0;
    double fSumY    = 0.0;
    double fSumXY   = 0.0;
    double fValX, fValY;
    for (USHORT i = 0; i < nC1; i++)
        for (USHORT j = 0; j < nR1; j++)
        {
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fValX = pMat1->GetDouble(i,j);
                fValY = pMat2->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        }
    if (fCount < 1.0)
        SetNoValue();
    else
        PushDouble( fSumY/fCount + (fCount*fSumXY-fSumX*fSumY)/
                    (fCount*fSumSqrX-fSumX*fSumX) * (fVal - fSumX/fCount) );
}




