/*************************************************************************
 *
 *  $RCSfile: excdefs.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-29 09:15:20 $
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
//_________________________________________________________
//
// excdefs.hxx: constants for import/export
//_________________________________________________________


//_________________________________________________________
// ExcETabNumBuffer

#define EXC_TABBUF_INVALID          0xFFFF
#define EXC_TABBUF_MASKTAB          0x0000FFFF
#define EXC_TABBUF_MASKFLAGS        0xFFFF0000
#define EXC_TABBUF_FLAGIGNORE       0x00010000
#define EXC_TABBUF_FLAGEXT          0x00020000

//_________________________________________________________
// (0x0018, 0x0218) NAME

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

//_________________________________________________________
// (0x0031) FONT

// attributes
#define EXC_FONTATTR_NONE           0x0000
#define EXC_FONTATTR_ITALIC         0x0002
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

// underline
#define EXC_FONTUL_NONE             0x00
#define EXC_FONTUL_SINGLE           0x01
#define EXC_FONTUL_DOUBLE           0x02
#define EXC_FONTUL_DOTTED           0x21

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

//_________________________________________________________
// (0x0059, 0x005A) CRN, XCT

// CRN data types
#define EXC_CRN_DOUBLE              0x01
#define EXC_CRN_STRING              0x02
#define EXC_CRN_BOOL                0x04
#define EXC_CRN_ERROR               0x10

//_________________________________________________________
// (0x0092) PALETTE (class ExcPalette2)

// color types
#define EXC_COLOR_CELLTEXT          0x0001
#define EXC_COLOR_CELLBORDER        0x0002
#define EXC_COLOR_CELLBGROUND       0x0003
#define EXC_COLOR_CHARTTEXT         0x0011
#define EXC_COLOR_CHARTLINE         0x0012
#define EXC_COLOR_CHARTAREA         0x0013

// special color indices
#define EXC_COLIND_AUTOTEXT         77
#define EXC_COLIND_AUTOLINE         77
#define EXC_COLIND_AUTOFILLBG       77
#define EXC_COLIND_AUTOFILLFG       78

// internal
#define EXC_PAL2_INDEXBASE          0xFFFF0000
#define EXC_PAL2_IGNORE             0xFFFFFFFF

//_________________________________________________________
// (0x009B, 0x009D, 0x009E) AUTOFILTER

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

//_________________________________________________________
// (0x00AE, 0x00AF) SCENARIO, SCENMAN

#define EXC_SCEN_MAXCELL            32
#define EXC_SCEN_MAXSTRINGLEN       0xFF

//_________________________________________________________
// (0x007D) COLINFO

// flags
#define EXC_COL_HIDDEN              0x0001
#define EXC_COL_COLLAPSED           0x1000

// outline
#define EXC_COL_LEVELFLAGS(nOL)     ((nOL & 0x0007) << 8)
#define EXC_COL_GETLEVEL(nFlag)     ((nFlag & 0x0700) >> 8)

//_________________________________________________________
// (0x0208) ROW

// flags
#define EXC_ROW_COLLAPSED           0x0010
#define EXC_ROW_ZEROHEIGHT          0x0020
#define EXC_ROW_UNSYNCED            0x0040
#define EXC_ROW_GHOSTDIRTY          0x0080

// outline
#define EXC_ROW_LEVELFLAGS(nOL)     (nOL & 0x0007)
#define EXC_ROW_GETLEVEL(nFlag)     (nFlag & 0x0007)

// unknown, always save
#define EXC_ROW_FLAGCOMMON          0x0100

// row height
#define EXC_ROW_VALZEROHEIGHT       0x00FF
#define EXC_ROW_FLAGDEFHEIGHT       0x8000

//_________________________________________________________
// Specials for outlines

#define EXC_OUTLINE_MAX             7
#define EXC_OUTLINE_COUNT           (EXC_OUTLINE_MAX + 1)

//_________________________________________________________
// data pilot / pivot tables

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

//_________________________________________________________
// defines for change tracking

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
#define EXC_CHTR_ACCEPT             0x0001
#define EXC_CHTR_REJECT             0x0003

// list for 3D references - internal or external
#define EXC_CHTR_3DREF_INT          0x0001
#define EXC_CHTR_3DREF_EXT          0x0002

//_________________________________________________________

#endif // _EXCDEFS_HXX
