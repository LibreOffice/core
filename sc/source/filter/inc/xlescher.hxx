/*************************************************************************
 *
 *  $RCSfile: xlescher.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:49:14 $
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

#ifndef SC_XLESCHER_HXX
#define SC_XLESCHER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif


// (0x005D) OBJ ===============================================================

const sal_uInt16 EXC_ID_OBJ                 = 0x005D;

const sal_uInt16 EXC_OBJ_INVALID_ID         = 0x0000;

// sub records
const sal_uInt16 EXC_ID_OBJ_FTEND           = 0x0000;   /// End of OBJ.
const sal_uInt16 EXC_ID_OBJ_FTGMO           = 0x0006;   /// Group marker.
const sal_uInt16 EXC_ID_OBJ_FTCF            = 0x0007;   /// Clipboard format.
const sal_uInt16 EXC_ID_OBJ_FTPIOGRBIT      = 0x0008;   /// Option flags.
const sal_uInt16 EXC_ID_OBJ_FTPICTFMLA      = 0x0009;   /// OLE link formula.
const sal_uInt16 EXC_ID_OBJ_FTCBLS          = 0x000A;   /// Check box/radio button data.
const sal_uInt16 EXC_ID_OBJ_FTSBS           = 0x000C;   /// Scroll bar data.
const sal_uInt16 EXC_ID_OBJ_FTSBSFMLA       = 0x000E;   /// Scroll bar/list box/combo box cell link.
const sal_uInt16 EXC_ID_OBJ_FTLBSDATA       = 0x0013;   /// List box/combo box data.
const sal_uInt16 EXC_ID_OBJ_FTCBLSFMLA      = 0x0014;   /// Check box/radio button cell link.
const sal_uInt16 EXC_ID_OBJ_FTCMO           = 0x0015;   /// Common object settings.
const sal_uInt16 EXC_ID_OBJ_FTUNKNOWN       = 0xFFFF;   /// For internal use only.

// ftCmo: object types
const sal_uInt16 EXC_OBJ_CMO_GROUP          = 0x0000;
const sal_uInt16 EXC_OBJ_CMO_LINE           = 0x0001;
const sal_uInt16 EXC_OBJ_CMO_RECTANGLE      = 0x0002;
const sal_uInt16 EXC_OBJ_CMO_ELLIPSE        = 0x0003;
const sal_uInt16 EXC_OBJ_CMO_ARC            = 0x0004;
const sal_uInt16 EXC_OBJ_CMO_CHART          = 0x0005;
const sal_uInt16 EXC_OBJ_CMO_TEXT           = 0x0006;
const sal_uInt16 EXC_OBJ_CMO_BUTTON         = 0x0007;
const sal_uInt16 EXC_OBJ_CMO_PICTURE        = 0x0008;
const sal_uInt16 EXC_OBJ_CMO_POLYGON        = 0x0009;
const sal_uInt16 EXC_OBJ_CMO_CHECKBOX       = 0x000B;
const sal_uInt16 EXC_OBJ_CMO_OPTIONBUTTON   = 0x000C;
const sal_uInt16 EXC_OBJ_CMO_EDIT           = 0x000D;
const sal_uInt16 EXC_OBJ_CMO_LABEL          = 0x000E;
const sal_uInt16 EXC_OBJ_CMO_DIALOG         = 0x000F;
const sal_uInt16 EXC_OBJ_CMO_SPIN           = 0x0010;
const sal_uInt16 EXC_OBJ_CMO_SCROLLBAR      = 0x0011;
const sal_uInt16 EXC_OBJ_CMO_LISTBOX        = 0x0012;
const sal_uInt16 EXC_OBJ_CMO_GROUPBOX       = 0x0013;
const sal_uInt16 EXC_OBJ_CMO_COMBOBOX       = 0x0014;
const sal_uInt16 EXC_OBJ_CMO_NOTE           = 0x0019;
const sal_uInt16 EXC_OBJ_CMO_DRAWING        = 0x001E;
const sal_uInt16 EXC_OBJ_CMO_UNKNOWN        = 0xFFFF;   /// For internal use only.

// ftPioGrbit: flags
const sal_uInt16 EXC_OBJ_PIO_LINKED         = 0x0002;
const sal_uInt16 EXC_OBJ_PIO_SYMBOL         = 0x0008;

// ftCbls: Check box/radio button data
const sal_uInt16 EXC_OBJ_CBLS_STATEMASK     = 0x0003;
const sal_uInt16 EXC_OBJ_CBLS_STATE_UNCHECK = 0x0000;
const sal_uInt16 EXC_OBJ_CBLS_STATE_CHECK   = 0x0001;
const sal_uInt16 EXC_OBJ_CBLS_STATE_TRI     = 0x0002;
const sal_uInt16 EXC_OBJ_CBLS_3D            = 0x0001;

// ftLbsData: List box data
const sal_uInt16 EXC_OBJ_LBS_SELMASK        = 0x0030;
const sal_uInt16 EXC_OBJ_LBS_SEL_SIMPLE     = 0x0000;   /// Simple selection.
const sal_uInt16 EXC_OBJ_LBS_SEL_MULTI      = 0x0010;   /// Multi selection.
const sal_uInt16 EXC_OBJ_LBS_SEL_EXT        = 0x0020;   /// Extended selection.
const sal_uInt16 EXC_OBJ_LBS_3D             = 0x0008;

/** Value binding mode for cells linked to form controls. */
enum XclCtrlBindMode
{
    xlBindContent,      /// Binds cell to content of control.
    xlBindPosition      /// Binds cell to position in control (i.e. listbox selection index).
};


// (0x01B6) TXO ===============================================================

const sal_uInt16 EXC_ID_TXO                 = 0x01B6;

/** Horizontal alignment flags. */
enum XclTxoHorAlign
{
    xlTxoHAlignLeft                         = 0x01,
    xlTxoHAlignCenter                       = 0x02,
    xlTxoHAlignRight                        = 0x03,
    xlTxoHAlignJustify                      = 0x04,
    xlTxoHAlign_Default                     = xlTxoHAlignLeft
};

/** Vertical alignment flags. */
enum XclTxoVerAlign
{
    xlTxoVAlignTop                          = 0x01,
    xlTxoVAlignCenter                       = 0x02,
    xlTxoVAlignBottom                       = 0x03,
    xlTxoVAlignJustify                      = 0x04,
    xlTxoVAlign_Default                     = xlTxoVAlignTop
};

/** Rotation. */
enum XclTxoRotation
{
    xlTxoNoRot                              = 0x0000,       /// Not rotated.
    xlTxoRotStacked                         = 0x0001,       /// characters stacked.
    xlTxoRot90ccw                           = 0x0002,       /// 90 degr. counterclockwise.
    xlTxoRot90cw                            = 0x0003,       /// 90 degr. clockwise.
    xlTxoRot_Default                        = xlTxoNoRot
};


// ============================================================================

#endif

