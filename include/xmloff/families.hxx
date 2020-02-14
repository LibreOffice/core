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
#ifndef INCLUDED_XMLOFF_FAMILIES_HXX
#define INCLUDED_XMLOFF_FAMILIES_HXX

/** These defines determine the unique ids for XML style-families
    used in the SvXMLAutoStylePoolP.
 */

#define XmlStyleFamily::PAGE_MASTER_NAME       "page-layout"
#define XmlStyleFamily::PAGE_MASTER_PREFIX     "pm"
#define XmlStyleFamily::TABLE_TABLE_STYLES_NAME    "table"
#define XmlStyleFamily::TABLE_TABLE_STYLES_PREFIX  "ta"
#define XmlStyleFamily::TABLE_COLUMN_STYLES_NAME   "table-column"
#define XmlStyleFamily::TABLE_COLUMN_STYLES_PREFIX "co"
#define XmlStyleFamily::TABLE_ROW_STYLES_NAME  "table-row"
#define XmlStyleFamily::TABLE_ROW_STYLES_PREFIX    "ro"
#define XmlStyleFamily::TABLE_CELL_STYLES_NAME "table-cell"
#define XmlStyleFamily::TABLE_CELL_STYLES_PREFIX   "ce"
#define XmlStyleFamily::SD_GRAPHICS_NAME       "graphic"
#define XmlStyleFamily::SD_GRAPHICS_PREFIX     "gr"
#define XmlStyleFamily::SD_PRESENTATION_NAME   "presentation"
#define XmlStyleFamily::SD_PRESENTATION_PREFIX "pr"
#define XmlStyleFamily::SD_POOL_NAME           "default"
#define XmlStyleFamily::SD_DRAWINGPAGE_NAME    "drawing-page"
#define XmlStyleFamily::SD_DRAWINGPAGE_PREFIX  "dp"
#define XmlStyleFamily::SCH_CHART_NAME         "chart"
#define XmlStyleFamily::SCH_CHART_PREFIX       "ch"
#define XmlStyleFamily::CONTROL_PREFIX         "ctrl"

enum class XmlStyleFamily
{
// Misc (Pool)
// reserved: 0..99
#define XmlStyleFamily::DATA_STYLE             0
#define XmlStyleFamily::PAGE_MASTER            1
#define XmlStyleFamily::MASTER_PAGE            2

// Text
// reserved: 100..199
#define XmlStyleFamily::TEXT_PARAGRAPH         100
#define XmlStyleFamily::TEXT_TEXT              101
#define XmlStyleFamily::TEXT_LIST              102
#define XmlStyleFamily::TEXT_OUTLINE           103
#define XmlStyleFamily::TEXT_FOOTNOTECONFIG    105
#define XmlStyleFamily::TEXT_ENDNOTECONFIG     106
#define XmlStyleFamily::TEXT_SECTION           107
#define XmlStyleFamily::TEXT_FRAME             108 // export only
#define XmlStyleFamily::TEXT_RUBY              109
#define XmlStyleFamily::TEXT_BIBLIOGRAPHYCONFIG 110
#define XmlStyleFamily::TEXT_LINENUMBERINGCONFIG 111

// Table
// reserved: 200..299
#define XmlStyleFamily::TABLE_TABLE            200
#define XmlStyleFamily::TABLE_COLUMN           202
#define XmlStyleFamily::TABLE_ROW              203
#define XmlStyleFamily::TABLE_CELL             204
#define XmlStyleFamily::TABLE_TEMPLATE_ID      205

// Impress/Draw
// reserved: 300..399
#define XmlStyleFamily::SD_GRAPHICS_ID         300

#define XmlStyleFamily::SD_PRESENTATION_ID     301
// families for derived from SvXMLStyleContext
#define XmlStyleFamily::SD_PAGEMASTERCONEXT_ID         302
#define XmlStyleFamily::SD_PAGEMASTERSTYLECONEXT_ID    306
#define XmlStyleFamily::SD_PRESENTATIONPAGELAYOUT_ID   303
// family for draw pool
#define XmlStyleFamily::SD_POOL_ID             304
// family for presentation drawpage properties
#define XmlStyleFamily::SD_DRAWINGPAGE_ID      305

#define XmlStyleFamily::SD_GRADIENT_ID     306
#define XmlStyleFamily::SD_HATCH_ID        307
#define XmlStyleFamily::SD_FILL_IMAGE_ID       308
#define XmlStyleFamily::SD_MARKER_ID       309
#define XmlStyleFamily::SD_STROKE_DASH_ID      310

// Chart
// reserved: 400..499
#define XmlStyleFamily::SCH_CHART_ID           400

// Math
// reserved: 500..599


// Forms/Controls
// reserved 600..649
#define XmlStyleFamily::CONTROL_ID             600

#endif // INCLUDED_XMLOFF_FAMILIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
