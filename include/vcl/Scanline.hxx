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

#ifndef INCLUDED_VCL_SCANLINE_HXX
#define INCLUDED_VCL_SCANLINE_HXX

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

typedef sal_uInt8*        Scanline;
typedef const sal_uInt8*  ConstScanline;

enum class ScanlineFormat {
    NONE              = 0x00000000,

    N1BitMsbPal       = 0x00000001,
    N1BitLsbPal       = 0x00000002,

    N8BitPal          = 0x00000010,

    N24BitTcBgr       = 0x00000100,
    N24BitTcRgb       = 0x00000200,

    N32BitTcAbgr      = 0x00000800,
    N32BitTcArgb      = 0x00001000,
    N32BitTcBgra      = 0x00002000,
    N32BitTcRgba      = 0x00004000,
    N32BitTcMask      = 0x00008000,

    TopDown           = 0x00010000 // scanline adjustment
};

namespace o3tl
{
    template<> struct typed_flags<ScanlineFormat> : is_typed_flags<ScanlineFormat, 0x0001fb13> {};
}

inline ScanlineFormat RemoveScanline(ScanlineFormat nFormat)
{
    return nFormat & ~ScanlineFormat::TopDown;
}

#endif // INCLUDED_VCL_SCANLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
