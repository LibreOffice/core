/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_TOOLS_HELPERS_HXX
#define INCLUDED_TOOLS_HELPERS_HXX

#include <sal/config.h>
#include <sal/types.h>
#include <o3tl/safeint.hxx>
#include <cassert>
#include <type_traits>

template<typename T>
inline
typename std::enable_if<
             std::is_signed<T>::value || std::is_floating_point<T>::value, long >::type
MinMax(T nVal, long nMin, long nMax)
{
    assert(nMin <= nMax);
    return nVal >= nMin
        ? (nVal <= nMax ? static_cast<long>(nVal) : nMax) : nMin;
}

template<typename T>
inline
typename std::enable_if<
             std::is_unsigned<T>::value, long >::type
MinMax(T nVal, long nMin, long nMax)
{
    assert(nMin <= nMax);
    return nMax < 0
        ? nMax
        : ((nMin < 0 || nVal >= static_cast<unsigned long>(nMin))
           ? (nVal <= static_cast<unsigned long>(nMax)
              ? static_cast<long>(nVal) : nMax)
           : nMin);
}

inline sal_uInt32 AlignedWidth4Bytes(sal_uInt32 nWidthBits)
{
    if (nWidthBits > SAL_MAX_UINT32 - 31)
        nWidthBits = SAL_MAX_UINT32;
    else
        nWidthBits += 31;
    return (nWidthBits >> 5) << 2;
}

inline long FRound( double fVal )
{
    return fVal > 0.0 ? static_cast<long>( fVal + 0.5 ) : -static_cast<long>( -fVal + 0.5 );
}

// return (n >= 0)? (n*72+63)/127: (n*72-63)/127;
inline sal_Int64 sanitiseMm100ToTwip(sal_Int64 n)
{
    if (n >= 0)
    {
        if (o3tl::checked_multiply<sal_Int64>(n, 72, n) || o3tl::checked_add<sal_Int64>(n, 63, n))
            n = SAL_MAX_INT64;
    }
    else
    {
        if (o3tl::checked_multiply<sal_Int64>(n, 72, n) || o3tl::checked_sub<sal_Int64>(n, 63, n))
            n = SAL_MIN_INT64;
    }
    return n / 127;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
