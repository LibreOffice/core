/*************************************************************************
 *
 *  $RCSfile: excdefs.hxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:36:48 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// Supbooks, ExcETabNumBuffer =================================================

#define EXC_TABBUF_INVALID          0xFFFF

// (0x001C) NOTE ==============================================================

#define EXC_NOTE5_MAXTEXT           2048

// (0x0031) FONT ==============================================================

// color
#define EXC_FONTCOL_IGNORE          0x7FFF

// height
#define EXC_FONTHGHT_COEFF          20.0

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
#define EXC_OBJT_DRAWING            0x1E

// (0x0092) PALETTE ===========================================================

// special color indices
#define EXC_COLIND_AUTOTEXT         77
#define EXC_COLIND_AUTOLINE         77
#define EXC_COLIND_AUTOFILLBG       77
#define EXC_COLIND_AUTOFILLFG       78

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

// (0x00E5) CELLMERGING =======================================================

#define EXC_MERGE_MAXCOUNT          1024

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

