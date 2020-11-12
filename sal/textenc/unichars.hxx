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

#ifndef INCLUDED_SAL_TEXTENC_UNICHARS_HXX
#define INCLUDED_SAL_TEXTENC_UNICHARS_HXX

#include <sal/config.h>

#include <cassert>

#include <rtl/character.hxx>
#include <sal/types.h>

#define RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER 0xFFFD

bool ImplIsControlOrFormat(sal_uInt32 nUtf32);

// All code points that are high-surrogates, as of Unicode 3.1.1.
inline bool ImplIsHighSurrogate(sal_uInt32 nUtf32) { return nUtf32 >= 0xD800 && nUtf32 <= 0xDBFF; }

// All code points that are low-surrogates, as of Unicode 3.1.1.
inline bool ImplIsLowSurrogate(sal_uInt32 nUtf32) { return nUtf32 >= 0xDC00 && nUtf32 <= 0xDFFF; }

bool ImplIsPrivateUse(sal_uInt32 nUtf32);

bool ImplIsZeroWidth(sal_uInt32 nUtf32);

inline sal_uInt32 ImplGetHighSurrogate(sal_uInt32 nUtf32)
{
    assert(nUtf32 >= 0x10000);
    return ((nUtf32 - 0x10000) >> 10) | 0xD800;
}

inline sal_uInt32 ImplGetLowSurrogate(sal_uInt32 nUtf32)
{
    assert(nUtf32 >= 0x10000);
    return ((nUtf32 - 0x10000) & 0x3FF) | 0xDC00;
}

inline sal_uInt32 ImplCombineSurrogates(sal_uInt32 nHigh, sal_uInt32 nLow)
{
    assert(ImplIsHighSurrogate(nHigh) && ImplIsLowSurrogate(nLow));
    return (((nHigh & 0x3FF) << 10) | (nLow & 0x3FF)) + 0x10000;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
