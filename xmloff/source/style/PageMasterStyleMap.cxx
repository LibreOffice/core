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

#include <PageMasterStyleMap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmlprop.hxx>

using namespace ::xmloff::token;

#define MAP(name,prefix,token,type,context,version)  { name, prefix, token, type, context, version, false }
#define DPMAP(name,prefix,token,type,context) MAP(name, prefix, token, type|XML_TYPE_PROP_DRAWING_PAGE, context, SvtSaveOptions::ODFSVER_013)
#define PLMAP(name,prefix,token,type,context) \
        MAP(name, prefix, token, type|XML_TYPE_PROP_PAGE_LAYOUT, context, SvtSaveOptions::ODFSVER_010)
#define PLMAP_12(name,prefix,token,type,context) \
        MAP(name, prefix, token, type|XML_TYPE_PROP_PAGE_LAYOUT, context, SvtSaveOptions::ODFSVER_012)
#define PLMAP_ODF13(name,prefix,token,type,context) \
        MAP(name, prefix, token, type|XML_TYPE_PROP_PAGE_LAYOUT, context, SvtSaveOptions::ODFSVER_013)
#define PLMAP_EXT(name,prefix,token,type,context) \
    MAP(name, prefix, token, type|XML_TYPE_PROP_PAGE_LAYOUT, context, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED)
#define HFMAP(name,prefix,token,type,context) \
        MAP(name, prefix, token, type|XML_TYPE_PROP_HEADER_FOOTER, context, SvtSaveOptions::ODFSVER_010)


const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
    //////////////////////////////////////////////////////////////////////////
    // Section for 'page-layout-properties'

    // page master
    PLMAP( PROP_PageStyleLayout,     XML_NAMESPACE_STYLE,    XML_PAGE_USAGE,                    XML_PM_TYPE_PAGESTYLELAYOUT | MID_FLAG_SPECIAL_ITEM,    CTF_PM_PAGEUSAGE ),
    PLMAP( PROP_Width,                 XML_NAMESPACE_FO,        XML_PAGE_WIDTH,                    XML_TYPE_MEASURE,                                        0 ),
    PLMAP( PROP_Height,                XML_NAMESPACE_FO,        XML_PAGE_HEIGHT,                XML_TYPE_MEASURE,                                        0 ),
    PLMAP( PROP_NumberingType,            XML_NAMESPACE_STYLE,    XML_NUM_FORMAT,                    XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY,        0 ),
    PLMAP( PROP_NumberingType,            XML_NAMESPACE_STYLE,    XML_NUM_LETTER_SYNC,            XML_PM_TYPE_NUMLETTERSYNC,                                0 ),
    PLMAP( PROP_PrinterPaperTray,        XML_NAMESPACE_STYLE,    XML_PAPER_TRAY_NAME,            XML_TYPE_STRING | MID_FLAG_PROPERTY_MAY_THROW,         0 ),
    PLMAP( PROP_IsLandscape,                XML_NAMESPACE_STYLE,    XML_PRINT_ORIENTATION,            XML_PM_TYPE_PRINTORIENTATION,                            0 ),
    PLMAP( PROP_TopMargin,                XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,                                        CTF_PM_MARGINALL ),
    PLMAP( PROP_TopMargin,                XML_NAMESPACE_FO,        XML_MARGIN_TOP,                    XML_TYPE_MEASURE,                                        CTF_PM_MARGINTOP ),
    PLMAP( PROP_BottomMargin,            XML_NAMESPACE_FO,        XML_MARGIN_BOTTOM,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINBOTTOM ),
    PLMAP( PROP_LeftMargin,                XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINLEFT ),
    PLMAP( PROP_RightMargin,                XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINRIGHT ),
    PLMAP( PROP_TopBorder,                XML_NAMESPACE_FO,        XML_BORDER,                     XML_TYPE_BORDER,                                        CTF_PM_BORDERALL ),
    PLMAP( PROP_TopBorder,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                    XML_TYPE_BORDER,                                        CTF_PM_BORDERTOP ),
    PLMAP( PROP_BottomBorder,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,                                        CTF_PM_BORDERBOTTOM ),
    PLMAP( PROP_LeftBorder,                XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERLEFT ),
    PLMAP( PROP_RightBorder,                XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERRIGHT ),
    PLMAP( PROP_TopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHALL ),
    PLMAP( PROP_TopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHTOP ),
    PLMAP( PROP_BottomBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHBOTTOM ),
    PLMAP( PROP_LeftBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHLEFT ),
    PLMAP( PROP_RightBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHRIGHT ),
    PLMAP( PROP_TopBorderDistance,        XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,                                        CTF_PM_PADDINGALL ),
    PLMAP( PROP_TopBorderDistance,        XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGTOP ),
    PLMAP( PROP_BottomBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,                                        CTF_PM_PADDINGBOTTOM ),
    PLMAP( PROP_LeftBorderDistance,        XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGLEFT ),
    PLMAP( PROP_RightBorderDistance,     XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGRIGHT ),
    PLMAP( PROP_ShadowFormat,            XML_NAMESPACE_STYLE,    XML_SHADOW,                        XML_TYPE_TEXT_SHADOW,                                    0 ),
    PLMAP( PROP_BackColor,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    0 ),
    PLMAP( PROP_BackTransparent,            XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        0 ),
    PLMAP( PROP_BackGraphicLocation,        XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_GRAPHICPOSITION ),
    PLMAP( PROP_BackGraphicFilter,        XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_GRAPHICFILTER ),
    PLMAP( PROP_BackGraphic,            XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_GRAPHICURL ),
    PLMAP( PROP_PrintAnnotations,        XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTANNOTATIONS | MID_FLAG_MULTI_PROPERTY,                            CTF_PM_PRINT_ANNOTATIONS ),
    PLMAP( PROP_PrintCharts,                XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTCHARTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_CHARTS ),
    PLMAP( PROP_PrintDrawing,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTDRAWING | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_DRAWING ),
    PLMAP( PROP_PrintFormulas,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTFORMULAS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_FORMULAS ),
    PLMAP( PROP_PrintGrid,                XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTGRID | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_PRINT_GRID ),
    PLMAP( PROP_PrintHeaders,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTHEADERS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_HEADERS ),
    PLMAP( PROP_PrintObjects,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTOBJECTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_OBJECTS ),
    PLMAP( PROP_PrintZeroValues,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTZEROVALUES | MID_FLAG_MERGE_ATTRIBUTE,                            CTF_PM_PRINT_ZEROVALUES ),
    PLMAP( PROP_PrintDownFirst,            XML_NAMESPACE_STYLE,    XML_PRINT_PAGE_ORDER,            XML_PM_TYPE_PRINTPAGEORDER,                                0 ),
    PLMAP( PROP_FirstPageNumber,            XML_NAMESPACE_STYLE,    XML_FIRST_PAGE_NUMBER,            XML_PM_TYPE_FIRSTPAGENUMBER,                            0 ),
    PLMAP( PROP_PageScale,                XML_NAMESPACE_STYLE,    XML_SCALE_TO,                    XML_TYPE_PERCENT16,                                        CTF_PM_SCALETO ),
    PLMAP( PROP_ScaleToPages,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_PAGES,             XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOPAGES ),
    // ODF 1.3 OFFICE-3857
    PLMAP_ODF13( PROP_ScaleToPagesX,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_X,                  XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOX ),
    PLMAP_ODF13( PROP_ScaleToPagesY,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_Y,                 XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOY ),
    PLMAP_ODF13( PROP_ScaleToPagesX,            XML_NAMESPACE_LO_EXT,    XML_SCALE_TO_X,                  XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOX ),
    PLMAP_ODF13( PROP_ScaleToPagesY,            XML_NAMESPACE_LO_EXT,    XML_SCALE_TO_Y,                 XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOY ),
    PLMAP( PROP_CenterHorizontally,        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_HORIZONTAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( PROP_CenterVertically,        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_VERTICAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( PROP_TextColumns,                XML_NAMESPACE_STYLE,    XML_COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_PM_TEXTCOLUMNS ),
    PLMAP( PROP_RegisterModeActive,        XML_NAMESPACE_STYLE,    XML_REGISTER_TRUE,    XML_TYPE_BOOL, 0 ),
    PLMAP( PROP_RegisterParagraphStyle,    XML_NAMESPACE_STYLE,    XML_REGISTER_TRUTH_REF_STYLE_NAME,    XML_TYPE_STYLENAME| MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_PM_REGISTER_STYLE ),
    PLMAP( PROP_WritingMode,                 XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_TYPE_TEXT_WRITING_MODE | MID_FLAG_MULTI_PROPERTY, CTF_PM_WRITINGMODE ),
    PLMAP( PROP_RtlGutter,                   XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_SW_TYPE_RTLGUTTER | MID_FLAG_MULTI_PROPERTY, CTF_PM_RTLGUTTER ),

    // Index 53: Grid definitions
    PLMAP( PROP_GridColor, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_COLOR, XML_TYPE_COLOR, 0 ),
    PLMAP( PROP_GridLines, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_LINES, XML_TYPE_NUMBER16, 0 ),
    PLMAP( PROP_GridBaseHeight, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( PROP_GridRubyHeight, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( PROP_GridMode, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_MODE, XML_TYPE_LAYOUT_GRID_MODE, 0 ),
    PLMAP( PROP_RubyBelow, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_BELOW, XML_TYPE_BOOL, 0 ),
    PLMAP( PROP_GridPrint, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_PRINT, XML_TYPE_BOOL, 0 ),
    PLMAP( PROP_GridDisplay, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_DISPLAY, XML_TYPE_BOOL, 0 ),

    //text grid enhancement for better CJK support
    PLMAP_12( PROP_GridBaseWidth, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_WIDTH, XML_TYPE_MEASURE, CTP_PM_GRID_BASE_WIDTH ),
    PLMAP_12( PROP_GridSnapToChars, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_SNAP_TO, XML_TYPE_BOOL, CTP_PM_GRID_SNAP_TO ),
      //export as a default attribute
    PLMAP_12( PROP_StandardPageMode, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_STANDARD_MODE, XML_TYPE_BOOL|MID_FLAG_DEFAULT_ITEM_EXPORT, CTF_PM_STANDARD_MODE ),

    PLMAP( PROP_UserDefinedAttributes,    XML_NAMESPACE_TEXT,        XML_XMLNS,                        XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    //Index 65: fill attributes; use PLMAP macro here instead of GMAP, this list is ordered and its order is used
    // to decide in which section in ODF to export the contained stuff (the PageMasterStyle creates several XML
    // sections, for Page, Header and Footer). The needed order seems to rely not on filtering, but using sections
    // based on the order used in this list.
    // Also need own defines for the used context flags (e.g. CTF_PM_FILLGRADIENTNAME instead of
    // CTF_FILLGRADIENTNAME) since these are used to *filter* up to which entry the attributes belong to the
    // 'page-layout-properties' section (!), see SvXMLAutoStylePoolP_Impl::exportXML, look for XmlStyleFamily::PAGE_MASTER
    // note: these are duplicated below, in g_XMLPageMasterDrawingPageStyleMap
    PLMAP( PROP_FillStyle,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  0 ),
    PLMAP( PROP_FillColor,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         0 ),
    PLMAP( PROP_FillColor2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         0 ),
    PLMAP( PROP_FillGradientName,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLGRADIENTNAME ),
    PLMAP( PROP_FillGradientStepCount,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      0 ),
    PLMAP( PROP_FillHatchName,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLHATCHNAME ),
    PLMAP( PROP_FillBackground,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          0 ),
    PLMAP( PROP_FillBitmapName,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLBITMAPNAME ),
    PLMAP( PROP_FillTransparence,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         0 ),    // exists in SW, too
    PLMAP( PROP_FillTransparenceGradientName,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLTRANSNAME ),
    PLMAP( PROP_FillBitmapSizeX,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     0 ),
    PLMAP( PROP_FillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       0 ),
    PLMAP( PROP_FillBitmapSizeY,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     0 ),
    PLMAP( PROP_FillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       0 ),
    PLMAP( PROP_FillBitmapMode,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_FILLBITMAPMODE ),
    PLMAP( PROP_FillBitmapPositionOffsetX,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       0 ),
    PLMAP( PROP_FillBitmapPositionOffsetY,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       0 ),
    PLMAP( PROP_FillBitmapRectanglePoint,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            0 ),
    PLMAP( PROP_FillBitmapOffsetX,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_REPEAT_OFFSET_X ),
    PLMAP( PROP_FillBitmapOffsetY,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_REPEAT_OFFSET_Y ),

    // Index 85: footnote
    PLMAP( PROP_FootnoteHeight,            XML_NAMESPACE_STYLE,    XML_FOOTNOTE_MAX_HEIGHT, XML_TYPE_MEASURE, CTF_PM_FTN_HEIGHT ),
    PLMAP( PROP_FootnoteLineAdjust,        XML_NAMESPACE_STYLE,    XML__EMPTY,        XML_TYPE_TEXT_HORIZONTAL_ADJUST|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_ADJUST ),
    PLMAP( PROP_FootnoteLineColor,        XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM,        CTF_PM_FTN_LINE_COLOR ),
    PLMAP( PROP_FootnoteLineDistance,    XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,CTF_PM_FTN_DISTANCE ),
    PLMAP( PROP_FootnoteLineRelativeWidth, XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_PERCENT8|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_WIDTH ),
    PLMAP( PROP_FootnoteLineTextDistance, XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_DISTANCE ),
    PLMAP( PROP_FootnoteLineWeight,        XML_NAMESPACE_STYLE,    XML_FOOTNOTE_SEP,    XML_TYPE_MEASURE16|MID_FLAG_ELEMENT_ITEM,    CTF_PM_FTN_LINE_WEIGHT ),
    PLMAP( PROP_FootnoteLineStyle,     XML_NAMESPACE_STYLE,    XML_EMPTY,  XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM,  CTF_PM_FTN_LINE_STYLE ),
    PLMAP_EXT(PROP_GutterMargin, XML_NAMESPACE_LO_EXT, XML_MARGIN_GUTTER, XML_TYPE_MEASURE, CTF_PM_MARGINGUTTER),

    //////////////////////////////////////////////////////////////////////////
    //Index 92: Section for 'header-style' own section, all members *have* to use CTF_PM_HEADERFLAG in the context entry (the 5th one)
    HFMAP( PROP_HeaderHeight,                XML_NAMESPACE_SVG,        XML_HEIGHT,                     XML_TYPE_MEASURE,        CTF_PM_HEADERHEIGHT ),
    HFMAP( PROP_HeaderHeight,                XML_NAMESPACE_FO,        XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,        CTF_PM_HEADERMINHEIGHT ),
    HFMAP( PROP_HeaderIsDynamicHeight,        XML_NAMESPACE_STYLE,    XML__EMPTY,                        XML_TYPE_BOOL,            CTF_PM_HEADERDYNAMIC ),
    HFMAP( PROP_HeaderLeftMargin,            XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINALL ),
    HFMAP( PROP_HeaderLeftMargin,            XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINLEFT ),
    HFMAP( PROP_HeaderRightMargin,            XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINRIGHT ),
    HFMAP( PROP_HeaderBodyDistance,        XML_NAMESPACE_FO,        XML_MARGIN_BOTTOM,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINBOTTOM ),
    HFMAP( PROP_HeaderTopBorder,                XML_NAMESPACE_FO,        XML_BORDER,                        XML_TYPE_BORDER,        CTF_PM_HEADERBORDERALL ),
    HFMAP( PROP_HeaderTopBorder,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_HEADERBORDERTOP ),
    HFMAP( PROP_HeaderBottomBorder,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERBOTTOM ),
    HFMAP( PROP_HeaderLeftBorder,            XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERLEFT ),
    HFMAP( PROP_HeaderRightBorder,            XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERRIGHT ),
    HFMAP( PROP_HeaderTopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHALL ),
    HFMAP( PROP_HeaderTopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHTOP ),
    HFMAP( PROP_HeaderBottomBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHBOTTOM ),
    HFMAP( PROP_HeaderLeftBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHLEFT ),
    HFMAP( PROP_HeaderRightBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHRIGHT ),
    HFMAP( PROP_HeaderTopBorderDistance,     XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGALL ),
    HFMAP( PROP_HeaderTopBorderDistance,     XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGTOP ),
    HFMAP( PROP_HeaderBottomBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGBOTTOM ),
    HFMAP( PROP_HeaderLeftBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGLEFT ),
    HFMAP( PROP_HeaderRightBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGRIGHT ),
    HFMAP( PROP_HeaderShadowFormat,            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,    CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderBackColor,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderBackTransparent,        XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderBackGraphicLocation,    XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERGRAPHICPOSITION ),
    HFMAP( PROP_HeaderBackGraphicFilter,        XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_HEADERGRAPHICFILTER ),
    HFMAP( PROP_HeaderBackGraphic,        XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_HEADERGRAPHICURL ),
    HFMAP( PROP_HeaderDynamicSpacing,        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,            XML_TYPE_BOOL,          CTF_PM_HEADERFLAG ),

    //Index 121: Header DrawingLayer FillAttributes
    // Use HFMAP to get XML_TYPE_PROP_HEADER_FOOTER ORed to the 4th entry
    // Names have to begin with 'Header', all 5th entries need to be ORed with the CTF_PM_HEADERFLAG
    HFMAP( PROP_HeaderFillStyle,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillColor,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillColor2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillGradientName,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLGRADIENTNAME ),
    HFMAP( PROP_HeaderFillGradientStepCount,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillHatchName,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLHATCHNAME ),
    HFMAP( PROP_HeaderFillBackground,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapName,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLBITMAPNAME ),
    HFMAP( PROP_HeaderFillTransparence,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         CTF_PM_HEADERFLAG ),    // exists in SW, too
    HFMAP( PROP_HeaderFillTransparenceGradientName,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLTRANSNAME ),
    HFMAP( PROP_HeaderFillBitmapSizeX,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapSizeY,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapMode,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapPositionOffsetX,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapPositionOffsetY,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapRectanglePoint,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            CTF_PM_HEADERFLAG ),
    HFMAP( PROP_HeaderFillBitmapOffsetX,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_HEADERREPEAT_OFFSET_X ),
    HFMAP( PROP_HeaderFillBitmapOffsetY,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_HEADERREPEAT_OFFSET_Y ),

    //////////////////////////////////////////////////////////////////////////
    //Index 141: Section for 'footer-style' own section, all members *have* to use CTF_PM_FOOTERFLAG in the context entry (the 5th one)
    HFMAP( PROP_FooterHeight,                XML_NAMESPACE_SVG,        XML_HEIGHT,                     XML_TYPE_MEASURE,        CTF_PM_FOOTERHEIGHT ),
    HFMAP( PROP_FooterHeight,                XML_NAMESPACE_FO,        XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,        CTF_PM_FOOTERMINHEIGHT ),
    HFMAP( PROP_FooterIsDynamicHeight,        XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,            CTF_PM_FOOTERDYNAMIC ),
    HFMAP( PROP_FooterLeftMargin,            XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINALL ),
    HFMAP( PROP_FooterLeftMargin,            XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINLEFT ),
    HFMAP( PROP_FooterRightMargin,            XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINRIGHT ),
    HFMAP( PROP_FooterBodyDistance,        XML_NAMESPACE_FO,        XML_MARGIN_TOP,                 XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINTOP ),
    HFMAP( PROP_FooterTopBorder,                XML_NAMESPACE_FO,        XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERALL ),
    HFMAP( PROP_FooterTopBorder,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERTOP ),
    HFMAP( PROP_FooterBottomBorder,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERBOTTOM ),
    HFMAP( PROP_FooterLeftBorder,            XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERLEFT ),
    HFMAP( PROP_FooterRightBorder,            XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERRIGHT ),
    HFMAP( PROP_FooterTopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHALL ),
    HFMAP( PROP_FooterTopBorder,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHTOP ),
    HFMAP( PROP_FooterBottomBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHBOTTOM ),
    HFMAP( PROP_FooterLeftBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHLEFT ),
    HFMAP( PROP_FooterRightBorder,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHRIGHT ),
    HFMAP( PROP_FooterTopBorderDistance,     XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGALL ),
    HFMAP( PROP_FooterTopBorderDistance,     XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGTOP ),
    HFMAP( PROP_FooterBottomBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGBOTTOM ),
    HFMAP( PROP_FooterLeftBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGLEFT ),
    HFMAP( PROP_FooterRightBorderDistance,    XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGRIGHT ),
    HFMAP( PROP_FooterShadowFormat,            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,    CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterBackColor,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterBackTransparent,        XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterBackGraphicLocation,    XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERGRAPHICPOSITION ),
    HFMAP( PROP_FooterBackGraphicFilter,     XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_FOOTERGRAPHICFILTER ),
    HFMAP( PROP_FooterBackGraphic,           XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_FOOTERGRAPHICURL ),
    HFMAP( PROP_FooterDynamicSpacing,        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,           XML_TYPE_BOOL,          CTF_PM_FOOTERFLAG ),

    //Index 170: Footer DrawingLayer FillAttributes
    // Use HFMAP to get XML_TYPE_PROP_HEADER_FOOTER ORed to the 4th entry
    // Names have to begin with 'Footer', all 5th entries need to be ORed with the CTF_PM_FOOTERFLAG
    HFMAP( PROP_FooterFillStyle,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillColor,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillColor2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillGradientName,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLGRADIENTNAME ),
    HFMAP( PROP_FooterFillGradientStepCount,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillHatchName,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLHATCHNAME ),
    HFMAP( PROP_FooterFillBackground,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapName,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLBITMAPNAME ),
    HFMAP( PROP_FooterFillTransparence,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         CTF_PM_FOOTERFLAG ),    // exists in SW, too
    HFMAP( PROP_FooterFillTransparenceGradientName,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLTRANSNAME ),
    HFMAP( PROP_FooterFillBitmapSizeX,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapSizeY,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapLogicalSize,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapMode,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapPositionOffsetX,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapPositionOffsetY,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapRectanglePoint,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            CTF_PM_FOOTERFLAG ),
    HFMAP( PROP_FooterFillBitmapOffsetX,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_FOOTERREPEAT_OFFSET_X ),
    HFMAP( PROP_FooterFillBitmapOffsetY,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_FOOTERREPEAT_OFFSET_Y ),

    { nullptr } // index 190
};

XMLPropertyMapEntry const g_XMLPageMasterDrawingPageStyleMap[] =
{
    // ODF 1.3 OFFICE-3937 style of family "drawing-page" referenced from style:master-page
    // duplication of relevant part of aXMLPageMasterStyleMap but as DP type
    DPMAP( PROP_FillStyle,                    XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                CTF_PM_FILL),
    DPMAP( PROP_BackgroundFullSize,           XML_NAMESPACE_DRAW,     XML_BACKGROUND_SIZE,        XML_SW_TYPE_PRESPAGE_BACKSIZE,                        CTF_PM_BACKGROUNDSIZE),
    DPMAP( PROP_FillColor,                    XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                       0),
    DPMAP( PROP_FillColor2,                   XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                       0),
    DPMAP( PROP_FillGradientName,             XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLGRADIENTNAME),
    DPMAP( PROP_FillGradientStepCount,        XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                    0),
    DPMAP( PROP_FillHatchName,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLHATCHNAME),
    DPMAP( PROP_FillBackground,               XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                        0),
    DPMAP( PROP_FillBitmapName,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLBITMAPNAME),
    DPMAP( PROP_FillTransparence,             XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,       0),    /* exists in SW, too */
    DPMAP( PROP_FillTransparenceGradientName, XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLTRANSNAME),
    DPMAP( PROP_FillBitmapSizeX,              XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,   0),
    DPMAP( PROP_FillBitmapLogicalSize,        XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,     0),
    DPMAP( PROP_FillBitmapSizeY,              XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,   0),
    DPMAP( PROP_FillBitmapLogicalSize,        XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,     0),
    DPMAP( PROP_FillBitmapMode,               XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,      CTF_PM_FILLBITMAPMODE),
    DPMAP( PROP_FillBitmapPositionOffsetX,    XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                     0),
    DPMAP( PROP_FillBitmapPositionOffsetY,    XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                     0),
    DPMAP( PROP_FillBitmapRectanglePoint,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                          0),
    DPMAP( PROP_FillBitmapOffsetX,            XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_PM_REPEAT_OFFSET_X),
    DPMAP( PROP_FillBitmapOffsetY,            XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_PM_REPEAT_OFFSET_Y),

    { nullptr }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
