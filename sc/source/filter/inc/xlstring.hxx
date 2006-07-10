/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlstring.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:04:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLSTRING_HXX
#define SC_XLSTRING_HXX

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// Constants and enumerations =================================================

/** Flags used to specify import/export mode of strings. */
typedef sal_uInt16 XclStrFlags;

const XclStrFlags EXC_STR_DEFAULT           = 0x0000;   /// Default string settings.
const XclStrFlags EXC_STR_FORCEUNICODE      = 0x0001;   /// Always use UCS-2 characters (default: try to compress). BIFF8 only.
const XclStrFlags EXC_STR_8BITLENGTH        = 0x0002;   /// 8-bit string length field (default: 16-bit).
const XclStrFlags EXC_STR_SMARTFLAGS        = 0x0004;   /// Omit flags on empty string (default: read/write always). BIFF8 only.
const XclStrFlags EXC_STR_KEEPFORMATS       = 0x0008;   /// Keep old formats when reading unformatted string (default: clear formats).

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

