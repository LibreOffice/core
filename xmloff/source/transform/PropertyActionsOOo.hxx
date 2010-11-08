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

#ifndef _XMLOFF_PROPERTYACTIONSOOO_HXX
#define _XMLOFF_PROPERTYACTIONSOOO_HXX

#include "TransformerAction.hxx"
#include "TransformerActionInit.hxx"
#include "AttrTransformerAction.hxx"

enum XMLPropOOOTransformerAction
{
    XML_PTACTION_LINE_MODE=XML_ATACTION_USER_DEFINED,
    XML_PTACTION_UNDERLINE,
    XML_PTACTION_LINETHROUGH,
    XML_PTACTION_KEEP_WITH_NEXT,
    XML_PTACTION_SPLINES,
    XML_ATACTION_INCH2IN_DUPLICATE,
    XML_PTACTION_INTERVAL_MAJOR,
    XML_PTACTION_INTERVAL_MINOR,
    XML_ATACTION_COPY_DUPLICATE,
    XML_PTACTION_SYMBOL,
    XML_PTACTION_SYMBOL_IMAGE_NAME,
    XML_PTACTION_TRANSPARENCY,
    XML_PTACTION_BREAK_INSIDE,
    XML_ATACTION_CAPTION_ESCAPE_OOO,
    XML_ATACTION_MOVE_PROTECT,
    XML_ATACTION_SIZE_PROTECT,
    XML_ATACTION_PROTECT,
    // No image transparancy info in lable document (#i50322#)
    XML_ATACTION_WRITER_BACK_GRAPHIC_TRANSPARENCY,
    XML_PTACTION_END=XML_TACTION_END
};

extern XMLTransformerActionInit aGraphicPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aGraphicPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aDrawingPagePropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aPageLayoutPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aHeaderFooterPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTextPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aParagraphPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aSectionPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTablePropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableColumnPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableRowPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aTableCellPropertyOOoElemActionTable[];
extern XMLTransformerActionInit aListLevelPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOOoAttrActionTable[];
extern XMLTransformerActionInit aChartPropertyOOoElemActionTable[];

#endif  //  _XMLOFF_PROPERTYACTIONSOOO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
