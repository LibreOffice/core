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

#include <tools/toolsdllapi.h>
#include <tools/long.hxx>

#include <cassert>
#include <compare>
#include <limits>
#include <string_view>

#define MAX_DIGITS 4

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC BigInt
{
private:
    // we only use one of these two fields at a time
    union {
        sal_Int32       nVal;
        sal_uInt32      nNum[MAX_DIGITS];
    };
    sal_uInt8       nLen;    // current length, if 0, data is in nVal, otherwise data is in nNum
    bool            bIsNeg;    // Is Sign negative?

    TOOLS_DLLPRIVATE BigInt MakeBig() const;
    TOOLS_DLLPRIVATE void Normalize();
    TOOLS_DLLPRIVATE bool ABS_IsLessLong(BigInt const &) const;
    TOOLS_DLLPRIVATE void AddLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void SubLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void MultLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE void DivModLong(BigInt const &, BigInt &, bool) const;
    TOOLS_DLLPRIVATE void DivMod(BigInt const &, bool);
    TOOLS_DLLPRIVATE bool ABS_IsLess(BigInt const &) const;

public:
    BigInt()
        : nVal(0)
        , nLen(0)
        , bIsNeg(false)
    {
    }

    BigInt(sal_Int32 nValue)
        : nVal(nValue)
        , nLen(0)
        , bIsNeg(false)
    {
    }

    BigInt( double nVal );
    BigInt( sal_uInt32 nVal );
    BigInt( sal_Int64 nVal );
    BigInt( const BigInt& rBigInt );
    BigInt( std::u16string_view rString );

    template <typename N>
        requires(std::is_integral_v<N> && std::is_signed_v<N> && sizeof(N) <= sizeof(sal_Int32))
    BigInt(N val)
        : BigInt(sal_Int32(val))
    {
    }

    template <typename N>
        requires(std::is_integral_v<N> && std::is_unsigned_v<N> && sizeof(N) <= sizeof(sal_uInt32))
    BigInt(N val)
        : BigInt(sal_uInt32(val))
    {
    }

    template <typename N>
        requires(std::is_integral_v<N> && std::is_signed_v<N> && sizeof(N) == sizeof(sal_Int64))
    BigInt(N val)
        : BigInt(sal_Int64(val))
    {
    }

    operator        sal_Int16() const;
    operator        sal_uInt16() const;
    operator        sal_Int32() const;
    operator        sal_uInt32() const;
    operator        double() const;
    operator        sal_Int64() const;

    bool            IsNeg() const { return !IsLong() ? bIsNeg : nVal < 0; }
    bool            IsZero() const { return IsLong() && nVal == 0; }
    bool            IsLong() const { return nLen == 0; }

    void            Abs();

    BigInt&         operator  =( const BigInt& rVal );
    BigInt&         operator +=( const BigInt& rVal );
    BigInt&         operator -=( const BigInt& rVal );
    BigInt&         operator *=( const BigInt& rVal );
    BigInt&         operator /=( const BigInt& rVal );
    BigInt&         operator %=( const BigInt& rVal );

    BigInt&         operator  =( sal_Int32 nValue );

    /* Scale and round value */
    static tools::Long Scale(tools::Long nVal, tools::Long nMult, tools::Long nDiv);

    TOOLS_DLLPUBLIC friend          bool operator==( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend std::strong_ordering operator<=> ( const BigInt& rVal1, const BigInt& rVal2 );
};

inline BigInt::operator sal_Int16() const
{
    if ( nLen == 0 && nVal >= SAL_MIN_INT16 && nVal <= SAL_MAX_INT16 )
        return static_cast<sal_Int16>(nVal);
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_uInt16() const
{
    if ( nLen == 0 && nVal >= 0 && nVal <= SAL_MAX_UINT16 )
        return static_cast<sal_uInt16>(nVal);
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_Int32() const
{
    if (nLen == 0)
        return nVal;
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_uInt32() const
{
    if ( nLen == 0 && nVal >= 0 )
        return static_cast<sal_uInt32>(nVal);
    assert(false && "out of range");
    return 0;
}

inline BigInt::operator sal_Int64() const
{
    constexpr sal_uInt64 maxForPosInt64 = std::numeric_limits<sal_Int64>::max();
    constexpr sal_uInt64 maxForNegInt64 = std::numeric_limits<sal_Int64>::min();
    switch (nLen)
    {
        case 0:
            return nVal;
        case 1:
            return bIsNeg ? -sal_Int64(nNum[0]) : nNum[0];
        case 2:
            if (sal_uInt64 n = (sal_uInt64(nNum[1]) << 32) + nNum[0]; bIsNeg && n <= maxForNegInt64)
                return -sal_Int64(n); // maxForNegInt64 will convert correctly
            else if (!bIsNeg && n <= maxForPosInt64)
                return n;
    }
    assert(false && "out of range");
    return 0;
}

inline BigInt& BigInt::operator =( sal_Int32 nValue )
{
    nLen = 0;
    nVal = nValue;

    return *this;
}

inline void BigInt::Abs()
{
    if ( nLen != 0 )
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
