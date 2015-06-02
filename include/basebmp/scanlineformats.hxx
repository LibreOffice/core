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

#ifndef INCLUDED_BASEBMP_SCANLINEFORMATS_HXX
#define INCLUDED_BASEBMP_SCANLINEFORMATS_HXX

#include <sal/config.h>

/* Definition of Scanline formats */

namespace basebmp {

enum class Format
{
    NONE,
    OneBitMsbGrey,
    OneBitLsbGrey,
    OneBitMsbPal,
    OneBitLsbPal,
    FourBitMsbGrey,
    FourBitLsbGrey,
    FourBitMsbPal,
    FourBitLsbPal,
    EightBitPal,
    EightBitGrey,
    SixteenBitLsbTcMask,
    SixteenBitMsbTcMask,
    TwentyFourBitTcMask,
    // CAIRO_FORMAT_RGB24, each pixel is a 32-bit quantity, with the upper 8
    // bits unused. Red, Green, and Blue are stored in the remaining 24 bits in
    // that order (below U is for unused)
    ThirtyTwoBitTcMaskBGRX,
    // The order of the channels code letters indicates the order of the
    // channel bytes in memory
    ThirtyTwoBitTcMaskBGRA,
    ThirtyTwoBitTcMaskARGB,
    ThirtyTwoBitTcMaskABGR,
    ThirtyTwoBitTcMaskRGBA,
    LAST = ThirtyTwoBitTcMaskRGBA
};

const char *formatName(Format nScanlineFormat);

}

#endif /* INCLUDED_BASEBMP_SCANLINEFORMATS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
