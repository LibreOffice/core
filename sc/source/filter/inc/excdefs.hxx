/*************************************************************************
 *
 *  $RCSfile: excdefs.hxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:04:12 $
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

// (0x001C) NOTE ==============================================================

#define EXC_NOTE5_MAXTEXT           2048

// (0x0031) FONT ==============================================================

// color
#define EXC_FONTCOL_IGNORE          0x7FFF

// height
#define EXC_FONTHGHT_COEFF          20.0

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
const sal_uInt16 EXC_WIN2_MIRRORED          = 0x0040;
#define EXC_WIN2_OUTLINE            0x0080
#define EXC_WIN2_FROZENNOSPLIT      0x0100
#define EXC_WIN2_SELECTED           0x0200
#define EXC_WIN2_DISPLAYED          0x0400

// Specials for outlines ======================================================

#define EXC_OUTLINE_MAX             7
#define EXC_OUTLINE_COUNT           (EXC_OUTLINE_MAX + 1)

// defines for change tracking ================================================

#define EXC_STREAM_USERNAMES        CREATE_STRING( "User Names" )
#define EXC_STREAM_REVLOG           CREATE_STRING( "Revision Log" )

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

