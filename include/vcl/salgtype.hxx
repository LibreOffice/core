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

#ifndef INCLUDED_VCL_SALGTYPE_HXX
#define INCLUDED_VCL_SALGTYPE_HXX

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

enum class DeviceFormat {
                            NONE = -1,
                            DEFAULT = 0,
                            BITMASK = 1,
#ifdef IOS
                            GRAYSCALE = 8
#endif
                        };

typedef sal_uInt32 SalColor;

constexpr SalColor MAKE_SALCOLOR(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b) {
    return sal_uInt32(b) | (sal_uInt32(g) << 8) | (sal_uInt32(r) << 16);
}

#define SALCOLOR_RED( n )           (static_cast<sal_uInt8>((n)>>16))
#define SALCOLOR_GREEN( n )         (static_cast<sal_uInt8>((static_cast<sal_uInt16>(n)) >> 8))
#define SALCOLOR_BLUE( n )          (static_cast<sal_uInt8>(n))
#define SALCOLOR_NONE           (~SalColor(0))

// must equal to class Point
struct SalPoint
{
    long        mnX;
    long        mnY;
};

typedef const SalPoint*   PCONSTSALPOINT;

struct SalTwoRect
{
    long        mnSrcX;
    long        mnSrcY;
    long        mnSrcWidth;
    long        mnSrcHeight;
    long        mnDestX;
    long        mnDestY;
    long        mnDestWidth;
    long        mnDestHeight;

    SalTwoRect(long nSrcX, long nSrcY, long nSrcWidth, long nSrcHeight,
               long nDestX, long nDestY, long nDestWidth, long nDestHeight)
        : mnSrcX(nSrcX), mnSrcY(nSrcY), mnSrcWidth(nSrcWidth), mnSrcHeight(nSrcHeight)
        , mnDestX(nDestX), mnDestY(nDestY), mnDestWidth(nDestWidth), mnDestHeight(nDestHeight)
    {
    }
};

enum class SalROPColor {
    N0, N1, Invert
};

enum class SalInvert {
    NONE       = 0x00,
    Highlight  = 0x01,
    N50        = 0x02,
    TrackFrame = 0x04
};
namespace o3tl
{
    template<> struct typed_flags<SalInvert> : is_typed_flags<SalInvert, 0x07> {};
}

#endif // INCLUDED_VCL_SALGTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
