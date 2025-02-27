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
#include <osl/endian.h>

namespace o3tl
{
/** 16-bit unsigned integer type that can be used in a struct to read from data that is big endian.
 *
 * Type can't be instantiated but only used in a struct which is reinterpret_cast from bytes.
 */
class sal_uInt16_BE
{
private:
    sal_uInt16 mnValue;
    sal_uInt16_BE() = delete;

public:
    constexpr operator sal_uInt16() const
    {
#ifdef OSL_LITENDIAN
        return OSL_SWAPWORD(mnValue);
#else
        return mnValue;
#endif
    }
};

/** 32-bit unsigned integer type that can be used in a struct to read from data that is big endian.
 *
 * Type can't be instantiated but only used in a struct which is reinterpret_cast from bytes.
 */
class sal_uInt32_BE
{
private:
    sal_uInt32 mnValue;
    sal_uInt32_BE() = delete;

public:
    constexpr operator sal_uInt32() const
    {
#ifdef OSL_LITENDIAN
        return OSL_SWAPDWORD(mnValue);
#else
        return mnValue;
#endif
    }
};

} // namespace o3tl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
