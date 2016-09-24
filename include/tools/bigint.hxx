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
#ifndef INCLUDED_TOOLS_BIGINT_HXX
#define INCLUDED_TOOLS_BIGINT_HXX

#include <climits>
#include <rtl/ustring.hxx>
#include <tools/toolsdllapi.h>
#include <tools/solar.h>

class SvStream;

#define MAX_DIGITS 8

class Fraction;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC BigInt
{
private:
    long            nVal;
    unsigned short  nNum[MAX_DIGITS];
    sal_uInt8       nLen        : 5;    // current length
    bool            bIsNeg      : 1,    // Is Sign negative?
                    bIsBig      : 1,    // sal_True == BigInt
                    bIsSet      : 1;    // Not "Null" (not "not 0")

    TOOLS_DLLPRIVATE void MakeBigInt(BigInt const &);
    TOOLS_DLLPRIVATE void Normalize();
    TOOLS_DLLPRIVATE void Mult(BigInt const &, sal_uInt16);
    TOOLS_DLLPRIVATE void Div(sal_uInt16, sal_uInt16 &);
    TOOLS_DLLPRIVATE bool IsLess(BigInt const &) const;
    TOOLS_DLLPRIVATE void AddLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void SubLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void MultLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE void DivLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE void ModLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE bool ABS_IsLess(BigInt const &) const;

public:
    BigInt()
        : nVal(0)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
        , bIsSet(false)
    {
    }

    BigInt(short nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
        , bIsSet(true)
    {
    }

    BigInt(long nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
        , bIsSet(true)
    {
    }

    BigInt(int nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
        , bIsSet(true)
    {
    }

    BigInt( double nVal );

    BigInt(sal_uInt16 nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
        , bIsSet(true)
    {
    }

    BigInt( sal_uInt32 nVal );
#if SAL_TYPES_SIZEOFLONG < SAL_TYPES_SIZEOFLONGLONG
    BigInt( long long nVal );
#endif
    BigInt( const BigInt& rBigInt );
    BigInt( const OUString& rString );

    operator        short() const;
    operator        long()  const;
    operator        int()   const;
    operator        double() const;
    operator        sal_uInt16() const;
    operator        sal_uIntPtr() const;

    bool            IsSet() const { return bIsSet; }
    bool            IsNeg() const;
    bool            IsZero() const;
    bool            IsLong() const { return !bIsBig; }

    void            Abs();

    BigInt&         operator  =( const BigInt& rVal );
    BigInt&         operator +=( const BigInt& rVal );
    BigInt&         operator -=( const BigInt& rVal );
    BigInt&         operator *=( const BigInt& rVal );
    BigInt&         operator /=( const BigInt& rVal );
    BigInt&         operator %=( const BigInt& rVal );

    BigInt&         operator  =( const short      nValue );
    BigInt&         operator  =( const long       nValue );
    BigInt&         operator  =( const int        nValue );
    BigInt&         operator  =( const sal_uInt16 nValue );

    friend inline   BigInt operator +( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator -( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator *( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator /( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator %( const BigInt& rVal1, const BigInt& rVal2 );

    TOOLS_DLLPUBLIC friend          bool operator==( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator!=( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend          bool operator< ( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend          bool operator> ( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator<=( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator>=( const BigInt& rVal1, const BigInt& rVal2 );

    friend class Fraction;
};

inline BigInt::operator short() const
{
    if ( !bIsBig && nVal >= SHRT_MIN && nVal <= SHRT_MAX )
        return (short)nVal;
    else
        return 0;
}

inline BigInt::operator long() const
{
    if ( !bIsBig )
        return nVal;
    else
        return 0;
}

inline BigInt::operator int() const
{
    if ( !bIsBig && (nVal == (long)(int)nVal) )
        return (int)nVal;
    else
        return 0;
}

inline BigInt::operator sal_uInt16() const
{
    if ( !bIsBig && nVal >= 0 && nVal <= (long)USHRT_MAX )
        return (sal_uInt16)nVal;
    else
        return 0;
}

inline BigInt& BigInt::operator =( const short nValue )
{
    bIsSet = true;
    bIsBig = false;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const long nValue )
{
    bIsSet = true;
    bIsBig = false;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const int nValue )
{
    bIsSet = true;
    bIsBig = false;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const sal_uInt16 nValue )
{
    bIsSet = true;
    bIsBig = false;
    nVal   = nValue;

    return *this;
}

inline bool BigInt::IsNeg() const
{
    if ( !bIsBig )
        return (nVal < 0);
    else
        return (bool)bIsNeg;
}

inline bool BigInt::IsZero() const
{
    if ( bIsBig )
        return false;
    else
        return (nVal == 0);
}

inline void BigInt::Abs()
{
    if ( bIsBig )
        bIsNeg = false;
    else if ( nVal < 0 )
        nVal = -nVal;
}

inline BigInt operator+( const BigInt &rVal1, const BigInt &rVal2 )
{
    BigInt aErg( rVal1 );
    aErg += rVal2;
    return aErg;
}

inline BigInt operator-( const BigInt &rVal1, const BigInt &rVal2 )
{
    BigInt aErg( rVal1 );
    aErg -= rVal2;
    return aErg;
}

inline BigInt operator*( const BigInt &rVal1, const BigInt &rVal2 )
{
    BigInt aErg( rVal1 );
    aErg *= rVal2;
    return aErg;
}

inline BigInt operator/( const BigInt &rVal1, const BigInt &rVal2 )
{
    BigInt aErg( rVal1 );
    aErg /= rVal2;
    return aErg;
}

inline BigInt operator%( const BigInt &rVal1, const BigInt &rVal2 )
{
    BigInt aErg( rVal1 );
    aErg %= rVal2;
    return aErg;
}

inline bool operator!=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !(rVal1 == rVal2);
}

inline bool operator<=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !( rVal1 > rVal2);
}

inline bool operator>=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !(rVal1 < rVal2);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
