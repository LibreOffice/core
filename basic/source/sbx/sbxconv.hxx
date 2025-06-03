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

#pragma once

#include "sbxdec.hxx"
#include <basic/sberrors.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxcore.hxx>
#include <basic/sbxdef.hxx>

#include <o3tl/float_int_conversion.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/math.hxx>
#include <sal/types.h>

class SbxArray;

template <std::integral I> I ConvertWithOverflowTo(double f)
{
    f = rtl::math::round(f);
    if (!o3tl::convertsToAtMost(f, std::numeric_limits<I>::max()))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return std::numeric_limits<I>::max();
    }
    if (!o3tl::convertsToAtLeast(f, std::numeric_limits<I>::min()))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return std::numeric_limits<I>::min();
    }
    return f;
}

template <std::integral To, std::integral From> To ConvertWithOverflowTo(From n)
{
    using ValidRange = o3tl::ValidRange<To>;
    if (ValidRange::isAbove(n))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return std::numeric_limits<To>::max();
    }
    if (ValidRange::isBelow(n))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return std::numeric_limits<To>::min();
    }
    return n;
}

template <std::floating_point To, std::floating_point From> inline To ConvertWithOverflowTo(From n)
{
    if constexpr (std::numeric_limits<From>::max() > std::numeric_limits<To>::max())
    {
        if (n > std::numeric_limits<To>::max())
        {
            SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
            return std::numeric_limits<To>::max();
        }
    }
    if constexpr (std::numeric_limits<From>::lowest() < std::numeric_limits<To>::lowest())
    {
        if (n < std::numeric_limits<To>::lowest())
        {
            SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
            return std::numeric_limits<To>::lowest();
        }
    }
    // tests for underflow - storing value too small for precision of single
    if constexpr (std::numeric_limits<From>::min() < std::numeric_limits<To>::min())
    {
        if (n > 0 && n < std::numeric_limits<To>::min())
        {
            SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
            return std::numeric_limits<To>::min();
        }
        if (n < 0 && n > -std::numeric_limits<To>::min())
        {
            SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
            return -std::numeric_limits<To>::min();
        }
    }
    return n;
}

template <std::floating_point To, std::integral From> inline To ConvertWithOverflowTo(From n)
{
    // No integral types overflow floats
    return static_cast<To>(n);
}

template <typename T1, typename T2> void assignWithOverflowTo(T1& dest, T2 src)
{
    dest = ConvertWithOverflowTo<T1>(src);
}

template <typename T> void assignWithOverflowTo(T& dest, SbxDecimal& dec)
{
    dest = dec.getWithOverflow<T>();
}

// SBXSCAN.CXX
extern void ImpCvtNum( double nNum, short nPrec, OUString& rRes, bool bCoreString=false );
extern ErrCode ImpScan
    ( std::u16string_view rSrc, double& nVal, SbxDataType& rType, sal_Int32* pLen,
      bool* pHasNumber, bool bOnlyIntntl );
// A version that uses defaults / compatibility settings for bOnlyIntntl
extern ErrCode ImpScan
    ( std::u16string_view rSrc, double& nVal, SbxDataType& rType, sal_Int32* pLen );

void ImpGetIntntlSep( sal_Unicode& rcDecimalSep, sal_Unicode& rcThousandSep, sal_Unicode& rcDecimalSepAlt );

// SBXINT.CXX

sal_Unicode ImpGetChar( const SbxValues* );
void        ImpPutChar( SbxValues*, sal_Unicode );
sal_uInt8   ImpGetByte( const SbxValues* );
void        ImpPutByte( SbxValues*, sal_uInt8 );
sal_Int16   ImpGetInteger( const SbxValues* );
void        ImpPutInteger( SbxValues*, sal_Int16 );
sal_uInt16  ImpGetUShort( const SbxValues* );
void        ImpPutUShort( SbxValues*, sal_uInt16 );
sal_Int32   ImpGetLong( const SbxValues* );
void        ImpPutLong( SbxValues*, sal_Int32 );
sal_uInt32  ImpGetULong( const SbxValues* );
void        ImpPutULong( SbxValues*, sal_uInt32 );
sal_Int64   ImpGetInt64( const SbxValues* );
void        ImpPutInt64( SbxValues*, sal_Int64 );
sal_uInt64  ImpGetUInt64( const SbxValues* );
void        ImpPutUInt64( SbxValues*, sal_uInt64 );
double      ImpGetDouble( const SbxValues* );
void        ImpPutDouble( SbxValues*, double, bool bCoreString=false );
float       ImpGetSingle( const SbxValues* );
void        ImpPutSingle( SbxValues*, float );

// SBXCURR.CXX

sal_Int64   ImpGetCurrency( const SbxValues* );
void        ImpPutCurrency( SbxValues*, const sal_Int64 );

inline sal_Int64 CurFrom(double d)
{
    double result = d > 0 ? (d * CURRENCY_FACTOR + 0.5) : (d * CURRENCY_FACTOR - 0.5);
    if (result >= double(SAL_MAX_INT64)) // double(SAL_MAX_INT64) is greater than SAL_MAX_INT64
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return SAL_MAX_INT64;
    }
    if (result < double(SAL_MIN_INT64))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return SAL_MIN_INT64;
    }
    return result;
}

template <std::integral I> sal_Int64 CurFrom(I n)
{
    using ValidRange = o3tl::ValidRange<sal_Int64, SAL_MIN_INT64 / CURRENCY_FACTOR, SAL_MAX_INT64 / CURRENCY_FACTOR>;
    if (ValidRange::isAbove(n))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return SAL_MAX_INT64;
    }
    if (ValidRange::isBelow(n))
    {
        SbxBase::SetError(ERRCODE_BASIC_MATH_OVERFLOW);
        return SAL_MIN_INT64;
    }
    return n * CURRENCY_FACTOR;
}

template <std::integral I> I CurTo(sal_Int64 cur_val)
{
    return ConvertWithOverflowTo<I>(CurTo<sal_Int64>(cur_val));
}

template <> inline sal_Int64 CurTo<sal_Int64>(sal_Int64 cur_val)
{
    sal_Int64 i = cur_val / CURRENCY_FACTOR;
    // Rounding (half-to-even)
    int f = cur_val % CURRENCY_FACTOR;
    if (i % 2 == 1)
    {
        if (f < 0)
            --f;
        else
            ++f;
    }
    if (f > CURRENCY_FACTOR / 2)
        ++i;
    else if (f < -CURRENCY_FACTOR / 2)
        --i;
    return i;
}

template <std::floating_point F> F CurTo(sal_Int64 r)
{
    return ConvertWithOverflowTo<F>(static_cast<double>(r) / CURRENCY_FACTOR);
}

// SBXDEC.CXX

SbxDecimal* ImpCreateDecimal( SbxValues* p );
SbxDecimal* ImpGetDecimal( const SbxValues* p );
void ImpPutDecimal( SbxValues* p, SbxDecimal* pDec );

// SBXDATE.CXX

double  ImpGetDate( const SbxValues* );
void    ImpPutDate( SbxValues*, double );

// SBXSTR.CXX

OUString     ImpGetString( const SbxValues* );
OUString     ImpGetCoreString( const SbxValues* );
void    ImpPutString( SbxValues*, const OUString* );

// SBXBOOL.CXX

enum SbxBOOL ImpGetBool( const SbxValues* );
void    ImpPutBool( SbxValues*, sal_Int16 );

// ByteArray <--> String
SbxArray* StringToByteArray(const OUString& rStr);
OUString ByteArrayToString(SbxArray* pArr);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
