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
#ifndef INCLUDED_TOOLS_COLORDATA_HXX
#define INCLUDED_TOOLS_COLORDATA_HXX

#include <sal/types.h>

// Color types
typedef sal_uInt32 ColorData;

constexpr ColorData TRGB_COLORDATA(
    sal_uInt8 TRANSPARENCE, sal_uInt8 RED, sal_uInt8 GREEN, sal_uInt8 BLUE)
{
    return sal_uInt32(BLUE) | (sal_uInt32(GREEN) << 8) | (sal_uInt32(RED) << 16)
        | (sal_uInt32(TRANSPARENCE) << 24);
}

constexpr ColorData RGB_COLORDATA(sal_uInt8 r, sal_uInt8 g, sal_uInt8 b) {
    return sal_uInt32(b) | (sal_uInt32(g) << 8) | (sal_uInt32(r) << 16);
}

#define COLORDATA_RED( n )          (static_cast<sal_uInt8>((n)>>16))
#define COLORDATA_GREEN( n )        (static_cast<sal_uInt8>((static_cast<sal_uInt16>(n)) >> 8))
#define COLORDATA_BLUE( n )         (static_cast<sal_uInt8>(n))
#define COLORDATA_TRANSPARENCY( n ) (static_cast<sal_uInt8>((n)>>24))
#define COLORDATA_RGB( n )          (static_cast<ColorData>((n) & 0x00FFFFFF))

constexpr sal_uInt8 ColorChannelMerge(sal_uInt8 nDst, sal_uInt8 nSrc, sal_uInt8 nSrcTrans)
{
    return static_cast<sal_uInt8>(((static_cast<sal_Int32>(nDst)-nSrc)*nSrcTrans+((nSrc<<8)|nDst))>>8);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
