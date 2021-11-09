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

#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include "txtprhdl.hxx"
#include <xmlsdtypes.hxx>
#include <sal/log.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

#define M_E_( a, p, l, t, c ) \
    { a, XML_NAMESPACE_##p, XML_##l, t, c, SvtSaveOptions::ODFSVER_010, false }

#define M_EV_( a, p, l, t, c, v ) \
    { a, XML_NAMESPACE_##p, XML_##l, t, c, v, false }

#define M_ED_( a, p, l, t, c ) \
    { a, XML_NAMESPACE_##p, XML_##l, (t) | MID_FLAG_DEFAULT_ITEM_EXPORT, c, SvtSaveOptions::ODFSVER_010, false }

// text properties
#define MT_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_TEXT), c )
#define MT_ED( a, p, l, t, c ) \
    M_ED_( a, p, l, (t|XML_TYPE_PROP_TEXT), c )

// paragraph properties
#define MP_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_PARAGRAPH), c )
#define MP_ED( a, p, l, t, c ) \
    M_ED_( a, p, l, (t|XML_TYPE_PROP_PARAGRAPH), c )

// graphic properties
#define MG_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_GRAPHIC), c )
#define MG_ED( a, p, l, t, c ) \
    M_ED_( a, p, l, (t|XML_TYPE_PROP_GRAPHIC), c )
#define MG_EV( a, p, l, t, c, v ) \
    M_EV_( a, p, l, (t|XML_TYPE_PROP_GRAPHIC), c, v )

// section properties
#define MS_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_SECTION), c )

// ruby properties
#define MR_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_RUBY), c )
#define MR_EV( a, p, l, t, c, v ) \
    M_EV_( a, p, l, (t|XML_TYPE_PROP_RUBY), c, v )

// cell properties
#define MC_E( a, p, l, t, c ) \
    M_E_( a, p, l, (t|XML_TYPE_PROP_TABLE_CELL), c )

#define MAP_ODF13(name,prefix,token,type,context)  { name, prefix, token, type, context, SvtSaveOptions::ODFSVER_013, false }

// extensions import/export
#define MAP_EXT(name,prefix,token,type,context)  { name, prefix, token, type, context, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false }
// extensions import only
#define MAP_EXT_I(name,prefix,token,type,context)  { name, prefix, token, type, context, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, true }

#define M_END() { nullptr, 0, XML_TOKEN_INVALID, 0, 0, SvtSaveOptions::ODFSVER_010, false }

#define MAP_(name,prefix,token,type,context)  { name, prefix, token, type, context, SvtSaveOptions::ODFSVER_010, false }
#define GMAP(name,prefix,token,type,context) MAP_(name,prefix,token,static_cast<sal_Int32>(type|XML_TYPE_PROP_GRAPHIC),context)

constexpr OUStringLiteral EMPTY = u"";

constexpr OUStringLiteral ADJUST_BLUE = u"AdjustBlue";
constexpr OUStringLiteral ADJUST_CONTRAST = u"AdjustContrast";
constexpr OUStringLiteral ADJUST_GREEN = u"AdjustGreen";
constexpr OUStringLiteral ADJUST_LUMINANCE = u"AdjustLuminance";
constexpr OUStringLiteral ADJUST_RED = u"AdjustRed";
constexpr OUStringLiteral ALLOW_OVERLAP = u"AllowOverlap";
constexpr OUStringLiteral ANCHOR_TYPE = u"AnchorType";
constexpr OUStringLiteral BACK_COLOR = u"BackColor";
constexpr OUStringLiteral BACK_COLOR_RGB = u"BackColorRGB";
constexpr OUStringLiteral BACK_COLOR_TRANSPARENCY = u"BackColorTransparency";
constexpr OUStringLiteral BACK_GRAPHIC = u"BackGraphic";
constexpr OUStringLiteral BACK_GRAPHIC_FILTER = u"BackGraphicFilter";
constexpr OUStringLiteral BACK_GRAPHIC_LOCATION = u"BackGraphicLocation";
constexpr OUStringLiteral BACK_GRAPHIC_TRANSPARENCY = u"BackGraphicTransparency";
constexpr OUStringLiteral BACK_TRANSPARENT = u"BackTransparent";
constexpr OUStringLiteral BORDER_DISTANCE = u"BorderDistance";
constexpr OUStringLiteral BOTTOM_BORDER = u"BottomBorder";
constexpr OUStringLiteral BOTTOM_BORDER_DISTANCE = u"BottomBorderDistance";
constexpr OUStringLiteral BOTTOM_MARGIN = u"BottomMargin";
constexpr OUStringLiteral BREAK_TYPE = u"BreakType";
constexpr OUStringLiteral CHAR_AUTO_KERNING = u"CharAutoKerning";
constexpr OUStringLiteral CHAR_BACK_COLOR = u"CharBackColor";
constexpr OUStringLiteral CHAR_BACK_TRANSPARENT = u"CharBackTransparent";
constexpr OUStringLiteral CHAR_BOTTOM_BORDER = u"CharBottomBorder";
constexpr OUStringLiteral CHAR_BOTTOM_BORDER_DISTANCE = u"CharBottomBorderDistance";
constexpr OUStringLiteral CHAR_CASE_MAP = u"CharCaseMap";
constexpr OUStringLiteral CHAR_COLOR = u"CharColor";
constexpr OUStringLiteral CHAR_COMBINE_IS_ON = u"CharCombineIsOn";
constexpr OUStringLiteral CHAR_COMBINE_PREFIX = u"CharCombinePrefix";
constexpr OUStringLiteral CHAR_COMBINE_SUFFIX = u"CharCombineSuffix";
constexpr OUStringLiteral CHAR_CONTOURED = u"CharContoured";
constexpr OUStringLiteral CHAR_DIFF_HEIGHT = u"CharDiffHeight";
constexpr OUStringLiteral CHAR_DIFF_HEIGHT_ASIAN = u"CharDiffHeightAsian";
constexpr OUStringLiteral CHAR_DIFF_HEIGHT_COMPLEX = u"CharDiffHeightComplex";
constexpr OUStringLiteral CHAR_EMPHASIS = u"CharEmphasis";
constexpr OUStringLiteral CHAR_ESCAPEMENT = u"CharEscapement";
constexpr OUStringLiteral CHAR_ESCAPEMENT_HEIGHT = u"CharEscapementHeight";
constexpr OUStringLiteral CHAR_FLASH = u"CharFlash";
constexpr OUStringLiteral CHAR_FONT_CHAR_SET = u"CharFontCharSet";
constexpr OUStringLiteral CHAR_FONT_CHAR_SET_ASIAN = u"CharFontCharSetAsian";
constexpr OUStringLiteral CHAR_FONT_CHAR_SET_COMPLEX = u"CharFontCharSetComplex";
constexpr OUStringLiteral CHAR_FONT_FAMILY = u"CharFontFamily";
constexpr OUStringLiteral CHAR_FONT_FAMILY_ASIAN = u"CharFontFamilyAsian";
constexpr OUStringLiteral CHAR_FONT_FAMILY_COMPLEX = u"CharFontFamilyComplex";
constexpr OUStringLiteral CHAR_FONT_NAME = u"CharFontName";
constexpr OUStringLiteral CHAR_FONT_NAME_ASIAN = u"CharFontNameAsian";
constexpr OUStringLiteral CHAR_FONT_NAME_COMPLEX = u"CharFontNameComplex";
constexpr OUStringLiteral CHAR_FONT_PITCH = u"CharFontPitch";
constexpr OUStringLiteral CHAR_FONT_PITCH_ASIAN = u"CharFontPitchAsian";
constexpr OUStringLiteral CHAR_FONT_PITCH_COMPLEX = u"CharFontPitchComplex";
constexpr OUStringLiteral CHAR_FONT_STYLE_NAME = u"CharFontStyleName";
constexpr OUStringLiteral CHAR_FONT_STYLE_NAME_ASIAN = u"CharFontStyleNameAsian";
constexpr OUStringLiteral CHAR_FONT_STYLE_NAME_COMPLEX = u"CharFontStyleNameComplex";
constexpr OUStringLiteral CHAR_HEIGHT = u"CharHeight";
constexpr OUStringLiteral CHAR_HEIGHT_ASIAN = u"CharHeightAsian";
constexpr OUStringLiteral CHAR_HEIGHT_COMPLEX = u"CharHeightComplex";
constexpr OUStringLiteral CHAR_HIDDEN = u"CharHidden";
constexpr OUStringLiteral CHAR_HIGHLIGHT = u"CharHighlight";
constexpr OUStringLiteral CHAR_KERNING = u"CharKerning";
constexpr OUStringLiteral CHAR_LEFT_BORDER = u"CharLeftBorder";
constexpr OUStringLiteral CHAR_LEFT_BORDER_DISTANCE = u"CharLeftBorderDistance";
constexpr OUStringLiteral CHAR_LOCALE = u"CharLocale";
constexpr OUStringLiteral CHAR_LOCALE_ASIAN = u"CharLocaleAsian";
constexpr OUStringLiteral CHAR_LOCALE_COMPLEX = u"CharLocaleComplex";
constexpr OUStringLiteral CHAR_OVERLINE = u"CharOverline";
constexpr OUStringLiteral CHAR_OVERLINE_COLOR = u"CharOverlineColor";
constexpr OUStringLiteral CHAR_OVERLINE_HAS_COLOR = u"CharOverlineHasColor";
constexpr OUStringLiteral CHAR_POSTURE = u"CharPosture";
constexpr OUStringLiteral CHAR_POSTURE_ASIAN = u"CharPostureAsian";
constexpr OUStringLiteral CHAR_POSTURE_COMPLEX = u"CharPostureComplex";
constexpr OUStringLiteral CHAR_PROP_HEIGHT = u"CharPropHeight";
constexpr OUStringLiteral CHAR_PROP_HEIGHT_ASIAN = u"CharPropHeightAsian";
constexpr OUStringLiteral CHAR_PROP_HEIGHT_COMPLEX = u"CharPropHeightComplex";
constexpr OUStringLiteral CHAR_RELIEF = u"CharRelief";
constexpr OUStringLiteral CHAR_RIGHT_BORDER = u"CharRightBorder";
constexpr OUStringLiteral CHAR_RIGHT_BORDER_DISTANCE = u"CharRightBorderDistance";
constexpr OUStringLiteral CHAR_ROTATION = u"CharRotation";
constexpr OUStringLiteral CHAR_ROTATION_IS_FIT_TO_LINE = u"CharRotationIsFitToLine";
constexpr OUStringLiteral CHAR_SCALE_WIDTH = u"CharScaleWidth";
constexpr OUStringLiteral CHAR_SHADING_VALUE = u"CharShadingValue";
constexpr OUStringLiteral CHAR_SHADOWED = u"CharShadowed";
constexpr OUStringLiteral CHAR_SHADOW_FORMAT = u"CharShadowFormat";
constexpr OUStringLiteral CHAR_STRIKEOUT = u"CharStrikeout";
constexpr OUStringLiteral CHAR_STYLE_NAME = u"CharStyleName";
constexpr OUStringLiteral CHAR_TOP_BORDER = u"CharTopBorder";
constexpr OUStringLiteral CHAR_TOP_BORDER_DISTANCE = u"CharTopBorderDistance";
constexpr OUStringLiteral CHAR_TRANSPARENCE = u"CharTransparence";
constexpr OUStringLiteral CHAR_UNDERLINE = u"CharUnderline";
constexpr OUStringLiteral CHAR_UNDERLINE_COLOR = u"CharUnderlineColor";
constexpr OUStringLiteral CHAR_UNDERLINE_HAS_COLOR = u"CharUnderlineHasColor";
constexpr OUStringLiteral CHAR_WEIGHT = u"CharWeight";
constexpr OUStringLiteral CHAR_WEIGHT_ASIAN = u"CharWeightAsian";
constexpr OUStringLiteral CHAR_WEIGHT_COMPLEX = u"CharWeightComplex";
constexpr OUStringLiteral CHAR_WORD_MODE = u"CharWordMode";
constexpr OUStringLiteral COLLAPSING_BORDERS = u"CollapsingBorders";
constexpr OUStringLiteral CONTENT_PROTECTED = u"ContentProtected";
constexpr OUStringLiteral CONTOUR_OUTSIDE = u"ContourOutside";
constexpr OUStringLiteral DEFAULT_OUTLINE_LEVEL = u"DefaultOutlineLevel";
constexpr OUStringLiteral DONT_BALANCE_TEXT_COLUMNS = u"DontBalanceTextColumns";
constexpr OUStringLiteral DROP_CAP_CHAR_STYLE_NAME = u"DropCapCharStyleName";
constexpr OUStringLiteral DROP_CAP_FORMAT = u"DropCapFormat";
constexpr OUStringLiteral DROP_CAP_WHOLE_WORD = u"DropCapWholeWord";
constexpr OUStringLiteral EDIT_IN_READONLY = u"EditInReadonly";
constexpr OUStringLiteral ENDNOTE_IS_COLLECT_AT_TEXT_END = u"EndnoteIsCollectAtTextEnd";
constexpr OUStringLiteral ENDNOTE_IS_OWN_NUMBERING = u"EndnoteIsOwnNumbering";
constexpr OUStringLiteral ENDNOTE_IS_RESTART_NUMBERING = u"EndnoteIsRestartNumbering";
constexpr OUStringLiteral ENDNOTE_NUMBERING_PREFIX = u"EndnoteNumberingPrefix";
constexpr OUStringLiteral ENDNOTE_NUMBERING_SUFFIX = u"EndnoteNumberingSuffix";
constexpr OUStringLiteral ENDNOTE_NUMBERING_TYPE = u"EndnoteNumberingType";
constexpr OUStringLiteral ENDNOTE_RESTART_NUMBERING_AT = u"EndnoteRestartNumberingAt";
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
constexpr OUStringLiteral FONT_INDEPENDENT_LINE_SPACING = u"FontIndependentLineSpacing";
constexpr OUStringLiteral FOOTNOTE_IS_COLLECT_AT_TEXT_END = u"FootnoteIsCollectAtTextEnd";
constexpr OUStringLiteral FOOTNOTE_IS_OWN_NUMBERING = u"FootnoteIsOwnNumbering";
constexpr OUStringLiteral FOOTNOTE_IS_RESTART_NUMBERING = u"FootnoteIsRestartNumbering";
constexpr OUStringLiteral FOOTNOTE_NUMBERING_PREFIX = u"FootnoteNumberingPrefix";
constexpr OUStringLiteral FOOTNOTE_NUMBERING_SUFFIX = u"FootnoteNumberingSuffix";
constexpr OUStringLiteral FOOTNOTE_NUMBERING_TYPE = u"FootnoteNumberingType";
constexpr OUStringLiteral FOOTNOTE_RESTART_NUMBERING_AT = u"FootnoteRestartNumberingAt";
constexpr OUStringLiteral GAMMA = u"Gamma";
constexpr OUStringLiteral GRAPHIC_COLOR_MODE = u"GraphicColorMode";
constexpr OUStringLiteral GRAPHIC_CROP = u"GraphicCrop";
constexpr OUStringLiteral GRAPHIC_IS_INVERTED = u"GraphicIsInverted";
constexpr OUStringLiteral HEIGHT = u"Height";
constexpr OUStringLiteral HORI_MIRRORED_ON_EVEN_PAGES = u"HoriMirroredOnEvenPages";
constexpr OUStringLiteral HORI_MIRRORED_ON_ODD_PAGES = u"HoriMirroredOnOddPages";
constexpr OUStringLiteral HORI_ORIENT = u"HoriOrient";
constexpr OUStringLiteral HORI_ORIENT_POSITION = u"HoriOrientPosition";
constexpr OUStringLiteral HORI_ORIENT_RELATION = u"HoriOrientRelation";
constexpr OUStringLiteral HYPER_LINK_URL = u"HyperLinkURL";
constexpr OUStringLiteral IS_FOLLOWING_TEXT_FLOW = u"IsFollowingTextFlow";
constexpr OUStringLiteral IS_SPLIT_ALLOWED = u"IsSplitAllowed";
constexpr OUStringLiteral LEFT_BORDER = u"LeftBorder";
constexpr OUStringLiteral LEFT_BORDER_DISTANCE = u"LeftBorderDistance";
constexpr OUStringLiteral LEFT_MARGIN = u"LeftMargin";
constexpr OUStringLiteral NUMBERING_STYLE_NAME = u"NumberingStyleName";
constexpr OUStringLiteral NUMBER_FORMAT = u"NumberFormat";
constexpr OUStringLiteral OPAQUE = u"Opaque";
constexpr OUStringLiteral PAGE_DESC_NAME = u"PageDescName";
constexpr OUStringLiteral PAGE_NUMBER_OFFSET = u"PageNumberOffset";
constexpr OUStringLiteral PAGE_TOGGLE = u"PageToggle";
constexpr OUStringLiteral PARA_ADJUST = u"ParaAdjust";
constexpr OUStringLiteral PARA_BACK_COLOR = u"ParaBackColor";
constexpr OUStringLiteral PARA_BACK_GRAPHIC = u"ParaBackGraphic";
constexpr OUStringLiteral PARA_BACK_GRAPHIC_FILTER = u"ParaBackGraphicFilter";
constexpr OUStringLiteral PARA_BACK_GRAPHIC_LOCATION = u"ParaBackGraphicLocation";
constexpr OUStringLiteral PARA_BACK_TRANSPARENT = u"ParaBackTransparent";
constexpr OUStringLiteral PARA_BOTTOM_MARGIN = u"ParaBottomMargin";
constexpr OUStringLiteral PARA_BOTTOM_MARGIN_RELATIVE = u"ParaBottomMarginRelative";
constexpr OUStringLiteral PARA_CONTEXT_MARGIN = u"ParaContextMargin";
constexpr OUStringLiteral PARA_EXPAND_SINGLE_WORD = u"ParaExpandSingleWord";
constexpr OUStringLiteral PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent";
constexpr OUStringLiteral PARA_FIRST_LINE_INDENT_RELATIVE = u"ParaFirstLineIndentRelative";
constexpr OUStringLiteral PARA_HYPHENATION_MAX_HYPHENS = u"ParaHyphenationMaxHyphens";
constexpr OUStringLiteral PARA_HYPHENATION_MAX_LEADING_CHARS = u"ParaHyphenationMaxLeadingChars";
constexpr OUStringLiteral PARA_HYPHENATION_MAX_TRAILING_CHARS = u"ParaHyphenationMaxTrailingChars";
constexpr OUStringLiteral PARA_HYPHENATION_NO_CAPS = u"ParaHyphenationNoCaps";
constexpr OUStringLiteral PARA_IS_AUTO_FIRST_LINE_INDENT = u"ParaIsAutoFirstLineIndent";
constexpr OUStringLiteral PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance";
constexpr OUStringLiteral PARA_IS_CONNECT_BORDER = u"ParaIsConnectBorder";
constexpr OUStringLiteral PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules";
constexpr OUStringLiteral PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation";
constexpr OUStringLiteral PARA_IS_HYPHENATION = u"ParaIsHyphenation";
constexpr OUStringLiteral PARA_KEEP_TOGETHER = u"ParaKeepTogether";
constexpr OUStringLiteral PARA_LAST_LINE_ADJUST = u"ParaLastLineAdjust";
constexpr OUStringLiteral PARA_LEFT_MARGIN = u"ParaLeftMargin";
constexpr OUStringLiteral PARA_LEFT_MARGIN_RELATIVE = u"ParaLeftMarginRelative";
constexpr OUStringLiteral PARA_LINE_NUMBER_COUNT = u"ParaLineNumberCount";
constexpr OUStringLiteral PARA_LINE_NUMBER_START_VALUE = u"ParaLineNumberStartValue";
constexpr OUStringLiteral PARA_LINE_SPACING = u"ParaLineSpacing";
constexpr OUStringLiteral PARA_ORPHANS = u"ParaOrphans";
constexpr OUStringLiteral PARA_REGISTER_MODE_ACTIVE = u"ParaRegisterModeActive";
constexpr OUStringLiteral PARA_RIGHT_MARGIN = u"ParaRightMargin";
constexpr OUStringLiteral PARA_RIGHT_MARGIN_RELATIVE = u"ParaRightMarginRelative";
constexpr OUStringLiteral PARA_SHADOW_FORMAT = u"ParaShadowFormat";
constexpr OUStringLiteral PARA_SPLIT = u"ParaSplit";
constexpr OUStringLiteral PARA_TAB_STOPS = u"ParaTabStops";
constexpr OUStringLiteral PARA_TOP_MARGIN = u"ParaTopMargin";
constexpr OUStringLiteral PARA_TOP_MARGIN_RELATIVE = u"ParaTopMarginRelative";
constexpr OUStringLiteral PARA_USER_DEFINED_ATTRIBUTES = u"ParaUserDefinedAttributes";
constexpr OUStringLiteral PARA_VERT_ALIGNMENT = u"ParaVertAlignment";
constexpr OUStringLiteral PARA_WIDOWS = u"ParaWidows";
constexpr OUStringLiteral PAR_RSID = u"ParRsid";
constexpr OUStringLiteral POSITION_PROTECTED = u"PositionProtected";
constexpr OUStringLiteral PRINT = u"Print";
constexpr OUStringLiteral RELATIVE_HEIGHT = u"RelativeHeight";
constexpr OUStringLiteral RELATIVE_HEIGHT_RELATION = u"RelativeHeightRelation";
constexpr OUStringLiteral RELATIVE_WIDTH = u"RelativeWidth";
constexpr OUStringLiteral RELATIVE_WIDTH_RELATION = u"RelativeWidthRelation";
constexpr OUStringLiteral RIGHT_BORDER = u"RightBorder";
constexpr OUStringLiteral RIGHT_BORDER_DISTANCE = u"RightBorderDistance";
constexpr OUStringLiteral RIGHT_MARGIN = u"RightMargin";
constexpr OUStringLiteral RSID = u"Rsid";
constexpr OUStringLiteral RUBY_ADJUST = u"RubyAdjust";
constexpr OUStringLiteral RUBY_IS_ABOVE = u"RubyIsAbove";
constexpr OUStringLiteral RUBY_POSITION = u"RubyPosition";
constexpr OUStringLiteral SECTION_LEFT_MARGIN = u"SectionLeftMargin";
constexpr OUStringLiteral SECTION_RIGHT_MARGIN = u"SectionRightMargin";
constexpr OUStringLiteral SHADOW_FORMAT = u"ShadowFormat";
constexpr OUStringLiteral SHADOW_TRANSPARENCE = u"ShadowTransparence";
constexpr OUStringLiteral SIZE_PROTECTED = u"SizeProtected";
constexpr OUStringLiteral SIZE_TYPE = u"SizeType";
constexpr OUStringLiteral SNAP_TO_GRID = u"SnapToGrid";
constexpr OUStringLiteral SURROUND_ANCHOR_ONLY = u"SurroundAnchorOnly";
constexpr OUStringLiteral SURROUND_CONTOUR = u"SurroundContour";
constexpr OUStringLiteral TAB_STOP_DISTANCE = u"TabStopDistance";
constexpr OUStringLiteral TEXT_COLUMNS = u"TextColumns";
constexpr OUStringLiteral TEXT_USER_DEFINED_ATTRIBUTES = u"TextUserDefinedAttributes";
constexpr OUStringLiteral TEXT_VERTICAL_ADJUST = u"TextVerticalAdjust";
constexpr OUStringLiteral TEXT_WRAP = u"TextWrap";
constexpr OUStringLiteral TOP_BORDER = u"TopBorder";
constexpr OUStringLiteral TOP_BORDER_DISTANCE = u"TopBorderDistance";
constexpr OUStringLiteral TOP_MARGIN = u"TopMargin";
constexpr OUStringLiteral TRANSPARENCY = u"Transparency";
constexpr OUStringLiteral USER_DEFINED_ATTRIBUTES = u"UserDefinedAttributes";
constexpr OUStringLiteral VERT_MIRRORED = u"VertMirrored";
constexpr OUStringLiteral VERT_ORIENT = u"VertOrient";
constexpr OUStringLiteral VERT_ORIENT_POSITION = u"VertOrientPosition";
constexpr OUStringLiteral VERT_ORIENT_RELATION = u"VertOrientRelation";
constexpr OUStringLiteral WIDTH = u"Width";
constexpr OUStringLiteral WIDTH_TYPE = u"WidthType";
constexpr OUStringLiteral WRAP_INFLUENCE_ON_POSITION = u"WrapInfluenceOnPosition";
constexpr OUStringLiteral WRITING_MODE = u"WritingMode";

XMLPropertyMapEntry const aXMLParaPropMap[] =
{
    // RES_UNKNOWNATR_CONTAINER
    MP_E( PARA_USER_DEFINED_ATTRIBUTES, TEXT, XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    // fill attributes for paragraph backgrounds
    // #i125045# moved to the front to be able to exclude these in lcl_txtprmap_getMap
    // for TextPropMap::SHAPE_PARA to not have these double for Shapes (which already have these)
    GMAP( FILL_STYLE,                       XML_NAMESPACE_DRAW, XML_FILL,                   XML_SW_TYPE_FILLSTYLE, 0 ),
    GMAP( FILL_COLOR,                       XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    GMAP( FILL_COLOR2,                     XML_NAMESPACE_DRAW, XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR, 0 ),
    GMAP( FILL_GRADIENT_NAME,               XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    GMAP( FILL_GRADIENT_STEP_COUNT,          XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16, 0 ),
    GMAP( FILL_HATCH_NAME,                  XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( FILL_BACKGROUND,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    GMAP( FILL_BITMAP_NAME,                 XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    GMAP( FILL_TRANSPARENCE,               XML_NAMESPACE_DRAW, XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),    // exists in SW, too
    GMAP( FILL_TRANSPARENCE_GRADIENT_NAME,   XML_NAMESPACE_DRAW, XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    GMAP( FILL_BITMAP_SIZE_X,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_SIZE_Y,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_MODE,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_X,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_Y,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_RECTANGLE_POINT,       XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT, 0 ),
    GMAP( FILL_BITMAP_OFFSET_X,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    GMAP( FILL_BITMAP_OFFSET_Y,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // RES_LR_SPACE
    // !!! DO NOT REORDER THE MARGINS !!!
    MP_E( PARA_LEFT_MARGIN,         FO, MARGIN,     XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARAMARGINALL ),
    MP_E( PARA_LEFT_MARGIN_RELATIVE, FO, MARGIN,     XML_TYPE_PERCENT16, CTF_PARAMARGINALL_REL ),
    MP_E( PARA_LEFT_MARGIN,         FO, MARGIN_LEFT,        XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARALEFTMARGIN ),
    MP_E( PARA_LEFT_MARGIN_RELATIVE, FO, MARGIN_LEFT,        XML_TYPE_PERCENT16, CTF_PARALEFTMARGIN_REL ),
    MP_E( PARA_RIGHT_MARGIN,            FO, MARGIN_RIGHT,       XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARARIGHTMARGIN ),
    MP_E( PARA_RIGHT_MARGIN_RELATIVE,    FO, MARGIN_RIGHT,       XML_TYPE_PERCENT16, CTF_PARARIGHTMARGIN_REL ),
    // RES_UL_SPACE
    MP_E( PARA_TOP_MARGIN,          FO, MARGIN_TOP,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARATOPMARGIN ),
    MP_E( PARA_TOP_MARGIN_RELATIVE,  FO, MARGIN_TOP,         XML_TYPE_PERCENT16, CTF_PARATOPMARGIN_REL ),
    MP_E( PARA_BOTTOM_MARGIN,       FO, MARGIN_BOTTOM,      XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARABOTTOMMARGIN ),
    MP_E( PARA_BOTTOM_MARGIN_RELATIVE,FO,    MARGIN_BOTTOM,      XML_TYPE_PERCENT16, CTF_PARABOTTOMMARGIN_REL ),
    MAP_ODF13( PARA_CONTEXT_MARGIN, XML_NAMESPACE_STYLE, XML_CONTEXTUAL_SPACING, XML_TYPE_BOOL|XML_TYPE_PROP_PARAGRAPH, 0 ),    // ODF 1.3 OFFICE-3767 and was written by LO<=4.2
    MAP_ODF13( PARA_CONTEXT_MARGIN, XML_NAMESPACE_LO_EXT, XML_CONTEXTUAL_SPACING, XML_TYPE_BOOL|XML_TYPE_PROP_PARAGRAPH, 0 ),     // extension namespace
    // RES_CHRATR_CASEMAP
    MT_E( CHAR_CASE_MAP,        FO,     FONT_VARIANT,       XML_TYPE_TEXT_CASEMAP_VAR,  0 ),
    MT_E( CHAR_CASE_MAP,        FO,     TEXT_TRANSFORM,     XML_TYPE_TEXT_CASEMAP,  0 ),
    // RES_CHRATR_COLOR
    MT_ED( CHAR_COLOR,     FO,     COLOR,              XML_TYPE_COLORAUTO|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_COLOR,     STYLE,  USE_WINDOW_FONT_COLOR,  XML_TYPE_ISAUTOCOLOR|MID_FLAG_MERGE_PROPERTY,   0 ),
    MAP_EXT_I(CHAR_TRANSPARENCE, XML_NAMESPACE_DRAW, XML_OPACITY, XML_TYPE_NEG_PERCENT16 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT(CHAR_TRANSPARENCE, XML_NAMESPACE_LO_EXT, XML_OPACITY, XML_TYPE_NEG_PERCENT16 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT("CharColorTheme", XML_NAMESPACE_LO_EXT, XML_THEME_COLOR, XML_TYPE_THEME_COLOR, 0),
    MAP_EXT("CharColorLumMod", XML_NAMESPACE_LO_EXT, XML_COLOR_LUM_MOD, XML_TYPE_PERCENT100 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT("CharColorLumOff", XML_NAMESPACE_LO_EXT, XML_COLOR_LUM_OFF, XML_TYPE_PERCENT100 | XML_TYPE_PROP_TEXT, 0),
    // RES_CHRATR_CONTOUR
    MT_E( CHAR_CONTOURED,  STYLE,  TEXT_OUTLINE,       XML_TYPE_BOOL,  0 ),
    // RES_CHRATR_CROSSEDOUT
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),
    // RES_CHRATR_ESCAPEMENT
    MT_E( CHAR_ESCAPEMENT,      STYLE, TEXT_POSITION,  XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MT_E( CHAR_ESCAPEMENT_HEIGHT, STYLE, TEXT_POSITION, XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_CHRATR_FONT
    MT_ED( CHAR_FONT_NAME,  STYLE,  FONT_NAME,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME ),
    MT_ED( CHAR_FONT_NAME,  FO,     FONT_FAMILY,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME ),
    MT_ED( CHAR_FONT_STYLE_NAME,STYLE,   FONT_STYLE_NAME,    XML_TYPE_STRING, CTF_FONTSTYLENAME ),
    MT_ED( CHAR_FONT_FAMILY,    STYLE,  FONT_FAMILY_GENERIC,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY ),
    MT_ED( CHAR_FONT_PITCH, STYLE,  FONT_PITCH,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH ),
    MT_ED( CHAR_FONT_CHAR_SET,   STYLE,  FONT_CHARSET,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET ),
    // RES_CHRATR_FONTSIZE
    MT_ED( CHAR_HEIGHT,          FO,   FONT_SIZE,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT ),
    MT_ED( CHAR_PROP_HEIGHT,FO, FONT_SIZE,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL ),
    MT_ED( CHAR_DIFF_HEIGHT,STYLE,FONT_SIZE_REL,        XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF ),
    // RES_CHRATR_KERNING
    MT_E( CHAR_KERNING,        FO,     LETTER_SPACING,     XML_TYPE_TEXT_KERNING, 0 ),
    // RES_CHRATR_LANGUAGE
    MT_ED( CHAR_LOCALE,     STYLE,     RFC_LANGUAGE_TAG,   XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     LANGUAGE,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     SCRIPT,             XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     COUNTRY,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_POSTURE
    MT_E( CHAR_POSTURE,        FO,     FONT_STYLE,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_UNUSED1
    // RES_CHRATR_SHADOWED
    MT_E( CHAR_SHADOWED,   FO,     TEXT_SHADOW,        XML_TYPE_TEXT_SHADOWED, 0 ),
    // RES_CHRATR_UNDERLINE
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, CTF_UNDERLINE ),
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_UNDERLINE_COLOR, STYLE,  TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, CTF_UNDERLINE_COLOR  ),
    MT_E( CHAR_UNDERLINE_HAS_COLOR,  STYLE,  TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, CTF_UNDERLINE_HASCOLOR   ),
    // RES_CHRATR_WEIGHT
    MT_E( CHAR_WEIGHT,     FO,     FONT_WEIGHT,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_RSID
    { RSID, XML_NAMESPACE_OFFICE_EXT, XML_RSID, XML_TYPE_HEX|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },
    // RES_PARATR_RSID
    { PAR_RSID, XML_NAMESPACE_OFFICE_EXT, XML_PARRSID, XML_TYPE_HEX|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },
    // RES_CHRATR_WORDLINEMODE
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_UNDERLINE_MODE,        XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_OVERLINE_MODE,     XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_LINE_THROUGH_MODE,     XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_AUTOKERN
    MT_E( CHAR_AUTO_KERNING,    STYLE,  LETTER_KERNING,     XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_BLINK
    MT_E( CHAR_FLASH,      STYLE,  TEXT_BLINKING,      XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_NOHYPHEN
    // TODO: not used?
    // RES_CHRATR_UNUSED2
    // RES_CHRATR_BACKGROUND
    MT_E( CHAR_BACK_COLOR,  FO, BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, CTF_CHAR_BACKGROUND ),
    MT_E( CHAR_BACK_TRANSPARENT,    FO, BACKGROUND_COLOR, XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, CTF_CHAR_BACKGROUND_TRANSPARENCY),
    MT_E( CHAR_BACK_COLOR,  FO, TEXT_BACKGROUND_COLOR, XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_OLDTEXTBACKGROUND ),
    // RES_CHRATR_CJK_FONT
    MT_ED( CHAR_FONT_NAME_ASIAN, STYLE,  FONT_NAME_ASIAN,            XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CJK ),
    MT_ED( CHAR_FONT_NAME_ASIAN, STYLE,      FONT_FAMILY_ASIAN,      XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CJK ),
    MT_ED( CHAR_FONT_STYLE_NAME_ASIAN,STYLE,  FONT_STYLE_NAME_ASIAN,  XML_TYPE_STRING, CTF_FONTSTYLENAME_CJK ),
    MT_ED( CHAR_FONT_FAMILY_ASIAN,   STYLE,  FONT_FAMILY_GENERIC_ASIAN,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CJK ),
    MT_ED( CHAR_FONT_PITCH_ASIAN,    STYLE,  FONT_PITCH_ASIAN,           XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CJK ),
    MT_ED( CHAR_FONT_CHAR_SET_ASIAN,  STYLE,  FONT_CHARSET_ASIAN,     XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CJK ),
    // RES_CHRATR_CJK_FONTSIZE
    MT_ED( CHAR_HEIGHT_ASIAN,         STYLE,    FONT_SIZE_ASIAN,            XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CJK ),
    MT_ED( CHAR_PROP_HEIGHT_ASIAN,STYLE, FONT_SIZE_ASIAN,            XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CJK ),
    MT_ED( CHAR_DIFF_HEIGHT_ASIAN,STYLE,FONT_SIZE_REL_ASIAN,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CJK ),
    // RES_CHRATR_CJK_LANGUAGE
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      RFC_LANGUAGE_TAG_ASIAN, XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      LANGUAGE_ASIAN,             XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      SCRIPT_ASIAN,           XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      COUNTRY_ASIAN,          XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CJK_POSTURE
    MT_E( CHAR_POSTURE_ASIAN,       STYLE,      FONT_STYLE_ASIAN,           XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CJK_WEIGHT
    MT_E( CHAR_WEIGHT_ASIAN,        STYLE,      FONT_WEIGHT_ASIAN,      XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_CTL_FONT
    MT_ED( CHAR_FONT_NAME_COMPLEX,   STYLE,  FONT_NAME_COMPLEX,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CTL ),
    MT_ED( CHAR_FONT_NAME_COMPLEX,   STYLE,      FONT_FAMILY_COMPLEX,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CTL ),
    MT_ED( CHAR_FONT_STYLE_NAME_COMPLEX,STYLE,    FONT_STYLE_NAME_COMPLEX,    XML_TYPE_STRING, CTF_FONTSTYLENAME_CTL ),
    MT_ED( CHAR_FONT_FAMILY_COMPLEX, STYLE,  FONT_FAMILY_GENERIC_COMPLEX,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CTL ),
    MT_ED( CHAR_FONT_PITCH_COMPLEX,  STYLE,  FONT_PITCH_COMPLEX,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CTL ),
    MT_ED( CHAR_FONT_CHAR_SET_COMPLEX,    STYLE,  FONT_CHARSET_COMPLEX,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CTL ),
    // RES_CHRATR_CTL_FONTSIZE
    MT_ED( CHAR_HEIGHT_COMPLEX,       STYLE,    FONT_SIZE_COMPLEX,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CTL ),
    MT_ED( CHAR_PROP_HEIGHT_COMPLEX,STYLE,   FONT_SIZE_COMPLEX,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CTL ),
    MT_ED( CHAR_DIFF_HEIGHT_COMPLEX,STYLE,FONT_SIZE_REL_COMPLEX,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CTL ),
    // RES_CHRATR_CTL_LANGUAGE
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      RFC_LANGUAGE_TAG_COMPLEX,   XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      LANGUAGE_COMPLEX,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      SCRIPT_COMPLEX,             XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      COUNTRY_COMPLEX,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CTL_POSTURE
    MT_E( CHAR_POSTURE_COMPLEX,     STYLE,      FONT_STYLE_COMPLEX,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CTL_WEIGHT
    MT_E( CHAR_WEIGHT_COMPLEX,      STYLE,      FONT_WEIGHT_COMPLEX,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_ROTATE
    MT_E( CHAR_ROTATION,           STYLE,      TEXT_ROTATION_ANGLE,        XML_TYPE_TEXT_ROTATION_ANGLE, 0 ),
    MT_E( CHAR_ROTATION_IS_FIT_TO_LINE,    STYLE,      TEXT_ROTATION_SCALE,        XML_TYPE_TEXT_ROTATION_SCALE, 0 ),
    // RES_CHRATR_EMPHASIS_MARK
    MT_E( CHAR_EMPHASIS,           STYLE,      TEXT_EMPHASIZE,             XML_TYPE_TEXT_EMPHASIZE, 0 ),
    // RES_CHRATR_TWO_LINES
    MT_E( CHAR_COMBINE_IS_ON,            STYLE,      TEXT_COMBINE,               XML_TYPE_TEXT_COMBINE, 0 ),
    MT_E( CHAR_COMBINE_PREFIX,      STYLE,      TEXT_COMBINE_START_CHAR,    XML_TYPE_TEXT_COMBINECHAR, 0 ),
    MT_E( CHAR_COMBINE_SUFFIX,      STYLE,      TEXT_COMBINE_END_CHAR,      XML_TYPE_TEXT_COMBINECHAR, 0 ),
    // RES_CHRATR_SCALEW
    MT_E( CHAR_SCALE_WIDTH,         STYLE,      TEXT_SCALE,                 XML_TYPE_PERCENT16, 0 ),
    //RES_CHRATR_RELIEF
    MT_E( CHAR_RELIEF,             STYLE,      FONT_RELIEF,                XML_TYPE_TEXT_FONT_RELIEF, 0 ),
    // RES_CHRATR_HIDDEN
    MT_E( CHAR_HIDDEN,              TEXT,       DISPLAY,                    XML_TYPE_TEXT_HIDDEN_AS_DISPLAY|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_TEXT_DISPLAY ),
    // RES_CHRATR_OVERLINE
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_STYLE,        XML_TYPE_TEXT_OVERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_TYPE,     XML_TYPE_TEXT_OVERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_WIDTH,        XML_TYPE_TEXT_OVERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE_COLOR,  STYLE,  TEXT_OVERLINE_COLOR,        XML_TYPE_TEXT_OVERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE_HAS_COLOR,   STYLE,  TEXT_OVERLINE_COLOR,        XML_TYPE_TEXT_OVERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    // RES_CHRATR_BOX
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERWIDTH ),
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERWIDTH ),
    MAP_EXT( CHAR_RIGHT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERWIDTH ),
    MAP_EXT( CHAR_TOP_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERWIDTH ),
    MAP_EXT( CHAR_BOTTOM_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERWIDTH ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERWIDTH ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERWIDTH ),
    MAP_EXT_I( CHAR_RIGHT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERWIDTH ),
    MAP_EXT_I( CHAR_TOP_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERWIDTH ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERWIDTH ),

    MAP_EXT( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERDISTANCE ),
    MAP_EXT( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING_LEFT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERDISTANCE ),
    MAP_EXT( CHAR_RIGHT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING_RIGHT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERDISTANCE ),
    MAP_EXT( CHAR_TOP_BORDER_DISTANCE,  XML_NAMESPACE_LO_EXT, XML_PADDING_TOP, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERDISTANCE ),
    MAP_EXT( CHAR_BOTTOM_BORDER_DISTANCE,XML_NAMESPACE_LO_EXT, XML_PADDING_BOTTOM, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERDISTANCE ),
    MAP_EXT_I( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERDISTANCE ),
    MAP_EXT_I( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERDISTANCE ),
    MAP_EXT_I( CHAR_RIGHT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERDISTANCE ),
    MAP_EXT_I( CHAR_TOP_BORDER_DISTANCE,  XML_NAMESPACE_FO, XML_PADDING_TOP, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERDISTANCE ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER_DISTANCE,XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERDISTANCE ),

    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDER ),
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LEFT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDER ),
    MAP_EXT( CHAR_RIGHT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_RIGHT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDER ),
    MAP_EXT( CHAR_TOP_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_TOP, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDER ),
    MAP_EXT( CHAR_BOTTOM_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_BOTTOM, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDER ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_FO, XML_BORDER, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDER ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDER ),
    MAP_EXT_I( CHAR_RIGHT_BORDER, XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDER ),
    MAP_EXT_I( CHAR_TOP_BORDER, XML_NAMESPACE_FO, XML_BORDER_TOP, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDER ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER, XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDER ),
    // RES_CHRATR_SHADOW
    MAP_EXT( CHAR_SHADOW_FORMAT, XML_NAMESPACE_LO_EXT, XML_SHADOW, XML_TYPE_TEXT_SHADOW|XML_TYPE_PROP_TEXT, 0 ),
    MAP_EXT_I( CHAR_SHADOW_FORMAT, XML_NAMESPACE_STYLE, XML_SHADOW, XML_TYPE_TEXT_SHADOW|XML_TYPE_PROP_TEXT, 0 ),
    // RES_CHRATR_HIGHLIGHT
    MT_E( CHAR_HIGHLIGHT,  FO, BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY| MID_FLAG_NO_PROPERTY_IMPORT, CTF_CHAR_HIGHLIGHT ),
    // RES_TXTATR_INETFMT
    // TODO
    // RES_TXTATR_REFMARK
    // TODO
    // RES_TXTATR_TOXMARK
    // TODO
    // RES_TXTATR_CHARFMT
//  M_E_SI( TEXT,   style_name,         RES_TXTATR_CHARFMT, 0 ),
    // RES_TXTATR_CJK_RUBY
    // TODO
    // RES_TXTATR_FIELD
    // TODO
    // RES_TXTATR_FLYCNT
    // TODO
    // RES_TXTATR_FTN
    // TODO
    // RES_TXTATR_SOFTHYPH
    // TODO
    // RES_TXTATR_HARDBLANK
    // TODO

    // RES_PARATR_LINESPACING
    MP_E( PARA_LINE_SPACING,        FO,     LINE_HEIGHT,            XML_TYPE_LINE_SPACE_FIXED, 0 ),
    MP_E( PARA_LINE_SPACING,        STYLE,  LINE_HEIGHT_AT_LEAST,   XML_TYPE_LINE_SPACE_MINIMUM, 0 ),
    MP_E( PARA_LINE_SPACING,        STYLE,  LINE_SPACING,           XML_TYPE_LINE_SPACE_DISTANCE, 0 ),
    // RES_PARATR_ADJUST
    MP_E( PARA_ADJUST,         FO,     TEXT_ALIGN,         XML_TYPE_TEXT_ADJUST, CTF_SD_SHAPE_PARA_ADJUST ),
    MP_E( PARA_LAST_LINE_ADJUST, FO,     TEXT_ALIGN_LAST,    XML_TYPE_TEXT_ADJUSTLAST, CTF_PARA_ADJUSTLAST ),
    MP_E( PARA_EXPAND_SINGLE_WORD,STYLE, JUSTIFY_SINGLE_WORD,XML_TYPE_BOOL, 0 ),
    // RES_PARATR_SPLIT
    MP_E( PARA_SPLIT,          FO,     KEEP_TOGETHER,      XML_TYPE_TEXT_SPLIT, 0 ),
    // RES_PARATR_ORPHANS
    MP_E( PARA_ORPHANS,            FO,     ORPHANS,            XML_TYPE_NUMBER8, 0 ),
    // RES_PARATR_WIDOWS
    MP_E( PARA_WIDOWS,         FO,     WIDOWS,             XML_TYPE_NUMBER8, 0 ),
    // RES_PARATR_TABSTOP
    MP_ED( PARA_TAB_STOPS,      STYLE,  TAB_STOPS,          MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_TABSTOP, CTF_TABSTOP ), // this is not really a string!
    // RES_PARATR_HYPHENZONE
    MT_E( PARA_IS_HYPHENATION,  FO,     HYPHENATE,          XML_TYPE_BOOL, 0 ),
    MT_E( PARA_HYPHENATION_MAX_LEADING_CHARS, FO, HYPHENATION_REMAIN_CHAR_COUNT, XML_TYPE_NUMBER16_NO_ZERO, 0 ),
    MT_E( PARA_HYPHENATION_MAX_TRAILING_CHARS,FO, HYPHENATION_PUSH_CHAR_COUNT, XML_TYPE_NUMBER16_NO_ZERO, 0 ),
    MP_E( PARA_HYPHENATION_MAX_HYPHENS,  FO, HYPHENATION_LADDER_COUNT, XML_TYPE_NUMBER16_NONE, 0 ),
    MAP_EXT( PARA_HYPHENATION_NO_CAPS,  XML_NAMESPACE_LO_EXT, XML_HYPHENATION_NO_CAPS, XML_TYPE_BOOL|XML_TYPE_PROP_TEXT, 0 ),
    // RES_PARATR_DROP
    MP_E( DROP_CAP_WHOLE_WORD,   STYLE,  LENGTH,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL, CTF_DROPCAPWHOLEWORD ),
    MP_E( DROP_CAP_CHAR_STYLE_NAME,   STYLE,  STYLE_NAME, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_DROPCAPCHARSTYLE ),
    MP_E( DROP_CAP_FORMAT,      STYLE,  DROP_CAP,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_DROPCAP, CTF_DROPCAPFORMAT ),
    // RES_PARATR_REGISTER
    MP_E( PARA_REGISTER_MODE_ACTIVE, STYLE,  REGISTER_TRUE,  XML_TYPE_BOOL, 0 ),
    // RES_PARATR_NUMRULE
    MP_E( NUMBERING_STYLE_NAME, STYLE,  LIST_STYLE_NAME,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STYLENAME, CTF_NUMBERINGSTYLENAME ),

    // RES_FILL_ORDER
    // not required
    // RES_FRM_SIZE
    // not required
    // RES_PAPER_BIN
    // not required
    // RES_LR_SPACE

    MP_E( PARA_FIRST_LINE_INDENT,        FO, TEXT_INDENT,        XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARAFIRSTLINE ),
    MP_E( PARA_FIRST_LINE_INDENT_RELATIVE,    FO, TEXT_INDENT,    XML_TYPE_PERCENT, CTF_PARAFIRSTLINE_REL ),
    MP_E( PARA_IS_AUTO_FIRST_LINE_INDENT,  STYLE, AUTO_TEXT_INDENT,    XML_TYPE_BOOL, 0 ),
    // RES_PAGEDESC
    MP_E( PAGE_DESC_NAME,           STYLE,  MASTER_PAGE_NAME,           MID_FLAG_SPECIAL_ITEM|XML_TYPE_STYLENAME, CTF_PAGEDESCNAME ),
    MP_E( PAGE_NUMBER_OFFSET,       STYLE,  PAGE_NUMBER,            XML_TYPE_NUMBER16_AUTO|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_PAGENUMBEROFFSET ),
    // RES_BREAK : TODO: does this work?
    MP_E( BREAK_TYPE,      FO, BREAK_BEFORE,       XML_TYPE_TEXT_BREAKBEFORE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MP_E( BREAK_TYPE,      FO, BREAK_AFTER,        XML_TYPE_TEXT_BREAKAFTER, 0 ),
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
    // DO NOT REORDER these!
    MP_E( PARA_BACK_COLOR,  FO, BACKGROUND_COLOR,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    MP_E( PARA_BACK_TRANSPARENT,    FO, BACKGROUND_COLOR,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MP_E( PARA_BACK_GRAPHIC_LOCATION,    STYLE,  POSITION,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    MP_E( PARA_BACK_GRAPHIC_FILTER,STYLE,    FILTER_NAME,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    MP_E( PARA_BACK_GRAPHIC, STYLE,  BACKGROUND_IMAGE,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_GRAPHIC, CTF_BACKGROUND_URL ),

    // RES_BOX
    MP_E( LEFT_BORDER,         STYLE,  BORDER_LINE_WIDTH,        XML_TYPE_BORDER_WIDTH, CTF_ALLBORDERWIDTH ),
    MP_E( LEFT_BORDER,         STYLE,  BORDER_LINE_WIDTH_LEFT,   XML_TYPE_BORDER_WIDTH, CTF_LEFTBORDERWIDTH ),
    MP_E( RIGHT_BORDER,            STYLE,  BORDER_LINE_WIDTH_RIGHT,  XML_TYPE_BORDER_WIDTH, CTF_RIGHTBORDERWIDTH ),
    MP_E( TOP_BORDER,          STYLE,  BORDER_LINE_WIDTH_TOP,    XML_TYPE_BORDER_WIDTH, CTF_TOPBORDERWIDTH ),
    MP_E( BOTTOM_BORDER,       STYLE,  BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH, CTF_BOTTOMBORDERWIDTH ),

    MP_E( LEFT_BORDER_DISTANCE, FO,     PADDING,                  XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_ALLBORDERDISTANCE ), // need special import filtering
    MP_E( LEFT_BORDER_DISTANCE, FO,     PADDING_LEFT,             XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_LEFTBORDERDISTANCE ),
    MP_E( RIGHT_BORDER_DISTANCE,    FO,     PADDING_RIGHT,            XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_RIGHTBORDERDISTANCE ),
    MP_E( TOP_BORDER_DISTANCE,  FO,     PADDING_TOP,              XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_TOPBORDERDISTANCE ),
    MP_E( BOTTOM_BORDER_DISTANCE,FO,    PADDING_BOTTOM,           XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_BOTTOMBORDERDISTANCE ),

    MP_E( LEFT_BORDER,         FO,     BORDER,                   XML_TYPE_BORDER, CTF_ALLBORDER ),
    MP_E( LEFT_BORDER,         FO,     BORDER_LEFT,              XML_TYPE_BORDER, CTF_LEFTBORDER ),
    MP_E( RIGHT_BORDER,            FO,     BORDER_RIGHT,             XML_TYPE_BORDER, CTF_RIGHTBORDER ),
    MP_E( TOP_BORDER,          FO,     BORDER_TOP,               XML_TYPE_BORDER, CTF_TOPBORDER ),
    MP_E( BOTTOM_BORDER,       FO,     BORDER_BOTTOM,            XML_TYPE_BORDER, CTF_BOTTOMBORDER ),
    // RES_SHADOW
    MP_E( PARA_SHADOW_FORMAT,   STYLE,  SHADOW,     XML_TYPE_TEXT_SHADOW, 0 ),
    // RES_FRMMACRO
    // not required
    // RES_COL
    // not required
    // RES_KEEP
    MP_E( PARA_KEEP_TOGETHER,   FO, KEEP_WITH_NEXT,     XML_TYPE_TEXT_KEEP, 0 ),
    // RES_URL
    // not required
    // RES_EDIT_IN_READONLY
    // not required
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required

    // RES_LINENUMBER
    MP_E( PARA_LINE_NUMBER_COUNT,    TEXT,   NUMBER_LINES,           XML_TYPE_BOOL, 0 ),
    MP_E( PARA_LINE_NUMBER_START_VALUE, TEXT, LINE_NUMBER,            XML_TYPE_NUMBER, 0 ),

    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    MP_ED( PARA_IS_CHARACTER_DISTANCE, STYLE, TEXT_AUTOSPACE, XML_TYPE_TEXT_AUTOSPACE, 0 ),
    MP_ED( PARA_IS_HANGING_PUNCTUATION, STYLE, PUNCTUATION_WRAP, XML_TYPE_TEXT_PUNCTUATION_WRAP, 0 ),
    MP_ED( PARA_IS_FORBIDDEN_RULES, STYLE, LINE_BREAK, XML_TYPE_TEXT_LINE_BREAK, 0 ),
    MP_E( TAB_STOP_DISTANCE, STYLE, TAB_STOP_DISTANCE, XML_TYPE_MEASURE, 0 ),

    // RES_PARATR_VERTALIGN
    MP_E( PARA_VERT_ALIGNMENT, STYLE, VERTICAL_ALIGN,   XML_TYPE_TEXT_VERTICAL_ALIGN, 0 ),

    // RES_PARATR_SNAPTOGRID
    MP_E( SNAP_TO_GRID, STYLE, SNAP_TO_LAYOUT_GRID, XML_TYPE_BOOL, 0 ),

    MP_ED( WRITING_MODE,      STYLE, WRITING_MODE,       XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT, CTF_TEXTWRITINGMODE ),

    MP_E( PARA_IS_CONNECT_BORDER, STYLE, JOIN_BORDER,  XML_TYPE_BOOL, 0 ),

    MP_E( DEFAULT_OUTLINE_LEVEL, STYLE, DEFAULT_OUTLINE_LEVEL, XML_TYPE_TEXT_NUMBER8_ONE_BASED|MID_FLAG_SPECIAL_ITEM_EXPORT|MID_FLAG_NO_PROPERTY_IMPORT, CTF_DEFAULT_OUTLINE_LEVEL ),

    MP_ED( FONT_INDEPENDENT_LINE_SPACING, STYLE, FONT_INDEPENDENT_LINE_SPACING, XML_TYPE_BOOL, 0 ),

    M_END()
};


XMLPropertyMapEntry const aXMLAdditionalTextDefaultsMap[] =
{
    // RES_FOLLOW_TEXT_FLOW - DVO #i18732#
    MG_ED( IS_FOLLOWING_TEXT_FLOW, STYLE, FLOW_WITH_TEXT,      XML_TYPE_BOOL, 0 ),

    // #i28701# - RES_WRAP_INFLUENCE_ON_OBJPOS
    MG_ED( WRAP_INFLUENCE_ON_POSITION, DRAW, WRAP_INFLUENCE_ON_POSITION, XML_TYPE_WRAP_INFLUENCE_ON_POSITION, 0 ),

    M_END()
};

XMLPropertyMapEntry const aXMLTextPropMap[] =
{
    // RES_CHRATR_CASEMAP
    MT_E( CHAR_CASE_MAP,        FO,     FONT_VARIANT,       XML_TYPE_TEXT_CASEMAP_VAR,  0 ),
    MT_E( CHAR_CASE_MAP,        FO,     TEXT_TRANSFORM,     XML_TYPE_TEXT_CASEMAP,  0 ),
    // RES_CHRATR_COLOR
    MT_ED( CHAR_COLOR,     FO,     COLOR,              XML_TYPE_COLORAUTO|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_COLOR,     STYLE,  USE_WINDOW_FONT_COLOR,  XML_TYPE_ISAUTOCOLOR|MID_FLAG_MERGE_PROPERTY,   0 ),
    MAP_EXT_I(CHAR_TRANSPARENCE, XML_NAMESPACE_DRAW, XML_OPACITY, XML_TYPE_NEG_PERCENT16 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT(CHAR_TRANSPARENCE, XML_NAMESPACE_LO_EXT, XML_OPACITY, XML_TYPE_NEG_PERCENT16 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT("CharColorTheme", XML_NAMESPACE_LO_EXT, XML_THEME_COLOR, XML_TYPE_THEME_COLOR | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT("CharColorLumMod", XML_NAMESPACE_LO_EXT, XML_COLOR_LUM_MOD, XML_TYPE_PERCENT100 | XML_TYPE_PROP_TEXT, 0),
    MAP_EXT("CharColorLumOff", XML_NAMESPACE_LO_EXT, XML_COLOR_LUM_OFF, XML_TYPE_PERCENT100 | XML_TYPE_PROP_TEXT, 0),
    // RES_CHRATR_CONTOUR
    MT_E( CHAR_CONTOURED,  STYLE,  TEXT_OUTLINE,       XML_TYPE_BOOL,  0 ),
    // RES_CHRATR_CROSSEDOUT
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_STYLE,    XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY, 0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_TYPE,     XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,  0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_WIDTH,    XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY, 0),
    MT_E( CHAR_STRIKEOUT,  STYLE,  TEXT_LINE_THROUGH_TEXT,     XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,  0),
    // RES_CHRATR_ESCAPEMENT
    MT_E( CHAR_ESCAPEMENT,      STYLE, TEXT_POSITION,  XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MT_E( CHAR_ESCAPEMENT_HEIGHT, STYLE, TEXT_POSITION, XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_CHRATR_FONT
    MT_ED( CHAR_FONT_NAME,  STYLE,  FONT_NAME,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME ),
    MT_ED( CHAR_FONT_NAME,  FO,     FONT_FAMILY,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME ),
    MT_ED( CHAR_FONT_STYLE_NAME,STYLE,   FONT_STYLE_NAME,    XML_TYPE_STRING, CTF_FONTSTYLENAME ),
    MT_ED( CHAR_FONT_FAMILY,    STYLE,  FONT_FAMILY_GENERIC,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY ),
    MT_ED( CHAR_FONT_PITCH, STYLE,  FONT_PITCH,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH ),
    MT_ED( CHAR_FONT_CHAR_SET,   STYLE,  FONT_CHARSET,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET ),
    // RES_CHRATR_FONTSIZE
    MT_ED( CHAR_HEIGHT,          FO,   FONT_SIZE,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT ),
    MT_ED( CHAR_PROP_HEIGHT,FO, FONT_SIZE,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL ),
    MT_ED( CHAR_DIFF_HEIGHT,STYLE,FONT_SIZE_REL,        XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF ),
    // RES_CHRATR_KERNING
    MT_E( CHAR_KERNING,        FO,     LETTER_SPACING,     XML_TYPE_TEXT_KERNING, 0 ),
    // RES_CHRATR_LANGUAGE
    MT_ED( CHAR_LOCALE,     STYLE,     RFC_LANGUAGE_TAG,   XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     LANGUAGE,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     SCRIPT,             XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE,        FO,     COUNTRY,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_POSTURE
    MT_E( CHAR_POSTURE,        FO,     FONT_STYLE,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_UNUSED1
    // RES_CHRATR_SHADOWED
    MT_E( CHAR_SHADOWED,   FO,     TEXT_SHADOW,        XML_TYPE_TEXT_SHADOWED, 0 ),
    // VALIDATED UP TO THIS LINE
    // RES_CHRATR_UNDERLINE
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_STYLE,       XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, CTF_UNDERLINE ),
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_TYPE,        XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_UNDERLINE,  STYLE,  TEXT_UNDERLINE_WIDTH,       XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_UNDERLINE_COLOR, STYLE,  TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, CTF_UNDERLINE_COLOR  ),
    MT_E( CHAR_UNDERLINE_HAS_COLOR,  STYLE,  TEXT_UNDERLINE_COLOR,       XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, CTF_UNDERLINE_HASCOLOR   ),
    // RES_CHRATR_WEIGHT
    MT_E( CHAR_WEIGHT,     FO,     FONT_WEIGHT,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_RSID
    { RSID, XML_NAMESPACE_OFFICE_EXT, XML_RSID, XML_TYPE_HEX|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },
    // RES_PARATR_RSID
    { PAR_RSID, XML_NAMESPACE_OFFICE_EXT, XML_PARRSID, XML_TYPE_HEX|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },
    // RES_CHRATR_WORDLINEMODE
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_UNDERLINE_MODE,        XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_OVERLINE_MODE,     XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_WORD_MODE,   STYLE,  TEXT_LINE_THROUGH_MODE,     XML_TYPE_TEXT_LINE_MODE|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_AUTOKERN
    MT_E( CHAR_AUTO_KERNING,    STYLE,  LETTER_KERNING,     XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_BLINK
    MT_E( CHAR_FLASH,      STYLE,  TEXT_BLINKING,      XML_TYPE_BOOL, 0 ),
    // RES_CHRATR_NOHYPHEN
    // TODO: not used?
    // RES_CHRATR_UNUSED2
    // RES_CHRATR_BACKGROUND
    MT_E( CHAR_BACK_COLOR,  FO, BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, CTF_CHAR_BACKGROUND ),
    MT_E( CHAR_BACK_TRANSPARENT,    FO, BACKGROUND_COLOR, XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, CTF_CHAR_BACKGROUND_TRANSPARENCY),
    { CHAR_SHADING_VALUE, XML_NAMESPACE_LO_EXT, XML_CHAR_SHADING_VALUE, XML_TYPE_NUMBER|XML_TYPE_PROP_TEXT, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },
    MT_E( CHAR_BACK_COLOR,  FO, TEXT_BACKGROUND_COLOR, XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_OLDTEXTBACKGROUND ),
    // RES_CHRATR_CJK_FONT
    MT_ED( CHAR_FONT_NAME_ASIAN, STYLE,  FONT_NAME_ASIAN,            XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CJK ),
    MT_ED( CHAR_FONT_NAME_ASIAN, STYLE,      FONT_FAMILY_ASIAN,      XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CJK ),
    MT_ED( CHAR_FONT_STYLE_NAME_ASIAN,STYLE,  FONT_STYLE_NAME_ASIAN,  XML_TYPE_STRING, CTF_FONTSTYLENAME_CJK ),
    MT_ED( CHAR_FONT_FAMILY_ASIAN,   STYLE,  FONT_FAMILY_GENERIC_ASIAN,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CJK ),
    MT_ED( CHAR_FONT_PITCH_ASIAN,    STYLE,  FONT_PITCH_ASIAN,           XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CJK ),
    MT_ED( CHAR_FONT_CHAR_SET_ASIAN,  STYLE,  FONT_CHARSET_ASIAN,     XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CJK ),
    // RES_CHRATR_CJK_FONTSIZE
    MT_ED( CHAR_HEIGHT_ASIAN,         STYLE,    FONT_SIZE_ASIAN,            XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CJK ),
    MT_ED( CHAR_PROP_HEIGHT_ASIAN,STYLE, FONT_SIZE_ASIAN,            XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CJK ),
    MT_ED( CHAR_DIFF_HEIGHT_ASIAN,STYLE,FONT_SIZE_REL_ASIAN,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CJK ),
    // RES_CHRATR_CJK_LANGUAGE
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      RFC_LANGUAGE_TAG_ASIAN, XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      LANGUAGE_ASIAN,             XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      SCRIPT_ASIAN,           XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_ASIAN,       STYLE,      COUNTRY_ASIAN,          XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CJK_POSTURE
    MT_E( CHAR_POSTURE_ASIAN,       STYLE,      FONT_STYLE_ASIAN,           XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CJK_WEIGHT
    MT_E( CHAR_WEIGHT_ASIAN,        STYLE,      FONT_WEIGHT_ASIAN,      XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_CTL_FONT
    MT_ED( CHAR_FONT_NAME_COMPLEX,   STYLE,  FONT_NAME_COMPLEX,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CTL ),
    MT_ED( CHAR_FONT_NAME_COMPLEX,   STYLE,      FONT_FAMILY_COMPLEX,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CTL ),
    MT_ED( CHAR_FONT_STYLE_NAME_COMPLEX,STYLE,    FONT_STYLE_NAME_COMPLEX,    XML_TYPE_STRING, CTF_FONTSTYLENAME_CTL ),
    MT_ED( CHAR_FONT_FAMILY_COMPLEX, STYLE,  FONT_FAMILY_GENERIC_COMPLEX,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CTL ),
    MT_ED( CHAR_FONT_PITCH_COMPLEX,  STYLE,  FONT_PITCH_COMPLEX,         XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CTL ),
    MT_ED( CHAR_FONT_CHAR_SET_COMPLEX,    STYLE,  FONT_CHARSET_COMPLEX,       XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CTL ),
    // RES_CHRATR_CTL_FONTSIZE
    MT_ED( CHAR_HEIGHT_COMPLEX,       STYLE,    FONT_SIZE_COMPLEX,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CTL ),
    MT_ED( CHAR_PROP_HEIGHT_COMPLEX,STYLE,   FONT_SIZE_COMPLEX,          XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL_CTL ),
    MT_ED( CHAR_DIFF_HEIGHT_COMPLEX,STYLE,FONT_SIZE_REL_COMPLEX,     XML_TYPE_CHAR_HEIGHT_DIFF, CTF_CHARHEIGHT_DIFF_CTL ),
    // RES_CHRATR_CTL_LANGUAGE
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      RFC_LANGUAGE_TAG_COMPLEX,   XML_TYPE_CHAR_RFC_LANGUAGE_TAG|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      LANGUAGE_COMPLEX,           XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      SCRIPT_COMPLEX,             XML_TYPE_CHAR_SCRIPT|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_ED( CHAR_LOCALE_COMPLEX,     STYLE,      COUNTRY_COMPLEX,            XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 ),
    // RES_CHRATR_CTL_POSTURE
    MT_E( CHAR_POSTURE_COMPLEX,     STYLE,      FONT_STYLE_COMPLEX,         XML_TYPE_TEXT_POSTURE, 0 ),
    // RES_CHRATR_CTL_WEIGHT
    MT_E( CHAR_WEIGHT_COMPLEX,      STYLE,      FONT_WEIGHT_COMPLEX,        XML_TYPE_TEXT_WEIGHT, 0 ),
    // RES_CHRATR_ROTATE
    MT_E( CHAR_ROTATION,           STYLE,      TEXT_ROTATION_ANGLE,        XML_TYPE_TEXT_ROTATION_ANGLE, 0 ),
    MT_E( CHAR_ROTATION_IS_FIT_TO_LINE,    STYLE,      TEXT_ROTATION_SCALE,        XML_TYPE_TEXT_ROTATION_SCALE, 0 ),
    // RES_CHRATR_EMPHASIS_MARK
    MT_E( CHAR_EMPHASIS,           STYLE,      TEXT_EMPHASIZE,             XML_TYPE_TEXT_EMPHASIZE, 0 ),
    // RES_CHRATR_TWO_LINES
    MT_E( CHAR_COMBINE_IS_ON,            STYLE,      TEXT_COMBINE,               XML_TYPE_TEXT_COMBINE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MT_E( CHAR_COMBINE_PREFIX,      STYLE,      TEXT_COMBINE_START_CHAR,    XML_TYPE_TEXT_COMBINECHAR, 0 ),
    MT_E( CHAR_COMBINE_SUFFIX,      STYLE,      TEXT_COMBINE_END_CHAR,      XML_TYPE_TEXT_COMBINECHAR, 0 ),
    // RES_CHRATR_SCALEW
    MT_E( CHAR_SCALE_WIDTH,         STYLE,      TEXT_SCALE,                 XML_TYPE_PERCENT16, 0 ),
    // combined characters field, does not correspond to a property
    MT_E( EMPTY,                       STYLE,      TEXT_COMBINE,               XML_TYPE_TEXT_COMBINE_CHARACTERS|MID_FLAG_NO_PROPERTY, CTF_COMBINED_CHARACTERS_FIELD ),
    //RES_CHRATR_RELIEF
    MT_E( CHAR_RELIEF,             STYLE,      FONT_RELIEF,                XML_TYPE_TEXT_FONT_RELIEF, 0 ),
    // RES_CHRATR_HIDDEN
    MT_E( CHAR_HIDDEN,              TEXT,       DISPLAY,                    XML_TYPE_TEXT_HIDDEN_AS_DISPLAY|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_TEXT_DISPLAY ),
    // RES_CHRATR_OVERLINE
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_STYLE,        XML_TYPE_TEXT_OVERLINE_STYLE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_TYPE,     XML_TYPE_TEXT_OVERLINE_TYPE|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE,   STYLE,  TEXT_OVERLINE_WIDTH,        XML_TYPE_TEXT_OVERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE_COLOR,  STYLE,  TEXT_OVERLINE_COLOR,        XML_TYPE_TEXT_OVERLINE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    MT_E( CHAR_OVERLINE_HAS_COLOR,   STYLE,  TEXT_OVERLINE_COLOR,        XML_TYPE_TEXT_OVERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    // RES_CHRATR_BOX
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERWIDTH ),
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERWIDTH ),
    MAP_EXT( CHAR_RIGHT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERWIDTH ),
    MAP_EXT( CHAR_TOP_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERWIDTH ),
    MAP_EXT( CHAR_BOTTOM_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERWIDTH ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERWIDTH ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_LEFT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERWIDTH ),
    MAP_EXT_I( CHAR_RIGHT_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_RIGHT, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERWIDTH ),
    MAP_EXT_I( CHAR_TOP_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_TOP, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERWIDTH ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER, XML_NAMESPACE_STYLE, XML_BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERWIDTH ),

    MAP_EXT( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERDISTANCE ),
    MAP_EXT( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING_LEFT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERDISTANCE ),
    MAP_EXT( CHAR_RIGHT_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING_RIGHT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERDISTANCE ),
    MAP_EXT( CHAR_TOP_BORDER_DISTANCE, XML_NAMESPACE_LO_EXT, XML_PADDING_TOP, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERDISTANCE ),
    MAP_EXT( CHAR_BOTTOM_BORDER_DISTANCE,XML_NAMESPACE_LO_EXT, XML_PADDING_BOTTOM, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERDISTANCE ),
    MAP_EXT_I( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDERDISTANCE ),
    MAP_EXT_I( CHAR_LEFT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING_LEFT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDERDISTANCE ),
    MAP_EXT_I( CHAR_RIGHT_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING_RIGHT, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDERDISTANCE ),
    MAP_EXT_I( CHAR_TOP_BORDER_DISTANCE, XML_NAMESPACE_FO, XML_PADDING_TOP, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDERDISTANCE ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER_DISTANCE,XML_NAMESPACE_FO, XML_PADDING_BOTTOM, XML_TYPE_MEASURE|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDERDISTANCE ),

    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDER ),
    MAP_EXT( CHAR_LEFT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_LEFT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDER ),
    MAP_EXT( CHAR_RIGHT_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_RIGHT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDER ),
    MAP_EXT( CHAR_TOP_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_TOP, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDER ),
    MAP_EXT( CHAR_BOTTOM_BORDER, XML_NAMESPACE_LO_EXT, XML_BORDER_BOTTOM, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDER ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_FO, XML_BORDER, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARALLBORDER ),
    MAP_EXT_I( CHAR_LEFT_BORDER, XML_NAMESPACE_FO, XML_BORDER_LEFT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARLEFTBORDER ),
    MAP_EXT_I( CHAR_RIGHT_BORDER, XML_NAMESPACE_FO, XML_BORDER_RIGHT, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARRIGHTBORDER ),
    MAP_EXT_I( CHAR_TOP_BORDER, XML_NAMESPACE_FO, XML_BORDER_TOP, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARTOPBORDER ),
    MAP_EXT_I( CHAR_BOTTOM_BORDER, XML_NAMESPACE_FO, XML_BORDER_BOTTOM, XML_TYPE_BORDER|XML_TYPE_PROP_TEXT, CTF_CHARBOTTOMBORDER ),
    // RES_CHRATR_SHADOW
    MAP_EXT( CHAR_SHADOW_FORMAT, XML_NAMESPACE_LO_EXT, XML_SHADOW, XML_TYPE_TEXT_SHADOW|XML_TYPE_PROP_TEXT, 0 ),
    MAP_EXT_I( CHAR_SHADOW_FORMAT, XML_NAMESPACE_STYLE, XML_SHADOW, XML_TYPE_TEXT_SHADOW|XML_TYPE_PROP_TEXT, 0 ),
    // RES_CHRATR_HIGHLIGHT
    MT_E( CHAR_HIGHLIGHT,  FO, BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT, CTF_CHAR_HIGHLIGHT ),
    // RES_TXTATR_INETFMT
    MT_E( HYPER_LINK_URL,           TEXT,       XMLNS,                      XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_IMPORT,    CTF_HYPERLINK_URL ),
    // RES_TXTATR_REFMARK
    // TODO
    // RES_TXTATR_TOXMARK
    // TODO
    // RES_TXTATR_CHARFMT
    MT_E( CHAR_STYLE_NAME,          TEXT,       STYLE_NAME,                 XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_IMPORT,    CTF_CHAR_STYLE_NAME ),
    // RES_TXTATR_CJK_RUBY
    // TODO
    // RES_TXTATR_FIELD
    // TODO
    // RES_TXTATR_FLYCNT
    // TODO
    // RES_TXTATR_FTN
    // TODO
    // RES_TXTATR_SOFTHYPH
    // TODO
    // RES_TXTATR_HARDBLANK
    // TODO
    // RES_UNKNOWNATR_CONTAINER
    MT_E( TEXT_USER_DEFINED_ATTRIBUTES, TEXT, XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    MT_ED( PARA_IS_CHARACTER_DISTANCE, STYLE, TEXT_AUTOSPACE, XML_TYPE_TEXT_AUTOSPACE, 0 ),
    MT_ED( PARA_IS_HANGING_PUNCTUATION, STYLE, PUNCTUATION_WRAP, XML_TYPE_TEXT_PUNCTUATION_WRAP, 0 ),
    MT_ED( PARA_IS_FORBIDDEN_RULES, STYLE, LINE_BREAK, XML_TYPE_TEXT_LINE_BREAK, 0 ),
    MT_E( TAB_STOP_DISTANCE, STYLE, TAB_STOP_DISTANCE, XML_TYPE_MEASURE, 0 ),

    M_END()
};

XMLPropertyMapEntry const aXMLFramePropMap[] =
{
    // RES_FILL_ORDER
    // TODO: not required???
    // RES_FRM_SIZE
    MG_ED( WIDTH,         SVG, WIDTH,             XML_TYPE_MEASURE, CTF_FRAMEWIDTH_ABS ),
    MG_ED( WIDTH,         FO,  MIN_WIDTH,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_FRAMEWIDTH_MIN_ABS ),
    MG_ED( RELATIVE_WIDTH, FO,  MIN_WIDTH,         XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEWIDTH_MIN_REL ),
    MG_ED( RELATIVE_WIDTH, STYLE, REL_WIDTH,       XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEWIDTH_REL ),
    MG_ED( WIDTH_TYPE,     FO, TEXT_BOX,           XML_TYPE_NUMBER16|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FRAMEWIDTH_TYPE ),
//  M_ED( "RelativeWidth",  STYLE, REL_WIDTH,       XML_TYPE_TEXT_REL_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, 0 ),
//  M_ED( "IsSyncWidthToHeight",STYLE, REL_WIDTH,   XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, 0 ),

    MG_ED( HEIGHT,            SVG, HEIGHT,            XML_TYPE_MEASURE, CTF_FRAMEHEIGHT_ABS ),
    MG_ED( HEIGHT,            FO, MIN_HEIGHT,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_FRAMEHEIGHT_MIN_ABS ),
    MG_ED( RELATIVE_HEIGHT,    FO, MIN_HEIGHT,         XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEHEIGHT_MIN_REL ),
    MG_ED( RELATIVE_HEIGHT,    STYLE, REL_HEIGHT,      XML_TYPE_TEXT_REL_WIDTH_HEIGHT, CTF_FRAMEHEIGHT_REL ),
//  M_ED( "RelativeHeight", STYLE, REL_HEIGHT,      XML_TYPE_TEXT_REL_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_FRAMEHEIGHT_REL ),
//  M_ED( "IsSyncHeightToWidth",STYLE, REL_HEIGHT,  XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_SYNCHEIGHT ),
//  M_ED( "IsSyncHeightToWidth",STYLE, REL_HEIGHT,  XML_TYPE_TEXT_SYNC_WIDTH_HEIGHT_MIN, CTF_SYNCHEIGHT_MIN ),
    MG_ED( SIZE_TYPE,      FO, TEXT_BOX,           XML_TYPE_NUMBER16|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SIZETYPE ),
    // RES_PAPER_BIN
    // not required
    // RES_ANCHOR
    // moved to here because it is not used for automatic styles
    MG_ED( ANCHOR_TYPE,            TEXT,   ANCHOR_TYPE,            XML_TYPE_TEXT_ANCHOR_TYPE, CTF_ANCHORTYPE ),
    // AnchorPage number is not required for styles!
    MG_ED( HORI_ORIENT_POSITION,    SVG,    X,        XML_TYPE_MEASURE, 0 ),
    MG_ED( VERT_ORIENT_POSITION,    SVG,    Y,        XML_TYPE_MEASURE, 0 ),
    // ***** The map for automatic styles starts here *****
    // RES_LR_SPACE
    MG_E( LEFT_MARGIN,             FO, MARGIN,     XML_TYPE_MEASURE,  CTF_MARGINALL ),
    MG_E( LEFT_MARGIN,             FO, MARGIN_LEFT,        XML_TYPE_MEASURE,  CTF_MARGINLEFT ),
    MG_E( RIGHT_MARGIN,                FO, MARGIN_RIGHT,       XML_TYPE_MEASURE, CTF_MARGINRIGHT ),
    // RES_UL_SPACE
    MG_E( TOP_MARGIN,              FO, MARGIN_TOP,         XML_TYPE_MEASURE, CTF_MARGINTOP ),
    MG_E( BOTTOM_MARGIN,           FO, MARGIN_BOTTOM,      XML_TYPE_MEASURE, CTF_MARGINBOTTOM ),
    // RES_PAGEDESC
    // not required
    // RES_BREAK
    // not required
    // RES_CNTNT
    // not required (accessed using API)
    // RES_HEADER
    // not required
    // RES_FOOTER
    // not required
    // RES_PRINT
    MG_E( PRINT,                  STYLE,  PRINT_CONTENT,  XML_TYPE_BOOL, 0 ),
    // RES_OPAQUE
    MG_ED( OPAQUE,                    STYLE,  RUN_THROUGH,    XML_TYPE_TEXT_OPAQUE, 0 ),
    // RES_PROTECT
    MG_E( CONTENT_PROTECTED,       STYLE,  PROTECT,        XML_TYPE_TEXT_PROTECT_CONTENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MG_E( SIZE_PROTECTED,      STYLE,  PROTECT,        XML_TYPE_TEXT_PROTECT_SIZE|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MG_E( POSITION_PROTECTED,      STYLE,  PROTECT,        XML_TYPE_TEXT_PROTECT_POSITION|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_SURROUND
    MG_ED( TEXT_WRAP,              STYLE,  WRAP,   XML_TYPE_TEXT_WRAP, CTF_WRAP ),
    MG_ED( SURROUND_ANCHOR_ONLY,        STYLE,  NUMBER_WRAPPED_PARAGRAPHS,  XML_TYPE_TEXT_PARAGRAPH_ONLY, CTF_WRAP_PARAGRAPH_ONLY ),
    MG_E( SURROUND_CONTOUR,            STYLE,  WRAP_CONTOUR,   XML_TYPE_BOOL, CTF_WRAP_CONTOUR ),
    MG_E( CONTOUR_OUTSIDE,         STYLE,  WRAP_CONTOUR_MODE,  XML_TYPE_TEXT_WRAP_OUTSIDE, CTF_WRAP_CONTOUR_MODE ),
    // RES_VERT_ORIENT
    MG_ED( VERT_ORIENT,                STYLE,  VERTICAL_POS,         XML_TYPE_TEXT_VERTICAL_POS, CTF_VERTICALPOS ),
    MG_ED( VERT_ORIENT,                STYLE,  VERTICAL_POS,         XML_TYPE_TEXT_VERTICAL_POS_AT_CHAR, CTF_VERTICALPOS_ATCHAR ),
    MG_ED( VERT_ORIENT,                STYLE,  VERTICAL_REL,         XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR|MID_FLAG_MULTI_PROPERTY, CTF_VERTICALREL_ASCHAR ),
    MG_ED( VERT_ORIENT_RELATION,        STYLE,  VERTICAL_REL,         XML_TYPE_TEXT_VERTICAL_REL, CTF_VERTICALREL ),
    MG_ED( VERT_ORIENT_RELATION,        STYLE,  VERTICAL_REL,         XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_PAGE ),
    MG_ED( VERT_ORIENT_RELATION,        STYLE,  VERTICAL_REL,         XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_VERTICALREL_FRAME ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL|XML_TYPE_PROP_GRAPHIC|MID_FLAG_DEFAULT_ITEM_EXPORT, CTF_VERTICALREL ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT|XML_TYPE_PROP_GRAPHIC|MID_FLAG_DEFAULT_ITEM_EXPORT, CTF_VERTICALREL_PAGE ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT|XML_TYPE_PROP_GRAPHIC|MID_FLAG_DEFAULT_ITEM_EXPORT, CTF_VERTICALREL_FRAME ),
    // RES_HORI_ORIENT
    MG_ED( HORI_ORIENT,                STYLE,  HORIZONTAL_POS,       XML_TYPE_TEXT_HORIZONTAL_POS|MID_FLAG_MULTI_PROPERTY, CTF_HORIZONTALPOS ),
    MG_ED( PAGE_TOGGLE,        STYLE,  HORIZONTAL_POS,       XML_TYPE_TEXT_HORIZONTAL_MIRROR, CTF_HORIZONTALMIRROR ),
    MG_ED( HORI_ORIENT,                STYLE,  HORIZONTAL_POS,       XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALPOS_MIRRORED ),
    MG_ED( HORI_ORIENT_RELATION,        STYLE,  HORIZONTAL_REL,       XML_TYPE_TEXT_HORIZONTAL_REL, CTF_HORIZONTALREL ),
    MG_ED( HORI_ORIENT_RELATION,        STYLE,  HORIZONTAL_REL,       XML_TYPE_TEXT_HORIZONTAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_HORIZONTALREL_FRAME ),
    // RES_ANCHOR
    // see above
    // RES_BACKGROUND
    // DO NOT REORDER these!
    MG_ED( BACK_COLOR_RGB,  FO, BACKGROUND_COLOR,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    MG_ED( BACK_TRANSPARENT,   FO, BACKGROUND_COLOR,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, CTF_BACKGROUND_TRANSPARENT ),
    MG_ED( BACK_COLOR_TRANSPARENCY, STYLE, BACKGROUND_TRANSPARENCY, XML_TYPE_PERCENT8, CTF_BACKGROUND_TRANSPARENCY ),

    MG_E( BACK_GRAPHIC_TRANSPARENCY, STYLE, BACKGROUND_IMAGE_TRANSPARENCY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_PERCENT8, CTF_BACKGROUND_TRANSPARENCY ),
    MG_E( BACK_GRAPHIC_LOCATION,    STYLE,  POSITION,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    MG_E( BACK_GRAPHIC_FILTER,STYLE,    FILTER_NAME,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    MG_E( BACK_GRAPHIC, STYLE,  BACKGROUND_IMAGE,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_GRAPHIC, CTF_BACKGROUND_URL ),

    // fill attributes
    GMAP( FILL_STYLE,                      XML_NAMESPACE_DRAW, XML_FILL,                   XML_SW_TYPE_FILLSTYLE, 0 ),
    GMAP( FILL_COLOR,                      XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    GMAP( FILL_COLOR2,                     XML_NAMESPACE_DRAW, XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR, 0 ),
    GMAP( FILL_GRADIENT_NAME,               XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    GMAP( FILL_GRADIENT_STEP_COUNT,          XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16, 0 ),
    GMAP( FILL_HATCH_NAME,                  XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( FILL_BACKGROUND,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    GMAP( FILL_BITMAP_NAME,                 XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    GMAP( FILL_TRANSPARENCE,               XML_NAMESPACE_DRAW, XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),    // exists in SW, too
    GMAP( FILL_TRANSPARENCE_GRADIENT_NAME,   XML_NAMESPACE_DRAW, XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    GMAP( FILL_BITMAP_SIZE_X,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_SIZE_Y,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SW_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_MODE,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SW_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_X,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_Y,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_RECTANGLE_POINT,       XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SW_TYPE_BITMAP_REFPOINT, 0 ),
    GMAP( FILL_BITMAP_OFFSET_X,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    GMAP( FILL_BITMAP_OFFSET_Y,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SW_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // RES_BOX
    MG_ED( LEFT_BORDER,            STYLE,  BORDER_LINE_WIDTH,        XML_TYPE_BORDER_WIDTH, CTF_ALLBORDERWIDTH ),
    MG_ED( LEFT_BORDER,            STYLE,  BORDER_LINE_WIDTH_LEFT,   XML_TYPE_BORDER_WIDTH, CTF_LEFTBORDERWIDTH ),
    MG_ED( RIGHT_BORDER,           STYLE,  BORDER_LINE_WIDTH_RIGHT,  XML_TYPE_BORDER_WIDTH, CTF_RIGHTBORDERWIDTH ),
    MG_ED( TOP_BORDER,         STYLE,  BORDER_LINE_WIDTH_TOP,    XML_TYPE_BORDER_WIDTH, CTF_TOPBORDERWIDTH ),
    MG_ED( BOTTOM_BORDER,      STYLE,  BORDER_LINE_WIDTH_BOTTOM, XML_TYPE_BORDER_WIDTH, CTF_BOTTOMBORDERWIDTH ),

    MG_ED( LEFT_BORDER_DISTANCE,    FO,     PADDING,                  XML_TYPE_MEASURE, CTF_ALLBORDERDISTANCE ), // need special import filtering
    MG_ED( LEFT_BORDER_DISTANCE,    FO,     PADDING_LEFT,             XML_TYPE_MEASURE, CTF_LEFTBORDERDISTANCE ),
    MG_ED( RIGHT_BORDER_DISTANCE,   FO,     PADDING_RIGHT,            XML_TYPE_MEASURE, CTF_RIGHTBORDERDISTANCE ),
    MG_ED( TOP_BORDER_DISTANCE, FO,     PADDING_TOP,              XML_TYPE_MEASURE, CTF_TOPBORDERDISTANCE ),
    MG_ED( BOTTOM_BORDER_DISTANCE,FO,   PADDING_BOTTOM,           XML_TYPE_MEASURE, CTF_BOTTOMBORDERDISTANCE ),

    // There is an additional property for controls!
    MG_ED( LEFT_BORDER,            FO,     BORDER,                   XML_TYPE_BORDER|MID_FLAG_MULTI_PROPERTY, CTF_ALLBORDER ),
    MG_ED( LEFT_BORDER,            FO,     BORDER_LEFT,              XML_TYPE_BORDER, CTF_LEFTBORDER ),
    MG_ED( RIGHT_BORDER,           FO,     BORDER_RIGHT,             XML_TYPE_BORDER, CTF_RIGHTBORDER ),
    MG_ED( TOP_BORDER,         FO,     BORDER_TOP,               XML_TYPE_BORDER, CTF_TOPBORDER ),
    MG_ED( BOTTOM_BORDER,      FO,     BORDER_BOTTOM,            XML_TYPE_BORDER, CTF_BOTTOMBORDER ),
    // RES_SHADOW
    MG_E( SHADOW_FORMAT,       STYLE,  SHADOW,                 XML_TYPE_TEXT_SHADOW, 0 ),
    MG_E( SHADOW_TRANSPARENCE, DRAW, SHADOW_OPACITY, XML_TYPE_NEG_PERCENT, 0 ),
    // RES_FRMMACRO
    // TODO
    // RES_COL
    MG_E( TEXT_COLUMNS,            STYLE,  COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_TEXTCOLUMNS ),
    // RES_KEEP
    // not required
    // RES_URL
    // not required (exported as draw:a element)
    // RES_EDIT_IN_READONLY
    MG_ED( EDIT_IN_READONLY,        STYLE,  EDITABLE,                   XML_TYPE_BOOL, 0 ),
    // RES_LAYOUT_SPLIT
    // not required
    // RES_CHAIN
    // not required (exported at text:text-box element)
    // RES_LINENUMBER
    // not required
    // RES_FTN_AT_TXTEND
    // not required
    // RES_END_AT_TXTEND
    // not required
    // RES_COLUMNBALANCE
    // TODO
    // RES_UNKNOWNATR_CONTAINER
//  M_E_SE( TEXT, xmlns, RES_UNKNOWNATR_CONTAINER, 0 ),
    // RES_GRFATR_MIRRORGRF (vertical MUST be processed after horizontal!)
    MG_E( HORI_MIRRORED_ON_EVEN_PAGES,        STYLE,  MIRROR,     XML_TYPE_TEXT_MIRROR_HORIZONTAL_LEFT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MG_E( HORI_MIRRORED_ON_ODD_PAGES,     STYLE,  MIRROR,     XML_TYPE_TEXT_MIRROR_HORIZONTAL_RIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MG_E( VERT_MIRRORED,       STYLE,  MIRROR,     XML_TYPE_TEXT_MIRROR_VERTICAL|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 ),
    // RES_GRFATR_CROPGRF
    MG_EV( GRAPHIC_CROP,           FO,     CLIP,       XML_TYPE_TEXT_CLIP, CTF_TEXT_CLIP, SvtSaveOptions::ODFSVER_012 ),
    MG_E( GRAPHIC_CROP,            FO,     CLIP,       XML_TYPE_TEXT_CLIP11, CTF_TEXT_CLIP11 ),
    // RES_GRFATR_ROTATION
    // not required (exported as svg:transform attribute)
    // RES_GRFATR_LUMINANCE
    MG_E( ADJUST_LUMINANCE,  DRAW, LUMINANCE,          XML_TYPE_PERCENT16, 0 ),        // signed?
    // RES_GRFATR_CONTRAST
    MG_E( ADJUST_CONTRAST, DRAW,   CONTRAST,           XML_TYPE_PERCENT16, 0 ),        // signed?
    // RES_GRFATR_CHANNELR
    MG_E( ADJUST_RED,      DRAW, RED,                  XML_TYPE_PERCENT16, 0 ),        // signed?
    // RES_GRFATR_CHANNELG
    MG_E( ADJUST_GREEN,        DRAW, GREEN,                XML_TYPE_PERCENT16, 0 ),        // signed?
    // RES_GRFATR_CHANNELB
    MG_E( ADJUST_BLUE,     DRAW, BLUE,                 XML_TYPE_PERCENT16, 0 ),        // signed?
    // RES_GRFATR_GAMMA
    MG_E( GAMMA,          DRAW, GAMMA,                XML_TYPE_DOUBLE_PERCENT, 0 ),           // signed?
    // RES_GRFATR_INVERT
    MG_E( GRAPHIC_IS_INVERTED, DRAW, COLOR_INVERSION,       XML_TYPE_BOOL, 0 ),
    // RES_GRFATR_TRANSPARENCY
    MG_E( TRANSPARENCY,   DRAW, IMAGE_OPACITY,        XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ), // #i25616#
    // RES_GRFATR_DRAWMODE
    MG_E( GRAPHIC_COLOR_MODE, DRAW, COLOR_MODE,         XML_TYPE_COLOR_MODE, 0 ),
    MG_E( WRITING_MODE,      STYLE, WRITING_MODE,       XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT, 0 ),
    MAP_EXT_I(WRITING_MODE, XML_NAMESPACE_LO_EXT, XML_WRITING_MODE, XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT|XML_TYPE_PROP_GRAPHIC, 0),
    // RES_FOLLOW_TEXT_FLOW - DVO #i18732#
    MG_E( IS_FOLLOWING_TEXT_FLOW, DRAW, FLOW_WITH_TEXT,      XML_TYPE_BOOL|MID_FLAG_SPECIAL_ITEM_EXPORT, CTF_OLD_FLOW_WITH_TEXT ),
    MG_E( IS_FOLLOWING_TEXT_FLOW, STYLE, FLOW_WITH_TEXT,     XML_TYPE_BOOL, 0 ),
    // #i28701# - RES_WRAP_INFLUENCE_ON_OBJPOS
    MG_E( WRAP_INFLUENCE_ON_POSITION, DRAW, WRAP_INFLUENCE_ON_POSITION, XML_TYPE_WRAP_INFLUENCE_ON_POSITION, 0 ),
    MAP_EXT(ALLOW_OVERLAP, XML_NAMESPACE_LO_EXT, XML_ALLOW_OVERLAP, XML_TYPE_BOOL|XML_TYPE_PROP_GRAPHIC, 0),

    // special entries for floating frames
    MG_E( EMPTY,           DRAW,   FRAME_DISPLAY_SCROLLBAR,    XML_TYPE_BOOL|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY, CTF_FRAME_DISPLAY_SCROLLBAR ),
    MG_E( EMPTY,           DRAW,   FRAME_DISPLAY_BORDER,   XML_TYPE_BOOL|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY, CTF_FRAME_DISPLAY_BORDER ),
    MG_E( EMPTY,           DRAW,   FRAME_MARGIN_HORIZONTAL,    XML_TYPE_MEASURE_PX|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,   CTF_FRAME_MARGIN_HORI ),
    MG_E( EMPTY,           DRAW,   FRAME_MARGIN_VERTICAL,  XML_TYPE_MEASURE_PX|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,   CTF_FRAME_MARGIN_VERT ),
    MG_E( EMPTY,           DRAW,   VISIBLE_AREA_LEFT,      XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,  CTF_OLE_VIS_AREA_LEFT ),
    MG_E( EMPTY,           DRAW,   VISIBLE_AREA_TOP,       XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,  CTF_OLE_VIS_AREA_TOP ),
    MG_E( EMPTY,           DRAW,   VISIBLE_AREA_WIDTH,     XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,  CTF_OLE_VIS_AREA_WIDTH ),
    MG_E( EMPTY,           DRAW,   VISIBLE_AREA_HEIGHT,    XML_TYPE_MEASURE|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY,  CTF_OLE_VIS_AREA_HEIGHT ),
    MG_E( EMPTY,           DRAW,   DRAW_ASPECT,            XML_TYPE_TEXT_DRAW_ASPECT|MID_FLAG_NO_PROPERTY|MID_FLAG_MULTI_PROPERTY, CTF_OLE_DRAW_ASPECT ),
    MG_E( USER_DEFINED_ATTRIBUTES, TEXT, XMLNS, XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    MAP_EXT(RELATIVE_WIDTH_RELATION, XML_NAMESPACE_LO_EXT, XML_REL_WIDTH_REL, XML_TYPE_TEXT_HORIZONTAL_REL|XML_TYPE_PROP_GRAPHIC, CTF_RELWIDTHREL),
    MAP_EXT(RELATIVE_HEIGHT_RELATION, XML_NAMESPACE_LO_EXT, XML_REL_HEIGHT_REL, XML_TYPE_TEXT_VERTICAL_REL|XML_TYPE_PROP_GRAPHIC, CTF_RELHEIGHTREL),
    MG_E(TEXT_VERTICAL_ADJUST, DRAW, TEXTAREA_VERTICAL_ALIGN, XML_TYPE_VERTICAL_ALIGN, 0),

    M_END()
};

XMLPropertyMapEntry const aXMLShapePropMap[] =
{
    // RES_LR_SPACE
    MG_E( LEFT_MARGIN,             FO, MARGIN_LEFT,        XML_TYPE_MEASURE,  0),
    MG_E( RIGHT_MARGIN,                FO, MARGIN_RIGHT,       XML_TYPE_MEASURE, 0 ),
    // RES_UL_SPACE
    MG_E( TOP_MARGIN,              FO, MARGIN_TOP,         XML_TYPE_MEASURE, 0 ),
    MG_E( BOTTOM_MARGIN,           FO, MARGIN_BOTTOM,      XML_TYPE_MEASURE, 0 ),
    // RES_OPAQUE
    MG_ED( OPAQUE,                    STYLE,  RUN_THROUGH,    XML_TYPE_TEXT_OPAQUE, 0 ),
    // RES_SURROUND
    MG_E( TEXT_WRAP,               STYLE,  WRAP,   XML_TYPE_TEXT_WRAP, CTF_WRAP ),
    MG_E( SURROUND_ANCHOR_ONLY,     STYLE,  NUMBER_WRAPPED_PARAGRAPHS,  XML_TYPE_TEXT_PARAGRAPH_ONLY, CTF_WRAP_PARAGRAPH_ONLY ),
    MG_E( SURROUND_CONTOUR,            STYLE,  WRAP_CONTOUR,   XML_TYPE_BOOL, CTF_WRAP_CONTOUR ),
    MG_E( CONTOUR_OUTSIDE,         STYLE,  WRAP_CONTOUR_MODE,  XML_TYPE_TEXT_WRAP_OUTSIDE, CTF_WRAP_CONTOUR_MODE ),
    // Use own CTF ids for positioning attributes (#i28749#)
    // RES_VERT_ORIENT
    MG_E( VERT_ORIENT,         STYLE,  VERTICAL_POS,   XML_TYPE_TEXT_VERTICAL_POS, CTF_SHAPE_VERTICALPOS ),
    // Add property for at-character anchored shapes (#i26791#)
    MG_E( VERT_ORIENT,         STYLE,  VERTICAL_POS,   XML_TYPE_TEXT_VERTICAL_POS_AT_CHAR, CTF_SHAPE_VERTICALPOS_ATCHAR ),
    MG_E( VERT_ORIENT,         STYLE,  VERTICAL_REL,   XML_TYPE_TEXT_VERTICAL_REL_AS_CHAR|MID_FLAG_MULTI_PROPERTY, CTF_VERTICALREL_ASCHAR ),
    MG_E( VERT_ORIENT_RELATION, STYLE,  VERTICAL_REL,   XML_TYPE_TEXT_VERTICAL_REL, CTF_SHAPE_VERTICALREL ),
    MG_E( VERT_ORIENT_RELATION, STYLE,  VERTICAL_REL,   XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SHAPE_VERTICALREL_PAGE ),
    MG_E( VERT_ORIENT_RELATION, STYLE,  VERTICAL_REL,   XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SHAPE_VERTICALREL_FRAME ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL|XML_TYPE_PROP_GRAPHIC, CTF_VERTICALREL ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL_PAGE|MID_FLAG_SPECIAL_ITEM_IMPORT|XML_TYPE_PROP_GRAPHIC, CTF_VERTICALREL_PAGE ),
    MAP_EXT_I( VERT_ORIENT_RELATION,    XML_NAMESPACE_LO_EXT, XML_VERTICAL_REL, XML_TYPE_TEXT_VERTICAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT|XML_TYPE_PROP_GRAPHIC, CTF_VERTICALREL_FRAME ),
    // RES_HORI_ORIENT
    MG_E( HORI_ORIENT,         STYLE,  HORIZONTAL_POS, XML_TYPE_TEXT_HORIZONTAL_POS|MID_FLAG_MULTI_PROPERTY, CTF_SHAPE_HORIZONTALPOS ),
    MG_E( PAGE_TOGGLE,         STYLE,  HORIZONTAL_POS, XML_TYPE_TEXT_HORIZONTAL_MIRROR, CTF_SHAPE_HORIZONTALMIRROR ),
    MG_E( HORI_ORIENT,         STYLE,  HORIZONTAL_POS, XML_TYPE_TEXT_HORIZONTAL_POS_MIRRORED|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SHAPE_HORIZONTALPOS_MIRRORED ),
    MG_E( HORI_ORIENT_RELATION, STYLE,  HORIZONTAL_REL, XML_TYPE_TEXT_HORIZONTAL_REL, CTF_SHAPE_HORIZONTALREL ),
    MG_E( HORI_ORIENT_RELATION, STYLE,  HORIZONTAL_REL, XML_TYPE_TEXT_HORIZONTAL_REL_FRAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_SHAPE_HORIZONTALREL_FRAME ),
    // RES_WRAP_INFLUENCE_ON_OBJPOS (#i28701#)
    MG_ED( WRAP_INFLUENCE_ON_POSITION, DRAW, WRAP_INFLUENCE_ON_POSITION, XML_TYPE_WRAP_INFLUENCE_ON_POSITION, 0 ),
    MAP_EXT(ALLOW_OVERLAP, XML_NAMESPACE_LO_EXT, XML_ALLOW_OVERLAP, XML_TYPE_BOOL|XML_TYPE_PROP_GRAPHIC, 0),
    // UserDefinedAttributes is already contained in the map this one is
    // chained to.

    // RES_FOLLOW_TEXT_FLOW (#i26791#)
    MG_ED( IS_FOLLOWING_TEXT_FLOW, STYLE, FLOW_WITH_TEXT,      XML_TYPE_BOOL, 0 ),

    // RES_FRM_SIZE
    MAP_EXT(RELATIVE_WIDTH_RELATION, XML_NAMESPACE_LO_EXT, XML_REL_WIDTH_REL, XML_TYPE_TEXT_HORIZONTAL_REL|XML_TYPE_PROP_GRAPHIC, CTF_RELWIDTHREL),
    MAP_EXT(RELATIVE_HEIGHT_RELATION, XML_NAMESPACE_LO_EXT, XML_REL_HEIGHT_REL, XML_TYPE_TEXT_VERTICAL_REL|XML_TYPE_PROP_GRAPHIC, CTF_RELHEIGHTREL),

    M_END()
};

XMLPropertyMapEntry const aXMLSectionPropMap[] =
{
    // RES_COL
    MS_E( TEXT_COLUMNS,            STYLE,  COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_TEXTCOLUMNS ),

    // RES_BACKGROUND
    // DO NOT REORDER these!
    MS_E( BACK_COLOR,  FO, BACKGROUND_COLOR,       XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    MS_E( BACK_TRANSPARENT,    FO, BACKGROUND_COLOR,       XML_TYPE_ISTRANSPARENT|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    MS_E( BACK_GRAPHIC_LOCATION,    STYLE,  POSITION,   MID_FLAG_SPECIAL_ITEM|XML_TYPE_BUILDIN_CMP_ONLY, CTF_BACKGROUND_POS  ),
    MS_E( BACK_GRAPHIC_FILTER,STYLE,    FILTER_NAME,    MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING, CTF_BACKGROUND_FILTER ),
    MS_E( BACK_GRAPHIC, STYLE,  BACKGROUND_IMAGE,   MID_FLAG_ELEMENT_ITEM|XML_TYPE_GRAPHIC, CTF_BACKGROUND_URL ),

    // move protect-flag into section element
//  M_E( "IsProtected",         STYLE,  PROTECT,    XML_TYPE_BOOL, 0 ),

    MS_E( DONT_BALANCE_TEXT_COLUMNS, TEXT, DONT_BALANCE_TEXT_COLUMNS, XML_TYPE_BOOL, 0 ),

    MS_E( WRITING_MODE,      STYLE, WRITING_MODE,       XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT, 0 ),

    MS_E( SECTION_LEFT_MARGIN,      FO, MARGIN_LEFT,        XML_TYPE_MEASURE,  0),
    MS_E( SECTION_RIGHT_MARGIN,     FO, MARGIN_RIGHT,       XML_TYPE_MEASURE,  0),

    // section footnote settings
    MS_E( FOOTNOTE_IS_OWN_NUMBERING,     TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_NUM_OWN ),
    MS_E( FOOTNOTE_IS_RESTART_NUMBERING, TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_NUM_RESTART ),
    MS_E( FOOTNOTE_RESTART_NUMBERING_AT, TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_FOOTNOTE_NUM_RESTART_AT ),
    MS_E( FOOTNOTE_NUMBERING_TYPE,      TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_FOOTNOTE_NUM_TYPE ),
    MS_E( FOOTNOTE_NUMBERING_PREFIX,        TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_FOOTNOTE_NUM_PREFIX ),
    MS_E( FOOTNOTE_NUMBERING_SUFFIX,        TEXT,   _EMPTY, MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_FOOTNOTE_NUM_SUFFIX ),
    MS_E( FOOTNOTE_IS_COLLECT_AT_TEXT_END, TEXT,   NOTES_CONFIGURATION,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_BOOL,    CTF_SECTION_FOOTNOTE_END ),

    // section footnote settings
    MS_E( ENDNOTE_IS_OWN_NUMBERING,      TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_NUM_OWN ),
    MS_E( ENDNOTE_IS_RESTART_NUMBERING,  TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_NUM_RESTART ),
    MS_E( ENDNOTE_RESTART_NUMBERING_AT,  TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_ENDNOTE_NUM_RESTART_AT ),
    MS_E( ENDNOTE_NUMBERING_TYPE,       TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_NUMBER16,CTF_SECTION_ENDNOTE_NUM_TYPE ),
    MS_E( ENDNOTE_NUMBERING_PREFIX,     TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_ENDNOTE_NUM_PREFIX ),
    MS_E( ENDNOTE_NUMBERING_SUFFIX,     TEXT,   _EMPTY,     MID_FLAG_SPECIAL_ITEM|XML_TYPE_STRING,  CTF_SECTION_ENDNOTE_NUM_SUFFIX ),
    MS_E( ENDNOTE_IS_COLLECT_AT_TEXT_END,  TEXT,   NOTES_CONFIGURATION,        MID_FLAG_ELEMENT_ITEM|XML_TYPE_BOOL,    CTF_SECTION_ENDNOTE_END ),
    MS_E( USER_DEFINED_ATTRIBUTES,      TEXT,   XMLNS,      XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),
    // RES_EDIT_IN_READONLY
    MS_E( EDIT_IN_READONLY,             STYLE,  EDITABLE,  XML_TYPE_BOOL, 0 ),
    M_END()
};

XMLPropertyMapEntry const aXMLRubyPropMap[] =
{
    MR_E( RUBY_ADJUST, STYLE, RUBY_ALIGN, XML_TYPE_TEXT_RUBY_ADJUST, 0 ),
    MR_E( RUBY_IS_ABOVE,    STYLE, RUBY_POSITION, XML_TYPE_TEXT_RUBY_IS_ABOVE, 0 ),
    MR_EV( RUBY_POSITION,   LO_EXT, RUBY_POSITION, XML_TYPE_TEXT_RUBY_POSITION, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    M_END()
};


XMLPropertyMapEntry const aXMLTableDefaultsMap[] =
{
    // RES_COLLAPSING_BORDERS: only occurs in tables, but we need to
    // read/write the default for this item
    M_ED_( COLLAPSING_BORDERS, TABLE, BORDER_MODEL, XML_TYPE_PROP_TABLE | XML_TYPE_BORDER_MODEL | MID_FLAG_NO_PROPERTY_IMPORT, CTF_BORDER_MODEL ),

    M_END()
};

XMLPropertyMapEntry const aXMLTableRowDefaultsMap[] =
{
    // RES_ROW_SPLIT: only occurs in table rows, but we need to
    // read/write the default for this item
    M_ED_( IS_SPLIT_ALLOWED, FO, KEEP_TOGETHER, XML_TYPE_PROP_TABLE_ROW | XML_TYPE_TEXT_NKEEP | MID_FLAG_NO_PROPERTY_IMPORT, CTF_KEEP_TOGETHER ),

    M_END()
};

XMLPropertyMapEntry const aXMLCellPropMap[] =
{
    MC_E( BACK_COLOR,            FO,    BACKGROUND_COLOR, XML_TYPE_COLORTRANSPARENT|MID_FLAG_MULTI_PROPERTY, 0 ),
    MC_E( LEFT_BORDER,           FO,    BORDER_LEFT,      XML_TYPE_BORDER,                                   0 ),
    MC_E( RIGHT_BORDER,          FO,    BORDER_RIGHT,     XML_TYPE_BORDER,                                   0 ),
    MC_E( TOP_BORDER,            FO,    BORDER_TOP,       XML_TYPE_BORDER,                                   0 ),
    MC_E( BOTTOM_BORDER,         FO,    BORDER_BOTTOM,    XML_TYPE_BORDER,                                   0 ),
    MC_E( BORDER_DISTANCE,       FO,    PADDING,          XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY,          0 ),
    MC_E( LEFT_BORDER_DISTANCE,   FO,    PADDING_LEFT,     XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY,          0 ),
    MC_E( RIGHT_BORDER_DISTANCE,  FO,    PADDING_RIGHT,    XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY,          0 ),
    MC_E( TOP_BORDER_DISTANCE,    FO,    PADDING_TOP,      XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY,          0 ),
    MC_E( BOTTOM_BORDER_DISTANCE, FO,    PADDING_BOTTOM,   XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY,          0 ),
    MC_E( VERT_ORIENT,           STYLE, VERTICAL_ALIGN,   XML_TYPE_TEXT_VERTICAL_POS,                        0 ),
    MC_E( WRITING_MODE,          STYLE, WRITING_MODE,     XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT,           0 ),
    MC_E( NUMBER_FORMAT,         STYLE, DATA_STYLE_NAME,  XML_TYPE_NUMBER|MID_FLAG_SPECIAL_ITEM_EXPORT,      0 ),
    // paragraph properties
    MP_E( PARA_ADJUST,           FO,    TEXT_ALIGN,       XML_TYPE_TEXT_ADJUST,                              0 ),
    // text properties
    MT_ED( CHAR_COLOR,           FO,    COLOR,                    XML_TYPE_COLORAUTO|MID_FLAG_MERGE_PROPERTY,                0 ),
    MT_ED( CHAR_COLOR,           STYLE, USE_WINDOW_FONT_COLOR,    XML_TYPE_ISAUTOCOLOR|MID_FLAG_MERGE_PROPERTY,              0 ),
    MT_E( CHAR_SHADOWED,         FO,    TEXT_SHADOW,              XML_TYPE_TEXT_SHADOWED,                                    0 ),
    MT_E( CHAR_CONTOURED,        STYLE, TEXT_OUTLINE,             XML_TYPE_BOOL,                                             0 ),
    MT_E( CHAR_STRIKEOUT,        STYLE, TEXT_LINE_THROUGH_STYLE,  XML_TYPE_TEXT_CROSSEDOUT_STYLE|MID_FLAG_MERGE_PROPERTY,    0 ),
    MT_E( CHAR_STRIKEOUT,        STYLE, TEXT_LINE_THROUGH_TYPE,   XML_TYPE_TEXT_CROSSEDOUT_TYPE|MID_FLAG_MERGE_PROPERTY,     0 ),
    MT_E( CHAR_STRIKEOUT,        STYLE, TEXT_LINE_THROUGH_WIDTH,  XML_TYPE_TEXT_CROSSEDOUT_WIDTH|MID_FLAG_MERGE_PROPERTY,    0 ),
    MT_E( CHAR_STRIKEOUT,        STYLE, TEXT_LINE_THROUGH_TEXT,   XML_TYPE_TEXT_CROSSEDOUT_TEXT|MID_FLAG_MERGE_PROPERTY,     0 ),
    MT_E( CHAR_UNDERLINE,        STYLE, TEXT_UNDERLINE_STYLE,     XML_TYPE_TEXT_UNDERLINE_STYLE|MID_FLAG_MERGE_PROPERTY,     0 ),
    MT_E( CHAR_UNDERLINE,        STYLE, TEXT_UNDERLINE_TYPE,      XML_TYPE_TEXT_UNDERLINE_TYPE|MID_FLAG_MERGE_PROPERTY,      0 ),
    MT_E( CHAR_UNDERLINE,        STYLE, TEXT_UNDERLINE_WIDTH,     XML_TYPE_TEXT_UNDERLINE_WIDTH|MID_FLAG_MERGE_PROPERTY,     0 ),
    MT_E( CHAR_UNDERLINE_COLOR,   STYLE, TEXT_UNDERLINE_COLOR,     XML_TYPE_TEXT_UNDERLINE_COLOR|MID_FLAG_MULTI_PROPERTY,     0 ),
    MT_E( CHAR_UNDERLINE_HAS_COLOR,STYLE, TEXT_UNDERLINE_COLOR,     XML_TYPE_TEXT_UNDERLINE_HASCOLOR|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    // STANDARD FONT
    MT_ED( CHAR_HEIGHT,          FO,    FONT_SIZE,                XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY,              0 ),
    MT_E( CHAR_WEIGHT,           FO,    FONT_WEIGHT,              XML_TYPE_TEXT_WEIGHT,                                      0 ),
    MT_E( CHAR_POSTURE,          FO,    FONT_STYLE,               XML_TYPE_TEXT_POSTURE,                                     0 ),
    // RES_CHRATR_FONT
    MT_ED( CHAR_FONT_NAME,        STYLE, FONT_NAME,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT,              CTF_FONTNAME       ),
    MT_ED( CHAR_FONT_NAME,        FO,    FONT_FAMILY,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME ),
    MT_ED( CHAR_FONT_STYLE_NAME,   STYLE, FONT_STYLE_NAME,    XML_TYPE_STRING,                                           CTF_FONTSTYLENAME  ),
    MT_ED( CHAR_FONT_FAMILY,      STYLE, FONT_FAMILY_GENERIC,XML_TYPE_TEXT_FONTFAMILY,                                  CTF_FONTFAMILY     ),
    MT_ED( CHAR_FONT_PITCH,       STYLE, FONT_PITCH,         XML_TYPE_TEXT_FONTPITCH,                                   CTF_FONTPITCH      ),
    MT_ED( CHAR_FONT_CHAR_SET,     STYLE, FONT_CHARSET,       XML_TYPE_TEXT_FONTENCODING,                                CTF_FONTCHARSET    ),
    // CJK FONT
    MT_ED( CHAR_HEIGHT_ASIAN,         STYLE, FONT_SIZE_ASIAN,            XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY,  0                  ),
    MT_E( CHAR_WEIGHT_ASIAN,          STYLE, FONT_WEIGHT_ASIAN,          XML_TYPE_TEXT_WEIGHT,                          0                  ),
    MT_E( CHAR_POSTURE_ASIAN,         STYLE, FONT_STYLE_ASIAN,           XML_TYPE_TEXT_POSTURE,                         0                  ),
    // RES_CHRATR_CJK_FONT
    MT_ED( CHAR_FONT_NAME_ASIAN,       STYLE, FONT_NAME_ASIAN,            XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT,              CTF_FONTNAME_CJK       ),
    MT_ED( CHAR_FONT_NAME_ASIAN,       STYLE, FONT_FAMILY_ASIAN,          XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CJK ),
    MT_ED( CHAR_FONT_STYLE_NAME_ASIAN,  STYLE, FONT_STYLE_NAME_ASIAN,      XML_TYPE_STRING,                                           CTF_FONTSTYLENAME_CJK  ),
    MT_ED( CHAR_FONT_FAMILY_ASIAN,     STYLE, FONT_FAMILY_GENERIC_ASIAN,  XML_TYPE_TEXT_FONTFAMILY,                                  CTF_FONTFAMILY_CJK     ),
    MT_ED( CHAR_FONT_PITCH_ASIAN,      STYLE, FONT_PITCH_ASIAN,           XML_TYPE_TEXT_FONTPITCH,                                   CTF_FONTPITCH_CJK      ),
    MT_ED( CHAR_FONT_CHAR_SET_ASIAN,    STYLE, FONT_CHARSET_ASIAN,         XML_TYPE_TEXT_FONTENCODING,                                CTF_FONTCHARSET_CJK    ),
    // CTL FONT
    MT_ED( CHAR_HEIGHT_COMPLEX,       STYLE, FONT_SIZE_COMPLEX,          XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY,              0                      ),
    MT_E( CHAR_WEIGHT_COMPLEX,        STYLE, FONT_WEIGHT_COMPLEX,        XML_TYPE_TEXT_WEIGHT,                                      0                      ),
    MT_E( CHAR_POSTURE_COMPLEX,       STYLE, FONT_STYLE_COMPLEX,         XML_TYPE_TEXT_POSTURE,                                     0                      ),
    // RES_CHRATR_CTL_FONT
    MT_ED( CHAR_FONT_NAME_COMPLEX,     STYLE, FONT_NAME_COMPLEX,          XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT,              CTF_FONTNAME_CTL       ),
    MT_ED( CHAR_FONT_NAME_COMPLEX,     STYLE, FONT_FAMILY_COMPLEX,        XML_TYPE_TEXT_FONTFAMILYNAME|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTFAMILYNAME_CTL ),
    MT_ED( CHAR_FONT_STYLE_NAME_COMPLEX,STYLE, FONT_STYLE_NAME_COMPLEX,    XML_TYPE_STRING,                                           CTF_FONTSTYLENAME_CTL  ),
    MT_ED( CHAR_FONT_FAMILY_COMPLEX,   STYLE, FONT_FAMILY_GENERIC_COMPLEX,XML_TYPE_TEXT_FONTFAMILY,                                  CTF_FONTFAMILY_CTL     ),
    MT_ED( CHAR_FONT_PITCH_COMPLEX,    STYLE, FONT_PITCH_COMPLEX,         XML_TYPE_TEXT_FONTPITCH,                                   CTF_FONTPITCH_CTL      ),
    MT_ED( CHAR_FONT_CHAR_SET_COMPLEX,  STYLE, FONT_CHARSET_COMPLEX,       XML_TYPE_TEXT_FONTENCODING,                                CTF_FONTCHARSET_CTL    ),

    M_END()
};

static XMLPropertyMapEntry const *lcl_txtprmap_getMap( TextPropMap nType )
{
    XMLPropertyMapEntry const *pMap = nullptr;
    switch( nType )
    {
    case TextPropMap::TEXT:
        pMap = aXMLTextPropMap;
        break;
    case TextPropMap::SHAPE_PARA:
        // #i125045# use [21] instead of [1] for text props for Shapes, indices
        // [1..20] contain the DrawingLayer FillStyle attributes corresponding to
        // [XATTR_FILL_FIRST .. XATTR_FILL_LAST] and would be double since Shapes
        // already contain these (usually in aXMLSDProperties)
        pMap = &(aXMLParaPropMap[21]);
        assert( pMap->meXMLName == XML_MARGIN && " shape para map changed" );
        break;
    case TextPropMap::PARA:
        pMap = aXMLParaPropMap;
        break;
    case TextPropMap::FRAME:
        pMap = aXMLFramePropMap;
        break;
    case TextPropMap::AUTO_FRAME:
        pMap = &(aXMLFramePropMap[13]);
        assert( pMap->meXMLName == XML_MARGIN && " frame map changed" );
        break;
    case TextPropMap::SHAPE:
        pMap = aXMLShapePropMap;
        break;
    case TextPropMap::SECTION:
        pMap = aXMLSectionPropMap;
        break;
    case TextPropMap::RUBY:
        pMap = aXMLRubyPropMap;
        break;
    case TextPropMap::TEXT_ADDITIONAL_DEFAULTS:
        pMap = aXMLAdditionalTextDefaultsMap;
        break;
    case TextPropMap::TABLE_DEFAULTS:
        pMap = aXMLTableDefaultsMap;
        break;
    case TextPropMap::TABLE_ROW_DEFAULTS:
        pMap = aXMLTableRowDefaultsMap;
        break;
    case TextPropMap::CELL:
        pMap = aXMLCellPropMap;
        break;
    }
    SAL_WARN_IF( !pMap, "xmloff", "illegal map type" );
    return pMap;
}

const XMLPropertyMapEntry* XMLTextPropertySetMapper::getPropertyMapForType( TextPropMap _nType )
{
    return lcl_txtprmap_getMap( _nType );
}

XMLTextPropertySetMapper::XMLTextPropertySetMapper( TextPropMap nType, bool bForExport ) :
    XMLPropertySetMapper( lcl_txtprmap_getMap( nType ),
                          new XMLTextPropertyHandlerFactory, bForExport )
{
}

XMLTextPropertySetMapper::~XMLTextPropertySetMapper()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
