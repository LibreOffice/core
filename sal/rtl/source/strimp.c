/*************************************************************************
 *
 *  $RCSfile: strimp.c,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 14:54:23 $
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

#include <math.h>

#ifndef _RTL_STRIMP_H_
#include <strimp.h>
#endif

/* ======================================================================= */
/* Help functions for String and UString                                   */
/* ======================================================================= */

const sal_Char* rtl_ImplGetDigits()
{
    static sal_Char aImplRtlDigits[] =
    {
        '0' , '1' , '2' , '3' , '4' , '5' ,
        '6' , '7' , '8' , '9' , 'a' , 'b' ,
        'c' , 'd' , 'e' , 'f' , 'g' , 'h' ,
        'i' , 'j' , 'k' , 'l' , 'm' , 'n' ,
        'o' , 'p' , 'q' , 'r' , 's' , 't' ,
        'u' , 'v' , 'w' , 'x' , 'y' , 'z'
    };

    return aImplRtlDigits;
}

/* ----------------------------------------------------------------------- */

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix )
{
    sal_Int16 n = -1;
    if ( (ch >= '0') && (ch <= '9') )
        n = ch-'0';
    else if ( (ch >= 'a') && (ch <= 'z') )
        n = ch-'a'+10;
    else if ( (ch >= 'A') && (ch <= 'Z') )
        n = ch-'A'+10;
    return (n < nRadix) ? n : -1;
}

/* ----------------------------------------------------------------------- */

#define FLOAT_MAX_POS       ( 3.402823e+38)
#define FLOAT_MAX_NEG       (-3.402823e+38)
#define FLOAT_MIN_POS       ( 1.175494351e-38)
#define FLOAT_MIN_NEG       (-1.175494351e-38)

/* ----------------------------------------------------------------------- */

static sal_Int32 SAL_CALL rtl_ImplFloatNumToString( sal_Char* pStr,
                                                    double d,
                                                    sal_Int16 nSignificantDigits )
{
    sal_Char*   pTempStr = pStr;
    sal_Char*   pEndStr;
    sal_Int16   i;
    sal_Int16   nDigit;
    sal_Int16   nDotPos;
    sal_Int16   nExpDigits;
    sal_Bool    bExp;
    sal_Bool    bDotSet;
    double      dExp;
    double      dRem;

    if ( d == 0.0 )
    {
        *pTempStr = '0';
        *pTempStr++;
        *pTempStr = '.';
        *pTempStr++;
        *pTempStr = '0';
        *pTempStr++;
    }
    else
    {
        if ( d < 0.0 )
        {
            *pTempStr = '-';
            *pTempStr++;
            d = -d;
        }

        dExp = log10( d );
        bExp = sal_False;
        if ( (dExp > 7) || (dExp <= -7) )
            bExp = sal_True;

        dExp = floor( dExp );
        d /= pow( 10, dExp );
        while ( d > 10 )
        {
            d /= 10.0;
            dExp += 1.0;
        }

        if ( d < 1.0 )
            nSignificantDigits++;

        nDotPos = bExp ? 0 : (sal_Int16)dExp;
        bDotSet = sal_False;

        /* handle leading zeros */
        if ( nDotPos < 0 )
        {
            *pTempStr = '0';
            *pTempStr++;
            *pTempStr = '.';
            *pTempStr++;
            while( ++nDotPos < 0 )
            {
                *pTempStr = '0';
                *pTempStr++;
            }
            pEndStr = pTempStr;
            bDotSet = sal_True;
        }
        else
            pEndStr = pTempStr;

        for ( i=0; i < nSignificantDigits; i++ )
        {
            nDigit = (sal_Int16)d;
            *pTempStr = '0' + nDigit;
            *pTempStr++;
            if ( i == nDotPos && !bDotSet )
            {
                *pTempStr = '.';
                *pTempStr++;
                pEndStr = pTempStr+1;   /* We want one 0 behind the . */
            }
            else if ( nDigit )
                pEndStr = pTempStr;

            d -= (double)nDigit;
            d *= 10.0;
        }

        /* Add/Kill trailing zeros */
        while ( pTempStr < pEndStr )
        {
            *pTempStr = '0';
            pTempStr++;
        }
        pTempStr = pEndStr;

        /* exponent */
        if ( bExp )
        {
            *pTempStr = 'E';
            *pTempStr++;
            if ( dExp < 0.0 )
            {
                dExp = -dExp;
                *pTempStr = '-';
                *pTempStr++;
            }

            nExpDigits = 1;
            while ( dExp >= 10.0 )
            {
                nExpDigits++;
                dExp /= 10;
            }
            for ( i=0; i < nExpDigits; i++ )
            {
                nDigit = (sal_Int16)dExp; /* sometimes if the debugger shows dExp= 2, the cast produces 1 */
                dRem = fmod( dExp, 1 );

                /* max exponent is about 357 */
                if ( dRem >0.999 )
                    nDigit++;

                *pTempStr = '0' + nDigit;
                *pTempStr++;

                dExp -= (double)nDigit;
                dExp *= 10.0;
            }
        }
    }

    *pTempStr = 0;
    return (sal_Int32)(pTempStr-pStr);
}

/* ----------------------------------------------------------------------- */

static sal_Int32 SAL_CALL rtl_ImplGetInfinityStr( sal_Char* pStr,
                                                  sal_Bool bNeg )
{
    static const sal_Char aInfinityStr[] = "Infinity";
    const sal_Char* pTempStr = aInfinityStr;

    if ( bNeg )
    {
        *pStr = '-';
        pStr++;
    }

    while ( *pTempStr )
    {
        *pStr = *pTempStr;
        pStr++;
        pTempStr++;
    }
    *pStr = 0;

    if ( bNeg )
        return sizeof( aInfinityStr );
    else
        return sizeof( aInfinityStr )-1;
}

/* ----------------------------------------------------------------------- */

static sal_Int32 SAL_CALL rtl_ImplGetNaNStr( sal_Char* pStr )
{
    *pStr = 'N';
    pStr++;
    *pStr = 'a';
    pStr++;
    *pStr = 'N';
    pStr++;
    *pStr = 0;
    return 3;
}

/* ----------------------------------------------------------------------- */

#define FLOAT_SIGNMASK      ((sal_uInt32)0x80000000)
#define FLOAT_EXPMASK       ((sal_uInt32)0x7f800000)
#define FLOAT_FRACTMASK     (~(FLOAT_SIGNMASK|FLOAT_EXPMASK))
#define FLOAT_EXPSHIFT      ((sal_uInt32)23)

sal_Int32 SAL_CALL rtl_ImplFloatToString( sal_Char* pStr, float f )
{
    /* Discover obvious special cases of NaN and Infinity
     * (like in Java Ctor FloatingDecimal( float f ) ) */
    sal_uInt32 nFBits       = *(sal_uInt32*)(&f);
    sal_uInt32 nBinExp      = (sal_uInt32)((nFBits & FLOAT_EXPMASK) >> FLOAT_EXPSHIFT);
    sal_uInt32 nFractBits   = nFractBits & FLOAT_FRACTMASK;
    if ( nBinExp == (sal_uInt32)(FLOAT_EXPMASK>>FLOAT_EXPSHIFT) )
    {
        if ( !nFractBits )
            return rtl_ImplGetInfinityStr( pStr, (sal_Bool)(f < 0.0) );
        else
            return rtl_ImplGetNaNStr( pStr );
    }

    return rtl_ImplFloatNumToString( pStr, (double)f, 8 );
}

/* ----------------------------------------------------------------------- */

#ifndef WNT
#define DOUBLE_SIGNMASK     ((sal_uInt64)0x8000000000000000LL)
#define DOUBLE_EXPMASK      ((sal_uInt64)0x7ff0000000000000LL)
#else
#define DOUBLE_SIGNMASK     ((sal_uInt64)0x8000000000000000L)
#define DOUBLE_EXPMASK      ((sal_uInt64)0x7ff0000000000000L)
#endif
#define DOUBLE_FRACTMASK    (~(DOUBLE_SIGNMASK|DOUBLE_EXPMASK))
#define DOUBLE_EXPSHIFT     ((sal_uInt32)52)

sal_Int32 SAL_CALL rtl_ImplDoubleToString( sal_Char* pStr, double d )
{
    /* Discover obvious special cases of NaN and Infinity.
     * (like in Java Ctor FloatingDecimal( double d ) ) */
    sal_uInt64 nDBits       = *(sal_uInt64*)(&d);
    sal_uInt32 nBinExp      = (sal_uInt32)((nDBits & DOUBLE_EXPMASK) >> DOUBLE_EXPSHIFT);
    sal_uInt64 nFractBits   = nDBits & DOUBLE_FRACTMASK;
    if ( nBinExp == (sal_uInt32)(DOUBLE_EXPMASK >> DOUBLE_EXPSHIFT) )
    {
        if ( !nFractBits )
            return rtl_ImplGetInfinityStr( pStr, (sal_Bool)(d < 0.0) );
        else
            return rtl_ImplGetNaNStr( pStr );
    }

    return rtl_ImplFloatNumToString( pStr, d, 17 );
}

/* ----------------------------------------------------------------------- */

double rtl_ImplCalcPow10( int nExp )
{
    static double rtl_fImplExpValues[] =
    {
        1.0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10,
        1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20
    };

    sal_Bool    bNeg = sal_False;
    double      fRet = 1.0;

    if ( nExp < 0 )
    {
        bNeg = sal_True;
        nExp = -nExp;
    }

    while ( nExp >= 20 )
    {
        fRet *= rtl_fImplExpValues[20];
        nExp -= 20;
    }

    fRet *= rtl_fImplExpValues[nExp];
    return bNeg ? 1.0/fRet : fRet;
}

/* ======================================================================= */

sal_Bool rtl_ImplIsWhitespace( sal_Unicode c )
{
    /* Space or Control character? */
    if ( (c <= 32) && c )
        return sal_True;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ( (c < 0x2000) || (c > 0x206F) )
        return sal_False;

    if ( ((c >= 0x2000) && (c <= 0x200B)) ||    /* All Spaces           */
         (c == 0x2028) ||                       /* LINE SEPARATOR       */
         (c == 0x2029) )                        /* PARAGRAPH SEPARATOR  */
        return sal_True;

    return sal_False;
}
