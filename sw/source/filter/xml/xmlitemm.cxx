/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <editeng/memberids.h>
#include <editeng/xmlcnitm.hxx>
#include <hintids.hxx>
#include <svx/unomid.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "xmlitmap.hxx"
#include <xmloff/xmltoken.hxx>

#include <unomid.h>
#include <fmtfsize.hxx>

using namespace ::xmloff::token;

#define MAP_ENTRY( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, static_cast<sal_uInt16>(w), m }
#define M_E_SI( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, static_cast<sal_uInt16>(w), MID_SW_FLAG_SPECIAL_ITEM_IMPORT|m }
#define M_E_SE( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, static_cast<sal_uInt16>(w), MID_SW_FLAG_SPECIAL_ITEM_EXPORT|m }
#define M_E_SIE( p, l, w, m ) \
    { XML_NAMESPACE_##p, XML_##l, static_cast<sal_uInt16>(w), MID_SW_FLAG_SPECIAL_ITEM_EXPORT|MID_SW_FLAG_SPECIAL_ITEM_IMPORT|m }

#define M_END { 0, XML_TOKEN_INVALID, 0, 0 }

SvXMLItemMapEntry const aXMLTableItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    M_E_SE( STYLE,  WIDTH,          RES_FRM_SIZE,   MID_FRMSIZE_WIDTH ),
    M_E_SE( STYLE,  REL_WIDTH,      RES_FRM_SIZE,   MID_FRMSIZE_REL_WIDTH ),
    // RES_PAPER_BIN
    // not required
    // TODO: RES_LR_SPACE
    M_E_SE( FO,     MARGIN,         0xFFFF/*invalid*/, MID_SW_FLAG_NO_ITEM_IMPORT),
    M_E_SIE( FO,    MARGIN_LEFT,        RES_LR_SPACE,   MID_L_MARGIN ),
    M_E_SIE( FO,    MARGIN_RIGHT,       RES_LR_SPACE,   MID_R_MARGIN ),
    // RES_UL_SPACE
    M_E_SI( FO,     MARGIN_TOP,         RES_UL_SPACE,   MID_UP_MARGIN ),
    M_E_SI( FO,     MARGIN_BOTTOM,      RES_UL_SPACE,   MID_LO_MARGIN ),
    // RES_PAGEDESC
    MAP_ENTRY( STYLE,    PAGE_NUMBER,        RES_PAGEDESC,   MID_PAGEDESC_PAGENUMOFFSET),
    // RES_BREAK
    MAP_ENTRY( FO,   BREAK_BEFORE,       RES_BREAK,  MID_BREAK_BEFORE ),
    MAP_ENTRY( FO,   BREAK_AFTER,        RES_BREAK,  MID_BREAK_AFTER ),
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
    MAP_ENTRY( TABLE,    ALIGN,                  RES_HORI_ORIENT, 0 ),
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    MAP_ENTRY( FO,   BACKGROUND_COLOR,       RES_BACKGROUND, MID_BACK_COLOR ),
    MAP_ENTRY( STYLE,    BACKGROUND_IMAGE,       RES_BACKGROUND, MID_SW_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    // not required
    // RES_SHADOW
    MAP_ENTRY( STYLE,    SHADOW,             RES_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    MAP_ENTRY( FO, KEEP_WITH_NEXT,       RES_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    MAP_ENTRY( STYLE,    MAY_BREAK_BETWEEN_ROWS, RES_LAYOUT_SPLIT, 0 ),
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
    MAP_ENTRY( STYLE, WRITING_MODE, RES_FRAMEDIR, 0 ),

    // RES_COLLAPSING_BORDERS
    MAP_ENTRY( TABLE, BORDER_MODEL, RES_COLLAPSING_BORDERS, 0 ),

    M_END
};

SvXMLItemMapEntry const aXMLTableColItemMap[] =
{
    M_E_SI( STYLE,  COLUMN_WIDTH,     RES_FRM_SIZE, MID_FRMSIZE_COL_WIDTH ),
    MAP_ENTRY( STYLE,        REL_COLUMN_WIDTH, RES_FRM_SIZE, MID_FRMSIZE_REL_COL_WIDTH ),
    M_END
};

SvXMLItemMapEntry const aXMLTableRowItemMap[] =
{
    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    MAP_ENTRY( STYLE,    ROW_HEIGHT,         RES_FRM_SIZE,   MID_FRMSIZE_FIX_HEIGHT ),
    MAP_ENTRY( STYLE,    MIN_ROW_HEIGHT,     RES_FRM_SIZE,   MID_FRMSIZE_MIN_HEIGHT ),
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
    MAP_ENTRY( FO,   BACKGROUND_COLOR,       RES_BACKGROUND, MID_BACK_COLOR ),
    MAP_ENTRY( STYLE,    BACKGROUND_IMAGE,       RES_BACKGROUND, MID_SW_FLAG_ELEMENT_ITEM ),
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
    M_E_SE( STYLE, KEEP_TOGETHER, RES_ROW_SPLIT, 0 ),
    M_E_SE( FO, KEEP_TOGETHER, RES_ROW_SPLIT, 0 ),
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

SvXMLItemMapEntry const aXMLTableCellItemMap[] =
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
    MAP_ENTRY( STYLE,    VERTICAL_ALIGN,                 RES_VERT_ORIENT, 0 ),
    // RES_HORI_ORIENT
    // not required
    // RES_ANCHOR
    // not required
    // RES_BACKGROUND
    MAP_ENTRY( FO,   BACKGROUND_COLOR,       RES_BACKGROUND, MID_BACK_COLOR ),
    MAP_ENTRY( STYLE,    BACKGROUND_IMAGE,       RES_BACKGROUND, MID_SW_FLAG_ELEMENT_ITEM ),
    // RES_BOX
    MAP_ENTRY( STYLE,    BORDER_LINE_WIDTH,        RES_BOX, ALL_BORDER_LINE_WIDTH ),
    MAP_ENTRY( STYLE,    BORDER_LINE_WIDTH_LEFT,   RES_BOX, LEFT_BORDER_LINE_WIDTH ),
    MAP_ENTRY( STYLE,    BORDER_LINE_WIDTH_RIGHT,  RES_BOX, RIGHT_BORDER_LINE_WIDTH ),
    MAP_ENTRY( STYLE,    BORDER_LINE_WIDTH_TOP,    RES_BOX, TOP_BORDER_LINE_WIDTH ),
    MAP_ENTRY( STYLE,    BORDER_LINE_WIDTH_BOTTOM, RES_BOX, BOTTOM_BORDER_LINE_WIDTH ),
    MAP_ENTRY( FO,   PADDING,                  RES_BOX, ALL_BORDER_PADDING ),
    MAP_ENTRY( FO,   PADDING_LEFT,             RES_BOX, LEFT_BORDER_PADDING ),
    MAP_ENTRY( FO,   PADDING_RIGHT,            RES_BOX, RIGHT_BORDER_PADDING ),
    MAP_ENTRY( FO,   PADDING_TOP,              RES_BOX, TOP_BORDER_PADDING ),
    MAP_ENTRY( FO,   PADDING_BOTTOM,           RES_BOX, BOTTOM_BORDER_PADDING ),
    MAP_ENTRY( FO,   BORDER,                   RES_BOX, ALL_BORDER ),
    MAP_ENTRY( FO,   BORDER_LEFT,              RES_BOX, LEFT_BORDER ),
    MAP_ENTRY( FO,   BORDER_RIGHT,             RES_BOX, RIGHT_BORDER ),
    MAP_ENTRY( FO,   BORDER_TOP,               RES_BOX, TOP_BORDER ),
    MAP_ENTRY( FO,   BORDER_BOTTOM,            RES_BOX, BOTTOM_BORDER ),
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

    // RES_FRAMEDIR
    MAP_ENTRY( STYLE, WRITING_MODE, RES_FRAMEDIR, 0 ),
    MAP_ENTRY( LO_EXT, WRITING_MODE, RES_FRAMEDIR, 0 ),

    M_END
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
