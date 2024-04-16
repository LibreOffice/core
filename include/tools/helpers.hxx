/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include <sal/types.h>
#include <tools/long.hxx>
#include <cassert>
#include <limits>
#include <type_traits>

template<typename T>
inline
typename std::enable_if<
             std::is_signed<T>::value || std::is_floating_point<T>::value, long >::type
MinMax(T nVal, tools::Long nMin, tools::Long nMax)
{
    assert(nMin <= nMax);
    if (nVal >= nMin)
    {
        if (nVal <= nMax)
            return static_cast<tools::Long>(nVal);
        else
           return nMax;
    }
    else
    {
        return nMin;
    }
}

template<typename T>
inline
typename std::enable_if<
             std::is_unsigned<T>::value, long >::type
MinMax(T nVal, tools::Long nMin, tools::Long nMax)
{
    assert(nMin <= nMax);
    if (nMax < 0)
    {
        return nMax;
    }
    else
    {
        if (nMin < 0 || nVal >= static_cast<unsigned long>(nMin))
        {
            if (nVal <= static_cast<unsigned long>(nMax))
                return static_cast<tools::Long>(nVal);
            else
                return nMax;
        }
        else
        {
            return nMin;
        }
    }
}

inline sal_uInt32 AlignedWidth4Bytes(sal_uInt32 nWidthBits)
{
    if (nWidthBits > SAL_MAX_UINT32 - 31)
        nWidthBits = SAL_MAX_UINT32;
    else
        nWidthBits += 31;
    return (nWidthBits >> 5) << 2;
}

//valid range:  (-180,180]
template <typename T>
[[nodiscard]] inline typename std::enable_if<std::is_signed<T>::value, T>::type
NormAngle180(T angle)
{
    while (angle <= -180)
        angle += 360;
    while (angle > 180)
        angle -= 360;
    return angle;
}

//valid range:  [0,360)
template <typename T> [[nodiscard]] inline T NormAngle360(T angle)
{
    while (angle < 0)
        angle += 360;
    while (angle >= 360)
        angle -= 360;
    return angle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
