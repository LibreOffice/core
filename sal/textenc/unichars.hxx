/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SAL_TEXTENC_UNICHARS_HXX
#define INCLUDED_SAL_TEXTENC_UNICHARS_HXX

#include "sal/config.h"

#include <cassert>

#include "sal/types.h"

#define RTL_TEXTENC_UNICODE_REPLACEMENT_CHARACTER 0xFFFD

inline bool ImplIsNoncharacter(sal_uInt32 nUtf32)
{
    return (nUtf32 >= 0xFDD0 && nUtf32 <= 0xFDEF)
           || (nUtf32 & 0xFFFF) >= 0xFFFE
           || nUtf32 > 0x10FFFF;
}
    // All code points that are noncharacters, as of Unicode 3.1.1.

bool ImplIsControlOrFormat(sal_uInt32 nUtf32);

inline bool ImplIsHighSurrogate(sal_uInt32 nUtf32)
{ return nUtf32 >= 0xD800 && nUtf32 <= 0xDBFF; }
    // All code points that are high-surrogates, as of Unicode 3.1.1.

inline bool ImplIsLowSurrogate(sal_uInt32 nUtf32)
{ return nUtf32 >= 0xDC00 && nUtf32 <= 0xDFFF; }
    // All code points that are low-surrogates, as of Unicode 3.1.1.

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
