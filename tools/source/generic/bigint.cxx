/*************************************************************************
 *
 *  $RCSfile: bigint.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:11:15 $
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
#include <tools.h>

#define private public
#include <bigint.hxx>
#undef private

#ifndef _STRING_HXX
#include <string.hxx>
#endif
#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif

#include <string.h>
#include <ctype.h>

static void SubLong( BigInt& rA, BigInt& rB, BigInt& rErg );

static const long MY_MAXLONG  = 0x3fffffff;
static const long MY_MINLONG  = -MY_MAXLONG;
static const long MY_MAXSHORT = 0x00007fff;
static const long MY_MINSHORT = -MY_MAXSHORT;

/* Die ganzen Algorithmen zur Addition, Subtraktion, Multiplikation und
 * Division von langen Zahlen stammen aus SEMINUMERICAL ALGORITHMS von
 * DONALD E. KNUTH aus der Reihe The Art of Computer Programming. Zu finden
 * sind diese Algorithmen im Kapitel 4.3.1. The Classical Algorithms.
 */

// Muss auf UINT16/INT16/UINT32/INT32 umgestellt werden !!! W.P.

// -----------------------------------------------------------------------

static void MakeBigInt( BigInt& rThis, const BigInt& rVal )
{
    if ( rVal.bIsBig )
    {
        memcpy( (void*)&rThis, (const void*)&rVal, sizeof( BigInt ) );
        while ( rThis.nLen > 1 && rThis.nNum[rThis.nLen-1] == 0 )
            rThis.nLen--;
    }
    else
    {
        long nTmp = rVal.nVal;

        rThis.nVal   = rVal.nVal;
        rThis.bIsBig = sal_True;
        if ( nTmp < 0 )
        {
            rThis.bIsNeg = sal_True;
            nTmp = -nTmp;
        }
        else
            rThis.bIsNeg = sal_False;

        rThis.nNum[0] = (sal_uInt16)(nTmp & 0xffffL);
        rThis.nNum[1] = (sal_uInt16)(nTmp >> 16);
#ifndef _WIN16
        if ( nTmp & 0xffff0000L )
#else
        long l = 0xffff0000L;
        if ( nTmp & l )
#endif
            rThis.nLen = 2;
        else
            rThis.nLen = 1;
    }
}

// -----------------------------------------------------------------------

static void Normalize( BigInt& rThis )
{
    if ( rThis.bIsBig )
    {
        while ( rThis.nLen > 1 && rThis.nNum[rThis.nLen-1] == 0 )
            rThis.nLen--;

        if ( rThis.nLen < 3 )
        {
            if ( rThis.nLen < 2 )
                rThis.nVal = rThis.nNum[0];
            else if ( rThis.nNum[1] & 0x8000 )
                return;
            else
                rThis.nVal = ((long)rThis.nNum[1] << 16) + rThis.nNum[0];

            rThis.bIsBig = sal_False;

            if ( rThis.bIsNeg )
                rThis.nVal = -rThis.nVal;
        }
        // else ist nVal undefiniert !!! W.P.
    }
    // wozu, nLen ist doch undefiniert ??? W.P.
    else if ( rThis.nVal & 0xFFFF0000L )
        rThis.nLen = 2;
    else
        rThis.nLen = 1;
}

// -----------------------------------------------------------------------

static void Mult( BigInt& rThis, const BigInt &rVal, sal_uInt16 nMul )
{
    sal_uInt16 nK = 0;
    for ( int i = 0; i < rVal.nLen; i++ )
    {
        sal_uInt32 nTmp = (sal_uInt32)rVal.nNum[i] * (sal_uInt32)nMul + nK;
        nK            = (sal_uInt16)(nTmp >> 16);
        rThis.nNum[i] = (sal_uInt16)nTmp;
    }

    if ( nK )
    {
        rThis.nNum[rVal.nLen] = nK;
        rThis.nLen = rVal.nLen + 1;
    }
    else
        rThis.nLen = rVal.nLen;

    rThis.bIsBig = sal_True;
    rThis.bIsNeg = rVal.bIsNeg;
}

// -----------------------------------------------------------------------

static void Div( BigInt& rThis, sal_uInt16 nDiv, sal_uInt16& rRem )
{
    sal_uInt32 nK = 0;
    for ( int i = rThis.nLen - 1; i >= 0; i-- )
    {
        sal_uInt32 nTmp = (sal_uInt32)rThis.nNum[i] + (nK << 16);
        rThis.nNum[i] = (sal_uInt16)(nTmp / nDiv);
        nK            = nTmp % nDiv;
    }
    rRem = (sal_uInt16)nK;

    if ( rThis.nNum[rThis.nLen-1] == 0 )
        rThis.nLen -= 1;
}

// -----------------------------------------------------------------------

static sal_Bool IsLess( const BigInt& rThis, const BigInt& rVal )
{
    if ( rVal.nLen < rThis.nLen)
        return sal_True;
    if ( rVal.nLen > rThis.nLen )
        return sal_False;

    int i;
    for ( i = rThis.nLen - 1; i > 0 && rThis.nNum[i] == rVal.nNum[i]; i-- )
    {
    }
    return rVal.nNum[i] < rThis.nNum[i];
}

// -----------------------------------------------------------------------

static void AddLong( BigInt& rA, BigInt& rB, BigInt& rErg )
{
    if ( rA.bIsNeg == rB.bIsNeg )
    {
        int  i;
        char nLen;

        // wenn die Zahlen unterschiedlich lang sind, sollte zunaechst bei
        // der kleineren Zahl die fehlenden Ziffern mit 0 initialisert werden
        if (rA.nLen >= rB.nLen)
        {
            nLen = rA.nLen;
            for (i = rB.nLen; i < nLen; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            nLen = rB.nLen;
            for (i = rA.nLen; i < nLen; i++)
                rA.nNum[i] = 0;
        }

        // Die Ziffern werden von hinten nach vorne addiert
        long k;
        long nZ = 0;
        for (i = 0, k = 0; i < nLen; i++) {
            nZ = (long)rA.nNum[i] + (long)rB.nNum[i] + k;
            if (nZ & 0xff0000L)
                k = 1;
            else
                k = 0;
            rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
        }
        // Trat nach der letzten Addition ein Ueberlauf auf, muss dieser
        // noch ins Ergebis uebernommen werden
        if (nZ & 0xff0000L) // oder if(k)
        {
            rErg.nNum[i] = 1;
            nLen++;
        }
        // Die Laenge und das Vorzeichen setzen
        rErg.nLen   = nLen;
        rErg.bIsNeg = rA.bIsNeg && rB.bIsNeg;
        rErg.bIsBig = sal_True;
    }
    // Wenn nur einer der beiden Operanten negativ ist, wird aus der
    // Addition eine Subtaktion
    else if (rA.bIsNeg)
    {
        rA.bIsNeg = sal_False;
        SubLong(rB, rA, rErg);
        rA.bIsNeg = sal_True;
    }
    else
    {
        rB.bIsNeg = sal_False;
        SubLong(rA, rB, rErg);
        rB.bIsNeg = sal_True;
    }
}

// -----------------------------------------------------------------------

static void SubLong( BigInt& rA, BigInt& rB, BigInt& rErg )
{
    if ( rA.bIsNeg == rB.bIsNeg )
    {
        int  i;
        char nLen;
        long nZ, k;

        // wenn die Zahlen unterschiedlich lang sind, sollte zunaechst bei
        // der kleineren Zahl die fehlenden Ziffern mit 0 initialisert werden
        if (rA.nLen >= rB.nLen)
        {
            nLen = rA.nLen;
            for (i = rB.nLen; i < nLen; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            nLen = rB.nLen;
            for (i = rA.nLen; i < nLen; i++)
                rA.nNum[i] = 0;
        }

        if ( IsLess(rA, rB) )
        {
            for (i = 0, k = 0; i < nLen; i++)
            {
                nZ = (long)rA.nNum[i] - (long)rB.nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            rErg.bIsNeg = rA.bIsNeg;
        }
        else
        {
            for (i = 0, k = 0; i < nLen; i++)
            {
                nZ = (long)rB.nNum[i] - (long)rA.nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            // wenn a < b, dann Vorzeichen vom Ergebnis umdrehen
            rErg.bIsNeg = !rA.bIsNeg;
        }
        rErg.nLen   = nLen;
        rErg.bIsBig = sal_True;
    }
    // Wenn nur einer der beiden Operanten negativ ist, wird aus der
    // Subtaktion eine Addition
    else if (rA.bIsNeg)
    {
        rA.bIsNeg = sal_False;
        AddLong(rA, rB, rErg);
        rA.bIsNeg = sal_True;
        rErg.bIsNeg = sal_True;
    }
    else
    {
        rB.bIsNeg = sal_False;
        AddLong(rA, rB, rErg);
        rB.bIsNeg = sal_True;
        rErg.bIsNeg = sal_False;
    }
}

// -----------------------------------------------------------------------

static void MultLong( const BigInt& rA, const BigInt& rB, BigInt& rErg )
{
    int    i, j;
    sal_uInt32  nZ, k;

    rErg.bIsNeg = rA.bIsNeg != rB.bIsNeg;
    rErg.bIsBig = sal_True;
    rErg.nLen   = rA.nLen + rB.nLen;

    for (i = 0; i < rErg.nLen; i++)
        rErg.nNum[i] = 0;

    for (j = 0; j < rB.nLen; j++)
    {
        for (i = 0, k = 0; i < rA.nLen; i++)
        {
            nZ = (sal_uInt32)rA.nNum[i] * (sal_uInt32)rB.nNum[j] +
                 (sal_uInt32)rErg.nNum[i + j] + k;
            rErg.nNum[i + j] = (sal_uInt16)(nZ & 0xffffUL);
            k = nZ >> 16;
        }
        rErg.nNum[i + j] = (sal_uInt16)k;
    }
}

// -----------------------------------------------------------------------

static void DivLong( const BigInt& rA, const BigInt& rB, BigInt& rErg )
{
    int    i, j;
    long   nTmp;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    Mult( aTmpA, rA, nMult );
    if ( aTmpA.nLen == rA.nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    Mult( aTmpB, rB, nMult );

    for (j = aTmpA.nLen - 1; j >= nLenB; j--)
    { // Raten des Divisors
        nTmp = ( (long)aTmpA.nNum[j] << 16 ) + aTmpA.nNum[j - 1];
        if (aTmpA.nNum[j] == aTmpB.nNum[nLenB1])
            nQ = 0xFFFF;
        else
            nQ = (sal_uInt16)(((sal_uInt32)nTmp) / aTmpB.nNum[nLenB1]);

        if ( ((sal_uInt32)aTmpB.nNum[nLenB1 - 1] * nQ) >
            ((((sal_uInt32)nTmp) - aTmpB.nNum[nLenB1] * nQ) << 16) + aTmpA.nNum[j - 2])
            nQ--;
        // Und hier faengt das Teilen an
        nK = 0;
        nTmp = 0;
        for (i = 0; i < nLenB; i++)
        {
            nTmp = (long)aTmpA.nNum[j - nLenB + i]
                   - ((long)aTmpB.nNum[i] * nQ)
                   - nK;
            aTmpA.nNum[j - nLenB + i] = (sal_uInt16)nTmp;
            nK = (sal_uInt16) (nTmp >> 16);
            if ( nK )
                nK = (sal_uInt16)(0x10000UL - nK);
        }
        aTmpA.nNum[j - nLenB + i] -= nK;
        if (aTmpA.nNum[j - nLenB + i] == 0)
            rErg.nNum[j - nLenB] = nQ;
        else
        {
            rErg.nNum[j - nLenB] = nQ - 1;
            nK = 0;
            for (i = 0; i < nLenB; i++)
            {
                nTmp = aTmpA.nNum[j - nLenB + i] + aTmpB.nNum[i] + nK;
                aTmpA.nNum[j - nLenB + i] = (sal_uInt16)(nTmp & 0xFFFFL);
                if (nTmp & 0xFFFF0000L)
                    nK = 1;
                else
                    nK = 0;
            }
        }
    }

    rErg.bIsNeg = rA.bIsNeg != rB.bIsNeg;
    rErg.bIsBig = sal_True;
    rErg.nLen   = rA.nLen - rB.nLen + 1;
}

// -----------------------------------------------------------------------

static void ModLong( const BigInt& rA, const BigInt& rB, BigInt& rErg )
{
    short  i, j;
    long   nTmp;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    Mult( aTmpA, rA, nMult);
    if ( aTmpA.nLen == rA.nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    Mult( aTmpB, rB, nMult);

    for (j = aTmpA.nLen - 1; j >= nLenB; j--)
    { // Raten des Divisors
        nTmp = ( (long)aTmpA.nNum[j] << 16 ) + aTmpA.nNum[j - 1];
        if (aTmpA.nNum[j] == aTmpB.nNum[nLenB1])
            nQ = 0xFFFF;
        else
            nQ = (sal_uInt16)(((sal_uInt32)nTmp) / aTmpB.nNum[nLenB1]);

        if ( ((sal_uInt32)aTmpB.nNum[nLenB1 - 1] * nQ) >
            ((((sal_uInt32)nTmp) - aTmpB.nNum[nLenB1] * nQ) << 16) + aTmpA.nNum[j - 2])
            nQ--;
        // Und hier faengt das Teilen an
        nK = 0;
        nTmp = 0;
        for (i = 0; i < nLenB; i++)
        {
            nTmp = (long)aTmpA.nNum[j - nLenB + i]
                   - ((long)aTmpB.nNum[i] * nQ)
                   - nK;
            aTmpA.nNum[j - nLenB + i] = (sal_uInt16)nTmp;
            nK = (sal_uInt16) (nTmp >> 16);
            if ( nK )
                nK = (sal_uInt16)(0x10000UL - nK);
        }
        aTmpA.nNum[j - nLenB + i] -= nK;
        if (aTmpA.nNum[j - nLenB + i] == 0)
            rErg.nNum[j - nLenB] = nQ;
        else
        {
            rErg.nNum[j - nLenB] = nQ - 1;
            nK = 0;
            for (i = 0; i < nLenB; i++) {
                nTmp = aTmpA.nNum[j - nLenB + i] + aTmpB.nNum[i] + nK;
                aTmpA.nNum[j - nLenB + i] = (sal_uInt16)(nTmp & 0xFFFFL);
                if (nTmp & 0xFFFF0000L)
                    nK = 1;
                else
                    nK = 0;
            }
        }
    }

    rErg = aTmpA;
    Div( rErg, nMult, nQ );
}

// -----------------------------------------------------------------------

static sal_Bool ABS_IsLess( const BigInt& rA, const BigInt& rB )
{
    if (rA.bIsBig || rB.bIsBig)
    {
        BigInt nA, nB;
        MakeBigInt( nA, rA );
        MakeBigInt( nB, rB );
        if (nA.nLen == nB.nLen)
        {
            int i;
            for (i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i--)
            {
            }
            return nA.nNum[i] < nB.nNum[i];
        }
        else
            return nA.nLen < nB.nLen;
    }
    if ( rA.nVal < 0 )
        if ( rB.nVal < 0 )
            return rA.nVal > rB.nVal;
        else
            return rA.nVal > -rB.nVal;
    else
        if ( rB.nVal < 0 )
            return rA.nVal < -rB.nVal;
        else
            return rA.nVal < rB.nVal;
}

// -----------------------------------------------------------------------

BigInt::BigInt( const BigInt& rBigInt )
{
    if ( rBigInt.bIsBig )
        memcpy( (void*)this, (const void*)&rBigInt, sizeof( BigInt ) );
    else
    {
        bIsSet = rBigInt.bIsSet;
        bIsBig = sal_False;
        nVal   = rBigInt.nVal;
    }
}

// -----------------------------------------------------------------------

BigInt::BigInt( const ByteString& rString )
{
    bIsSet = sal_True;
    bIsNeg = sal_False;
    bIsBig = sal_False;
    nVal   = 0;

    sal_Bool bNeg = sal_False;
    const sal_Char* p = rString.GetBuffer();
    if ( *p == '-' )
    {
        bNeg = sal_True;
        p++;
    }
    while( *p >= '0' && *p <= '9' )
    {
        *this *= 10;
        *this += *p - '0';
        p++;
    }
    if ( bIsBig )
        bIsNeg = bNeg;
    else if( bNeg )
        nVal = -nVal;
}

// -----------------------------------------------------------------------

BigInt::BigInt( const UniString& rString )
{
    bIsSet = sal_True;
    bIsNeg = sal_False;
    bIsBig = sal_False;
    nVal   = 0;

    sal_Bool bNeg = sal_False;
    const sal_Unicode* p = rString.GetBuffer();
    if ( *p == '-' )
    {
        bNeg = sal_True;
        p++;
    }
    while( *p >= '0' && *p <= '9' )
    {
        *this *= 10;
        *this += *p - '0';
        p++;
    }
    if ( bIsBig )
        bIsNeg = bNeg;
    else if( bNeg )
        nVal = -nVal;
}

// -----------------------------------------------------------------------

BigInt::BigInt( double nValue )
{
    bIsSet = sal_True;

    if ( nValue < 0 )
    {
        nValue *= -1;
        bIsNeg  = sal_True;
    }
    else
    {
        bIsNeg  = sal_False;
    }

    if ( nValue < 1 )
    {
        bIsBig = sal_False;
        nVal   = 0;
    }
    else
    {
        bIsBig = sal_True;

        int i=0;

        while ( ( nValue > 65536.0 ) && ( i < MAX_DIGITS ) )
        {
            nNum[i] = (sal_uInt16) fmod( nValue, 65536.0 );
            nValue -= nNum[i];
            nValue /= 65536.0;
            i++;
        }
        if ( i < MAX_DIGITS )
            nNum[i++] = (sal_uInt16) nValue;

        nLen = i;

        if ( i < 3 )
            Normalize( *this );
    }
}

// -----------------------------------------------------------------------

BigInt::BigInt( sal_uInt32 nValue )
{
    bIsSet  = sal_True;
    if ( nValue & 0x80000000UL )
    {
        bIsBig  = sal_True;
        bIsNeg  = sal_False;
        nNum[0] = (sal_uInt16)(nValue & 0xffffUL);
        nNum[1] = (sal_uInt16)(nValue >> 16);
        nLen    = 2;
    }
    else
    {
        bIsBig = sal_False;
        nVal   = nValue;
    }
}

// -----------------------------------------------------------------------

BigInt::operator ULONG() const
{
    if ( !bIsBig )
        return (sal_uInt32)nVal;
    else if ( nLen == 2 )
    {
        sal_uInt32 nRet;
        nRet  = ((sal_uInt32)nNum[1]) << 16;
        nRet += nNum[0];
        return nRet;
    }
    return 0;
}

// -----------------------------------------------------------------------

BigInt::operator double() const
{
    if ( !bIsBig )
        return (double) nVal;
    else
    {
        int     i = nLen-1;
        double  nRet = (double) ((sal_uInt32)nNum[i]);

        while ( i )
        {
            nRet *= 65536.0;
            i--;
            nRet += (double) ((sal_uInt32)nNum[i]);
        }

        if ( bIsNeg )
            nRet *= -1;

        return nRet;
    }
}

// -----------------------------------------------------------------------

ByteString BigInt::GetByteString() const
{
    ByteString aString;

    if ( !bIsBig )
        aString = ByteString::CreateFromInt32( nVal );
    else
    {
        BigInt aTmp( *this );
        BigInt a1000000000( 1000000000L );
        aTmp.Abs();

        do
        {
            BigInt a = aTmp;
            a    %= a1000000000;
            aTmp /= a1000000000;

            ByteString aStr = aString;
            if ( a.nVal < 100000000L )
            { // leading 0s
                aString = ByteString::CreateFromInt32( a.nVal + 1000000000L );
                aString.Erase( 0, 1 );
            }
            else
                aString = ByteString::CreateFromInt32( a.nVal );
            aString += aStr;
        }
        while( aTmp.bIsBig );

        ByteString aStr = aString;
        if ( bIsNeg )
            aString = ByteString::CreateFromInt32( -aTmp.nVal );
        else
            aString = ByteString::CreateFromInt32( aTmp.nVal );
        aString += aStr;
    }

    return aString;
}

// -----------------------------------------------------------------------

UniString BigInt::GetString() const
{
    UniString aString;

    if ( !bIsBig )
        aString = UniString::CreateFromInt32( nVal );
    else
    {
        BigInt aTmp( *this );
        BigInt a1000000000( 1000000000L );
        aTmp.Abs();

        do
        {
            BigInt a = aTmp;
            a    %= a1000000000;
            aTmp /= a1000000000;

            UniString aStr = aString;
            if ( a.nVal < 100000000L )
            { // leading 0s
                aString = UniString::CreateFromInt32( a.nVal + 1000000000L );
                aString.Erase(0,1);
            }
            else
                aString = UniString::CreateFromInt32( a.nVal );
            aString += aStr;
        }
        while( aTmp.bIsBig );

        UniString aStr = aString;
        if ( bIsNeg )
            aString = UniString::CreateFromInt32( -aTmp.nVal );
        else
            aString = UniString::CreateFromInt32( aTmp.nVal );
        aString += aStr;
    }

    return aString;
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator=( const BigInt& rBigInt )
{
    if ( rBigInt.bIsBig )
        memcpy( (void*)this, (const void*)&rBigInt, sizeof( BigInt ) );
    else
    {
        bIsSet = rBigInt.bIsSet;
        bIsBig = sal_False;
        nVal   = rBigInt.nVal;
    }
    return *this;
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator+=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig )
    {
        if( nVal <= MY_MAXLONG && rVal.nVal <= MY_MAXLONG
            && nVal >= MY_MINLONG && rVal.nVal >= MY_MINLONG )
        { // wir bewegen uns im ungefaehrlichem Bereich
            nVal += rVal.nVal;
            return *this;
        }

        if( (nVal < 0) != (rVal.nVal < 0) )
        { // wir bewegen uns im ungefaehrlichem Bereich
            nVal += rVal.nVal;
            return *this;
        }
    }

    BigInt aTmp1, aTmp2;
    MakeBigInt( aTmp1, *this );
    MakeBigInt( aTmp2, rVal );
    AddLong( aTmp1, aTmp2, *this );
    Normalize( *this );
    return *this;
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator-=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig )
    {
        if ( nVal <= MY_MAXLONG && rVal.nVal <= MY_MAXLONG &&
             nVal >= MY_MINLONG && rVal.nVal >= MY_MINLONG )
        { // wir bewegen uns im ungefaehrlichem Bereich
            nVal -= rVal.nVal;
            return *this;
        }

        if ( (nVal < 0) == (rVal.nVal < 0) )
        { // wir bewegen uns im ungefaehrlichem Bereich
            nVal -= rVal.nVal;
            return *this;
        }
    }

    BigInt aTmp1, aTmp2;
    MakeBigInt( aTmp1, *this );
    MakeBigInt( aTmp2, rVal );
    SubLong( aTmp1, aTmp2, *this );
    Normalize( *this );
    return *this;
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator*=( const BigInt& rVal )
{
    if ( !bIsBig && !rVal.bIsBig
         && nVal <= MY_MAXSHORT && rVal.nVal <= MY_MAXSHORT
         && nVal >= MY_MINSHORT && rVal.nVal >= MY_MINSHORT )
         // nicht optimal !!! W.P.
    { // wir bewegen uns im ungefaehrlichem Bereich
        nVal *= rVal.nVal;
    }
    else
    {
        BigInt aTmp1, aTmp2;
        MakeBigInt( aTmp1, rVal );
        MakeBigInt( aTmp2, *this );
        MultLong(aTmp1, aTmp2, *this);
        Normalize( *this );
    }
    return *this;
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator/=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            DBG_ERROR( "BigInt::operator/ --> divide by zero" );
            return *this;
        }

        if ( !bIsBig )
        {
            // wir bewegen uns im ungefaehrlichem Bereich
            nVal /= rVal.nVal;
            return *this;
        }

        if ( rVal.nVal == 1 )
            return *this;

        if ( rVal.nVal == -1 )
        {
            bIsNeg = !bIsNeg;
            return *this;
        }

        if ( rVal.nVal <= (long)0xFFFF && rVal.nVal >= -(long)0xFFFF )
        {
            // ein BigInt durch ein sal_uInt16 teilen
            sal_uInt16 nTmp;
            if ( rVal.nVal < 0 )
            {
                nTmp = (sal_uInt16) -rVal.nVal;
                bIsNeg = !bIsNeg;
            }
            else
                nTmp = (sal_uInt16) rVal.nVal;

            Div( *this, nTmp, nTmp );
            Normalize( *this );
            return *this;
        }
    }

    if ( ABS_IsLess( *this, rVal ) )
    {
        *this = BigInt( (long)0 );
        return *this;
    }

    // BigInt durch BigInt teilen
    BigInt aTmp1, aTmp2;
    MakeBigInt( aTmp1, *this );
    MakeBigInt( aTmp2, rVal );
    DivLong(aTmp1, aTmp2, *this);
    Normalize( *this );
    return *this;
}

// -----------------------------------------------------------------------

void BigInt::DivMod( const BigInt& rVal, BigInt& rMod )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            DBG_ERROR( "BigInt::operator/ --> divide by zero" );
            return;
        }

        if ( !bIsBig )
        {
            // wir bewegen uns im ungefaehrlichem Bereich
            rMod  = BigInt( nVal % rVal.nVal );
            nVal /= rVal.nVal;
            return;
        }

        if ( rVal.nVal == 1 )
        {
            rMod = BigInt( (long)0 );
            return;
        }

        if ( rVal.nVal == -1 )
        {
            rMod = BigInt( (long)0 );
            bIsNeg = !bIsNeg;
            return;
        }

        if ( rVal.nVal <= (long)0xFFFF && rVal.nVal >= -(long)0xFFFF )
        {
            // ein BigInt durch ein sal_uInt16 teilen
            sal_uInt16 nTmp;
            if ( rVal.nVal < 0 )
            {
                nTmp = (sal_uInt16) -rVal.nVal;
                bIsNeg = !bIsNeg;
            }
            else
                nTmp = (sal_uInt16) rVal.nVal;

            Div( *this, nTmp, nTmp );
            rMod = BigInt( (long)nTmp );
            Normalize( *this );
            return;
        }
    }

    if ( ABS_IsLess( *this, rVal ) )
    {
        rMod  = *this;
        *this = BigInt( (long)0 );
        return;
    }

    // BigInt durch BigInt teilen
    BigInt aTmp1, aTmp2;
    MakeBigInt( aTmp1, *this );
    MakeBigInt( aTmp2, rVal );
    DivLong(aTmp1, aTmp2, *this);
    Normalize( *this );
    ModLong(aTmp1, aTmp2, rMod); // nicht optimal
    Normalize( rMod );
}

// -----------------------------------------------------------------------

BigInt& BigInt::operator%=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            DBG_ERROR( "BigInt::operator/ --> divide by zero" );
            return *this;
        }

        if ( !bIsBig )
        {
            // wir bewegen uns im ungefaehrlichem Bereich
            nVal %= rVal.nVal;
            return *this;
        }

        if ( rVal.nVal <= (long)0xFFFF && rVal.nVal >= -(long)0xFFFF )
        {
            // ein BigInt durch ein short teilen
            sal_uInt16 nTmp;
            if ( rVal.nVal < 0 )
            {
                nTmp = (sal_uInt16) -rVal.nVal;
                bIsNeg = !bIsNeg;
            }
            else
                nTmp = (sal_uInt16) rVal.nVal;

            Div( *this, nTmp, nTmp );
            *this = BigInt( (long)nTmp );
            return *this;
        }
    }

    if ( ABS_IsLess( *this, rVal ) )
        return *this;

    // BigInt durch BigInt teilen
    BigInt aTmp1, aTmp2;
    MakeBigInt( aTmp1, *this );
    MakeBigInt( aTmp2, rVal );
    ModLong(aTmp1, aTmp2, *this);
    Normalize( *this );
    return *this;
}

// -----------------------------------------------------------------------

sal_Bool operator==( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        MakeBigInt( nA, rVal1 );
        MakeBigInt( nB, rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                return nA.nNum[i] == nB.nNum[i];
            }
            return sal_False;
        }
        return sal_False;
    }
    return rVal1.nVal == rVal2.nVal;
}

// -----------------------------------------------------------------------

sal_Bool operator<( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        MakeBigInt( nA, rVal1 );
        MakeBigInt( nB, rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                if ( nA.bIsNeg )
                    return nA.nNum[i] > nB.nNum[i];
                else
                    return nA.nNum[i] < nB.nNum[i];
            }
            if ( nA.bIsNeg )
                return nA.nLen > nB.nLen;
            else
                return nA.nLen < nB.nLen;
        }
        return !nB.bIsNeg;
    }
    return rVal1.nVal < rVal2.nVal;
}

// -----------------------------------------------------------------------

sal_Bool operator >(const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        MakeBigInt( nA, rVal1 );
        MakeBigInt( nB, rVal2 );
        if ( nA.bIsNeg == nB.bIsNeg )
        {
            if ( nA.nLen == nB.nLen )
            {
                int i;
                for ( i = nA.nLen - 1; i > 0 && nA.nNum[i] == nB.nNum[i]; i-- )
                {
                }

                if ( nA.bIsNeg )
                    return nA.nNum[i] < nB.nNum[i];
                else
                    return nA.nNum[i] > nB.nNum[i];
            }
            if ( nA.bIsNeg )
                return nA.nLen < nB.nLen;
            else
                return nA.nLen > nB.nLen;
        }
        return !nA.bIsNeg;
    }

    return rVal1.nVal > rVal2.nVal;
}
