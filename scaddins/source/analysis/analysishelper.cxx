/*************************************************************************
 *
 *  $RCSfile: analysishelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: gt $ $Date: 2001-04-06 13:59:16 $
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


#include "analysishelper.hxx"

#include <string.h>
#include <stdio.h>



static const double nKorrVal[] = {
    0, 9e-1, 9e-2, 9e-3, 9e-4, 9e-5, 9e-6, 9e-7, 9e-8,
    9e-9, 9e-10, 9e-11, 9e-12, 9e-13, 9e-14, 9e-15
};


double Round( double fVal, short nDec )
{
    if ( fVal == 0.0  )
        return fVal;

    // sign adjustment
    sal_Bool bSign = fVal < 0.0;
    if ( bSign )
        fVal = -fVal;

    double fFac;
    if ( nDec != 0 )
    {
        // max 20 decimals, we don't have unlimited precision
        // #38810# and no overflow on fVal*=fFac
        if ( nDec < -20 || 20 < nDec || fVal > (1.7976931348623158e+308 / 1e20) )
            return bSign ? -fVal : fVal;

        fFac = pow( 10.0, nDec );
        fVal *= fFac;
    }
    //else  //! uninitialized fFac, not needed

            int nExp;       // exponent for correction
            if ( fVal > 0.0 )
                nExp = (int) floor( log10( fVal ) );
            else
                nExp = 0;
            int nIndex = 15 - nExp;
            if ( nIndex > 15 )
                nIndex = 15;
            else if ( nIndex <= 1 )
                nIndex = 0;
            fVal = floor( fVal + 0.5 + nKorrVal[nIndex] );

    if ( nDec != 0 )
        fVal /= fFac;

    return bSign ? -fVal : fVal;
}


double _Test( sal_Int32 nMode, double f1, double f2, double f3 )
{
    double      f = -1.0;
    switch( nMode )
    {
        case 0:     f = GammaN( f1, sal_uInt32( f2 ) );     break;
        case 1:     f = Gamma( f1 );                        break;
    }

    return f;
}


static sal_uInt16 aDaysInMonth[ 13 ] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{

    if( nMonth != 2 )
        return aDaysInMonth[ nMonth ];
    else
    {
        if( IsLeapYear( nYear ) )
            return aDaysInMonth[ nMonth ] + 1;
        else
            return aDaysInMonth[ nMonth ];
    }
}


sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear, sal_Bool bLeapYear )
{

    if( nMonth != 2 )
        return aDaysInMonth[ nMonth ];
    else
    {
        if( bLeapYear )
            return aDaysInMonth[ nMonth ] + 1;
        else
            return aDaysInMonth[ nMonth ];
    }
}


/**
 * Convert a date to a count of days starting from 01/01/0001
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts a Day , Month, Year representation
 * to this internal Date value.
 *
 */

sal_Int32 DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    sal_Int32 nDays = ((sal_Int32)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);

    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;

    return nDays;
}


/**
 * Convert a count of days starting from 01/01/0001 to a date
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function converts this internal Date value
 * to a Day , Month, Year representation of a Date.
 *
 */

void DaysToDate( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
{
    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = (sal_uInt16)((nTempDays / 365) - i);
        nTempDays -= ((sal_Int32) rYear -1) * 365;
        nTempDays -= (( rYear -1) / 4) - (( rYear -1) / 100) + ((rYear -1) / 400);
        bCalc = sal_False;
        if ( nTempDays < 1 )
        {
            i++;
            bCalc = sal_True;
        }
        else
        {
            if ( nTempDays > 365 )
            {
                if ( (nTempDays != 366) || !IsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( (sal_Int32)nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = (sal_uInt16)nTempDays;
}


/**
 * Get the null date used by the spreadsheet document
 *
 * The internal representation of a Date used in this Addin
 * is the number of days between 01/01/0001 and the date
 * this function returns this internal Date value for the document null date
 *
 */

sal_Int32 GetNullDate( constREFXPS& xOpt ) THROWDEF_RTE
{
    if( xOpt.is() )
    {
        try
        {
            uno::Any    aAny = xOpt->getPropertyValue( STRFROMASCII( "NullDate" ) );
            util::Date  aDate;
            if( aAny >>= aDate )
                return DateToDays( aDate.Day, aDate.Month, aDate.Year );
        }
        catch( uno::Exception& )
        {
        }
    }

    // no null date available -> no calculations possible
    throw uno::RuntimeException();
}


sal_Int32 GetDiffDate360(
                sal_uInt16 nDay1, sal_uInt16 nMonth1, sal_uInt16 nYear1, sal_Bool bLeapYear1,
                sal_uInt16 nDay2, sal_uInt16 nMonth2, sal_uInt16 nYear2,
                sal_Bool bUSAMethod )
{
    if( nDay1 == 31 )
        nDay1--;
    else if( bUSAMethod && ( nMonth1 == 2 && ( nDay1 == 29 || ( nDay1 == 28 && !bLeapYear1 ) ) ) )
            nDay1 = 30;

    if( nDay2 == 31 )
    {
        if( bUSAMethod && nDay1 != 30 )
        {
            //aDate2 += 1;      -> 1.xx.yyyy
            nDay2 = 1;
            if( nMonth2 == 12 )
            {
                nYear2++;
                nMonth2 = 1;
            }
            else
                nMonth2++;
        }
        else
            nDay2 = 30;
    }

    return nDay2 + nMonth2 * 30 + nYear2 * 360 - nDay1 - nMonth1 * 30 - nYear1 * 360;
}


sal_Int32 GetDaysInYears( sal_uInt16 nYear1, sal_uInt16 nYear2 )
{
    sal_uInt16  nLeaps = 0;
    for( sal_uInt16 n = nYear1 ; n <= nYear2 ; n++ )
    {
        if( IsLeapYear( n ) )
            nLeaps++;
    }

    sal_uInt32  nSum = 1;
    nSum += nYear2;
    nSum -= nYear1;
    nSum *= 365;
    nSum += nLeaps;

    return nSum;
}


double Fak( sal_Int32 n )
{
    if( n > 0 )
    {
        double  fRet = n;
        double  f = n - 1;

        while( f >= 2.0 )
        {
            fRet *= f;
            f--;
        }

        return fRet;
    }
    else if( !n )
        return 1.0;
    else
        return 0.0;
}


double GetGcd( double f1, double f2 )
{
    double  f = fmod( f1, f2 );
    while( f > 0.0 )
    {
        f1 = f2;
        f2 = f;
        f = fmod( f1, f2 );
    }

    return f2;
}


double GammaHelp( double& x, sal_Bool& bReflect )
{
    double c[6] = {76.18009173, -86.50532033, 24.01409822,
                   -1.231739516, 0.120858003E-2, -0.536382E-5};
    if (x >= 1.0)
    {
        bReflect = sal_False;
        x -= 1.0;
    }
    else
    {
        bReflect = sal_True;
        x = 1.0 - x;
    }
    double s, anum;
    s = 1.0;
    anum = x;
    for (sal_uInt16 i = 0; i < 6; i++)
    {
        anum += 1.0;
        s += c[i]/anum;
    }
    s *= 2.506628275;                   // sqrt(2*PI)
    return s;
}


double Gamma( double x )
{
    sal_Bool bReflect;
    double G = GammaHelp(x, bReflect);
    G = pow(x+5.5,x+0.5)*G/exp(x+5.5);
    if (bReflect)
        G = PI*x/(G*sin(PI*x));
    return G;
}


double GammaN( double x, sal_uInt32 nIter )
{
    double  n = nIter;
    double  f = pow( n, x ) / x;
    double  c = 1.0;

    while( nIter )
    {
        x++;

        f /= x;
        f *= c;

        nIter--;
        c++;
    }

    return f;
}


double Bessel( double fNum, sal_Int32 nOrder, sal_Bool bModfied ) THROWDEF_RTE
{
    if( nOrder < 0 )
        return -1.0;

    double      fZ, fZm, fN1, fN2, fn1, fn2, fAct, fOld;
    sal_Int16   nIterMax = 100;

    fZ = fNum * 0.5;                            // x/2
    fZm = fZ * fZ;                              // (x/2)^2
    fZ = pow( fZ, double( nOrder ) );           // (x/2)^n

    fN1 = Fak( nOrder );                        // n!
    fn1 = 0.0;
    fN2 = 1.0;
    fn2 = double( nOrder );

    fAct = fZ / fN1;
    fOld = fAct * 0.9;

    if( bModfied )
    {
        while( fAct != fOld && nIterMax )
        {
            fZ *= fZm;

            fn1++;
            fN1 *= fn1;

            fn2++;
            fN2 *= fn2;

            fOld = fAct;

            fAct += fZ / fN1 / fN2;

            nIterMax--;
        }
    }
    else
    {
        sal_Bool    bAdd = sal_False;   // start with second term, so subtraction is first in loop

        while( fAct != fOld && nIterMax )
        {
            fZ *= fZm;

            fn1++;
            fN1 *= fn1;

            fn2++;
            fN2 *= fn2;

            fOld = fAct;

            if( bAdd )
                fAct += fZ / fN1 / fN2;
            else
                fAct -= fZ / fN1 / fN2;

            nIterMax--;
            bAdd = !bAdd;
        }
    }

    return fAct;
}


double BesselR( double fNum, double fOrder ) THROWDEF_RTE
{
    double      fZ, fZm, fN, fn, fAct, fOld;
    sal_Int16   nIterMax = 100;

    fZ = fNum * 0.5;                // x/2
    fZm = fZ * fZ;                  // (x/2)^2
    fZ = pow( fZ, fOrder );         // (x/2)^n

    fN = 1.0;                       // effectiv: k!
    fn = 1.0;

    fOrder++;
    fAct = fZ / Gamma( fOrder );
    fOld = fAct * 0.9;

    while( fAct != fOld && nIterMax )
    {
        fZ *= fZm;

        fN *= fn;
        fn++;

        fOld = fAct;

        fOrder++;
        double  fG = Gamma( fOrder );
        fAct += fZ / fN / fG;

        nIterMax--;
    }

    return fAct;
}


double ConvertToDec( const STRING& aStr, sal_uInt16 nBase, sal_uInt16 nCharLim ) THROWDEF_RTE_IAE
{
    if ( nBase < 2 || nBase > 36 )
        return -1.0;

    sal_uInt32      nStrLen = aStr.getLength();
    if( nStrLen > nCharLim )
        THROW_IAE;
    else if( !nStrLen )
        return 0.0;

    double          fVal = 0.0;

    register const sal_Unicode* p = aStr.getStr();

    sal_uInt16          nFirstDig;
    sal_Bool            bFirstDig = sal_True;
    double              fBase = nBase;

    while ( *p )
    {
        sal_uInt16      n;

        if( '0' <= *p && *p <= '9' )
            n = *p - '0';
        else if( 'A' <= *p && *p <= 'Z' )
            n = 10 + ( *p - 'A' );
        else if ( 'a' <= *p && *p <= 'z' )
            n = 10 + ( *p - 'a' );
        else
            n = nBase;

        if( n < nBase )
        {
            if( bFirstDig )
            {
                bFirstDig = sal_False;
                nFirstDig = n;
            }
            fVal = fVal * fBase + double( n );
        }
        else
            // illegal char!
            THROW_IAE;

        p++;

    }

    if( nStrLen == nCharLim && !bFirstDig && nFirstDig == nBase - 1 )
    {   // handling negativ values
        fVal = ( pow( double( nBase ), double( nCharLim ) ) - fVal );   // complement
        fVal *= -1.0;
    }

    return fVal;
}


static inline sal_Char GetMaxChar( sal_uInt16 nBase )
{
    const sal_Char* c = "--123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    return c[ nBase ];
}


STRING ConvertFromDec( sal_Int64 nNum, sal_Int64 nMin, sal_Int64 nMax, sal_uInt16 nBase,
    sal_Int32 nPlaces, sal_Int32 nMaxPlaces ) THROWDEF_RTE_IAE
{
    if( nNum < nMin || nNum > nMax || nPlaces <= 0 || nPlaces > nMaxPlaces )
        THROW_IAE;

    sal_Bool        bNeg = nNum < 0;
    if( bNeg )
        nNum = sal_Int64( pow( double( nBase ), double( nMaxPlaces ) ) ) + nNum;

    STRING          aRet( STRING::valueOf( nNum, nBase ).toUpperCase() );

    sal_Int32       nLen = aRet.len();
    if( !bNeg && nLen > nPlaces )
        THROW_IAE;
    else if( ( bNeg && nLen < nMaxPlaces ) || ( !bNeg && nLen < nPlaces ) )
    {
        sal_uInt32  nLeft = nPlaces - nLen;
        sal_Char*   p = new sal_Char[ nLeft + 1 ];
        memset( p, bNeg? GetMaxChar( nBase ) : '0', nLeft );
        p[ nLeft ] = 0x00;
        STRING  aTmp( p, nLeft, RTL_TEXTENCODING_MS_1252 );
        aTmp += aRet;
        aRet = aTmp;

        delete[] p;
    }

    return aRet;
}


double Erf( double x )
{
    if( x == 0.0 )
        return 0.0;

    double      f, fZm, fN2, fn1, fOld, fT;
    sal_Bool    bAdd = sal_True;
    sal_Int32   nMaxIter = 1000;

    fZm = x;
    fZm *= x;   // x^2

    fn1 = 2.0;
    fN2 = 3.0;

    fT = x * fZm;

    f = x - fT / fN2;

    fOld = f * 0.9;

    while( f != fOld && nMaxIter )
    {
        fOld = f;

        fN2 += 2.0;

        fT /= fn1;
        fT *= fZm;

        fn1++;

        if( bAdd )
            f += fT / fN2;
        else
            f -= fT / fN2;

        bAdd = !bAdd;

        nMaxIter--;
    }

    return f * 1.128379167095512573896; // * 2/sqrt(PI)
}


inline sal_Bool IsNum( sal_Unicode c )
{
    return c >= '0' && c <= '9';
}


inline sal_Bool IsComma( sal_Unicode c )
{
    return c == '.' || c == ',';
}


inline sal_Bool IsExpStart( sal_Unicode c )
{
    return c == 'e' || c == 'E';
}


inline sal_Bool IsImagUnit( sal_Unicode c )
{
    return c == 'i' || c == 'j';
}


inline sal_uInt16 GetVal( sal_Unicode c )
{
    return sal_uInt16( c - '0' );
}


sal_Bool ParseDouble( const sal_Unicode*& rp, double& rRet )
{
    double              fInt = 0.0;
    double              fFrac = 0.0;
    double              fMult = 0.1;    // multiplier to multiply digits with, when adding fractional ones
    sal_Int32           nExp = 0;
    sal_Int32           nMaxExp = 307;
    sal_uInt16          nDigCnt = 18;   // max. number of digits to read in, rest doesn't matter

    enum State  { S_End = 0, S_Sign, S_IntStart, S_Int, S_IgnoreIntDigs, S_Frac, S_IgnoreFracDigs, S_ExpSign, S_Exp };

    State           eS = S_Sign;

    sal_Bool            bNegNum = sal_False;
    sal_Bool            bNegExp = sal_False;

    const sal_Unicode*  p = rp;
    sal_Unicode         c;

    while( eS )
    {
        c = *p;
        switch( eS )
        {
            case S_Sign:
                if( IsNum( c ) )
                {
                    fInt = GetVal( c );
                    nDigCnt--;
                    eS = S_Int;
                }
                else if( c == '-' )
                {
                    bNegNum = sal_True;
                    eS = S_IntStart;
                }
                else if( c == '+' )
                    eS = S_IntStart;
                else if( IsComma( c ) )
                    eS = S_Frac;
                else
                    return sal_False;
                break;
            case S_IntStart:
                if( IsNum( c ) )
                {
                    fInt = GetVal( c );
                    nDigCnt--;
                    eS = S_Int;
                }
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsImagUnit( c ) )
                {
                    rRet = bNegNum? -1.0 : 1.0;
                    return sal_True;
                }
                else
                    return sal_False;
                break;
            case S_Int:
                if( IsNum( c ) )
                {
                    fInt *= 10.0;
                    fInt += double( GetVal( c ) );
                    nDigCnt--;
                    if( !nDigCnt )
                        eS = S_IgnoreIntDigs;
                }
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_IgnoreIntDigs:
                if( IsNum( c ) )
                    nExp++;         // just multiply num with 10... ;-)
                else if( IsComma( c ) )
                    eS = S_Frac;
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_Frac:
                if( IsNum( c ) )
                {
                    fFrac += double( GetVal( c ) ) * fMult;
                    nDigCnt--;
                    if( nDigCnt )
                        fMult *= 0.1;
                    else
                        eS = S_IgnoreFracDigs;
                }
                else if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else
                    eS = S_End;
                break;
            case S_IgnoreFracDigs:
                if( IsExpStart( c ) )
                    eS = S_ExpSign;
                else if( !IsNum( c ) )
                    eS = S_End;
                break;
            case S_ExpSign:
                if( IsNum( c ) )
                {
                    nExp = GetVal( c );
                    eS = S_Exp;
                }
                else if( c == '-' )
                {
                    bNegExp = sal_True;
                    eS = S_Exp;
                }
                else if( c != '+' )
                    eS = S_End;
                break;
            case S_Exp:
                if( IsNum( c ) )
                {
                    nExp *= 10;
                    nExp += GetVal( c );
                    if( nExp > nMaxExp )
                        return sal_False;
                }
                else
                    eS = S_End;
                break;
        }

        p++;
    }

    p--;        // set pointer back to last
    rp = p;

    fInt += fFrac;
    sal_Int32   nLog10 = sal_Int32( log( fInt ) );

    if( bNegExp )
        nExp = -nExp;

    if( nLog10 + nExp > nMaxExp )
        return sal_False;

    fInt *= pow( 10.0, nExp );

    if( bNegNum )
        fInt = -fInt;

    rRet = fInt;

    return sal_True;
}


STRING GetString( double f, sal_Bool bLeadingSign, sal_uInt16 nMaxDig )
{
    const int       nBuff = 256;
    sal_Char*       pBuff = new sal_Char[ nBuff + 1 ];
    const char*     pFormStr = bLeadingSign? "%+.*g" : "%.*g";
    int             nLen = sprintf( pBuff, pFormStr, int( nMaxDig ), f );

    STRING          aRet( pBuff, nLen, RTL_TEXTENCODING_MS_1252 );

    delete pBuff;

    return aRet;
}


double Exp10( sal_Int16 n )
{
    return pow( 10.0, double( n ) );
}




const sal_uInt32 List::nStartSize = 16;
const sal_uInt32 List::nIncrSize = 16;


void List::_Grow( void )
{
    nSize += nIncrSize;

    void**          pNewData = new void*[ nSize ];
    memcpy( pNewData, pData, nNew * sizeof( void* ) );

    delete pData;
    pData = pNewData;
}


List::List( void )
{
    nSize = nStartSize;
    pData = new void*[ nSize ];
    nNew = nAct = 0;
}


List::~List()
{
    delete pData;
}


void List::Insert( void* p, sal_uInt32 n )
{
    if( n >= nNew )
        Append( p );
    else
    {
        Grow();

        void**      pIns = pData + n;
        memmove( pIns, pIns + 1, 1 );

        *pIns = p;

        nNew++;
    }
}




FuncData::FuncData( void )
{
    pIntName = "<no internal name assigned>";
    pGerman = "KEIN_NAME_ZUGEWIESEN";
    pEnglish = "NO_NAME_ASSIGNED";
    pDescr = "<no description available>";
    ppParam = ppParamDescr = NULL;
    nParam = nParamDescr = 0;
}


FuncData::~FuncData()
{
    if( ppParam )
        delete[] ppParam;
    if( ppParamDescr )
        delete[] ppParamDescr;
}


const sal_Char* FuncData::GetParam( sal_uInt32 nInd ) const
{
    if( !nInd )
        return "(internal)";

    nInd--;

    if( nInd < nParam )
        return ppParam[ nInd ];
    else
        return "<no name for parameter available>";
}


const sal_Char* FuncData::GetParamDescr( sal_uInt32 nInd ) const
{
    if( !nInd )
        return "(internal)";

    nInd--;

    if( nInd < nParamDescr )
        return ppParamDescr[ nInd ];
    else
        return "<no name for parameter description available>";
}


FuncData* FuncData::CloneFromList( const sal_Char**& rpp )
{
    const sal_Char**    pp = rpp;
    const sal_Char*     pAct = *pp;

    if( pAct == EOL )
        return NULL;

    FuncData*           p = new FuncData;
    sal_Bool            bRead = sal_True;
    CStrList            aPNList;
    CStrList            aPDList;

    while( bRead )
    {
        if( pAct == EOE )
            bRead = sal_False;
        else
        {
            switch( *pAct )
            {
                case 'i':   p->pIntName = pAct + 1;     break;
                case '1':   p->pGerman = pAct + 1;      break;
                case '2':   p->pEnglish = pAct + 1;     break;
                case 'd':   p->pDescr = pAct + 1;       break;
                case 'p':   aPNList.Append( pAct + 1 ); break;
                case 'P':   aPDList.Append( pAct + 1 ); break;
            }

        }

        pp++;
        pAct = *pp;
    }

    sal_uInt32      nE = aPNList.Count();
    sal_uInt32      n;
    if( nE )
    {
        if( nE > 255 )
            nE = 255;

        p->nParam = nE;
        const sal_Char**    pArray = new const sal_Char*[ nE ];
        p->ppParam = pArray;

        for( n = 0 ; n < nE ; n++ )
            pArray[ n ] = aPNList.Get( n );
    }

    nE = aPDList.Count();
    if( nE )
    {
        if( nE > 255 )
            nE = 255;

        p->nParamDescr = nE;
        const sal_Char**    pArray = new const sal_Char*[ nE ];
        p->ppParamDescr = pArray;

        for( n = 0 ; n < nE ; n++ )
            pArray[ n ] = aPDList.Get( n );
    }

    rpp = pp;

    return p;
}




FuncDataList::FuncDataList( const sal_Char** pFD )
{
    nLast = 0xFFFFFFFF;

    const sal_Char**    pp = pFD;
    FuncData*           p = FuncData::CloneFromList( pp );

    while( p )
    {
        Append( p );
        p = FuncData::CloneFromList( pp );
    }

}


FuncDataList::~FuncDataList()
{
    for( FuncData* p = ( FuncData* ) First() ; p ; p = ( FuncData* ) Next() )
        delete p;
}


const FuncData* FuncDataList::Get(  const OUString& aProgrammaticName ) const
{
    if( aLastName == aProgrammaticName )
        return Get( nLast );

    ( ( FuncDataList* ) this )->aLastName = aProgrammaticName;

    sal_uInt32  nE = Count();
    for( sal_uInt32 n = 0 ; n < nE ; n++ )
    {
        const FuncData* p = Get( n );
        if( aProgrammaticName.compareToAscii( p->pIntName ) == 0 )
        {
            ( ( FuncDataList* ) this )->nLast = n;
            return p;
        }
    }

    ( ( FuncDataList* ) this )->nLast = 0xFFFFFFFF;
    return NULL;
}




SortedIndividualInt32List::SortedIndividualInt32List()
{
}


SortedIndividualInt32List::~SortedIndividualInt32List()
{
}


void SortedIndividualInt32List::Insert( sal_Int32 nVal )
{
    sal_Int32   n = Count();

    while( n )
    {
        n--;
        sal_Int32   nRef = Get( n );
        if( nVal == nRef )
            return;
        else if( nVal > nRef )
        {
            List::Insert( ( void* ) nVal, n + 1 );
            return;
        }
    }

    // smalest (or first) element
    List::Insert( ( void* ) nVal, sal_uInt32( 0 ) );
}


sal_Int32 SortedIndividualInt32List::CountCondition( sal_Int32 nMinVal, sal_Int32 nMaxVal ) const
{
    sal_uInt32  nE = Count();

    if( !nE || nMaxVal < Get( 0 ) || nMinVal > Get( nE - 1 ) )
        // don't care if list is empty
        // ~ if nMaxVal is smaler than smalest value in list
        // ~ if nMinVal is bigger than biggest value in list
        return 0;

    sal_uInt32  n;
    sal_Int32   nCnt = 0;

    for( n = 0 ; n < nE && Get( n ) < nMinVal ; n++);   // skip values less than nMinVal

    for( ; n < nE ; n++ )
    {
        if( Get( n ) <= nMaxVal )
            nCnt++;
        else
            break;
    }

    return nCnt;
}


sal_Bool SortedIndividualInt32List::Find( sal_Int32 nVal ) const
{
    sal_uInt32  nE = Count();

    if( !nE || nVal < Get( 0 ) || nVal > Get( nE - 1 ) )
        return sal_False;

    // linear search

    for( sal_uInt32 n = 0 ; n < nE ; n++ )
    {
        sal_Int32   nRef = Get( n );

        if( nRef == nVal )
            return sal_True;
        else if( nRef > nVal )
            return sal_False;
    }
    return sal_False;
}


void SortedIndividualInt32List::InsertHolidayList( const SEQSEQ( sal_Int32 )& aHD, sal_Int32 nND, sal_Bool bInsOnWE )
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = aHD.getLength();
    sal_Int32   nE2;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( sal_Int32 )& rList = aHD[ n1 ];
        nE2 = rList.getLength();
        const sal_Int32*        pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            sal_Int32   n = pList[ n2 ];

            if( n )
            {
                n += nND;
                if( bInsOnWE || GetDayOfWeek( n ) < 5 )
                    Insert( n );
            }
        }
    }
}




DoubleList::~DoubleList()
{
    for( double* p = ( double* ) List::First(); p ; p = ( double* ) List::Next() )
        delete p;
}


sal_Bool DoubleList::Append( const SEQSEQ( double )& aVLst )
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = aVLst.getLength();
    sal_Int32   nE2;
    sal_Int32   nZ = 0;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( double )&    rList = aVLst[ n1 ];
        nE2 = rList.getLength();
        const double*           pList = rList.getConstArray();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
        {
            double  f = pList[ n2 ];
            if( IsFaulty( f ) )
                return sal_False;
            if( IsProper( f ) )
                _Append( f );
        }
    }

    return sal_True;
}


sal_Bool DoubleList::IsProper( double ) const
{
    return sal_True;
}


sal_Bool DoubleList::IsFaulty( double ) const
{
    return sal_False;
}




sal_Bool ChkDoubleList1::IsProper( double f ) const
{
    return f > 0.0;
}

sal_Bool ChkDoubleList1::IsFaulty( double f ) const
{
    return f < 0.0;
}




Complex::Complex( const STRING& r ) THROWDEF_RTE_IAE
{
    if( !ParseString( r, *this ) )
        THROW_IAE;
}


sal_Bool Complex::ParseString( const STRING& rStr, Complex& rCompl )
{
    const sal_Unicode*      pStr = ( const sal_Unicode * ) rStr;
    double                  f;

    if( !ParseDouble( pStr, f ) )
        return sal_False;

    switch( *pStr )
    {
        case '-':   // imag part follows
        case '+':
            {
            double          r = f;
            if( ParseDouble( pStr, f ) && ( *pStr == 'j' || *pStr == 'i' ) )
            {
                pStr++;
                if( *pStr == 0 )
                {
                    rCompl.r = r;
                    rCompl.i = f;
                    return sal_True;
                }
            }
            }
            break;
        case 'j':
        case 'i':
            pStr++;
            if( *pStr == 0 )
            {
                rCompl.i = f;
                rCompl.r = 0.0;
                return sal_True;
            }
            break;
        case 0:     // only real-part
            rCompl.r = f;
            rCompl.i = 0.0;
            return sal_True;
    }

    return sal_False;
}


STRING Complex::GetString( sal_Bool bi ) const
{
    static const STRING aI( "i", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aJ( "j", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aPlus( "+", 1, RTL_TEXTENCODING_MS_1252 );
    static const STRING aMinus( "-", 1, RTL_TEXTENCODING_MS_1252 );

    STRING              aRet( ::GetString( r ) );

    if( i == 1.0 )
        aRet += aPlus;
    else if( i == -1.0 )
        aRet += aMinus;
    else
        aRet += ::GetString( i, sal_True );

    aRet += bi? aI : aJ;

    return aRet;
}


double Complex::Arg( void ) const THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
        THROW_IAE;

    double  phi = acos( r / Abs() );

    if( i < 0.0 )
        phi = -phi;

    return phi;
}


void Complex::Power( double fPower ) THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
    {
        if( fPower > 0 )
            r = i = 0.0;
        else
            THROW_IAE;
    }

    double      p, phi;

    p = Abs();

    phi = acos( r / p );
    if( i < 0.0 )
        phi = -phi;

    p = pow( p, fPower );
    phi *= fPower;

    r = cos( phi ) * p;
    i = sin( phi ) * p;
}


void Complex::Sin( void ) THROWDEF_RTE_IAE
{
    double r_;

    r_ = sin( r ) * cosh( i );
    i = -cos( r ) * sinh( i );
    r = r_;
}


void Complex::Cos( void ) THROWDEF_RTE_IAE
{
    double r_;

    r_ = cos( r ) * cosh( i );
    i = -sin( r ) * sinh( i );
    r = r_;
}


void Complex::Div( const Complex& z ) THROWDEF_RTE_IAE
{
    if( z.r == 0 && z.i == 0 )
        THROW_IAE;

    double  a1 = r;
    double  a2 = z.r;
    double  b1 = i;
    double  b2 = z.i;

    double  f = 1.0 / ( a2 * a2 + b2 * b2 );

    r = ( a1 * a2 + b1 * b2 ) * f;
    i = ( a2 * b1 - a1 * b2 ) * f;
}


void Complex::Exp( void )
{
    Mult( exp( r ) );
}


void Complex::Ln( void ) THROWDEF_RTE_IAE
{
    if( r == 0.0 && i == 0.0 )
        THROW_IAE;

    double  r_;

    r_ = log( Abs() );

    i = atan( i / r );
    r = r_;
}


void Complex::Log10( void ) THROWDEF_RTE_IAE
{
    Ln();
    Mult( 0.434294481903251828 );   // * log10( e )
}


void Complex::Log2( void ) THROWDEF_RTE_IAE
{
    Ln();
    Mult( 1.442695040888963407 );   // * log2( e )
}




ComplexList::~ComplexList()
{
    for( Complex* p = ( Complex* ) First() ; p ; p = ( Complex* ) Next() )
        delete p;
}


void ComplexList::Append( const SEQSEQ( STRING )& r ) THROWDEF_RTE_IAE
{
    sal_Int32   n1, n2;
    sal_Int32   nE1 = r.getLength();
    sal_Int32   nE2;

    for( n1 = 0 ; n1 < nE1 ; n1++ )
    {
        const SEQ( STRING )&    rList = r[ n1 ];
        nE2 = rList.getLength();

        for( n2 = 0 ; n2 < nE2 ; n2++ )
            Append( new Complex( rList[ n2 ] ) );
    }
}




ConvertData::ConvertData( const sal_Char p[], double fC, ConvertDataClass e ) : aName( p, strlen( p ), RTL_TEXTENCODING_MS_1252 )
{
    fConst = fC;
    eClass = e;
}


sal_Int16 ConvertData::GetMatchingLevel( const STRING& rRef ) const
{
    if( aName.equals( rRef ) )
        return 0;
    else
    {
        const sal_Unicode*  p = rRef.getStr();

        if( wcscmp( aName.getStr(), p + 1 ) == 0 )
        {
            sal_Int16       n;
            switch( *p )
            {
                case 'y':   n = -24;    break;      // yocto
                case 'z':   n = -21;    break;      // zepto
                case 'a':   n = -18;    break;
                case 'f':   n = -15;    break;
                case 'p':   n = -12;    break;
                case 'n':   n = -9;     break;
                case 'u':   n = -6;     break;
                case 'm':   n = -3;     break;
                case 'c':   n = -2;     break;
                case 'd':   n = -1;     break;
                case 'e':   n = 1;      break;
                case 'h':   n = 2;      break;
                case 'k':   n = 3;      break;
                case 'M':   n = 6;      break;
                case 'G':   n = 9;      break;
                case 'T':   n = 12;     break;
                case 'P':   n = 15;     break;
                case 'E':   n = 18;     break;
                case 'Z':   n = 21;     break;      // zetta
                case 'Y':   n = 24;     break;      // yotta
                default:
                            n = INV_MATCHLEV;
            }

            return n;
        }
        else
            return INV_MATCHLEV;
    }
}


double ConvertData::Convert(
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const THROWDEF_RTE_IAE
{
    if( Class() != r.Class() )
        THROW_IAE;

    nLevFrom -= nLevTo;                         // effectiv level

    f *= r.fConst / fConst;

    if( nLevFrom )
        f *= Exp10( nLevFrom );

    return f;
}


double ConvertData::ConvertToBase( double f, sal_Int16 n ) const
{
    return f / fConst * Exp10( n );
}


double ConvertData::ConvertFromBase( double f, sal_Int16 n ) const
{
    return f * fConst * Exp10( -n );
}




double ConvertDataLinear::Convert(
    double f, const ConvertData& r, sal_Int16 nLevFrom, sal_Int16 nLevTo ) const THROWDEF_RTE_IAE
{
    if( Class() != r.Class() )
        THROW_IAE;

//  return Round( r.ConvertFromBase( ConvertToBase( f, nLevFrom ), nLevTo ), 13 );
    return r.ConvertFromBase( ConvertToBase( f, nLevFrom ), nLevTo );
}


double ConvertDataLinear::ConvertToBase( double f, sal_Int16 n ) const
{
    if( n )
        f *= Exp10( n );

    f /= fConst;
    f -= fOffs;

    return f;
}


double ConvertDataLinear::ConvertFromBase( double f, sal_Int16 n ) const
{
    f += fOffs;
    f *= fConst;

    if( n )
        f *= Exp10( -n );

    return f;
}




ConvertDataList::ConvertDataList( void )
{
#define NEWD(str,unit,cl)   Append(new ConvertData(str,unit,cl))
#define NEWL(str,unit,offs,cl)  Append(new ConvertDataLinear(str,unit,offs,cl))

    // *** are extra and not standard Excel Analysis Addin!

                                                                // Gramm ->
    NEWD( "g",      1.0000000000000000E00,  CDC_Mass        );      // Gramm
    NEWD( "sg",     6.8522050005347800E-05, CDC_Mass        );      // Stueck
    NEWD( "lbm",    2.2046229146913400E-03, CDC_Mass        );      // Pfund (Handelsgewicht)
//  NEWD( "u",      6.0221700000000000E23,  CDC_Mass        );      // U (atomare Masseinheit)
    NEWD( "u",      6.0221370000000000E23,  CDC_Mass        );      // U (atomare Masseinheit)
    NEWD( "ozm",    3.5273971800362700E-02, CDC_Mass        );      // Unze (Handelsgewicht)
    NEWD( "stone",  1.574730e-04,           CDC_Mass        );      // Stone                                    ***
    NEWD( "ton",    1.102311e-06,           CDC_Mass        );      // ton                                      ***
    NEWD( "grain",  1.543236E01,            CDC_Mass        );      // grain                                    ***
    NEWD( "pweight",    7.054792E-01,       CDC_Mass        );      // Pennyweight                              ***
    NEWD( "hweight",    1.968413E-05,       CDC_Mass        );      // Hundredweight                            ***
    NEWD( "shweight",   2.204623e-05,       CDC_Mass        );      // Shorthundredweight                       ***

                                                                // Meter ->
    NEWD( "m",      1.0000000000000000E00,  CDC_Length      );      // Meter
    NEWD( "mi",     6.2137119223733397E-04, CDC_Length      );      // Britsche Meile       6,21371192237333969617434184363e-4
    NEWD( "Nmi",    5.3995680345572354E-04, CDC_Length      );      // Nautische Meile      5,39956803455723542116630669546e-4
    NEWD( "in",     3.9370078740157480E01,  CDC_Length      );      // Zoll                 39,37007874015748031496062992126
    NEWD( "ft",     3.2808398950131234E00,  CDC_Length      );      // Fuss                 3,2808398950131233595800524934383
    NEWD( "yd",     1.0936132983377078E00,  CDC_Length      );      // Yard                 1,0936132983377077865266841644794
    NEWD( "ang",    1.0000000000000000E10,  CDC_Length      );      // Angstroem
    NEWD( "Pica",   2.8346456692913386E03,  CDC_Length      );      // Pica (1/72 Zoll)     2834,6456692913385826771653543307
    NEWD( "ell",    8.748906E-01,           CDC_Length      );      // Elle?                                    ***

                                                                // Sekunde ->
    NEWD( "yr",     3.1688087814028950E-08, CDC_Time        );      // Jahr
    NEWD( "day",    1.1574074074074074E-05, CDC_Time        );      // Tag
    NEWD( "hr",     2.7777777777777778E-04, CDC_Time        );      // Stunde
    NEWD( "mn",     1.6666666666666667E-02, CDC_Time        );      // Minute
    NEWD( "sec",    1.0000000000000000E00,  CDC_Time        );      // Sekunde

                                                                // Pascal ->
    NEWD( "Pa",     1.0000000000000000E00,  CDC_Pressure    );      // Pascal
    NEWD( "atm",    9.8692329999819300E-06, CDC_Pressure    );      // Atmoshpaere
    NEWD( "mmHg",   7.5006170799862700E-03, CDC_Pressure    );      // mm Quecksilbersaeule
    NEWD( "Torr",   7.5006380000000000E-03, CDC_Pressure    );      // Torr                                     ***
    NEWD( "psi",    1.4503770000000000E-04, CDC_Pressure    );      // psi                                      ***

                                                                // Newton ->
    NEWD( "N",      1.0000000000000000E00,  CDC_Force       );      // Newton
    NEWD( "dyn",    1.0000000000000000E05,  CDC_Force       );      // Dyn
    NEWD( "pond",   1.019716E02,            CDC_Force       );      // Pond

                                                                // Joule ->
    NEWD( "J",      1.0000000000000000E00,  CDC_Energy      );      // Joule
//  NEWD( "e",      9.99999519343231E06,    CDC_Energy      );      // Erg
    NEWD( "e",      1.0000000000000000E07,  CDC_Energy      );      // Erg  -> http://www.chemie.fu-berlin.de/chemistry/general/si.html
    NEWD( "c",      2.3900624947346700E-01, CDC_Energy      );      // Thermodynamische Kalorie
    NEWD( "cal",    2.3884619064201700E-01, CDC_Energy      );      // Kalorie
    NEWD( "eV",     6.2414570000000000E18,  CDC_Energy      );      // Elektronenvolt
//  NEWD( "HPh",    3.72506430801000E-07,   CDC_Energy      );      // Pferdestaerke mal Stunde
    NEWD( "HPh",    3.7250611111111111E-07, CDC_Energy      );      // Horsepower mal Stunde
    NEWD( "Wh",     2.7777777777777778E-04, CDC_Energy      );      // Watt mal Stunde
    NEWD( "BTU",    9.4781506734901500E-04, CDC_Energy      );      // BTU

                                                                // Watt ->
    NEWD( "W",      1.0000000000000000E00,  CDC_Power       );      // Watt
    NEWD( "HP",     1.341022E-03,           CDC_Power       );      // Horsepower
    NEWD( "PS",     1.359622E-03,           CDC_Power       );      // Pferdestaerke                            ***
//EWD( "HP",        1.4102006031908E-03,    CDC_Power       );      // PS   ->Excel seams to be a little bit wrong...
                                                                    //  either this doesn't fit to J -> HPh....

                                                                // Tesla ->
    NEWD( "T",      1.0000000000000000E00,  CDC_Magnetism   );      // Tesla
    NEWD( "ga",     1.0000000000000000E04,  CDC_Magnetism   );      // Gauss

                                                                // Grad Kelvin ->
    NEWL( "C",      1.0000000000000000E00,  -2.7315000000000000E02, CDC_Temperature );      // Grad Celsius
    NEWL( "F",      1.8000000000000000E00,  -2.5537222222222222E02, CDC_Temperature );      // Grad Fahrenheit
    NEWL( "K",      1.0000000000000000E00,  +0.0000000000000000E00, CDC_Temperature );      // Grad Kelvin
    NEWL( "Reau",   8.0000000000000000E-01, -2.7315000000000000E02, CDC_Temperature );      // Grad Reaumur     ***
    NEWL( "Rank",   1.8000000000000000E00,  +0.0000000000000000E00, CDC_Temperature );      // Grad Rankine     ***

                                                                // Liter ->
    NEWD( "tsp",    2.0284000000000000E02,  CDC_Volume      );      // Teeloeffel (Teaspoon)
    NEWD( "tbs",    6.7613333333333333E01,  CDC_Volume      );      // Essloeffel (Tablespoon)
    NEWD( "oz",     3.3806666666666667E01,  CDC_Volume      );      // Unze (Fluessigkeit)
    NEWD( "cup",    4.2258333333333333E00,  CDC_Volume      );      // Tasse (Cup)
    NEWD( "pt",     2.1129166666666667E00,  CDC_Volume      );      // Pint
    NEWD( "qt",     1.0564583333333333E00,  CDC_Volume      );      // Quart
    NEWD( "gal",    2.6411458333333333E-01, CDC_Volume      );      // Gallone
    NEWD( "l",      1.0000000000000000E00,  CDC_Volume      );      // Liter
    NEWD( "m3",     1.0000000000000000E03,  CDC_Volume      );      // Meter^3                                  ***
    NEWD( "mi3",    2.3991275857892772E-10, CDC_Volume      );      // Britsche Meile^3                         ***
    NEWD( "Nmi3",   1.5742621468581148E-10, CDC_Volume      );      // Nautische Meile^3                        ***
    NEWD( "in3",    6.1023744094732284E04,  CDC_Volume      );      // Zoll^3                                   ***
    NEWD( "ft3",    3.5314666721488590E01,  CDC_Volume      );      // Fuss^3                                   ***
    NEWD( "yd3",    1.3079506193143922E00,  CDC_Volume      );      // Yard^3                                   ***
    NEWD( "ang3",   1.0000000000000000E30,  CDC_Volume      );      // Angstroem^3                              ***
    NEWD( "Pica3",  2.2776990435870636E10,  CDC_Volume      );      // Pica^3                                   ***
    NEWD( "barrel", 6.289811E-03,           CDC_Volume      );      // Barrel (=42gal?)                         ***
    NEWD( "bushel", 2.837759E-02,           CDC_Volume      );      // Bushel                                   ***
    NEWD( "regton", 3.531467E-04,           CDC_Volume      );      // Register ton                             ***
    NEWD( "Sixpack",2.0,                    CDC_Volume      );      // Sixpack                                  ***

                                                                // Meter^2 ->
    NEWD( "m2",     1.0000000000000000E00,  CDC_Area        );      // Meter^2                                  ***
    NEWD( "mi2",    3.8610215854244585E-07, CDC_Area        );      // Britsche Meile^2                         ***
    NEWD( "Nmi2",   2.9155334959812286E-07, CDC_Area        );      // Nautische Meile^2                        ***
    NEWD( "in2",    1.5500031000062000E03,  CDC_Area        );      // Zoll^2                                   ***
    NEWD( "ft2",    1.0763910416709722E01,  CDC_Area        );      // Fuss^2                                   ***
    NEWD( "yd2",    1.0936132983377078E00,  CDC_Area        );      // Yard^2                                   ***
    NEWD( "ang2",   1.0000000000000000E20,  CDC_Area        );      // Angstroem^2                              ***
    NEWD( "Pica2",  8.0352160704321409E06,  CDC_Area        );      // Pica^2                                   ***
    NEWD( "Morgen", 2.553167E03,            CDC_Area        );      // Morgen                                   ***
    NEWD( "ar",     1.000000E02,            CDC_Area        );      // Ar                                       ***
    NEWD( "acre",   4.046856E03,            CDC_Area        );      // acre (oder auch Acker)                   ***
    NEWD( "ha",     1.000000E04,            CDC_Area        );      // Hektar                                   ***
    NEWD( "Quadratlatschen",    0.1764,     CDC_Area        );      //                                          ***

                                                                // Meter/Sekunde ->
    NEWD( "m/s",    1.0000000000000000E00,  CDC_Speed       );      // Meter/Sekunde                            ***
    NEWD( "m/h",    3.6000000000000000E03,  CDC_Speed       );      // Meter/Stunde -> km/h                     ***
    NEWD( "mph",    2.2369362920544023E00,  CDC_Speed       );      // (Britsche) Meilen/Stunde                 ***
    NEWD( "kn",     1.9438444924406048E00,  CDC_Speed       );      // Knoten = (Nautische) Meilen/Stunde       ***
    NEWD( "admkn",  1.9438446603753486E00,  CDC_Speed       );      // Admiraltyknot                            ***

                                                                //
//  Append( new ConvertData( "",        ,   CDC_Length  ) );        //
//  NEWD( "",   ,       );      //
}


ConvertDataList::~ConvertDataList()
{
    for( ConvertData* p = First() ; p ; p = Next() )
        delete p;
}


double ConvertDataList::Convert( double fVal, const STRING& rFrom, const STRING& rTo ) THROWDEF_RTE_IAE
{
    if( rFrom == rTo )
        return fVal;

    ConvertData*    pFrom = NULL;
    ConvertData*    pTo = NULL;
    sal_Bool        bSearchFrom = sal_True;
    sal_Bool        bSearchTo = sal_True;
    sal_Int16       nLevelFrom = 0;
    sal_Int16       nLevelTo = 0;

    ConvertData*    p = First();
    while( p && ( bSearchFrom || bSearchTo ) )
    {
#define CHECKIT(part)

        if( bSearchFrom )
        {
            sal_Int16   n = p->GetMatchingLevel( rFrom );
            if( n != INV_MATCHLEV )
            {
                if( n )
                {   // only first match for partial equality rulz a little bit more
                    pFrom = p;
                    nLevelFrom = n;
                }
                else
                {   // ... but exact match rulz most
                    pFrom = p;
                    bSearchFrom = sal_False;
                    nLevelFrom = n;
                }
            }
        }

        if( bSearchTo )
        {
            sal_Int16   n = p->GetMatchingLevel( rTo );
            if( n != INV_MATCHLEV )
            {
                if( n )
                {   // only first match for partial equality rulz a little bit more
                    pTo = p;
                    nLevelTo = n;
                }
                else
                {   // ... but exact match rulz most
                    pTo = p;
                    bSearchTo = sal_False;
                    nLevelTo = n;
                }
            }
        }

        p = Next();
    }

    if( pFrom && pTo )
        return pFrom->Convert( fVal, *pTo, nLevelFrom, nLevelTo );
    else
        THROW_IAE;
}

