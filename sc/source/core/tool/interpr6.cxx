/*************************************************************************
 *
 *  $RCSfile: interpr6.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
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


#include <math.h>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "interpre.hxx"


//! #66556# for os2icci3 this function MUST be compiled without optimizations,
//! otherwise it won't work at all or even worse will produce false results!
double ScInterpreter::GetGammaDist(double x, double alpha, double beta)
{
    if (x == 0.0)
        return 0.0;

    x /= beta;
    double gamma = alpha;

    double c = 0.918938533204672741;
    double d[10] = {
        0.833333333333333333E-1,
        -0.277777777777777778E-2,
        0.793650793650793651E-3,
        -0.595238095238095238E-3,
        0.841750841750841751E-3,
        -0.191752691752691753E-2,
        0.641025641025641025E-2,
        -0.295506535947712418E-1,
        0.179644372368830573,
        -0.139243221690590111E1
    };

    int ipr = 6;

    double dx = x;
    double dgamma = gamma;
    int maxit = 10000;

    double z = dgamma;
    double den = 1.0;
    while ( z < 10.0 )      //! approx?
    {
        den *= z;
        z += 1.0;
    }

    double z2 = z*z;
    double z3 = z*z2;
    double z4 = z2*z2;
    double z5 = z2*z3;
    double a = ( z - 0.5 ) * log(z) - z + c;
    double b = d[0]/z + d[1]/z3 + d[2]/z5 + d[3]/(z2*z5) + d[4]/(z4*z5) +
               d[5]/(z*z5*z5) + d[6]/(z3*z5*z5) + d[7]/(z5*z5*z5) + d[8]/(z2*z5*z5*z5);
    double g = exp(a+b) / den;

    double sum = 1.0 / dgamma;
    double term = 1.0 / dgamma;
    double cut1 = dx - dgamma;
    double cut2 = dx * 10000000000.0;

    for ( int i=1; i<=maxit; i++ )
    {
        double ai = i;
        term = dx * term / ( dgamma + ai );
        sum += term;
        double cutoff = cut1 + ( cut2 * term / sum );
        if ( ai > cutoff )
        {
            double t = sum;
            return pow( dx, dgamma ) * exp( -dx ) * t / g;
        }
    }

    DBG_ERROR("GetGammaDist bricht nicht ab");

    return 1.0;     // should not happen ...
}

#if 0
//! this algorithm doesn't work right in every cases!
double ScInterpreter::GetGammaDist(double x, double alpha, double beta)
{
    if (x == 0.0)
        return 0.0;
    double fEps = 1.0E-6;
    double fGamma;
    double G = GetLogGamma(alpha);
    x /= beta;
    G = alpha*log(x)-x-G;
    if (x <= alpha+1.0)
    {
        if (x < fEps || fabs(G) >= 500.0)
            fGamma = 0.0;
        else
        {
            double fF = 1.0/alpha;
            double fS = fF;
            double anum = alpha;
            for (USHORT i = 0; i < 100; i++)
            {
                anum += 1.0;
                fF *= x/anum;
                fS += fF;
                if (fF < fEps)
                    i = 100;
            }
            fGamma = fS*exp(G);
        }
    }
    else
    {
        if (fabs(G) >= 500.0)
            fGamma = 1.0;
        else
        {
            double a0, b0, a1, b1, cf, fnorm, a2j, a2j1, cfnew;
            a0 = 0.0; b0 = 1.0; a1 = 1.0;
            b1 = x;
            cf = fEps;
            fnorm = 1.0;
            cfnew = 0.0;
            for (USHORT j = 1; j <= 100; j++)
            {
                a2j = ((double) j) - alpha;
                a2j1 = (double) j;
                a0 = (a1+a2j*a0); // *fnorm;
                b0 = (b1+a2j*b0); // *fnorm;
                a1 = (x*a0+a2j1*a1)*fnorm;
                b1 = (x*b0+a2j1*b1)*fnorm;
                if (b1 != 0.0)
                {
                    fnorm = 1.0/b1;
                    cfnew = a1*fnorm;
                    if (fabs(cf-cfnew)/cf < fEps)
                        j = 101;
                    else
                        cf = cfnew;
                }
            }
            fGamma = 1.0 - exp(G)*cfnew;
        }
    }
    return fGamma;
}
#endif


