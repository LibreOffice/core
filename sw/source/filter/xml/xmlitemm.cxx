/*************************************************************************
 *
 *  $RCSfile: xmlitemm.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-07 14:05:53 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include <hintids.hxx>

#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLITMAP_HXX
#include <xmloff/xmlitmap.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

#ifndef _UNOMID_H
#include <unomid.h>
#endif

#define M_E( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, m }
#define M_E_SI( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, MID_FLAG_SPECIAL_ITEM_IMPORT|m }
#define M_E_SE( p, l, w, m ) \
    { XML_NAMESPACE_##p, sXML_##l, w, MID_FLAG_SPECIAL_ITEM_EXPORT|m }


SvXMLItemMapEntry aXMLTableItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E_SE( STYLE,  width,          RES_FRM_SIZE,   MID_FRMSIZE_WIDTH ),
    M_E_SE( STYLE,  rel_width,      RES_FRM_SIZE,   MID_FRMSIZE_REL_WIDTH ),
    // RES_PAPER_BIN
    // not required
    // TODO: RES_LR_SPACE
    M_E_SE( FO, margin_left,        RES_LR_SPACE,   MID_L_MARGIN ),
    M_E_SE( FO, margin_right,       RES_LR_SPACE,   MID_R_MARGIN ),
    // RES_UL_SPACE
    M_E( FO,    margin_top,         RES_UL_SPACE,   MID_UP_MARGIN ),
    M_E( FO,    margin_bottom,      RES_UL_SPACE,   MID_LO_MARGIN ),
    // RES_PAGEDESC
    M_E( STYLE, page_number,        RES_PAGEDESC,   MID_PAGEDESC_PAGENUMOFFSET),
    // RES_BREAK
    M_E( FO,    break_before,       RES_BREAK,  MID_BREAK_BEFORE ),
    M_E( FO,    break_after,        RES_BREAK,  MID_BREAK_AFTER ),
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    // not required
    // RES_HORI_ORIENT
    M_E( TABLE, align,                  RES_HORI_ORIENT, 0 ),
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_SHADOW
    M_E( STYLE, shadow,             RES_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    M_E( FO, keep_with_next,        RES_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    M_E( STYLE, may_break_between_rows, RES_LAYOUT_SPLIT, 0 ),
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableColItemMap[] =
{
    M_E_SI( STYLE,  column_width,     RES_FRM_SIZE, MID_FRMSIZE_COL_WIDTH ),
    M_E( STYLE,     rel_column_width, RES_FRM_SIZE, MID_FRMSIZE_REL_COL_WIDTH ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableRowItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E( STYLE, row_height,         RES_FRM_SIZE,   MID_FRMSIZE_FIX_HEIGHT ),
    M_E( STYLE, min_row_height,     RES_FRM_SIZE,   MID_FRMSIZE_MIN_HEIGHT ),
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE
    // not required
    // RES_UL_SPACE
    // not required
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    // not required
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_ANCHOR
    // not required
    // RES_SHADOW
    // not required
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    // not required
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};

SvXMLItemMapEntry aXMLTableCellItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    // not required
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE
    // not required
    // RES_UL_SPACE
    // not required
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    // not required
    // RES_OPAQUE
    // not required
    // RES_PROTECT
    // not required
    // RES_SURROUND
    // not required
    // RES_VERT_ORIENT
    M_E( FO,    vertical_align,                 RES_VERT_ORIENT, 0 ),
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,    background_color,       RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE, background_image,       RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    M_E( STYLE, border_line_width,        RES_BOX, ALL_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_left,   RES_BOX, LEFT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_right,  RES_BOX, RIGHT_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_top,    RES_BOX, TOP_BORDER_LINE_WIDTH ),
    M_E( STYLE, border_line_width_bottom, RES_BOX, BOTTOM_BORDER_LINE_WIDTH ),
    M_E( FO,    padding,                  RES_BOX, ALL_BORDER_PADDING ),
    M_E( FO,    padding_left,             RES_BOX, LEFT_BORDER_PADDING ),
    M_E( FO,    padding_right,            RES_BOX, RIGHT_BORDER_PADDING ),
    M_E( FO,    padding_top,              RES_BOX, TOP_BORDER_PADDING ),
    M_E( FO,    padding_bottom,           RES_BOX, BOTTOM_BORDER_PADDING ),
    M_E( FO,    border,                   RES_BOX, ALL_BORDER ),
    M_E( FO,    border_left,              RES_BOX, LEFT_BORDER ),
    M_E( FO,    border_right,             RES_BOX, RIGHT_BORDER ),
    M_E( FO,    border_top,               RES_BOX, TOP_BORDER ),
    M_E( FO,    border_bottom,            RES_BOX, BOTTOM_BORDER ),
    // RES_SHADOW
    // not required
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    // not required
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    {0,0,0,0 }
};
