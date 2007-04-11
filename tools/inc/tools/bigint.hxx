/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bigint.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:08:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _BIGINT_HXX
#define _BIGINT_HXX

#include <climits>

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif

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
    operator        USHORT() const;
    operator        ULONG() const;

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

inline BigInt::operator USHORT() const
{
    if ( !bIsBig && nVal >= 0 && nVal <= USHRT_MAX )
        return (USHORT)nVal;
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
