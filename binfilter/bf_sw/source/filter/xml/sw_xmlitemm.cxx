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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#ifndef _SVX_UNOMID_HXX
#include <bf_svx/unomid.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

#ifndef _UNOMID_H
#include <unomid.h>
#endif
namespace binfilter {

using namespace ::binfilter::xmloff::token;

#define M_E( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, w, m }
#define M_E_SI( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, w, MID_FLAG_SPECIAL_ITEM_IMPORT|m }
#define M_E_SE( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, w, MID_FLAG_SPECIAL_ITEM_EXPORT|m }
                                                
#define M_END { 0, XML_TOKEN_INVALID, 0, 0 }

SvXMLItemMapEntry aXMLTableItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E_SE( STYLE,	WIDTH,			RES_FRM_SIZE,	MID_FRMSIZE_WIDTH ),
    M_E_SE( STYLE,	REL_WIDTH,		RES_FRM_SIZE,	MID_FRMSIZE_REL_WIDTH ),
    // RES_PAPER_BIN
    // not required
    // TODO: RES_LR_SPACE
    M_E_SE( FO,	MARGIN_LEFT,		RES_LR_SPACE,	MID_L_MARGIN ),
    M_E_SE( FO,	MARGIN_RIGHT,		RES_LR_SPACE,	MID_R_MARGIN ),
    // RES_UL_SPACE
    M_E( FO,	MARGIN_TOP,			RES_UL_SPACE,	MID_UP_MARGIN ),
    M_E( FO,	MARGIN_BOTTOM,		RES_UL_SPACE,	MID_LO_MARGIN ),
    // RES_PAGEDESC
    M_E( STYLE,	PAGE_NUMBER,		RES_PAGEDESC,	MID_PAGEDESC_PAGENUMOFFSET),
    // RES_BREAK
    M_E( FO,	BREAK_BEFORE,		RES_BREAK,	MID_BREAK_BEFORE ),
    M_E( FO,	BREAK_AFTER,		RES_BREAK,	MID_BREAK_AFTER ),
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
    M_E( TABLE,	ALIGN,					RES_HORI_ORIENT, 0 ),
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,	BACKGROUND_COLOR,		RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE,	BACKGROUND_IMAGE,		RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_SHADOW
    M_E( STYLE,	SHADOW,				RES_SHADOW,	0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    M_E( FO, KEEP_WITH_NEXT,		RES_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    M_E( STYLE,	MAY_BREAK_BETWEEN_ROWS,	RES_LAYOUT_SPLIT, 0 ),
    // RES_CHAIN
    // not required
    // RES_LINENUMBER	
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_UNKNOWNATR_CONTAINER
    M_E_SE( TEXT, XMLNS, RES_UNKNOWNATR_CONTAINER, 0 ),

    // RES_FRAMEDIR
    M_E( STYLE, WRITING_MODE, RES_FRAMEDIR, 0 ),
    M_END
};

SvXMLItemMapEntry aXMLTableColItemMap[] =
{
    M_E_SI( STYLE,	COLUMN_WIDTH,	  RES_FRM_SIZE,	MID_FRMSIZE_COL_WIDTH ),
    M_E( STYLE,		REL_COLUMN_WIDTH, RES_FRM_SIZE,	MID_FRMSIZE_REL_COL_WIDTH ),
    M_END
};

SvXMLItemMapEntry aXMLTableRowItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E( STYLE,	ROW_HEIGHT,			RES_FRM_SIZE,	MID_FRMSIZE_FIX_HEIGHT ),
    M_E( STYLE,	MIN_ROW_HEIGHT,		RES_FRM_SIZE,	MID_FRMSIZE_MIN_HEIGHT ),
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
    M_E( FO,	BACKGROUND_COLOR,		RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE,	BACKGROUND_IMAGE,		RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
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
    M_E_SE( TEXT, XMLNS, RES_UNKNOWNATR_CONTAINER, 0 ),
    M_END
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
    M_E( FO,	VERTICAL_ALIGN,					RES_VERT_ORIENT, 0 ),
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    M_E( FO,	BACKGROUND_COLOR,		RES_BACKGROUND, MID_BACK_COLOR ),
    M_E( STYLE,	BACKGROUND_IMAGE,		RES_BACKGROUND, MID_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    M_E( STYLE,	BORDER_LINE_WIDTH,		  RES_BOX, ALL_BORDER_LINE_WIDTH ),
    M_E( STYLE,	BORDER_LINE_WIDTH_LEFT,	  RES_BOX, LEFT_BORDER_LINE_WIDTH ),
    M_E( STYLE,	BORDER_LINE_WIDTH_RIGHT,  RES_BOX, RIGHT_BORDER_LINE_WIDTH ),
    M_E( STYLE,	BORDER_LINE_WIDTH_TOP,	  RES_BOX, TOP_BORDER_LINE_WIDTH ),
    M_E( STYLE,	BORDER_LINE_WIDTH_BOTTOM, RES_BOX, BOTTOM_BORDER_LINE_WIDTH ),
    M_E( FO, 	PADDING,				  RES_BOX, ALL_BORDER_PADDING ),
    M_E( FO, 	PADDING_LEFT,			  RES_BOX, LEFT_BORDER_PADDING ),
    M_E( FO, 	PADDING_RIGHT,			  RES_BOX, RIGHT_BORDER_PADDING ),
    M_E( FO, 	PADDING_TOP,			  RES_BOX, TOP_BORDER_PADDING ),
    M_E( FO, 	PADDING_BOTTOM,			  RES_BOX, BOTTOM_BORDER_PADDING ),
    M_E( FO,	BORDER,					  RES_BOX, ALL_BORDER ),
    M_E( FO,	BORDER_LEFT,			  RES_BOX, LEFT_BORDER ),
    M_E( FO,	BORDER_RIGHT,			  RES_BOX, RIGHT_BORDER ),
    M_E( FO,	BORDER_TOP,				  RES_BOX, TOP_BORDER ),
    M_E( FO,	BORDER_BOTTOM,			  RES_BOX, BOTTOM_BORDER ),
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
    M_E_SE( TEXT, XMLNS, RES_UNKNOWNATR_CONTAINER, 0 ),
    M_END
};
}
