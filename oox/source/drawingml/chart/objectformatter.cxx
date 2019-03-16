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

#include <drawingml/chart/objectformatter.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/lineproperties.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <oox/drawingml/theme.hxx>
#include <drawingml/chart/chartspacemodel.hxx>
#include <oox/helper/modelobjecthelper.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/properties.hxx>

namespace oox {
namespace drawingml {
namespace chart {

using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

using ::oox::core::XmlFilterBase;

namespace {

struct AutoFormatPatternEntry
{
    sal_Int32 const     mnColorToken;       /// Theme color token.
    sal_Int32 const     mnModToken;         /// Color modification token.
    sal_Int32 const     mnModValue;         /// Color modification value.
};

#define AUTOFORMAT_PATTERN_COLOR( color_token ) \
    { color_token, XML_TOKEN_INVALID, 0 }

#define AUTOFORMAT_PATTERN_COLORMOD( color_token, mod_token, mod_value ) \
    { color_token, mod_token, mod_value }

#define AUTOFORMAT_PATTERN_END() \
    AUTOFORMAT_PATTERN_COLOR( XML_TOKEN_INVALID )

static const AutoFormatPatternEntry spAutoFormatPattern1[] =
{
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 88500 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 55000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 78000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 92500 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 70000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 30000 ),
    AUTOFORMAT_PATTERN_END()
};

static const AutoFormatPatternEntry spAutoFormatPattern2[] =
{
    AUTOFORMAT_PATTERN_COLOR( XML_accent1 ),
    AUTOFORMAT_PATTERN_COLOR( XML_accent2 ),
    AUTOFORMAT_PATTERN_COLOR( XML_accent3 ),
    AUTOFORMAT_PATTERN_COLOR( XML_accent4 ),
    AUTOFORMAT_PATTERN_COLOR( XML_accent5 ),
    AUTOFORMAT_PATTERN_COLOR( XML_accent6 ),
    AUTOFORMAT_PATTERN_END()
};

static const AutoFormatPatternEntry spAutoFormatPattern3[] =
{
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent1, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent2, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent3, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent4, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent5, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_accent6, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN_END()
};

static const AutoFormatPatternEntry spAutoFormatPattern4[] =
{
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint,  5000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 55000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 78000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 15000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 70000 ),
    AUTOFORMAT_PATTERN_COLORMOD( XML_dk1, XML_tint, 30000 ),
    AUTOFORMAT_PATTERN_END()
};

#undef AUTOFORMAT_PATTERN_COLOR
#undef AUTOFORMAT_PATTERN_COLORMOD
#undef AUTOFORMAT_PATTERN_END

struct AutoFormatEntry
{
    sal_Int32 const     mnFirstStyleIdx;    /// First chart style index.
    sal_Int32 const     mnLastStyleIdx;     /// Last chart style index.
    sal_Int32 const     mnThemedIdx;        /// Themed style index.
    sal_Int32 const     mnColorToken;       /// Theme color token.
    sal_Int32 const     mnModToken;         /// Color modification token.
    sal_Int32 const     mnModValue;         /// Color modification value.
    sal_Int32 const     mnRelLineWidth;     /// Relative line width (percent).
    const AutoFormatPatternEntry* mpPattern;/// Color cycling pattern for data series.
    bool const          mbFadedColor;       /// True = Faded color for data series.
};

#define AUTOFORMAT_COLOR( first, last, themed_style, color_token ) \
    { first, last, themed_style, color_token, XML_TOKEN_INVALID, 0, 100, nullptr, false }

#define AUTOFORMAT_COLORMOD( first, last, themed_style, color_token, mod_token, mod_value ) \
    { first, last, themed_style, color_token, mod_token, mod_value, 100, nullptr, false }

#define AUTOFORMAT_ACCENTSMOD( first, themed_style, mod_token, mod_value ) \
    AUTOFORMAT_COLORMOD( first,     first,     themed_style, XML_accent1, mod_token, mod_value ), \
    AUTOFORMAT_COLORMOD( first + 1, first + 1, themed_style, XML_accent2, mod_token, mod_value ), \
    AUTOFORMAT_COLORMOD( first + 2, first + 2, themed_style, XML_accent3, mod_token, mod_value ), \
    AUTOFORMAT_COLORMOD( first + 3, first + 3, themed_style, XML_accent4, mod_token, mod_value ), \
    AUTOFORMAT_COLORMOD( first + 4, first + 4, themed_style, XML_accent5, mod_token, mod_value ), \
    AUTOFORMAT_COLORMOD( first + 5, first + 5, themed_style, XML_accent6, mod_token, mod_value )

#define AUTOFORMAT_PATTERN( first, last, themed_style, line_width, pattern ) \
    { first, last, themed_style, XML_TOKEN_INVALID, XML_TOKEN_INVALID, 0, line_width, pattern, false }

#define AUTOFORMAT_FADED( first, last, themed_style, color_token, line_width ) \
    { first, last, themed_style, color_token, XML_TOKEN_INVALID, 0, line_width, nullptr, true }

#define AUTOFORMAT_FADEDACCENTS( first, themed_style, line_width ) \
    AUTOFORMAT_FADED( first,     first,     themed_style, XML_accent1, line_width ), \
    AUTOFORMAT_FADED( first + 1, first + 1, themed_style, XML_accent2, line_width ), \
    AUTOFORMAT_FADED( first + 2, first + 2, themed_style, XML_accent3, line_width ), \
    AUTOFORMAT_FADED( first + 3, first + 3, themed_style, XML_accent4, line_width ), \
    AUTOFORMAT_FADED( first + 4, first + 4, themed_style, XML_accent5, line_width ), \
    AUTOFORMAT_FADED( first + 5, first + 5, themed_style, XML_accent6, line_width )

#define AUTOFORMAT_INVISIBLE( first, last ) \
    AUTOFORMAT_COLOR( first, last, -1, XML_TOKEN_INVALID )

#define AUTOFORMAT_END() \
    AUTOFORMAT_INVISIBLE( -1, -1 )

static const AutoFormatEntry spNoFormats[] =
{
    AUTOFORMAT_INVISIBLE( 1, 48 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spChartSpaceFill[] =
{
    AUTOFORMAT_COLOR( 1, 32, THEMED_STYLE_SUBTLE, XML_bg1 ),
    AUTOFORMAT_COLOR( 33, 40, THEMED_STYLE_SUBTLE, XML_lt1 ),
    AUTOFORMAT_COLOR( 41, 48, THEMED_STYLE_SUBTLE, XML_dk1 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spDataTableLines[] =
{
    AUTOFORMAT_COLORMOD(  1, 32, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 75000 ),
    AUTOFORMAT_COLORMOD( 33, 40, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 75000 ),
    // 41...48: no line, same as Chart2
    AUTOFORMAT_END()
};

static const AutoFormatEntry spPlotArea2dFills[] =
{
    AUTOFORMAT_COLOR(       1, 32, THEMED_STYLE_SUBTLE, XML_bg1 ),
    AUTOFORMAT_COLORMOD(   33, 34, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 20000 ),
    AUTOFORMAT_ACCENTSMOD( 35,     THEMED_STYLE_SUBTLE,          XML_tint, 20000 ), // tint not documented!?
    AUTOFORMAT_COLORMOD(   41, 48, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 95000 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spWallFloorLines[] =
{
    AUTOFORMAT_COLORMOD(  1, 32, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 75000 ),
    AUTOFORMAT_COLORMOD( 33, 40, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 75000 ),
    // 41...48: no line, same as Chart2
    AUTOFORMAT_END()
};

static const AutoFormatEntry spWallFloorFills[] =
{
    AUTOFORMAT_INVISIBLE(   1, 32 ),
    AUTOFORMAT_COLORMOD(   33, 34, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 20000 ),
    AUTOFORMAT_ACCENTSMOD( 35,     THEMED_STYLE_SUBTLE,          XML_tint, 20000 ), // tint not documented!?
    AUTOFORMAT_COLORMOD(   41, 48, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 95000 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spAxisLines[] =
{
    AUTOFORMAT_COLORMOD(  1, 32, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 75000 ),   // tint not documented!?
    AUTOFORMAT_COLORMOD( 33, 48, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 75000 ),   // tint not documented!?
    AUTOFORMAT_END()
};

static const AutoFormatEntry spMajorGridLines[] =
{
    AUTOFORMAT_COLORMOD(  1, 32, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 75000 ),   // tint not documented!?
    AUTOFORMAT_COLORMOD( 33, 48, THEMED_STYLE_SUBTLE, XML_dk1, XML_tint, 75000 ),   // tint not documented!?
    AUTOFORMAT_END()
};

static const AutoFormatEntry spMinorGridLines[] =
{
    AUTOFORMAT_COLORMOD(  1, 40, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 50000 ),
    AUTOFORMAT_COLORMOD( 41, 48, THEMED_STYLE_SUBTLE, XML_tx1, XML_tint, 90000 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spOtherLines[] =
{
    AUTOFORMAT_COLOR(     1, 32, THEMED_STYLE_SUBTLE, XML_tx1 ),
    AUTOFORMAT_COLOR(    33, 34, THEMED_STYLE_SUBTLE, XML_dk1 ),
    AUTOFORMAT_COLORMOD( 35, 40, THEMED_STYLE_SUBTLE, XML_dk1, XML_shade, 25000 ),
    AUTOFORMAT_COLOR(    41, 48, THEMED_STYLE_SUBTLE, XML_lt1 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spLinearSeriesLines[] =
{
    AUTOFORMAT_PATTERN(       1,  1, THEMED_STYLE_SUBTLE, 300, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(       2,  2, THEMED_STYLE_SUBTLE, 300, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS(  3,     THEMED_STYLE_SUBTLE, 300 ),
    AUTOFORMAT_PATTERN(       9,  9, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      10, 10, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 11,     THEMED_STYLE_SUBTLE, 500 ),
    AUTOFORMAT_PATTERN(      17, 17, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      18, 18, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 19,     THEMED_STYLE_SUBTLE, 500 ),
    AUTOFORMAT_PATTERN(      25, 25, THEMED_STYLE_SUBTLE, 700, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      26, 26, THEMED_STYLE_SUBTLE, 700, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 27,     THEMED_STYLE_SUBTLE, 700 ),
    AUTOFORMAT_PATTERN(      33, 33, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      34, 34, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 35,     THEMED_STYLE_SUBTLE, 500 ),
    AUTOFORMAT_PATTERN(      41, 42, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern4 ),
    AUTOFORMAT_PATTERN(      42, 42, THEMED_STYLE_SUBTLE, 500, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 43,     THEMED_STYLE_SUBTLE, 500 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spFilledSeriesLines[] =
{
    AUTOFORMAT_INVISIBLE(   1,  8 ),
    AUTOFORMAT_COLOR(       9, 16, THEMED_STYLE_SUBTLE, XML_lt1 ),
    AUTOFORMAT_INVISIBLE(  17, 32 ),
    AUTOFORMAT_COLORMOD(   33, 33, THEMED_STYLE_SUBTLE, XML_dk1, XML_shade, 50000 ),
    AUTOFORMAT_PATTERN(    34, 34, THEMED_STYLE_SUBTLE, 100, spAutoFormatPattern3 ),
    AUTOFORMAT_ACCENTSMOD( 35,     THEMED_STYLE_SUBTLE,          XML_shade, 50000 ),
    AUTOFORMAT_INVISIBLE(  41, 48 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spFilledSeries2dFills[] =
{
    AUTOFORMAT_PATTERN(       1,  1, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(       2,  2, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS(  3,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(       9,  9, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      10, 10, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 11,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      17, 17, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      18, 18, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 19,     THEMED_STYLE_INTENSE, 100 ),
    AUTOFORMAT_PATTERN(      25, 25, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      26, 26, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 27,     THEMED_STYLE_INTENSE, 100 ),
    AUTOFORMAT_PATTERN(      33, 33, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      34, 34, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 35,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      41, 42, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern4 ),
    AUTOFORMAT_PATTERN(      42, 42, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 43,     THEMED_STYLE_INTENSE, 100 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spFilledSeries3dFills[] =
{
    AUTOFORMAT_PATTERN(       1,  1, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(       2,  2, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS(  3,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(       9,  9, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      10, 10, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 11,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      17, 17, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      18, 18, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 19,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      25, 25, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      26, 26, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 27,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      33, 33, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern1 ),
    AUTOFORMAT_PATTERN(      34, 34, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 35,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_PATTERN(      41, 42, THEMED_STYLE_SUBTLE,  100, spAutoFormatPattern4 ),
    AUTOFORMAT_PATTERN(      42, 42, THEMED_STYLE_INTENSE, 100, spAutoFormatPattern2 ),
    AUTOFORMAT_FADEDACCENTS( 43,     THEMED_STYLE_SUBTLE,  100 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spUpDownBarLines[] =
{
    AUTOFORMAT_COLOR(       1, 16, THEMED_STYLE_SUBTLE, XML_tx1 ),
    AUTOFORMAT_INVISIBLE(  17, 32 ),
    AUTOFORMAT_COLOR(      33, 34, THEMED_STYLE_SUBTLE, XML_dk1 ),
    AUTOFORMAT_ACCENTSMOD( 35,     THEMED_STYLE_SUBTLE, XML_shade, 25000 ),
    AUTOFORMAT_INVISIBLE(  41, 48 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spUpBarFills[] =
{
    AUTOFORMAT_COLORMOD(    1,  1, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(    2,  2, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  5000 ),
    AUTOFORMAT_ACCENTSMOD(  3,     THEMED_STYLE_SUBTLE,           XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(    9,  9, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(   10, 10, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  5000 ),
    AUTOFORMAT_ACCENTSMOD( 11,     THEMED_STYLE_SUBTLE,           XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(   17, 17, THEMED_STYLE_INTENSE, XML_dk1, XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(   18, 18, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  5000 ),
    AUTOFORMAT_ACCENTSMOD( 19,     THEMED_STYLE_INTENSE,          XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(   25, 25, THEMED_STYLE_INTENSE, XML_dk1, XML_tint, 25000 ),
    AUTOFORMAT_COLORMOD(   26, 26, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  5000 ),
    AUTOFORMAT_ACCENTSMOD( 27,     THEMED_STYLE_INTENSE,          XML_tint, 25000 ),
    AUTOFORMAT_COLOR(      33, 40, THEMED_STYLE_SUBTLE,  XML_lt1 ),
    AUTOFORMAT_COLORMOD(   41, 41, THEMED_STYLE_INTENSE, XML_dk1, XML_tint, 25000 ),
    AUTOFORMAT_COLOR(      42, 42, THEMED_STYLE_INTENSE, XML_lt1 ),
    AUTOFORMAT_ACCENTSMOD( 43,     THEMED_STYLE_INTENSE,          XML_tint, 25000 ),
    AUTOFORMAT_END()
};

static const AutoFormatEntry spDownBarFills[] =
{
    AUTOFORMAT_COLORMOD(    1,  1, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLORMOD(    2,  2, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  95000 ),
    AUTOFORMAT_ACCENTSMOD(  3,     THEMED_STYLE_SUBTLE,           XML_shade, 25000 ),
    AUTOFORMAT_COLORMOD(    9,  9, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLORMOD(   10, 10, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  95000 ),
    AUTOFORMAT_ACCENTSMOD( 11,     THEMED_STYLE_SUBTLE,           XML_shade, 25000 ),
    AUTOFORMAT_COLORMOD(   17, 17, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLORMOD(   18, 18, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  95000 ),
    AUTOFORMAT_ACCENTSMOD( 19,     THEMED_STYLE_INTENSE,          XML_shade, 25000 ),
    AUTOFORMAT_COLORMOD(   25, 25, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLORMOD(   26, 26, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  95000 ),
    AUTOFORMAT_ACCENTSMOD( 27,     THEMED_STYLE_INTENSE,          XML_shade, 25000 ),
    AUTOFORMAT_COLORMOD(   33, 33, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLORMOD(   34, 34, THEMED_STYLE_SUBTLE,  XML_dk1, XML_tint,  95000 ),
    AUTOFORMAT_ACCENTSMOD( 35,     THEMED_STYLE_SUBTLE,           XML_shade, 25000 ),
    AUTOFORMAT_COLORMOD(   41, 41, THEMED_STYLE_INTENSE, XML_dk1, XML_tint,  85000 ),
    AUTOFORMAT_COLOR(      42, 42, THEMED_STYLE_INTENSE, XML_dk1 ),
    AUTOFORMAT_ACCENTSMOD( 43,     THEMED_STYLE_INTENSE,          XML_shade, 25000 ),
    AUTOFORMAT_END()
};

#undef AUTOFORMAT_COLOR
#undef AUTOFORMAT_COLORMOD
#undef AUTOFORMAT_ACCENTSMOD
#undef AUTOFORMAT_PATTERN
#undef AUTOFORMAT_FADED
#undef AUTOFORMAT_FADEDACCENTS
#undef AUTOFORMAT_INVISIBLE
#undef AUTOFORMAT_END

const AutoFormatEntry* lclGetAutoFormatEntry( const AutoFormatEntry* pEntries, sal_Int32 nStyle )
{
    for( ; pEntries && (pEntries->mnFirstStyleIdx >= 0); ++pEntries )
        if( (pEntries->mnFirstStyleIdx <= nStyle) && (nStyle <= pEntries->mnLastStyleIdx) )
            return pEntries;
    return nullptr;
}

struct AutoTextEntry
{
    sal_Int32 const     mnFirstStyleIdx;    /// First chart style index.
    sal_Int32 const     mnLastStyleIdx;     /// Last chart style index.
    sal_Int32 const     mnThemedFont;       /// Themed font (minor/major).
    sal_Int32 const     mnColorToken;       /// Theme color token.
    sal_Int32 const     mnDefFontSize;      /// Default font size (1/100 points).
    sal_Int32 const     mnRelFontSize;      /// Font size relative to chart global font (percent).
    bool const          mbBold;             /// True = bold font.
};

#define AUTOTEXT_COLOR( first, last, themed_font, color_token, def_font_size, rel_font_size, bold ) \
    { first, last, themed_font, color_token, def_font_size, rel_font_size, bold }

#define AUTOTEXT_END() \
    AUTOTEXT_COLOR( -1, -1, XML_none, XML_TOKEN_INVALID, 1000, 100, false )

static const AutoTextEntry spChartTitleTexts[] =
{
    AUTOTEXT_COLOR(  1, 40, XML_minor, XML_tx1, 1800, 120, true ),
    AUTOTEXT_COLOR( 41, 48, XML_minor, XML_lt1, 1800, 120, true ),
    AUTOTEXT_END()
};

static const AutoTextEntry spAxisTitleTexts[] =
{
    AUTOTEXT_COLOR(  1, 40, XML_minor, XML_tx1, 1000, 100, true ),
    AUTOTEXT_COLOR( 41, 48, XML_minor, XML_lt1, 1000, 100, true ),
    AUTOTEXT_END()
};

static const AutoTextEntry spOtherTexts[] =
{
    AUTOTEXT_COLOR(  1, 40, XML_minor, XML_tx1, 1000, 100, false ),
    AUTOTEXT_COLOR( 41, 48, XML_minor, XML_lt1, 1000, 100, false ),
    AUTOTEXT_END()
};

#undef AUTOTEXT_COLOR
#undef AUTOTEXT_END

const AutoTextEntry* lclGetAutoTextEntry( const AutoTextEntry* pEntries, sal_Int32 nStyle )
{
    for( ; pEntries && (pEntries->mnFirstStyleIdx >= 0); ++pEntries )
        if( (pEntries->mnFirstStyleIdx <= nStyle) && (nStyle <= pEntries->mnLastStyleIdx) )
            return pEntries;
    return nullptr;
}

// These PropIds arrays will be indexed into using a ShapeProperty enum

static const ShapePropertyIds spnCommonPropIds =
{
    PROP_LineStyle, PROP_LineWidth, PROP_LineColor, PROP_LineTransparence, PROP_LineDashName,
    PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID,
    PROP_FillStyle, PROP_FillColor, PROP_FillTransparence, PROP_INVALID, PROP_FillGradientName,
    PROP_FillBitmapName, PROP_FillBitmapMode, PROP_FillBitmapSizeX, PROP_FillBitmapSizeY,
    PROP_FillBitmapPositionOffsetX, PROP_FillBitmapPositionOffsetY, PROP_FillBitmapRectanglePoint,
    PROP_FillHatchName, PROP_FillBackground
};

static const ShapePropertyIds spnLinearPropIds =
{
    PROP_LineStyle, PROP_LineWidth, PROP_Color, PROP_Transparency, PROP_LineDashName,
    PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID,
    PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID,
    PROP_INVALID, PROP_INVALID, PROP_INVALID, PROP_INVALID,
    PROP_INVALID, PROP_INVALID, PROP_INVALID,
    PROP_INVALID, PROP_INVALID
};

static const ShapePropertyIds spnFilledPropIds =
{
    PROP_BorderStyle,
    PROP_BorderWidth,
    PROP_BorderColor,
    PROP_BorderTransparency,
    PROP_BorderDashName,
    PROP_INVALID,
    PROP_INVALID,
    PROP_INVALID,
    PROP_INVALID,
    PROP_INVALID,
    PROP_INVALID,
    PROP_INVALID,
    PROP_FillStyle,
    PROP_Color,
    PROP_Transparency,
    PROP_FillTransparenceGradientName,
    PROP_GradientName,
    PROP_FillBitmapName,
    PROP_FillBitmapMode,
    PROP_FillBitmapSizeX,
    PROP_FillBitmapSizeY,
    PROP_FillBitmapPositionOffsetX,
    PROP_FillBitmapPositionOffsetY,
    PROP_FillBitmapRectanglePoint,
    PROP_HatchName,
    PROP_FillBackground
};

/** Property info for common chart objects, to be used in ShapePropertyMap. */
static const ShapePropertyInfo saCommonPropInfo( spnCommonPropIds, false, true, true, true, true );
/** Property info for linear data series, to be used in ShapePropertyMap. */
static const ShapePropertyInfo saLinearPropInfo( spnLinearPropIds, false, true, true, true, true );
/** Property info for filled data series, to be used in ShapePropertyMap. */
static const ShapePropertyInfo saFilledPropInfo( spnFilledPropIds, false, true, true, true, true );

/** Contains information about formatting of a specific chart object type. */
struct ObjectTypeFormatEntry
{
    ObjectType          meObjType;          /// Object type for automatic format.
    const ShapePropertyInfo& mrPropInfo;    /// Property info for the ShapePropertyMap class.
    const AutoFormatEntry* mpAutoLines;     /// Automatic line formatting for all chart styles.
    const AutoFormatEntry* mpAutoFills;     /// Automatic fill formatting for all chart styles.
    const AutoTextEntry* mpAutoTexts;       /// Automatic text attributes for all chart styles.
    bool const          mbIsFrame;          /// True = object is a frame, false = object is a line.
    ObjectTypeFormatEntry(ObjectType eObjType, const ShapePropertyInfo& rPropInfo,
                                 const AutoFormatEntry* pAutoLines,
                                 const AutoFormatEntry* pAutoFills,
                                 const AutoTextEntry* pAutoTexts,
                                 bool bIsFrame)
        :meObjType(eObjType), mrPropInfo(rPropInfo), mpAutoLines(pAutoLines)
        ,mpAutoFills(pAutoFills), mpAutoTexts(pAutoTexts), mbIsFrame(bIsFrame)
    {} // prevent creation of implicit default ctor which fails in MSVC
};

#define TYPEFORMAT_FRAME( obj_type, prop_type, auto_texts, auto_lines, auto_fills ) \
    { obj_type, prop_type, auto_lines, auto_fills, auto_texts, true }

#define TYPEFORMAT_LINE( obj_type, prop_type, auto_texts, auto_lines ) \
    { obj_type, prop_type, auto_lines, nullptr,  auto_texts, false }

static const ObjectTypeFormatEntry spObjTypeFormatEntries[] =
{
    //                object type                property info      auto text          auto line            auto fill
    TYPEFORMAT_FRAME( OBJECTTYPE_CHARTSPACE,     saCommonPropInfo, nullptr,                 spNoFormats,         spChartSpaceFill ),
    TYPEFORMAT_FRAME( OBJECTTYPE_CHARTTITLE,     saCommonPropInfo, spChartTitleTexts, nullptr /* eq to Ch2 */,   nullptr /* eq to Ch2 */),
    TYPEFORMAT_FRAME( OBJECTTYPE_LEGEND,         saCommonPropInfo, spOtherTexts,      spNoFormats,         spNoFormats ),
    TYPEFORMAT_FRAME( OBJECTTYPE_PLOTAREA2D,     saCommonPropInfo, nullptr,                 nullptr /* eq to Ch2 */,   spPlotArea2dFills ),
    TYPEFORMAT_FRAME( OBJECTTYPE_PLOTAREA3D,     saCommonPropInfo, nullptr,                 nullptr /* eq to Ch2 */,   nullptr /* eq to Ch2 */ ),
    TYPEFORMAT_FRAME( OBJECTTYPE_WALL,           saCommonPropInfo, nullptr,                 spWallFloorLines,    spWallFloorFills ),
    TYPEFORMAT_FRAME( OBJECTTYPE_FLOOR,          saCommonPropInfo, nullptr,                 spWallFloorLines,    spWallFloorFills ),
    TYPEFORMAT_LINE(  OBJECTTYPE_AXIS,           saCommonPropInfo, spOtherTexts,      spAxisLines ),
    TYPEFORMAT_FRAME( OBJECTTYPE_AXISTITLE,      saCommonPropInfo, spAxisTitleTexts,  nullptr /* eq to Ch2 */,   nullptr /* eq to Ch2 */ ),
    TYPEFORMAT_FRAME( OBJECTTYPE_AXISUNIT,       saCommonPropInfo, spAxisTitleTexts,  nullptr /* eq in Ch2 */,   nullptr /* eq in Ch2 */ ),
    TYPEFORMAT_LINE(  OBJECTTYPE_MAJORGRIDLINE,  saCommonPropInfo, nullptr,                 spMajorGridLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_MINORGRIDLINE,  saCommonPropInfo, nullptr,                 spMinorGridLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_LINEARSERIES2D, saLinearPropInfo, nullptr,                 spLinearSeriesLines ),
    TYPEFORMAT_FRAME( OBJECTTYPE_FILLEDSERIES2D, saFilledPropInfo, nullptr,                 spFilledSeriesLines, spFilledSeries2dFills ),
    TYPEFORMAT_FRAME( OBJECTTYPE_FILLEDSERIES3D, saFilledPropInfo, nullptr,                 spFilledSeriesLines, spFilledSeries3dFills ),
    TYPEFORMAT_FRAME( OBJECTTYPE_DATALABEL,      saCommonPropInfo, spOtherTexts,      nullptr /* eq to Ch2 */,   nullptr /* eq to Ch2 */ ),
    TYPEFORMAT_LINE(  OBJECTTYPE_TRENDLINE,      saCommonPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_FRAME( OBJECTTYPE_TRENDLINELABEL, saCommonPropInfo, spOtherTexts,      nullptr /* eq to Ch2 */,   nullptr /* eq to Ch2 */ ),
    TYPEFORMAT_LINE(  OBJECTTYPE_ERRORBAR,       saCommonPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_SERLINE,        saCommonPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_LEADERLINE,     saCommonPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_DROPLINE,       saCommonPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_LINE(  OBJECTTYPE_HILOLINE,       saLinearPropInfo, nullptr,                 spOtherLines ),
    TYPEFORMAT_FRAME( OBJECTTYPE_UPBAR,          saCommonPropInfo, nullptr,                 spUpDownBarLines,    spUpBarFills ),
    TYPEFORMAT_FRAME( OBJECTTYPE_DOWNBAR,        saCommonPropInfo, nullptr,                 spUpDownBarLines,    spDownBarFills ),
    TYPEFORMAT_LINE(  OBJECTTYPE_DATATABLE,      saCommonPropInfo, spOtherTexts,      spDataTableLines )
};

#undef TYPEFORMAT_FRAME
#undef TYPEFORMAT_LINE

void lclConvertPictureOptions( FillProperties& orFillProps, const PictureOptionsModel& rPicOptions )
{
    bool bStacked = (rPicOptions.mnPictureFormat == XML_stack) || (rPicOptions.mnPictureFormat == XML_stackScale);
    orFillProps.maBlipProps.moBitmapMode = bStacked ? XML_tile : XML_stretch;
}

} // namespace

struct ObjectFormatterData;

class DetailFormatterBase
{
public:
    explicit            DetailFormatterBase(
                            ObjectFormatterData& rData,
                            const AutoFormatEntry* pAutoFormatEntry );
    explicit            DetailFormatterBase(
                            ObjectFormatterData& rData,
                            const AutoTextEntry* pAutoTextEntry );

protected:
    /** Returns the placeholder color which may depend on the passed series index. */
    ::Color         getPhColor( sal_Int32 nSeriesIdx ) const;

private:
    /** Resolves and returns the scheme color with the passed transformation. */
    ::Color         getSchemeColor( sal_Int32 nColorToken, sal_Int32 nModToken, sal_Int32 nModValue ) const;

protected:
    typedef ::std::vector< ::Color > ColorPatternVec;

    ObjectFormatterData& mrData;            /// Shared formatter data.
    ::Color             mnPhClr;            /// RGB placeholder color for themed style.
    ColorPatternVec     maColorPattern;     /// Different cycling colors for data series.
};

class LineFormatter : public DetailFormatterBase
{
public:
    explicit            LineFormatter(
                            ObjectFormatterData& rData,
                            const AutoFormatEntry* pAutoFormatEntry,
                            const ObjectType eObjType );

    /** Converts line formatting to the passed property set. */
    void                convertFormatting(
                            ShapePropertyMap& rPropMap,
                            const ModelRef< Shape >& rxShapeProp,
                            sal_Int32 nSeriesIdx );

private:
    LinePropertiesPtr   mxAutoLine;         /// Automatic line properties.
};

class FillFormatter : public DetailFormatterBase
{
public:
    explicit            FillFormatter(
                            ObjectFormatterData& rData,
                            const AutoFormatEntry* pAutoFormatEntry,
                            const ObjectType eObjType );

    /** Converts area formatting to the passed property set. */
    void                convertFormatting(
                            ShapePropertyMap& rPropMap,
                            const ModelRef< Shape >& rxShapeProp,
                            const PictureOptionsModel* pPicOptions,
                            sal_Int32 nSeriesIdx );

private:
    FillPropertiesPtr   mxAutoFill;         /// Automatic fill properties.
};

class TextFormatter : public DetailFormatterBase
{
public:
    explicit            TextFormatter(
                            ObjectFormatterData& rData,
                            const AutoTextEntry* pAutoTextEntry,
                            const ModelRef< TextBody >& rxGlobalTextProp );

    /** Converts text formatting to the passed property set. */
    void                convertFormatting(
                            PropertySet& rPropSet,
                            const TextCharacterProperties* pTextProps );
    /** Converts text formatting to the passed property set. */
    void                convertFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp );

private:
    TextCharacterPropertiesPtr mxAutoText;  /// Automatic text properties.
};

/** Formatter for a specific object type. */
class ObjectTypeFormatter
{
public:
    explicit            ObjectTypeFormatter(
                            ObjectFormatterData& rData,
                            const ObjectTypeFormatEntry& rEntry,
                            const ChartSpaceModel& rChartSpace,
                            const ObjectType eObjType );

    /** Sets frame formatting properties to the passed property set. */
    void                convertFrameFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< Shape >& rxShapeProp,
                            const PictureOptionsModel* pPicOptions,
                            sal_Int32 nSeriesIdx );

    /** Sets text formatting properties to the passed property set. */
    void                convertTextFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp );

    /** Sets frame/text formatting properties to the passed property set. */
    void                convertFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< Shape >& rxShapeProp,
                            const ModelRef< TextBody >& rxTextProp );

    /** Sets text formatting properties to the passed property set. */
    void                convertTextFormatting(
                            PropertySet& rPropSet,
                            const TextCharacterProperties& rTextProps );

    /** Sets automatic fill properties to the passed property set. */
    void                convertAutomaticFill(
                            PropertySet& rPropSet,
                            sal_Int32 nSeriesIdx );

private:
    LineFormatter       maLineFormatter;    /// Converter for line formatting.
    FillFormatter       maFillFormatter;    /// Converter for fill formatting.
    TextFormatter       maTextFormatter;    /// Converter for text formatting.
    ModelObjectHelper&  mrModelObjHelper;   /// Helper for named drawing formatting.
    const ObjectTypeFormatEntry& mrEntry;   /// Additional settings.
};

struct ObjectFormatterData
{
    typedef RefMap< ObjectType, ObjectTypeFormatter > ObjectTypeFormatterMap;

    const XmlFilterBase& mrFilter;              /// Base filter object.
    ObjectTypeFormatterMap maTypeFormatters;    /// Formatters for all types of objects in a chart.
    ModelObjectHelper   maModelObjHelper;       /// Helper for named drawing formatting (dashes, gradients, bitmaps).
    Reference< XNumberFormats > mxNumFmts;      /// Number formats collection of container document.
    Reference< XNumberFormatTypes > mxNumTypes; /// Number format types collection of container document.
    Locale const        maEnUsLocale;           /// Locale struct containing en-US.
    Locale const        maFromLocale;           /// Empty locale struct.
    sal_Int32           mnMaxSeriesIdx;         /// Maximum series index used for color cycling/fading.

    explicit            ObjectFormatterData(
                            const XmlFilterBase& rFilter,
                            const Reference< XChartDocument >& rxChartDoc,
                            const ChartSpaceModel& rChartSpace );

    ObjectTypeFormatter* getTypeFormatter( ObjectType eObjType );
};

DetailFormatterBase::DetailFormatterBase( ObjectFormatterData& rData, const AutoFormatEntry* pAutoFormatEntry ) :
    mrData( rData ),
    mnPhClr( 0xffffffff )
{
    if( pAutoFormatEntry )
    {
        if( pAutoFormatEntry->mpPattern )
        {
            // prepare multi-color pattern
            for( const AutoFormatPatternEntry* pPatternEntry = pAutoFormatEntry->mpPattern; pPatternEntry->mnColorToken != XML_TOKEN_INVALID; ++pPatternEntry )
                maColorPattern.push_back( getSchemeColor( pPatternEntry->mnColorToken, pPatternEntry->mnModToken, pPatternEntry->mnModValue ) );
        }
        else if( pAutoFormatEntry->mnColorToken != XML_TOKEN_INVALID )
        {
            // prepare color or single-color pattern (color fading)
            mnPhClr = getSchemeColor( pAutoFormatEntry->mnColorToken, pAutoFormatEntry->mnModToken, pAutoFormatEntry->mnModValue );
            if( pAutoFormatEntry->mbFadedColor )
                maColorPattern.push_back( mnPhClr );
        }
    }
}

DetailFormatterBase::DetailFormatterBase( ObjectFormatterData& rData, const AutoTextEntry* pAutoTextEntry ) :
    mrData( rData ),
    mnPhClr( 0xffffffff )
{
    if( pAutoTextEntry && (pAutoTextEntry->mnColorToken != XML_TOKEN_INVALID) )
        mnPhClr = getSchemeColor( pAutoTextEntry->mnColorToken, XML_TOKEN_INVALID, 0 );
}

::Color DetailFormatterBase::getPhColor( sal_Int32 nSeriesIdx ) const
{
    if( maColorPattern.empty() || (mrData.mnMaxSeriesIdx < 0) || (nSeriesIdx < 0) )
        return mnPhClr;

    /*  Apply tint/shade depending on the cycle index. The colors of leading
        series are darkened (color shade), the colors of trailing series are
        lightened (color tint). Shade/tint is applied in an exclusive range of
        -70% to 70%.

        Example 1: 3 data series using single-color shading with accent color 1
        (e.g. automatic chart style #3). Shade/tint is applied per series.
        Shade/tint changes in steps of 140%/(<series_count+1) = 140%/4 = 35%,
        starting at -70%:
            Step 1: -70% -> Not used.
            Step 2: -35% -> Series 1 has 35% shade of accent color 1.
            Step 3:   0% -> Series 2 has pure accent color 1.
            Step 4:  35% -> Series 3 has 35% tint of accent color 1.
            Step 5:  70% -> Not used.

        Example 2: 20 data series using accent color pattern (e.g. automatic
        chart style #2). Each color cycle has a size of 6 series (accent colors
        1 to 6). Shade/tint is applied per color cycle.
            Cycle #1: Series 1...6 are based on accent colors 1 to 6.
            Cycle #2: Series 7...12 are based on accent colors 1 to 6.
            Cycle #3: Series 13...18 are based on accent colors 1 to 6.
            Cycle #4: Series 19...20 are based on accent colors 1 to 2.
        Shade/tint changes in steps of 140%/(cycle_count+1) = 140%/5 = 28%,
        starting at -70%:
            Step 1: -70% -> Not used.
            Step 2: -42% -> Cycle #1 has 42% shade of accent colors 1...6
            step 3: -14% -> Cycle #2 has 14% shade of accent colors 1...6
            step 4:  14% -> Cycle #3 has 14% tint of accent colors 1...6
            step 5:  42% -> Cycle #4 has 42% tint of accent colors 1...6
            step 6:  70% -> Not used.
     */
    ::Color nPhClr = maColorPattern[ static_cast< size_t >( nSeriesIdx % maColorPattern.size() ) ];
    size_t nCycleIdx = static_cast< size_t >( nSeriesIdx / maColorPattern.size() );
    size_t nMaxCycleIdx = static_cast< size_t >( mrData.mnMaxSeriesIdx / maColorPattern.size() );
    double fShadeTint = static_cast< double >( nCycleIdx + 1 ) / (nMaxCycleIdx + 2) * 1.4 - 0.7;
    if( fShadeTint != 0.0 )
    {
        Color aColor;
        aColor.setSrgbClr( nPhClr );
        aColor.addChartTintTransformation( fShadeTint );
        nPhClr = aColor.getColor( mrData.mrFilter.getGraphicHelper() );
    }

    return nPhClr;
}

::Color DetailFormatterBase::getSchemeColor( sal_Int32 nColorToken, sal_Int32 nModToken, sal_Int32 nModValue ) const
{
    Color aColor;
    aColor.setSchemeClr( nColorToken );
    if( nModToken != XML_TOKEN_INVALID )
        aColor.addTransformation( nModToken, nModValue );
    return aColor.getColor( mrData.mrFilter.getGraphicHelper() );
}

LineFormatter::LineFormatter( ObjectFormatterData& rData, const AutoFormatEntry* pAutoFormatEntry, const ObjectType eObjType ) :
   DetailFormatterBase(rData, pAutoFormatEntry)
{
    if( pAutoFormatEntry )
    {
        mxAutoLine.reset( new LineProperties );
        mxAutoLine->maLineFill.moFillType = XML_noFill;
        if( const Theme* pTheme = mrData.mrFilter.getCurrentTheme() )
            if( const LineProperties* pLineProps = pTheme->getLineStyle( pAutoFormatEntry->mnThemedIdx ) )
                *mxAutoLine = *pLineProps;
        // set automatic border property for chartarea, because of tdf#81437 and tdf#82217
        if ( eObjType == OBJECTTYPE_CHARTSPACE )
        {
            mxAutoLine->maLineFill.moFillType = rData.mrFilter.getGraphicHelper().getDefaultChartAreaLineStyle();
            mxAutoLine->moLineWidth = rData.mrFilter.getGraphicHelper().getDefaultChartAreaLineWidth();
            // this value is what MSO 2016 use as a default color for chartspace border
            mxAutoLine->maLineFill.maFillColor.setSrgbClr( 0xD9D9D9 );
        }
        // change line width according to chart auto style
        if( mxAutoLine->moLineWidth.has() )
            mxAutoLine->moLineWidth = mxAutoLine->moLineWidth.get() * pAutoFormatEntry->mnRelLineWidth / 100;
    }
}

void LineFormatter::convertFormatting( ShapePropertyMap& rPropMap, const ModelRef< Shape >& rxShapeProp, sal_Int32 nSeriesIdx )
{
    LineProperties aLineProps;
    if( mxAutoLine.get() )
        aLineProps.assignUsed( *mxAutoLine );
    if( rxShapeProp.is() )
        aLineProps.assignUsed( rxShapeProp->getLineProperties() );
    aLineProps.pushToPropMap( rPropMap, mrData.mrFilter.getGraphicHelper(), getPhColor( nSeriesIdx ) );
}

FillFormatter::FillFormatter( ObjectFormatterData& rData, const AutoFormatEntry* pAutoFormatEntry, const ObjectType eObjType ) :
    DetailFormatterBase( rData, pAutoFormatEntry )
{
    if( pAutoFormatEntry )
    {
        mxAutoFill.reset( new FillProperties );
        if( eObjType != OBJECTTYPE_CHARTSPACE )
            mxAutoFill->moFillType = XML_noFill;
        if( const Theme* pTheme = mrData.mrFilter.getCurrentTheme() )
            if( const FillProperties* pFillProps = pTheme->getFillStyle( pAutoFormatEntry->mnThemedIdx ) )
                *mxAutoFill = *pFillProps;

        if (eObjType == OBJECTTYPE_CHARTSPACE)
        {
            mxAutoFill->moFillType = rData.mrFilter.getGraphicHelper().getDefaultChartAreaFillStyle();
        }
    }
}

void FillFormatter::convertFormatting( ShapePropertyMap& rPropMap, const ModelRef< Shape >& rxShapeProp, const PictureOptionsModel* pPicOptions, sal_Int32 nSeriesIdx )
{
    FillProperties aFillProps;
    if( mxAutoFill.get() )
        aFillProps.assignUsed( *mxAutoFill );
    if( rxShapeProp.is() )
        aFillProps.assignUsed( rxShapeProp->getFillProperties() );
    if( pPicOptions )
        lclConvertPictureOptions( aFillProps, *pPicOptions );
    aFillProps.pushToPropMap( rPropMap, mrData.mrFilter.getGraphicHelper(), 0, getPhColor( nSeriesIdx ) );
}

namespace {

const TextCharacterProperties* lclGetTextProperties( const ModelRef< TextBody >& rxTextProp )
{
    return (rxTextProp.is() && !rxTextProp->getParagraphs().empty()) ?
        &rxTextProp->getParagraphs().front()->getProperties().getTextCharacterProperties() : nullptr;
}

} // namespace

TextFormatter::TextFormatter( ObjectFormatterData& rData, const AutoTextEntry* pAutoTextEntry, const ModelRef< TextBody >& rxGlobalTextProp ) :
    DetailFormatterBase( rData, pAutoTextEntry )
{
    if( pAutoTextEntry )
    {
        mxAutoText.reset( new TextCharacterProperties );
        if( const Theme* pTheme = mrData.mrFilter.getCurrentTheme() )
            if( const TextCharacterProperties* pTextProps = pTheme->getFontStyle( pAutoTextEntry->mnThemedFont ) )
                *mxAutoText = *pTextProps;
        ::Color nTextColor = getPhColor( -1 );
        if( sal_Int32(nTextColor) >= 0 ) {
            mxAutoText->maFillProperties.maFillColor.setSrgbClr( nTextColor );
            mxAutoText->maFillProperties.moFillType.set(XML_solidFill);
        }
        mxAutoText->moHeight = pAutoTextEntry->mnDefFontSize;
        mxAutoText->moBold = pAutoTextEntry->mbBold;

        if( const TextCharacterProperties* pTextProps = lclGetTextProperties( rxGlobalTextProp ) )
        {
            mxAutoText->assignUsed( *pTextProps );
            if( pTextProps->moHeight.has() )
                mxAutoText->moHeight = pTextProps->moHeight.get() * pAutoTextEntry->mnRelFontSize / 100;
        }
    }
}

void TextFormatter::convertFormatting( PropertySet& rPropSet, const TextCharacterProperties* pTextProps )
{
    TextCharacterProperties aTextProps;
    if( mxAutoText.get() )
        aTextProps.assignUsed( *mxAutoText );
    if( pTextProps )
        aTextProps.assignUsed( *pTextProps );
    aTextProps.pushToPropSet( rPropSet, mrData.mrFilter );
}

void TextFormatter::convertFormatting( PropertySet& rPropSet, const ModelRef< TextBody >& rxTextProp )
{
    convertFormatting( rPropSet, lclGetTextProperties( rxTextProp ) );
}

ObjectTypeFormatter::ObjectTypeFormatter( ObjectFormatterData& rData, const ObjectTypeFormatEntry& rEntry, const ChartSpaceModel& rChartSpace, const ObjectType eObjType ) :
    maLineFormatter(   rData, lclGetAutoFormatEntry( rEntry.mpAutoLines,   rChartSpace.mnStyle ), eObjType ),
    maFillFormatter(   rData, lclGetAutoFormatEntry( rEntry.mpAutoFills,   rChartSpace.mnStyle ), eObjType ),
    maTextFormatter(   rData, lclGetAutoTextEntry(   rEntry.mpAutoTexts,   rChartSpace.mnStyle ), rChartSpace.mxTextProp ),
    mrModelObjHelper( rData.maModelObjHelper ),
    mrEntry( rEntry )
{
}

void ObjectTypeFormatter::convertFrameFormatting( PropertySet& rPropSet, const ModelRef< Shape >& rxShapeProp, const PictureOptionsModel* pPicOptions, sal_Int32 nSeriesIdx )
{
    ShapePropertyMap aPropMap( mrModelObjHelper, mrEntry.mrPropInfo );
    maLineFormatter.convertFormatting( aPropMap, rxShapeProp, nSeriesIdx );
    if( mrEntry.mbIsFrame )
        maFillFormatter.convertFormatting( aPropMap, rxShapeProp, pPicOptions, nSeriesIdx );
    rPropSet.setProperties( aPropMap );
}

void ObjectTypeFormatter::convertTextFormatting( PropertySet& rPropSet, const ModelRef< TextBody >& rxTextProp )
{
    maTextFormatter.convertFormatting( rPropSet, rxTextProp );
}

void ObjectTypeFormatter::convertFormatting( PropertySet& rPropSet, const ModelRef< Shape >& rxShapeProp, const ModelRef< TextBody >& rxTextProp )
{
    convertFrameFormatting( rPropSet, rxShapeProp, nullptr, -1 );
    convertTextFormatting( rPropSet, rxTextProp );
}

void ObjectTypeFormatter::convertTextFormatting( PropertySet& rPropSet, const TextCharacterProperties& rTextProps )
{
    maTextFormatter.convertFormatting( rPropSet, &rTextProps );
}

void ObjectTypeFormatter::convertAutomaticFill( PropertySet& rPropSet, sal_Int32 nSeriesIdx )
{
    ShapePropertyMap aPropMap( mrModelObjHelper, mrEntry.mrPropInfo );
    ModelRef< Shape > xShapeProp;
    maFillFormatter.convertFormatting( aPropMap, xShapeProp, nullptr, nSeriesIdx );
    rPropSet.setProperties( aPropMap );
}

ObjectFormatterData::ObjectFormatterData( const XmlFilterBase& rFilter, const Reference< XChartDocument >& rxChartDoc, const ChartSpaceModel& rChartSpace ) :
    mrFilter( rFilter ),
    maModelObjHelper( Reference< XMultiServiceFactory >( rxChartDoc, UNO_QUERY ) ),
    maEnUsLocale( "en", "US", OUString() ),
    mnMaxSeriesIdx( -1 )
{
    for(auto const &rEntry : spObjTypeFormatEntries)
        maTypeFormatters[ rEntry.meObjType ].reset( new ObjectTypeFormatter( *this, rEntry, rChartSpace, rEntry.meObjType ) );

    try
    {
        Reference< XNumberFormatsSupplier > xNumFmtsSupp( rxChartDoc, UNO_QUERY_THROW );
        mxNumFmts = xNumFmtsSupp->getNumberFormats();
        mxNumTypes.set( mxNumFmts, UNO_QUERY );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxNumFmts.is() && mxNumTypes.is(), "ObjectFormatterData::ObjectFormatterData - cannot get number formats" );
}

ObjectTypeFormatter* ObjectFormatterData::getTypeFormatter( ObjectType eObjType )
{
    OSL_ENSURE( maTypeFormatters.has( eObjType ), "ObjectFormatterData::getTypeFormatter - unknown object type" );
    return maTypeFormatters.get( eObjType ).get();
}

ObjectFormatter::ObjectFormatter( const XmlFilterBase& rFilter, const Reference< XChartDocument >& rxChartDoc, const ChartSpaceModel& rChartSpace ) :
    mxData( new ObjectFormatterData( rFilter, rxChartDoc, rChartSpace ) )
{
}

ObjectFormatter::~ObjectFormatter()
{
}

void ObjectFormatter::setMaxSeriesIndex( sal_Int32 nMaxSeriesIdx )
{
    mxData->mnMaxSeriesIdx = nMaxSeriesIdx;
}

sal_Int32 ObjectFormatter::getMaxSeriesIndex() const
{
    return mxData->mnMaxSeriesIdx;
}

void ObjectFormatter::convertFrameFormatting( PropertySet& rPropSet, const ModelRef< Shape >& rxShapeProp, ObjectType eObjType, sal_Int32 nSeriesIdx )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertFrameFormatting( rPropSet, rxShapeProp, nullptr, nSeriesIdx );
}

void ObjectFormatter::convertFrameFormatting( PropertySet& rPropSet, const ModelRef< Shape >& rxShapeProp, const PictureOptionsModel& rPicOptions, ObjectType eObjType, sal_Int32 nSeriesIdx )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertFrameFormatting( rPropSet, rxShapeProp, &rPicOptions, nSeriesIdx );
}

void ObjectFormatter::convertTextFormatting( PropertySet& rPropSet, const ModelRef< TextBody >& rxTextProp, ObjectType eObjType )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertTextFormatting( rPropSet, rxTextProp );
}

void ObjectFormatter::convertFormatting( PropertySet& rPropSet, const ModelRef< Shape >& rxShapeProp, const ModelRef< TextBody >& rxTextProp, ObjectType eObjType )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertFormatting( rPropSet, rxShapeProp, rxTextProp );
}

void ObjectFormatter::convertTextFormatting( PropertySet& rPropSet, const TextCharacterProperties& rTextProps, ObjectType eObjType )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertTextFormatting( rPropSet, rTextProps );
}

void ObjectFormatter::convertTextRotation( PropertySet& rPropSet, const ModelRef< TextBody >& rxTextProp, bool bSupportsStacked, sal_Int32 nDefaultRotation )
{
    if( rxTextProp.is() )
    {
        bool bStacked = false;
        if( bSupportsStacked )
        {
            sal_Int32 nVert = rxTextProp->getTextProperties().moVert.get( XML_horz );
            bStacked = (nVert == XML_wordArtVert) || (nVert == XML_wordArtVertRtl);
            rPropSet.setProperty( PROP_StackCharacters, bStacked );
        }

        /*  Chart2 expects rotation angle as double value in range of [0,360).
            OOXML counts clockwise, Chart2 counts counterclockwise. */
        double fAngle = static_cast< double >( bStacked ? 0 : rxTextProp->getTextProperties().moRotation.get( nDefaultRotation ) );
        // MS Office UI allows values only in range of [-90,90].
        if ( fAngle < -5400000.0 || fAngle > 5400000.0 )
        {
            fAngle = 0.0;
        }
        fAngle = getDoubleIntervalValue< double >( -fAngle / 60000.0, 0.0, 360.0 );
        rPropSet.setProperty( PROP_TextRotation, fAngle );
    }
}

void ObjectFormatter::convertTextWrap( PropertySet& rPropSet, const ModelRef< TextBody >& rxTextProp )
{
    if( rxTextProp.is() )
    {
        PropertyMap& aPropMap = rxTextProp->getTextProperties().maPropertyMap;
        if( aPropMap.hasProperty(PROP_TextWordWrap) )
        {
            Any aValue = aPropMap.getProperty( PROP_TextWordWrap );
            if( aValue.hasValue() )
            {
                bool bValue = false;
                aValue >>= bValue;
                rPropSet.setProperty( PROP_TextWordWrap, bValue );
            }
        }
    }
}

void ObjectFormatter::convertNumberFormat( PropertySet& rPropSet, const NumberFormat& rNumberFormat, bool bAxis, bool bShowPercent )
{
    if( mxData->mxNumFmts.is() )
    {
        const bool bGeneral = rNumberFormat.maFormatCode.equalsIgnoreAsciiCase("general");
        const bool bPercent = !bAxis && bShowPercent && !rNumberFormat.mbSourceLinked;
        sal_Int32 nPropId = bPercent ? PROP_PercentageNumberFormat : PROP_NumberFormat;
        OUString sFormatCode(rNumberFormat.maFormatCode);
        if (bPercent && bGeneral)
            sFormatCode = "0%";
        try
        {
            sal_Int32 nIndex = bGeneral && !bPercent ?
                mxData->mxNumTypes->getStandardIndex( mxData->maFromLocale ) :
                mxData->mxNumFmts->addNewConverted( sFormatCode, mxData->maEnUsLocale, mxData->maFromLocale );
            if( nIndex >= 0 )
                rPropSet.setProperty( nPropId, nIndex );
        }
        catch( Exception& )
        {
            OSL_FAIL( OStringBuffer( "ObjectFormatter::convertNumberFormat - cannot create number format '" ).
                append( OUStringToOString( rNumberFormat.maFormatCode, osl_getThreadTextEncoding() ) ).append( '\'' ).getStr() );
        }

        // Setting "LinkNumberFormatToSource" does not really work, at least not for axis :-/
        if (!bAxis)
            rPropSet.setProperty(PROP_LinkNumberFormatToSource, makeAny(rNumberFormat.mbSourceLinked));
        else
            rPropSet.setProperty(PROP_LinkNumberFormatToSource, makeAny(rNumberFormat.maFormatCode.isEmpty()));
    }
}

void ObjectFormatter::convertAutomaticFill( PropertySet& rPropSet, ObjectType eObjType, sal_Int32 nSeriesIdx )
{
    if( ObjectTypeFormatter* pFormat = mxData->getTypeFormatter( eObjType ) )
        pFormat->convertAutomaticFill( rPropSet, nSeriesIdx );
}

bool ObjectFormatter::isAutomaticFill( const ModelRef< Shape >& rxShapeProp )
{
    return !rxShapeProp || !rxShapeProp->getFillProperties().moFillType.has();
}

bool ObjectFormatter::getTextRotation( const ModelRef< TextBody >& rxTextProp, sal_Int32 nDefaultRotation )
{
    if( rxTextProp.is() )
    {
        double fAnglevalue = static_cast< double >( rxTextProp->getTextProperties().moRotation.get( nDefaultRotation ) );
        if( fAnglevalue < -5400000.0 || fAnglevalue > 5400000.0 || fAnglevalue == 0.0 )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
