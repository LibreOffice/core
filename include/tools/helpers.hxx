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

inline long AlignedWidth4Bytes( long nWidthBits )
{
    return ( ( nWidthBits + 31 ) >> 5 ) << 2;
}

inline long FRound( double fVal )
{
    return fVal > 0.0 ? static_cast<long>( fVal + 0.5 ) : -static_cast<long>( -fVal + 0.5 );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
