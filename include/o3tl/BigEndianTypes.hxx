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

namespace o3tl
{
/** 16-bit unsigned integer type that can be used in a struct to read from data that is big endian
    and potentially misaligned.
 *
 * Type can't be instantiated but only used in a struct which is reinterpret_cast from bytes.
 */
class sal_uInt16_BE
{
private:
    sal_uInt8 mnValue[2];
    sal_uInt16_BE() = delete;

public:
    constexpr operator sal_uInt16() const
    {
        return sal_uInt32(mnValue[1]) | (sal_uInt32(mnValue[0]) << 8);
    }
};

/** 32-bit unsigned integer type that can be used in a struct to read from data that is big endian
    and potentially misaligned.
 *
 * Type can't be instantiated but only used in a struct which is reinterpret_cast from bytes.
 */
class sal_uInt32_BE
{
private:
    sal_uInt8 mnValue[4];
    sal_uInt32_BE() = delete;

public:
    constexpr operator sal_uInt32() const
    {
        return sal_uInt32(mnValue[3]) | (sal_uInt32(mnValue[2]) << 8)
               | (sal_uInt32(mnValue[1]) << 16) | (sal_uInt32(mnValue[0]) << 24);
    }
};

} // namespace o3tl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
