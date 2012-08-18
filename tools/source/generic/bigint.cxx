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

#include <tools/tools.h>
#include <tools/bigint.hxx>
#include <tools/string.hxx>

#include <string.h>
#include <ctype.h>

static const long MY_MAXLONG  = 0x3fffffff;
static const long MY_MINLONG  = -MY_MAXLONG;
static const long MY_MAXSHORT = 0x00007fff;
static const long MY_MINSHORT = -MY_MAXSHORT;

/* Die ganzen Algorithmen zur Addition, Subtraktion, Multiplikation und
 * Division von langen Zahlen stammen aus SEMINUMERICAL ALGORITHMS von
 * DONALD E. KNUTH aus der Reihe The Art of Computer Programming. Zu finden
 * sind diese Algorithmen im Kapitel 4.3.1. The Classical Algorithms.
 */

// Muss auf sal_uInt16/INT16/sal_uInt32/sal_Int32 umgestellt werden !!! W.P.

void BigInt::MakeBigInt( const BigInt& rVal )
{
    if ( rVal.bIsBig )
    {
        memcpy( (void*)this, (const void*)&rVal, sizeof( BigInt ) );
        while ( nLen > 1 && nNum[nLen-1] == 0 )
            nLen--;
    }
    else
    {
        long nTmp = rVal.nVal;

        nVal   = rVal.nVal;
        bIsBig = sal_True;
        if ( nTmp < 0 )
        {
            bIsNeg = sal_True;
            nTmp = -nTmp;
        }
        else
            bIsNeg = sal_False;

        nNum[0] = (sal_uInt16)(nTmp & 0xffffL);
        nNum[1] = (sal_uInt16)(nTmp >> 16);
        if ( nTmp & 0xffff0000L )
            nLen = 2;
        else
            nLen = 1;
    }
}

void BigInt::Normalize()
{
    if ( bIsBig )
    {
        while ( nLen > 1 && nNum[nLen-1] == 0 )
            nLen--;

        if ( nLen < 3 )
        {
            if ( nLen < 2 )
                nVal = nNum[0];
            else if ( nNum[1] & 0x8000 )
                return;
            else
                nVal = ((long)nNum[1] << 16) + nNum[0];

            bIsBig = sal_False;

            if ( bIsNeg )
                nVal = -nVal;
        }
        // else ist nVal undefiniert !!! W.P.
    }
    // wozu, nLen ist doch undefiniert ??? W.P.
    else if ( nVal & 0xFFFF0000L )
        nLen = 2;
    else
        nLen = 1;
}

void BigInt::Mult( const BigInt &rVal, sal_uInt16 nMul )
{
    sal_uInt16 nK = 0;
    for ( int i = 0; i < rVal.nLen; i++ )
    {
        sal_uInt32 nTmp = (sal_uInt32)rVal.nNum[i] * (sal_uInt32)nMul + nK;
        nK            = (sal_uInt16)(nTmp >> 16);
        nNum[i] = (sal_uInt16)nTmp;
    }

    if ( nK )
    {
        nNum[rVal.nLen] = nK;
        nLen = rVal.nLen + 1;
    }
    else
        nLen = rVal.nLen;

    bIsBig = sal_True;
    bIsNeg = rVal.bIsNeg;
}

void BigInt::Div( sal_uInt16 nDiv, sal_uInt16& rRem )
{
    sal_uInt32 nK = 0;
    for ( int i = nLen - 1; i >= 0; i-- )
    {
        sal_uInt32 nTmp = (sal_uInt32)nNum[i] + (nK << 16);
        nNum[i] = (sal_uInt16)(nTmp / nDiv);
        nK            = nTmp % nDiv;
    }
    rRem = (sal_uInt16)nK;

    if ( nNum[nLen-1] == 0 )
        nLen -= 1;
}

sal_Bool BigInt::IsLess( const BigInt& rVal ) const
{
    if ( rVal.nLen < nLen)
        return sal_True;
    if ( rVal.nLen > nLen )
        return sal_False;

    int i;
    for ( i = nLen - 1; i > 0 && nNum[i] == rVal.nNum[i]; i-- )
    {
    }
    return rVal.nNum[i] < nNum[i];
}

void BigInt::AddLong( BigInt& rB, BigInt& rErg )
{
    if ( bIsNeg == rB.bIsNeg )
    {
        int  i;
        char len;

        // wenn die Zahlen unterschiedlich lang sind, sollte zunaechst bei
        // der kleineren Zahl die fehlenden Ziffern mit 0 initialisert werden
        if (nLen >= rB.nLen)
        {
            len = nLen;
            for (i = rB.nLen; i < len; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            len = rB.nLen;
            for (i = nLen; i < len; i++)
                nNum[i] = 0;
        }

        // Die Ziffern werden von hinten nach vorne addiert
        long k;
        long nZ = 0;
        for (i = 0, k = 0; i < len; i++) {
            nZ = (long)nNum[i] + (long)rB.nNum[i] + k;
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
            len++;
        }
        // Die Laenge und das Vorzeichen setzen
        rErg.nLen   = len;
        rErg.bIsNeg = bIsNeg && rB.bIsNeg;
        rErg.bIsBig = sal_True;
    }
    // Wenn nur einer der beiden Operanten negativ ist, wird aus der
    // Addition eine Subtaktion
    else if (bIsNeg)
    {
        bIsNeg = sal_False;
        rB.SubLong(*this, rErg);
        bIsNeg = sal_True;
    }
    else
    {
        rB.bIsNeg = sal_False;
        SubLong(rB, rErg);
        rB.bIsNeg = sal_True;
    }
}

void BigInt::SubLong( BigInt& rB, BigInt& rErg )
{
    if ( bIsNeg == rB.bIsNeg )
    {
        int  i;
        char len;
        long nZ, k;

        // wenn die Zahlen unterschiedlich lang sind, sollte zunaechst bei
        // der kleineren Zahl die fehlenden Ziffern mit 0 initialisert werden
        if (nLen >= rB.nLen)
        {
            len = nLen;
            for (i = rB.nLen; i < len; i++)
                rB.nNum[i] = 0;
        }
        else
        {
            len = rB.nLen;
            for (i = nLen; i < len; i++)
                nNum[i] = 0;
        }

        if ( IsLess(rB) )
        {
            for (i = 0, k = 0; i < len; i++)
            {
                nZ = (long)nNum[i] - (long)rB.nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            rErg.bIsNeg = bIsNeg;
        }
        else
        {
            for (i = 0, k = 0; i < len; i++)
            {
                nZ = (long)rB.nNum[i] - (long)nNum[i] + k;
                if (nZ < 0)
                    k = -1;
                else
                    k = 0;
                rErg.nNum[i] = (sal_uInt16)(nZ & 0xffffL);
            }
            // wenn a < b, dann Vorzeichen vom Ergebnis umdrehen
            rErg.bIsNeg = !bIsNeg;
        }
        rErg.nLen   = len;
        rErg.bIsBig = sal_True;
    }
    // Wenn nur einer der beiden Operanten negativ ist, wird aus der
    // Subtaktion eine Addition
    else if (bIsNeg)
    {
        bIsNeg = sal_False;
        AddLong(rB, rErg);
        bIsNeg = sal_True;
        rErg.bIsNeg = sal_True;
    }
    else
    {
        rB.bIsNeg = sal_False;
        AddLong(rB, rErg);
        rB.bIsNeg = sal_True;
        rErg.bIsNeg = sal_False;
    }
}

void BigInt::MultLong( const BigInt& rB, BigInt& rErg ) const
{
    int    i, j;
    sal_uInt32  nZ, k;

    rErg.bIsNeg = bIsNeg != rB.bIsNeg;
    rErg.bIsBig = sal_True;
    rErg.nLen   = nLen + rB.nLen;

    for (i = 0; i < rErg.nLen; i++)
        rErg.nNum[i] = 0;

    for (j = 0; j < rB.nLen; j++)
    {
        for (i = 0, k = 0; i < nLen; i++)
        {
            nZ = (sal_uInt32)nNum[i] * (sal_uInt32)rB.nNum[j] +
                 (sal_uInt32)rErg.nNum[i + j] + k;
            rErg.nNum[i + j] = (sal_uInt16)(nZ & 0xffffUL);
            k = nZ >> 16;
        }
        rErg.nNum[i + j] = (sal_uInt16)k;
    }
}

void BigInt::DivLong( const BigInt& rB, BigInt& rErg ) const
{
    int    i, j;
    long   nTmp;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    aTmpA.Mult( *this, nMult );
    if ( aTmpA.nLen == nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    aTmpB.Mult( rB, nMult );

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
        unsigned short& rNum( aTmpA.nNum[j - nLenB + i] );
        rNum = rNum - nK;   // MSVC yields a warning on -= here, so don't use it
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

    rErg.bIsNeg = bIsNeg != rB.bIsNeg;
    rErg.bIsBig = sal_True;
    rErg.nLen   = nLen - rB.nLen + 1;
}

void BigInt::ModLong( const BigInt& rB, BigInt& rErg ) const
{
    short  i, j;
    long   nTmp;
    sal_uInt16 nK, nQ, nMult;
    short  nLenB  = rB.nLen;
    short  nLenB1 = rB.nLen - 1;
    BigInt aTmpA, aTmpB;

    nMult = (sal_uInt16)(0x10000L / ((long)rB.nNum[nLenB1] + 1));

    aTmpA.Mult( *this, nMult);
    if ( aTmpA.nLen == nLen )
    {
        aTmpA.nNum[aTmpA.nLen] = 0;
        aTmpA.nLen++;
    }

    aTmpB.Mult( rB, nMult);

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
        unsigned short& rNum( aTmpA.nNum[j - nLenB + i] );
        rNum = rNum - nK;
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
    rErg.Div( nMult, nQ );
}

sal_Bool BigInt::ABS_IsLess( const BigInt& rB ) const
{
    if (bIsBig || rB.bIsBig)
    {
        BigInt nA, nB;
        nA.MakeBigInt( *this );
        nB.MakeBigInt( rB );
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
    if ( nVal < 0 )
        if ( rB.nVal < 0 )
            return nVal > rB.nVal;
        else
            return nVal > -rB.nVal;
    else
        if ( rB.nVal < 0 )
            return nVal < -rB.nVal;
        else
            return nVal < rB.nVal;
}

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

BigInt::BigInt( const rtl::OUString& rString )
{
    bIsSet = sal_True;
    bIsNeg = sal_False;
    bIsBig = sal_False;
    nVal   = 0;

    sal_Bool bNeg = sal_False;
    const sal_Unicode* p = rString.getStr();
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
            Normalize();
    }
}

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

BigInt::operator sal_uIntPtr() const
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

rtl::OUString BigInt::GetString() const
{
    String aString;

    if ( !bIsBig )
        aString = String::CreateFromInt32( nVal );
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

            String aStr = aString;
            if ( a.nVal < 100000000L )
            { // leading 0s
                aString = String::CreateFromInt32( a.nVal + 1000000000L );
                aString.Erase(0,1);
            }
            else
                aString = String::CreateFromInt32( a.nVal );
            aString += aStr;
        }
        while( aTmp.bIsBig );

        String aStr = aString;
        if ( bIsNeg )
            aString = String::CreateFromInt32( -aTmp.nVal );
        else
            aString = String::CreateFromInt32( aTmp.nVal );
        aString += aStr;
    }

    return aString;
}

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
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.AddLong( aTmp2, *this );
    Normalize();
    return *this;
}

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
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.SubLong( aTmp2, *this );
    Normalize();
    return *this;
}

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
        aTmp1.MakeBigInt( rVal );
        aTmp2.MakeBigInt( *this );
        aTmp1.MultLong(aTmp2, *this);
        Normalize();
    }
    return *this;
}

BigInt& BigInt::operator/=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            OSL_FAIL( "BigInt::operator/ --> divide by zero" );
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

            Div( nTmp, nTmp );
            Normalize();
            return *this;
        }
    }

    if ( ABS_IsLess( rVal ) )
    {
        *this = BigInt( (long)0 );
        return *this;
    }

    // BigInt durch BigInt teilen
    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.DivLong(aTmp2, *this);
    Normalize();
    return *this;
}

BigInt& BigInt::operator%=( const BigInt& rVal )
{
    if ( !rVal.bIsBig )
    {
        if ( rVal.nVal == 0 )
        {
            OSL_FAIL( "BigInt::operator/ --> divide by zero" );
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

            Div( nTmp, nTmp );
            *this = BigInt( (long)nTmp );
            return *this;
        }
    }

    if ( ABS_IsLess( rVal ) )
        return *this;

    // BigInt durch BigInt teilen
    BigInt aTmp1, aTmp2;
    aTmp1.MakeBigInt( *this );
    aTmp2.MakeBigInt( rVal );
    aTmp1.ModLong(aTmp2, *this);
    Normalize();
    return *this;
}

sal_Bool operator==( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
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

sal_Bool operator<( const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
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

sal_Bool operator >(const BigInt& rVal1, const BigInt& rVal2 )
{
    if ( rVal1.bIsBig || rVal2.bIsBig )
    {
        BigInt nA, nB;
        nA.MakeBigInt( rVal1 );
        nB.MakeBigInt( rVal2 );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
