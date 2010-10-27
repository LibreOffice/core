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

#ifndef SC_XLSTRING_HXX
#define SC_XLSTRING_HXX

#include "ftools.hxx"

// Constants and enumerations =================================================

/** Flags used to specify import/export mode of strings. */
typedef sal_uInt16 XclStrFlags;

const XclStrFlags EXC_STR_DEFAULT           = 0x0000;   /// Default string settings.
const XclStrFlags EXC_STR_FORCEUNICODE      = 0x0001;   /// Always use UCS-2 characters (default: try to compress). BIFF8 only.
const XclStrFlags EXC_STR_8BITLENGTH        = 0x0002;   /// 8-bit string length field (default: 16-bit).
const XclStrFlags EXC_STR_SMARTFLAGS        = 0x0004;   /// Omit flags on empty string (default: read/write always). BIFF8 only.
const XclStrFlags EXC_STR_SEPARATEFORMATS   = 0x0008;   /// Import: Keep old formats when reading unformatted string (default: clear formats); Export: Write unformatted string.
const XclStrFlags EXC_STR_NOHEADER          = 0x0010;   /// Export: Don't write the length and flag fields.

// ----------------------------------------------------------------------------

const sal_uInt16 EXC_STR_MAXLEN_8BIT        = 0x00FF;
const sal_uInt16 EXC_STR_MAXLEN             = 0xFFFF;

const sal_uInt8 EXC_STRF_16BIT              = 0x01;
const sal_uInt8 EXC_STRF_FAREAST            = 0x04;
const sal_uInt8 EXC_STRF_RICH               = 0x08;
const sal_uInt8 EXC_STRF_UNKNOWN            = 0xF2;

// Fixed-size characters
const sal_uInt8 EXC_LF_C                    = '\x0A';       /// LF character (used for line break).
const sal_uInt16 EXC_LF                     = EXC_LF_C;     /// LF character (unicode).
const sal_uInt8 EXC_NUL_C                   = '\x00';       /// NUL chararcter.
const sal_uInt16 EXC_NUL                    = EXC_NUL_C;    /// NUL chararcter (unicode).

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

    explicit inline     XclFormatRun() : mnChar( 0 ), mnFontIdx( 0 ) {}
    explicit inline     XclFormatRun( sal_uInt16 nChar, sal_uInt16 nFontIdx ) :
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

// ----------------------------------------------------------------------------

/** A vector with all formatting runs for a rich-string. */
typedef ::std::vector< XclFormatRun > XclFormatRunVec;

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
