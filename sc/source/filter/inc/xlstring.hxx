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

#include <vector>
#include <o3tl/typed_flags_set.hxx>

// Constants and enumerations =================================================

/** Flags used to specify import/export mode of strings. */
enum class XclStrFlags : sal_uInt16 {
    NONE              = 0x0000,   /// Default string settings.
    ForceUnicode      = 0x0001,   /// Always use UCS-2 characters (default: try to compress). BIFF8 only.
    EightBitLength    = 0x0002,   /// 8-bit string length field (default: 16-bit).
    SmartFlags        = 0x0004,   /// Omit flags on empty string (default: read/write always). BIFF8 only.
    SeparateFormats   = 0x0008,   /// Import: Keep old formats when reading unformatted string (default: clear formats); Export: Write unformatted string.
    NoHeader          = 0x0010,   /// Export: Don't write the length and flag fields.
};
namespace o3tl {
    template<> struct typed_flags<XclStrFlags> : is_typed_flags<XclStrFlags, 0x001f> {};
}


const sal_uInt16 EXC_STR_MAXLEN_8BIT        = 0x00FF;
const sal_uInt16 EXC_STR_MAXLEN             = 0x7FFF;

const sal_uInt8 EXC_STRF_16BIT              = 0x01;
const sal_uInt8 EXC_STRF_FAREAST            = 0x04;
const sal_uInt8 EXC_STRF_RICH               = 0x08;
const sal_uInt8 EXC_STRF_UNKNOWN            = 0xF2;

// Fixed-size characters
const sal_uInt8 EXC_LF_C                    = '\x0A';       /// LF character (used for line break).
const sal_uInt16 EXC_LF                     = EXC_LF_C;     /// LF character (unicode).
const sal_uInt8 EXC_NUL_C                   = '\x00';       /// NUL character.
const sal_uInt16 EXC_NUL                    = EXC_NUL_C;    /// NUL character (unicode).

// Rich-string formatting runs ================================================

/** Represents a formatting run for rich-strings.

    An Excel formatting run stores the first formatted character in a
    rich-string and the index of a font used to format this and the following
    characters.
 */
struct XclFormatRun
{
    sal_uInt16          mnChar;         /// First character this format applies to.
    sal_uInt16          mnFontIdx;      /// Excel font index for the next characters.

    explicit XclFormatRun() : mnChar( 0 ), mnFontIdx( 0 ) {}
    explicit XclFormatRun( sal_uInt16 nChar, sal_uInt16 nFontIdx ) :
                            mnChar( nChar ), mnFontIdx( nFontIdx ) {}
};

inline bool operator==( const XclFormatRun& rLeft, const XclFormatRun& rRight )
{
    return (rLeft.mnChar == rRight.mnChar) && (rLeft.mnFontIdx == rRight.mnFontIdx);
}

inline bool operator<( const XclFormatRun& rLeft, const XclFormatRun& rRight )
{
    return (rLeft.mnChar < rRight.mnChar) || ((rLeft.mnChar == rRight.mnChar) && (rLeft.mnFontIdx < rRight.mnFontIdx));
}

/** A vector with all formatting runs for a rich-string. */
typedef ::std::vector< XclFormatRun > XclFormatRunVec;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
