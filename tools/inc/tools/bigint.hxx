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
#ifndef _BIGINT_HXX
#define _BIGINT_HXX

#include <climits>
#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>

class SvStream;
#ifdef _TLBIGINT_INT64
struct SbxINT64;
struct SbxUINT64;
namespace binfilter { class SbxINT64Converter; }
#endif

// ----------
// - BigInt -
// ----------

#define MAX_DIGITS 8

class Fraction;

class TOOLS_DLLPUBLIC BigInt
{
#ifdef _TLBIGINT_INT64
    friend class ::binfilter::SbxINT64Converter;
#endif

private:
    long            nVal;
    unsigned short  nNum[MAX_DIGITS];
    sal_uInt8       nLen        : 5;    // Aktuelle Laenge
    sal_Bool        bIsNeg      : 1,    // Is Sign negative
                    bIsBig      : 1,    // sal_True == BigInt
                    bIsSet      : 1;    // Not "Null" (not not 0)

    TOOLS_DLLPRIVATE void MakeBigInt(BigInt const &);
    TOOLS_DLLPRIVATE void Normalize();
    TOOLS_DLLPRIVATE void Mult(BigInt const &, sal_uInt16);
    TOOLS_DLLPRIVATE void Div(sal_uInt16, sal_uInt16 &);
    TOOLS_DLLPRIVATE sal_Bool IsLess(BigInt const &) const;
    TOOLS_DLLPRIVATE void AddLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void SubLong(BigInt &, BigInt &);
    TOOLS_DLLPRIVATE void MultLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE void DivLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE void ModLong(BigInt const &, BigInt &) const;
    TOOLS_DLLPRIVATE sal_Bool ABS_IsLess(BigInt const &) const;

public:
                    BigInt();
                    BigInt( short nVal );
                    BigInt( long nVal );
                    BigInt( int nVal );
                    BigInt( double nVal );
                    BigInt( sal_uInt16 nVal );
                    BigInt( sal_uInt32 nVal );
                    BigInt( const BigInt& rBigInt );
                    BigInt( const ByteString& rString );
                    BigInt( const UniString& rString );
#ifdef _TLBIGINT_INT64
                    BigInt( const SbxINT64  &r );
                    BigInt( const SbxUINT64 &r );
#endif

    operator        short() const;
    operator        long()  const;
    operator        int()   const;
    operator        double() const;
    operator        sal_uInt16() const;
    operator        sal_uIntPtr() const;

    void            Set( sal_Bool bSet ) { bIsSet = bSet; }
    ByteString      GetByteString() const;
    UniString       GetString() const;

    sal_Bool            IsSet() const { return bIsSet; }
    sal_Bool            IsNeg() const;
    sal_Bool            IsZero() const;
    sal_Bool            IsOne() const;
    sal_Bool            IsLong() const { return !bIsBig; }
    void            Abs();
    void            DivMod( const BigInt &rDivisor, BigInt &rMod );
#ifdef _TLBIGINT_INT64
    sal_Bool            INT64 ( SbxINT64  *p ) const;
    sal_Bool            UINT64( SbxUINT64 *p ) const;
#endif

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

    TOOLS_DLLPUBLIC friend          sal_Bool operator==( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   sal_Bool operator!=( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend          sal_Bool operator< ( const BigInt& rVal1, const BigInt& rVal2 );
    TOOLS_DLLPUBLIC friend          sal_Bool operator> ( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   sal_Bool operator<=( const BigInt& rVal1, const BigInt& rVal2 );
    friend inline   sal_Bool operator>=( const BigInt& rVal1, const BigInt& rVal2 );

    friend class Fraction;
};

inline BigInt::BigInt()
{
    bIsSet = sal_False;
    bIsBig = sal_False;
    nVal   = 0;
}

inline BigInt::BigInt( short nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;
}

inline BigInt::BigInt( long nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;
}

inline BigInt::BigInt( int nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;
}

inline BigInt::BigInt( sal_uInt16 nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;
}

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
    if ( !bIsBig && nVal >= 0 && nVal <= USHRT_MAX )
        return (sal_uInt16)nVal;
    else
        return 0;
}

inline BigInt& BigInt::operator =( const short nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const long nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const int nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;

    return *this;
}

inline BigInt& BigInt::operator =( const sal_uInt16 nValue )
{
    bIsSet = sal_True;
    bIsBig = sal_False;
    nVal   = nValue;

    return *this;
}

inline sal_Bool BigInt::IsNeg() const
{
    if ( !bIsBig )
        return (nVal < 0);
    else
        return (sal_Bool)bIsNeg;
}

inline sal_Bool BigInt::IsZero() const
{
    if ( bIsBig )
        return sal_False;
    else
        return (nVal == 0);
}

inline sal_Bool BigInt::IsOne() const
{
    if ( bIsBig )
        return sal_False;
    else
        return (nVal == 1);
}

inline void BigInt::Abs()
{
    if ( bIsBig )
        bIsNeg = sal_False;
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

inline sal_Bool operator!=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !(rVal1 == rVal2);
}

inline sal_Bool operator<=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !( rVal1 > rVal2);
}

inline sal_Bool operator>=( const BigInt& rVal1, const BigInt& rVal2 )
{
    return !(rVal1 < rVal2);
}

#endif
