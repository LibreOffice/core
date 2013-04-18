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
#ifndef _XMLOFF_FAMILIES_HXX_
#define _XMLOFF_FAMILIES_HXX_

/** These defines determine the unique ids for XML style-families
    used in the SvXMLAutoStylePoolP.
 */

// Misc (Pool)
// reserved: 0..99
#define XML_STYLE_FAMILY_DATA_STYLE             0
#define XML_STYLE_FAMILY_DATA_STYLE_NAME        "data-style"
#define XML_STYLE_FAMILY_PAGE_MASTER            1
#define XML_STYLE_FAMILY_PAGE_MASTER_NAME       "page-layout"
#define XML_STYLE_FAMILY_PAGE_MASTER_PREFIX     "pm"
#define XML_STYLE_FAMILY_MASTER_PAGE            2

// Text
// reserved: 100..199
#define XML_STYLE_FAMILY_TEXT_PARAGRAPH         100
#define XML_STYLE_FAMILY_TEXT_TEXT              101
#define XML_STYLE_FAMILY_TEXT_LIST              102
#define XML_STYLE_FAMILY_TEXT_OUTLINE           103
#define XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG    105
#define XML_STYLE_FAMILY_TEXT_ENDNOTECONFIG     106
#define XML_STYLE_FAMILY_TEXT_SECTION           107
#define XML_STYLE_FAMILY_TEXT_FRAME             108 // export only
#define XML_STYLE_FAMILY_TEXT_RUBY              109
#define XML_STYLE_FAMILY_TEXT_BIBLIOGRAPHYCONFIG 110
#define XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG 111

// Table
// reserved: 200..299
#define XML_STYLE_FAMILY_TABLE_TABLE            200
#define XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME    "table"
#define XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX  "ta"
#define XML_STYLE_FAMILY_TABLE_COLUMN           202
#define XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME   "table-column"
#define XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX "co"
#define XML_STYLE_FAMILY_TABLE_ROW              203
#define XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME  "table-row"
#define XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX    "ro"
#define XML_STYLE_FAMILY_TABLE_CELL             204
#define XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME "table-cell"
#define XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX   "ce"
#define XML_STYLE_FAMILY_TABLE_TEMPLATE_ID      205

// Impress/Draw
// reserved: 300..399
#define XML_STYLE_FAMILY_SD_GRAPHICS_ID         300
#define XML_STYLE_FAMILY_SD_GRAPHICS_NAME       "graphic"
#define XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX     "gr"

#define XML_STYLE_FAMILY_SD_PRESENTATION_ID     301
#define XML_STYLE_FAMILY_SD_PRESENTATION_NAME   "presentation"
#define XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX "pr"
// families for derived from SvXMLStyleContext
#define XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID         302
#define XML_STYLE_FAMILY_SD_PAGEMASTERSTYLECONEXT_ID    306
#define XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID   303
// family for draw pool
#define XML_STYLE_FAMILY_SD_POOL_ID             304
#define XML_STYLE_FAMILY_SD_POOL_NAME           "default"
// family for presentation drawpage properties
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID      305
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME    "drawing-page"
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_PREFIX  "dp"

#define XML_STYLE_FAMILY_SD_GRADIENT_ID     306
#define XML_STYLE_FAMILY_SD_HATCH_ID        307
#define XML_STYLE_FAMILY_SD_FILL_IMAGE_ID       308
#define XML_STYLE_FAMILY_SD_MARKER_ID       309
#define XML_STYLE_FAMILY_SD_STROKE_DASH_ID      310

// Chart
// reserved: 400..499
#define XML_STYLE_FAMILY_SCH_CHART_ID           400
#define XML_STYLE_FAMILY_SCH_CHART_NAME         "chart"
#define XML_STYLE_FAMILY_SCH_CHART_PREFIX       "ch"

// Math
// reserved: 500..599


// Forms/Controls
// reserved 600..649
#define XML_STYLE_FAMILY_CONTROL_ID             600
#define XML_STYLE_FAMILY_CONTROL_NAME           "control"
#define XML_STYLE_FAMILY_CONTROL_PREFIX         "ctrl"

#endif  // _XMLOFF_FAMILIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
