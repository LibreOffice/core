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

#include <sal/types.h>

/* Definition of Scanline formats */

namespace basebmp { namespace Format
{
    static const sal_Int32 NONE                         = 0;
    static const sal_Int32 ONE_BIT_MSB_GREY             = (sal_Int32)0x01;
    static const sal_Int32 ONE_BIT_LSB_GREY             = (sal_Int32)0x02;
    static const sal_Int32 ONE_BIT_MSB_PAL              = (sal_Int32)0x03;
    static const sal_Int32 ONE_BIT_LSB_PAL              = (sal_Int32)0x04;
    static const sal_Int32 FOUR_BIT_MSB_GREY            = (sal_Int32)0x05;
    static const sal_Int32 FOUR_BIT_LSB_GREY            = (sal_Int32)0x06;
    static const sal_Int32 FOUR_BIT_MSB_PAL             = (sal_Int32)0x07;
    static const sal_Int32 FOUR_BIT_LSB_PAL             = (sal_Int32)0x08;
    static const sal_Int32 EIGHT_BIT_PAL                = (sal_Int32)0x09;
    static const sal_Int32 EIGHT_BIT_GREY               = (sal_Int32)0x0A;
    static const sal_Int32 SIXTEEN_BIT_LSB_TC_MASK      = (sal_Int32)0x0B;
    static const sal_Int32 SIXTEEN_BIT_MSB_TC_MASK      = (sal_Int32)0x0C;
    static const sal_Int32 TWENTYFOUR_BIT_TC_MASK       = (sal_Int32)0x0D;
    // The order of the channels code letters indicates the order of the
    // channel bytes in memory, I think
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_BGRA   = (sal_Int32)0x0E;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_ARGB   = (sal_Int32)0x0F;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_ABGR   = (sal_Int32)0x10;
    static const sal_Int32 THIRTYTWO_BIT_TC_MASK_RGBA   = (sal_Int32)0x11;
    static const sal_Int32 MAX                          = (sal_Int32)0x11;

    const char *formatName(sal_Int32 nScanlineFormat);
} }

#endif /* INCLUDED_BASEBMP_SCANLINEFORMATS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
