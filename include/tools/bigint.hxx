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

#include <rtl/ustring.hxx>
#include <tools/toolsdllapi.h>
#include <tools/long.hxx>

#define MAX_DIGITS 8

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC BigInt
{
private:
    // we only use one of these two fields at a time
    union {
        sal_Int32       nVal;
        sal_uInt16      nNum[MAX_DIGITS];
    };
    sal_uInt8       nLen        : 5;    // current length
    bool            bIsNeg      : 1,    // Is Sign negative?
                    bIsBig      : 1;    // if true , value is in nNum array

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
    {
    }

    BigInt(sal_Int32 nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
    {
    }

#if SAL_TYPES_SIZEOFLONG == 4
    BigInt(int nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
        , bIsBig(false)
    {
    }
#endif

    BigInt( double nVal );
    BigInt( sal_uInt32 nVal );
    BigInt( sal_Int64 nVal );
    BigInt( const BigInt& rBigInt );
    BigInt( const OUString& rString );

    operator        sal_Int16() const;
    operator        sal_uInt16() const;
    operator        sal_Int32() const;
    operator        sal_uInt32() const;
    operator        double() const;
#if SAL_TYPES_SIZEOFPOINTER == 8
    operator        tools::Long() const;
#endif

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

    BigInt&         operator  =( sal_Int32 nValue );

    friend inline   BigInt operator +( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator -( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator *( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator /( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   BigInt operator %( const BigInt& rVal1, const BigInt& rVal2 );

    TOOLS_DLLPUBLIC friend          bool operator==( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator!=( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend          bool operator< ( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator> ( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator<=( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   bool operator>=( const BigInt& rVal1, const BigInt& rVal2 );

    friend class Fraction;
};

inline BigInt::operator sal_Int16() const
{
    if ( !bIsBig && nVal >= SAL_MIN_INT16 && nVal <= SAL_MAX_INT16 )
        return static_cast<sal_Int16>(nVal);
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_uInt16() const
{
    if ( !bIsBig && nVal >= 0 && nVal <= SAL_MAX_UINT16 )
        return static_cast<sal_uInt16>(nVal);
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_Int32() const
{
    if (!bIsBig)
        return nVal;
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_uInt32() const
{
    if ( !bIsBig && nVal >= 0 )
        return static_cast<sal_uInt32>(nVal);
    assert(false && "out of range");
    return 0;
}

#if SAL_TYPES_SIZEOFPOINTER == 8
inline BigInt::operator tools::Long() const
{
    // Clamp to int32 since long is int32 on Windows.
    if (!bIsBig)
        return nVal;
    assert(false && "out of range");
    return 0;
}
#endif

inline BigInt& BigInt::operator =( sal_Int32 nValue )
{
    bIsBig = false;
    nVal   = nValue;

    return *this;
}

inline bool BigInt::IsNeg() const
{
    if ( !bIsBig )
        return (nVal < 0);
    else
        return bIsNeg;
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

inline bool operator>(const BigInt& rVal1, const BigInt& rVal2) { return rVal2 < rVal1; }

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
