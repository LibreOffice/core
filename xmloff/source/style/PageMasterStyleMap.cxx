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

constexpr OUStringLiteral BACKGROUND_FULL_SIZE = u"BackgroundFullSize";
constexpr OUStringLiteral BACK_COLOR = u"BackColor";
constexpr OUStringLiteral BACK_GRAPHIC = u"BackGraphic";
constexpr OUStringLiteral BACK_GRAPHIC_FILTER = u"BackGraphicFilter";
constexpr OUStringLiteral BACK_GRAPHIC_LOCATION = u"BackGraphicLocation";
constexpr OUStringLiteral BACK_TRANSPARENT = u"BackTransparent";
constexpr OUStringLiteral BOTTOM_BORDER = u"BottomBorder";
constexpr OUStringLiteral BOTTOM_BORDER_DISTANCE = u"BottomBorderDistance";
constexpr OUStringLiteral BOTTOM_MARGIN = u"BottomMargin";
constexpr OUStringLiteral CENTER_HORIZONTALLY = u"CenterHorizontally";
constexpr OUStringLiteral CENTER_VERTICALLY = u"CenterVertically";
constexpr OUStringLiteral FILL_BACKGROUND = u"FillBackground";
constexpr OUStringLiteral FILL_BITMAP_LOGICAL_SIZE = u"FillBitmapLogicalSize";
constexpr OUStringLiteral FILL_BITMAP_MODE = u"FillBitmapMode";
constexpr OUStringLiteral FILL_BITMAP_NAME = u"FillBitmapName";
constexpr OUStringLiteral FILL_BITMAP_OFFSET_X = u"FillBitmapOffsetX";
constexpr OUStringLiteral FILL_BITMAP_OFFSET_Y = u"FillBitmapOffsetY";
constexpr OUStringLiteral FILL_BITMAP_POSITION_OFFSET_X = u"FillBitmapPositionOffsetX";
constexpr OUStringLiteral FILL_BITMAP_POSITION_OFFSET_Y = u"FillBitmapPositionOffsetY";
constexpr OUStringLiteral FILL_BITMAP_RECTANGLE_POINT = u"FillBitmapRectanglePoint";
constexpr OUStringLiteral FILL_BITMAP_SIZE_X = u"FillBitmapSizeX";
constexpr OUStringLiteral FILL_BITMAP_SIZE_Y = u"FillBitmapSizeY";
constexpr OUStringLiteral FILL_COLOR = u"FillColor";
constexpr OUStringLiteral FILL_COLOR2 = u"FillColor2";
constexpr OUStringLiteral FILL_GRADIENT_NAME = u"FillGradientName";
constexpr OUStringLiteral FILL_GRADIENT_STEP_COUNT = u"FillGradientStepCount";
constexpr OUStringLiteral FILL_HATCH_NAME = u"FillHatchName";
constexpr OUStringLiteral FILL_STYLE = u"FillStyle";
constexpr OUStringLiteral FILL_TRANSPARENCE = u"FillTransparence";
constexpr OUStringLiteral FILL_TRANSPARENCE_GRADIENT_NAME = u"FillTransparenceGradientName";
constexpr OUStringLiteral FIRST_PAGE_NUMBER = u"FirstPageNumber";
constexpr OUStringLiteral FOOTER_BACK_COLOR = u"FooterBackColor";
constexpr OUStringLiteral FOOTER_BACK_GRAPHIC = u"FooterBackGraphic";
constexpr OUStringLiteral FOOTER_BACK_GRAPHIC_FILTER = u"FooterBackGraphicFilter";
constexpr OUStringLiteral FOOTER_BACK_GRAPHIC_LOCATION = u"FooterBackGraphicLocation";
constexpr OUStringLiteral FOOTER_BACK_TRANSPARENT = u"FooterBackTransparent";
constexpr OUStringLiteral FOOTER_BODY_DISTANCE = u"FooterBodyDistance";
constexpr OUStringLiteral FOOTER_BOTTOM_BORDER = u"FooterBottomBorder";
constexpr OUStringLiteral FOOTER_BOTTOM_BORDER_DISTANCE = u"FooterBottomBorderDistance";
constexpr OUStringLiteral FOOTER_DYNAMIC_SPACING = u"FooterDynamicSpacing";
constexpr OUStringLiteral FOOTER_FILL_BACKGROUND = u"FooterFillBackground";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_LOGICAL_SIZE = u"FooterFillBitmapLogicalSize";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_MODE = u"FooterFillBitmapMode";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_NAME = u"FooterFillBitmapName";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_OFFSET_X = u"FooterFillBitmapOffsetX";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_OFFSET_Y = u"FooterFillBitmapOffsetY";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_POSITION_OFFSET_X = u"FooterFillBitmapPositionOffsetX";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_POSITION_OFFSET_Y = u"FooterFillBitmapPositionOffsetY";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_RECTANGLE_POINT = u"FooterFillBitmapRectanglePoint";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_SIZE_X = u"FooterFillBitmapSizeX";
constexpr OUStringLiteral FOOTER_FILL_BITMAP_SIZE_Y = u"FooterFillBitmapSizeY";
constexpr OUStringLiteral FOOTER_FILL_COLOR = u"FooterFillColor";
constexpr OUStringLiteral FOOTER_FILL_COLOR2 = u"FooterFillColor2";
constexpr OUStringLiteral FOOTER_FILL_GRADIENT_NAME = u"FooterFillGradientName";
constexpr OUStringLiteral FOOTER_FILL_GRADIENT_STEP_COUNT = u"FooterFillGradientStepCount";
constexpr OUStringLiteral FOOTER_FILL_HATCH_NAME = u"FooterFillHatchName";
constexpr OUStringLiteral FOOTER_FILL_STYLE = u"FooterFillStyle";
constexpr OUStringLiteral FOOTER_FILL_TRANSPARENCE = u"FooterFillTransparence";
constexpr OUStringLiteral FOOTER_FILL_TRANSPARENCE_GRADIENT_NAME = u"FooterFillTransparenceGradientName";
constexpr OUStringLiteral FOOTER_HEIGHT = u"FooterHeight";
constexpr OUStringLiteral FOOTER_IS_DYNAMIC_HEIGHT = u"FooterIsDynamicHeight";
constexpr OUStringLiteral FOOTER_LEFT_BORDER = u"FooterLeftBorder";
constexpr OUStringLiteral FOOTER_LEFT_BORDER_DISTANCE = u"FooterLeftBorderDistance";
constexpr OUStringLiteral FOOTER_LEFT_MARGIN = u"FooterLeftMargin";
constexpr OUStringLiteral FOOTER_RIGHT_BORDER = u"FooterRightBorder";
constexpr OUStringLiteral FOOTER_RIGHT_BORDER_DISTANCE = u"FooterRightBorderDistance";
constexpr OUStringLiteral FOOTER_RIGHT_MARGIN = u"FooterRightMargin";
constexpr OUStringLiteral FOOTER_SHADOW_FORMAT = u"FooterShadowFormat";
constexpr OUStringLiteral FOOTER_TOP_BORDER = u"FooterTopBorder";
constexpr OUStringLiteral FOOTER_TOP_BORDER_DISTANCE = u"FooterTopBorderDistance";
constexpr OUStringLiteral FOOTNOTE_HEIGHT = u"FootnoteHeight";
constexpr OUStringLiteral FOOTNOTE_LINE_ADJUST = u"FootnoteLineAdjust";
constexpr OUStringLiteral FOOTNOTE_LINE_COLOR = u"FootnoteLineColor";
constexpr OUStringLiteral FOOTNOTE_LINE_DISTANCE = u"FootnoteLineDistance";
constexpr OUStringLiteral FOOTNOTE_LINE_RELATIVE_WIDTH = u"FootnoteLineRelativeWidth";
constexpr OUStringLiteral FOOTNOTE_LINE_STYLE = u"FootnoteLineStyle";
constexpr OUStringLiteral FOOTNOTE_LINE_TEXT_DISTANCE = u"FootnoteLineTextDistance";
constexpr OUStringLiteral FOOTNOTE_LINE_WEIGHT = u"FootnoteLineWeight";
constexpr OUStringLiteral GRID_BASE_HEIGHT = u"GridBaseHeight";
constexpr OUStringLiteral GRID_BASE_WIDTH = u"GridBaseWidth";
constexpr OUStringLiteral GRID_COLOR = u"GridColor";
constexpr OUStringLiteral GRID_DISPLAY = u"GridDisplay";
constexpr OUStringLiteral GRID_LINES = u"GridLines";
constexpr OUStringLiteral GRID_MODE = u"GridMode";
constexpr OUStringLiteral GRID_PRINT = u"GridPrint";
constexpr OUStringLiteral GRID_RUBY_HEIGHT = u"GridRubyHeight";
constexpr OUStringLiteral GRID_SNAP_TO_CHARS = u"GridSnapToChars";
constexpr OUStringLiteral GUTTER_MARGIN = u"GutterMargin";
constexpr OUStringLiteral HEADER_BACK_COLOR = u"HeaderBackColor";
constexpr OUStringLiteral HEADER_BACK_GRAPHIC = u"HeaderBackGraphic";
constexpr OUStringLiteral HEADER_BACK_GRAPHIC_FILTER = u"HeaderBackGraphicFilter";
constexpr OUStringLiteral HEADER_BACK_GRAPHIC_LOCATION = u"HeaderBackGraphicLocation";
constexpr OUStringLiteral HEADER_BACK_TRANSPARENT = u"HeaderBackTransparent";
constexpr OUStringLiteral HEADER_BODY_DISTANCE = u"HeaderBodyDistance";
constexpr OUStringLiteral HEADER_BOTTOM_BORDER = u"HeaderBottomBorder";
constexpr OUStringLiteral HEADER_BOTTOM_BORDER_DISTANCE = u"HeaderBottomBorderDistance";
constexpr OUStringLiteral HEADER_DYNAMIC_SPACING = u"HeaderDynamicSpacing";
constexpr OUStringLiteral HEADER_FILL_BACKGROUND = u"HeaderFillBackground";
constexpr OUStringLiteral HEADER_FILL_BITMAP_LOGICAL_SIZE = u"HeaderFillBitmapLogicalSize";
constexpr OUStringLiteral HEADER_FILL_BITMAP_MODE = u"HeaderFillBitmapMode";
constexpr OUStringLiteral HEADER_FILL_BITMAP_NAME = u"HeaderFillBitmapName";
constexpr OUStringLiteral HEADER_FILL_BITMAP_OFFSET_X = u"HeaderFillBitmapOffsetX";
constexpr OUStringLiteral HEADER_FILL_BITMAP_OFFSET_Y = u"HeaderFillBitmapOffsetY";
constexpr OUStringLiteral HEADER_FILL_BITMAP_POSITION_OFFSET_X = u"HeaderFillBitmapPositionOffsetX";
constexpr OUStringLiteral HEADER_FILL_BITMAP_POSITION_OFFSET_Y = u"HeaderFillBitmapPositionOffsetY";
constexpr OUStringLiteral HEADER_FILL_BITMAP_RECTANGLE_POINT = u"HeaderFillBitmapRectanglePoint";
constexpr OUStringLiteral HEADER_FILL_BITMAP_SIZE_X = u"HeaderFillBitmapSizeX";
constexpr OUStringLiteral HEADER_FILL_BITMAP_SIZE_Y = u"HeaderFillBitmapSizeY";
constexpr OUStringLiteral HEADER_FILL_COLOR = u"HeaderFillColor";
constexpr OUStringLiteral HEADER_FILL_COLOR2 = u"HeaderFillColor2";
constexpr OUStringLiteral HEADER_FILL_GRADIENT_NAME = u"HeaderFillGradientName";
constexpr OUStringLiteral HEADER_FILL_GRADIENT_STEP_COUNT = u"HeaderFillGradientStepCount";
constexpr OUStringLiteral HEADER_FILL_HATCH_NAME = u"HeaderFillHatchName";
constexpr OUStringLiteral HEADER_FILL_STYLE = u"HeaderFillStyle";
constexpr OUStringLiteral HEADER_FILL_TRANSPARENCE = u"HeaderFillTransparence";
constexpr OUStringLiteral HEADER_FILL_TRANSPARENCE_GRADIENT_NAME = u"HeaderFillTransparenceGradientName";
constexpr OUStringLiteral HEADER_HEIGHT = u"HeaderHeight";
constexpr OUStringLiteral HEADER_IS_DYNAMIC_HEIGHT = u"HeaderIsDynamicHeight";
constexpr OUStringLiteral HEADER_LEFT_BORDER = u"HeaderLeftBorder";
constexpr OUStringLiteral HEADER_LEFT_BORDER_DISTANCE = u"HeaderLeftBorderDistance";
constexpr OUStringLiteral HEADER_LEFT_MARGIN = u"HeaderLeftMargin";
constexpr OUStringLiteral HEADER_RIGHT_BORDER = u"HeaderRightBorder";
constexpr OUStringLiteral HEADER_RIGHT_BORDER_DISTANCE = u"HeaderRightBorderDistance";
constexpr OUStringLiteral HEADER_RIGHT_MARGIN = u"HeaderRightMargin";
constexpr OUStringLiteral HEADER_SHADOW_FORMAT = u"HeaderShadowFormat";
constexpr OUStringLiteral HEADER_TOP_BORDER = u"HeaderTopBorder";
constexpr OUStringLiteral HEADER_TOP_BORDER_DISTANCE = u"HeaderTopBorderDistance";
constexpr OUStringLiteral HEIGHT = u"Height";
constexpr OUStringLiteral IS_LANDSCAPE = u"IsLandscape";
constexpr OUStringLiteral LEFT_BORDER = u"LeftBorder";
constexpr OUStringLiteral LEFT_BORDER_DISTANCE = u"LeftBorderDistance";
constexpr OUStringLiteral LEFT_MARGIN = u"LeftMargin";
constexpr OUStringLiteral NUMBERING_TYPE = u"NumberingType";
constexpr OUStringLiteral PAGE_SCALE = u"PageScale";
constexpr OUStringLiteral PAGE_STYLE_LAYOUT = u"PageStyleLayout";
constexpr OUStringLiteral PRINTER_PAPER_TRAY = u"PrinterPaperTray";
constexpr OUStringLiteral PRINT_ANNOTATIONS = u"PrintAnnotations";
constexpr OUStringLiteral PRINT_CHARTS = u"PrintCharts";
constexpr OUStringLiteral PRINT_DOWN_FIRST = u"PrintDownFirst";
constexpr OUStringLiteral PRINT_DRAWING = u"PrintDrawing";
constexpr OUStringLiteral PRINT_FORMULAS = u"PrintFormulas";
constexpr OUStringLiteral PRINT_GRID = u"PrintGrid";
constexpr OUStringLiteral PRINT_HEADERS = u"PrintHeaders";
constexpr OUStringLiteral PRINT_OBJECTS = u"PrintObjects";
constexpr OUStringLiteral PRINT_ZERO_VALUES = u"PrintZeroValues";
constexpr OUStringLiteral REGISTER_MODE_ACTIVE = u"RegisterModeActive";
constexpr OUStringLiteral REGISTER_PARAGRAPH_STYLE = u"RegisterParagraphStyle";
constexpr OUStringLiteral RIGHT_BORDER = u"RightBorder";
constexpr OUStringLiteral RIGHT_BORDER_DISTANCE = u"RightBorderDistance";
constexpr OUStringLiteral RIGHT_MARGIN = u"RightMargin";
constexpr OUStringLiteral RTL_GUTTER = u"RtlGutter";
constexpr OUStringLiteral RUBY_BELOW = u"RubyBelow";
constexpr OUStringLiteral SCALE_TO_PAGES = u"ScaleToPages";
constexpr OUStringLiteral SCALE_TO_PAGES_X = u"ScaleToPagesX";
constexpr OUStringLiteral SCALE_TO_PAGES_Y = u"ScaleToPagesY";
constexpr OUStringLiteral SHADOW_FORMAT = u"ShadowFormat";
constexpr OUStringLiteral STANDARD_PAGE_MODE = u"StandardPageMode";
constexpr OUStringLiteral TEXT_COLUMNS = u"TextColumns";
constexpr OUStringLiteral TOP_BORDER = u"TopBorder";
constexpr OUStringLiteral TOP_BORDER_DISTANCE = u"TopBorderDistance";
constexpr OUStringLiteral TOP_MARGIN = u"TopMargin";
constexpr OUStringLiteral USER_DEFINED_ATTRIBUTES = u"UserDefinedAttributes";
constexpr OUStringLiteral WIDTH = u"Width";
constexpr OUStringLiteral WRITING_MODE = u"WritingMode";

const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
    //////////////////////////////////////////////////////////////////////////
    // Section for 'page-layout-properties'

    // page master
    PLMAP( PAGE_STYLE_LAYOUT,            XML_NAMESPACE_STYLE,    XML_PAGE_USAGE,                    XML_PM_TYPE_PAGESTYLELAYOUT | MID_FLAG_SPECIAL_ITEM,    CTF_PM_PAGEUSAGE ),
    PLMAP( WIDTH,                    XML_NAMESPACE_FO,        XML_PAGE_WIDTH,                    XML_TYPE_MEASURE,                                        0 ),
    PLMAP( HEIGHT,                    XML_NAMESPACE_FO,        XML_PAGE_HEIGHT,                XML_TYPE_MEASURE,                                        0 ),
    PLMAP( NUMBERING_TYPE,            XML_NAMESPACE_STYLE,    XML_NUM_FORMAT,                    XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY,        0 ),
    PLMAP( NUMBERING_TYPE,            XML_NAMESPACE_STYLE,    XML_NUM_LETTER_SYNC,            XML_PM_TYPE_NUMLETTERSYNC,                                0 ),
    PLMAP( PRINTER_PAPER_TRAY,        XML_NAMESPACE_STYLE,    XML_PAPER_TRAY_NAME,            XML_TYPE_STRING | MID_FLAG_PROPERTY_MAY_THROW,         0 ),
    PLMAP( IS_LANDSCAPE,                XML_NAMESPACE_STYLE,    XML_PRINT_ORIENTATION,            XML_PM_TYPE_PRINTORIENTATION,                            0 ),
    PLMAP( TOP_MARGIN,                XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,                                        CTF_PM_MARGINALL ),
    PLMAP( TOP_MARGIN,                XML_NAMESPACE_FO,        XML_MARGIN_TOP,                    XML_TYPE_MEASURE,                                        CTF_PM_MARGINTOP ),
    PLMAP( BOTTOM_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN_BOTTOM,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINBOTTOM ),
    PLMAP( LEFT_MARGIN,                XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINLEFT ),
    PLMAP( RIGHT_MARGIN,                XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,                                        CTF_PM_MARGINRIGHT ),
    PLMAP( TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER,                     XML_TYPE_BORDER,                                        CTF_PM_BORDERALL ),
    PLMAP( TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                    XML_TYPE_BORDER,                                        CTF_PM_BORDERTOP ),
    PLMAP( BOTTOM_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,                                        CTF_PM_BORDERBOTTOM ),
    PLMAP( LEFT_BORDER,                XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERLEFT ),
    PLMAP( RIGHT_BORDER,                XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERRIGHT ),
    PLMAP( TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHALL ),
    PLMAP( TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHTOP ),
    PLMAP( BOTTOM_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHBOTTOM ),
    PLMAP( LEFT_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHLEFT ),
    PLMAP( RIGHT_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,                                    CTF_PM_BORDERWIDTHRIGHT ),
    PLMAP( TOP_BORDER_DISTANCE,        XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,                                        CTF_PM_PADDINGALL ),
    PLMAP( TOP_BORDER_DISTANCE,        XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGTOP ),
    PLMAP( BOTTOM_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,                                        CTF_PM_PADDINGBOTTOM ),
    PLMAP( LEFT_BORDER_DISTANCE,        XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGLEFT ),
    PLMAP( RIGHT_BORDER_DISTANCE,     XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,                                        CTF_PM_PADDINGRIGHT ),
    PLMAP( SHADOW_FORMAT,            XML_NAMESPACE_STYLE,    XML_SHADOW,                        XML_TYPE_TEXT_SHADOW,                                    0 ),
    PLMAP( BACK_COLOR,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    0 ),
    PLMAP( BACK_TRANSPARENT,            XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        0 ),
    PLMAP( BACK_GRAPHIC_LOCATION,        XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_GRAPHICPOSITION ),
    PLMAP( BACK_GRAPHIC_FILTER,        XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_GRAPHICFILTER ),
    PLMAP( BACK_GRAPHIC,            XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_GRAPHICURL ),
    PLMAP( PRINT_ANNOTATIONS,        XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTANNOTATIONS | MID_FLAG_MULTI_PROPERTY,                            CTF_PM_PRINT_ANNOTATIONS ),
    PLMAP( PRINT_CHARTS,                XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTCHARTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_CHARTS ),
    PLMAP( PRINT_DRAWING,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTDRAWING | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_DRAWING ),
    PLMAP( PRINT_FORMULAS,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTFORMULAS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_FORMULAS ),
    PLMAP( PRINT_GRID,                XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTGRID | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_PRINT_GRID ),
    PLMAP( PRINT_HEADERS,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTHEADERS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_HEADERS ),
    PLMAP( PRINT_OBJECTS,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTOBJECTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,    CTF_PM_PRINT_OBJECTS ),
    PLMAP( PRINT_ZERO_VALUES,            XML_NAMESPACE_STYLE,    XML_PRINT,                        XML_PM_TYPE_PRINTZEROVALUES | MID_FLAG_MERGE_ATTRIBUTE,                            CTF_PM_PRINT_ZEROVALUES ),
    PLMAP( PRINT_DOWN_FIRST,            XML_NAMESPACE_STYLE,    XML_PRINT_PAGE_ORDER,            XML_PM_TYPE_PRINTPAGEORDER,                                0 ),
    PLMAP( FIRST_PAGE_NUMBER,            XML_NAMESPACE_STYLE,    XML_FIRST_PAGE_NUMBER,            XML_PM_TYPE_FIRSTPAGENUMBER,                            0 ),
    PLMAP( PAGE_SCALE,                XML_NAMESPACE_STYLE,    XML_SCALE_TO,                    XML_TYPE_PERCENT16,                                        CTF_PM_SCALETO ),
    PLMAP( SCALE_TO_PAGES,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_PAGES,             XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOPAGES ),
    // ODF 1.3 OFFICE-3857
    PLMAP_ODF13( SCALE_TO_PAGES_X,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_X,                  XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOX ),
    PLMAP_ODF13( SCALE_TO_PAGES_Y,            XML_NAMESPACE_STYLE,    XML_SCALE_TO_Y,                 XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOY ),
    PLMAP_ODF13( SCALE_TO_PAGES_X,            XML_NAMESPACE_LO_EXT,    XML_SCALE_TO_X,                  XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOX ),
    PLMAP_ODF13( SCALE_TO_PAGES_Y,            XML_NAMESPACE_LO_EXT,    XML_SCALE_TO_Y,                 XML_TYPE_NUMBER16,                                        CTF_PM_SCALETOY ),
    PLMAP( CENTER_HORIZONTALLY,        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_HORIZONTAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( CENTER_VERTICALLY,        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_VERTICAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( TEXT_COLUMNS,                XML_NAMESPACE_STYLE,    XML_COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_PM_TEXTCOLUMNS ),
    PLMAP( REGISTER_MODE_ACTIVE,        XML_NAMESPACE_STYLE,    XML_REGISTER_TRUE,    XML_TYPE_BOOL, 0 ),
    PLMAP( REGISTER_PARAGRAPH_STYLE,    XML_NAMESPACE_STYLE,    XML_REGISTER_TRUTH_REF_STYLE_NAME,    XML_TYPE_STYLENAME| MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_PM_REGISTER_STYLE ),
    PLMAP( WRITING_MODE,                 XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_TYPE_TEXT_WRITING_MODE | MID_FLAG_MULTI_PROPERTY, CTF_PM_WRITINGMODE ),
    PLMAP( RTL_GUTTER,                   XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_SW_TYPE_RTLGUTTER | MID_FLAG_MULTI_PROPERTY, CTF_PM_RTLGUTTER ),

    // Index 53: Grid definitions
    PLMAP( GRID_COLOR, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_COLOR, XML_TYPE_COLOR, 0 ),
    PLMAP( GRID_LINES, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_LINES, XML_TYPE_NUMBER16, 0 ),
    PLMAP( GRID_BASE_HEIGHT, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( GRID_RUBY_HEIGHT, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( GRID_MODE, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_MODE, XML_TYPE_LAYOUT_GRID_MODE, 0 ),
    PLMAP( RUBY_BELOW, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_BELOW, XML_TYPE_BOOL, 0 ),
    PLMAP( GRID_PRINT, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_PRINT, XML_TYPE_BOOL, 0 ),
    PLMAP( GRID_DISPLAY, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_DISPLAY, XML_TYPE_BOOL, 0 ),

    //text grid enhancement for better CJK support
    PLMAP_12( GRID_BASE_WIDTH, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_WIDTH, XML_TYPE_MEASURE, CTP_PM_GRID_BASE_WIDTH ),
    PLMAP_12( GRID_SNAP_TO_CHARS, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_SNAP_TO, XML_TYPE_BOOL, CTP_PM_GRID_SNAP_TO ),
      //export as a default attribute
    PLMAP_12( STANDARD_PAGE_MODE, XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_STANDARD_MODE, XML_TYPE_BOOL|MID_FLAG_DEFAULT_ITEM_EXPORT, CTF_PM_STANDARD_MODE ),

    PLMAP( USER_DEFINED_ATTRIBUTES,    XML_NAMESPACE_TEXT,        XML_XMLNS,                        XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    //Index 65: fill attributes; use PLMAP macro here instead of GMAP, this list is ordered and its order is used
    // to decide in which section in ODF to export the contained stuff (the PageMasterStyle creates several XML
    // sections, for Page, Header and Footer). The needed order seems to rely not on filtering, but using sections
    // based on the order used in this list.
    // Also need own defines for the used context flags (e.g. CTF_PM_FILLGRADIENTNAME instead of
    // CTF_FILLGRADIENTNAME) since these are used to *filter* up to which entry the attributes belong to the
    // 'page-layout-properties' section (!), see SvXMLAutoStylePoolP_Impl::exportXML, look for XmlStyleFamily::PAGE_MASTER
    // note: these are duplicated below, in g_XMLPageMasterDrawingPageStyleMap
    PLMAP( FILL_STYLE,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  0 ),
    PLMAP( FILL_COLOR,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         0 ),
    PLMAP( FILL_COLOR2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         0 ),
    PLMAP( FILL_GRADIENT_NAME,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLGRADIENTNAME ),
    PLMAP( FILL_GRADIENT_STEP_COUNT,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      0 ),
    PLMAP( FILL_HATCH_NAME,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLHATCHNAME ),
    PLMAP( FILL_BACKGROUND,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          0 ),
    PLMAP( FILL_BITMAP_NAME,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLBITMAPNAME ),
    PLMAP( FILL_TRANSPARENCE,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         0 ),    // exists in SW, too
    PLMAP( FILL_TRANSPARENCE_GRADIENT_NAME,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FILLTRANSNAME ),
    PLMAP( FILL_BITMAP_SIZE_X,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     0 ),
    PLMAP( FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       0 ),
    PLMAP( FILL_BITMAP_SIZE_Y,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     0 ),
    PLMAP( FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       0 ),
    PLMAP( FILL_BITMAP_MODE,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_FILLBITMAPMODE ),
    PLMAP( FILL_BITMAP_POSITION_OFFSET_X,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       0 ),
    PLMAP( FILL_BITMAP_POSITION_OFFSET_Y,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       0 ),
    PLMAP( FILL_BITMAP_RECTANGLE_POINT,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            0 ),
    PLMAP( FILL_BITMAP_OFFSET_X,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_REPEAT_OFFSET_X ),
    PLMAP( FILL_BITMAP_OFFSET_Y,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_REPEAT_OFFSET_Y ),

    // Index 85: footnote
    PLMAP( FOOTNOTE_HEIGHT,            XML_NAMESPACE_STYLE,    XML_FOOTNOTE_MAX_HEIGHT, XML_TYPE_MEASURE, CTF_PM_FTN_HEIGHT ),
    PLMAP( FOOTNOTE_LINE_ADJUST,        XML_NAMESPACE_STYLE,    XML__EMPTY,        XML_TYPE_TEXT_HORIZONTAL_ADJUST|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_ADJUST ),
    PLMAP( FOOTNOTE_LINE_COLOR,        XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM,        CTF_PM_FTN_LINE_COLOR ),
    PLMAP( FOOTNOTE_LINE_DISTANCE,    XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,CTF_PM_FTN_DISTANCE ),
    PLMAP( FOOTNOTE_LINE_RELATIVE_WIDTH, XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_PERCENT8|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_WIDTH ),
    PLMAP( FOOTNOTE_LINE_TEXT_DISTANCE, XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_DISTANCE ),
    PLMAP( FOOTNOTE_LINE_WEIGHT,        XML_NAMESPACE_STYLE,    XML_FOOTNOTE_SEP,    XML_TYPE_MEASURE16|MID_FLAG_ELEMENT_ITEM,    CTF_PM_FTN_LINE_WEIGHT ),
    PLMAP( FOOTNOTE_LINE_STYLE,     XML_NAMESPACE_STYLE,    XML_EMPTY,  XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM,  CTF_PM_FTN_LINE_STYLE ),
    PLMAP_EXT(GUTTER_MARGIN, XML_NAMESPACE_LO_EXT, XML_MARGIN_GUTTER, XML_TYPE_MEASURE, CTF_PM_MARGINGUTTER),

    //////////////////////////////////////////////////////////////////////////
    //Index 92: Section for 'header-style' own section, all members *have* to use CTF_PM_HEADERFLAG in the context entry (the 5th one)
    HFMAP( HEADER_HEIGHT,                XML_NAMESPACE_SVG,        XML_HEIGHT,                     XML_TYPE_MEASURE,        CTF_PM_HEADERHEIGHT ),
    HFMAP( HEADER_HEIGHT,                XML_NAMESPACE_FO,        XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,        CTF_PM_HEADERMINHEIGHT ),
    HFMAP( HEADER_IS_DYNAMIC_HEIGHT,        XML_NAMESPACE_STYLE,    XML__EMPTY,                        XML_TYPE_BOOL,            CTF_PM_HEADERDYNAMIC ),
    HFMAP( HEADER_LEFT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINALL ),
    HFMAP( HEADER_LEFT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINLEFT ),
    HFMAP( HEADER_RIGHT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINRIGHT ),
    HFMAP( HEADER_BODY_DISTANCE,        XML_NAMESPACE_FO,        XML_MARGIN_BOTTOM,                XML_TYPE_MEASURE,        CTF_PM_HEADERMARGINBOTTOM ),
    HFMAP( HEADER_TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER,                        XML_TYPE_BORDER,        CTF_PM_HEADERBORDERALL ),
    HFMAP( HEADER_TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_HEADERBORDERTOP ),
    HFMAP( HEADER_BOTTOM_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERBOTTOM ),
    HFMAP( HEADER_LEFT_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERLEFT ),
    HFMAP( HEADER_RIGHT_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERRIGHT ),
    HFMAP( HEADER_TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHALL ),
    HFMAP( HEADER_TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHTOP ),
    HFMAP( HEADER_BOTTOM_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHBOTTOM ),
    HFMAP( HEADER_LEFT_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHLEFT ),
    HFMAP( HEADER_RIGHT_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,    CTF_PM_HEADERBORDERWIDTHRIGHT ),
    HFMAP( HEADER_TOP_BORDER_DISTANCE,     XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGALL ),
    HFMAP( HEADER_TOP_BORDER_DISTANCE,     XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGTOP ),
    HFMAP( HEADER_BOTTOM_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGBOTTOM ),
    HFMAP( HEADER_LEFT_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGLEFT ),
    HFMAP( HEADER_RIGHT_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_HEADERPADDINGRIGHT ),
    HFMAP( HEADER_SHADOW_FORMAT,            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,    CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_BACK_COLOR,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_BACK_TRANSPARENT,        XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_BACK_GRAPHIC_LOCATION,    XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERGRAPHICPOSITION ),
    HFMAP( HEADER_BACK_GRAPHIC_FILTER,        XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_HEADERGRAPHICFILTER ),
    HFMAP( HEADER_BACK_GRAPHIC,        XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_HEADERGRAPHICURL ),
    HFMAP( HEADER_DYNAMIC_SPACING,        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,            XML_TYPE_BOOL,          CTF_PM_HEADERFLAG ),

    //Index 121: Header DrawingLayer FillAttributes
    // Use HFMAP to get XML_TYPE_PROP_HEADER_FOOTER ORed to the 4th entry
    // Names have to begin with 'Header', all 5th entries need to be ORed with the CTF_PM_HEADERFLAG
    HFMAP( HEADER_FILL_STYLE,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_COLOR,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_COLOR2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_GRADIENT_NAME,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLGRADIENTNAME ),
    HFMAP( HEADER_FILL_GRADIENT_STEP_COUNT,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_HATCH_NAME,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLHATCHNAME ),
    HFMAP( HEADER_FILL_BACKGROUND,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_NAME,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLBITMAPNAME ),
    HFMAP( HEADER_FILL_TRANSPARENCE,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         CTF_PM_HEADERFLAG ),    // exists in SW, too
    HFMAP( HEADER_FILL_TRANSPARENCE_GRADIENT_NAME,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_HEADERFILLTRANSNAME ),
    HFMAP( HEADER_FILL_BITMAP_SIZE_X,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_SIZE_Y,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_MODE,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_POSITION_OFFSET_X,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_POSITION_OFFSET_Y,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_RECTANGLE_POINT,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            CTF_PM_HEADERFLAG ),
    HFMAP( HEADER_FILL_BITMAP_OFFSET_X,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_HEADERREPEAT_OFFSET_X ),
    HFMAP( HEADER_FILL_BITMAP_OFFSET_Y,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_HEADERREPEAT_OFFSET_Y ),

    //////////////////////////////////////////////////////////////////////////
    //Index 141: Section for 'footer-style' own section, all members *have* to use CTF_PM_FOOTERFLAG in the context entry (the 5th one)
    HFMAP( FOOTER_HEIGHT,                XML_NAMESPACE_SVG,        XML_HEIGHT,                     XML_TYPE_MEASURE,        CTF_PM_FOOTERHEIGHT ),
    HFMAP( FOOTER_HEIGHT,                XML_NAMESPACE_FO,        XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,        CTF_PM_FOOTERMINHEIGHT ),
    HFMAP( FOOTER_IS_DYNAMIC_HEIGHT,        XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,            CTF_PM_FOOTERDYNAMIC ),
    HFMAP( FOOTER_LEFT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN,                        XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINALL ),
    HFMAP( FOOTER_LEFT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN_LEFT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINLEFT ),
    HFMAP( FOOTER_RIGHT_MARGIN,            XML_NAMESPACE_FO,        XML_MARGIN_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINRIGHT ),
    HFMAP( FOOTER_BODY_DISTANCE,        XML_NAMESPACE_FO,        XML_MARGIN_TOP,                 XML_TYPE_MEASURE,        CTF_PM_FOOTERMARGINTOP ),
    HFMAP( FOOTER_TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERALL ),
    HFMAP( FOOTER_TOP_BORDER,                XML_NAMESPACE_FO,        XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERTOP ),
    HFMAP( FOOTER_BOTTOM_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_BOTTOM,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERBOTTOM ),
    HFMAP( FOOTER_LEFT_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERLEFT ),
    HFMAP( FOOTER_RIGHT_BORDER,            XML_NAMESPACE_FO,        XML_BORDER_RIGHT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERRIGHT ),
    HFMAP( FOOTER_TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,            XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHALL ),
    HFMAP( FOOTER_TOP_BORDER,                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,        XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHTOP ),
    HFMAP( FOOTER_BOTTOM_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,    XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHBOTTOM ),
    HFMAP( FOOTER_LEFT_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHLEFT ),
    HFMAP( FOOTER_RIGHT_BORDER,            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,    CTF_PM_FOOTERBORDERWIDTHRIGHT ),
    HFMAP( FOOTER_TOP_BORDER_DISTANCE,     XML_NAMESPACE_FO,        XML_PADDING,                    XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGALL ),
    HFMAP( FOOTER_TOP_BORDER_DISTANCE,     XML_NAMESPACE_FO,        XML_PADDING_TOP,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGTOP ),
    HFMAP( FOOTER_BOTTOM_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGBOTTOM ),
    HFMAP( FOOTER_LEFT_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_LEFT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGLEFT ),
    HFMAP( FOOTER_RIGHT_BORDER_DISTANCE,    XML_NAMESPACE_FO,        XML_PADDING_RIGHT,                XML_TYPE_MEASURE,        CTF_PM_FOOTERPADDINGRIGHT ),
    HFMAP( FOOTER_SHADOW_FORMAT,            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,    CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_BACK_COLOR,                XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_BACK_TRANSPARENT,        XML_NAMESPACE_FO,        XML_BACKGROUND_COLOR,            XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,        CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_BACK_GRAPHIC_LOCATION,    XML_NAMESPACE_STYLE,    XML_POSITION,                    XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERGRAPHICPOSITION ),
    HFMAP( FOOTER_BACK_GRAPHIC_FILTER,     XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_FOOTERGRAPHICFILTER ),
    HFMAP( FOOTER_BACK_GRAPHIC,           XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,            XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_FOOTERGRAPHICURL ),
    HFMAP( FOOTER_DYNAMIC_SPACING,        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,           XML_TYPE_BOOL,          CTF_PM_FOOTERFLAG ),

    //Index 170: Footer DrawingLayer FillAttributes
    // Use HFMAP to get XML_TYPE_PROP_HEADER_FOOTER ORed to the 4th entry
    // Names have to begin with 'Footer', all 5th entries need to be ORed with the CTF_PM_FOOTERFLAG
    HFMAP( FOOTER_FILL_STYLE,                     XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                  CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_COLOR,                     XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                         CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_COLOR2,                    XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                         CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_GRADIENT_NAME,              XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLGRADIENTNAME ),
    HFMAP( FOOTER_FILL_GRADIENT_STEP_COUNT,         XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                      CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_HATCH_NAME,                 XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLHATCHNAME ),
    HFMAP( FOOTER_FILL_BACKGROUND,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                          CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_NAME,                XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLBITMAPNAME ),
    HFMAP( FOOTER_FILL_TRANSPARENCE,              XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,         CTF_PM_FOOTERFLAG ),    // exists in SW, too
    HFMAP( FOOTER_FILL_TRANSPARENCE_GRADIENT_NAME,  XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,         CTF_PM_FOOTERFILLTRANSNAME ),
    HFMAP( FOOTER_FILL_BITMAP_SIZE_X,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_SIZE_Y,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,     CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_LOGICAL_SIZE,         XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,       CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_MODE,                XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,        CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_POSITION_OFFSET_X,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                       CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_POSITION_OFFSET_Y,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                       CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_RECTANGLE_POINT,      XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                            CTF_PM_FOOTERFLAG ),
    HFMAP( FOOTER_FILL_BITMAP_OFFSET_X,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY,   CTF_PM_FOOTERREPEAT_OFFSET_X ),
    HFMAP( FOOTER_FILL_BITMAP_OFFSET_Y,             XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY,   CTF_PM_FOOTERREPEAT_OFFSET_Y ),

    { nullptr, 0, XML_EMPTY, 0, 0, SvtSaveOptions::ODFSVER_010, false } // index 190
};

XMLPropertyMapEntry const g_XMLPageMasterDrawingPageStyleMap[] =
{
    // ODF 1.3 OFFICE-3937 style of family "drawing-page" referenced from style:master-page
    // duplication of relevant part of aXMLPageMasterStyleMap but as DP type
    DPMAP(FILL_STYLE,                    XML_NAMESPACE_DRAW,     XML_FILL,                   XML_SW_TYPE_FILLSTYLE,                                CTF_PM_FILL),
    DPMAP(BACKGROUND_FULL_SIZE,           XML_NAMESPACE_DRAW,     XML_BACKGROUND_SIZE,        XML_SW_TYPE_PRESPAGE_BACKSIZE,                        CTF_PM_BACKGROUNDSIZE),
    DPMAP(FILL_COLOR,                    XML_NAMESPACE_DRAW,     XML_FILL_COLOR,             XML_TYPE_COLOR,                                       0),
    DPMAP(FILL_COLOR2,                   XML_NAMESPACE_DRAW,     XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR,                                       0),
    DPMAP(FILL_GRADIENT_NAME,             XML_NAMESPACE_DRAW,     XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLGRADIENTNAME),
    DPMAP(FILL_GRADIENT_STEP_COUNT,        XML_NAMESPACE_DRAW,     XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16,                                    0),
    DPMAP(FILL_HATCH_NAME,                XML_NAMESPACE_DRAW,     XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLHATCHNAME),
    DPMAP(FILL_BACKGROUND,               XML_NAMESPACE_DRAW,     XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL,                                        0),
    DPMAP(FILL_BITMAP_NAME,               XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLBITMAPNAME),
    DPMAP(FILL_TRANSPARENCE,             XML_NAMESPACE_DRAW,     XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY,       0),    /* exists in SW, too */
    DPMAP(FILL_TRANSPARENCE_GRADIENT_NAME, XML_NAMESPACE_DRAW,     XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT,       CTF_PM_FILLTRANSNAME),
    DPMAP(FILL_BITMAP_SIZE_X,              XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,   0),
    DPMAP(FILL_BITMAP_LOGICAL_SIZE,        XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,     0),
    DPMAP(FILL_BITMAP_SIZE_Y,              XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY,   0),
    DPMAP(FILL_BITMAP_LOGICAL_SIZE,        XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY,     0),
    DPMAP(FILL_BITMAP_MODE,               XML_NAMESPACE_STYLE,    XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY,      CTF_PM_FILLBITMAPMODE),
    DPMAP(FILL_BITMAP_POSITION_OFFSET_X,    XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT,                                     0),
    DPMAP(FILL_BITMAP_POSITION_OFFSET_Y,    XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT,                                     0),
    DPMAP(FILL_BITMAP_RECTANGLE_POINT,     XML_NAMESPACE_DRAW,     XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT,                          0),
    DPMAP(FILL_BITMAP_OFFSET_X,            XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_PM_REPEAT_OFFSET_X),
    DPMAP(FILL_BITMAP_OFFSET_Y,            XML_NAMESPACE_DRAW,     XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_PM_REPEAT_OFFSET_Y),

    { nullptr, 0, XML_EMPTY, 0, 0, SvtSaveOptions::ODFSVER_010, false }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
