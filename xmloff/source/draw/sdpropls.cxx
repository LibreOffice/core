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
#include <utility>
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
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/txtprmap.hxx>
#include <XMLClipPropertyHandler.hxx>
#include <XMLIsPercentagePropertyHandler.hxx>
#include <XMLPercentOrMeasurePropertyHandler.hxx>
#include <XMLTextColumnsPropertyHandler.hxx>
#include <xmloff/XMLComplexColorHandler.hxx>
#include <animations.hxx>
#include <sax/tools/converter.hxx>
#include <xmlsdtypes.hxx>
#include <xmlprop.hxx>

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
#define MAP_END() { nullptr }

// entry list for graphic properties

const XMLPropertyMapEntry aXMLSDProperties[] =
{
    // this entry must be first! this is needed for XMLShapeImportHelper::CreateExternalShapePropMapper

    // ^^^though CreateExternalShapePropMapper is gone now, hmm^^^
    GMAP( PROP_UserDefinedAttributes,          XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    // stroke attributes
    GMAP( PROP_LineStyle,                      XML_NAMESPACE_DRAW, XML_STROKE,                 XML_SD_TYPE_STROKE, 0 ),
    GMAP( PROP_LineDashName,                   XML_NAMESPACE_DRAW, XML_STROKE_DASH,            XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT , CTF_DASHNAME ),
    GMAP( PROP_LineWidth,                      XML_NAMESPACE_SVG,  XML_STROKE_WIDTH,           XML_TYPE_MEASURE, 0 ),
    GMAP_D( PROP_LineColor,                     XML_NAMESPACE_SVG,  XML_STROKE_COLOR,           XML_TYPE_COLOR, 0),
    GMAPV( PROP_LineComplexColor,              XML_NAMESPACE_LO_EXT, XML_STROKE_COMPLEX_COLOR, XML_TYPE_COMPLEX_COLOR|MID_FLAG_ELEMENT_ITEM, CTF_COMPLEX_COLOR, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAP( PROP_LineStartName,                  XML_NAMESPACE_DRAW, XML_MARKER_START,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINESTARTNAME ),
    GMAP( PROP_LineStartWidth,                 XML_NAMESPACE_DRAW, XML_MARKER_START_WIDTH,     XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_LineStartCenter,                XML_NAMESPACE_DRAW, XML_MARKER_START_CENTER,    XML_TYPE_BOOL, 0 ),
    GMAP( PROP_LineEndName,                    XML_NAMESPACE_DRAW, XML_MARKER_END,             XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINEENDNAME ),
    GMAP( PROP_LineEndWidth,                   XML_NAMESPACE_DRAW, XML_MARKER_END_WIDTH,       XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_LineEndCenter,                  XML_NAMESPACE_DRAW, XML_MARKER_END_CENTER,      XML_TYPE_BOOL, 0 ),
    GMAP( PROP_LineTransparence,               XML_NAMESPACE_SVG,  XML_STROKE_OPACITY,         XML_SD_TYPE_OPACITY, 0 ),
    GMAP( PROP_LineJoint,                      XML_NAMESPACE_DRAW, XML_STROKE_LINEJOIN,        XML_SD_TYPE_LINEJOIN, 0 ),
    GMAP( PROP_LineCap,                        XML_NAMESPACE_SVG , XML_STROKE_LINECAP,         XML_SD_TYPE_LINECAP, 0 ),

    // fill attributes
    GMAP( PROP_FillStyle,                      XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, CTF_FILLSTYLE ),
    GMAP_D(PROP_FillColor,                     XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, CTF_FILLCOLOR ),
    GMAP_D(PROP_FillColor2,                    XML_NAMESPACE_DRAW, XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR, 0),
    GMAPV( PROP_FillComplexColor,              XML_NAMESPACE_LO_EXT, XML_FILL_COMPLEX_COLOR, XML_TYPE_COMPLEX_COLOR|MID_FLAG_ELEMENT_ITEM, CTF_COMPLEX_COLOR, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAP( PROP_FillGradientName,               XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    GMAP( PROP_FillGradientStepCount,          XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16, 0 ),
    GMAP( PROP_FillHatchName,                  XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( PROP_FillBackground,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    GMAPV( PROP_FillUseSlideBackground,        XML_NAMESPACE_LO_EXT, XML_FILL_USE_SLIDE_BACKGROUND, XML_TYPE_BOOL, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAP( PROP_FillBitmapName,                 XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    GMAP( PROP_FillTransparence,               XML_NAMESPACE_DRAW, XML_OPACITY,                XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),    // exists in SW, too
    GMAP( PROP_FillTransparenceGradientName,   XML_NAMESPACE_DRAW, XML_OPACITY_NAME,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    GMAP( PROP_FillBitmapSizeX,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_FillBitmapLogicalSize,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_FillBitmapSizeY,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_FillBitmapLogicalSize,          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_FillBitmapMode,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_FillBitmapPositionOffsetX,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    GMAP( PROP_FillBitmapPositionOffsetY,      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    GMAP( PROP_FillBitmapRectanglePoint,       XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    GMAP( PROP_FillBitmapOffsetX,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    GMAP( PROP_FillBitmapOffsetY,              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // text frame attributes
    GMAP( PROP_TextHorizontalAdjust,           XML_NAMESPACE_DRAW, XML_TEXTAREA_HORIZONTAL_ALIGN,  XML_SD_TYPE_TEXT_ALIGN, 0 ),
    GMAP( PROP_TextVerticalAdjust,             XML_NAMESPACE_DRAW, XML_TEXTAREA_VERTICAL_ALIGN,    XML_SD_TYPE_VERTICAL_ALIGN, 0 ),
    GMAP( PROP_TextAutoGrowHeight,             XML_NAMESPACE_DRAW, XML_AUTO_GROW_HEIGHT,       XML_TYPE_BOOL, 0 ),
    GMAP( PROP_TextAutoGrowWidth,              XML_NAMESPACE_DRAW, XML_AUTO_GROW_WIDTH,        XML_TYPE_BOOL, 0 ),
    GMAP( PROP_TextFitToSize,                  XML_NAMESPACE_DRAW, XML_FIT_TO_SIZE,            XML_SD_TYPE_FITTOSIZE|MID_FLAG_MERGE_PROPERTY, 0),
    GMAPV( PROP_TextFitToSize,                 XML_NAMESPACE_STYLE, XML_SHRINK_TO_FIT,         XML_SD_TYPE_FITTOSIZE_AUTOFIT|MID_FLAG_MERGE_PROPERTY, 0, SvtSaveOptions::ODFSVER_012 ),
    GMAP( PROP_TextContourFrame,               XML_NAMESPACE_DRAW, XML_FIT_TO_CONTOUR,         XML_TYPE_BOOL, 0 ),
    GMAP( PROP_TextMaximumFrameHeight,         XML_NAMESPACE_FO,   XML_MAX_HEIGHT,             XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_TextMaximumFrameWidth,          XML_NAMESPACE_FO,   XML_MAX_WIDTH,              XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_TextMinimumFrameHeight,         XML_NAMESPACE_FO,   XML_MIN_HEIGHT,             XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( PROP_TextMinimumFrameWidth,          XML_NAMESPACE_FO,   XML_MIN_WIDTH,              XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_TextUpperDistance,              XML_NAMESPACE_FO,   XML_PADDING_TOP,            XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( PROP_TextLowerDistance,              XML_NAMESPACE_FO,   XML_PADDING_BOTTOM,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( PROP_TextLeftDistance,               XML_NAMESPACE_FO,   XML_PADDING_LEFT,           XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( PROP_TextRightDistance,              XML_NAMESPACE_FO,   XML_PADDING_RIGHT,          XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    PMAP( PROP_TextWritingMode,                XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_SD_TYPE_WRITINGMODE|MID_FLAG_MULTI_PROPERTY, CTF_WRITINGMODE ),
    GMAP( PROP_NumberingRules,                 XML_NAMESPACE_TEXT, XML_LIST_STYLE,             XML_SD_TYPE_NUMBULLET|MID_FLAG_ELEMENT_ITEM, CTF_NUMBERINGRULES ),
    GMAP( PROP_NumberingRules,                 XML_NAMESPACE_TEXT, XML_LIST_STYLE_NAME,        XML_TYPE_STRING, CTF_SD_NUMBERINGRULES_NAME ),
    GMAP( PROP_TextWordWrap,                   XML_NAMESPACE_FO,   XML_WRAP_OPTION,            XML_TYPE_WRAP_OPTION, 0 ),
    GMAP( PROP_TextChainNextName,              XML_NAMESPACE_DRAW,   XML_CHAIN_NEXT_NAME,      XML_TYPE_STRING, 0 ),
    GMAP( PROP_TextClipVerticalOverflow,       XML_NAMESPACE_STYLE, XML_OVERFLOW_BEHAVIOR,     XML_TYPE_TEXT_OVERFLOW_BEHAVIOR, 0 ),

    GMAP( PROP_TextColumns,                    XML_NAMESPACE_STYLE, XML_COLUMNS, XML_TYPE_TEXT_COLUMNS|MID_FLAG_ELEMENT_ITEM, CTF_TEXTCOLUMNS ),

    // shadow attributes
    GMAP( PROP_Shadow,                         XML_NAMESPACE_DRAW, XML_SHADOW,                 XML_SD_TYPE_VISIBLE_HIDDEN, 0 ),
    GMAP( PROP_ShadowXDistance,                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X,        XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_ShadowYDistance,                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y,        XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_ShadowColor,                        XML_NAMESPACE_DRAW, XML_SHADOW_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( PROP_ShadowTransparence,             XML_NAMESPACE_DRAW, XML_SHADOW_OPACITY, XML_TYPE_NEG_PERCENT, 0 ),
    GMAPV( PROP_ShadowBlur,                     XML_NAMESPACE_LO_EXT, XML_SHADOW_BLUR, XML_TYPE_MEASURE, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // glow attributes
    GMAPV( PROP_GlowEffectRadius,                 XML_NAMESPACE_LO_EXT, XML_GLOW_RADIUS,          XML_TYPE_MEASURE  , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_GlowEffectColor,                  XML_NAMESPACE_LO_EXT, XML_GLOW_COLOR,           XML_TYPE_COLOR    , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_GlowEffectTransparency,           XML_NAMESPACE_LO_EXT, XML_GLOW_TRANSPARENCY,    XML_TYPE_PERCENT16, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // soft edge attributes
    GMAPV( PROP_SoftEdgeRadius,                XML_NAMESPACE_LO_EXT, XML_SOFTEDGE_RADIUS,      XML_TYPE_MEASURE          , 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // graphic attributes
    GMAP( PROP_GraphicColorMode,               XML_NAMESPACE_DRAW, XML_COLOR_MODE,             XML_TYPE_COLOR_MODE, 0 ), // exists in SW, too, with same property name
    GMAP( PROP_AdjustLuminance,                XML_NAMESPACE_DRAW, XML_LUMINANCE,              XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( PROP_AdjustContrast,                 XML_NAMESPACE_DRAW, XML_CONTRAST,               XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( PROP_Gamma,                          XML_NAMESPACE_DRAW, XML_GAMMA,                  XML_TYPE_DOUBLE_PERCENT, 0 ), // signed? exists in SW, too, with same property name
    GMAP( PROP_AdjustRed,                      XML_NAMESPACE_DRAW, XML_RED,                    XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( PROP_AdjustGreen,                    XML_NAMESPACE_DRAW, XML_GREEN,                  XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( PROP_AdjustBlue,                     XML_NAMESPACE_DRAW, XML_BLUE,                   XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAPV( PROP_GraphicCrop,                   XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP, CTF_TEXT_CLIP, SvtSaveOptions::ODFSVER_012), // exists in SW, too, with same property name
    GMAP( PROP_GraphicCrop,                    XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP11, CTF_TEXT_CLIP11 ), // exists in SW, too, with same property name
    GMAP( PROP_Transparency,                   XML_NAMESPACE_DRAW, XML_IMAGE_OPACITY,          XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ), // exists in SW, too, with same property name // #i25616#
    GMAP( PROP_IsMirrored,                     XML_NAMESPACE_STYLE,    XML_MIRROR,             XML_TYPE_SD_MIRROR|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too // #i40214#

    // animation text attributes
    TMAP( PROP_TextAnimationKind,              XML_NAMESPACE_STYLE,XML_TEXT_BLINKING,          XML_TYPE_TEXT_ANIMATION_BLINKING, CTF_TEXTANIMATION_BLINKING ),
    GMAP( PROP_TextAnimationKind,              XML_NAMESPACE_TEXT, XML_ANIMATION,              XML_TYPE_TEXT_ANIMATION, CTF_TEXTANIMATION_KIND ),
    GMAP( PROP_TextAnimationDirection,         XML_NAMESPACE_TEXT, XML_ANIMATION_DIRECTION,    XML_TYPE_TEXT_ANIMATION_DIRECTION, 0 ),
    GMAP( PROP_TextAnimationStartInside,       XML_NAMESPACE_TEXT, XML_ANIMATION_START_INSIDE, XML_TYPE_BOOL, 0 ),
    GMAP( PROP_TextAnimationStopInside,            XML_NAMESPACE_TEXT, XML_ANIMATION_STOP_INSIDE,  XML_TYPE_BOOL, 0 ),
    GMAP( PROP_TextAnimationCount,             XML_NAMESPACE_TEXT, XML_ANIMATION_REPEAT,       XML_TYPE_NUMBER16, 0 ),
    GMAP( PROP_TextAnimationDelay,             XML_NAMESPACE_TEXT, XML_ANIMATION_DELAY,        XML_TYPE_DURATION16_MS, 0 ),
    GMAP( PROP_TextAnimationAmount,                XML_NAMESPACE_TEXT, XML_ANIMATION_STEPS,        XML_TYPE_TEXT_ANIMATION_STEPS, 0 ),

    // connector attributes
    GMAP( PROP_EdgeNode1HorzDist,              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL,  XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_EdgeNode1VertDist,              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_EdgeNode2HorzDist,              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_EdgeNode2VertDist,              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL,      XML_TYPE_MEASURE, 0 ),

    // measure attributes
    GMAP( PROP_MeasureLineDistance,                XML_NAMESPACE_DRAW, XML_LINE_DISTANCE,          XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_MeasureHelpLineOverhang,            XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG,         XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_MeasureHelpLineDistance,            XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE,         XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_MeasureHelpLine1Length,         XML_NAMESPACE_DRAW, XML_START_GUIDE,            XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_MeasureHelpLine2Length,         XML_NAMESPACE_DRAW, XML_END_GUIDE,              XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_MeasureTextHorizontalPosition,  XML_NAMESPACE_DRAW, XML_MEASURE_ALIGN,          XML_SD_TYPE_MEASURE_HALIGN, 0 ),
    GMAP( PROP_MeasureTextVerticalPosition,    XML_NAMESPACE_DRAW, XML_MEASURE_VERTICAL_ALIGN, XML_SD_TYPE_MEASURE_VALIGN, 0 ),
    GMAP( PROP_MeasureUnit,                        XML_NAMESPACE_DRAW, XML_UNIT,                   XML_SD_TYPE_MEASURE_UNIT, 0 ),
    GMAP( PROP_MeasureShowUnit,                    XML_NAMESPACE_DRAW, XML_SHOW_UNIT,              XML_TYPE_BOOL, 0 ),
    GMAP( PROP_MeasureBelowReferenceEdge,      XML_NAMESPACE_DRAW, XML_PLACING,                XML_SD_TYPE_MEASURE_PLACING, 0 ),
    GMAP( PROP_MeasureTextRotate90,                XML_NAMESPACE_DRAW, XML_PARALLEL,               XML_TYPE_BOOL, 0 ),
    GMAP( PROP_MeasureDecimalPlaces,           XML_NAMESPACE_DRAW, XML_DECIMAL_PLACES,         XML_TYPE_NUMBER16, 0 ),

    // 3D geometry attributes
    GMAP( PROP_D3DHorizontalSegments,          XML_NAMESPACE_DR3D, XML_HORIZONTAL_SEGMENTS,    XML_TYPE_NUMBER, 0 ),
    GMAP( PROP_D3DVerticalSegments,                XML_NAMESPACE_DR3D, XML_VERTICAL_SEGMENTS,      XML_TYPE_NUMBER, 0 ),
    GMAP( PROP_D3DPercentDiagonal,             XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING,          XML_TYPE_PERCENT, 0 ),
    GMAP( PROP_D3DBackscale,                   XML_NAMESPACE_DR3D, XML_BACK_SCALE,             XML_TYPE_PERCENT, 0 ),
    GMAP( PROP_D3DEndAngle,                        XML_NAMESPACE_DR3D, XML_END_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( PROP_D3DDepth,                       XML_NAMESPACE_DR3D, XML_DEPTH,                  XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_D3DDoubleSided,                 XML_NAMESPACE_DR3D, XML_BACKFACE_CULLING,       XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // #107245# New 3D properties which are possible for lathe and extrude 3d objects
    GMAP( PROP_D3DCloseFront,                  XML_NAMESPACE_DR3D, XML_CLOSE_FRONT,            XML_TYPE_BOOL, 0 ),
    GMAP( PROP_D3DCloseBack,                   XML_NAMESPACE_DR3D, XML_CLOSE_BACK,             XML_TYPE_BOOL, 0 ),

    // 3D lighting attributes
    GMAP( PROP_D3DNormalsKind,                 XML_NAMESPACE_DR3D, XML_NORMALS_KIND,           XML_SD_TYPE_NORMALS_KIND, 0 ),
    GMAP( PROP_D3DNormalsInvert,               XML_NAMESPACE_DR3D, XML_NORMALS_DIRECTION,      XML_SD_TYPE_NORMALS_DIRECTION, 0 ),

    // 3D texture attributes
    GMAP( PROP_D3DTextureProjectionX,          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_X,  XML_SD_TYPE_TEX_GENERATION_MODE_X, 0 ),
    GMAP( PROP_D3DTextureProjectionY,          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_Y,  XML_SD_TYPE_TEX_GENERATION_MODE_Y, 0 ),
    GMAP( PROP_D3DTextureKind,                 XML_NAMESPACE_DR3D, XML_TEX_KIND,               XML_SD_TYPE_TEX_KIND, 0 ),
    GMAP( PROP_D3DTextureMode,                 XML_NAMESPACE_DR3D, XML_TEX_MODE,               XML_SD_TYPE_TEX_MODE, 0 ),
    GMAP( PROP_D3DTextureFilter,               XML_NAMESPACE_DR3D, XML_TEX_FILTER,             XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // 3D material attributes
    GMAP( PROP_D3DMaterialColor,               XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR,          XML_TYPE_COLOR, 0 ),
    GMAP( PROP_D3DMaterialEmission,                XML_NAMESPACE_DR3D, XML_EMISSIVE_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( PROP_D3DMaterialSpecular,                XML_NAMESPACE_DR3D, XML_SPECULAR_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( PROP_D3DMaterialSpecularIntensity,   XML_NAMESPACE_DR3D, XML_SHININESS,              XML_TYPE_PERCENT, 0 ),

    // 3D shadow attributes
    GMAP( PROP_D3DShadow3D,                        XML_NAMESPACE_DR3D, XML_SHADOW,                 XML_SD_TYPE_VISIBLE_HIDDEN, 0 ),

    // #FontWork# attributes
    GMAP( PROP_FontWorkStyle,                  XML_NAMESPACE_DRAW, XML_FONTWORK_STYLE,                 XML_SD_TYPE_FONTWORK_STYLE| MID_FLAG_ELEMENT_ITEM_EXPORT, CTF_FONTWORK_STYLE  ),
    GMAP( PROP_FontWorkAdjust,                 XML_NAMESPACE_DRAW, XML_FONTWORK_ADJUST,                XML_SD_TYPE_FONTWORK_ADJUST | MID_FLAG_ELEMENT_ITEM_EXPORT,CTF_FONTWORK_ADJUST ),
    GMAP( PROP_FontWorkDistance,               XML_NAMESPACE_DRAW, XML_FONTWORK_DISTANCE,              XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_DISTANCE   ),
    GMAP( PROP_FontWorkStart,                  XML_NAMESPACE_DRAW, XML_FONTWORK_START,                 XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_START  ),
    GMAP( PROP_FontWorkMirror,                 XML_NAMESPACE_DRAW, XML_FONTWORK_MIRROR,                XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_MIRROR ),
    GMAP( PROP_FontWorkOutline,                XML_NAMESPACE_DRAW, XML_FONTWORK_OUTLINE,               XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_OUTLINE    ),
    GMAP( PROP_FontWorkShadow,                 XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW,                XML_SD_TYPE_FONTWORK_SHADOW | MID_FLAG_ELEMENT_ITEM_EXPORT,CTF_FONTWORK_SHADOW ),
    GMAP( PROP_FontWorkShadowColor,            XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_COLOR,          XML_TYPE_COLOR | MID_FLAG_ELEMENT_ITEM_EXPORT,             CTF_FONTWORK_SHADOWCOLOR    ),
    GMAP( PROP_FontWorkShadowOffsetX,          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_X,       XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWOFFSETX  ),
    GMAP( PROP_FontWorkShadowOffsetY,          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_Y,       XML_TYPE_MEASURE | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWOFFSETY  ),
    GMAP( PROP_FontWorkForm,                   XML_NAMESPACE_DRAW, XML_FONTWORK_FORM,                  XML_SD_TYPE_FONTWORK_FORM | MID_FLAG_ELEMENT_ITEM_EXPORT,  CTF_FONTWORK_FORM   ),
    GMAP( PROP_FontWorkHideForm,               XML_NAMESPACE_DRAW, XML_FONTWORK_HIDE_FORM,             XML_TYPE_BOOL | MID_FLAG_ELEMENT_ITEM_EXPORT,              CTF_FONTWORK_HIDEFORM   ),
    GMAP( PROP_FontWorkShadowTransparence,     XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_TRANSPARENCE,   XML_TYPE_PERCENT | MID_FLAG_ELEMENT_ITEM_EXPORT,           CTF_FONTWORK_SHADOWTRANSPARENCE ),

    // #FontWork# attributes
    GMAPV( PROP_FontWorkStyle,              XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_STYLE,                 XML_SD_TYPE_FONTWORK_STYLE, CTF_FONTWORK_STYLE,         SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkAdjust,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_ADJUST,                XML_SD_TYPE_FONTWORK_ADJUST,CTF_FONTWORK_ADJUST,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkDistance,           XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_DISTANCE,              XML_TYPE_MEASURE,           CTF_FONTWORK_DISTANCE,      SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkStart,              XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_START,                 XML_TYPE_MEASURE,           CTF_FONTWORK_START,         SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkMirror,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_MIRROR,                XML_TYPE_BOOL,              CTF_FONTWORK_MIRROR,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkOutline,            XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_OUTLINE,               XML_TYPE_BOOL,              CTF_FONTWORK_OUTLINE,       SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkShadow,             XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW,                XML_SD_TYPE_FONTWORK_SHADOW,CTF_FONTWORK_SHADOW,        SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkShadowColor,        XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_COLOR,          XML_TYPE_COLOR,             CTF_FONTWORK_SHADOWCOLOR,   SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkShadowOffsetX,      XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_OFFSET_X,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETX, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkShadowOffsetY,      XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_OFFSET_Y,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETY, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkForm,               XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_FORM,                  XML_SD_TYPE_FONTWORK_FORM,  CTF_FONTWORK_FORM,          SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkHideForm,           XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_HIDE_FORM,             XML_TYPE_BOOL,              CTF_FONTWORK_HIDEFORM,      SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),
    GMAPV( PROP_FontWorkShadowTransparence, XML_NAMESPACE_DRAW_EXT, XML_FONTWORK_SHADOW_TRANSPARENCE,   XML_TYPE_PERCENT,           CTF_FONTWORK_SHADOWTRANSPARENCE, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED),

    // control attributes (border exists one more time for the text additions of shapes)
    GMAP( PROP_ControlSymbolColor,             XML_NAMESPACE_DRAW, XML_SYMBOL_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( PROP_ControlBackground,              XML_NAMESPACE_FO,   XML_BACKGROUND_COLOR,       XML_TYPE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_ControlBorder,                  XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( PROP_ControlBorderColor,             XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER_COLOR|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( PROP_ControlDataStyle,               XML_NAMESPACE_STYLE,XML_DATA_STYLE_NAME,        XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_EXPORT|MID_FLAG_SPECIAL_ITEM, CTF_SD_CONTROL_SHAPE_DATA_STYLE ),
    GMAP( PROP_ControlTextEmphasis,            XML_NAMESPACE_STYLE,XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
    GMAP( PROP_ImageScaleMode,                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_IMAGE_SCALE_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( PROP_ControlWritingMode,             XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT|MID_FLAG_MULTI_PROPERTY, CTF_CONTROLWRITINGMODE ),

    // special entries for floating frames
    GMAP( PROP_FrameIsAutoScroll,          XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_SCROLLBAR,    XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_SCROLLBAR ),
    GMAP( PROP_FrameIsBorder,              XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_BORDER,       XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_BORDER ),
    GMAP( PROP_FrameMarginWidth,           XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_HORIZONTAL,    XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_HORI ),
    GMAP( PROP_FrameMarginHeight,          XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_VERTICAL,      XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_VERT ),
    GMAP( PROP_VisibleArea,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,   CTF_SD_OLE_VIS_AREA_IMPORT_LEFT ),
    GMAP( PROP_VisibleArea,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,    CTF_SD_OLE_VIS_AREA_IMPORT_TOP ),
    GMAP( PROP_VisibleArea,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,  CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH ),
    GMAP( PROP_VisibleArea,                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY, CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT ),
    GMAP( PROP_IsInternal,                 XML_NAMESPACE_DRAW, XML__EMPTY,                     XML_TYPE_BUILDIN_CMP_ONLY,                          CTF_SD_OLE_ISINTERNAL ),
    GMAP( PROP_IsInternal,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,    CTF_SD_OLE_VIS_AREA_EXPORT_LEFT ),
    GMAP( PROP_IsInternal,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,     CTF_SD_OLE_VIS_AREA_EXPORT_TOP ),
    GMAP( PROP_IsInternal,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,   CTF_SD_OLE_VIS_AREA_EXPORT_WIDTH ),
    GMAP( PROP_IsInternal,                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,  CTF_SD_OLE_VIS_AREA_EXPORT_HEIGHT ),

    GMAP( PROP_Aspect,                     XML_NAMESPACE_DRAW, XML_DRAW_ASPECT,                XML_TYPE_TEXT_DRAW_ASPECT|MID_FLAG_MULTI_PROPERTY,  CTF_SD_OLE_ASPECT ),

    // caption properties
    GMAP( PROP_CaptionType,                    XML_NAMESPACE_DRAW, XML_CAPTION_TYPE,               XML_SD_TYPE_CAPTION_TYPE, 0 ),
    GMAP( PROP_CaptionIsFixedAngle,            XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE_TYPE,         XML_SD_TYPE_CAPTION_ANGLE_TYPE, 0 ),
    GMAP( PROP_CaptionAngle,               XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( PROP_CaptionGap,                 XML_NAMESPACE_DRAW, XML_CAPTION_GAP,                XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_CaptionEscapeDirection,     XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE_DIRECTION,   XML_SD_TYPE_CAPTION_ESC_DIR, 0 ),
    GMAP( PROP_CaptionIsEscapeRelative,    XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_IS_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ISESCREL ),
    GMAP( PROP_CaptionEscapeRelative,      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCREL ),
    GMAP( PROP_CaptionEscapeAbsolute,      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_ABS|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCABS ),
    GMAP( PROP_CaptionLineLength,          XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH,        XML_TYPE_MEASURE, 0 ),
    GMAP( PROP_CaptionIsFitLineLength,     XML_NAMESPACE_DRAW, XML_CAPTION_FIT_LINE_LENGTH,    XML_TYPE_BOOL, 0 ),

    // misc object properties
    GMAP( PROP_MoveProtect,                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_MOVE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_MOVE_PROTECT ),
    GMAP( PROP_SizeProtect,                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_SIZE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_SIZE_PROTECT ),
    GMAP( PROP_WritingMode,                XML_NAMESPACE_STYLE, XML_WRITING_MODE,              XML_SD_TYPE_WRITINGMODE2, CTF_WRITINGMODE2 ),
    { PROP_WritingMode, XML_NAMESPACE_LO_EXT, XML_WRITING_MODE, XML_SD_TYPE_WRITINGMODE2|XML_TYPE_PROP_GRAPHIC, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, true},
    { PROP_Decorative, XML_NAMESPACE_LO_EXT, XML_DECORATIVE, XML_TYPE_BOOL|XML_TYPE_PROP_GRAPHIC, 0, SvtSaveOptions::ODFSVER_FUTURE_EXTENDED, false },

    MAP_END()
};

// entry list for presentation page properties

const XMLPropertyMapEntry aXMLSDPresPageProps[] =
{
    DPMAP( PROP_UserDefinedAttributes,     XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    DPMAP( PROP_Change,                        XML_NAMESPACE_PRESENTATION, XML_TRANSITION_TYPE,    XML_SD_TYPE_PRESPAGE_TYPE, CTF_PAGE_TRANS_TYPE ),
    DPMAP( PROP_Effect,                        XML_NAMESPACE_PRESENTATION, XML_TRANSITION_STYLE,   XML_SD_TYPE_PRESPAGE_STYLE, CTF_PAGE_TRANS_STYLE ),
    DPMAP( PROP_Speed,                     XML_NAMESPACE_PRESENTATION, XML_TRANSITION_SPEED,   XML_SD_TYPE_PRESPAGE_SPEED, CTF_PAGE_TRANS_SPEED ),
    DPMAP( PROP_HighResDuration,           XML_NAMESPACE_PRESENTATION, XML_DURATION,           XML_SD_TYPE_PRESPAGE_DURATION, CTF_PAGE_TRANS_DURATION ),
    DPMAP( PROP_Visible,                   XML_NAMESPACE_PRESENTATION, XML_VISIBILITY,         XML_SD_TYPE_PRESPAGE_VISIBILITY, CTF_PAGE_VISIBLE ),
    DPMAP( PROP_Sound,                     XML_NAMESPACE_PRESENTATION, XML_SOUND,              XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM, CTF_PAGE_SOUND_URL ),
    DPMAP( PROP_BackgroundFullSize,            XML_NAMESPACE_DRAW,         XML_BACKGROUND_SIZE,    XML_SD_TYPE_PRESPAGE_BACKSIZE, CTF_PAGE_BACKSIZE ),

    DPMAP( PROP_IsBackgroundVisible,       XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_VISIBLE, XML_TYPE_BOOL, 0 ),
    DPMAP( PROP_IsBackgroundObjectsVisible,    XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_OBJECTS_VISIBLE, XML_TYPE_BOOL, 0 ),

    DPMAP( PROP_FillStyle,                 XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, 0 ),
    DPMAP( PROP_FillColor,                 XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    DPMAP( PROP_FillGradientName,          XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    DPMAP( PROP_FillGradientStepCount,     XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER, 0 ),
    DPMAP( PROP_FillHatchName,             XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( PROP_FillBackground,                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    DPMAP( PROP_FillBitmapName,                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,    XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    DPMAP( PROP_FillTransparence,              XML_NAMESPACE_DRAW, XML_OPACITY,           XML_TYPE_NEG_PERCENT|MID_FLAG_MULTI_PROPERTY,  0 ),
    DPMAP( PROP_FillTransparenceGradientName,  XML_NAMESPACE_DRAW, XML_OPACITY_NAME,       XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    DPMAP( PROP_FillBitmapSizeX,           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( PROP_FillBitmapLogicalSize,     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( PROP_FillBitmapSizeY,           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( PROP_FillBitmapLogicalSize,     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( PROP_FillBitmapMode,            XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE, 0 ),
    DPMAP( PROP_FillBitmapPositionOffsetX, XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    DPMAP( PROP_FillBitmapPositionOffsetY, XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    DPMAP( PROP_FillBitmapRectanglePoint,  XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    DPMAP( PROP_FillBitmapOffsetX,         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    DPMAP( PROP_FillBitmapOffsetY,         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET, XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    DPMAP( PROP_IsHeaderVisible,           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_HEADER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_HEADER_VISIBLE ),
    DPMAP( PROP_IsFooterVisible,           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_FOOTER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_FOOTER_VISIBLE ),
    DPMAP( PROP_IsPageNumberVisible,       XML_NAMESPACE_PRESENTATION, XML_DISPLAY_PAGE_NUMBER,    XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_PAGE_NUMBER_VISIBLE ),
    DPMAP( PROP_IsDateTimeVisible,         XML_NAMESPACE_PRESENTATION, XML_DISPLAY_DATE_TIME,      XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_DATE_TIME_VISIBLE ),

    DPMAP( PROP_TransitionType,            XML_NAMESPACE_SMIL, XML_TYPE,                   XML_SD_TYPE_TRANSITION_TYPE, CTF_PAGE_TRANSITION_TYPE ),
    DPMAP( PROP_TransitionSubtype,         XML_NAMESPACE_SMIL, XML_SUBTYPE,                XML_SD_TYPE_TRANSTIION_SUBTYPE, CTF_PAGE_TRANSITION_SUBTYPE ),
    DPMAP( PROP_TransitionDirection,       XML_NAMESPACE_SMIL, XML_DIRECTION,              XML_SD_TYPE_TRANSTIION_DIRECTION, CTF_PAGE_TRANSITION_DIRECTION ),
    DPMAP( PROP_TransitionFadeColor,       XML_NAMESPACE_SMIL, XML_FADECOLOR,              XML_TYPE_COLOR, CTF_PAGE_TRANSITION_FADECOLOR ),
    MAP_END()
};

/** contains the attribute to property mapping for a drawing layer table
    WARNING: if attributes are added, SdXMLTableShapeContext::processAttribute needs to be updated!
*/
const XMLPropertyMapEntry aXMLTableShapeAttributes[] =
{
    MAP_( PROP_UseFirstRowStyle,       XML_NAMESPACE_TABLE, XML_USE_FIRST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( PROP_UseLastRowStyle,        XML_NAMESPACE_TABLE, XML_USE_LAST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( PROP_UseFirstColumnStyle,    XML_NAMESPACE_TABLE, XML_USE_FIRST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( PROP_UseLastColumnStyle,     XML_NAMESPACE_TABLE, XML_USE_LAST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( PROP_UseBandingRowStyle,     XML_NAMESPACE_TABLE, XML_USE_BANDING_ROWS_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_( PROP_UseBandingColumnStyle,  XML_NAMESPACE_TABLE, XML_USE_BANDING_COLUMNS_STYLES, XML_TYPE_BOOL, 0 ),
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

SvXMLEnumMapEntry<drawing::TextureKind2> const  aXML_TexKind_EnumMap[] =
{
    { XML_LUMINANCE,    drawing::TextureKind2_LUMINANCE },
    { XML_INTENSITY,    drawing::TextureKind2_INTENSITY },
    { XML_COLOR,        drawing::TextureKind2_COLOR },
    { XML_TOKEN_INVALID, drawing::TextureKind2(0) }
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

SvXMLEnumMapEntry<sal_Int16> const aXML_WritingMode2_EnumMap[] =
{
    { XML_LR_TB,    text::WritingMode2::LR_TB },
    { XML_RL_TB,    text::WritingMode2::RL_TB },
    { XML_TB_RL,    text::WritingMode2::TB_RL },
    { XML_TB_LR,    text::WritingMode2::TB_LR },
    { XML_PAGE,     text::WritingMode2::CONTEXT },
    { XML_BT_LR,    text::WritingMode2::BT_LR },
    { XML_TB_RL90,  text::WritingMode2::TB_RL90 },
    { XML_TOKEN_INVALID, text::WritingMode2::LR_TB }
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

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > xModel, SvXMLImport& rImport )
: mxModel(std::move( xModel )), mpExport(nullptr), mpImport( &rImport )
{
}

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > xModel, SvXMLExport& rExport )
: mxModel(std::move( xModel )), mpExport( &rExport ), mpImport(nullptr)
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
            case XML_SD_TYPE_WRITINGMODE2 :
            {
                pHdl = new XMLConstantsPropertyHandler ( aXML_WritingMode2_EnumMap, XML_LR_TB );
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
                    xCompare = xCompareFac->createAnyCompareByName( u"NumberingRules"_ustr );

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
            case XML_TYPE_COMPLEX_COLOR:
                pHdl = new XMLComplexColorHandler;
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

    XMLPropertyState* pGraphicWritingMode2 = nullptr;
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
            case CTF_WRITINGMODE2:
                pGraphicWritingMode2 = property;
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

    if (pGraphicWritingMode2)
    {
        // A style:writing-mode attribute G in graphic-properties is only evaluated if there is no
        // style:writing-mode attribute P in the paragraph-properties of the same graphic style.
        // Otherwise the value of P is used. For values lr-tb, rl-tb and tb-rl the values G and P
        // should be the same. But other values in G cannot be expressed in P and would produce default
        // 0 value in P, preventing evaluation of G.
        sal_Int16 eGraphicWritingMode;
        if ((pGraphicWritingMode2->maValue >>= eGraphicWritingMode)
            && eGraphicWritingMode >= text::WritingMode2::TB_LR && pShapeWritingMode)
            pShapeWritingMode->mnIndex = -1;
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
                if( rPropSet->getPropertyValue( u"VisibleArea"_ustr ) >>= aRect )
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
        comphelper::AttributeList& rAttrList,
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
