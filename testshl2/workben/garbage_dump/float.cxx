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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"
#include <stdio.h>
#include <double>
#include <math.h> // fabs()
// #include <sal/types.h>

#define sal_Int16 short
#define SAL_CALL

// LLA: from kernel.h
/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define MIN(x,y) ({ \
    const typeof(x) _x = (x);   \
    const typeof(y) _y = (y);   \
    (void) (&_x == &_y);        \
    _x < _y ? _x : _y; })

#define MAX(x,y) ({ \
    const typeof(x) _x = (x);   \
    const typeof(y) _y = (y);   \
    (void) (&_x == &_y);        \
    _x > _y ? _x : _y; })

//------------------------------------------------------------------------
// testing the method toDouble()
//------------------------------------------------------------------------
template<class T>
sal_Int16 SAL_CALL test_rtl_OString_checkPrecisionSize()
{
    sal_Int16 nSize = sizeof(T);
    T nCalcValue = 1.0;

    // (i + 1) is the current precision
    sal_Int16 i = 0;
    for (i=0;i<50;i++)
    {
        nCalcValue *= 10;
        T nValue = nCalcValue + 0.1;
        T dSub = nValue - nCalcValue;
        // ----- 0.11 ---- 0.1 ---- 0.09 -----
        if (0.11 > dSub && dSub < 0.09)
        {
            // due to the fact, that the value is break down we sub 1 from the precision value
            // but to suppress this, we start at zero, precision is i+1 till here --i;
            break;
        }
    }

    sal_Int16 j= 0;
    nCalcValue = 1.0;
    for (j=0;j<50;j++)
    {
        nCalcValue /= 10;
        T nValue = nCalcValue + 1.0;
        T dSub = nValue - 1.0;
        // ---- 0.02 ----- 0.01 ---- 0 --- -0.99 ---- -0.98 ----
        T dSubAbsolut = fabs(dSub);
        // ---- 0.02 ----- 0.01 ---- 0 (cut)
        if ( dSub == 0)
            break;
    }
    if (i != j)
    {
            // hmmm....
            // imho i +- 1 == j is a good value
            int n = i - j;
            if (n < 0) n = -n;
            if (n <= 1)
            {
                return MIN(i,j);
            }
            else
            {
                printf("warning: presision differs more than 1");
            }
        }

    return i;
}

void SAL_CALL test_precision( )
{
    sal_Int16 nPrecision;
    nPrecision = test_rtl_OString_checkPrecisionSize<float>();
    nPrecision = test_rtl_OString_checkPrecisionSize<double>();
    nPrecision = test_rtl_OString_checkPrecisionSize<long double>();
    nPrecision = test_rtl_OString_checkPrecisionSize<Double>();
}

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int argc, char* argv[] )
#else
int main( int argc, char* argv[] )
#endif
{
    test_precision();
    return 0;
}

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:
