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

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <xmloff/EnumPropertyHdl.hxx>
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <WordWrapPropertyHdl.hxx>
#include <enummaps.hxx>
#include "numithdl.hxx"
#include <XMLBitmapRepeatOffsetPropertyHandler.hxx>
#include <XMLFillBitmapSizePropertyHandler.hxx>
#include <XMLBitmapLogicalSizePropertyHandler.hxx>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#include <xmloff/controlpropertyhdl.hxx>
#include <xmloff/xmltoken.hxx>
#include "sdpropls.hxx"
#include <propimp0.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/txtprmap.hxx>
#include <XMLClipPropertyHandler.hxx>
#include <XMLIsPercentagePropertyHandler.hxx>
#include <XMLPercentOrMeasurePropertyHandler.hxx>
#include <XMLTextColumnsPropertyHandler.hxx>
#include <animations.hxx>
#include <sax/tools/converter.hxx>
#include <xmlsdtypes.hxx>

using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

#define MAP_(name,prefix,token,type,context)  { name, prefix, token, type, context, SvtSaveOptions::ODFSVER_010, false }
#define MAPV_(name,prefix,token,type,context,version)  { name, prefix, token, type, context, version, false }
#define GMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_GRAPHIC,context)
#define GMAP_D(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_GRAPHIC|MID_FLAG_DEFAULT_ITEM_EXPORT,context)
#define GMAPV(name,prefix,token,type,context,version) MAPV_(name,prefix,token,type|XML_TYPE_PROP_GRAPHIC,context,version)
#define DPMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_DRAWING_PAGE,context)
#define TMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_TEXT,context)
#define PMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_PARAGRAPH,context)
#define MAP_END() { nullptr, 0, XML_EMPTY, 0 ,0, SvtSaveOptions::ODFSVER_010, false }

// entry list for graphic properties

constexpr OUStringLiteral USER_DEFINED_ATTRIBUTES = u"UserDefinedAttributes";
constexpr OUStringLiteral LINE_STYLE = u"LineStyle";
constexpr OUStringLiteral ADJUST_BLUE = u"AdjustBlue";
constexpr OUStringLiteral ADJUST_CONTRAST = u"AdjustContrast";
constexpr OUStringLiteral ADJUST_GREEN = u"AdjustGreen";
constexpr OUStringLiteral ADJUST_LUMINANCE = u"AdjustLuminance";
constexpr OUStringLiteral ADJUST_RED = u"AdjustRed";
constexpr OUStringLiteral ASPECT = u"Aspect";
constexpr OUStringLiteral BACKGROUND_FULL_SIZE = u"BackgroundFullSize";
constexpr OUStringLiteral CAPTION_ANGLE = u"CaptionAngle";
constexpr OUStringLiteral CAPTION_ESCAPE_ABSOLUTE = u"CaptionEscapeAbsolute";
constexpr OUStringLiteral CAPTION_ESCAPE_DIRECTION = u"CaptionEscapeDirection";
constexpr OUStringLiteral CAPTION_ESCAPE_RELATIVE = u"CaptionEscapeRelative";
constexpr OUStringLiteral CAPTION_GAP = u"CaptionGap";
constexpr OUStringLiteral CAPTION_IS_ESCAPE_RELATIVE = u"CaptionIsEscapeRelative";
constexpr OUStringLiteral CAPTION_IS_FIT_LINE_LENGTH = u"CaptionIsFitLineLength";
constexpr OUStringLiteral CAPTION_IS_FIXED_ANGLE = u"CaptionIsFixedAngle";
constexpr OUStringLiteral CAPTION_LINE_LENGTH = u"CaptionLineLength";
constexpr OUStringLiteral CAPTION_TYPE = u"CaptionType";
constexpr OUStringLiteral CHANGE = u"Change";
constexpr OUStringLiteral CONTROL_BACKGROUND = u"ControlBackground";
constexpr OUStringLiteral CONTROL_BORDER = u"ControlBorder";
constexpr OUStringLiteral CONTROL_BORDER_COLOR = u"ControlBorderColor";
constexpr OUStringLiteral CONTROL_DATA_STYLE = u"ControlDataStyle";
constexpr OUStringLiteral CONTROL_SYMBOL_COLOR = u"ControlSymbolColor";
constexpr OUStringLiteral CONTROL_TEXT_EMPHASIS = u"ControlTextEmphasis";
constexpr OUStringLiteral CONTROL_WRITING_MODE = u"ControlWritingMode";
constexpr OUStringLiteral D3_DBACKSCALE = u"D3DBackscale";
constexpr OUStringLiteral D3_DCLOSE_BACK = u"D3DCloseBack";
constexpr OUStringLiteral D3_DCLOSE_FRONT = u"D3DCloseFront";
constexpr OUStringLiteral D3_DDEPTH = u"D3DDepth";
constexpr OUStringLiteral D3_DDOUBLE_SIDED = u"D3DDoubleSided";
constexpr OUStringLiteral D3_DEND_ANGLE = u"D3DEndAngle";
constexpr OUStringLiteral D3_DHORIZONTAL_SEGMENTS = u"D3DHorizontalSegments";
constexpr OUStringLiteral D3_DMATERIAL_COLOR = u"D3DMaterialColor";
constexpr OUStringLiteral D3_DMATERIAL_EMISSION = u"D3DMaterialEmission";
constexpr OUStringLiteral D3_DMATERIAL_SPECULAR = u"D3DMaterialSpecular";
constexpr OUStringLiteral D3_DMATERIAL_SPECULAR_INTENSITY = u"D3DMaterialSpecularIntensity";
constexpr OUStringLiteral D3_DNORMALS_INVERT = u"D3DNormalsInvert";
constexpr OUStringLiteral D3_DNORMALS_KIND = u"D3DNormalsKind";
constexpr OUStringLiteral D3_DPERCENT_DIAGONAL = u"D3DPercentDiagonal";
constexpr OUStringLiteral D3_DSHADOW3_D = u"D3DShadow3D";
constexpr OUStringLiteral D3_DTEXTURE_FILTER = u"D3DTextureFilter";
constexpr OUStringLiteral D3_DTEXTURE_KIND = u"D3DTextureKind";
constexpr OUStringLiteral D3_DTEXTURE_MODE = u"D3DTextureMode";
constexpr OUStringLiteral D3_DTEXTURE_PROJECTION_X = u"D3DTextureProjectionX";
constexpr OUStringLiteral D3_DTEXTURE_PROJECTION_Y = u"D3DTextureProjectionY";
constexpr OUStringLiteral D3_DVERTICAL_SEGMENTS = u"D3DVerticalSegments";
constexpr OUStringLiteral EDGE_NODE1_HORZ_DIST = u"EdgeNode1HorzDist";
constexpr OUStringLiteral EDGE_NODE1_VERT_DIST = u"EdgeNode1VertDist";
constexpr OUStringLiteral EDGE_NODE2_HORZ_DIST = u"EdgeNode2HorzDist";
constexpr OUStringLiteral EDGE_NODE2_VERT_DIST = u"EdgeNode2VertDist";
constexpr OUStringLiteral EFFECT = u"Effect";
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
constexpr OUStringLiteral FONT_WORK_ADJUST = u"FontWorkAdjust";
constexpr OUStringLiteral FONT_WORK_DISTANCE = u"FontWorkDistance";
constexpr OUStringLiteral FONT_WORK_FORM = u"FontWorkForm";
constexpr OUStringLiteral FONT_WORK_HIDE_FORM = u"FontWorkHideForm";
constexpr OUStringLiteral FONT_WORK_MIRROR = u"FontWorkMirror";
constexpr OUStringLiteral FONT_WORK_OUTLINE = u"FontWorkOutline";
constexpr OUStringLiteral FONT_WORK_SHADOW = u"FontWorkShadow";
constexpr OUStringLiteral FONT_WORK_SHADOW_COLOR = u"FontWorkShadowColor";
constexpr OUStringLiteral FONT_WORK_SHADOW_OFFSET_X = u"FontWorkShadowOffsetX";
constexpr OUStringLiteral FONT_WORK_SHADOW_OFFSET_Y = u"FontWorkShadowOffsetY";
constexpr OUStringLiteral FONT_WORK_SHADOW_TRANSPARENCE = u"FontWorkShadowTransparence";
constexpr OUStringLiteral FONT_WORK_START = u"FontWorkStart";
constexpr OUStringLiteral FONT_WORK_STYLE = u"FontWorkStyle";
constexpr OUStringLiteral FRAME_IS_AUTO_SCROLL = u"FrameIsAutoScroll";
constexpr OUStringLiteral FRAME_IS_BORDER = u"FrameIsBorder";
constexpr OUStringLiteral FRAME_MARGIN_HEIGHT = u"FrameMarginHeight";
constexpr OUStringLiteral FRAME_MARGIN_WIDTH = u"FrameMarginWidth";
constexpr OUStringLiteral GAMMA = u"Gamma";
constexpr OUStringLiteral GLOW_EFFECT_COLOR = u"GlowEffectColor";
constexpr OUStringLiteral GLOW_EFFECT_RADIUS = u"GlowEffectRadius";
constexpr OUStringLiteral GLOW_EFFECT_TRANSPARENCY = u"GlowEffectTransparency";
constexpr OUStringLiteral GRAPHIC_COLOR_MODE = u"GraphicColorMode";
constexpr OUStringLiteral GRAPHIC_CROP = u"GraphicCrop";
constexpr OUStringLiteral HIGH_RES_DURATION = u"HighResDuration";
constexpr OUStringLiteral IMAGE_SCALE_MODE = u"ImageScaleMode";
constexpr OUStringLiteral IS_BACKGROUND_OBJECTS_VISIBLE = u"IsBackgroundObjectsVisible";
constexpr OUStringLiteral IS_BACKGROUND_VISIBLE = u"IsBackgroundVisible";
constexpr OUStringLiteral IS_DATE_TIME_VISIBLE = u"IsDateTimeVisible";
constexpr OUStringLiteral IS_FOOTER_VISIBLE = u"IsFooterVisible";
constexpr OUStringLiteral IS_HEADER_VISIBLE = u"IsHeaderVisible";
constexpr OUStringLiteral IS_INTERNAL = u"IsInternal";
constexpr OUStringLiteral IS_MIRRORED = u"IsMirrored";
constexpr OUStringLiteral IS_PAGE_NUMBER_VISIBLE = u"IsPageNumberVisible";
constexpr OUStringLiteral LINE_CAP = u"LineCap";
constexpr OUStringLiteral LINE_COLOR = u"LineColor";
constexpr OUStringLiteral LINE_DASH_NAME = u"LineDashName";
constexpr OUStringLiteral LINE_END_CENTER = u"LineEndCenter";
constexpr OUStringLiteral LINE_END_NAME = u"LineEndName";
constexpr OUStringLiteral LINE_END_WIDTH = u"LineEndWidth";
constexpr OUStringLiteral LINE_JOINT = u"LineJoint";
constexpr OUStringLiteral LINE_START_CENTER = u"LineStartCenter";
constexpr OUStringLiteral LINE_START_NAME = u"LineStartName";
constexpr OUStringLiteral LINE_START_WIDTH = u"LineStartWidth";
constexpr OUStringLiteral LINE_TRANSPARENCE = u"LineTransparence";
constexpr OUStringLiteral LINE_WIDTH = u"LineWidth";
constexpr OUStringLiteral MEASURE_BELOW_REFERENCE_EDGE = u"MeasureBelowReferenceEdge";
constexpr OUStringLiteral MEASURE_DECIMAL_PLACES = u"MeasureDecimalPlaces";
constexpr OUStringLiteral MEASURE_HELP_LINE1_LENGTH = u"MeasureHelpLine1Length";
constexpr OUStringLiteral MEASURE_HELP_LINE2_LENGTH = u"MeasureHelpLine2Length";
constexpr OUStringLiteral MEASURE_HELP_LINE_DISTANCE = u"MeasureHelpLineDistance";
constexpr OUStringLiteral MEASURE_HELP_LINE_OVERHANG = u"MeasureHelpLineOverhang";
constexpr OUStringLiteral MEASURE_LINE_DISTANCE = u"MeasureLineDistance";
constexpr OUStringLiteral MEASURE_SHOW_UNIT = u"MeasureShowUnit";
constexpr OUStringLiteral MEASURE_TEXT_HORIZONTAL_POSITION = u"MeasureTextHorizontalPosition";
constexpr OUStringLiteral MEASURE_TEXT_ROTATE90 = u"MeasureTextRotate90";
constexpr OUStringLiteral MEASURE_TEXT_VERTICAL_POSITION = u"MeasureTextVerticalPosition";
constexpr OUStringLiteral MEASURE_UNIT = u"MeasureUnit";
constexpr OUStringLiteral MOVE_PROTECT = u"MoveProtect";
constexpr OUStringLiteral NUMBERING_RULES = u"NumberingRules";
constexpr OUStringLiteral SHADOW = u"Shadow";
constexpr OUStringLiteral SHADOW_BLUR = u"ShadowBlur";
constexpr OUStringLiteral SHADOW_COLOR = u"ShadowColor";
constexpr OUStringLiteral SHADOW_TRANSPARENCE = u"ShadowTransparence";
constexpr OUStringLiteral SHADOW_XDISTANCE = u"ShadowXDistance";
constexpr OUStringLiteral SHADOW_YDISTANCE = u"ShadowYDistance";
constexpr OUStringLiteral SIZE_PROTECT = u"SizeProtect";
constexpr OUStringLiteral SOFT_EDGE_RADIUS = u"SoftEdgeRadius";
constexpr OUStringLiteral SOUND = u"Sound";
constexpr OUStringLiteral SPEED = u"Speed";
constexpr OUStringLiteral TEXT_ANIMATION_AMOUNT = u"TextAnimationAmount";
constexpr OUStringLiteral TEXT_ANIMATION_COUNT = u"TextAnimationCount";
constexpr OUStringLiteral TEXT_ANIMATION_DELAY = u"TextAnimationDelay";
constexpr OUStringLiteral TEXT_ANIMATION_DIRECTION = u"TextAnimationDirection";
constexpr OUStringLiteral TEXT_ANIMATION_KIND = u"TextAnimationKind";
constexpr OUStringLiteral TEXT_ANIMATION_START_INSIDE = u"TextAnimationStartInside";
constexpr OUStringLiteral TEXT_ANIMATION_STOP_INSIDE = u"TextAnimationStopInside";
constexpr OUStringLiteral TEXT_AUTO_GROW_HEIGHT = u"TextAutoGrowHeight";
constexpr OUStringLiteral TEXT_AUTO_GROW_WIDTH = u"TextAutoGrowWidth";
constexpr OUStringLiteral TEXT_CHAIN_NEXT_NAME = u"TextChainNextName";
constexpr OUStringLiteral TEXT_COLUMNS = u"TextColumns";
constexpr OUStringLiteral TEXT_CONTOUR_FRAME = u"TextContourFrame";
constexpr OUStringLiteral TEXT_FIT_TO_SIZE = u"TextFitToSize";
constexpr OUStringLiteral TEXT_HORIZONTAL_ADJUST = u"TextHorizontalAdjust";
constexpr OUStringLiteral TEXT_LEFT_DISTANCE = u"TextLeftDistance";
constexpr OUStringLiteral TEXT_LOWER_DISTANCE = u"TextLowerDistance";
constexpr OUStringLiteral TEXT_MAXIMUM_FRAME_HEIGHT = u"TextMaximumFrameHeight";
constexpr OUStringLiteral TEXT_MAXIMUM_FRAME_WIDTH = u"TextMaximumFrameWidth";
constexpr OUStringLiteral TEXT_MINIMUM_FRAME_HEIGHT = u"TextMinimumFrameHeight";
constexpr OUStringLiteral TEXT_MINIMUM_FRAME_WIDTH = u"TextMinimumFrameWidth";
constexpr OUStringLiteral TEXT_RIGHT_DISTANCE = u"TextRightDistance";
constexpr OUStringLiteral TEXT_UPPER_DISTANCE = u"TextUpperDistance";
constexpr OUStringLiteral TEXT_VERTICAL_ADJUST = u"TextVerticalAdjust";
constexpr OUStringLiteral TEXT_WORD_WRAP = u"TextWordWrap";
constexpr OUStringLiteral TEXT_WRITING_MODE = u"TextWritingMode";
constexpr OUStringLiteral TRANSITION_DIRECTION = u"TransitionDirection";
constexpr OUStringLiteral TRANSITION_FADE_COLOR = u"TransitionFadeColor";
constexpr OUStringLiteral TRANSITION_SUBTYPE = u"TransitionSubtype";
constexpr OUStringLiteral TRANSITION_TYPE = u"TransitionType";
constexpr OUStringLiteral TRANSPARENCY = u"Transparency";
constexpr OUStringLiteral USE_BANDING_COLUMN_STYLE = u"UseBandingColumnStyle";
constexpr OUStringLiteral USE_BANDING_ROW_STYLE = u"UseBandingRowStyle";
constexpr OUStringLiteral USE_FIRST_COLUMN_STYLE = u"UseFirstColumnStyle";
constexpr OUStringLiteral USE_FIRST_ROW_STYLE = u"UseFirstRowStyle";
constexpr OUStringLiteral USE_LAST_COLUMN_STYLE = u"UseLastColumnStyle";
constexpr OUStringLiteral USE_LAST_ROW_STYLE = u"UseLastRowStyle";
constexpr OUStringLiteral VISIBLE = u"Visible";
constexpr OUStringLiteral VISIBLE_AREA = u"VisibleArea";

const XMLPropertyMapEntry aXMLSDProperties[] =
{
    // this entry must be first! this is needed for XMLShapeImportHelper::CreateExternalShapePropMapper

    // ^^^though CreateExternalShapePropMapper is gone now, hmm^^^
    GMAP( USER_DEFINED_ATTRIBUTES,          XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    // stroke attributes
    GMAP( LINE_STYLE,                      XML_NAMESPACE_DRAW, XML_STROKE,                 XML_SD_TYPE_STROKE, 0 ),
    GMAP( LINE_DASH_NAME,                   XML_NAMESPACE_DRAW, XML_STROKE_DASH,            XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT , CTF_DASHNAME ),
    GMAP( LINE_WIDTH,                      XML_NAMESPACE_SVG,  XML_STROKE_WIDTH,           XML_TYPE_MEASURE, 0 ),
    GMAP_D(LINE_COLOR,                     XML_NAMESPACE_SVG,  XML_STROKE_COLOR,           XML_TYPE_COLOR, 0),
    GMAP( LINE_START_NAME,                  XML_NAMESPACE_DRAW, XML_MARKER_START,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINESTARTNAME ),
    GMAP( LINE_START_WIDTH,                 XML_NAMESPACE_DRAW, XML_MARKER_START_WIDTH,     XML_TYPE_MEASURE, 0 ),
    GMAP( LINE_START_CENTER,                XML_NAMESPACE_DRAW, XML_MARKER_START_CENTER,    XML_TYPE_BOOL, 0 ),
    GMAP( LINE_END_NAME,                    XML_NAMESPACE_DRAW, XML_MARKER_END,             XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINEENDNAME ),
    GMAP( LINE_END_WIDTH,                   XML_NAMESPACE_DRAW, XML_MARKER_END_WIDTH,       XML_TYPE_MEASURE, 0 ),
    GMAP( LINE_END_CENTER,                  XML_NAMESPACE_DRAW, XML_MARKER_END_CENTER,      XML_TYPE_BOOL, 0 ),
    GMAP( LINE_TRANSPARENCE,               XML_NAMESPACE_SVG,  XML_STROKE_OPACITY,         XML_SD_TYPE_OPACITY, 0 ),
    GMAP( LINE_JOINT,                      XML_NAMESPACE_DRAW, XML_STROKE_LINEJOIN,        XML_SD_TYPE_LINEJOIN, 0 ),
    GMAP( LINE_CAP,                        XML_NAMESPACE_SVG , XML_STROKE_LINECAP,         XML_SD_TYPE_LINECAP, 0 ),

    // fill attributes
    GMAP( FILL_STYLE,                      XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, CTF_FILLSTYLE ),
    GMAP_D(FILL_COLOR,                     XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, CTF_FILLCOLOR ),
    GMAP_D(FILL_COLOR2,                    XML_NAMESPACE_DRAW, XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR, 0),
    GMAP( FILL_GRADIENT_NAME,               XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    GMAP( FILL_GRADIENT_STEP_COUNT,          XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16, 0 ),
    GMAP( FILL_HATCH_NAME,                  XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( FILL_BACKGROUND,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    GMAP( FILL_BITMAP_NAME,                 XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    GMAP( FILL_TRANSPARENCE,               XML_NAMESPACE_DRAW, XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),    // exists in SW, too
    GMAP( FILL_TRANSPARENCE_GRADIENT_NAME,   XML_NAMESPACE_DRAW, XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    GMAP( FILL_BITMAP_SIZE_X,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_SIZE_Y,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_LOGICAL_SIZE,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_MODE,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_X,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_POSITION_OFFSET_Y,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    GMAP( FILL_BITMAP_RECTANGLE_POINT,       XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    GMAP( FILL_BITMAP_OFFSET_X,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    GMAP( FILL_BITMAP_OFFSET_Y,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // text frame attributes
    GMAP( TEXT_HORIZONTAL_ADJUST,           XML_NAMESPACE_DRAW, XML_TEXTAREA_HORIZONTAL_ALIGN,  XML_SD_TYPE_TEXT_ALIGN, 0 ),
    GMAP( TEXT_VERTICAL_ADJUST,             XML_NAMESPACE_DRAW, XML_TEXTAREA_VERTICAL_ALIGN,    XML_SD_TYPE_VERTICAL_ALIGN, 0 ),
    GMAP( TEXT_AUTO_GROW_HEIGHT,             XML_NAMESPACE_DRAW, XML_AUTO_GROW_HEIGHT,       XML_TYPE_BOOL, 0 ),
    GMAP( TEXT_AUTO_GROW_WIDTH,              XML_NAMESPACE_DRAW, XML_AUTO_GROW_WIDTH,        XML_TYPE_BOOL, 0 ),
    GMAP( TEXT_FIT_TO_SIZE,                  XML_NAMESPACE_DRAW, XML_FIT_TO_SIZE,            XML_SD_TYPE_FITTOSIZE|MID_FLAG_MERGE_PROPERTY, 0),
    GMAPV( TEXT_FIT_TO_SIZE,                 XML_NAMESPACE_STYLE, XML_SHRINK_TO_FIT,         XML_SD_TYPE_FITTOSIZE_AUTOFIT|MID_FLAG_MERGE_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    GMAP( TEXT_CONTOUR_FRAME,               XML_NAMESPACE_DRAW, XML_FIT_TO_CONTOUR,         XML_TYPE_BOOL, 0 ),
    GMAP( TEXT_MAXIMUM_FRAME_HEIGHT,         XML_NAMESPACE_FO,   XML_MAX_HEIGHT,             XML_TYPE_MEASURE, 0 ),
    GMAP( TEXT_MAXIMUM_FRAME_WIDTH,          XML_NAMESPACE_FO,   XML_MAX_WIDTH,              XML_TYPE_MEASURE, 0 ),
    GMAP( TEXT_MINIMUM_FRAME_HEIGHT,         XML_NAMESPACE_FO,   XML_MIN_HEIGHT,             XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( TEXT_MINIMUM_FRAME_WIDTH,          XML_NAMESPACE_FO,   XML_MIN_WIDTH,              XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( TEXT_UPPER_DISTANCE,              XML_NAMESPACE_FO,   XML_PADDING_TOP,            XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( TEXT_LOWER_DISTANCE,              XML_NAMESPACE_FO,   XML_PADDING_BOTTOM,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( TEXT_LEFT_DISTANCE,               XML_NAMESPACE_FO,   XML_PADDING_LEFT,           XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( TEXT_RIGHT_DISTANCE,              XML_NAMESPACE_FO,   XML_PADDING_RIGHT,          XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    PMAP( TEXT_WRITING_MODE,                XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_SD_TYPE_WRITINGMODE|MID_FLAG_MULTI_PROPERTY, CTF_WRITINGMODE ),
    GMAP( NUMBERING_RULES,                 XML_NAMESPACE_TEXT, XML_LIST_STYLE,             XML_SD_TYPE_NUMBULLET|MID_FLAG_ELEMENT_ITEM, CTF_NUMBERINGRULES ),
    GMAP( NUMBERING_RULES,                 XML_NAMESPACE_TEXT, XML_LIST_STYLE_NAME,        XML_TYPE_STRING, CTF_SD_NUMBERINGRULES_NAME ),
    GMAP( TEXT_WORD_WRAP,                   XML_NAMESPACE_FO,   XML_WRAP_OPTION,            XML_TYPE_WRAP_OPTION, 0 ),
    GMAP( TEXT_CHAIN_NEXT_NAME,              XML_NAMESPACE_DRAW,   XML_CHAIN_NEXT_NAME,      XML_TYPE_STRING, 0 ),

    GMAP( TEXT_COLUMNS,                    XML_NAMESPACE_STYLE, XML_COLUMNS, XML_TYPE_TEXT_COLUMNS|MID_FLAG_ELEMENT_ITEM, CTF_TEXTCOLUMNS ),

    // shadow attributes
    GMAP( SHADOW,                         XML_NAMESPACE_DRAW, XML_SHADOW,                 XML_SD_TYPE_VISIBLE_HIDDEN, 0 ),
    GMAP( SHADOW_XDISTANCE,                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X,        XML_TYPE_MEASURE, 0 ),
    GMAP( SHADOW_YDISTANCE,                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y,        XML_TYPE_MEASURE, 0 ),
    GMAP( SHADOW_COLOR,                        XML_NAMESPACE_DRAW, XML_SHADOW_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( SHADOW_TRANSPARENCE,             XML_NAMESPACE_DRAW, XML_SHADOW_OPACITY, XML_TYPE_NEG_PERCENT, 0 ),
    GMAPV( SHADOW_BLUR,                     XML_NAMESPACE_LO_EXT, XML_SHADOW_BLUR, XML_TYPE_MEASURE, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // glow attributes
    GMAPV( GLOW_EFFECT_RADIUS,                 XML_NAMESPACE_LO_EXT, XML_GLOW_RADIUS,          XML_TYPE_MEASURE  , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( GLOW_EFFECT_COLOR,                  XML_NAMESPACE_LO_EXT, XML_GLOW_COLOR,           XML_TYPE_COLOR    , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( GLOW_EFFECT_TRANSPARENCY,           XML_NAMESPACE_LO_EXT, XML_GLOW_TRANSPARENCY,    XML_TYPE_PERCENT16, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // soft edge attributes
    GMAPV( SOFT_EDGE_RADIUS,                XML_NAMESPACE_LO_EXT, XML_SOFTEDGE_RADIUS,      XML_TYPE_MEASURE          , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // graphic attributes
    GMAP( GRAPHIC_COLOR_MODE,               XML_NAMESPACE_DRAW, XML_COLOR_MODE,             XML_TYPE_COLOR_MODE, 0 ), // exists in SW, too, with same property name
    GMAP( ADJUST_LUMINANCE,                XML_NAMESPACE_DRAW, XML_LUMINANCE,              XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( ADJUST_CONTRAST,                 XML_NAMESPACE_DRAW, XML_CONTRAST,               XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( GAMMA,                          XML_NAMESPACE_DRAW, XML_GAMMA,                  XML_TYPE_DOUBLE_PERCENT, 0 ), // signed? exists in SW, too, with same property name
    GMAP( ADJUST_RED,                      XML_NAMESPACE_DRAW, XML_RED,                    XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( ADJUST_GREEN,                    XML_NAMESPACE_DRAW, XML_GREEN,                  XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( ADJUST_BLUE,                     XML_NAMESPACE_DRAW, XML_BLUE,                   XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAPV( GRAPHIC_CROP,                   XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP, CTF_TEXT_CLIP, SvtSaveOptions::ODFSVER_012), // exists in SW, too, with same property name
    GMAP( GRAPHIC_CROP,                    XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP11, CTF_TEXT_CLIP11 ), // exists in SW, too, with same property name
    GMAP( TRANSPARENCY,                   XML_NAMESPACE_DRAW, XML_IMAGE_OPACITY,          XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ), // exists in SW, too, with same property name // #i25616#
    GMAP( IS_MIRRORED,                     XML_NAMESPACE_STYLE,    XML_MIRROR,             XML_TYPE_SD_MIRROR|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too // #i40214#

    // animation text attributes
    TMAP( TEXT_ANIMATION_KIND,              XML_NAMESPACE_STYLE,XML_TEXT_BLINKING,          XML_TYPE_TEXT_ANIMATION_BLINKING, CTF_TEXTANIMATION_BLINKING ),
    GMAP( TEXT_ANIMATION_KIND,              XML_NAMESPACE_TEXT, XML_ANIMATION,              XML_TYPE_TEXT_ANIMATION, CTF_TEXTANIMATION_KIND ),
    GMAP( TEXT_ANIMATION_DIRECTION,         XML_NAMESPACE_TEXT, XML_ANIMATION_DIRECTION,    XML_TYPE_TEXT_ANIMATION_DIRECTION, 0 ),
    GMAP( TEXT_ANIMATION_START_INSIDE,       XML_NAMESPACE_TEXT, XML_ANIMATION_START_INSIDE, XML_TYPE_BOOL, 0 ),
    GMAP( TEXT_ANIMATION_STOP_INSIDE,            XML_NAMESPACE_TEXT, XML_ANIMATION_STOP_INSIDE,  XML_TYPE_BOOL, 0 ),
    GMAP( TEXT_ANIMATION_COUNT,             XML_NAMESPACE_TEXT, XML_ANIMATION_REPEAT,       XML_TYPE_NUMBER16, 0 ),
    GMAP( TEXT_ANIMATION_DELAY,             XML_NAMESPACE_TEXT, XML_ANIMATION_DELAY,        XML_TYPE_DURATION16_MS, 0 ),
    GMAP( TEXT_ANIMATION_AMOUNT,                XML_NAMESPACE_TEXT, XML_ANIMATION_STEPS,        XML_TYPE_TEXT_ANIMATION_STEPS, 0 ),

    // connector attributes
    GMAP( EDGE_NODE1_HORZ_DIST,              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL,  XML_TYPE_MEASURE, 0 ),
    GMAP( EDGE_NODE1_VERT_DIST,              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( EDGE_NODE2_HORZ_DIST,              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( EDGE_NODE2_VERT_DIST,              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL,      XML_TYPE_MEASURE, 0 ),

    // measure attributes
    GMAP( MEASURE_LINE_DISTANCE,                XML_NAMESPACE_DRAW, XML_LINE_DISTANCE,          XML_TYPE_MEASURE, 0 ),
    GMAP( MEASURE_HELP_LINE_OVERHANG,            XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG,         XML_TYPE_MEASURE, 0 ),
    GMAP( MEASURE_HELP_LINE_DISTANCE,            XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE,         XML_TYPE_MEASURE, 0 ),
    GMAP( MEASURE_HELP_LINE1_LENGTH,         XML_NAMESPACE_DRAW, XML_START_GUIDE,            XML_TYPE_MEASURE, 0 ),
    GMAP( MEASURE_HELP_LINE2_LENGTH,         XML_NAMESPACE_DRAW, XML_END_GUIDE,              XML_TYPE_MEASURE, 0 ),
    GMAP( MEASURE_TEXT_HORIZONTAL_POSITION,  XML_NAMESPACE_DRAW, XML_MEASURE_ALIGN,          XML_SD_TYPE_MEASURE_HALIGN, 0 ),
    GMAP( MEASURE_TEXT_VERTICAL_POSITION,    XML_NAMESPACE_DRAW, XML_MEASURE_VERTICAL_ALIGN, XML_SD_TYPE_MEASURE_VALIGN, 0 ),
    GMAP( MEASURE_UNIT,                        XML_NAMESPACE_DRAW, XML_UNIT,                   XML_SD_TYPE_MEASURE_UNIT, 0 ),
    GMAP( MEASURE_SHOW_UNIT,                    XML_NAMESPACE_DRAW, XML_SHOW_UNIT,              XML_TYPE_BOOL, 0 ),
    GMAP( MEASURE_BELOW_REFERENCE_EDGE,      XML_NAMESPACE_DRAW, XML_PLACING,                XML_SD_TYPE_MEASURE_PLACING, 0 ),
    GMAP( MEASURE_TEXT_ROTATE90,                XML_NAMESPACE_DRAW, XML_PARALLEL,               XML_TYPE_BOOL, 0 ),
    GMAP( MEASURE_DECIMAL_PLACES,           XML_NAMESPACE_DRAW, XML_DECIMAL_PLACES,         XML_TYPE_NUMBER16, 0 ),

    // 3D geometry attributes
    GMAP( D3_DHORIZONTAL_SEGMENTS,          XML_NAMESPACE_DR3D, XML_HORIZONTAL_SEGMENTS,    XML_TYPE_NUMBER, 0 ),
    GMAP( D3_DVERTICAL_SEGMENTS,                XML_NAMESPACE_DR3D, XML_VERTICAL_SEGMENTS,      XML_TYPE_NUMBER, 0 ),
    GMAP( D3_DPERCENT_DIAGONAL,             XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING,          XML_TYPE_PERCENT, 0 ),
    GMAP( D3_DBACKSCALE,                   XML_NAMESPACE_DR3D, XML_BACK_SCALE,             XML_TYPE_PERCENT, 0 ),
    GMAP( D3_DEND_ANGLE,                        XML_NAMESPACE_DR3D, XML_END_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( D3_DDEPTH,                       XML_NAMESPACE_DR3D, XML_DEPTH,                  XML_TYPE_MEASURE, 0 ),
    GMAP( D3_DDOUBLE_SIDED,                 XML_NAMESPACE_DR3D, XML_BACKFACE_CULLING,       XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // #107245# New 3D properties which are possible for lathe and extrude 3d objects
    GMAP( D3_DCLOSE_FRONT,                  XML_NAMESPACE_DR3D, XML_CLOSE_FRONT,            XML_TYPE_BOOL, 0 ),
    GMAP( D3_DCLOSE_BACK,                   XML_NAMESPACE_DR3D, XML_CLOSE_BACK,             XML_TYPE_BOOL, 0 ),

    // 3D lighting attributes
    GMAP( D3_DNORMALS_KIND,                 XML_NAMESPACE_DR3D, XML_NORMALS_KIND,           XML_SD_TYPE_NORMALS_KIND, 0 ),
    GMAP( D3_DNORMALS_INVERT,               XML_NAMESPACE_DR3D, XML_NORMALS_DIRECTION,      XML_SD_TYPE_NORMALS_DIRECTION, 0 ),

    // 3D texture attributes
    GMAP( D3_DTEXTURE_PROJECTION_X,          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_X,  XML_SD_TYPE_TEX_GENERATION_MODE_X, 0 ),
    GMAP( D3_DTEXTURE_PROJECTION_Y,          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_Y,  XML_SD_TYPE_TEX_GENERATION_MODE_Y, 0 ),
    GMAP( D3_DTEXTURE_KIND,                 XML_NAMESPACE_DR3D, XML_TEX_KIND,               XML_SD_TYPE_TEX_KIND, 0 ),
    GMAP( D3_DTEXTURE_MODE,                 XML_NAMESPACE_DR3D, XML_TEX_MODE,               XML_SD_TYPE_TEX_MODE, 0 ),
    GMAP( D3_DTEXTURE_FILTER,               XML_NAMESPACE_DR3D, XML_TEX_FILTER,             XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // 3D material attributes
    GMAP( D3_DMATERIAL_COLOR,               XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR,          XML_TYPE_COLOR, 0 ),
    GMAP( D3_DMATERIAL_EMISSION,                XML_NAMESPACE_DR3D, XML_EMISSIVE_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( D3_DMATERIAL_SPECULAR,                XML_NAMESPACE_DR3D, XML_SPECULAR_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( D3_DMATERIAL_SPECULAR_INTENSITY,   XML_NAMESPACE_DR3D, XML_SHININESS,              XML_TYPE_PERCENT, 0 ),

    // 3D shadow attributes
    GMAP( D3_DSHADOW3_D,                        XML_NAMESPACE_DR3D, XML_SHADOW,                 XML_SD_TYPE_VISIBLE_HIDDEN, 0 ),

    // #FontWork# attributes
    GMAP( FONT_WORK_STYLE,                  XML_NAMESPACE_DRAW, XML_FONTWORK_STYLE,                 XML_SD_TYPE_FONTWORK_STYLE| MID_FLAG_ELEMENT_ITEM_EXPORT, CTF_FONTWORK_STYLE  ),
    GMAP( FONT_WORK_ADJUST,                 XML_NAMESPACE_DRAW, XML_FONTWORK_ADJUST,                XML_SD_TYPE_FONTWORK_ADJUST | MID_FLAG_ELEMENT_ITEM_EXPORT,CTF_FONTWORK_ADJUST ),
    GMAP( FONT_WORK_DISTANCE,               XML_NAMESPACE_DRAW, XML_FONTWORK_DISTANCE,              XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_DISTANCE   ),
    GMAP( FONT_WORK_START,                  XML_NAMESPACE_DRAW, XML_FONTWORK_START,                 XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_START  ),
    GMAP( FONT_WORK_MIRROR,                 XML_NAMESPACE_DRAW, XML_FONTWORK_MIRROR,                XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_MIRROR ),
    GMAP( FONT_WORK_OUTLINE,                XML_NAMESPACE_DRAW, XML_FONTWORK_OUTLINE,               XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_OUTLINE    ),
    GMAP( FONT_WORK_SHADOW,                 XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW,                XML_SD_TYPE_FONTWORK_SHADOW | MID_FLAG_ELEMENT_ITEM_EXPORT,CTF_FONTWORK_SHADOW ),
    GMAP( FONT_WORK_SHADOW_COLOR,            XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_COLOR,          XML_TYPE_COLOR | MID_FLAG_ELEMENT_ITEM_EXPORT,             CTF_FONTWORK_SHADOWCOLOR    ),
    GMAP( FONT_WORK_SHADOW_OFFSET_X,          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_X,       XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWOFFSETX  ),
    GMAP( FONT_WORK_SHADOW_OFFSET_Y,          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_Y,       XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWOFFSETY  ),
    GMAP( FONT_WORK_FORM,                   XML_NAMESPACE_DRAW, XML_FONTWORK_FORM,                  XML_SD_TYPE_FONTWORK_FORM | MID_FLAG_ELEMENT_ITEM_EXPORT,  CTF_FONTWORK_FORM   ),
    GMAP( FONT_WORK_HIDE_FORM,               XML_NAMESPACE_DRAW, XML_FONTWORK_HIDE_FORM,             XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_HIDEFORM   ),
    GMAP( FONT_WORK_SHADOW_TRANSPARENCE,     XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_TRANSPARENCE,   XML_TYPE_PERCENT | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWTRANSPARENCE ),

    // #FontWork# attributes
    GMAPV(FONT_WORK_STYLE,              XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_STYLE,                 XML_SD_TYPE_FONTWORK_STYLE, CTF_FONTWORK_STYLE,         SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_ADJUST,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_ADJUST,                XML_SD_TYPE_FONTWORK_ADJUST,CTF_FONTWORK_ADJUST,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_DISTANCE,           XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_DISTANCE,              XML_TYPE_MEASURE,           CTF_FONTWORK_DISTANCE,      SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_START,              XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_START,                 XML_TYPE_MEASURE,           CTF_FONTWORK_START,         SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_MIRROR,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_MIRROR,                XML_TYPE_BOOL,              CTF_FONTWORK_MIRROR,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_OUTLINE,            XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_OUTLINE,               XML_TYPE_BOOL,              CTF_FONTWORK_OUTLINE,       SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_SHADOW,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW,                XML_SD_TYPE_FONTWORK_SHADOW,CTF_FONTWORK_SHADOW,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_SHADOW_COLOR,        XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_COLOR,          XML_TYPE_COLOR,             CTF_FONTWORK_SHADOWCOLOR,   SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_SHADOW_OFFSET_X,      XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_OFFSET_X,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETX, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_SHADOW_OFFSET_Y,      XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_OFFSET_Y,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETY, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_FORM,               XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_FORM,                  XML_SD_TYPE_FONTWORK_FORM,  CTF_FONTWORK_FORM,          SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_HIDE_FORM,           XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_HIDE_FORM,             XML_TYPE_BOOL,              CTF_FONTWORK_HIDEFORM,      SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV(FONT_WORK_SHADOW_TRANSPARENCE, XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_TRANSPARENCE,   XML_TYPE_PERCENT,           CTF_FONTWORK_SHADOWTRANSPARENCE, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // control attributes (border exists one more time for the text additions of shapes)
    GMAP( CONTROL_SYMBOL_COLOR,             XML_NAMESPACE_DRAW, XML_SYMBOL_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( CONTROL_BACKGROUND,              XML_NAMESPACE_FO,   XML_BACKGROUND_COLOR,       XML_TYPE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( CONTROL_BORDER,                  XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( CONTROL_BORDER_COLOR,             XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER_COLOR|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( CONTROL_DATA_STYLE,               XML_NAMESPACE_STYLE,XML_DATA_STYLE_NAME,        XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_EXPORT|MID_FLAG_SPECIAL_ITEM, CTF_SD_CONTROL_SHAPE_DATA_STYLE ),
    GMAP( CONTROL_TEXT_EMPHASIS,            XML_NAMESPACE_STYLE,XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
    GMAP( IMAGE_SCALE_MODE,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_IMAGE_SCALE_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( CONTROL_WRITING_MODE,             XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT|MID_FLAG_MULTI_PROPERTY, CTF_CONTROLWRITINGMODE ),

    // special entries for floating frames
    GMAP( FRAME_IS_AUTO_SCROLL,          XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_SCROLLBAR,    XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_SCROLLBAR ),
    GMAP( FRAME_IS_BORDER,              XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_BORDER,       XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_BORDER ),
    GMAP( FRAME_MARGIN_WIDTH,           XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_HORIZONTAL,    XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_HORI ),
    GMAP( FRAME_MARGIN_HEIGHT,          XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_VERTICAL,      XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_VERT ),
    GMAP( VISIBLE_AREA,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,   CTF_SD_OLE_VIS_AREA_IMPORT_LEFT ),
    GMAP( VISIBLE_AREA,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,    CTF_SD_OLE_VIS_AREA_IMPORT_TOP ),
    GMAP( VISIBLE_AREA,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,  CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH ),
    GMAP( VISIBLE_AREA,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY, CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT ),
    GMAP( IS_INTERNAL,                 XML_NAMESPACE_DRAW, XML__EMPTY,                     XML_TYPE_BUILDIN_CMP_ONLY,                          CTF_SD_OLE_ISINTERNAL ),
    GMAP( IS_INTERNAL,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,    CTF_SD_OLE_VIS_AREA_EXPORT_LEFT ),
    GMAP( IS_INTERNAL,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,     CTF_SD_OLE_VIS_AREA_EXPORT_TOP ),
    GMAP( IS_INTERNAL,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,   CTF_SD_OLE_VIS_AREA_EXPORT_WIDTH ),
    GMAP( IS_INTERNAL,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,  CTF_SD_OLE_VIS_AREA_EXPORT_HEIGHT ),

    GMAP( ASPECT,                     XML_NAMESPACE_DRAW, XML_DRAW_ASPECT,                XML_TYPE_TEXT_DRAW_ASPECT|MID_FLAG_MULTI_PROPERTY,  CTF_SD_OLE_ASPECT ),

    // caption properties
    GMAP( CAPTION_TYPE,                    XML_NAMESPACE_DRAW, XML_CAPTION_TYPE,               XML_SD_TYPE_CAPTION_TYPE, 0 ),
    GMAP( CAPTION_IS_FIXED_ANGLE,            XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE_TYPE,         XML_SD_TYPE_CAPTION_ANGLE_TYPE, 0 ),
    GMAP( CAPTION_ANGLE,               XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( CAPTION_GAP,                 XML_NAMESPACE_DRAW, XML_CAPTION_GAP,                XML_TYPE_MEASURE, 0 ),
    GMAP( CAPTION_ESCAPE_DIRECTION,     XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE_DIRECTION,   XML_SD_TYPE_CAPTION_ESC_DIR, 0 ),
    GMAP( CAPTION_IS_ESCAPE_RELATIVE,    XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_IS_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ISESCREL ),
    GMAP( CAPTION_ESCAPE_RELATIVE,      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCREL ),
    GMAP( CAPTION_ESCAPE_ABSOLUTE,      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_ABS|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCABS ),
    GMAP( CAPTION_LINE_LENGTH,          XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH,        XML_TYPE_MEASURE, 0 ),
    GMAP( CAPTION_IS_FIT_LINE_LENGTH,     XML_NAMESPACE_DRAW, XML_CAPTION_FIT_LINE_LENGTH,    XML_TYPE_BOOL, 0 ),

    // misc object properties
    GMAP( MOVE_PROTECT,                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_MOVE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_MOVE_PROTECT ),
    GMAP( SIZE_PROTECT,                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_SIZE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_SIZE_PROTECT ),

    MAP_END()
};

// entry list for presentation page properties

const XMLPropertyMapEntry aXMLSDPresPageProps[] =
{
    DPMAP( USER_DEFINED_ATTRIBUTES,     XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    DPMAP( CHANGE,                      XML_NAMESPACE_PRESENTATION, XML_TRANSITION_TYPE,    XML_SD_TYPE_PRESPAGE_TYPE, CTF_PAGE_TRANS_TYPE ),
    DPMAP( EFFECT,                      XML_NAMESPACE_PRESENTATION, XML_TRANSITION_STYLE,   XML_SD_TYPE_PRESPAGE_STYLE, CTF_PAGE_TRANS_STYLE ),
    DPMAP( SPEED,                       XML_NAMESPACE_PRESENTATION, XML_TRANSITION_SPEED,   XML_SD_TYPE_PRESPAGE_SPEED, CTF_PAGE_TRANS_SPEED ),
    DPMAP( HIGH_RES_DURATION,           XML_NAMESPACE_PRESENTATION, XML_DURATION,           XML_SD_TYPE_PRESPAGE_DURATION, CTF_PAGE_TRANS_DURATION ),
    DPMAP( VISIBLE,                   XML_NAMESPACE_PRESENTATION, XML_VISIBILITY,         XML_SD_TYPE_PRESPAGE_VISIBILITY, CTF_PAGE_VISIBLE ),
    DPMAP( SOUND,                     XML_NAMESPACE_PRESENTATION, XML_SOUND,              XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM, CTF_PAGE_SOUND_URL ),
    DPMAP( BACKGROUND_FULL_SIZE,            XML_NAMESPACE_DRAW,         XML_BACKGROUND_SIZE,    XML_SD_TYPE_PRESPAGE_BACKSIZE, CTF_PAGE_BACKSIZE ),

    DPMAP( IS_BACKGROUND_VISIBLE,       XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_VISIBLE, XML_TYPE_BOOL, 0 ),
    DPMAP( IS_BACKGROUND_OBJECTS_VISIBLE,    XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_OBJECTS_VISIBLE, XML_TYPE_BOOL, 0 ),

    DPMAP( FILL_STYLE,                 XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, 0 ),
    DPMAP( FILL_COLOR,                 XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    DPMAP( FILL_GRADIENT_NAME,          XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    DPMAP( FILL_GRADIENT_STEP_COUNT,     XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER, 0 ),
    DPMAP( FILL_HATCH_NAME,             XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( FILL_BACKGROUND,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    DPMAP( FILL_BITMAP_NAME,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,    XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    DPMAP( FILL_TRANSPARENCE,              XML_NAMESPACE_DRAW, XML_OPACITY,           XML_TYPE_NEG_PERCENT|MID_FLAG_MULTI_PROPERTY,  0 ),
    DPMAP( FILL_TRANSPARENCE_GRADIENT_NAME,  XML_NAMESPACE_DRAW, XML_OPACITY_NAME,       XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    DPMAP( FILL_BITMAP_SIZE_X,           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( FILL_BITMAP_LOGICAL_SIZE,     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( FILL_BITMAP_SIZE_Y,           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( FILL_BITMAP_LOGICAL_SIZE,     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( FILL_BITMAP_MODE,            XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE, 0 ),
    DPMAP( FILL_BITMAP_POSITION_OFFSET_X, XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    DPMAP( FILL_BITMAP_POSITION_OFFSET_Y, XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    DPMAP( FILL_BITMAP_RECTANGLE_POINT,  XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    DPMAP( FILL_BITMAP_OFFSET_X,         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    DPMAP( FILL_BITMAP_OFFSET_Y,         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET, XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    DPMAP( IS_HEADER_VISIBLE,           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_HEADER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_HEADER_VISIBLE ),
    DPMAP( IS_FOOTER_VISIBLE,           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_FOOTER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_FOOTER_VISIBLE ),
    DPMAP( IS_PAGE_NUMBER_VISIBLE,       XML_NAMESPACE_PRESENTATION, XML_DISPLAY_PAGE_NUMBER,    XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_PAGE_NUMBER_VISIBLE ),
    DPMAP( IS_DATE_TIME_VISIBLE,         XML_NAMESPACE_PRESENTATION, XML_DISPLAY_DATE_TIME,      XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_DATE_TIME_VISIBLE ),

    DPMAP( TRANSITION_TYPE,            XML_NAMESPACE_SMIL, XML_TYPE,                   XML_SD_TYPE_TRANSITION_TYPE, CTF_PAGE_TRANSITION_TYPE ),
    DPMAP( TRANSITION_SUBTYPE,         XML_NAMESPACE_SMIL, XML_SUBTYPE,                XML_SD_TYPE_TRANSTIION_SUBTYPE, CTF_PAGE_TRANSITION_SUBTYPE ),
    DPMAP( TRANSITION_DIRECTION,       XML_NAMESPACE_SMIL, XML_DIRECTION,              XML_SD_TYPE_TRANSTIION_DIRECTION, CTF_PAGE_TRANSITION_DIRECTION ),
    DPMAP( TRANSITION_FADE_COLOR,       XML_NAMESPACE_SMIL, XML_FADECOLOR,              XML_TYPE_COLOR, CTF_PAGE_TRANSITION_FADECOLOR ),
    MAP_END()
};

/** contains the attribute to property mapping for a drawing layer table
    WARNING: if attributes are added, SdXMLTableShapeContext::processAttribute needs to be updated!
*/
const XMLPropertyMapEntry aXMLTableShapeAttributes[] =
{
    MAP_( USE_FIRST_ROW_STYLE,       XML_NAMESPACE_TABLE, XML_USE_FIRST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( USE_LAST_ROW_STYLE,        XML_NAMESPACE_TABLE, XML_USE_LAST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( USE_FIRST_COLUMN_STYLE,    XML_NAMESPACE_TABLE, XML_USE_FIRST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( USE_LAST_COLUMN_STYLE,     XML_NAMESPACE_TABLE, XML_USE_LAST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( USE_BANDING_ROW_STYLE,     XML_NAMESPACE_TABLE, XML_USE_BANDING_ROWS_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( USE_BANDING_COLUMN_STYLE,  XML_NAMESPACE_TABLE, XML_USE_BANDING_COLUMNS_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_END()
};

// implementation of factory for own graphic properties

SvXMLEnumMapEntry<drawing::LineStyle> const aXML_LineStyle_EnumMap[] =
{
    { XML_NONE,     drawing::LineStyle_NONE },
    { XML_SOLID,    drawing::LineStyle_SOLID },
    { XML_DASH,     drawing::LineStyle_DASH },
    { XML_TOKEN_INVALID, drawing::LineStyle(0) }
};

SvXMLEnumMapEntry<drawing::LineJoint> const aXML_LineJoint_EnumMap[] =
{
    { XML_NONE,     drawing::LineJoint_NONE },
    { XML_MITER,    drawing::LineJoint_MITER },
    { XML_ROUND,    drawing::LineJoint_ROUND },
    { XML_BEVEL,    drawing::LineJoint_BEVEL },
    { XML_MIDDLE,   drawing::LineJoint_MIDDLE },
    { XML_TOKEN_INVALID, drawing::LineJoint(0) }
};

SvXMLEnumMapEntry<drawing::LineCap> const aXML_LineCap_EnumMap[] =
{
    { XML_BUTT, drawing::LineCap_BUTT },
    { XML_ROUND, drawing::LineCap_ROUND },
    // use XML_GRADIENTSTYLE_SQUARE as XML_SQUARE, is defined as "square" already
    { XML_GRADIENTSTYLE_SQUARE, drawing::LineCap_SQUARE },
    { XML_TOKEN_INVALID, drawing::LineCap(0) }
};

SvXMLEnumMapEntry<drawing::FillStyle> const aXML_FillStyle_EnumMap[] =
{
    { XML_NONE,     drawing::FillStyle_NONE },
    { XML_SOLID,    drawing::FillStyle_SOLID },
    { XML_BITMAP,   drawing::FillStyle_BITMAP },
    { XML_GRADIENT, drawing::FillStyle_GRADIENT },
    { XML_HATCH,    drawing::FillStyle_HATCH },
    { XML_TOKEN_INVALID, drawing::FillStyle(0) }
};

SvXMLEnumMapEntry<sal_Int32> const aXML_PresChange_EnumMap[] =
{
    { XML_MANUAL,           0 },
    { XML_AUTOMATIC,        1 },
    { XML_SEMI_AUTOMATIC,   2 },
    { XML_TOKEN_INVALID,    0 }
};

SvXMLEnumMapEntry<presentation::AnimationSpeed> const aXML_TransSpeed_EnumMap[] =
{
    { XML_FAST,     presentation::AnimationSpeed_FAST },
    { XML_MEDIUM,   presentation::AnimationSpeed_MEDIUM },
    { XML_SLOW,     presentation::AnimationSpeed_SLOW },
    { XML_TOKEN_INVALID, presentation::AnimationSpeed(0) }
};

SvXMLEnumMapEntry<presentation::FadeEffect> const aXML_FadeEffect_EnumMap[] =
{
    { XML_NONE,                 presentation::FadeEffect_NONE },
    { XML_FADE_FROM_LEFT,       presentation::FadeEffect_FADE_FROM_LEFT },
    { XML_FADE_FROM_TOP,        presentation::FadeEffect_FADE_FROM_TOP },
    { XML_FADE_FROM_RIGHT,      presentation::FadeEffect_FADE_FROM_RIGHT },
    { XML_FADE_FROM_BOTTOM,     presentation::FadeEffect_FADE_FROM_BOTTOM },
    { XML_FADE_TO_CENTER,       presentation::FadeEffect_FADE_TO_CENTER },
    { XML_FADE_FROM_CENTER,     presentation::FadeEffect_FADE_FROM_CENTER },
    { XML_MOVE_FROM_LEFT,       presentation::FadeEffect_MOVE_FROM_LEFT },
    { XML_MOVE_FROM_TOP,        presentation::FadeEffect_MOVE_FROM_TOP },
    { XML_MOVE_FROM_RIGHT,      presentation::FadeEffect_MOVE_FROM_RIGHT },
    { XML_MOVE_FROM_BOTTOM,     presentation::FadeEffect_MOVE_FROM_BOTTOM },
    { XML_ROLL_FROM_TOP,        presentation::FadeEffect_ROLL_FROM_TOP },
    { XML_ROLL_FROM_LEFT,       presentation::FadeEffect_ROLL_FROM_LEFT },
    { XML_ROLL_FROM_RIGHT,      presentation::FadeEffect_ROLL_FROM_RIGHT },
    { XML_ROLL_FROM_BOTTOM,     presentation::FadeEffect_ROLL_FROM_BOTTOM },
    { XML_VERTICAL_STRIPES,     presentation::FadeEffect_VERTICAL_STRIPES },
    { XML_HORIZONTAL_STRIPES,   presentation::FadeEffect_HORIZONTAL_STRIPES },
    { XML_CLOCKWISE,            presentation::FadeEffect_CLOCKWISE },
    { XML_COUNTERCLOCKWISE,     presentation::FadeEffect_COUNTERCLOCKWISE },
    { XML_FADE_FROM_UPPERLEFT,  presentation::FadeEffect_FADE_FROM_UPPERLEFT },
    { XML_FADE_FROM_UPPERRIGHT, presentation::FadeEffect_FADE_FROM_UPPERRIGHT },
    { XML_FADE_FROM_LOWERLEFT,  presentation::FadeEffect_FADE_FROM_LOWERLEFT },
    { XML_FADE_FROM_LOWERRIGHT, presentation::FadeEffect_FADE_FROM_LOWERRIGHT },
    { XML_CLOSE_VERTICAL,       presentation::FadeEffect_CLOSE_VERTICAL },
    { XML_CLOSE_HORIZONTAL,     presentation::FadeEffect_CLOSE_HORIZONTAL },
    { XML_OPEN_VERTICAL,        presentation::FadeEffect_OPEN_VERTICAL },
    { XML_OPEN_HORIZONTAL,      presentation::FadeEffect_OPEN_HORIZONTAL },
    { XML_SPIRALIN_LEFT,        presentation::FadeEffect_SPIRALIN_LEFT },
    { XML_SPIRALIN_RIGHT,       presentation::FadeEffect_SPIRALIN_RIGHT },
    { XML_SPIRALOUT_LEFT,       presentation::FadeEffect_SPIRALOUT_LEFT },
    { XML_SPIRALOUT_RIGHT,      presentation::FadeEffect_SPIRALOUT_RIGHT },
    { XML_DISSOLVE,             presentation::FadeEffect_DISSOLVE },
    { XML_WAVYLINE_FROM_LEFT,   presentation::FadeEffect_WAVYLINE_FROM_LEFT },
    { XML_WAVYLINE_FROM_TOP,    presentation::FadeEffect_WAVYLINE_FROM_TOP },
    { XML_WAVYLINE_FROM_RIGHT,  presentation::FadeEffect_WAVYLINE_FROM_RIGHT },
    { XML_WAVYLINE_FROM_BOTTOM, presentation::FadeEffect_WAVYLINE_FROM_BOTTOM },
    { XML_RANDOM,               presentation::FadeEffect_RANDOM },
    { XML_STRETCH_FROM_LEFT,    presentation::FadeEffect_STRETCH_FROM_LEFT },
    { XML_STRETCH_FROM_TOP,     presentation::FadeEffect_STRETCH_FROM_TOP },
    { XML_STRETCH_FROM_RIGHT,   presentation::FadeEffect_STRETCH_FROM_RIGHT },
    { XML_STRETCH_FROM_BOTTOM,  presentation::FadeEffect_STRETCH_FROM_BOTTOM },
    { XML_VERTICAL_LINES,       presentation::FadeEffect_VERTICAL_LINES },
    { XML_HORIZONTAL_LINES,     presentation::FadeEffect_HORIZONTAL_LINES },
    { XML_MOVE_FROM_UPPERLEFT,  presentation::FadeEffect_MOVE_FROM_UPPERLEFT },
    { XML_MOVE_FROM_UPPERRIGHT, presentation::FadeEffect_MOVE_FROM_UPPERRIGHT },
    { XML_MOVE_FROM_LOWERRIGHT, presentation::FadeEffect_MOVE_FROM_LOWERRIGHT },
    { XML_MOVE_FROM_LOWERLEFT,  presentation::FadeEffect_MOVE_FROM_LOWERLEFT },
    { XML_UNCOVER_TO_LEFT,      presentation::FadeEffect_UNCOVER_TO_LEFT },
    { XML_UNCOVER_TO_UPPERLEFT, presentation::FadeEffect_UNCOVER_TO_UPPERLEFT },
    { XML_UNCOVER_TO_TOP,       presentation::FadeEffect_UNCOVER_TO_TOP },
    { XML_UNCOVER_TO_UPPERRIGHT,presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT },
    { XML_UNCOVER_TO_RIGHT,     presentation::FadeEffect_UNCOVER_TO_RIGHT },
    { XML_UNCOVER_TO_LOWERRIGHT,presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT },
    { XML_UNCOVER_TO_BOTTOM,    presentation::FadeEffect_UNCOVER_TO_BOTTOM },
    { XML_UNCOVER_TO_LOWERLEFT, presentation::FadeEffect_UNCOVER_TO_LOWERLEFT },
    { XML_VERTICAL_CHECKERBOARD,presentation::FadeEffect_VERTICAL_CHECKERBOARD },
    { XML_HORIZONTAL_CHECKERBOARD,presentation::FadeEffect_HORIZONTAL_CHECKERBOARD },
    { XML_TOKEN_INVALID, presentation::FadeEffect(0) }
};

SvXMLEnumMapEntry<drawing::ConnectorType> const aXML_ConnectionKind_EnumMap[] =
{
    { XML_STANDARD,     drawing::ConnectorType_STANDARD },
    { XML_CURVE,        drawing::ConnectorType_CURVE },
    { XML_LINE,         drawing::ConnectorType_LINE },
    { XML_LINES,        drawing::ConnectorType_LINES },
    { XML_TOKEN_INVALID, drawing::ConnectorType(0) }
};

SvXMLEnumMapEntry<drawing::BitmapMode> const aXML_BitmapMode_EnumMap[] =
{
    { XML_REPEAT,                   drawing::BitmapMode_REPEAT },
    { XML_STRETCH,                  drawing::BitmapMode_STRETCH },
    { XML_BACKGROUND_NO_REPEAT,     drawing::BitmapMode_NO_REPEAT },
    { XML_TOKEN_INVALID, drawing::BitmapMode(0) }
};

// 3D EnumMaps

SvXMLEnumMapEntry<drawing::NormalsKind> const  aXML_NormalsKind_EnumMap[] =
{
    { XML_OBJECT,       drawing::NormalsKind_SPECIFIC },
    { XML_FLAT,         drawing::NormalsKind_FLAT },
    { XML_SPHERE,       drawing::NormalsKind_SPHERE },
    { XML_TOKEN_INVALID, drawing::NormalsKind(0) }
};

SvXMLEnumMapEntry<drawing::TextureProjectionMode> const  aXML_TexGenerationX_EnumMap[] =
{
    { XML_OBJECT,       drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,     drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,       drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, drawing::TextureProjectionMode(0) }
};

SvXMLEnumMapEntry<drawing::TextureProjectionMode> const  aXML_TexGenerationY_EnumMap[] =
{
    { XML_OBJECT,       drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,     drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,       drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, drawing::TextureProjectionMode(0) }
};

SvXMLEnumMapEntry<drawing::TextureKind> const  aXML_TexKind_EnumMap[] =
{
    { XML_LUMINANCE,    drawing::TextureKind_LUMINANCE },
    { XML_COLOR,        drawing::TextureKind_COLOR },
    { XML_TOKEN_INVALID, drawing::TextureKind(0) }
};

SvXMLEnumMapEntry<drawing::TextureMode> const  aXML_TexMode_EnumMap[] =
{
    { XML_REPLACE,      drawing::TextureMode_REPLACE },
    { XML_MODULATE,     drawing::TextureMode_MODULATE },
    { XML_BLEND,        drawing::TextureMode_BLEND },
    { XML_TOKEN_INVALID, drawing::TextureMode(0) }
};

SvXMLEnumMapEntry<drawing::RectanglePoint> const aXML_RefPoint_EnumMap[] =
{
    { XML_TOP_LEFT,     drawing::RectanglePoint_LEFT_TOP },
    { XML_TOP,          drawing::RectanglePoint_MIDDLE_TOP },
    { XML_TOP_RIGHT,    drawing::RectanglePoint_RIGHT_TOP },
    { XML_LEFT,         drawing::RectanglePoint_LEFT_MIDDLE },
    { XML_CENTER,       drawing::RectanglePoint_MIDDLE_MIDDLE },
    { XML_RIGHT,        drawing::RectanglePoint_RIGHT_MIDDLE },
    { XML_BOTTOM_LEFT,  drawing::RectanglePoint_LEFT_BOTTOM },
    { XML_BOTTOM,       drawing::RectanglePoint_MIDDLE_BOTTOM },
    { XML_BOTTOM_RIGHT, drawing::RectanglePoint_RIGHT_BOTTOM },
    { XML_TOKEN_INVALID, drawing::RectanglePoint(0) }
};

SvXMLEnumMapEntry<drawing::CircleKind> const aXML_CircleKind_EnumMap[] =
{
    { XML_FULL,         drawing::CircleKind_FULL },
    { XML_SECTION,      drawing::CircleKind_SECTION },
    { XML_CUT,          drawing::CircleKind_CUT },
    { XML_ARC,          drawing::CircleKind_ARC },
    { XML_TOKEN_INVALID, drawing::CircleKind(0) }
};

SvXMLEnumMapEntry<text::WritingMode> const aXML_WritingMode_EnumMap[] =
{
    { XML_TB_RL,        text::WritingMode_TB_RL },
    { XML_LR_TB,        text::WritingMode_LR_TB },
    { XML_TOKEN_INVALID, text::WritingMode(0) }
};

SvXMLEnumMapEntry<drawing::TextAnimationKind> const pXML_TextAnimation_Enum[] =
{
    { XML_NONE,         drawing::TextAnimationKind_NONE },
    { XML_BLINKING,     drawing::TextAnimationKind_BLINK }, // will be filtered
    { XML_SCROLL,       drawing::TextAnimationKind_SCROLL },
    { XML_ALTERNATE,    drawing::TextAnimationKind_ALTERNATE },
    { XML_SLIDE,        drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, drawing::TextAnimationKind(0) }
};

SvXMLEnumMapEntry<drawing::TextAnimationKind> const pXML_TextAnimation_Blinking_Enum[] =
{
    { XML_FALSE,        drawing::TextAnimationKind_NONE },
    { XML_TRUE,         drawing::TextAnimationKind_BLINK },
    { XML_FALSE,        drawing::TextAnimationKind_SCROLL },
    { XML_FALSE,        drawing::TextAnimationKind_ALTERNATE },
    { XML_FALSE,        drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, drawing::TextAnimationKind(0) }
};

SvXMLEnumMapEntry<drawing::TextAnimationDirection> const pXML_TextAnimationDirection_Enum[] =
{
    { XML_LEFT,         drawing::TextAnimationDirection_LEFT },
    { XML_RIGHT,        drawing::TextAnimationDirection_RIGHT },    // will be filtered
    { XML_UP,           drawing::TextAnimationDirection_UP },
    { XML_DOWN,         drawing::TextAnimationDirection_DOWN },
    { XML_TOKEN_INVALID, drawing::TextAnimationDirection(0) }
};

SvXMLEnumMapEntry<drawing::TextHorizontalAdjust> const pXML_TextAlign_Enum[] =
{
    { XML_LEFT,         drawing::TextHorizontalAdjust_LEFT },
    { XML_CENTER,       drawing::TextHorizontalAdjust_CENTER },
    { XML_RIGHT,        drawing::TextHorizontalAdjust_RIGHT },
    { XML_JUSTIFY,      drawing::TextHorizontalAdjust_BLOCK },
    { XML_TOKEN_INVALID, drawing::TextHorizontalAdjust(0) }
};

SvXMLEnumMapEntry<drawing::TextVerticalAdjust> const pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,          drawing::TextVerticalAdjust_TOP },
    { XML_MIDDLE,       drawing::TextVerticalAdjust_CENTER },
    { XML_BOTTOM,       drawing::TextVerticalAdjust_BOTTOM },
    { XML_JUSTIFY,      drawing::TextVerticalAdjust_BLOCK },
    { XML_TOKEN_INVALID, drawing::TextVerticalAdjust(0) }
};

// note: PROPORTIONAL and ALLLINES are the same thing now!
SvXMLEnumMapEntry<drawing::TextFitToSizeType> const pXML_FitToSize_Enum_Odf12[] =
{
    { XML_FALSE,        drawing::TextFitToSizeType_NONE },
    { XML_TRUE,         drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_TRUE,         drawing::TextFitToSizeType_ALLLINES },
    { XML_FALSE,        drawing::TextFitToSizeType_AUTOFIT },
    { XML_TOKEN_INVALID, drawing::TextFitToSizeType(0) }
};

SvXMLEnumMapEntry<drawing::TextFitToSizeType> const pXML_FitToSize_Enum[] =
{
    { XML_FALSE,        drawing::TextFitToSizeType_NONE },
    { XML_TRUE,         drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_ALL,          drawing::TextFitToSizeType_ALLLINES },
    { XML_SHRINK_TO_FIT,drawing::TextFitToSizeType_AUTOFIT },
    { XML_TOKEN_INVALID, drawing::TextFitToSizeType(0) }
};

SvXMLEnumMapEntry<drawing::TextFitToSizeType> const pXML_ShrinkToFit_Enum[] =
{
    { XML_FALSE,        drawing::TextFitToSizeType_NONE },
    { XML_FALSE,        drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_FALSE,        drawing::TextFitToSizeType_ALLLINES },
    { XML_TRUE,         drawing::TextFitToSizeType_AUTOFIT },
    { XML_TOKEN_INVALID, drawing::TextFitToSizeType(0) }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_MeasureUnit_Enum[] =
{
    { XML_AUTOMATIC,    0 },
    { XML_MM,           1 },
    { XML_UNIT_CM,      2 },
    { XML_UNIT_M,       3 },
    { XML_KM,           4 },
    { XML_UNIT_PT,      6 },
    { XML_UNIT_PC,      7 },
    { XML_IN,           8 },
    { XML_UNIT_FOOT,    9 },
    { XML_MI,           10 },
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<drawing::MeasureTextHorzPos> const pXML_Measure_HAlign_Enum[] =
{
    { XML_AUTOMATIC,        drawing::MeasureTextHorzPos_AUTO },
    { XML_LEFT_OUTSIDE,     drawing::MeasureTextHorzPos_LEFTOUTSIDE },
    { XML_INSIDE,           drawing::MeasureTextHorzPos_INSIDE },
    { XML_RIGHT_OUTSIDE,    drawing::MeasureTextHorzPos_RIGHTOUTSIDE},
    { XML_TOKEN_INVALID, drawing::MeasureTextHorzPos(0) }
};

SvXMLEnumMapEntry<drawing::MeasureTextVertPos> const pXML_Measure_VAlign_Enum[] =
{
    { XML_AUTOMATIC,    drawing::MeasureTextVertPos_AUTO },
    { XML_ABOVE,        drawing::MeasureTextVertPos_EAST },
    { XML_BELOW,        drawing::MeasureTextVertPos_WEST },
    { XML_CENTER,       drawing::MeasureTextVertPos_CENTERED },
    { XML_TOKEN_INVALID, drawing::MeasureTextVertPos(0) }
};

// #FontWork#
SvXMLEnumMapEntry<sal_Int32> const pXML_Fontwork_Style_Enum[] =
{
    { XML_ROTATE,       0 }, //XFormTextStyle::Rotate,
    { XML_UPRIGHT,      1 }, //XFormTextStyle::Upright,
    { XML_SLANT_X,      2 }, //XFormTextStyle::SlantX,
    { XML_SLANT_Y,      3 }, //XFormTextStyle::SlantY,
    { XML_NONE,         4 }, //XFormTextStyle::NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_Fontwork_Adjust_Enum[] =
{
    { XML_LEFT,         0 }, //XFormTextAdjust::Left,
    { XML_RIGHT,        1 }, //XFormTextAdjust::Right,
    { XML_AUTOSIZE,     2 }, //XFormTextAdjust::AutoSize,
    { XML_CENTER,       3 }, //XFormTextAdjust::Center
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_Fontwork_Shadow_Enum[] =
{
    { XML_NORMAL,       0 }, //XFormTextShadow::Normal,
    { XML_SLANT,        1 }, //XFormTextShadow::Slant,
    { XML_NONE,         2 }, //XFormTextShadow::NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_Fontwork_Form_Enum[] =
{
    { XML_NONE,             0 }, //XFTFORM_NONE,
    { XML_TOPCIRCLE,        1 }, //XFTFORM_TOPCIRC,
    { XML_BOTTOMCIRCLE,     2 }, //XFTFORM_BOTCIRC,
    { XML_LEFTCIRCLE,       3 }, //XFTFORM_LFTCIRC,
    { XML_RIGHTCIRCLE,      4 }, //XFTFORM_RGTCIRC,
    { XML_TOPARC,           5 }, //XFTFORM_TOPARC,
    { XML_BOTTOMARC,        6 }, //XFTFORM_BOTARC,
    { XML_LEFTARC,          7 }, //XFTFORM_LFTARC,
    { XML_RIGHTARC,         8 }, //XFTFORM_RGTARC,
    { XML_BUTTON1,          9 }, //XFTFORM_BUTTON1,
    { XML_BUTTON2,          10 }, //XFTFORM_BUTTON2,
    { XML_BUTTON3,          11 }, //XFTFORM_BUTTON3,
    { XML_BUTTON4,          12 }, //XFTFORM_BUTTON4
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_Caption_Esc_Dir_Enum[] =
{
    { XML_HORIZONTAL,       0 }, //SdrCaptionEscDir::Horizontal,
    { XML_VERTICAL,         1 }, //SdrCaptionEscDir::Vertical,
    { XML_AUTO,             2 }, //SdrCaptionEscDir::BestFit,
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry<sal_Int32> const pXML_Caption_Type_Enum[] =
{
    { XML_STRAIGHT_LINE,            0 }, //SdrCaptionType::Type1,
    { XML_ANGLED_LINE,              1 }, //SdrCaptionType::Type2,
    { XML_ANGLED_CONNECTOR_LINE,    2 }, //SdrCaptionType::Type3,
    { XML_TOKEN_INVALID,0 }
};

namespace {

class XMLCaptionEscapeRelative : public XMLPropertyHandler
{
public:
    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;
};

}

bool XMLCaptionEscapeRelative::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue;

    if (!::sax::Converter::convertPercent( nValue, rStrImpValue ))
        return false;

    nValue *= 100;
    rValue <<= nValue;
    return true;
}

bool XMLCaptionEscapeRelative::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    if( !(rValue >>= nValue ) )
        return false;

    nValue /= 100;
    OUStringBuffer aOut;
    ::sax::Converter::convertPercent( aOut, nValue );
    rStrExpValue = aOut.makeStringAndClear();
    return true;
}

namespace {

class XMLMoveSizeProtectHdl : public XMLPropertyHandler
{
public:
    explicit XMLMoveSizeProtectHdl( sal_Int32 nType ) : mnType( nType ) {}

    virtual bool importXML(
            const OUString& rStrImpValue,
            css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML(
            OUString& rStrExpValue,
            const css::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const override;
private:
    const sal_Int32 mnType;
};

}

bool XMLMoveSizeProtectHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    const bool bValue = rStrImpValue.indexOf( GetXMLToken( mnType == XML_SD_TYPE_MOVE_PROTECT ? XML_POSITION : XML_SIZE ) ) != -1;
    rValue <<= bValue;
    return true;
}

bool XMLMoveSizeProtectHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bValue;
    if( !(rValue >>= bValue ) )
        return false;

    if( bValue )
    {
        if( !rStrExpValue.isEmpty() )
            rStrExpValue += " ";

        rStrExpValue += GetXMLToken( mnType == XML_SD_TYPE_MOVE_PROTECT ? XML_POSITION : XML_SIZE );
    }

    return true;
}

namespace {

class XMLSdHeaderFooterVisibilityTypeHdl : public XMLPropertyHandler
{
public:
    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

}

bool XMLSdHeaderFooterVisibilityTypeHdl::importXML(
        const OUString& rStrImpValue,
        css::uno::Any& rValue,
        const SvXMLUnitConverter& ) const
{
    // #i38644#
    // attributes with this type where saved with VISIBLE|HIDDEN prior
    // to src680m67. So we have to import that correctly
    const bool bBool = IsXMLToken(rStrImpValue, XML_TRUE) || IsXMLToken(rStrImpValue, XML_VISIBLE);
    rValue <<= bBool;
    return bBool || IsXMLToken(rStrImpValue, XML_FALSE) || IsXMLToken(rStrImpValue, XML_HIDDEN);
}

bool XMLSdHeaderFooterVisibilityTypeHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    bool bValue;

    if (rValue >>= bValue)
    {
        OUStringBuffer aOut;
        ::sax::Converter::convertBool( aOut, bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

namespace {

class XMLSdRotationAngleTypeHdl : public XMLPropertyHandler
{
public:
    virtual bool importXML(const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter) const override;
    virtual bool exportXML(OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter) const override;
};

}

bool XMLSdRotationAngleTypeHdl::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter&) const
{
    sal_Int32 nValue;
    bool const bRet = ::sax::Converter::convertNumber(nValue, rStrImpValue);
    if (bRet)
    {
        nValue = (nValue % 360);
        if (nValue < 0)
            nValue = 360 + nValue;
        sal_Int32 nAngle;
        if (nValue < 45 || nValue > 315)
            nAngle = 0;
        else if (nValue < 180)
            nAngle = 9000;
        else /* if nValue <= 315 ) */
            nAngle = 27000;

        rValue <<= nAngle;
    }
    return bRet;
}

bool XMLSdRotationAngleTypeHdl::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter&) const
{
    sal_Int32 nAngle;
    bool bRet = (rValue >>= nAngle) && nAngle != 0;
    if (bRet)
    {
        rStrExpValue = OUString::number(nAngle / 100);
    }
    return bRet;
}

namespace {

class XMLFitToSizeEnumPropertyHdl : public XMLEnumPropertyHdl
{
public:
    XMLFitToSizeEnumPropertyHdl(
            const SvXMLEnumMapEntry<drawing::TextFitToSizeType> *const pMap)
        : XMLEnumPropertyHdl(pMap) {}

    virtual bool importXML(const OUString& rStrImpValue, uno::Any& rValue,
                           const SvXMLUnitConverter& rUC) const override
    {
        // we don't know here what the actual attribute name is -
        // but we can combine the 2 attributes by just taking the
        // "largest" result value; this can never result in ALLLINES
        // so the implementation has to interpret PROPORTIONAL as ALLLINES;
        // both "true" is invalid anyway.
        Any any;
        auto const bRet = XMLEnumPropertyHdl::importXML(rStrImpValue, any, rUC);
        if (!bRet)
        {
            return false;
        }
        assert(any.hasValue());
        if (!rValue.hasValue() ||
            rValue.get<drawing::TextFitToSizeType>() < any.get<drawing::TextFitToSizeType>())
        {
            rValue = any;
        }
        return true;
    }
};

}

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > const & xModel, SvXMLImport& rImport )
: mxModel( xModel ), mpExport(nullptr), mpImport( &rImport )
{
}

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > const & xModel, SvXMLExport& rExport )
: mxModel( xModel ), mpExport( &rExport ), mpImport(nullptr)
{
}

XMLSdPropHdlFactory::~XMLSdPropHdlFactory()
{
}

const XMLPropertyHandler* XMLSdPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    const XMLPropertyHandler* pHdl = XMLPropertyHandlerFactory::GetPropertyHandler( nType );
    if(!pHdl)
    {
        switch(nType)
        {
            case XML_SD_TYPE_STROKE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_LineStyle_EnumMap);
                break;
            }
            case XML_SD_TYPE_LINEJOIN :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_LineJoint_EnumMap);
                break;
            }
            case XML_SD_TYPE_LINECAP :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_LineCap_EnumMap );
                break;
            }
            case XML_SD_TYPE_FILLSTYLE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_FillStyle_EnumMap );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_TYPE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_PresChange_EnumMap );
                break;
            }
            case XML_SD_TYPE_VISIBLE_HIDDEN:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_VISIBLE), GetXMLToken(XML_HIDDEN) );
                break;
            }
            case XML_TYPE_SD_MIRROR:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_HORIZONTAL), GetXMLToken(XML_NONE) );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_STYLE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_FadeEffect_EnumMap );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_SPEED :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TransSpeed_EnumMap );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_DURATION :
            {
                pHdl = new XMLDurationPropertyHdl;
                break;
            }
            case XML_SD_TYPE_TEXT_CROSSEDOUT :
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_SOLID), GetXMLToken(XML_NONE) );
                break;
            }
            case XML_SD_TYPE_OPACITY :
            {
                pHdl = new XMLOpacityPropertyHdl(mpImport);
                break;
            }
            case XML_SD_TYPE_WRITINGMODE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_WritingMode_EnumMap );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_VISIBILITY :
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_VISIBLE), GetXMLToken(XML_HIDDEN) );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_BACKSIZE:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_FULL), GetXMLToken(XML_BORDER) );
                break;
            }

            // 3D Properties

            case XML_SD_TYPE_BACKFACE_CULLING:
            {
                // #87922# DoubleSided -> BackfaceCulling
                // This sal_Bool needs to be flipped, DoubleSided sal_True -> NO Backface culling
                // and vice versa.
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_DISABLED), GetXMLToken(XML_ENABLED) );
                break;
            }

            case XML_SD_TYPE_NORMALS_KIND:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_NormalsKind_EnumMap );
                break;
            }
            case XML_SD_TYPE_NORMALS_DIRECTION:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_NORMAL), GetXMLToken(XML_INVERSE) );
                break;
            }
            case XML_SD_TYPE_TEX_GENERATION_MODE_X:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexGenerationX_EnumMap );
                break;
            }
            case XML_SD_TYPE_TEX_GENERATION_MODE_Y:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexGenerationY_EnumMap );
                break;
            }
            case XML_SD_TYPE_TEX_KIND:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexKind_EnumMap );
                break;
            }
            case XML_SD_TYPE_TEX_MODE:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexMode_EnumMap );
                break;
            }
            case XML_SD_TYPE_NUMBULLET:
            {
                uno::Reference<ucb::XAnyCompareFactory> xCompareFac( mxModel, uno::UNO_QUERY );
                uno::Reference<ucb::XAnyCompare> xCompare;
                if( xCompareFac.is() )
                    xCompare = xCompareFac->createAnyCompareByName( "NumberingRules" );

                pHdl = new XMLNumRulePropHdl( xCompare );
                break;
            }
            case XML_SD_TYPE_BITMAP_MODE:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_BitmapMode_EnumMap );
                break;
            }
            case XML_SD_TYPE_BITMAPREPOFFSETX:
            case XML_SD_TYPE_BITMAPREPOFFSETY:
            {
                pHdl = new XMLBitmapRepeatOffsetPropertyHandler( nType == XML_SD_TYPE_BITMAPREPOFFSETX );
                break;
            }
            case XML_SD_TYPE_FILLBITMAPSIZE:
            {
                pHdl = new XMLFillBitmapSizePropertyHandler();
                break;
            }
            case XML_SD_TYPE_LOGICAL_SIZE:
            {
                pHdl = new XMLBitmapLogicalSizePropertyHandler();
                break;
            }
            case XML_SD_TYPE_BITMAP_REFPOINT:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_RefPoint_EnumMap);
                break;
            }
            case XML_TYPE_TEXT_ANIMATION:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Enum);
                break;
            case XML_TYPE_TEXT_ANIMATION_BLINKING:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Blinking_Enum);
                break;
            case XML_TYPE_TEXT_ANIMATION_DIRECTION:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimationDirection_Enum);
                break;
            case XML_TYPE_TEXT_ANIMATION_STEPS:
                pHdl = new XMLTextAnimationStepPropertyHdl;
                break;
            case XML_SD_TYPE_TEXT_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAlign_Enum);
                break;
            case XML_SD_TYPE_VERTICAL_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_VerticalAlign_Enum);
                break;
            case XML_SD_TYPE_FITTOSIZE:
                {
                    if (mpExport
                        && (mpExport->getSaneDefaultVersion() // tdf#97630
                                != SvtSaveOptions::ODFSVER_012_EXT_COMPAT))
                    {
                        pHdl = new XMLFitToSizeEnumPropertyHdl(pXML_FitToSize_Enum_Odf12);
                    }
                    else
                    {   // import all values written by old LO
                        pHdl = new XMLFitToSizeEnumPropertyHdl(pXML_FitToSize_Enum);
                    }
                }
                break;
            case XML_SD_TYPE_FITTOSIZE_AUTOFIT:
                {
                    pHdl = new XMLFitToSizeEnumPropertyHdl(pXML_ShrinkToFit_Enum);
                }
                break;
            case XML_SD_TYPE_MEASURE_UNIT:
                pHdl = new XMLEnumPropertyHdl( pXML_MeasureUnit_Enum );
                break;
            case XML_SD_TYPE_MEASURE_HALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_HAlign_Enum);
                break;
            case XML_SD_TYPE_MEASURE_VALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_VAlign_Enum);
                break;
            case XML_SD_TYPE_MEASURE_PLACING:
                {
                    pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_BELOW), GetXMLToken(XML_ABOVE) );
                }
                break;
            case XML_TYPE_TEXT_CLIP11:
                pHdl = new XMLClipPropertyHandler( true );
                break;
            case XML_TYPE_TEXT_CLIP:
                pHdl = new XMLClipPropertyHandler( false );
                break;

            // #FontWork#
            case XML_SD_TYPE_FONTWORK_STYLE     :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Style_Enum );
                break;
            case XML_SD_TYPE_FONTWORK_ADJUST        :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Adjust_Enum );
                break;
            case XML_SD_TYPE_FONTWORK_SHADOW        :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Shadow_Enum );
                break;
            case XML_SD_TYPE_FONTWORK_FORM      :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Form_Enum );
                break;

            case XML_SD_TYPE_CONTROL_BORDER:
                pHdl = new ::xmloff::OControlBorderHandler( ::xmloff::OControlBorderHandler::STYLE );
                break;
            case XML_SD_TYPE_CONTROL_BORDER_COLOR:
                pHdl = new ::xmloff::OControlBorderHandler( ::xmloff::OControlBorderHandler::COLOR );
                break;
            case XML_SD_TYPE_IMAGE_SCALE_MODE:
                pHdl = new ::xmloff::ImageScaleModeHandler;
                break;
            case XML_TYPE_CONTROL_TEXT_EMPHASIZE:
                pHdl = new ::xmloff::OControlTextEmphasisHandler;
                break;

            case XML_SD_TYPE_CAPTION_ANGLE_TYPE:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_FIXED), GetXMLToken(XML_FREE) );
                break;
            }
            case XML_SD_TYPE_CAPTION_IS_ESC_REL:
                pHdl = new XMLIsPercentagePropertyHandler;
                break;
            case XML_SD_TYPE_CAPTION_ESC_REL:
                pHdl = new XMLCaptionEscapeRelative;
                break;
            case XML_SD_TYPE_CAPTION_ESC_ABS:
                pHdl = new XMLPercentOrMeasurePropertyHandler;
                break;
            case XML_SD_TYPE_CAPTION_ESC_DIR:
                pHdl = new XMLEnumPropertyHdl( pXML_Caption_Esc_Dir_Enum );
                break;
            case XML_SD_TYPE_CAPTION_TYPE:
                pHdl = new XMLEnumPropertyHdl( pXML_Caption_Type_Enum );
                break;
            case XML_SD_TYPE_DATETIMEUPDATE:
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_FIXED), GetXMLToken(XML_VARIABLE) );
                break;
            case XML_SD_TYPE_DATETIME_FORMAT:
                pHdl = new XMLDateTimeFormatHdl( mpExport );
                break;
            case XML_SD_TYPE_TRANSITION_TYPE:
                pHdl = new XMLEnumPropertyHdl( xmloff::aAnimations_EnumMap_TransitionType );
                break;
            case XML_SD_TYPE_TRANSTIION_SUBTYPE:
                pHdl = new XMLEnumPropertyHdl( xmloff::aAnimations_EnumMap_TransitionSubType );
                break;
            case XML_SD_TYPE_TRANSTIION_DIRECTION:
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_FORWARD), GetXMLToken(XML_REVERSE) );
                break;
            case XML_TYPE_WRAP_OPTION:
                pHdl = new XMLWordWrapPropertyHdl( mpImport );
                break;

            case XML_SD_TYPE_MOVE_PROTECT:
            case XML_SD_TYPE_SIZE_PROTECT:
                pHdl = new XMLMoveSizeProtectHdl( nType );
                break;
            case XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE:
                pHdl = new XMLSdHeaderFooterVisibilityTypeHdl;
                break;
            case XML_SD_TYPE_CELL_ROTATION_ANGLE:
                pHdl = new XMLSdRotationAngleTypeHdl;
                break;
            case XML_TYPE_TEXT_COLUMNS:
                pHdl = new XMLTextColumnsPropertyHandler;
                break;
        }

        if(pHdl)
            PutHdlCache(nType, pHdl);
    }

    return pHdl;
}

XMLShapePropertySetMapper::XMLShapePropertySetMapper(const rtl::Reference< XMLPropertyHandlerFactory >& rFactoryRef,
        bool bForExport)
: XMLPropertySetMapper( aXMLSDProperties, rFactoryRef, bForExport )
{
}

XMLShapePropertySetMapper::~XMLShapePropertySetMapper()
{
}

XMLShapeExportPropertyMapper::XMLShapeExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport )
: SvXMLExportPropertyMapper( rMapper )
, maNumRuleExp( rExport )
, mbIsInAutoStyles( true )
{
}

XMLShapeExportPropertyMapper::~XMLShapeExportPropertyMapper()
{
}

void XMLShapeExportPropertyMapper::ContextFilter(
    bool bEnableFoFontFamily,
    std::vector< XMLPropertyState >& rProperties,
    const uno::Reference< beans::XPropertySet >& rPropSet ) const
{
    XMLPropertyState* pRepeatOffsetX = nullptr;
    XMLPropertyState* pRepeatOffsetY = nullptr;
    XMLPropertyState* pTextAnimationBlinking = nullptr;
    XMLPropertyState* pTextAnimationKind = nullptr;

    // #FontWork#
    XMLPropertyState* pFontWorkStyle = nullptr;
    XMLPropertyState* pFontWorkAdjust = nullptr;
    XMLPropertyState* pFontWorkDistance = nullptr;
    XMLPropertyState* pFontWorkStart = nullptr;
    XMLPropertyState* pFontWorkMirror = nullptr;
    XMLPropertyState* pFontWorkOutline = nullptr;
    XMLPropertyState* pFontWorkShadow = nullptr;
    XMLPropertyState* pFontWorkShadowColor = nullptr;
    XMLPropertyState* pFontWorkShadowOffsetx = nullptr;
    XMLPropertyState* pFontWorkShadowOffsety = nullptr;
    XMLPropertyState* pFontWorkForm = nullptr;
    XMLPropertyState* pFontWorkHideform = nullptr;
    XMLPropertyState* pFontWorkShadowTransparence = nullptr;

    // OLE
    XMLPropertyState* pOLEVisAreaLeft = nullptr;
    XMLPropertyState* pOLEVisAreaTop = nullptr;
    XMLPropertyState* pOLEVisAreaWidth = nullptr;
    XMLPropertyState* pOLEVisAreaHeight = nullptr;
    XMLPropertyState* pOLEIsInternal = nullptr;

    // caption
    XMLPropertyState* pCaptionIsEscRel = nullptr;
    XMLPropertyState* pCaptionEscRel = nullptr;
    XMLPropertyState* pCaptionEscAbs = nullptr;

    // filter fo:clip
    XMLPropertyState* pClip11State = nullptr;
    XMLPropertyState* pClipState = nullptr;

    XMLPropertyState* pShapeWritingMode = nullptr;
    XMLPropertyState* pTextWritingMode = nullptr;
    XMLPropertyState* pControlWritingMode = nullptr;

    // filter properties
    for( auto& rProp : rProperties )
    {
        XMLPropertyState *property = &rProp;
        if( property->mnIndex == -1 )
            continue;

        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ))
        {
            case CTF_NUMBERINGRULES:
                {
                    if( mbIsInAutoStyles )
                        property->mnIndex = -1;
                }
                break;
            case CTF_SD_NUMBERINGRULES_NAME:
                {
                    // this property is not exported in the style:properties element
                    // because it's an XIndexAccess and not a string.
                    // This will be handled in SvXMLAutoStylePoolP::exportStyleAttributes
                    // This is suboptimal
                    if( !mbIsInAutoStyles )
                        property->mnIndex = -1;
                }
                break;
            case CTF_WRITINGMODE:
                pShapeWritingMode = property;
                break;
            case CTF_CONTROLWRITINGMODE:
                pControlWritingMode = property;
                break;
            case CTF_TEXTWRITINGMODE:
                pTextWritingMode = property;
                break;
            case CTF_REPEAT_OFFSET_X:
                pRepeatOffsetX = property;
                break;

            case CTF_REPEAT_OFFSET_Y:
                pRepeatOffsetY = property;
                break;

            case CTF_DASHNAME:
            case CTF_FILLGRADIENTNAME:
            case CTF_FILLHATCHNAME:
            case CTF_FILLBITMAPNAME:
                {
                    OUString aStr;
                    if( (property->maValue >>= aStr) && aStr.isEmpty() )
                        property->mnIndex = -1;
                }
                break;
            case CTF_TEXTANIMATION_BLINKING:
                pTextAnimationBlinking = property;
                break;
            case CTF_TEXTANIMATION_KIND:
                pTextAnimationKind = property;
                break;

            // #FontWork#
            case CTF_FONTWORK_STYLE:                pFontWorkStyle = property;              break;
            case CTF_FONTWORK_ADJUST:               pFontWorkAdjust = property;             break;
            case CTF_FONTWORK_DISTANCE:             pFontWorkDistance = property;           break;
            case CTF_FONTWORK_START:                pFontWorkStart = property;              break;
            case CTF_FONTWORK_MIRROR:               pFontWorkMirror = property;             break;
            case CTF_FONTWORK_OUTLINE:              pFontWorkOutline = property;            break;
            case CTF_FONTWORK_SHADOW:               pFontWorkShadow = property;             break;
            case CTF_FONTWORK_SHADOWCOLOR:          pFontWorkShadowColor = property;        break;
            case CTF_FONTWORK_SHADOWOFFSETX:        pFontWorkShadowOffsetx = property;      break;
            case CTF_FONTWORK_SHADOWOFFSETY:        pFontWorkShadowOffsety = property;      break;
            case CTF_FONTWORK_FORM:                 pFontWorkForm = property;               break;
            case CTF_FONTWORK_HIDEFORM:             pFontWorkHideform = property;           break;
            case CTF_FONTWORK_SHADOWTRANSPARENCE:   pFontWorkShadowTransparence = property; break;

            // OLE
            case CTF_SD_OLE_VIS_AREA_EXPORT_LEFT:           pOLEVisAreaLeft = property;     break;
            case CTF_SD_OLE_VIS_AREA_EXPORT_TOP:            pOLEVisAreaTop = property;      break;
            case CTF_SD_OLE_VIS_AREA_EXPORT_WIDTH:          pOLEVisAreaWidth = property;    break;
            case CTF_SD_OLE_VIS_AREA_EXPORT_HEIGHT:     pOLEVisAreaHeight = property;   break;
            case CTF_SD_OLE_ISINTERNAL:             pOLEIsInternal = property;      break;

            case CTF_FRAME_DISPLAY_SCROLLBAR:
                {
                    if( !property->maValue.hasValue() )
                        property->mnIndex = -1;
                }
                break;
            case CTF_FRAME_MARGIN_HORI:
            case CTF_FRAME_MARGIN_VERT:
                {
                    sal_Int32 nValue = 0;
                    if( (property->maValue >>= nValue) && (nValue < 0) )
                        property->mnIndex = -1;
                }
                break;

            case CTF_SD_MOVE_PROTECT:
                {
                    bool bProtected;
                    if( (property->maValue >>= bProtected) && !bProtected )
                        property->mnIndex = -1;
                }
                break;
            case CTF_SD_SIZE_PROTECT:
                {
                    bool bProtected;
                    if( (property->maValue >>= bProtected) && !bProtected )
                        property->mnIndex = -1;
                }
                break;
            case CTF_CAPTION_ISESCREL:              pCaptionIsEscRel = property;    break;
            case CTF_CAPTION_ESCREL:                pCaptionEscRel = property;      break;
            case CTF_CAPTION_ESCABS:                pCaptionEscAbs = property;      break;
            case CTF_TEXT_CLIP11:           pClip11State = property; break;
            case CTF_TEXT_CLIP:             pClipState = property; break;
        }
    }

    // check for duplicate writing mode
    if( pShapeWritingMode && (pTextWritingMode || pControlWritingMode) )
    {
        if( pTextWritingMode )
            pTextWritingMode->mnIndex = -1;
        if( pControlWritingMode )
            pControlWritingMode->mnIndex = -1;

        text::WritingMode eWritingMode;
        if( pShapeWritingMode->maValue >>= eWritingMode )
        {
            if( text::WritingMode_LR_TB == eWritingMode )
            {
                pShapeWritingMode->mnIndex = -1;
                pShapeWritingMode = nullptr;
            }
        }
    }
    else if( pTextWritingMode && pControlWritingMode )
    {
        pControlWritingMode->mnIndex = -1;

        sal_Int32 eWritingMode;
        if (pTextWritingMode->maValue >>= eWritingMode)
        {
            if (text::WritingMode2::LR_TB == eWritingMode)
            {
                pTextWritingMode->mnIndex = -1;
                pTextWritingMode = nullptr;
            }
        }
    }

    // do not export visual area for internal ole objects
    if( pOLEIsInternal )
    {
        bool bInternal;
        if( (pOLEIsInternal->maValue >>= bInternal) && !bInternal )
        {
            try
            {
                awt::Rectangle aRect;
                if( rPropSet->getPropertyValue( "VisibleArea" ) >>= aRect )
                {
                    if( pOLEVisAreaLeft )
                    {
                        pOLEVisAreaLeft->mnIndex = getPropertySetMapper()->FindEntryIndex( CTF_SD_OLE_VIS_AREA_IMPORT_LEFT );
                        pOLEVisAreaLeft->maValue <<= aRect;
                    }
                    if( pOLEVisAreaTop )
                    {
                        pOLEVisAreaTop->mnIndex = getPropertySetMapper()->FindEntryIndex( CTF_SD_OLE_VIS_AREA_IMPORT_TOP );
                        pOLEVisAreaTop->maValue <<=  aRect;
                    }
                    if( pOLEVisAreaWidth )
                    {
                        pOLEVisAreaWidth->mnIndex = getPropertySetMapper()->FindEntryIndex( CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH );
                        pOLEVisAreaWidth->maValue <<= aRect;
                    }
                    if( pOLEVisAreaHeight )
                    {
                        pOLEVisAreaHeight->mnIndex = getPropertySetMapper()->FindEntryIndex( CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT );
                        pOLEVisAreaHeight->maValue <<= aRect;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }
        else
        {
            if( pOLEVisAreaLeft )   pOLEVisAreaLeft->mnIndex = -1;
            if( pOLEVisAreaTop )    pOLEVisAreaTop->mnIndex = -1;
            if( pOLEVisAreaWidth )  pOLEVisAreaWidth->mnIndex = -1;
            if( pOLEVisAreaHeight ) pOLEVisAreaHeight->mnIndex = -1;
        }

        pOLEIsInternal->mnIndex = -1;
    }

    if( pTextAnimationBlinking && pTextAnimationKind )
    {
        drawing::TextAnimationKind eKind;
        if( (pTextAnimationKind->maValue >>= eKind) && eKind != drawing::TextAnimationKind_BLINK )
        {
            pTextAnimationBlinking->mnIndex = -1;
        }
        else
        {
            pTextAnimationKind->mnIndex = -1;
        }
    }

    if( pRepeatOffsetX && pRepeatOffsetY )
    {
        sal_Int32 nOffset = 0;
        if( ( pRepeatOffsetX->maValue >>= nOffset ) && ( nOffset == 0 ) )
            pRepeatOffsetX->mnIndex = -1;
        else
            pRepeatOffsetY->mnIndex = -1;
    }

    if(pFontWorkStyle)
    {
        // #FontWork#
        sal_Int32 nStyle = 0;

        if(pFontWorkStyle->maValue >>= nStyle)
        {
            if(/*XFormTextStyle::NONE*/4 == nStyle)
            {
                pFontWorkStyle->mnIndex = -1;
                if(pFontWorkAdjust)
                    pFontWorkAdjust->mnIndex = -1;
                if(pFontWorkDistance)
                    pFontWorkDistance->mnIndex = -1;
                if(pFontWorkStart)
                    pFontWorkStart->mnIndex = -1;
                if(pFontWorkMirror)
                    pFontWorkMirror->mnIndex = -1;
                if(pFontWorkOutline)
                    pFontWorkOutline->mnIndex = -1;
                if(pFontWorkShadow)
                    pFontWorkShadow->mnIndex = -1;
                if(pFontWorkShadowColor)
                    pFontWorkShadowColor->mnIndex = -1;
                if(pFontWorkShadowOffsetx)
                    pFontWorkShadowOffsetx->mnIndex = -1;
                if(pFontWorkShadowOffsety)
                    pFontWorkShadowOffsety->mnIndex = -1;
                if(pFontWorkForm)
                    pFontWorkForm->mnIndex = -1;
                if(pFontWorkHideform)
                    pFontWorkHideform->mnIndex = -1;
                if(pFontWorkShadowTransparence)
                    pFontWorkShadowTransparence->mnIndex = -1;
            }
        }
    }

    if( pCaptionIsEscRel )
    {
        bool bIsRel = false;
        pCaptionIsEscRel->maValue >>= bIsRel;

        if( bIsRel )
        {
            if( pCaptionEscAbs )
                pCaptionEscAbs->mnIndex = -1;
        }
        else
        {
            if( pCaptionEscRel )
                pCaptionEscRel->mnIndex = -1;
        }

        pCaptionIsEscRel->mnIndex = -1;
    }

    if( pClipState != nullptr && pClip11State != nullptr  )
        pClip11State->mnIndex = -1;

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

void XMLShapeExportPropertyMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
        case CTF_SD_CONTROL_SHAPE_DATA_STYLE:
            // not to be handled by the base class
            break;

        default:
            SvXMLExportPropertyMapper::handleSpecialItem( rAttrList, rProperty, rUnitConverter, rNamespaceMap, pProperties, nIdx );
            break;
    }
}

void XMLShapeExportPropertyMapper::handleElementItem(
    SvXMLExport& rExport,
    const XMLPropertyState& rProperty,
    SvXmlExportFlags nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
        case CTF_NUMBERINGRULES:
            {
                // only export list-styles as elements in styles section
                if( !mbIsInAutoStyles )
                {
                    uno::Reference< container::XIndexReplace > xNumRule( rProperty.maValue, uno::UNO_QUERY );
                    if( xNumRule.is() )
                        const_cast<XMLShapeExportPropertyMapper*>(this)->maNumRuleExp.exportNumberingRule(GetStyleName(), false, xNumRule);
                }
            }
            break;
        default:
            SvXMLExportPropertyMapper::handleElementItem( rExport, rProperty, nFlags, pProperties, nIdx );
            break;
    }
}

XMLPageExportPropertyMapper::XMLPageExportPropertyMapper( const rtl::Reference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport ) :
        SvXMLExportPropertyMapper( rMapper ),
        mrExport( rExport )
{
}

XMLPageExportPropertyMapper::~XMLPageExportPropertyMapper()
{
}

void XMLPageExportPropertyMapper::ContextFilter(
    bool bEnableFoFontFamily,
    std::vector< XMLPropertyState >& rProperties,
    const uno::Reference< beans::XPropertySet >& rPropSet ) const
{
    XMLPropertyState* pRepeatOffsetX = nullptr;
    XMLPropertyState* pRepeatOffsetY = nullptr;
    XMLPropertyState* pTransType = nullptr;
    XMLPropertyState* pTransDuration = nullptr;
    XMLPropertyState* pDateTimeUpdate = nullptr;
    XMLPropertyState* pDateTimeFormat = nullptr;
    XMLPropertyState* pTransitionFadeColor = nullptr;

    sal_Int16 nTransitionType = 0;

    // filter properties
    for( auto& rProp : rProperties )
    {
        XMLPropertyState *property = &rProp;
        if( property->mnIndex == -1 )
            continue;

        // find properties with context
        // to prevent writing this property set mnIndex member to -1
        switch( getPropertySetMapper()->GetEntryContextId( property->mnIndex ))
        {

            case CTF_REPEAT_OFFSET_X:
                pRepeatOffsetX = property;
                break;

            case CTF_REPEAT_OFFSET_Y:
                pRepeatOffsetY = property;
                break;
            case CTF_PAGE_TRANS_TYPE:
                pTransType = property;
                break;
            case CTF_PAGE_TRANS_STYLE:
                if( mrExport.getExportFlags() & SvXMLExportFlags::OASIS )
                    (*property).mnIndex = -1;
                break;
            case CTF_PAGE_TRANSITION_TYPE:
                {
                    if( (!(mrExport.getExportFlags() & SvXMLExportFlags::OASIS)) ||
                        (((*property).maValue >>= nTransitionType) && (nTransitionType == 0)) )
                            (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANSITION_SUBTYPE:
                {
                    sal_Int16 nTransitionSubtype = sal_Int16();
                    if( (!(mrExport.getExportFlags() & SvXMLExportFlags::OASIS)) ||
                        (((*property).maValue >>= nTransitionSubtype) && (nTransitionSubtype == 0)) )
                            (*property).mnIndex = -1;

                }
                break;
            case CTF_PAGE_TRANSITION_DIRECTION:
                {
                    bool bDirection;
                    if( (!(mrExport.getExportFlags() & SvXMLExportFlags::OASIS)) ||
                        (((*property).maValue >>= bDirection) && bDirection) )
                            (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANSITION_FADECOLOR:
                if( !(mrExport.getExportFlags() & SvXMLExportFlags::OASIS) )
                    (*property).mnIndex = -1;
                else
                    pTransitionFadeColor = property;
                break;
            case CTF_PAGE_TRANS_SPEED:
                {
                    presentation::AnimationSpeed aEnum;
                    if( ((*property).maValue >>= aEnum) && aEnum == presentation::AnimationSpeed_MEDIUM )
                        (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_VISIBLE:
                {
                    bool bVisible = false;
                    (*property).maValue >>= bVisible;
                    if( bVisible )
                        (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANS_DURATION:
                pTransDuration = property;
                break;
            case CTF_HEADER_TEXT:
            case CTF_FOOTER_TEXT:
            case CTF_DATE_TIME_TEXT:
                {
                    OUString aValue;
                    (*property).maValue >>= aValue;
                    if( aValue.isEmpty() )
                        (*property).mnIndex = -1;
                }
                break;

            case CTF_DATE_TIME_UPDATE:
                pDateTimeUpdate = property;
                break;

            case CTF_DATE_TIME_FORMAT:
                pDateTimeFormat = property;
                break;
        }
    }

    if( pTransitionFadeColor && nTransitionType != css::animations::TransitionType::FADE )
        pTransitionFadeColor->mnIndex = -1;

    if( pDateTimeFormat && pDateTimeUpdate )
    {
        bool bIsFixed = false;
        pDateTimeUpdate->maValue >>= bIsFixed;
        if( bIsFixed )
            pDateTimeFormat->mnIndex = -1;
    }

    if( pRepeatOffsetX && pRepeatOffsetY )
    {
        sal_Int32 nOffset = 0;
        if( ( pRepeatOffsetX->maValue >>= nOffset ) && ( nOffset == 0 ) )
            pRepeatOffsetX->mnIndex = -1;
        else
            pRepeatOffsetY->mnIndex = -1;
    }

    if( pTransType && pTransDuration )
    {
        sal_Int32 nChange = 0;
        pTransType->maValue >>= nChange;

        // only export duration for automatic
        if( nChange != 1 )
            pTransDuration->mnIndex = -1;

        // do not export default transition change
        if( nChange == 0 )
            pTransType->mnIndex = -1;
    }

    SvXMLExportPropertyMapper::ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

void XMLPageExportPropertyMapper::handleElementItem(
    SvXMLExport& rExport,
    const XMLPropertyState& rProperty,
    SvXmlExportFlags nFlags,
    const ::std::vector< XMLPropertyState > *pProperties,
    sal_uInt32 nIdx) const
{
    switch( getPropertySetMapper()->GetEntryContextId( rProperty.mnIndex ) )
    {
        case CTF_PAGE_SOUND_URL:
            {
                OUString aSoundURL;
                if( (rProperty.maValue >>= aSoundURL) && !aSoundURL.isEmpty() )
                {
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, mrExport.GetRelativeReference(aSoundURL) );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                    mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, true, true );
                }
            }
            break;
        default:
            SvXMLExportPropertyMapper::handleElementItem( rExport, rProperty, nFlags, pProperties, nIdx );
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
