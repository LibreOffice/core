/*************************************************************************
 *
 *  $RCSfile: excdefs.hxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: dr $ $Date: 2002-05-22 11:10:31 $
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

#ifndef _EXCDEFS_HXX
#define _EXCDEFS_HXX

// excdefs.hxx: defines for Excel import/export ===============================

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// unicode strings ============================================================

#define EXC_STR_16BIT               0x01
#define EXC_STR_FAREAST             0x04
#define EXC_STR_RICH                0x08
#define EXC_STR_UNKNOWN             0xF2

// in/out stream ==============================================================

const sal_uInt32 RECORD_SEEK_TO_BEGIN       = 0UL;
const sal_uInt32 RECORD_SEEK_TO_END         = ~0UL;

const sal_uInt16 EXC_MAXRECLEN_BIFF5        = 2080;
const sal_uInt16 EXC_MAXRECLEN_BIFF8        = 8224;

const sal_uInt16 EXC_ID_UNKNOWN             = 0xFFFF;

// cached values list (EXTERNNAME, ptgArray, ...) =============================

#define EXC_CACHEDVAL_EMPTY         0x00
#define EXC_CACHEDVAL_DOUBLE        0x01
#define EXC_CACHEDVAL_STRING        0x02
#define EXC_CACHEDVAL_BOOL          0x04
#define EXC_CACHEDVAL_ERROR         0x10

// Supbooks, ExcETabNumBuffer =================================================

#define EXC_TAB_INVALID             0xFFFF      // Deleted refs
#define EXC_TAB_EXTERNAL            0xFFFE      // Addins, DDE, OLE
#define EXC_TABBUF_INVALID          0xFFFF
#define EXC_TABBUF_MASKTAB          0x0000FFFFUL
#define EXC_TABBUF_MASKFLAGS        0xFFFF0000UL
#define EXC_TABBUF_FLAGIGNORE       0x00010000UL
#define EXC_TABBUF_FLAGEXT          0x00020000UL

// Style info =================================================================

/** Horizontal alignment of cell contents. */
enum XclHorAlign
{
    xlHAlignGeneral                 = 0x00,
    xlHAlignLeft                    = 0x01,
    xlHAlignCenter                  = 0x02,
    xlHAlignRight                   = 0x03,
    xlHAlignFill                    = 0x04,
    xlHAlignJustify                 = 0x05,
    xlHAlignCenterAcrSel            = 0x06,
    xlHAlignParent                  = 0xFF
};

/** Vertical alignment of cell contents. */
enum XclVerAlign
{
    xlVAlignTop                     = 0x00,
    xlVAlignCenter                  = 0x01,
    xlVAlignBottom                  = 0x02,
    xlVAlignJustify                 = 0x03,
    xlVAlignParent                  = 0xFF
};

/** Text orientation. */
enum XclTextOrient
{
    xlTextOrientNoRot               = 0x00,
    xlTextOrientTopBottom           = 0x01,
    xlTextOrient90ccw               = 0x02,
    xlTextOrient90cw                = 0x03,
    xlTextOrientRot                 = 0x04,
    xlTextOrientParent              = 0xFF
};

/** Text wrap (automatic line break). */
enum XclTextWrap
{
    xlTextWrapNo                    = 0x00,
    xlTextWrapYes                   = 0x01,
    xlTextWrapParent                = 0xFF
};

#define EXC_ROT_STACKED             0xFF


// (0x0014, 0x0015) HEADER, FOOTER ============================================

const sal_uInt16 EXC_ID_HEADER              = 0x0014;
const sal_uInt16 EXC_ID_FOOTER              = 0x0015;

// (0x0018, 0x0218) NAME ======================================================

// flags
#define EXC_NAME_HIDDEN             0x0001
#define EXC_NAME_FUNC               0x0002
#define EXC_NAME_VB                 0x0004
#define EXC_NAME_PROC               0x0008
#define EXC_NAME_CALCEXP            0x0010
#define EXC_NAME_BUILTIN            0x0020
#define EXC_NAME_FGROUPMASK         0x0FC0
#define EXC_NAME_BIG                0x1000

// "BuiltIn" names
#define EXC_BUILTIN_CONSOLIDATEAREA 0x00
#define EXC_BUILTIN_AUTOOPEN        0x01
#define EXC_BUILTIN_AUTOCLOSE       0x02
#define EXC_BUILTIN_EXTRACT         0x03
#define EXC_BUILTIN_DATABASE        0x04
#define EXC_BUILTIN_CRITERIA        0x05
#define EXC_BUILTIN_PRINTAREA       0x06
#define EXC_BUILTIN_PRINTTITLES     0x07
#define EXC_BUILTIN_RECORDER        0x08
#define EXC_BUILTIN_DATAFORM        0x09
#define EXC_BUILTIN_AUTOACTIVATE    0x0A
#define EXC_BUILTIN_AUTODEACTIVATE  0x0B
#define EXC_BUILTIN_SHEETTITLE      0x0C
#define EXC_BUILTIN_AUTOFILTER      0x0D
#define EXC_BUILTIN_UNKNOWN         0x0E

// (0x001C) NOTE ==============================================================

#define EXC_NOTE5_MAXTEXT           2048

// (0x0023) EXTERNNAME ========================================================

#define EXC_EXTN_BUILTIN            0x0001
#define EXC_EXTN_OLE                0x0010

// (0x0026, 0x0027, 0x0028, 0x0029) LEFT-, RIGHT-, TOP-, BOTTOMMARGIN =========

const sal_uInt16 EXC_ID_LEFTMARGIN          = 0x0026;
const sal_uInt16 EXC_ID_RIGHTMARGIN         = 0x0027;
const sal_uInt16 EXC_ID_TOPMARGIN           = 0x0028;
const sal_uInt16 EXC_ID_BOTTOMMARGIN        = 0x0029;

/** The type of a margin value. */
enum XclMarginType
{
    xlLeftMargin,
    xlRightMargin,
    xlTopMargin,
    xlBottomMargin
};

// (0x0031) FONT ==============================================================

// attributes
#define EXC_FONTATTR_NONE           0x0000
#define EXC_FONTATTR_BOLD           0x0001
#define EXC_FONTATTR_ITALIC         0x0002
#define EXC_FONTATTR_UNDERLINE      0x0004
#define EXC_FONTATTR_STRIKEOUT      0x0008
#define EXC_FONTATTR_OUTLINE        0x0010
#define EXC_FONTATTR_SHADOW         0x0020

// color
#define EXC_FONTCOL_IGNORE          0x7FFF

// weight
#define EXC_FONTWGHT_DONTKNOW       0
#define EXC_FONTWGHT_THIN           100
#define EXC_FONTWGHT_ULTRALIGHT     200
#define EXC_FONTWGHT_LIGHT          300
#define EXC_FONTWGHT_SEMILIGHT      350
#define EXC_FONTWGHT_NORMAL         400
#define EXC_FONTWGHT_MEDIUM         500
#define EXC_FONTWGHT_SEMIBOLD       600
#define EXC_FONTWGHT_BOLD           700
#define EXC_FONTWGHT_ULTRABOLD      800
#define EXC_FONTWGHT_BLACK          900

// height
#define EXC_FONTHGHT_COEFF          20.0

/** Text underline style. */
enum XclUnderline
{
    xlUnderlNone                    = 0x00,
    xlUnderlSingle                  = 0x01,
    xlUnderlDouble                  = 0x02,
    xlUnderlSingleAcc               = 0x21,
    xlUnderlDoubleAcc               = 0x22
};

/** Super-/subscript type. */
enum XclEscapement
{
    xlEscNone                       = 0x00,
    xlEscSuper                      = 0x01,
    xlEscSub                        = 0x02
};

// families
#define EXC_FONTFAM_DONTKNOW        0x00
#define EXC_FONTFAM_ROMAN           0x01
#define EXC_FONTFAM_SWISS           0x02
#define EXC_FONTFAM_SYSTEM          EXC_FONTFAM_SWISS
#define EXC_FONTFAM_MODERN          0x03
#define EXC_FONTFAM_SCRIPT          0x04
#define EXC_FONTFAM_DECORATIVE      0x05

// charsets
#define EXC_FONTCSET_DONTKNOW       0x00
#define EXC_FONTCSET_MS_1252        0x00
#define EXC_FONTCSET_APPLE_ROMAN    0x00
#define EXC_FONTCSET_IBM_437        0x00
#define EXC_FONTCSET_IBM_850        0xFF
#define EXC_FONTCSET_IBM_860        0x00
#define EXC_FONTCSET_IBM_861        0x00
#define EXC_FONTCSET_IBM_863        0x00
#define EXC_FONTCSET_IBM_865        0x00
#define EXC_FONTCSET_SYSTEM         0x00
#define EXC_FONTCSET_SYMBOL         0x02

// (0x003C) CONTINUE ==========================================================

#define EXC_CONT                    0x003C

// (0x0043, 0x00E0) XF ========================================================

#define EXC_XF_LOCKED               0x0001
#define EXC_XF_HIDDEN               0x0002
#define EXC_XF_STYLE                0x0004

#define EXC_XF_DIFF_VALFMT          0x0400
#define EXC_XF_DIFF_FONT            0x0800
#define EXC_XF_DIFF_ALIGN           0x1000
#define EXC_XF_DIFF_BORDER          0x2000
#define EXC_XF_DIFF_AREA            0x4000
#define EXC_XF_DIFF_PROT            0x8000

#define EXC_XF2_LLINE               0x08
#define EXC_XF2_RLINE               0x10
#define EXC_XF2_TLINE               0x20
#define EXC_XF2_BLINE               0x40
#define EXC_XF2_VALFMT_MASK         0x3F
#define EXC_XF2_LOCKED              0x40
#define EXC_XF2_HIDDEN              0x80

#define EXC_XF8_SHRINKTOFIT         0x0010
#define EXC_XF8_MERGE               0x0020

#define EXC_XF_GETPARENT(nFlag)         static_cast< sal_uInt16 >( (nFlag & 0xFFF0) >> 4 )
#define EXC_XF_GETHORALIGN(nFlag)       static_cast< XclHorAlign >( nFlag & 0x0007 )
#define EXC_XF_GETVERALIGN(nFlag)       static_cast< XclVerAlign >( (nFlag & 0x0030) >> 4 )
#define EXC_XF_GETTEXTWRAP(nFlag)       static_cast< XclTextWrap >( (nFlag & 0x0008) >> 3 )
#define EXC_XF4_GETTEXTORIENT(nFlag)    static_cast< XclTextOrient >( (nFlag & 0x00C0) >> 6 )
#define EXC_XF5_GETTEXTORIENT(nFlag)    static_cast< XclTextOrient >( (nFlag & 0x0300) >> 8 )

#define EXC_XF8_GETINDENT(nFlag)        (static_cast< sal_uInt16 >( nFlag & 0x000F ) * 200)
#define EXC_XF8_GETROT(nFlag)           static_cast< sal_uInt8 >( (nFlag & 0xFF00) >> 8 )

// (0x005D) OBJ ===============================================================

#define EXC_OBJT_LINE               0x01
#define EXC_OBJT_RECT               0x02
#define EXC_OBJT_ELLIP              0x03
#define EXC_OBJT_ARC                0x04
#define EXC_OBJT_CHART              0x05
#define EXC_OBJT_TEXT               0x06
#define EXC_OBJT_PICT               0x08
#define EXC_OBJT_POLYGON            0x09
#define EXC_OBJT_NOTE               0x19

// (0x007E) RK ================================================================

#define EXC_RK_100FLAG              0x01
#define EXC_RK_INTFLAG              0x02

#define EXC_RK_DBL                  0x00
#define EXC_RK_DBL100               EXC_RK_100FLAG
#define EXC_RK_INT                  EXC_RK_INTFLAG
#define EXC_RK_INT100               (EXC_RK_100FLAG|EXC_RK_INTFLAG)

// (0x0081) WSBOOL ============================================================

const sal_uInt16 EXC_ID_WSBOOL              = 0x0081;

const sal_uInt16 EXC_WSBOOL_ROWBELOW        = 0x0040;
const sal_uInt16 EXC_WSBOOL_COLBELOW        = 0x0080;
const sal_uInt16 EXC_WSBOOL_FITTOPAGE       = 0x0100;

const sal_uInt16 EXC_WSBOOL_DEFAULTFLAGS    = 0x04C1;

// (0x0092) PALETTE ===========================================================

// color types
#define EXC_COLOR_CELLTEXT          0x0001
#define EXC_COLOR_CELLBORDER        0x0002
#define EXC_COLOR_CELLBGROUND       0x0003
#define EXC_COLOR_CHARTTEXT         0x0011
#define EXC_COLOR_CHARTLINE         0x0012
#define EXC_COLOR_CHARTAREA         0x0013
#define EXC_COLOR_GRID              0x0101

// special color indices
#define EXC_COLIND_AUTOTEXT         77
#define EXC_COLIND_AUTOLINE         77
#define EXC_COLIND_AUTOFILLBG       77
#define EXC_COLIND_AUTOFILLFG       78

// internal
#define EXC_PAL2_INDEXBASE          0xFFFF0000
#define EXC_PAL2_IGNORE             0xFFFFFFFF

// (0x0093) STYLE =============================================================

#define EXC_STYLE_BUILTIN           0x8000
#define EXC_STYLE_MASK              0x0FFF

// (0x009B, 0x009D, 0x009E) AUTOFILTER ========================================

// flags
#define EXC_AFFLAG_AND              0x0000
#define EXC_AFFLAG_OR               0x0001
#define EXC_AFFLAG_ANDORMASK        0x0003
#define EXC_AFFLAG_SIMPLE1          0x0004
#define EXC_AFFLAG_SIMPLE2          0x0008
#define EXC_AFFLAG_TOP10            0x0010
#define EXC_AFFLAG_TOP10TOP         0x0020
#define EXC_AFFLAG_TOP10PERC        0x0040

// data types
#define EXC_AFTYPE_NOTUSED          0x00
#define EXC_AFTYPE_RK               0x02
#define EXC_AFTYPE_DOUBLE           0x04
#define EXC_AFTYPE_STRING           0x06
#define EXC_AFTYPE_BOOLERR          0x08
#define EXC_AFTYPE_INVALID          0x0A
#define EXC_AFTYPE_EMPTY            0x0C
#define EXC_AFTYPE_NOTEMPTY         0x0E

// comparison operands
#define EXC_AFOPER_NONE             0x00
#define EXC_AFOPER_LESS             0x01
#define EXC_AFOPER_EQUAL            0x02
#define EXC_AFOPER_LESSEQUAL        0x03
#define EXC_AFOPER_GREATER          0x04
#define EXC_AFOPER_NOTEQUAL         0x05
#define EXC_AFOPER_GREATEREQUAL     0x06

// (0x00AE, 0x00AF) SCENARIO, SCENMAN =========================================

#define EXC_SCEN_MAXCELL            32
#define EXC_SCEN_MAXSTRINGLEN       0xFF

// (0x00E5) CELLMERGING =======================================================

#define EXC_MERGE_MAXCOUNT          1024

// (0x01B8) HLINK =============================================================

const sal_uInt16 EXC_ID_HLINK               = 0x01B8;

const sal_uInt32 EXC_HLINK_BODY             = 0x00000001;   // contains file link or URL
const sal_uInt32 EXC_HLINK_ABS              = 0x00000002;   // absolute path
const sal_uInt32 EXC_HLINK_DESCR            = 0x00000014;   // description
const sal_uInt32 EXC_HLINK_MARK             = 0x00000008;   // text mark
const sal_uInt32 EXC_HLINK_FRAME            = 0x00000080;   // target frame
const sal_uInt32 EXC_HLINK_UNC              = 0x00000100;   // UNC path

const sal_uInt32 EXC_HLINK_GUID_FILE        = 0x00000303;   // start of file moniker GUID
const sal_uInt32 EXC_HLINK_GUID_URL         = 0x79EAC9E0;   // start of URL moniker GUID

// (0x007D) COLINFO ===========================================================

// flags
#define EXC_COL_HIDDEN              0x0001
#define EXC_COL_COLLAPSED           0x1000

// outline
#define EXC_COL_LEVELFLAGS(nOL)     ((nOL & 0x0007) << 8)
#define EXC_COL_GETLEVEL(nFlag)     ((nFlag & 0x0700) >> 8)

// (0x0208) ROW ===============================================================

// flags
#define EXC_ROW_COLLAPSED           0x0010
#define EXC_ROW_ZEROHEIGHT          0x0020
#define EXC_ROW_UNSYNCED            0x0040
#define EXC_ROW_GHOSTDIRTY          0x0080
#define EXC_ROW_XFMASK              0x0FFF

// outline
#define EXC_ROW_LEVELFLAGS(nOL)     (nOL & 0x0007)
#define EXC_ROW_GETLEVEL(nFlag)     (nFlag & 0x0007)

// unknown, always save
#define EXC_ROW_FLAGCOMMON          0x0100

// row height
#define EXC_ROW_VALZEROHEIGHT       0x00FF
#define EXC_ROW_FLAGDEFHEIGHT       0x8000

// (0x0236) TABLE =============================================================

#define EXC_TABOP_CALCULATE         0x0003
#define EXC_TABOP_ROW               0x0004
#define EXC_TABOP_BOTH              0x0008

// (0x023E) WINDOW2 ===========================================================

#define EXC_WIN2_SHOWFORMULAS       0x0001
#define EXC_WIN2_SHOWGRID           0x0002
#define EXC_WIN2_SHOWHEADINGS       0x0004
#define EXC_WIN2_FROZEN             0x0008
#define EXC_WIN2_SHOWZEROS          0x0010
#define EXC_WIN2_DEFAULTCOLOR       0x0020
#define EXC_WIN2_OUTLINE            0x0080
#define EXC_WIN2_FROZENNOSPLIT      0x0100
#define EXC_WIN2_SELECTED           0x0200
#define EXC_WIN2_DISPLAYED          0x0400

// (0x015F) LABELRANGES =======================================================

const sal_uInt16 EXC_ID_LABELRANGES         = 0x015F;

// (0x01B2) DVAL, (0x01BE) DV =================================================

const sal_uInt16 EXC_ID_DVAL                = 0x01B2;
const sal_uInt16 EXC_ID_DV                  = 0x01BE;

// data validation flags
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

// Specials for outlines ======================================================

#define EXC_OUTLINE_MAX             7
#define EXC_OUTLINE_COUNT           (EXC_OUTLINE_MAX + 1)

// data pilot / pivot tables ==================================================

// subtotal functions
#define EXC_PIVOT_SUBT_SUM          0x0000
#define EXC_PIVOT_SUBT_COUNT        0x0001
#define EXC_PIVOT_SUBT_AVERAGE      0x0002
#define EXC_PIVOT_SUBT_MAX          0x0003
#define EXC_PIVOT_SUBT_MIN          0x0004
#define EXC_PIVOT_SUBT_PROD         0x0005
#define EXC_PIVOT_SUBT_COUNTNUM     0x0006
#define EXC_PIVOT_SUBT_STDDEV       0x0007
#define EXC_PIVOT_SUBT_STDDEVP      0x0008
#define EXC_PIVOT_SUBT_VAR          0x0009
#define EXC_PIVOT_SUBT_VARP         0x000A

// field orientation
#define EXC_PIVOT_AXIS_NONE         0x0000
#define EXC_PIVOT_AXIS_ROW          0x0001
#define EXC_PIVOT_AXIS_COL          0x0002
#define EXC_PIVOT_AXIS_PAGE         0x0004
#define EXC_PIVOT_AXIS_DATA         0x0008
#define EXC_PIVOT_AXIS_RCP_MASK     (EXC_PIVOT_AXIS_ROW|EXC_PIVOT_AXIS_COL|EXC_PIVOT_AXIS_PAGE)

// misc xcl record flags
#define EXC_SXVIEW_COMMON           0x0208
#define EXC_SXVIEW_ROWGRAND         0x0001
#define EXC_SXVIEW_COLGRAND         0x0002

#define EXC_SXVDEX_COMMON           0x0A00141E
#define EXC_SXVDEX_SHOWALL          0x00000001

#define EXC_SXVI_HIDDEN             0x0001
#define EXC_SXVI_HIDEDETAIL         0x0002
#define EXC_SXVI_FORMULA            0x0004
#define EXC_SXVI_MISSING            0x0008

#define EXC_SXVS_EXCEL              0x0001
#define EXC_SXVS_EXTERN             0x0002
#define EXC_SXVS_MULTICONSR         0x0004
#define EXC_SXVS_PIVOTTAB           0x0008
#define EXC_SXVS_SCENMAN            0x0010

#define EXC_SXIVD_IDDATA            0xFFFE

// pivot cache record flags
#define EXC_SXFIELD_COMMON          0x0001
#define EXC_SXFIELD_READLATER       0x0002
#define EXC_SXFIELD_16BIT           0x0200

#define EXC_SXITEM_

// defines for web queries ====================================================
// (0x00DC) SXEXT, PARAMQRY, (0x0803) WEBQRYSETTINGS

#define EXC_WEBQRY_FILTER           "calc_HTML_WebQuery"

#define EXC_QSI_COMMON              0x0349      // common flags for export

#define EXC_PQRY_TABLES             0x0100
#define EXC_PQRY_COMMON             0x0044      // common flags for export

#define EXC_WQSETT_SPECTABLES       0x0002
#define EXC_WQSETT_COMMON           0x0023      // common flags for export
#define EXC_WQSETT_NOFORMAT         0x0001
#define EXC_WQSETT_FORMATRTF        0x0002
#define EXC_WQSETT_FORMATFULL       0x0003

// defines for change tracking ================================================

// opcodes
#define EXC_CHTR_OP_COLFLAG         0x0001
#define EXC_CHTR_OP_DELFLAG         0x0002
#define EXC_CHTR_OP_INSROW          0x0000
#define EXC_CHTR_OP_INSCOL          EXC_CHTR_OP_COLFLAG
#define EXC_CHTR_OP_DELROW          EXC_CHTR_OP_DELFLAG
#define EXC_CHTR_OP_DELCOL          (EXC_CHTR_OP_COLFLAG|EXC_CHTR_OP_DELFLAG)
#define EXC_CHTR_OP_MOVE            0x0004
#define EXC_CHTR_OP_INSTAB          0x0005
#define EXC_CHTR_OP_CELL            0x0008
#define EXC_CHTR_OP_RENAME          0x0009
#define EXC_CHTR_OP_NAME            0x000A
#define EXC_CHTR_OP_FORMAT          0x000B
#define EXC_CHTR_OP_UNKNOWN         0xFFFF

// data types
#define EXC_CHTR_TYPE_MASK          0x0007
#define EXC_CHTR_TYPE_FORMATMASK    0xFF00
#define EXC_CHTR_TYPE_EMPTY         0x0000
#define EXC_CHTR_TYPE_RK            0x0001
#define EXC_CHTR_TYPE_DOUBLE        0x0002
#define EXC_CHTR_TYPE_STRING        0x0003
#define EXC_CHTR_TYPE_BOOL          0x0004
#define EXC_CHTR_TYPE_FORMULA       0x0005

// accept flags
#define EXC_CHTR_NOTHING            0x0000
#define EXC_CHTR_ACCEPT             0x0001
#define EXC_CHTR_REJECT             0x0003

// ============================================================================

#endif // _EXCDEFS_HXX

