/*************************************************************************
 *
 *  $RCSfile: xlconst.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:42:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XLCONST_HXX
#define SC_XLCONST_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


// Common =====================================================================

// Excel sheet dimensions -----------------------------------------------------

const sal_uInt16 EXC_MAXCOL_BIFF2           = 255;
const sal_uInt16 EXC_MAXROW_BIFF2           = 16383;
const sal_uInt16 EXC_MAXTAB_BIFF2           = 0;

const sal_uInt16 EXC_MAXCOL_BIFF4           = EXC_MAXCOL_BIFF2;
const sal_uInt16 EXC_MAXROW_BIFF4           = EXC_MAXROW_BIFF2;
const sal_uInt16 EXC_MAXTAB_BIFF4           = 32767;

const sal_uInt16 EXC_MAXCOL_BIFF8           = EXC_MAXCOL_BIFF4;
const sal_uInt16 EXC_MAXROW_BIFF8           = 65535;
const sal_uInt16 EXC_MAXTAB_BIFF8           = EXC_MAXTAB_BIFF4;

const sal_uInt16 EXC_NOTAB                  = 0xFFFF;   /// An invalid sheet index, for common use.


// In/out stream --------------------------------------------------------------

const sal_uInt32 RECORD_SEEK_TO_BEGIN       = 0UL;
const sal_uInt32 RECORD_SEEK_TO_END         = ~0UL;

const sal_uInt16 EXC_MAXRECSIZE_BIFF5       = 2080;
const sal_uInt16 EXC_MAXRECSIZE_BIFF8       = 8224;

const sal_uInt16 EXC_ID_UNKNOWN             = 0xFFFF;
const sal_uInt16 EXC_ID_CONT                = 0x003C;


// String import/export -------------------------------------------------------

/** Flags used to specify import/export mode of strings. */
typedef sal_uInt16                          XclStrFlags;
const XclStrFlags EXC_STR_DEFAULT           = 0x0000;   /// Default string settings.
const XclStrFlags EXC_STR_FORCEUNICODE      = 0x0001;   /// Always use UCS-2 characters (default: try to compress). BIFF8 only.
const XclStrFlags EXC_STR_8BITLENGTH        = 0x0002;   /// 8-bit string length field (default: 16-bit).
const XclStrFlags EXC_STR_SMARTFLAGS        = 0x0004;   /// Omit flags on empty string (default: read/write always). BIFF8 only.

const sal_uInt8 EXC_STRF_16BIT              = 0x01;
const sal_uInt8 EXC_STRF_FAREAST            = 0x04;
const sal_uInt8 EXC_STRF_RICH               = 0x08;
const sal_uInt8 EXC_STRF_UNKNOWN            = 0xF2;

// Fixed-size characters
const sal_uInt8 EXC_LF_C                    = '\x0A';       /// LF character (used for line break).
const sal_uInt16 EXC_LF                     = EXC_LF_C;     /// LF character (unicode).
const sal_uInt8 EXC_NUL_C                   = '\x00';       /// NUL chararcter.
const sal_uInt16 EXC_NUL                    = EXC_NUL_C;    /// NUL chararcter (unicode).


// Encoded URLs ---------------------------------------------------------------

const sal_Unicode EXC_URLSTART_ENCODED      = '\x01';   /// Encoded URL.
const sal_Unicode EXC_URLSTART_SELF         = '\x02';   /// Reference to own workbook.
const sal_Unicode EXC_URLSTART_SELFENCODED  = '\x03';   /// Encoded self reference.

const sal_Unicode EXC_URL_DOSDRIVE          = '\x01';   /// DOS drive letter or UNC server name.
const sal_Unicode EXC_URL_DRIVEROOT         = '\x02';   /// Root directory of current drive.
const sal_Unicode EXC_URL_SUBDIR            = '\x03';   /// Directory name delimiter.
const sal_Unicode EXC_URL_PARENTDIR         = '\x04';   /// Parent directory.
const sal_Unicode EXC_URL_MACVOLUME         = '\x05';   /// MAC volume name.
const sal_Unicode EXC_URL_SHEETNAME         = '\x09';   /// Sheet name starts here (BIFF4).

const sal_Unicode EXC_DDE_DELIM             = '\x03';   /// DDE application-topic delimiter


// Cached values list (EXTERNNAME, ptgArray, ...) -----------------------------

const sal_uInt8 EXC_CACHEDVAL_EMPTY         = 0x00;
const sal_uInt8 EXC_CACHEDVAL_DOUBLE        = 0x01;
const sal_uInt8 EXC_CACHEDVAL_STRING        = 0x02;
const sal_uInt8 EXC_CACHEDVAL_BOOL          = 0x04;
const sal_uInt8 EXC_CACHEDVAL_ERROR         = 0x10;


// Measures -------------------------------------------------------------------

const sal_Int32 EXC_POINTS_PER_INCH         = 72;
const sal_Int32 EXC_TWIPS_PER_INCH          = EXC_POINTS_PER_INCH * 20;


// Line styles ----------------------------------------------------------------

const sal_uInt8 EXC_LINE_NONE               = 0x00;
const sal_uInt8 EXC_LINE_THIN               = 0x01;
const sal_uInt8 EXC_LINE_MEDIUM             = 0x02;
const sal_uInt8 EXC_LINE_THICK              = 0x05;
const sal_uInt8 EXC_LINE_DOUBLE             = 0x06;
const sal_uInt8 EXC_LINE_HAIR               = 0x07;


// Background patterns --------------------------------------------------------

const sal_uInt8 EXC_PATT_NONE               = 0x00;
const sal_uInt8 EXC_PATT_SOLID              = 0x01;
const sal_uInt8 EXC_PATT_50_PERC            = 0x02;
const sal_uInt8 EXC_PATT_75_PERC            = 0x03;
const sal_uInt8 EXC_PATT_25_PERC            = 0x04;
const sal_uInt8 EXC_PATT_12_5_PERC          = 0x11;
const sal_uInt8 EXC_PATT_6_25_PERC          = 0x12;


// Records (ordered by lowest record ID) ======================================

// (0x0007, 0x0207) STRING ----------------------------------------------------

const sal_uInt16 EXC_ID_STRING              = 0x0207;


// (0x001C) NOTE --------------------------------------------------------------

const sal_uInt16 EXC_ID_NOTE                = 0x001C;
const sal_uInt16 EXC_NOTE_VISIBLE           = 0x0002;


// (0x0012, 0x0019) PROTECT and WINDOWPROTECT --------------------

const sal_uInt16 EXC_ID_PROTECT             = 0x0012;
const sal_uInt16 EXC_ID_WINDOWPROTECT       = 0x0019;


// (0x0055) DEFCOLWIDTH -------------------------------------------------------

const sal_uInt16 EXC_ID_DEFCOLWIDTH         = 0x0055;


// (0x007D) COLINFO -----------------------------------------------------------

const sal_uInt16 EXC_ID_COLINFO             = 0x007D;

const sal_uInt16 EXC_COLINFO_HIDDEN         = 0x0001;
const sal_uInt16 EXC_COLINFO_COLLAPSED      = 0x1000;


// (0x007E) RK ----------------------------------------------------------------

const sal_Int32 EXC_RK_100FLAG              = 0x00000001;
const sal_Int32 EXC_RK_INTFLAG              = 0x00000002;
const sal_Int32 EXC_RK_VALUEMASK            = 0xFFFFFFFC;

const sal_Int32 EXC_RK_DBL                  = 0x00000000;
const sal_Int32 EXC_RK_DBL100               = EXC_RK_100FLAG;
const sal_Int32 EXC_RK_INT                  = EXC_RK_INTFLAG;
const sal_Int32 EXC_RK_INT100               = EXC_RK_100FLAG | EXC_RK_INTFLAG;


// (0x0081) WSBOOL ------------------------------------------------------------

const sal_uInt16 EXC_ID_WSBOOL              = 0x0081;

const sal_uInt16 EXC_WSBOOL_ROWBELOW        = 0x0040;
const sal_uInt16 EXC_WSBOOL_COLBELOW        = 0x0080;
const sal_uInt16 EXC_WSBOOL_FITTOPAGE       = 0x0100;

const sal_uInt16 EXC_WSBOOL_DEFAULTFLAGS    = 0x04C1;


// (0x008C) COUNTRY -----------------------------------------------------------

const sal_uInt16 EXC_ID_COUNTRY             = 0x008C;


// ============================================================================

#endif

