/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>

#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <xmloff/EnumPropertyHdl.hxx>
#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <xmloff/WordWrapPropertyHdl.hxx>
#include "numithdl.hxx"
#include "XMLBitmapRepeatOffsetPropertyHandler.hxx"
#include "XMLFillBitmapSizePropertyHandler.hxx"
#include "XMLBitmapLogicalSizePropertyHandler.hxx"
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
#include "propimp0.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <xmloff/txtprmap.hxx>
#include "XMLClipPropertyHandler.hxx"
#include "XMLIsPercentagePropertyHandler.hxx"
#include "XMLPercentOrMeasurePropertyHandler.hxx"
#include "animations.hxx"
#include <sax/tools/converter.hxx>

#include "sdxmlexp_impl.hxx"

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

#define _MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_010 }
#define _MAPV(name,prefix,token,type,context,version)  { name, sizeof(name)-1, prefix, token, type, context, version }
#define GMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_GRAPHIC,context)
#define GMAPV(name,prefix,token,type,context,version) _MAPV(name,prefix,token,type|XML_TYPE_PROP_GRAPHIC,context,version)
#define DPMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_DRAWING_PAGE,context)
#define TMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_TEXT,context)
#define PMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_PARAGRAPH,context)
#define MAP_END() { 0L, 0, 0, XML_EMPTY, 0 ,0, SvtSaveOptions::ODFVER_010}

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

const XMLPropertyMapEntry aXMLSDProperties[] =
{
    // this entry must be first! this is needed for XMLShapeImportHelper::CreateExternalShapePropMapper
    //
    // ^^^though CreateExternalShapePropMapper is gone now, hmm^^^
    GMAP( "UserDefinedAttributes",          XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    // stroke attributes
    GMAP( "LineStyle",                      XML_NAMESPACE_DRAW, XML_STROKE,                 XML_SD_TYPE_STROKE, 0 ),
    GMAP( "LineDashName",                   XML_NAMESPACE_DRAW, XML_STROKE_DASH,            XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT , CTF_DASHNAME ),
    GMAP( "LineWidth",                      XML_NAMESPACE_SVG,  XML_STROKE_WIDTH,           XML_TYPE_MEASURE, 0 ),
    GMAP( "LineColor",                      XML_NAMESPACE_SVG,  XML_STROKE_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( "LineStartName",                  XML_NAMESPACE_DRAW, XML_MARKER_START,           XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINESTARTNAME ),
    GMAP( "LineStartWidth",                 XML_NAMESPACE_DRAW, XML_MARKER_START_WIDTH,     XML_TYPE_MEASURE, 0 ),
    GMAP( "LineStartCenter",                XML_NAMESPACE_DRAW, XML_MARKER_START_CENTER,    XML_TYPE_BOOL, 0 ),
    GMAP( "LineEndName",                    XML_NAMESPACE_DRAW, XML_MARKER_END,             XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_LINEENDNAME ),
    GMAP( "LineEndWidth",                   XML_NAMESPACE_DRAW, XML_MARKER_END_WIDTH,       XML_TYPE_MEASURE, 0 ),
    GMAP( "LineEndCenter",                  XML_NAMESPACE_DRAW, XML_MARKER_END_CENTER,      XML_TYPE_BOOL, 0 ),
    GMAP( "LineTransparence",               XML_NAMESPACE_SVG,  XML_STROKE_OPACITY,         XML_SD_TYPE_OPACITY, 0 ),
    GMAP( "LineJoint",                      XML_NAMESPACE_DRAW, XML_STROKE_LINEJOIN,        XML_SD_TYPE_LINEJOIN, 0 ),

    // fill attributes
    GMAP( "FillStyle",                      XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, 0 ),
    GMAP( "FillColor",                      XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    GMAP( "FillColor2",                     XML_NAMESPACE_DRAW, XML_SECONDARY_FILL_COLOR,   XML_TYPE_COLOR, 0 ),
    GMAP( "FillGradientName",               XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    GMAP( "FillGradientStepCount",          XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER16, 0 ),
    GMAP( "FillHatchName",                  XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( "FillBackground",                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    GMAP( "FillBitmapName",                 XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    GMAP( "FillTransparence",               XML_NAMESPACE_DRAW, XML_OPACITY,            XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),    // exists in SW, too
    GMAP( "FillTransparenceGradientName",   XML_NAMESPACE_DRAW, XML_OPACITY_NAME,       XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),

    GMAP( "FillBitmapSizeX",                    XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "FillBitmapLogicalSize",          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "FillBitmapSizeY",                    XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "FillBitmapLogicalSize",          XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "FillBitmapMode",                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "FillBitmapPositionOffsetX",      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    GMAP( "FillBitmapPositionOffsetY",      XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    GMAP( "FillBitmapRectanglePoint",       XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    GMAP( "FillBitmapOffsetX",              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    GMAP( "FillBitmapOffsetY",              XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // text frame attributes
    GMAP( "TextHorizontalAdjust",           XML_NAMESPACE_DRAW, XML_TEXTAREA_HORIZONTAL_ALIGN,  XML_SD_TYPE_TEXT_ALIGN, 0 ),
    GMAP( "TextVerticalAdjust",             XML_NAMESPACE_DRAW, XML_TEXTAREA_VERTICAL_ALIGN,    XML_SD_TYPE_VERTICAL_ALIGN, 0 ),
    GMAP( "TextAutoGrowHeight",             XML_NAMESPACE_DRAW, XML_AUTO_GROW_HEIGHT,       XML_TYPE_BOOL, 0 ),
    GMAP( "TextAutoGrowWidth",              XML_NAMESPACE_DRAW, XML_AUTO_GROW_WIDTH,        XML_TYPE_BOOL, 0 ),
    GMAP( "TextFitToSize",                  XML_NAMESPACE_DRAW, XML_FIT_TO_SIZE,            XML_SD_TYPE_FITTOSIZE, 0 ),
    GMAP( "TextContourFrame",               XML_NAMESPACE_DRAW, XML_FIT_TO_CONTOUR,         XML_TYPE_BOOL, 0 ),
    GMAP( "TextMaximumFrameHeight",         XML_NAMESPACE_FO,   XML_MAX_HEIGHT,             XML_TYPE_MEASURE, 0 ),
    GMAP( "TextMaximumFrameWidth",          XML_NAMESPACE_FO,   XML_MAX_WIDTH,              XML_TYPE_MEASURE, 0 ),
    GMAP( "TextMinimumFrameHeight",         XML_NAMESPACE_FO,   XML_MIN_HEIGHT,             XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( "TextMinimumFrameWidth",          XML_NAMESPACE_FO,   XML_MIN_WIDTH,              XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "TextUpperDistance",              XML_NAMESPACE_FO,   XML_PADDING_TOP,            XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( "TextLowerDistance",              XML_NAMESPACE_FO,   XML_PADDING_BOTTOM,         XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( "TextLeftDistance",               XML_NAMESPACE_FO,   XML_PADDING_LEFT,           XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    GMAP( "TextRightDistance",              XML_NAMESPACE_FO,   XML_PADDING_RIGHT,          XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too
    PMAP( "TextWritingMode",                XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_SD_TYPE_WRITINGMODE|MID_FLAG_MULTI_PROPERTY, CTF_WRITINGMODE ),
    GMAP( "NumberingRules",                 XML_NAMESPACE_TEXT, XML_LIST_STYLE,             XML_SD_TYPE_NUMBULLET|MID_FLAG_ELEMENT_ITEM, CTF_NUMBERINGRULES ),
    GMAP( "NumberingRules",                 XML_NAMESPACE_TEXT, XML_LIST_STYLE_NAME,        XML_TYPE_STRING, CTF_SD_NUMBERINGRULES_NAME ),
    GMAP( "TextWordWrap",                   XML_NAMESPACE_FO,   XML_WRAP_OPTION,            XML_TYPE_WRAP_OPTION, 0 ),

    // shadow attributes
    GMAP( "Shadow",                         XML_NAMESPACE_DRAW, XML_SHADOW,                 XML_SD_TYPE_SHADOW, 0 ),
    GMAP( "ShadowXDistance",                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X,        XML_TYPE_MEASURE, 0 ),
    GMAP( "ShadowYDistance",                    XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y,        XML_TYPE_MEASURE, 0 ),
    GMAP( "ShadowColor",                        XML_NAMESPACE_DRAW, XML_SHADOW_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( "ShadowTransparence",             XML_NAMESPACE_DRAW, XML_SHADOW_OPACITY, XML_TYPE_NEG_PERCENT, 0 ),

    // graphic attributes
    GMAP( "GraphicColorMode",               XML_NAMESPACE_DRAW, XML_COLOR_MODE,             XML_TYPE_COLOR_MODE, 0 ), // exists in SW, too, with same property name
    GMAP( "AdjustLuminance",                XML_NAMESPACE_DRAW, XML_LUMINANCE,              XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( "AdjustContrast",                 XML_NAMESPACE_DRAW, XML_CONTRAST,               XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( "Gamma",                          XML_NAMESPACE_DRAW, XML_GAMMA,                  XML_TYPE_DOUBLE_PERCENT, 0 ), // signed? exists in SW, too, with same property name
    GMAP( "AdjustRed",                      XML_NAMESPACE_DRAW, XML_RED,                    XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( "AdjustGreen",                    XML_NAMESPACE_DRAW, XML_GREEN,                  XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAP( "AdjustBlue",                     XML_NAMESPACE_DRAW, XML_BLUE,                   XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    GMAPV( "GraphicCrop",                   XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP, CTF_TEXT_CLIP, SvtSaveOptions::ODFVER_012 ), // exists in SW, too, with same property name
    GMAP( "GraphicCrop",                    XML_NAMESPACE_FO,   XML_CLIP,                   XML_TYPE_TEXT_CLIP11, CTF_TEXT_CLIP11 ), // exists in SW, too, with same property name
    GMAP( "Transparency",                   XML_NAMESPACE_DRAW, XML_IMAGE_OPACITY,          XML_TYPE_NEG_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ), // exists in SW, too, with same property name // #i25616#
    GMAP( "IsMirrored",                     XML_NAMESPACE_STYLE,    XML_MIRROR,             XML_TYPE_SD_MIRROR|MID_FLAG_MULTI_PROPERTY, 0 ),  // exists in SW, too // #i40214#

    // animation text attributes
    TMAP( "TextAnimationKind",              XML_NAMESPACE_STYLE,XML_TEXT_BLINKING,          XML_TYPE_TEXT_ANIMATION_BLINKING, CTF_TEXTANIMATION_BLINKING ),
    GMAP( "TextAnimationKind",              XML_NAMESPACE_TEXT, XML_ANIMATION,              XML_TYPE_TEXT_ANIMATION, CTF_TEXTANIMATION_KIND ),
    GMAP( "TextAnimationDirection",         XML_NAMESPACE_TEXT, XML_ANIMATION_DIRECTION,    XML_TYPE_TEXT_ANIMATION_DIRECTION, 0 ),
    GMAP( "TextAnimationStartInside",       XML_NAMESPACE_TEXT, XML_ANIMATION_START_INSIDE, XML_TYPE_BOOL, 0 ),
    GMAP( "TextAnimationStopInside",            XML_NAMESPACE_TEXT, XML_ANIMATION_STOP_INSIDE,  XML_TYPE_BOOL, 0 ),
    GMAP( "TextAnimationCount",             XML_NAMESPACE_TEXT, XML_ANIMATION_REPEAT,       XML_TYPE_NUMBER16, 0 ),
    GMAP( "TextAnimationDelay",             XML_NAMESPACE_TEXT, XML_ANIMATION_DELAY,        XML_TYPE_DURATION16_MS, 0 ),
    GMAP( "TextAnimationAmount",                XML_NAMESPACE_TEXT, XML_ANIMATION_STEPS,        XML_TYPE_TEXT_ANIMATION_STEPS, 0 ),

    // connector attributes
    GMAP( "EdgeNode1HorzDist",              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL,  XML_TYPE_MEASURE, 0 ),
    GMAP( "EdgeNode1VertDist",              XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( "EdgeNode2HorzDist",              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL,    XML_TYPE_MEASURE, 0 ),
    GMAP( "EdgeNode2VertDist",              XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL,      XML_TYPE_MEASURE, 0 ),

    // measure attributes
    GMAP( "MeasureLineDistance",                XML_NAMESPACE_DRAW, XML_LINE_DISTANCE,          XML_TYPE_MEASURE, 0 ),
    GMAP( "MeasureHelpLineOverhang",            XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG,         XML_TYPE_MEASURE, 0 ),
    GMAP( "MeasureHelpLineDistance",            XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE,         XML_TYPE_MEASURE, 0 ),
    GMAP( "MeasureHelpLine1Length",         XML_NAMESPACE_DRAW, XML_START_GUIDE,            XML_TYPE_MEASURE, 0 ),
    GMAP( "MeasureHelpLine2Length",         XML_NAMESPACE_DRAW, XML_END_GUIDE,              XML_TYPE_MEASURE, 0 ),
    GMAP( "MeasureTextHorizontalPosition",  XML_NAMESPACE_DRAW, XML_MEASURE_ALIGN,          XML_SD_TYPE_MEASURE_HALIGN, 0 ),
    GMAP( "MeasureTextVerticalPosition",    XML_NAMESPACE_DRAW, XML_MEASURE_VERTICAL_ALIGN, XML_SD_TYPE_MEASURE_VALIGN, 0 ),
    GMAP( "MeasureUnit",                        XML_NAMESPACE_DRAW, XML_UNIT,                   XML_SD_TYPE_MEASURE_UNIT, 0 ),
    GMAP( "MeasureShowUnit",                    XML_NAMESPACE_DRAW, XML_SHOW_UNIT,              XML_TYPE_BOOL, 0 ),
    GMAP( "MeasureBelowReferenceEdge",      XML_NAMESPACE_DRAW, XML_PLACING,                XML_SD_TYPE_MEASURE_PLACING, 0 ),
    GMAP( "MeasureTextRotate90",                XML_NAMESPACE_DRAW, XML_PARALLEL,               XML_TYPE_BOOL, 0 ),
    GMAP( "MeasureDecimalPlaces",           XML_NAMESPACE_DRAW, XML_DECIMAL_PLACES,         XML_TYPE_NUMBER16, 0 ),

    // 3D geometry attributes
    GMAP( "D3DHorizontalSegments",          XML_NAMESPACE_DR3D, XML_HORIZONTAL_SEGMENTS,    XML_TYPE_NUMBER, 0 ),
    GMAP( "D3DVerticalSegments",                XML_NAMESPACE_DR3D, XML_VERTICAL_SEGMENTS,      XML_TYPE_NUMBER, 0 ),
    GMAP( "D3DPercentDiagonal",             XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING,          XML_TYPE_PERCENT, 0 ),
    GMAP( "D3DBackscale",                   XML_NAMESPACE_DR3D, XML_BACK_SCALE,             XML_TYPE_PERCENT, 0 ),
    GMAP( "D3DEndAngle",                        XML_NAMESPACE_DR3D, XML_END_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( "D3DDepth",                       XML_NAMESPACE_DR3D, XML_DEPTH,                  XML_TYPE_MEASURE, 0 ),
    GMAP( "D3DDoubleSided",                 XML_NAMESPACE_DR3D, XML_BACKFACE_CULLING,       XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // #107245# New 3D properties which are possible for lathe and extrude 3d objects
    GMAP( "D3DCloseFront",                  XML_NAMESPACE_DR3D, XML_CLOSE_FRONT,            XML_TYPE_BOOL, 0 ),
    GMAP( "D3DCloseBack",                   XML_NAMESPACE_DR3D, XML_CLOSE_BACK,             XML_TYPE_BOOL, 0 ),

    // 3D lighting attributes
    GMAP( "D3DNormalsKind",                 XML_NAMESPACE_DR3D, XML_NORMALS_KIND,           XML_SD_TYPE_NORMALS_KIND, 0 ),
    GMAP( "D3DNormalsInvert",               XML_NAMESPACE_DR3D, XML_NORMALS_DIRECTION,      XML_SD_TYPE_NORMALS_DIRECTION, 0 ),

    // 3D texture attributes
    GMAP( "D3DTextureProjectionX",          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_X,  XML_SD_TYPE_TEX_GENERATION_MODE_X, 0 ),
    GMAP( "D3DTextureProjectionY",          XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_Y,  XML_SD_TYPE_TEX_GENERATION_MODE_Y, 0 ),
    GMAP( "D3DTextureKind",                 XML_NAMESPACE_DR3D, XML_TEX_KIND,               XML_SD_TYPE_TEX_KIND, 0 ),
    GMAP( "D3DTextureMode",                 XML_NAMESPACE_DR3D, XML_TEX_MODE,               XML_SD_TYPE_TEX_MODE, 0 ),
    GMAP( "D3DTextureFilter",               XML_NAMESPACE_DR3D, XML_TEX_FILTER,             XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // 3D material attributes
    GMAP( "D3DMaterialColor",               XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR,          XML_TYPE_COLOR, 0 ),
    GMAP( "D3DMaterialEmission",                XML_NAMESPACE_DR3D, XML_EMISSIVE_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( "D3DMaterialSpecular",                XML_NAMESPACE_DR3D, XML_SPECULAR_COLOR,         XML_TYPE_COLOR, 0 ),
    GMAP( "D3DMaterialSpecularIntensity",   XML_NAMESPACE_DR3D, XML_SHININESS,              XML_TYPE_PERCENT, 0 ),

    // 3D shadow attributes
    GMAP( "D3DShadow3D",                        XML_NAMESPACE_DR3D, XML_SHADOW,                 XML_SD_TYPE_SHADOW, 0 ),

    // #FontWork# attributes
    GMAP( "FontWorkStyle",                  XML_NAMESPACE_DRAW, XML_FONTWORK_STYLE,             XML_SD_TYPE_FONTWORK_STYLE, CTF_FONTWORK_STYLE  ),
    GMAP( "FontWorkAdjust",                 XML_NAMESPACE_DRAW, XML_FONTWORK_ADJUST,                XML_SD_TYPE_FONTWORK_ADJUST,CTF_FONTWORK_ADJUST ),
    GMAP( "FontWorkDistance",               XML_NAMESPACE_DRAW, XML_FONTWORK_DISTANCE,              XML_TYPE_MEASURE,           CTF_FONTWORK_DISTANCE   ),
    GMAP( "FontWorkStart",                  XML_NAMESPACE_DRAW, XML_FONTWORK_START,                 XML_TYPE_MEASURE,           CTF_FONTWORK_START  ),
    GMAP( "FontWorkMirror",                 XML_NAMESPACE_DRAW, XML_FONTWORK_MIRROR,                XML_TYPE_BOOL,              CTF_FONTWORK_MIRROR ),
    GMAP( "FontWorkOutline",                    XML_NAMESPACE_DRAW, XML_FONTWORK_OUTLINE,               XML_TYPE_BOOL,              CTF_FONTWORK_OUTLINE    ),
    GMAP( "FontWorkShadow",                 XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW,                XML_SD_TYPE_FONTWORK_SHADOW,CTF_FONTWORK_SHADOW ),
    GMAP( "FontWorkShadowColor",                XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_COLOR,          XML_TYPE_COLOR,             CTF_FONTWORK_SHADOWCOLOR    ),
    GMAP( "FontWorkShadowOffsetX",          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_X,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETX  ),
    GMAP( "FontWorkShadowOffsetY",          XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_Y,       XML_TYPE_MEASURE,           CTF_FONTWORK_SHADOWOFFSETY  ),
    GMAP( "FontWorkForm",                   XML_NAMESPACE_DRAW, XML_FONTWORK_FORM,                  XML_SD_TYPE_FONTWORK_FORM,  CTF_FONTWORK_FORM   ),
    GMAP( "FontWorkHideForm",               XML_NAMESPACE_DRAW, XML_FONTWORK_HIDE_FORM,             XML_TYPE_BOOL,              CTF_FONTWORK_HIDEFORM   ),
    GMAP( "FontWorkShadowTransparence",     XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_TRANSPARENCE,   XML_TYPE_PERCENT,           CTF_FONTWORK_SHADOWTRANSPARENCE ),

    // control attributes (border exists one mor time for the text additions of shapes)
    GMAP( "ControlSymbolColor",             XML_NAMESPACE_DRAW, XML_SYMBOL_COLOR,           XML_TYPE_COLOR, 0 ),
    GMAP( "ControlBackground",              XML_NAMESPACE_FO,   XML_BACKGROUND_COLOR,       XML_TYPE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "ControlBorder",                  XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( "ControlBorderColor",             XML_NAMESPACE_FO,   XML_BORDER,                 XML_SD_TYPE_CONTROL_BORDER_COLOR|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    GMAP( "ControlDataStyle",               XML_NAMESPACE_STYLE,XML_DATA_STYLE_NAME,        XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_EXPORT|MID_FLAG_SPECIAL_ITEM, CTF_SD_CONTROL_SHAPE_DATA_STYLE ),
    GMAP( "ControlTextEmphasis",            XML_NAMESPACE_STYLE,XML_TEXT_EMPHASIZE,         XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),
    GMAP( "ImageScaleMode",                 XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_IMAGE_SCALE_MODE|MID_FLAG_MULTI_PROPERTY, 0 ),
    GMAP( "ControlWritingMode",             XML_NAMESPACE_STYLE,XML_WRITING_MODE,           XML_TYPE_TEXT_WRITING_MODE_WITH_DEFAULT|MID_FLAG_MULTI_PROPERTY, CTF_CONTROLWRITINGMODE ),

    // special entries for floating frames
    GMAP( "FrameIsAutoScroll",          XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_SCROLLBAR,    XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_SCROLLBAR ),
    GMAP( "FrameIsBorder",              XML_NAMESPACE_DRAW, XML_FRAME_DISPLAY_BORDER,       XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,              CTF_FRAME_DISPLAY_BORDER ),
    GMAP( "FrameMarginWidth",           XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_HORIZONTAL,    XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_HORI ),
    GMAP( "FrameMarginHeight",          XML_NAMESPACE_DRAW, XML_FRAME_MARGIN_VERTICAL,      XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,        CTF_FRAME_MARGIN_VERT ),
    GMAP( "VisibleArea",                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,   CTF_SD_OLE_VIS_AREA_IMPORT_LEFT ),
    GMAP( "VisibleArea",                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,    CTF_SD_OLE_VIS_AREA_IMPORT_TOP ),
    GMAP( "VisibleArea",                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY,  CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH ),
    GMAP( "VisibleArea",                XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY, CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT ),
    GMAP( "IsInternal",                 XML_NAMESPACE_DRAW, XML__EMPTY,                     XML_TYPE_BUILDIN_CMP_ONLY,                          CTF_SD_OLE_ISINTERNAL ),
    GMAP( "IsInternal",                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_LEFT,          XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,    CTF_SD_OLE_VIS_AREA_EXPORT_LEFT ),
    GMAP( "IsInternal",                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_TOP,           XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,     CTF_SD_OLE_VIS_AREA_EXPORT_TOP ),
    GMAP( "IsInternal",                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_WIDTH,         XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,   CTF_SD_OLE_VIS_AREA_EXPORT_WIDTH ),
    GMAP( "IsInternal",                 XML_NAMESPACE_DRAW, XML_VISIBLE_AREA_HEIGHT,        XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY|MID_FLAG_NO_PROPERTY_IMPORT,  CTF_SD_OLE_VIS_AREA_EXPORT_HEIGHT ),

    GMAP( "Aspect",                     XML_NAMESPACE_DRAW, XML_DRAW_ASPECT,                XML_TYPE_TEXT_DRAW_ASPECT|MID_FLAG_MULTI_PROPERTY,  CTF_SD_OLE_ASPECT ),

    // caption properties
    GMAP( "CaptionType",                    XML_NAMESPACE_DRAW, XML_CAPTION_TYPE,               XML_SD_TYPE_CAPTION_TYPE, 0 ),
    GMAP( "CaptionIsFixedAngle",            XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE_TYPE,         XML_SD_TYPE_CAPTION_ANGLE_TYPE, 0 ),
    GMAP( "CaptionAngle",               XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE,              XML_TYPE_NUMBER, 0 ),
    GMAP( "CaptionGap",                 XML_NAMESPACE_DRAW, XML_CAPTION_GAP,                XML_TYPE_MEASURE, 0 ),
    GMAP( "CaptionEscapeDirection",     XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE_DIRECTION,   XML_SD_TYPE_CAPTION_ESC_DIR, 0 ),
    GMAP( "CaptionIsEscapeRelative",    XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_IS_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ISESCREL ),
    GMAP( "CaptionEscapeRelative",      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCREL ),
    GMAP( "CaptionEscapeAbsolute",      XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE,             XML_SD_TYPE_CAPTION_ESC_ABS|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCABS ),
    GMAP( "CaptionLineLength",          XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH,        XML_TYPE_MEASURE, 0 ),
    GMAP( "CaptionIsFitLineLength",     XML_NAMESPACE_DRAW, XML_CAPTION_FIT_LINE_LENGTH,    XML_TYPE_BOOL, 0 ),

    // misc object properties
    GMAP( "MoveProtect",                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_MOVE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_MOVE_PROTECT ),
    GMAP( "SizeProtect",                    XML_NAMESPACE_STYLE, XML_PROTECT,               XML_SD_TYPE_SIZE_PROTECT|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MERGE_ATTRIBUTE, CTF_SD_SIZE_PROTECT ),
    MAP_END()
};

//////////////////////////////////////////////////////////////////////////////
// entry list for presentation page properties

const XMLPropertyMapEntry aXMLSDPresPageProps[] =
{
    DPMAP( "UserDefinedAttributes",     XML_NAMESPACE_TEXT, XML_XMLNS,                  XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    DPMAP( "Change",                        XML_NAMESPACE_PRESENTATION, XML_TRANSITION_TYPE,    XML_SD_TYPE_PRESPAGE_TYPE, CTF_PAGE_TRANS_TYPE ),
    DPMAP( "Effect",                        XML_NAMESPACE_PRESENTATION, XML_TRANSITION_STYLE,   XML_SD_TYPE_PRESPAGE_STYLE, CTF_PAGE_TRANS_STYLE ),
    DPMAP( "Speed",                     XML_NAMESPACE_PRESENTATION, XML_TRANSITION_SPEED,   XML_SD_TYPE_PRESPAGE_SPEED, CTF_PAGE_TRANS_SPEED ),
    DPMAP( "Duration",                  XML_NAMESPACE_PRESENTATION, XML_DURATION,           XML_SD_TYPE_PRESPAGE_DURATION, CTF_PAGE_TRANS_DURATION ),
    DPMAP( "Visible",                   XML_NAMESPACE_PRESENTATION, XML_VISIBILITY,         XML_SD_TYPE_PRESPAGE_VISIBILITY, CTF_PAGE_VISIBLE ),
    DPMAP( "Sound",                     XML_NAMESPACE_PRESENTATION, XML_SOUND,              XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM, CTF_PAGE_SOUND_URL ),
    DPMAP( "BackgroundFullSize",            XML_NAMESPACE_DRAW,         XML_BACKGROUND_SIZE,    XML_SD_TYPE_PRESPAGE_BACKSIZE, CTF_PAGE_BACKSIZE ),

    DPMAP( "IsBackgroundVisible",       XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_VISIBLE, XML_TYPE_BOOL, 0 ),
    DPMAP( "IsBackgroundObjectsVisible",    XML_NAMESPACE_PRESENTATION, XML_BACKGROUND_OBJECTS_VISIBLE, XML_TYPE_BOOL, 0 ),

    DPMAP( "FillStyle",                 XML_NAMESPACE_DRAW, XML_FILL,                   XML_SD_TYPE_FILLSTYLE, 0 ),
    DPMAP( "FillColor",                 XML_NAMESPACE_DRAW, XML_FILL_COLOR,             XML_TYPE_COLOR, 0 ),
    DPMAP( "FillGradientName",          XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME,     XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLGRADIENTNAME ),
    DPMAP( "FillGradientStepCount",     XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,    XML_TYPE_NUMBER, 0 ),
    DPMAP( "FillHatchName",             XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,        XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLHATCHNAME ),
    GMAP( "FillBackground",                 XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,       XML_TYPE_BOOL, 0 ),
    DPMAP( "FillBitmapName",                XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,    XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLBITMAPNAME ),
    DPMAP( "FillTransparenceGradientName",  XML_NAMESPACE_DRAW, XML_OPACITY_NAME,       XML_TYPE_STYLENAME|MID_FLAG_NO_PROPERTY_IMPORT, CTF_FILLTRANSNAME ),
    DPMAP( "FillBitmapSizeX",           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( "FillBitmapLogicalSize",     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,       XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( "FillBitmapSizeY",           XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( "FillBitmapLogicalSize",     XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,      XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    DPMAP( "FillBitmapMode",            XML_NAMESPACE_STYLE,XML_REPEAT,                 XML_SD_TYPE_BITMAP_MODE, 0 ),
    DPMAP( "FillBitmapPositionOffsetX", XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    DPMAP( "FillBitmapPositionOffsetY", XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    DPMAP( "FillBitmapRectanglePoint",  XML_NAMESPACE_DRAW, XML_FILL_IMAGE_REF_POINT,   XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    DPMAP( "FillBitmapOffsetX",         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET,     XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    DPMAP( "FillBitmapOffsetY",         XML_NAMESPACE_DRAW, XML_TILE_REPEAT_OFFSET, XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    DPMAP( "IsHeaderVisible",           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_HEADER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_HEADER_VISIBLE ),
    DPMAP( "IsFooterVisible",           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_FOOTER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_FOOTER_VISIBLE ),
    DPMAP( "IsPageNumberVisible",       XML_NAMESPACE_PRESENTATION, XML_DISPLAY_PAGE_NUMBER,    XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_PAGE_NUMBER_VISIBLE ),
    DPMAP( "IsDateTimeVisible",         XML_NAMESPACE_PRESENTATION, XML_DISPLAY_DATE_TIME,      XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_DATE_TIME_VISIBLE ),

    DPMAP( "TransitionType",            XML_NAMESPACE_SMIL, XML_TYPE,                   XML_SD_TYPE_TRANSITION_TYPE, CTF_PAGE_TRANSITION_TYPE ),
    DPMAP( "TransitionSubtype",         XML_NAMESPACE_SMIL, XML_SUBTYPE,                XML_SD_TYPE_TRANSTIION_SUBTYPE, CTF_PAGE_TRANSITION_SUBTYPE ),
    DPMAP( "TransitionDirection",       XML_NAMESPACE_SMIL, XML_DIRECTION,              XML_SD_TYPE_TRANSTIION_DIRECTION, CTF_PAGE_TRANSITION_DIRECTION ),
    DPMAP( "TransitionFadeColor",       XML_NAMESPACE_SMIL, XML_FADECOLOR,              XML_TYPE_COLOR, CTF_PAGE_TRANSITION_FADECOLOR ),
    MAP_END()
};

const XMLPropertyMapEntry aXMLSDPresPageProps_onlyHeadersFooter[] =
{
    DPMAP( "IsHeaderVisible",               XML_NAMESPACE_PRESENTATION, XML_DISPLAY_HEADER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_HEADER_VISIBLE ),
    DPMAP( "IsFooterVisible",               XML_NAMESPACE_PRESENTATION, XML_DISPLAY_FOOTER,         XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_FOOTER_VISIBLE ),
    DPMAP( "IsPageNumberVisible",           XML_NAMESPACE_PRESENTATION, XML_DISPLAY_PAGE_NUMBER,    XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_PAGE_NUMBER_VISIBLE ),
    DPMAP( "IsDateTimeVisible",             XML_NAMESPACE_PRESENTATION, XML_DISPLAY_DATE_TIME,      XML_SD_TYPE_HEADER_FOOTER_VISIBILITY_TYPE, CTF_DATE_TIME_VISIBLE ),

    MAP_END()
};

/** contains the attribute to property mapping for a drawing layer table
    WARNING: if attributes are added, SdXMLTableShapeContext::processAttribute needs to be updated!
*/
const XMLPropertyMapEntry aXMLTableShapeAttributes[] =
{
    _MAP( "UseFirstRowStyle",       XML_NAMESPACE_TABLE, XML_USE_FIRST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    _MAP( "UseLastRowStyle",        XML_NAMESPACE_TABLE, XML_USE_LAST_ROW_STYLES, XML_TYPE_BOOL, 0 ),
    _MAP( "UseFirstColumnStyle",    XML_NAMESPACE_TABLE, XML_USE_FIRST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    _MAP( "UseLastColumnStyle",     XML_NAMESPACE_TABLE, XML_USE_LAST_COLUMN_STYLES, XML_TYPE_BOOL, 0 ),
    _MAP( "UseBandingRowStyle",     XML_NAMESPACE_TABLE, XML_USE_BANDING_ROWS_STYLES, XML_TYPE_BOOL, 0 ),
    _MAP( "UseBandingColumnStyle",  XML_NAMESPACE_TABLE, XML_USE_BANDING_COLUMNS_STYLES, XML_TYPE_BOOL, 0 ),
    MAP_END()
};

//////////////////////////////////////////////////////////////////////////////
// implementation of factory for own graphic properties

SvXMLEnumMapEntry aXML_LineStyle_EnumMap[] =
{
    { XML_NONE,     drawing::LineStyle_NONE },
    { XML_SOLID,    drawing::LineStyle_SOLID },
    { XML_DASH,     drawing::LineStyle_DASH },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_LineJoint_EnumMap[] =
{
    { XML_NONE,     drawing::LineJoint_NONE },
    { XML_MITER,    drawing::LineJoint_MITER },
    { XML_ROUND,    drawing::LineJoint_ROUND },
    { XML_BEVEL,    drawing::LineJoint_BEVEL },
    { XML_MIDDLE,   drawing::LineJoint_MIDDLE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_FillStyle_EnumMap[] =
{
    { XML_NONE,     drawing::FillStyle_NONE },
    { XML_SOLID,    drawing::FillStyle_SOLID },
    { XML_BITMAP,   drawing::FillStyle_BITMAP },
    { XML_GRADIENT, drawing::FillStyle_GRADIENT },
    { XML_HATCH,    drawing::FillStyle_HATCH },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_PresChange_EnumMap[] =
{
    { XML_MANUAL,           0 },
    { XML_AUTOMATIC,        1 },
    { XML_SEMI_AUTOMATIC,   2 },
    { XML_TOKEN_INVALID,                    0 }
};

SvXMLEnumMapEntry aXML_TransSpeed_EnumMap[] =
{
    { XML_FAST,     presentation::AnimationSpeed_FAST },
    { XML_MEDIUM,   presentation::AnimationSpeed_MEDIUM },
    { XML_SLOW,     presentation::AnimationSpeed_SLOW },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_FadeEffect_EnumMap[] =
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
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry   aXML_ConnectionKind_EnumMap[] =
{
    { XML_STANDARD,     drawing::ConnectorType_STANDARD },
    { XML_CURVE,        drawing::ConnectorType_CURVE },
    { XML_LINE,         drawing::ConnectorType_LINE },
    { XML_LINES,        drawing::ConnectorType_LINES },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry   aXML_BitmapMode_EnumMap[] =
{
    { XML_REPEAT,                   drawing::BitmapMode_REPEAT },
    { XML_STRETCH,                  drawing::BitmapMode_STRETCH },
    { XML_BACKGROUND_NO_REPEAT,     drawing::BitmapMode_NO_REPEAT },
    { XML_TOKEN_INVALID, 0 }
};

//////////////////////////////////////////////////////////////////////////////
// 3D EnumMaps

SvXMLEnumMapEntry  aXML_NormalsKind_EnumMap[] =
{
    { XML_OBJECT,       drawing::NormalsKind_SPECIFIC },
    { XML_FLAT,         drawing::NormalsKind_FLAT },
    { XML_SPHERE,       drawing::NormalsKind_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationX_EnumMap[] =
{
    { XML_OBJECT,       drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,     drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,       drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationY_EnumMap[] =
{
    { XML_OBJECT,       drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,     drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,       drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexKind_EnumMap[] =
{
    { XML_LUMINANCE,    drawing::TextureKind_LUMINANCE },
    { XML_COLOR,        drawing::TextureKind_COLOR },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexMode_EnumMap[] =
{
    { XML_REPLACE,      drawing::TextureMode_REPLACE },
    { XML_MODULATE,     drawing::TextureMode_MODULATE },
    { XML_BLEND,        drawing::TextureMode_BLEND },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_RefPoint_EnumMap[] =
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
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_CircleKind_EnumMap[] =
{
    { XML_FULL,         drawing::CircleKind_FULL },
    { XML_SECTION,      drawing::CircleKind_SECTION },
    { XML_CUT,          drawing::CircleKind_CUT },
    { XML_ARC,          drawing::CircleKind_ARC },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_WritingMode_EnumMap[] =
{
    { XML_TB_RL,        text::WritingMode_TB_RL },
    { XML_LR_TB,        text::WritingMode_LR_TB },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_TextAnimation_Enum[] =
{
    { XML_NONE,         drawing::TextAnimationKind_NONE },
    { XML_BLINKING,     drawing::TextAnimationKind_BLINK }, // will be filtered
    { XML_SCROLL,       drawing::TextAnimationKind_SCROLL },
    { XML_ALTERNATE,    drawing::TextAnimationKind_ALTERNATE },
    { XML_SLIDE,        drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_TextAnimation_Blinking_Enum[] =
{
    { XML_FALSE,        drawing::TextAnimationKind_NONE },
    { XML_TRUE,         drawing::TextAnimationKind_BLINK },
    { XML_FALSE,        drawing::TextAnimationKind_SCROLL },
    { XML_FALSE,        drawing::TextAnimationKind_ALTERNATE },
    { XML_FALSE,        drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_TextAnimationDirection_Enum[] =
{
    { XML_LEFT,         drawing::TextAnimationDirection_LEFT },
    { XML_RIGHT,        drawing::TextAnimationDirection_RIGHT },    // will be filtered
    { XML_UP,           drawing::TextAnimationDirection_UP },
    { XML_DOWN,         drawing::TextAnimationDirection_DOWN },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_TextAlign_Enum[] =
{
    { XML_LEFT,         drawing::TextHorizontalAdjust_LEFT },
    { XML_CENTER,       drawing::TextHorizontalAdjust_CENTER },
    { XML_RIGHT,        drawing::TextHorizontalAdjust_RIGHT },
    { XML_JUSTIFY,      drawing::TextHorizontalAdjust_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,          drawing::TextVerticalAdjust_TOP },
    { XML_MIDDLE,       drawing::TextVerticalAdjust_CENTER },
    { XML_BOTTOM,       drawing::TextVerticalAdjust_BOTTOM },
    { XML_JUSTIFY,      drawing::TextVerticalAdjust_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_FitToSize_Enum_Odf12[] =
{
    { XML_FALSE,        drawing::TextFitToSizeType_NONE },
    { XML_TRUE,         drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_TRUE,         drawing::TextFitToSizeType_ALLLINES },
    { XML_TRUE,         drawing::TextFitToSizeType_AUTOFIT },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_FitToSize_Enum[] =
{
    { XML_FALSE,        drawing::TextFitToSizeType_NONE },
    { XML_TRUE,         drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_ALL,          drawing::TextFitToSizeType_ALLLINES },
    { XML_SHRINK_TO_FIT,drawing::TextFitToSizeType_AUTOFIT },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry const pXML_MeasureUnit_Enum[] =
{
    { XML_AUTOMATIC,    0 },
    { XML_UNIT_MM,      1 },
    { XML_UNIT_CM,      2 },
    { XML_UNIT_M,       3 },
    { XML_UNIT_KM,      4 },
    { XML_UNIT_PT,      6 },
    { XML_UNIT_PC,      7 },
    { XML_UNIT_INCH,    8 },
    { XML_UNIT_FOOT,    9 },
    { XML_UNIT_MILES,   10 },
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Measure_HAlign_Enum[] =
{
    { XML_AUTOMATIC,        drawing::MeasureTextHorzPos_AUTO },
    { XML_LEFT_OUTSIDE,     drawing::MeasureTextHorzPos_LEFTOUTSIDE },
    { XML_INSIDE,           drawing::MeasureTextHorzPos_INSIDE },
    { XML_RIGHT_OUTSIDE,    drawing::MeasureTextHorzPos_RIGHTOUTSIDE},
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Measure_VAlign_Enum[] =
{
    { XML_AUTOMATIC,    drawing::MeasureTextVertPos_AUTO },
    { XML_ABOVE,        drawing::MeasureTextVertPos_EAST },
    { XML_BELOW,        drawing::MeasureTextVertPos_WEST },
    { XML_CENTER,       drawing::MeasureTextVertPos_CENTERED },
    { XML_TOKEN_INVALID,0 }
};

// #FontWork#
SvXMLEnumMapEntry const pXML_Fontwork_Style_Enum[] =
{
    { XML_ROTATE,       0 }, //XFT_ROTATE,
    { XML_UPRIGHT,      1 }, //XFT_UPRIGHT,
    { XML_SLANT_X,      2 }, //XFT_SLANTX,
    { XML_SLANT_Y,      3 }, //XFT_SLANTY,
    { XML_NONE,         4 }, //XFT_NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Fontwork_Adjust_Enum[] =
{
    { XML_LEFT,         0 }, //XFT_LEFT,
    { XML_RIGHT,        1 }, //XFT_RIGHT,
    { XML_AUTOSIZE,     2 }, //XFT_AUTOSIZE,
    { XML_CENTER,       3 }, //XFT_CENTER
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Fontwork_Shadow_Enum[] =
{
    { XML_NORMAL,       0 }, //XFTSHADOW_NORMAL,
    { XML_SLANT,        1 }, //XFTSHADOW_SLANT,
    { XML_NONE,         2 }, //XFTSHADOW_NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Fontwork_Form_Enum[] =
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

SvXMLEnumMapEntry const pXML_Caption_Esc_Dir_Enum[] =
{
    { XML_HORIZONTAL,       0 }, //SDRCAPT_ESCHORIZONTAL,
    { XML_VERTICAL,         1 }, //SDRCAPT_ESCVERTICAL,
    { XML_AUTO,             2 }, //SDRCAPT_ESCBESTFIT,
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry const pXML_Caption_Type_Enum[] =
{
    { XML_STRAIGHT_LINE,            0 }, //SDRCAPT_TYPE1,
    { XML_ANGLED_LINE,              1 }, //SDRCAPT_TYPE2,
    { XML_ANGLED_CONNECTOR_LINE,    2 }, //SDRCAPT_TYPE3,
    { XML_TOKEN_INVALID,0 }
};

//////////////////////////////////////////////////////////////////////////////

class XMLCaptionEscapeRelative : public XMLPropertyHandler
{
public:
    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
};

sal_Bool XMLCaptionEscapeRelative::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue;

    if (!::sax::Converter::convertPercent( nValue, rStrImpValue ))
        return sal_False;

    nValue *= 100;
    rValue <<= nValue;
    return sal_True;
}

sal_Bool XMLCaptionEscapeRelative::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nValue = 0;
    if( !(rValue >>= nValue ) )
        return sal_False;

    nValue /= 100;
    OUStringBuffer aOut;
    ::sax::Converter::convertPercent( aOut, nValue );
    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}

//////////////////////////////////////////////////////////////////////////////

class XMLMoveSizeProtectHdl : public XMLPropertyHandler
{
public:
    XMLMoveSizeProtectHdl( sal_Int32 nType ) : mnType( nType ) {}

    virtual sal_Bool importXML(
            const ::rtl::OUString& rStrImpValue,
            ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML(
            ::rtl::OUString& rStrExpValue,
            const ::com::sun::star::uno::Any& rValue,
            const SvXMLUnitConverter& rUnitConverter ) const;
private:
    const sal_Int32 mnType;
};

sal_Bool XMLMoveSizeProtectHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    const sal_Bool bValue = rStrImpValue.indexOf( GetXMLToken( mnType == XML_SD_TYPE_MOVE_PROTECT ? XML_POSITION : XML_SIZE ) ) != -1;
    rValue <<= bValue;
    return sal_True;
}

sal_Bool XMLMoveSizeProtectHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bValue = sal_Bool();
    if( !(rValue >>= bValue ) )
        return sal_False;

    if( bValue )
    {
        if( !rStrExpValue.isEmpty() )
            rStrExpValue += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );

        rStrExpValue += GetXMLToken( mnType == XML_SD_TYPE_MOVE_PROTECT ? XML_POSITION : XML_SIZE );
    }

    return sal_True;
}


//////////////////////////////////////////////////////////////////////////////

class XMLSdHeaderFooterVisibilityTypeHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLSdHeaderFooterVisibilityTypeHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

XMLSdHeaderFooterVisibilityTypeHdl::~XMLSdHeaderFooterVisibilityTypeHdl()
{
}

sal_Bool XMLSdHeaderFooterVisibilityTypeHdl::importXML(
        const ::rtl::OUString& rStrImpValue,
        ::com::sun::star::uno::Any& rValue,
        const SvXMLUnitConverter& ) const
{
    // #i38644#
    // attributes with this type where saved with VISIBLE|HIDDEN prior
    // to src680m67. So we have to import that correctly
    const sal_Bool bBool = IsXMLToken(rStrImpValue, XML_TRUE) || IsXMLToken(rStrImpValue, XML_VISIBLE);
    rValue <<= bBool;
    return bBool || IsXMLToken(rStrImpValue, XML_FALSE) || IsXMLToken(rStrImpValue, XML_HIDDEN);
}

sal_Bool XMLSdHeaderFooterVisibilityTypeHdl::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        ::sax::Converter::convertBool( aOut, bValue );
        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}


//////////////////////////////////////////////////////////////////////////////

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > xModel, SvXMLImport& rImport )
: mxModel( xModel ), mpExport(0), mpImport( &rImport )
{
}

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > xModel, SvXMLExport& rExport )
: mxModel( xModel ), mpExport( &rExport ), mpImport(0)
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
                pHdl = new XMLEnumPropertyHdl( aXML_LineStyle_EnumMap, ::getCppuType((const drawing::LineStyle*)0) );
                break;
            }
            case XML_SD_TYPE_LINEJOIN :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_LineJoint_EnumMap, ::getCppuType((const drawing::LineJoint*)0) );
                break;
            }
            case XML_SD_TYPE_FILLSTYLE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_FillStyle_EnumMap, ::getCppuType((const drawing::FillStyle*)0) );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_TYPE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_PresChange_EnumMap, ::getCppuType((const sal_Int32*)0) );
                break;
            }
            case XML_SD_TYPE_SHADOW :
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
                pHdl = new XMLEnumPropertyHdl( aXML_FadeEffect_EnumMap, ::getCppuType((const presentation::FadeEffect*)0) );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_SPEED :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TransSpeed_EnumMap, ::getCppuType((const presentation::AnimationSpeed*)0) );
                break;
            }
            case XML_SD_TYPE_PRESPAGE_DURATION :
            {
                pHdl = new XMLDurationPropertyHdl();
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
                pHdl = new XMLEnumPropertyHdl( aXML_WritingMode_EnumMap, ::getCppuType((const text::WritingMode*)0) );
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

            //////////////////////////////////////////////////////////////////
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
                pHdl = new XMLEnumPropertyHdl( aXML_NormalsKind_EnumMap, ::getCppuType((const drawing::NormalsKind*)0) );
                break;
            }
            case XML_SD_TYPE_NORMALS_DIRECTION:
            {
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_NORMAL), GetXMLToken(XML_INVERSE) );
                break;
            }
            case XML_SD_TYPE_TEX_GENERATION_MODE_X:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexGenerationX_EnumMap, ::getCppuType((const drawing::TextureProjectionMode*)0) );
                break;
            }
            case XML_SD_TYPE_TEX_GENERATION_MODE_Y:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexGenerationY_EnumMap, ::getCppuType((const drawing::TextureProjectionMode*)0) );
                break;
            }
            case XML_SD_TYPE_TEX_KIND:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexKind_EnumMap, ::getCppuType((const drawing::TextureKind*)0) );
                break;
            }
            case XML_SD_TYPE_TEX_MODE:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_TexMode_EnumMap, ::getCppuType((const drawing::TextureMode*)0) );
                break;
            }
            case XML_SD_TYPE_NUMBULLET:
            {
                uno::Reference<ucb::XAnyCompareFactory> xCompareFac( mxModel, uno::UNO_QUERY );
                uno::Reference<ucb::XAnyCompare> xCompare;
                if( xCompareFac.is() )
                    xCompare = xCompareFac->createAnyCompareByName( OUString( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) ) );

                pHdl = new XMLNumRulePropHdl( xCompare );
                break;
            }
            case XML_SD_TYPE_BITMAP_MODE:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_BitmapMode_EnumMap, getCppuType((const drawing::BitmapMode*)0) );
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
                pHdl = new XMLEnumPropertyHdl( aXML_RefPoint_EnumMap, getCppuType((const ::com::sun::star::drawing::RectanglePoint*)0) );
                break;
            }
            case XML_TYPE_TEXT_ANIMATION:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Enum, ::getCppuType((const com::sun::star::drawing::TextAnimationKind*)0) );
                break;
            case XML_TYPE_TEXT_ANIMATION_BLINKING:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Blinking_Enum, ::getCppuType((const com::sun::star::drawing::TextAnimationKind*)0) );
                break;
            case XML_TYPE_TEXT_ANIMATION_DIRECTION:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimationDirection_Enum, ::getCppuType((const com::sun::star::drawing::TextAnimationDirection*)0) );
                break;
            case XML_TYPE_TEXT_ANIMATION_STEPS:
                pHdl = new XMLTextAnimationStepPropertyHdl;
                break;
            case XML_SD_TYPE_TEXT_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAlign_Enum, ::getCppuType((const com::sun::star::drawing::TextHorizontalAdjust*)0) );
                break;
            case XML_SD_TYPE_VERTICAL_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_VerticalAlign_Enum, ::getCppuType((const com::sun::star::drawing::TextVerticalAdjust*)0) );
                break;
            case XML_SD_TYPE_FITTOSIZE:
                {
                    if (mpExport && (mpExport->getDefaultVersion()
                                        <= SvtSaveOptions::ODFVER_012))
                    {
                        pHdl = new XMLEnumPropertyHdl(pXML_FitToSize_Enum_Odf12,
                            ::getCppuType(static_cast<const com::sun::star::drawing::TextFitToSizeType*>(0)));
                    }
                    else
                    {
                        pHdl = new XMLEnumPropertyHdl(pXML_FitToSize_Enum,
                            ::getCppuType(static_cast<const com::sun::star::drawing::TextFitToSizeType*>(0)));
                    }
                }
                break;
            case XML_SD_TYPE_MEASURE_UNIT:
                pHdl = new XMLEnumPropertyHdl( pXML_MeasureUnit_Enum, ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SD_TYPE_MEASURE_HALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_HAlign_Enum, ::getCppuType((const com::sun::star::drawing::MeasureTextHorzPos*)0) );
                break;
            case XML_SD_TYPE_MEASURE_VALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_VAlign_Enum, ::getCppuType((const com::sun::star::drawing::MeasureTextVertPos*)0) );
                break;
            case XML_SD_TYPE_MEASURE_PLACING:
                {
                    pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_BELOW), GetXMLToken(XML_ABOVE) );
                }
                break;
            case XML_TYPE_TEXT_CLIP11:
                pHdl = new XMLClipPropertyHandler( sal_True );
                break;
            case XML_TYPE_TEXT_CLIP:
                pHdl = new XMLClipPropertyHandler( sal_False );
                break;

            // #FontWork#
            case XML_SD_TYPE_FONTWORK_STYLE     :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Style_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_ADJUST        :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Adjust_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_SHADOW        :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Shadow_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_FORM      :
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Form_Enum , ::getCppuType((const sal_Int32*)0));
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
                pHdl = new XMLIsPercentagePropertyHandler();
                break;
            case XML_SD_TYPE_CAPTION_ESC_REL:
                pHdl = new XMLCaptionEscapeRelative();
                break;
            case XML_SD_TYPE_CAPTION_ESC_ABS:
                pHdl = new XMLPercentOrMeasurePropertyHandler( sal_False );
                break;
            case XML_SD_TYPE_CAPTION_ESC_DIR:
                pHdl = new XMLEnumPropertyHdl( pXML_Caption_Esc_Dir_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_CAPTION_TYPE:
                pHdl = new XMLEnumPropertyHdl( pXML_Caption_Type_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_DATETIMEUPDATE:
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_FIXED), GetXMLToken(XML_VARIABLE) );
                break;
            case XML_SD_TYPE_DATETIME_FORMAT:
                pHdl = new XMLDateTimeFormatHdl( mpExport );
                break;
            case XML_SD_TYPE_TRANSITION_TYPE:
                pHdl = new XMLEnumPropertyHdl( xmloff::getAnimationsEnumMap(xmloff::Animations_EnumMap_TransitionType) , ::getCppuType((const sal_Int16*)0));
                break;
            case XML_SD_TYPE_TRANSTIION_SUBTYPE:
                pHdl = new XMLEnumPropertyHdl( xmloff::getAnimationsEnumMap(xmloff::Animations_EnumMap_TransitionSubType) , ::getCppuType((const sal_Int16*)0));
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
                pHdl = new XMLSdHeaderFooterVisibilityTypeHdl();
                break;
        }

        if(pHdl)
            PutHdlCache(nType, pHdl);
    }

    return pHdl;
}

//////////////////////////////////////////////////////////////////////////////

XMLShapePropertySetMapper::XMLShapePropertySetMapper(const UniReference< XMLPropertyHandlerFactory >& rFactoryRef)
: XMLPropertySetMapper( aXMLSDProperties, rFactoryRef )
{
}

XMLShapePropertySetMapper::~XMLShapePropertySetMapper()
{
}

// ----------------------------------------

XMLShapeExportPropertyMapper::XMLShapeExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport )
: SvXMLExportPropertyMapper( rMapper )
, maNumRuleExp( rExport )
, mbIsInAutoStyles( sal_True )
, msCDATA( GetXMLToken(XML_CDATA))
, msTrue( GetXMLToken(XML_TRUE))
, msFalse( GetXMLToken(XML_FALSE))
{
}

XMLShapeExportPropertyMapper::~XMLShapeExportPropertyMapper()
{
}

void XMLShapeExportPropertyMapper::ContextFilter(
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    XMLPropertyState* pRepeatOffsetX = NULL;
    XMLPropertyState* pRepeatOffsetY = NULL;
    XMLPropertyState* pTextAnimationBlinking = NULL;
    XMLPropertyState* pTextAnimationKind = NULL;

    // #FontWork#
    XMLPropertyState* pFontWorkStyle = NULL;
    XMLPropertyState* pFontWorkAdjust = NULL;
    XMLPropertyState* pFontWorkDistance = NULL;
    XMLPropertyState* pFontWorkStart = NULL;
    XMLPropertyState* pFontWorkMirror = NULL;
    XMLPropertyState* pFontWorkOutline = NULL;
    XMLPropertyState* pFontWorkShadow = NULL;
    XMLPropertyState* pFontWorkShadowColor = NULL;
    XMLPropertyState* pFontWorkShadowOffsetx = NULL;
    XMLPropertyState* pFontWorkShadowOffsety = NULL;
    XMLPropertyState* pFontWorkForm = NULL;
    XMLPropertyState* pFontWorkHideform = NULL;
    XMLPropertyState* pFontWorkShadowTransparence = NULL;

    // OLE
    XMLPropertyState* pOLEVisAreaLeft = NULL;
    XMLPropertyState* pOLEVisAreaTop = NULL;
    XMLPropertyState* pOLEVisAreaWidth = NULL;
    XMLPropertyState* pOLEVisAreaHeight = NULL;
    XMLPropertyState* pOLEIsInternal = NULL;

    // caption
    XMLPropertyState* pCaptionIsEscRel = NULL;
    XMLPropertyState* pCaptionEscRel = NULL;
    XMLPropertyState* pCaptionEscAbs = NULL;

    // filter fo:clip
    XMLPropertyState* pClip11State = NULL;
    XMLPropertyState* pClipState = NULL;

    XMLPropertyState* pShapeWritingMode = NULL;
    XMLPropertyState* pTextWritingMode = NULL;
    XMLPropertyState* pControlWritingMode = NULL;

    // filter properties
    for( std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
            XMLPropertyState *property = &(*aIter);
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
                    // because its an XIndexAccess and not a string.
                    // This will be handled in SvXMLAutoStylePoolP::exportStyleAttributes
                    // This is suboptimal
                    if( !mbIsInAutoStyles )
                        property->mnIndex = -1;
                }
                break;
            case CTF_WRITINGMODE:
                {
                    pShapeWritingMode = property;
                    text::WritingMode eWritingMode;
                    if( property->maValue >>= eWritingMode )
                    {
                        if( text::WritingMode_LR_TB == eWritingMode )
                        {
                            property->mnIndex = -1;
                            pShapeWritingMode = 0;
                        }
                    }
                }
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
                    sal_Bool bProtected = sal_Bool();
                    if( (property->maValue >>= bProtected) && !bProtected )
                        property->mnIndex = -1;
                }
                break;
            case CTF_SD_SIZE_PROTECT:
                {
                    sal_Bool bProtected = sal_Bool();
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
    }
    else if( pTextWritingMode && pControlWritingMode )
    {
        pControlWritingMode->mnIndex = -1;
    }

    // do not export visual area for internal ole objects
    if( pOLEIsInternal )
    {
        sal_Bool bInternal = sal_Bool();
        if( (pOLEIsInternal->maValue >>= bInternal) && !bInternal )
        {
            try
            {
                const OUString sVisibleArea( RTL_CONSTASCII_USTRINGPARAM("VisibleArea") );
                awt::Rectangle aRect;
                if( rPropSet->getPropertyValue( sVisibleArea ) >>= aRect )
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
            if(/*XFT_NONE*/4 == nStyle)
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
        sal_Bool bIsRel = false;
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

    if( pClipState != NULL && pClip11State != NULL  )
        pClip11State->mnIndex = -1;

    SvXMLExportPropertyMapper::ContextFilter(rProperties, rPropSet);
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
    sal_uInt16 nFlags,
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
                        const_cast<XMLShapeExportPropertyMapper*>(this)->maNumRuleExp.exportNumberingRule( maStyleName, xNumRule );
                }
            }
            break;
        default:
            SvXMLExportPropertyMapper::handleElementItem( rExport, rProperty, nFlags, pProperties, nIdx );
            break;
    }
}

// ----------------------------------------

XMLPageExportPropertyMapper::XMLPageExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport ) :
        SvXMLExportPropertyMapper( rMapper ),
        mrExport( rExport ),
        msCDATA( GetXMLToken(XML_CDATA))
{
}

XMLPageExportPropertyMapper::~XMLPageExportPropertyMapper()
{
}

void XMLPageExportPropertyMapper::ContextFilter(
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    XMLPropertyState* pRepeatOffsetX = NULL;
    XMLPropertyState* pRepeatOffsetY = NULL;
    XMLPropertyState* pTransType = NULL;
    XMLPropertyState* pTransDuration = NULL;
    XMLPropertyState* pDateTimeUpdate = NULL;
    XMLPropertyState* pDateTimeFormat = NULL;
    XMLPropertyState* pTransitionFadeColor = NULL;

    sal_Int16 nTransitionType = 0;

    // filter properties
    for( std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState *property = &(*aIter);
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
                if( mrExport.getExportFlags() & EXPORT_OASIS )
                    (*property).mnIndex = -1;
                break;
            case CTF_PAGE_TRANSITION_TYPE:
                {
                    if( ((mrExport.getExportFlags() & EXPORT_OASIS) == 0) ||
                        (((*property).maValue >>= nTransitionType) && (nTransitionType == 0)) )
                            (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANSITION_SUBTYPE:
                {
                    sal_Int16 nTransitionSubtype = sal_Int16();
                    if( ((mrExport.getExportFlags() & EXPORT_OASIS) == 0) ||
                        (((*property).maValue >>= nTransitionSubtype) && (nTransitionSubtype == 0)) )
                            (*property).mnIndex = -1;

                }
                break;
            case CTF_PAGE_TRANSITION_DIRECTION:
                {
                    sal_Bool bDirection = sal_Bool();
                    if( ((mrExport.getExportFlags() & EXPORT_OASIS) == 0) ||
                        (((*property).maValue >>= bDirection) && bDirection) )
                            (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANSITION_FADECOLOR:
                if( ((mrExport.getExportFlags() & EXPORT_OASIS) == 0) )
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
                    sal_Bool bVisible = false;
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

    if( pTransitionFadeColor && nTransitionType != ::com::sun::star::animations::TransitionType::FADE )
        pTransitionFadeColor->mnIndex = -1;

    if( pDateTimeFormat && pDateTimeUpdate )
    {
        sal_Bool bIsFixed = false;
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

    SvXMLExportPropertyMapper::ContextFilter(rProperties, rPropSet);
}

void XMLPageExportPropertyMapper::handleElementItem(
    SvXMLExport& rExport,
    const XMLPropertyState& rProperty,
    sal_uInt16 nFlags,
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
                    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, sal_True, sal_True );
                }
            }
            break;
        default:
            SvXMLExportPropertyMapper::handleElementItem( rExport, rProperty, nFlags, pProperties, nIdx );
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
