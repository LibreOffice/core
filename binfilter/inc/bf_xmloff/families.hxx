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
#ifndef _XMLOFF_FAMILIES_HXX_
#define _XMLOFF_FAMILIES_HXX_
namespace binfilter {

/** These defines determine the unique ids for XML style-families
    used in the SvXMLAutoStylePoolP.
 */

// Misc (Pool)
// reserved: 0..99
#define XML_STYLE_FAMILY_DATA_STYLE				0
#define XML_STYLE_FAMILY_DATA_STYLE_NAME		"data-style"
#define XML_STYLE_FAMILY_PAGE_MASTER			1
#define XML_STYLE_FAMILY_PAGE_MASTER_NAME		"page-master"
#define XML_STYLE_FAMILY_PAGE_MASTER_PREFIX		"pm"
#define XML_STYLE_FAMILY_MASTER_PAGE			2

// Text
// reserved: 100..199
#define XML_STYLE_FAMILY_TEXT_PARAGRAPH			100
#define XML_STYLE_FAMILY_TEXT_TEXT				101
#define XML_STYLE_FAMILY_TEXT_LIST				102
#define XML_STYLE_FAMILY_TEXT_OUTLINE			103
#define XML_STYLE_FAMILY_TEXT_FOOTNOTECONFIG	105
#define XML_STYLE_FAMILY_TEXT_ENDNOTECONFIG		106
#define XML_STYLE_FAMILY_TEXT_SECTION			107
#define XML_STYLE_FAMILY_TEXT_FRAME				108	// export only
#define XML_STYLE_FAMILY_TEXT_RUBY				109
#define XML_STYLE_FAMILY_TEXT_BIBLIOGRAPHYCONFIG 110
#define XML_STYLE_FAMILY_TEXT_LINENUMBERINGCONFIG 111

// Table
// reserved: 200..299
#define XML_STYLE_FAMILY_TABLE_TABLE			200
#define XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME	"table"
#define XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX	"ta"
#define XML_STYLE_FAMILY_TABLE_COLUMN			202
#define XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME	"table-column"
#define XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX	"co"
#define XML_STYLE_FAMILY_TABLE_ROW				203
#define XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME	"table-row"
#define XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX	"ro"
#define XML_STYLE_FAMILY_TABLE_CELL				204
#define XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME	"table-cell"
#define XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX	"ce"

// Impress/Draw
// reserved: 300..399
#define XML_STYLE_FAMILY_SD_GRAPHICS_ID			300
#define XML_STYLE_FAMILY_SD_GRAPHICS_NAME		"graphics"
#define XML_STYLE_FAMILY_SD_GRAPHICS_PREFIX		"gr"

#define XML_STYLE_FAMILY_SD_PRESENTATION_ID		301
#define XML_STYLE_FAMILY_SD_PRESENTATION_NAME	"presentation"
#define XML_STYLE_FAMILY_SD_PRESENTATION_PREFIX	"pr"
// families for derived from SvXMLStyleContext
#define XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID			302
#define XML_STYLE_FAMILY_SD_PAGEMASTERSTYLECONEXT_ID	306
#define XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID	303
// family for draw pool
#define XML_STYLE_FAMILY_SD_POOL_ID				304
#define XML_STYLE_FAMILY_SD_POOL_NAME			"default"
// family for presentation drawpage properties
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID		305
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME	"drawing-page"
#define XML_STYLE_FAMILY_SD_DRAWINGPAGE_PREFIX	"dp"

// Chart
// reserved: 400..499
#define XML_STYLE_FAMILY_SCH_CHART_ID			400
#define XML_STYLE_FAMILY_SCH_CHART_NAME			"chart"
#define XML_STYLE_FAMILY_SCH_CHART_PREFIX		"ch"

// Math
// reserved: 500..599

// Forms/Controls
// reserved 600..649
#define XML_STYLE_FAMILY_CONTROL_ID				600
#define XML_STYLE_FAMILY_CONTROL_NAME			"control"
#define XML_STYLE_FAMILY_CONTROL_PREFIX			"ctrl"

}//end of namespace binfilter
#endif	// _XMLOFF_FAMILIES_HXX_
