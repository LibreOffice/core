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

#ifndef INCLUDED_XMLOFF_SOURCE_TRANSFORM_PROPERTYACTIONSOOO_HXX
#define INCLUDED_XMLOFF_SOURCE_TRANSFORM_PROPERTYACTIONSOOO_HXX

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
    // No image transparency info in label document (#i50322#)
    XML_ATACTION_WRITER_BACK_GRAPHIC_TRANSPARENCY
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

#endif // INCLUDED_XMLOFF_SOURCE_TRANSFORM_PROPERTYACTIONSOOO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
