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

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

typedef sal_uInt8*        Scanline;
typedef const sal_uInt8*  ConstScanline;

enum class ScanlineFormat : sal_uInt8
{
    NONE,
    // 1 Bit - still needed?
    N1BitMsbPal,
    // 8 Bit Palette
    N8BitPal,
    // 24 Bit
    N24BitTcBgr,
    N24BitTcRgb,
    // 32 Bit
    N32BitTcAbgr,
    N32BitTcArgb,
    N32BitTcBgra,
    N32BitTcRgba,
    N32BitTcMask,
};

enum class ScanlineDirection : sal_uInt8
{
    BottomUp,
    TopDown
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
