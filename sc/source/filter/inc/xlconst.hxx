/*************************************************************************
 *
 *  $RCSfile: xlconst.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:38:11 $
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


// (0x0014, 0x0015) HEADER, FOOTER --------------------------------------------

const sal_uInt16 EXC_ID_HEADER              = 0x0014;
const sal_uInt16 EXC_ID_FOOTER              = 0x0015;


// (0x0017) EXC_ID_EXTERNSHEET ------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNSHEET         = 0x0017;


// (0x0018, 0x0218) NAME ------------------------------------------------------

// flags
const sal_uInt16 EXC_NAME_HIDDEN            = 0x0001;
const sal_uInt16 EXC_NAME_FUNC              = 0x0002;
const sal_uInt16 EXC_NAME_VB                = 0x0004;
const sal_uInt16 EXC_NAME_PROC              = 0x0008;
const sal_uInt16 EXC_NAME_CALCEXP           = 0x0010;
const sal_uInt16 EXC_NAME_BUILTIN           = 0x0020;
const sal_uInt16 EXC_NAME_FGROUPMASK        = 0x0FC0;
const sal_uInt16 EXC_NAME_BIG               = 0x1000;

// codes for BuiltIn names
const sal_Unicode EXC_BUILTIN_CONSOLIDATEAREA   = 0x0000;
const sal_Unicode EXC_BUILTIN_AUTOOPEN          = 0x0001;
const sal_Unicode EXC_BUILTIN_AUTOCLOSE         = 0x0002;
const sal_Unicode EXC_BUILTIN_EXTRACT           = 0x0003;
const sal_Unicode EXC_BUILTIN_DATABASE          = 0x0004;
const sal_Unicode EXC_BUILTIN_CRITERIA          = 0x0005;
const sal_Unicode EXC_BUILTIN_PRINTAREA         = 0x0006;
const sal_Unicode EXC_BUILTIN_PRINTTITLES       = 0x0007;
const sal_Unicode EXC_BUILTIN_RECORDER          = 0x0008;
const sal_Unicode EXC_BUILTIN_DATAFORM          = 0x0009;
const sal_Unicode EXC_BUILTIN_AUTOACTIVATE      = 0x000A;
const sal_Unicode EXC_BUILTIN_AUTODEACTIVATE    = 0x000B;
const sal_Unicode EXC_BUILTIN_SHEETTITLE        = 0x000C;
const sal_Unicode EXC_BUILTIN_AUTOFILTER        = 0x000D;
const sal_Unicode EXC_BUILTIN_UNKNOWN           = 0x000E;


// (0x001A, 0x001B) VERTICAL-, HORIZONTALPAGEBREAKS ---------------------------

const sal_uInt16 EXC_ID_VERTPAGEBREAKS      = 0x001A;
const sal_uInt16 EXC_ID_HORPAGEBREAKS       = 0x001B;


// (0x001C) NOTE --------------------------------------------------------------

const sal_uInt16 EXC_ID_NOTE                = 0x001C;
const sal_uInt16 EXC_NOTE_VISIBLE           = 0x0002;


// (0x001E, 0x041E) FORMAT ----------------------------------------------------

const sal_uInt16 EXC_ID_FORMAT              = 0x041E;

const sal_uInt16 EXC_FORMAT_OFFSET5         = 164;
const sal_uInt16 EXC_FORMAT_OFFSET8         = 164;


// (0x0023) EXTERNNAME --------------------------------------------------------

const sal_uInt16 EXC_ID_EXTERNNAME          = 0x0023;

const sal_uInt16 EXC_EXTN_BUILTIN           = 0x0001;
const sal_uInt16 EXC_EXTN_OLE               = 0x0010;
const sal_uInt16 EXC_EXTN_OLE_OR_DDE        = 0xFFFE;

const sal_uInt16 EXC_EXTN_EXPDDE_STDDOC     = 0x7FEA;  /// for export
const sal_uInt16 EXC_EXTN_EXPDDE            = 0x7FE2;  /// for export


// (0x0026, 0x0027, 0x0028, 0x0029) LEFT-, RIGHT-, TOP-, BOTTOMMARGIN ---------

const sal_uInt16 EXC_ID_LEFTMARGIN          = 0x0026;
const sal_uInt16 EXC_ID_RIGHTMARGIN         = 0x0027;
const sal_uInt16 EXC_ID_TOPMARGIN           = 0x0028;
const sal_uInt16 EXC_ID_BOTTOMMARGIN        = 0x0029;

// (0x0012, 0x0019) PROTECT and WINDOWPROTECT --------------------

const sal_uInt16 EXC_ID_PROTECT             = 0x0012;
const sal_uInt16 EXC_ID_WINDOWPROTECT       = 0x0019;


// (0x0031) FONT --------------------------------------------------------------

const sal_uInt16 EXC_ID_FONT                = 0x0031;

const sal_uInt16 EXC_FONT_APP               = 0;        /// Application font index.

const sal_uInt32 EXC_FONT_MAXCOUNT4         = 0x00FF;
const sal_uInt32 EXC_FONT_MAXCOUNT5         = 0x00FF;
const sal_uInt32 EXC_FONT_MAXCOUNT8         = 0xFFFF;

// attributes
const sal_uInt16 EXC_FONTATTR_NONE          = 0x0000;
const sal_uInt16 EXC_FONTATTR_BOLD          = 0x0001;
const sal_uInt16 EXC_FONTATTR_ITALIC        = 0x0002;
const sal_uInt16 EXC_FONTATTR_UNDERLINE     = 0x0004;
const sal_uInt16 EXC_FONTATTR_STRIKEOUT     = 0x0008;
const sal_uInt16 EXC_FONTATTR_OUTLINE       = 0x0010;
const sal_uInt16 EXC_FONTATTR_SHADOW        = 0x0020;

// weight
const sal_uInt16 EXC_FONTWGHT_DONTKNOW      = 0;
const sal_uInt16 EXC_FONTWGHT_THIN          = 100;
const sal_uInt16 EXC_FONTWGHT_ULTRALIGHT    = 200;
const sal_uInt16 EXC_FONTWGHT_LIGHT         = 300;
const sal_uInt16 EXC_FONTWGHT_SEMILIGHT     = 350;
const sal_uInt16 EXC_FONTWGHT_NORMAL        = 400;
const sal_uInt16 EXC_FONTWGHT_MEDIUM        = 500;
const sal_uInt16 EXC_FONTWGHT_SEMIBOLD      = 600;
const sal_uInt16 EXC_FONTWGHT_BOLD          = 700;
const sal_uInt16 EXC_FONTWGHT_ULTRABOLD     = 800;
const sal_uInt16 EXC_FONTWGHT_BLACK         = 900;

// families
const sal_uInt8 EXC_FONTFAM_DONTKNOW        = 0x00;
const sal_uInt8 EXC_FONTFAM_ROMAN           = 0x01;
const sal_uInt8 EXC_FONTFAM_SWISS           = 0x02;
const sal_uInt8 EXC_FONTFAM_SYSTEM          = EXC_FONTFAM_SWISS;
const sal_uInt8 EXC_FONTFAM_MODERN          = 0x03;
const sal_uInt8 EXC_FONTFAM_SCRIPT          = 0x04;
const sal_uInt8 EXC_FONTFAM_DECORATIVE      = 0x05;

// charsets
const sal_uInt8 EXC_FONTCSET_DONTKNOW       = 0x00;
const sal_uInt8 EXC_FONTCSET_MS_1252        = 0x00;
const sal_uInt8 EXC_FONTCSET_APPLE_ROMAN    = 0x00;
const sal_uInt8 EXC_FONTCSET_IBM_437        = 0x00;
const sal_uInt8 EXC_FONTCSET_IBM_850        = 0xFF;
const sal_uInt8 EXC_FONTCSET_IBM_860        = 0x00;
const sal_uInt8 EXC_FONTCSET_IBM_861        = 0x00;
const sal_uInt8 EXC_FONTCSET_IBM_863        = 0x00;
const sal_uInt8 EXC_FONTCSET_IBM_865        = 0x00;
const sal_uInt8 EXC_FONTCSET_SYSTEM         = 0x00;
const sal_uInt8 EXC_FONTCSET_SYMBOL         = 0x02;

// color
const sal_uInt16 EXC_FONT_AUTOCOLOR         = 0x7FFF;


// (0x0043, 0x0243, 0x0443, 0x00E0) XF ----------------------------------------

const sal_uInt16 EXC_ID2_XF                 = 0x0043;
const sal_uInt16 EXC_ID3_XF                 = 0x0243;
const sal_uInt16 EXC_ID4_XF                 = 0x0443;
const sal_uInt16 EXC_ID_XF                  = 0x00E0;

const sal_uInt32 EXC_XF_MAXCOUNT            = 4050;     /// Maximum number of all XF records.
const sal_uInt32 EXC_XF_MAXSTYLECOUNT       = 1536;     /// Arbitrary maximum number of style XFs.
const sal_uInt16 EXC_XF_USEROFFSET          = 21;       /// Index to first user defined record.
const sal_uInt16 EXC_XF_DEFAULTSTYLE        = 0;        /// Excel index to cefault style XF.
const sal_uInt16 EXC_XF_DEFAULTCELL         = 15;       /// Excel index to cefault cell XF.

const sal_uInt16 EXC_XF_NOCOLOR             = 0;
const sal_uInt16 EXC_XF_AUTOCOLOR           = 64;
const sal_uInt16 EXC_XF_TRANSPCOLOR         = 65;

const sal_uInt16 EXC_XF_LOCKED              = 0x0001;
const sal_uInt16 EXC_XF_HIDDEN              = 0x0002;
const sal_uInt16 EXC_XF_STYLE               = 0x0004;
const sal_uInt16 EXC_XF_STYLEPARENT         = 0x0FFF;   /// Syles don't have a parent.
const sal_uInt16 EXC_XF_WRAPPED             = 0x0008;   /// Automatic line break.
const sal_uInt16 EXC_XF_SHRINK              = 0x0010;   /// Shrink to fit into cell.

const sal_uInt8 EXC_XF_DIFF_VALFMT          = 0x01;
const sal_uInt8 EXC_XF_DIFF_FONT            = 0x02;
const sal_uInt8 EXC_XF_DIFF_ALIGN           = 0x04;
const sal_uInt8 EXC_XF_DIFF_BORDER          = 0x08;
const sal_uInt8 EXC_XF_DIFF_AREA            = 0x10;
const sal_uInt8 EXC_XF_DIFF_PROT            = 0x20;

const sal_uInt8 EXC_XF2_VALFMT_MASK         = 0x3F;
const sal_uInt8 EXC_XF2_LOCKED              = 0x40;
const sal_uInt8 EXC_XF2_HIDDEN              = 0x80;
const sal_uInt8 EXC_XF2_LEFTLINE            = 0x08;
const sal_uInt8 EXC_XF2_RIGHTLINE           = 0x10;
const sal_uInt8 EXC_XF2_TOPLINE             = 0x20;
const sal_uInt8 EXC_XF2_BOTTOMLINE          = 0x40;
const sal_uInt8 EXC_XF2_BACKGROUND          = 0x80;

const sal_uInt16 EXC_XF8_SHRINKTOFIT        = 0x0010;
const sal_uInt16 EXC_XF8_MERGE              = 0x0020;

const sal_uInt8 EXC_XF8_STACKED             = 0xFF;     /// Special rotation angle.


// (0x0055) DEFCOLWIDTH -------------------------------------------------------

const sal_uInt16 EXC_ID_DEFCOLWIDTH         = 0x0055;


// (0x0059, 0x005A) XCT, CRN --------------------------------------------------

const sal_uInt16 EXC_ID_XCT                 = 0x0059;
const sal_uInt16 EXC_ID_CRN                 = 0x005A;


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


// (0x0092) PALETTE -----------------------------------------------------------

const sal_uInt16 EXC_ID_PALETTE             = 0x0092;

const sal_uInt16 EXC_COLOR_OFFSET2          = 0;
const sal_uInt16 EXC_COLOR_OFFSET3          = 8;
const sal_uInt16 EXC_COLOR_OFFSET5          = 8;
const sal_uInt16 EXC_COLOR_OFFSET8          = 8;

const sal_uInt16 EXC_COLOR_BIFF2_BLACK      = 0;
const sal_uInt16 EXC_COLOR_BIFF2_WHITE      = 1;


// (0x00E9) BITMAP ------------------------------------------------------------

const sal_uInt16 EXC_ID_BITMAP              = 0x00E9;
const sal_uInt32 EXC_BITMAP_UNKNOWNID       = 0x00010009;
const sal_uInt32 EXC_BITMAP_MAXREC          = 0x201C;
const sal_uInt32 EXC_BITMAP_MAXCONT         = 0x2014;


// (0x00FC, 0x00FF) SST, EXTSST -----------------------------------------------

const sal_uInt16 EXC_ID_SST                 = 0x00FC;
const sal_uInt16 EXC_ID_EXTSST              = 0x00FF;


// (0x013D) TABID -------------------------------------------------------------

const sal_uInt16 EXC_ID_TABID               = 0x013D;


// (0x015F) LABELRANGES -------------------------------------------------------

const sal_uInt16 EXC_ID_LABELRANGES         = 0x015F;


// (0x01AE) SUPBOOK -----------------------------------------------------------

const sal_uInt16 EXC_ID_SUPBOOK             = 0x01AE;

const sal_uInt16 EXC_TAB_EXTERNAL           = 0xFFFE;     /// Addins, DDE, OLE.
const sal_uInt16 EXC_TAB_INVALID            = 0xFFFF;     /// Deleted 3D reference.

const sal_uInt16 EXC_SUPB_SELF              = 0x0401;
const sal_uInt16 EXC_SUPB_ADDIN             = 0x3A01;


// (0x01B2) DVAL, (0x01BE) DV -------------------------------------------------

const sal_uInt16 EXC_ID_DVAL                = 0x01B2;
const sal_uInt16 EXC_ID_DV                  = 0x01BE;

const sal_uInt32 EXC_DVAL_NOOBJ             = 0xFFFFFFFF;

// data validation flags
const sal_uInt32 EXC_DV_STRINGLIST          = 0x00000080;
const sal_uInt32 EXC_DV_IGNOREBLANK         = 0x00000100;
const sal_uInt32 EXC_DV_SUPPRESSDROPDOWN    = 0x00000200;
const sal_uInt32 EXC_DV_SHOWPROMPT          = 0x00040000;
const sal_uInt32 EXC_DV_SHOWERROR           = 0x00080000;

// data validation data mode
const sal_uInt32 EXC_DV_MODE_MASK           = 0x0000000F;
const sal_uInt32 EXC_DV_MODE_ANY            = 0x00000000;
const sal_uInt32 EXC_DV_MODE_WHOLE          = 0x00000001;
const sal_uInt32 EXC_DV_MODE_DECIMAL        = 0x00000002;
const sal_uInt32 EXC_DV_MODE_LIST           = 0x00000003;
const sal_uInt32 EXC_DV_MODE_DATE           = 0x00000004;
const sal_uInt32 EXC_DV_MODE_TIME           = 0x00000005;
const sal_uInt32 EXC_DV_MODE_TEXTLEN        = 0x00000006;
const sal_uInt32 EXC_DV_MODE_CUSTOM         = 0x00000007;

// data validation conditions
const sal_uInt32 EXC_DV_COND_MASK           = 0x00F00000;
const sal_uInt32 EXC_DV_COND_BETWEEN        = 0x00000000;
const sal_uInt32 EXC_DV_COND_NOTBETWEEN     = 0x00100000;
const sal_uInt32 EXC_DV_COND_EQUAL          = 0x00200000;
const sal_uInt32 EXC_DV_COND_NOTEQUAL       = 0x00300000;
const sal_uInt32 EXC_DV_COND_GREATER        = 0x00400000;
const sal_uInt32 EXC_DV_COND_LESS           = 0x00500000;
const sal_uInt32 EXC_DV_COND_EQGREATER      = 0x00600000;
const sal_uInt32 EXC_DV_COND_EQLESS         = 0x00700000;

// data validation error style
const sal_uInt32 EXC_DV_ERROR_MASK          = 0x00000070;
const sal_uInt32 EXC_DV_ERROR_STOP          = 0x00000000;
const sal_uInt32 EXC_DV_ERROR_WARNING       = 0x00000010;
const sal_uInt32 EXC_DV_ERROR_INFO          = 0x00000020;


// (0x01B8) HLINK -------------------------------------------------------------

const sal_uInt16 EXC_ID_HLINK               = 0x01B8;

const sal_uInt32 EXC_HLINK_BODY             = 0x00000001;   /// Contains file link or URL.
const sal_uInt32 EXC_HLINK_ABS              = 0x00000002;   /// Absolute path.
const sal_uInt32 EXC_HLINK_DESCR            = 0x00000014;   /// Description.
const sal_uInt32 EXC_HLINK_MARK             = 0x00000008;   /// Text mark.
const sal_uInt32 EXC_HLINK_FRAME            = 0x00000080;   /// Target frame.
const sal_uInt32 EXC_HLINK_UNC              = 0x00000100;   /// UNC path.


// (0x0293) STYLE -------------------------------------------------------------

const sal_uInt16 EXC_ID_STYLE               = 0x0293;

const sal_uInt16 EXC_STYLE_BUILTIN          = 0x8000;
const sal_uInt16 EXC_STYLE_XFMASK           = 0x0FFF;

const sal_uInt8 EXC_STYLE_NORMAL            = 0x00;         /// "Normal" style.
const sal_uInt8 EXC_STYLE_ROWLEVEL          = 0x01;         /// "RowLevel_*" styles.
const sal_uInt8 EXC_STYLE_COLLEVEL          = 0x02;         /// "ColLevel_*" styles.
const sal_uInt8 EXC_STYLE_COMMA             = 0x03;         /// "Comma" style.
const sal_uInt8 EXC_STYLE_CURRENCY          = 0x04;         /// "Currency" style.
const sal_uInt8 EXC_STYLE_PERCENT           = 0x05;         /// "Percent" style.
const sal_uInt8 EXC_STYLE_COMMA_0           = 0x06;         /// "Comma [0]" style.
const sal_uInt8 EXC_STYLE_CURRENCY_0        = 0x07;         /// "Currency [0]" style.
const sal_uInt8 EXC_STYLE_USERDEF           = 0xFF;         /// No built-in style.

const sal_uInt8 EXC_STYLE_LEVELCOUNT        = 7;            /// Number of outline level styles.
const sal_uInt8 EXC_STYLE_NOLEVEL           = 0xFF;         /// Default value for unused level.


// web queries ================================================================

#define EXC_WEBQRY_FILTER                   "calc_HTML_WebQuery"

// (0x00CD) SXSTRING
const sal_uInt16 EXC_ID_SXSTRING            = 0x00CD;

// (0x00DC) PARAMQRY
const sal_uInt16 EXC_ID_PQRY                = 0x00DC;
const sal_uInt16 EXC_PQRY_DOC               = 0x0000;       /// Entire document.
const sal_uInt16 EXC_PQRY_TABLES            = 0x0100;       /// All tables.
const sal_uInt16 EXC_PQRY_DEFAULTFLAGS      = 0x0044;       /// Flags for export.

// (0x01AD) QSI
const sal_uInt16 EXC_ID_QSI                 = 0x01AD;
const sal_uInt16 EXC_QSI_DEFAULTFLAGS       = 0x0349;       /// Flags for export.

// (0x0802) unknown record
const sal_uInt16 EXC_ID_0802                = 0x0802;

// (0x0803) WEBQRYSETTINGS
const sal_uInt16 EXC_ID_WQSETT              = 0x0803;
const sal_uInt16 EXC_WQSETT_ALL             = 0x0000;       /// All tables or entire document.
const sal_uInt16 EXC_WQSETT_SPECTABLES      = 0x0002;       /// Specific tables.
const sal_uInt16 EXC_WQSETT_DEFAULTFLAGS    = 0x0023;       /// Flags for export.
const sal_uInt16 EXC_WQSETT_NOFORMAT        = 0x0001;
const sal_uInt16 EXC_WQSETT_FORMATRTF       = 0x0002;
const sal_uInt16 EXC_WQSETT_FORMATFULL      = 0x0003;

// (0x0804) WEBQRYTABLES
const sal_uInt16 EXC_ID_WQTABLES            = 0x0804;


// ============================================================================

#endif

