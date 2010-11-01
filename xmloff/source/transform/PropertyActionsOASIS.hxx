/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_PROPERTYACTIONSOASIS_HXX
#define _XMLOFF_PROPERTYACTIONSOASIS_HXX

#include "TransformerAction.hxx"
#include "TransformerActionInit.hxx"
#include "AttrTransformerAction.hxx"

enum XMLPropOASISTransformerAction
{
    XML_OPTACTION_LINE_MODE=XML_ATACTION_USER_DEFINED,
    XML_OPTACTION_UNDERLINE_TYPE,
    XML_OPTACTION_UNDERLINE_STYLE,
    XML_OPTACTION_UNDERLINE_WIDTH,
    XML_OPTACTION_LINETHROUGH_TYPE,
    XML_OPTACTION_LINETHROUGH_STYLE,
    XML_OPTACTION_LINETHROUGH_WIDTH,
    XML_OPTACTION_LINETHROUGH_TEXT,
    XML_OPTACTION_KEEP_WITH_NEXT,
    XML_OPTACTION_INTERPOLATION,
    XML_OPTACTION_INTERVAL_MAJOR,
    XML_OPTACTION_INTERVAL_MINOR_DIVISOR,
    XML_OPTACTION_SYMBOL_TYPE,
    XML_OPTACTION_SYMBOL_NAME,
    XML_OPTACTION_OPACITY,
    XML_OPTACTION_IMAGE_OPACITY,
    XML_OPTACTION_KEEP_TOGETHER,
    XML_OPTACTION_CONTROL_TEXT_ALIGN,
    XML_OPTACTION_DRAW_WRITING_MODE,
    XML_ATACTION_CAPTION_ESCAPE_OASIS,
    XML_ATACTION_DECODE_PROTECT,
    XML_OPTACTION_END=XML_ATACTION_END
};

extern XMLTransformerActionInit aGraphicPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aDrawingPagePropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aPageLayoutPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aHeaderFooterPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aSectionPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTablePropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableColumnPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableRowPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aListLevelPropertyOASISAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOASISAttrActionTable[];

#endif  //  _XMLOFF_PROPERTYACTIONSOASIS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
