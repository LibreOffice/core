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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_ 
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_ 
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_ 
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_RECTANGLEPOINT_HPP_ 
#include <com/sun/star/drawing/RectanglePoint.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_ 
#include <com/sun/star/drawing/CircleKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_BitmapMode_HPP_ 
#include <com/sun/star/drawing/BitmapMode.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_ 
#include <com/sun/star/text/WritingMode.hpp>
#endif

#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include <EnumPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include <NamedBoolPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_NUMRULE_HXX
#include "numithdl.hxx"
#endif

#ifndef _XMLOFF_XMLBITMAPREPEATOFFSETPROPERTYHANDLER_HXX
#include "XMLBitmapRepeatOffsetPropertyHandler.hxx"
#endif

#ifndef _XMLOFF_XMLFILLBITMAPSIZEPROPERTYHANDLER_HXX
#include "XMLFillBitmapSizePropertyHandler.hxx"
#endif

#ifndef _XMLOFF_XMLBITMAPLOGICALSIZEPROPERTYHANDLER_HXX
#include "XMLBitmapLogicalSizePropertyHandler.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONKIND_HPP_
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONDIRECTION_HPP_
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_ 
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_ 
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTFITTOSIZETYPE_HPP_ 
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTHORZPOS_HPP_ 
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTVERTPOS_HPP_ 
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#endif

#ifndef _XMLOFF_CONTROLBORDERHDL_HXX_
#include "ControlBorderHandler.hxx"
#endif


#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _PROPIMP0_HXX
#include "propimp0.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX 
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmlnmspe.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREPROJECTIONMODE_HPP_
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREKIND_HPP_
#include <com/sun/star/drawing/TextureKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif

#ifndef _XMLOFF_TEXTPRMAP_HXX_
#include "txtprmap.hxx"
#endif

#ifndef _XMLOFF_XMLCLIPPROPERTYHANDLER_HXX
#include "XMLClipPropertyHandler.hxx"
#endif

#ifndef _XMLOFF_XMLISPERCENTAGEPROPERTYHANDLER_HXX
#include "XMLIsPercentagePropertyHandler.hxx"
#endif

#ifndef _XMLOFF_XMLPERCENTORMEASUREPROPERTYHANDLER_HXX
#include "XMLPercentOrMeasurePropertyHandler.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

#define MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context }

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

const XMLPropertyMapEntry aXMLSDProperties[] = 
{
    // this entry must be first! this is needed for XMLShapeImportHelper::CreateExternalShapePropMapper
    MAP( "UserDefinedAttributes",			XML_NAMESPACE_TEXT, XML_XMLNS,					XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    // stroke attributes
    MAP( "LineStyle",						XML_NAMESPACE_DRAW, XML_STROKE, 				XML_SD_TYPE_STROKE, 0 ),
    MAP( "LineDashName",					XML_NAMESPACE_DRAW,	XML_STROKE_DASH,			XML_TYPE_STRING, CTF_DASHNAME ),
    MAP( "LineWidth",						XML_NAMESPACE_SVG,	XML_STROKE_WIDTH,			XML_TYPE_MEASURE, 0 ),
    MAP( "LineColor",						XML_NAMESPACE_SVG,	XML_STROKE_COLOR,			XML_TYPE_COLOR, 0 ),
    MAP( "LineStartName",					XML_NAMESPACE_DRAW,	XML_MARKER_START,			XML_TYPE_STRING, CTF_LINESTARTNAME ),
    MAP( "LineStartWidth",					XML_NAMESPACE_DRAW,	XML_MARKER_START_WIDTH, 	XML_TYPE_MEASURE, 0 ),
    MAP( "LineStartCenter",				    XML_NAMESPACE_DRAW, XML_MARKER_START_CENTER,	XML_TYPE_BOOL, 0 ),
    MAP( "LineEndName",					    XML_NAMESPACE_DRAW,	XML_MARKER_END, 			XML_TYPE_STRING, CTF_LINEENDNAME ),
    MAP( "LineEndWidth",					XML_NAMESPACE_DRAW,	XML_MARKER_END_WIDTH,		XML_TYPE_MEASURE, 0 ),
    MAP( "LineEndCenter",					XML_NAMESPACE_DRAW, XML_MARKER_END_CENTER,		XML_TYPE_BOOL, 0 ),
    MAP( "LineTransparence",				XML_NAMESPACE_SVG,	XML_STROKE_OPACITY, 		XML_SD_TYPE_OPACITY, 0 ),
    MAP( "LineJoint",						XML_NAMESPACE_SVG,	XML_STROKE_LINEJOIN,		XML_SD_TYPE_LINEJOIN, 0 ),

    // fill attributes
    MAP( "FillStyle",						XML_NAMESPACE_DRAW,	XML_FILL,					XML_SD_TYPE_FILLSTYLE, 0 ),
    MAP( "FillColor",						XML_NAMESPACE_DRAW, XML_FILL_COLOR, 			XML_TYPE_COLOR, 0 ),
    MAP( "FillGradientName",				XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, 	XML_TYPE_STRING, CTF_FILLGRADIENTNAME ),
    MAP( "FillGradientStepCount",			XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,	XML_TYPE_NUMBER, 0 ),
    MAP( "FillHatchName",					XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,		XML_TYPE_STRING, CTF_FILLHATCHNAME ),
    MAP( "FillBackground",					XML_NAMESPACE_DRAW, XML_FILL_HATCH_SOLID,		XML_TYPE_BOOL, 0 ),
    MAP( "FillBitmapName",					XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,		XML_TYPE_STRING, CTF_FILLBITMAPNAME ),
    MAP( "FillTransparence",				XML_NAMESPACE_DRAW, XML_TRANSPARENCY,			XML_TYPE_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "FillTransparenceGradientName",	XML_NAMESPACE_DRAW, XML_TRANSPARENCY_NAME,		XML_TYPE_STRING, CTF_FILLTRANSNAME ),

    MAP( "FillBitmapSizeX",			    	XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_WIDTH,		XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapLogicalSize",			XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,		XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapSizeY",			    	XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_HEIGHT,		XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapLogicalSize",			XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,		XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapMode",					XML_NAMESPACE_STYLE,XML_REPEAT, 				XML_SD_TYPE_BITMAP_MODE, 0 ),
    MAP( "FillBitmapPositionOffsetX",		XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    MAP( "FillBitmapPositionOffsetY",		XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    MAP( "FillBitmapRectanglePoint",		XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT,	XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    MAP( "FillBitmapOffsetX",				XML_NAMESPACE_DRAW,	XML_TILE_REPEAT_OFFSET, 	XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    MAP( "FillBitmapOffsetY",				XML_NAMESPACE_DRAW,	XML_TILE_REPEAT_OFFSET, 	XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    // text frame attributes
    MAP( "TextHorizontalAdjust",			XML_NAMESPACE_DRAW,	XML_TEXTAREA_HORIZONTAL_ALIGN,	XML_SD_TYPE_TEXT_ALIGN, 0 ),
    MAP( "TextVerticalAdjust",				XML_NAMESPACE_DRAW,	XML_TEXTAREA_VERTICAL_ALIGN,	XML_SD_TYPE_VERTICAL_ALIGN, 0 ),
    MAP( "TextAutoGrowHeight",				XML_NAMESPACE_DRAW,	XML_AUTO_GROW_WIDTH,		XML_TYPE_BOOL, 0 ),
    MAP( "TextAutoGrowWidth",				XML_NAMESPACE_DRAW,	XML_AUTO_GROW_HEIGHT,		XML_TYPE_BOOL, 0 ),
    MAP( "TextFitToSize",					XML_NAMESPACE_DRAW,	XML_FIT_TO_SIZE,			XML_SD_TYPE_FITTOSIZE, 0 ),
    MAP( "TextContourFrame",				XML_NAMESPACE_DRAW,	XML_FIT_TO_CONTOUR, 		XML_TYPE_BOOL, 0 ),
    MAP( "TextMaximumFrameHeight",			XML_NAMESPACE_FO,	XML_MAX_HEIGHT, 			XML_TYPE_MEASURE, 0 ),
    MAP( "TextMaximumFrameWidth",			XML_NAMESPACE_FO,	XML_MAX_WIDTH,				XML_TYPE_MEASURE, 0 ),
    MAP( "TextMinimumFrameHeight",			XML_NAMESPACE_FO,	XML_MIN_HEIGHT, 			XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "TextMinimumFrameWidth",			XML_NAMESPACE_FO,	XML_MIN_WIDTH,				XML_TYPE_MEASURE, 0 ),
    MAP( "TextUpperDistance",				XML_NAMESPACE_FO,	XML_PADDING_TOP,			XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "TextLowerDistance",				XML_NAMESPACE_FO,	XML_PADDING_BOTTOM, 		XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "TextLeftDistance",				XML_NAMESPACE_FO,	XML_PADDING_LEFT,			XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "TextRightDistance",				XML_NAMESPACE_FO,	XML_PADDING_RIGHT,			XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, 0 ),	// exists in SW, too
    MAP( "TextWritingMode",			    	XML_NAMESPACE_DRAW,	XML_WRITING_MODE,			XML_SD_TYPE_WRITINGMODE, CTF_WRITINGMODE ),
    MAP( "NumberingRules",					XML_NAMESPACE_TEXT,	XML_LIST_STYLE, 			XML_SD_TYPE_NUMBULLET|MID_FLAG_ELEMENT_ITEM, CTF_NUMBERINGRULES ),
    MAP( "NumberingRules",					XML_NAMESPACE_TEXT,	XML_LIST_STYLE_NAME,		XML_TYPE_STRING, CTF_SD_NUMBERINGRULES_NAME ),
    MAP( "FontIndependentLineSpacing",		XML_NAMESPACE_DRAW,	XML_FONT_INDEPENDENT_LINE_SPACING, XML_TYPE_BOOL, 0 ),

    // shadow attributes
    MAP( "Shadow",							XML_NAMESPACE_DRAW,	XML_SHADOW, 				XML_SD_TYPE_SHADOW, 0 ),
    MAP( "ShadowXDistance",			    	XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_X,		XML_TYPE_MEASURE, 0 ),
    MAP( "ShadowYDistance",			    	XML_NAMESPACE_DRAW, XML_SHADOW_OFFSET_Y,		XML_TYPE_MEASURE, 0 ),
    MAP( "ShadowColor",				    	XML_NAMESPACE_DRAW, XML_SHADOW_COLOR,			XML_TYPE_COLOR, 0 ),
    MAP( "ShadowTransparence",				XML_NAMESPACE_DRAW, XML_SHADOW_TRANSPARENCY,	XML_TYPE_PERCENT, 0 ),

    // graphic attributes
    MAP( "GraphicColorMode",				XML_NAMESPACE_DRAW, XML_COLOR_MODE, 			XML_TYPE_COLOR_MODE, 0 ), // exists in SW, too, with same property name
    MAP( "AdjustLuminance",			    	XML_NAMESPACE_DRAW, XML_LUMINANCE,				XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    MAP( "AdjustContrast",					XML_NAMESPACE_DRAW,	XML_CONTRAST,				XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    MAP( "Gamma",							XML_NAMESPACE_DRAW, XML_GAMMA,					XML_TYPE_DOUBLE, 0 ), // signed? exists in SW, too, with same property name
    MAP( "AdjustRed",						XML_NAMESPACE_DRAW, XML_RED,					XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    MAP( "AdjustGreen",				    	XML_NAMESPACE_DRAW, XML_GREEN,					XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    MAP( "AdjustBlue",						XML_NAMESPACE_DRAW, XML_BLUE,					XML_TYPE_PERCENT16, 0 ), // signed? exists in SW, too, with same property name
    MAP( "GraphicCrop",				    	XML_NAMESPACE_FO,	XML_CLIP,					XML_TYPE_TEXT_CLIP, 0 ), // exists in SW, too, with same property name
    MAP( "Transparency",					XML_NAMESPACE_DRAW, XML_TRANSPARENCY,			XML_TYPE_PERCENT16|MID_FLAG_MULTI_PROPERTY, 0 ), // exists in SW, too, with same property name
    MAP( "IsMirrored",						XML_NAMESPACE_DRAW,	XML_MIRROR,					XML_TYPE_BOOL, 0 ),

    // animation text attributes
    MAP( "TextAnimationKind",				XML_NAMESPACE_STYLE,XML_TEXT_BLINKING,			XML_TYPE_TEXT_ANIMATION_BLINKING, CTF_TEXTANIMATION_BLINKING ),
    MAP( "TextAnimationKind",				XML_NAMESPACE_TEXT, XML_ANIMATION,				XML_TYPE_TEXT_ANIMATION, CTF_TEXTANIMATION_KIND ),
    MAP( "TextAnimationDirection",			XML_NAMESPACE_TEXT, XML_ANIMATION_DIRECTION,	XML_TYPE_TEXT_ANIMATION_DIRECTION, 0 ),
    MAP( "TextAnimationStartInside",		XML_NAMESPACE_TEXT, XML_ANIMATION_START_INSIDE, XML_TYPE_BOOL, 0 ),
    MAP( "TextAnimationStopInside",	    	XML_NAMESPACE_TEXT, XML_ANIMATION_STOP_INSIDE,	XML_TYPE_BOOL, 0 ),
    MAP( "TextAnimationCount",				XML_NAMESPACE_TEXT, XML_ANIMATION_REPEAT,		XML_TYPE_NUMBER16, 0 ),
    MAP( "TextAnimationDelay",				XML_NAMESPACE_TEXT, XML_ANIMATION_DELAY,		XML_TYPE_DURATION16_MS, 0 ),
    MAP( "TextAnimationAmount",		    	XML_NAMESPACE_TEXT, XML_ANIMATION_STEPS,		XML_TYPE_TEXT_ANIMATION_STEPS, 0 ),

    // connector attributes
    MAP( "EdgeNode1HorzDist",				XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_HORIZONTAL,	XML_TYPE_MEASURE, 0 ),
    MAP( "EdgeNode1VertDist",				XML_NAMESPACE_DRAW, XML_START_LINE_SPACING_VERTICAL,	XML_TYPE_MEASURE, 0 ),
    MAP( "EdgeNode2HorzDist",				XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_HORIZONTAL,	XML_TYPE_MEASURE, 0 ),
    MAP( "EdgeNode2VertDist",				XML_NAMESPACE_DRAW, XML_END_LINE_SPACING_VERTICAL,		XML_TYPE_MEASURE, 0 ),

    // measure attributes
    MAP( "MeasureLineDistance",		    	XML_NAMESPACE_DRAW, XML_LINE_DISTANCE,			XML_TYPE_MEASURE, 0 ),
    MAP( "MeasureHelpLineOverhang",	    	XML_NAMESPACE_DRAW, XML_GUIDE_OVERHANG, 		XML_TYPE_MEASURE, 0 ),
    MAP( "MeasureHelpLineDistance",	    	XML_NAMESPACE_DRAW, XML_GUIDE_DISTANCE, 		XML_TYPE_MEASURE, 0 ),
    MAP( "MeasureHelpLine1Length",			XML_NAMESPACE_DRAW, XML_START_GUIDE,			XML_TYPE_MEASURE, 0 ),
    MAP( "MeasureHelpLine2Length",			XML_NAMESPACE_DRAW, XML_END_GUIDE,				XML_TYPE_MEASURE, 0 ),
    MAP( "MeasureTextHorizontalPosition",	XML_NAMESPACE_DRAW, XML_MEASURE_ALIGN,			XML_SD_TYPE_MEASURE_HALIGN, 0 ),
    MAP( "MeasureTextVerticalPosition", 	XML_NAMESPACE_DRAW, XML_MEASURE_VERTICAL_ALIGN, XML_SD_TYPE_MEASURE_VALIGN, 0 ),
    MAP( "MeasureUnit",				    	XML_NAMESPACE_DRAW, XML_UNIT,					XML_SD_TYPE_MEASURE_UNIT, 0 ),
    MAP( "MeasureShowUnit",			    	XML_NAMESPACE_DRAW, XML_SHOW_UNIT,				XML_TYPE_BOOL, 0 ),
    MAP( "MeasureBelowReferenceEdge",		XML_NAMESPACE_DRAW, XML_PLACING,				XML_SD_TYPE_MEASURE_PLACING, 0 ),
    MAP( "MeasureTextRotate90",		    	XML_NAMESPACE_DRAW, XML_PARALLEL,				XML_TYPE_BOOL, 0 ),
    MAP( "MeasureDecimalPlaces",			XML_NAMESPACE_DRAW, XML_DECIMAL_PLACES,			XML_TYPE_NUMBER16, 0 ),

    // 3D geometry attributes
    MAP( "D3DHorizontalSegments",			XML_NAMESPACE_DR3D, XML_HORIZONTAL_SEGMENTS,	XML_TYPE_NUMBER, 0 ),
    MAP( "D3DVerticalSegments",		    	XML_NAMESPACE_DR3D, XML_VERTICAL_SEGMENTS,		XML_TYPE_NUMBER, 0 ),
    MAP( "D3DPercentDiagonal",				XML_NAMESPACE_DR3D, XML_EDGE_ROUNDING,			XML_TYPE_PERCENT, 0 ),
    MAP( "D3DBackscale",					XML_NAMESPACE_DR3D, XML_BACK_SCALE, 			XML_TYPE_PERCENT, 0 ),
    MAP( "D3DEndAngle",				    	XML_NAMESPACE_DR3D, XML_END_ANGLE,				XML_TYPE_NUMBER, 0 ),
    MAP( "D3DDepth",						XML_NAMESPACE_DR3D, XML_DEPTH,					XML_TYPE_MEASURE, 0 ),
    MAP( "D3DDoubleSided",					XML_NAMESPACE_DR3D, XML_BACKFACE_CULLING,		XML_SD_TYPE_BACKFACE_CULLING, 0 ),
    
    // #107245# New 3D properties which are possible for lathe and extrude 3d objects
    MAP( "D3DCloseFront",		    		XML_NAMESPACE_DR3D, XML_CLOSE_FRONT,			XML_TYPE_BOOL, 0 ),
    MAP( "D3DCloseBack",		    		XML_NAMESPACE_DR3D, XML_CLOSE_BACK,				XML_TYPE_BOOL, 0 ),
    
    // 3D lighting attributes
    MAP( "D3DNormalsKind",					XML_NAMESPACE_DR3D, XML_NORMALS_KIND,			XML_SD_TYPE_NORMALS_KIND, 0 ),
    MAP( "D3DNormalsInvert",				XML_NAMESPACE_DR3D, XML_NORMALS_DIRECTION,		XML_SD_TYPE_NORMALS_DIRECTION, 0 ),

    // 3D texture attributes
    MAP( "D3DTextureProjectionX",			XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_X,	XML_SD_TYPE_TEX_GENERATION_MODE_X, 0 ),
    MAP( "D3DTextureProjectionY",			XML_NAMESPACE_DR3D, XML_TEX_GENERATION_MODE_Y,	XML_SD_TYPE_TEX_GENERATION_MODE_Y, 0 ),
    MAP( "D3DTextureKind",					XML_NAMESPACE_DR3D, XML_TEX_KIND,				XML_SD_TYPE_TEX_KIND, 0 ),
    MAP( "D3DTextureMode",					XML_NAMESPACE_DR3D, XML_TEX_MODE,				XML_SD_TYPE_TEX_MODE, 0 ),
    MAP( "D3DTextureFilter",				XML_NAMESPACE_DR3D, XML_TEX_FILTER, 			XML_SD_TYPE_BACKFACE_CULLING, 0 ),

    // 3D material attributes
    MAP( "D3DMaterialColor",				XML_NAMESPACE_DR3D, XML_DIFFUSE_COLOR,			XML_TYPE_COLOR, 0 ),
    MAP( "D3DMaterialEmission",		    	XML_NAMESPACE_DR3D, XML_EMISSIVE_COLOR, 		XML_TYPE_COLOR, 0 ),
    MAP( "D3DMaterialSpecular",		    	XML_NAMESPACE_DR3D, XML_SPECULAR_COLOR, 		XML_TYPE_COLOR, 0 ),
    MAP( "D3DMaterialSpecularIntensity",	XML_NAMESPACE_DR3D, XML_SHININESS,				XML_TYPE_PERCENT, 0 ),

    // 3D shadow attributes
    MAP( "D3DShadow3D",				    	XML_NAMESPACE_DR3D, XML_SHADOW, 				XML_SD_TYPE_SHADOW, 0 ),

    // #FontWork# attributes
    MAP( "FontWorkStyle",					XML_NAMESPACE_DRAW, XML_FONTWORK_STYLE,				XML_SD_TYPE_FONTWORK_STYLE,	CTF_FONTWORK_STYLE	),
    MAP( "FontWorkAdjust",					XML_NAMESPACE_DRAW, XML_FONTWORK_ADJUST,				XML_SD_TYPE_FONTWORK_ADJUST,CTF_FONTWORK_ADJUST	),
    MAP( "FontWorkDistance",				XML_NAMESPACE_DRAW, XML_FONTWORK_DISTANCE,				XML_TYPE_MEASURE,			CTF_FONTWORK_DISTANCE	),
    MAP( "FontWorkStart",					XML_NAMESPACE_DRAW, XML_FONTWORK_START, 				XML_TYPE_MEASURE,			CTF_FONTWORK_START	),
    MAP( "FontWorkMirror",					XML_NAMESPACE_DRAW, XML_FONTWORK_MIRROR,				XML_TYPE_BOOL,				CTF_FONTWORK_MIRROR	),
    MAP( "FontWorkOutline",			    	XML_NAMESPACE_DRAW, XML_FONTWORK_OUTLINE,				XML_TYPE_BOOL,				CTF_FONTWORK_OUTLINE	),
    MAP( "FontWorkShadow",					XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW,				XML_SD_TYPE_FONTWORK_SHADOW,CTF_FONTWORK_SHADOW	),
    MAP( "FontWorkShadowColor",		    	XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_COLOR,			XML_TYPE_COLOR,				CTF_FONTWORK_SHADOWCOLOR	),
    MAP( "FontWorkShadowOffsetX",			XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_X,		XML_TYPE_MEASURE,			CTF_FONTWORK_SHADOWOFFSETX	),
    MAP( "FontWorkShadowOffsetY",			XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_OFFSET_Y,		XML_TYPE_MEASURE,			CTF_FONTWORK_SHADOWOFFSETY	),
    MAP( "FontWorkForm",					XML_NAMESPACE_DRAW, XML_FONTWORK_FORM,					XML_SD_TYPE_FONTWORK_FORM,	CTF_FONTWORK_FORM	),
    MAP( "FontWorkHideForm",				XML_NAMESPACE_DRAW, XML_FONTWORK_HIDE_FORM,			    XML_TYPE_BOOL,				CTF_FONTWORK_HIDEFORM	),
    MAP( "FontWorkShadowTransparence",		XML_NAMESPACE_DRAW, XML_FONTWORK_SHADOW_TRANSPARENCE,	XML_TYPE_PERCENT,			CTF_FONTWORK_SHADOWTRANSPARENCE	),

    // control attributes (border exists one mor time for the text additions of shapes)
    MAP( "ControlBackground",				XML_NAMESPACE_FO,	XML_BACKGROUND_COLOR,		XML_TYPE_COLOR|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "ControlBorder",					XML_NAMESPACE_FO,	XML_BORDER, 				XML_SD_TYPE_CONTROL_BORDER|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "ControlDataStyle",				XML_NAMESPACE_STYLE,XML_DATA_STYLE_NAME,		XML_TYPE_STRING|MID_FLAG_NO_PROPERTY_EXPORT|MID_FLAG_SPECIAL_ITEM, CTF_SD_CONTROL_SHAPE_DATA_STYLE ),
    MAP( "ControlTextEmphasis",			XML_NAMESPACE_STYLE,XML_TEXT_EMPHASIZE, 		XML_TYPE_CONTROL_TEXT_EMPHASIZE, 0 ),

    // special entries for floating frames
    MAP( "FrameIsAutoScroll",			XML_NAMESPACE_DRAW,	XML_FRAME_DISPLAY_SCROLLBAR,	XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,				CTF_FRAME_DISPLAY_SCROLLBAR ),
    MAP( "FrameIsBorder",				XML_NAMESPACE_DRAW,	XML_FRAME_DISPLAY_BORDER,		XML_TYPE_BOOL|MID_FLAG_MULTI_PROPERTY,				CTF_FRAME_DISPLAY_BORDER ),
    MAP( "FrameMarginWidth",			XML_NAMESPACE_DRAW,	XML_FRAME_MARGIN_HORIZONTAL,	XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,		CTF_FRAME_MARGIN_HORI ),
    MAP( "FrameMarginHeight",			XML_NAMESPACE_DRAW,	XML_FRAME_MARGIN_VERTICAL,		XML_TYPE_MEASURE_PX|MID_FLAG_MULTI_PROPERTY,		CTF_FRAME_MARGIN_VERT ),
    MAP( "VisibleArea",				XML_NAMESPACE_DRAW,	XML_VISIBLE_AREA_LEFT,			XML_TYPE_RECTANGLE_LEFT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY,	CTF_SD_OLE_VIS_AREA_LEFT ),
    MAP( "VisibleArea",				XML_NAMESPACE_DRAW,	XML_VISIBLE_AREA_TOP,			XML_TYPE_RECTANGLE_TOP|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY,		CTF_SD_OLE_VIS_AREA_TOP ),
    MAP( "VisibleArea",				XML_NAMESPACE_DRAW,	XML_VISIBLE_AREA_WIDTH, 		XML_TYPE_RECTANGLE_WIDTH|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY,	CTF_SD_OLE_VIS_AREA_WIDTH ),
    MAP( "VisibleArea",				XML_NAMESPACE_DRAW,	XML_VISIBLE_AREA_HEIGHT,		XML_TYPE_RECTANGLE_HEIGHT|MID_FLAG_MERGE_PROPERTY|MID_FLAG_MULTI_PROPERTY,	CTF_SD_OLE_VIS_AREA_HEIGHT ),
    MAP( "IsInternal",					XML_NAMESPACE_DRAW, XML__EMPTY,     				XML_TYPE_BUILDIN_CMP_ONLY,							CTF_SD_OLE_ISINTERNAL ),

    // caption properties
    MAP( "CaptionType",			    	XML_NAMESPACE_DRAW, XML_CAPTION_TYPE,				XML_SD_TYPE_CAPTION_TYPE, 0 ),
    MAP( "CaptionIsFixedAngle",	    	XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE_TYPE, 		XML_SD_TYPE_CAPTION_ANGLE_TYPE,	0 ),
    MAP( "CaptionAngle",				XML_NAMESPACE_DRAW, XML_CAPTION_ANGLE,				XML_TYPE_NUMBER, 0 ),
    MAP( "CaptionGap",					XML_NAMESPACE_DRAW,	XML_CAPTION_GAP,				XML_TYPE_MEASURE, 0 ),
    MAP( "CaptionEscapeDirection",		XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE_DIRECTION,	XML_SD_TYPE_CAPTION_ESC_DIR, 0 ),
    MAP( "CaptionIsEscapeRelative",	XML_NAMESPACE_DRAW,	XML_CAPTION_ESCAPE, 			XML_SD_TYPE_CAPTION_IS_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ISESCREL ),
    MAP( "CaptionEscapeRelative",		XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE, 			XML_SD_TYPE_CAPTION_ESC_REL|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCREL ),
    MAP( "CaptionEscapeAbsolute",		XML_NAMESPACE_DRAW, XML_CAPTION_ESCAPE, 			XML_SD_TYPE_CAPTION_ESC_ABS|MID_FLAG_MULTI_PROPERTY, CTF_CAPTION_ESCABS ),
    MAP( "CaptionLineLength",			XML_NAMESPACE_DRAW, XML_CAPTION_LINE_LENGTH,		XML_TYPE_MEASURE, 0 ),
    MAP( "CaptionIsFitLineLength",		XML_NAMESPACE_DRAW, XML_CAPTION_FIT_LINE_LENGTH,	XML_TYPE_BOOL, 0 ),

    // misc object properties
    MAP( "MoveProtect",			    	XML_NAMESPACE_DRAW, XML_MOVE_PROTECT,				XML_TYPE_BOOL, CTF_SD_MOVE_PROTECT ),
    MAP( "SizeProtect",			    	XML_NAMESPACE_DRAW, XML_SIZE_PROTECT,				XML_TYPE_BOOL, CTF_SD_SIZE_PROTECT ),
    { 0L }
};

//////////////////////////////////////////////////////////////////////////////
// entry list for presentation page properties

const XMLPropertyMapEntry aXMLSDPresPageProps[] = 
{
    MAP( "UserDefinedAttributes",		XML_NAMESPACE_TEXT, XML_XMLNS,					XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

    MAP( "Change",						XML_NAMESPACE_PRESENTATION, XML_TRANSITION_TYPE,	XML_SD_TYPE_PRESPAGE_TYPE, CTF_PAGE_TRANS_TYPE ),
    MAP( "Effect",						XML_NAMESPACE_PRESENTATION, XML_TRANSITION_STYLE,   XML_SD_TYPE_PRESPAGE_STYLE, CTF_PAGE_TRANS_STYLE ),
    MAP( "Speed",						XML_NAMESPACE_PRESENTATION, XML_TRANSITION_SPEED,   XML_SD_TYPE_PRESPAGE_SPEED, CTF_PAGE_TRANS_SPEED ),
    MAP( "Duration",					XML_NAMESPACE_PRESENTATION, XML_DURATION,		    XML_SD_TYPE_PRESPAGE_DURATION, CTF_PAGE_TRANS_DURATION ),
    MAP( "Visible",					XML_NAMESPACE_PRESENTATION, XML_VISIBILITY,		    XML_SD_TYPE_PRESPAGE_VISIBILITY, CTF_PAGE_VISIBLE ),
    MAP( "Sound",						XML_NAMESPACE_PRESENTATION, XML_SOUND,			    XML_TYPE_STRING|MID_FLAG_ELEMENT_ITEM, CTF_PAGE_SOUND_URL ),
    MAP( "BackgroundFullSize",			XML_NAMESPACE_DRAW,			XML_BACKGROUND_SIZE,	XML_SD_TYPE_PRESPAGE_BACKSIZE, CTF_PAGE_BACKSIZE ),

    MAP( "IsBackgroundVisible",		XML_NAMESPACE_PRESENTATION,	XML_BACKGROUND_VISIBLE, XML_TYPE_BOOL, 0 ),
    MAP( "IsBackgroundObjectsVisible",	XML_NAMESPACE_PRESENTATION,	XML_BACKGROUND_OBJECTS_VISIBLE, XML_TYPE_BOOL, 0 ),

    MAP( "FillStyle",					XML_NAMESPACE_DRAW,	XML_FILL,					XML_SD_TYPE_FILLSTYLE, 0 ),
    MAP( "FillColor",					XML_NAMESPACE_DRAW, XML_FILL_COLOR, 			XML_TYPE_COLOR, 0 ),
    MAP( "FillGradientName",			XML_NAMESPACE_DRAW, XML_FILL_GRADIENT_NAME, 	XML_TYPE_STRING, 0 ),
    MAP( "FillGradientStepCount",		XML_NAMESPACE_DRAW, XML_GRADIENT_STEP_COUNT,	XML_TYPE_NUMBER, 0 ),
    MAP( "FillHatchName",				XML_NAMESPACE_DRAW, XML_FILL_HATCH_NAME,		XML_TYPE_STRING, 0 ),
    MAP( "FillBitmapName",				XML_NAMESPACE_DRAW, XML_FILL_IMAGE_NAME,		XML_TYPE_STRING, 0 ),
    MAP( "FillTransparenceName",		XML_NAMESPACE_DRAW, XML_TRANSPARENCY_NAME,		XML_TYPE_STRING, 0 ),
    MAP( "FillBitmapSizeX",			XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_WIDTH,		XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapLogicalSize",		XML_NAMESPACE_DRAW, XML_FILL_IMAGE_WIDTH,		XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapSizeY",			XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_HEIGHT,		XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapLogicalSize",		XML_NAMESPACE_DRAW, XML_FILL_IMAGE_HEIGHT,		XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY, 0 ),
    MAP( "FillBitmapMode",				XML_NAMESPACE_STYLE,XML_REPEAT, 				XML_SD_TYPE_BITMAP_MODE, 0 ),
    MAP( "FillBitmapPositionOffsetX",	XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT_X, XML_TYPE_PERCENT, 0 ),
    MAP( "FillBitmapPositionOffsetY",	XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT_Y, XML_TYPE_PERCENT, 0 ),
    MAP( "FillBitmapRectanglePoint",	XML_NAMESPACE_DRAW,	XML_FILL_IMAGE_REF_POINT,	XML_SD_TYPE_BITMAP_REFPOINT, 0 ),
    MAP( "FillBitmapOffsetX",			XML_NAMESPACE_DRAW,	XML_TILE_REPEAT_OFFSET,	    XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X ),
    MAP( "FillBitmapOffsetY",			XML_NAMESPACE_DRAW,	XML_TILE_REPEAT_OFFSET,	XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y ),

    { 0L }
};

//////////////////////////////////////////////////////////////////////////////
// implementation of factory for own graphic properties

SvXMLEnumMapEntry aXML_LineStyle_EnumMap[] =
{
    { XML_NONE, 	drawing::LineStyle_NONE },
    { XML_SOLID,	drawing::LineStyle_SOLID },
    { XML_DASH,	    drawing::LineStyle_DASH },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_LineJoint_EnumMap[] =
{
    { XML_NONE,	    drawing::LineJoint_NONE },
    { XML_MITER,	drawing::LineJoint_MITER },
    { XML_ROUND,	drawing::LineJoint_ROUND },
    { XML_BEVEL,	drawing::LineJoint_BEVEL },
    { XML_MIDDLE,	drawing::LineJoint_MIDDLE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_FillStyle_EnumMap[] =
{
    { XML_NONE,	    drawing::FillStyle_NONE },
    { XML_SOLID,	drawing::FillStyle_SOLID },
    { XML_BITMAP,	drawing::FillStyle_BITMAP },
    { XML_GRADIENT, drawing::FillStyle_GRADIENT },
    { XML_HATCH,	drawing::FillStyle_HATCH },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_PresChange_EnumMap[] =
{
    { XML_MANUAL,			0 },
    { XML_AUTOMATIC,		1 },
    { XML_SEMI_AUTOMATIC,	2 },
    { XML_TOKEN_INVALID,					0 }
};

SvXMLEnumMapEntry aXML_TransSpeed_EnumMap[] =
{
    { XML_FAST,     presentation::AnimationSpeed_FAST },
    { XML_MEDIUM,	presentation::AnimationSpeed_MEDIUM },
    { XML_SLOW,	    presentation::AnimationSpeed_SLOW },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_FadeEffect_EnumMap[] =
{
    { XML_NONE,				    presentation::FadeEffect_NONE },
    { XML_FADE_FROM_LEFT,		presentation::FadeEffect_FADE_FROM_LEFT },
    { XML_FADE_FROM_TOP,		presentation::FadeEffect_FADE_FROM_TOP },
    { XML_FADE_FROM_RIGHT,		presentation::FadeEffect_FADE_FROM_RIGHT },
    { XML_FADE_FROM_BOTTOM, 	presentation::FadeEffect_FADE_FROM_BOTTOM },
    { XML_FADE_TO_CENTER,		presentation::FadeEffect_FADE_TO_CENTER },
    { XML_FADE_FROM_CENTER,	    presentation::FadeEffect_FADE_FROM_CENTER },
    { XML_MOVE_FROM_LEFT,		presentation::FadeEffect_MOVE_FROM_LEFT },
    { XML_MOVE_FROM_TOP,		presentation::FadeEffect_MOVE_FROM_TOP },
    { XML_MOVE_FROM_RIGHT,		presentation::FadeEffect_MOVE_FROM_RIGHT },
    { XML_MOVE_FROM_BOTTOM,	    presentation::FadeEffect_MOVE_FROM_BOTTOM },
    { XML_ROLL_FROM_TOP,		presentation::FadeEffect_ROLL_FROM_TOP },
    { XML_ROLL_FROM_LEFT,		presentation::FadeEffect_ROLL_FROM_LEFT },
    { XML_ROLL_FROM_RIGHT,		presentation::FadeEffect_ROLL_FROM_RIGHT },
    { XML_ROLL_FROM_BOTTOM,	    presentation::FadeEffect_ROLL_FROM_BOTTOM },
    { XML_VERTICAL_STRIPES,	    presentation::FadeEffect_VERTICAL_STRIPES },
    { XML_HORIZONTAL_STRIPES,	presentation::FadeEffect_HORIZONTAL_STRIPES },
    { XML_CLOCKWISE,			presentation::FadeEffect_CLOCKWISE },
    { XML_COUNTERCLOCKWISE,	    presentation::FadeEffect_COUNTERCLOCKWISE },
    { XML_FADE_FROM_UPPERLEFT,	presentation::FadeEffect_FADE_FROM_UPPERLEFT },
    { XML_FADE_FROM_UPPERRIGHT,	presentation::FadeEffect_FADE_FROM_UPPERRIGHT },
    { XML_FADE_FROM_LOWERLEFT,	presentation::FadeEffect_FADE_FROM_LOWERLEFT },
    { XML_FADE_FROM_LOWERRIGHT,	presentation::FadeEffect_FADE_FROM_LOWERRIGHT },
    { XML_CLOSE_VERTICAL,		presentation::FadeEffect_CLOSE_VERTICAL },
    { XML_CLOSE_HORIZONTAL, 	presentation::FadeEffect_CLOSE_HORIZONTAL },
    { XML_OPEN_VERTICAL,		presentation::FadeEffect_OPEN_VERTICAL },
    { XML_OPEN_HORIZONTAL,		presentation::FadeEffect_OPEN_HORIZONTAL },
    { XML_SPIRALIN_LEFT,		presentation::FadeEffect_SPIRALIN_LEFT },
    { XML_SPIRALIN_RIGHT,		presentation::FadeEffect_SPIRALIN_RIGHT },
    { XML_SPIRALOUT_LEFT,		presentation::FadeEffect_SPIRALOUT_LEFT },
    { XML_SPIRALOUT_RIGHT,		presentation::FadeEffect_SPIRALOUT_RIGHT },
    { XML_DISSOLVE, 			presentation::FadeEffect_DISSOLVE },
    { XML_WAVYLINE_FROM_LEFT,	presentation::FadeEffect_WAVYLINE_FROM_LEFT },
    { XML_WAVYLINE_FROM_TOP,	presentation::FadeEffect_WAVYLINE_FROM_TOP },
    { XML_WAVYLINE_FROM_RIGHT,	presentation::FadeEffect_WAVYLINE_FROM_RIGHT },
    { XML_WAVYLINE_FROM_BOTTOM,	presentation::FadeEffect_WAVYLINE_FROM_BOTTOM },
    { XML_RANDOM,				presentation::FadeEffect_RANDOM },
    { XML_STRETCH_FROM_LEFT,	presentation::FadeEffect_STRETCH_FROM_LEFT },
    { XML_STRETCH_FROM_TOP, 	presentation::FadeEffect_STRETCH_FROM_TOP },
    { XML_STRETCH_FROM_RIGHT,	presentation::FadeEffect_STRETCH_FROM_RIGHT },
    { XML_STRETCH_FROM_BOTTOM,	presentation::FadeEffect_STRETCH_FROM_BOTTOM },
    { XML_VERTICAL_LINES,		presentation::FadeEffect_VERTICAL_LINES },
    { XML_HORIZONTAL_LINES, 	presentation::FadeEffect_HORIZONTAL_LINES },
    { XML_MOVE_FROM_UPPERLEFT,	presentation::FadeEffect_MOVE_FROM_UPPERLEFT },
    { XML_MOVE_FROM_UPPERRIGHT,	presentation::FadeEffect_MOVE_FROM_UPPERRIGHT },
    { XML_MOVE_FROM_LOWERRIGHT,	presentation::FadeEffect_MOVE_FROM_LOWERRIGHT },
    { XML_MOVE_FROM_LOWERLEFT,	presentation::FadeEffect_MOVE_FROM_LOWERLEFT },
    { XML_UNCOVER_TO_LEFT,		presentation::FadeEffect_UNCOVER_TO_LEFT },
    { XML_UNCOVER_TO_UPPERLEFT,	presentation::FadeEffect_UNCOVER_TO_UPPERLEFT },
    { XML_UNCOVER_TO_TOP,		presentation::FadeEffect_UNCOVER_TO_TOP },
    { XML_UNCOVER_TO_UPPERRIGHT,presentation::FadeEffect_UNCOVER_TO_UPPERRIGHT },
    { XML_UNCOVER_TO_RIGHT,		presentation::FadeEffect_UNCOVER_TO_RIGHT },
    { XML_UNCOVER_TO_LOWERRIGHT,presentation::FadeEffect_UNCOVER_TO_LOWERRIGHT },
    { XML_UNCOVER_TO_BOTTOM,	presentation::FadeEffect_UNCOVER_TO_BOTTOM },
    { XML_UNCOVER_TO_LOWERLEFT,	presentation::FadeEffect_UNCOVER_TO_LOWERLEFT },
    { XML_VERTICAL_CHECKERBOARD,presentation::FadeEffect_VERTICAL_CHECKERBOARD },
    { XML_HORIZONTAL_CHECKERBOARD,presentation::FadeEffect_HORIZONTAL_CHECKERBOARD },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry	aXML_ConnectionKind_EnumMap[] =
{
    { XML_STANDARD,	    drawing::ConnectorType_STANDARD },
    { XML_CURVE,		drawing::ConnectorType_CURVE },
    { XML_LINE,		    drawing::ConnectorType_LINE },
    { XML_LINES,		drawing::ConnectorType_LINES },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry	aXML_BitmapMode_EnumMap[] =
{
    { XML_REPEAT,					drawing::BitmapMode_REPEAT },
    { XML_STRETCH,					drawing::BitmapMode_STRETCH },
    { XML_BACKGROUND_NO_REPEAT, 	drawing::BitmapMode_NO_REPEAT },
    { XML_TOKEN_INVALID, 0 }
};

//////////////////////////////////////////////////////////////////////////////
// 3D EnumMaps

SvXMLEnumMapEntry  aXML_NormalsKind_EnumMap[] =
{
    { XML_OBJECT,		drawing::NormalsKind_SPECIFIC },
    { XML_FLAT,		    drawing::NormalsKind_FLAT },
    { XML_SPHERE,		drawing::NormalsKind_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationX_EnumMap[] =
{
    { XML_OBJECT,		drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,	    drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,		drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationY_EnumMap[] =
{
    { XML_OBJECT,		drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { XML_PARALLEL,	    drawing::TextureProjectionMode_PARALLEL },
    { XML_SPHERE,		drawing::TextureProjectionMode_SPHERE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexKind_EnumMap[] =
{
    { XML_LUMINANCE,	drawing::TextureKind_LUMINANCE },
//    { XML_INTENSITY,	drawing::TextureKind_INTENSITY },
    { XML_COLOR,		drawing::TextureKind_COLOR },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry  aXML_TexMode_EnumMap[] =
{
    { XML_REPLACE,		drawing::TextureMode_REPLACE },
    { XML_MODULATE,	    drawing::TextureMode_MODULATE },
    { XML_BLEND,		drawing::TextureMode_BLEND },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_RefPoint_EnumMap[] =
{
    { XML_TOP_LEFT,	    drawing::RectanglePoint_LEFT_TOP },
    { XML_TOP,			drawing::RectanglePoint_MIDDLE_TOP },
    { XML_TOP_RIGHT,	drawing::RectanglePoint_RIGHT_TOP },
    { XML_LEFT,		    drawing::RectanglePoint_LEFT_MIDDLE },
    { XML_CENTER,		drawing::RectanglePoint_MIDDLE_MIDDLE },
    { XML_RIGHT,		drawing::RectanglePoint_RIGHT_MIDDLE },
    { XML_BOTTOM_LEFT,	drawing::RectanglePoint_LEFT_BOTTOM },
    { XML_BOTTOM,		drawing::RectanglePoint_MIDDLE_BOTTOM },
    { XML_BOTTOM_RIGHT, drawing::RectanglePoint_RIGHT_BOTTOM },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_CircleKind_EnumMap[] =
{
    { XML_FULL,		    drawing::CircleKind_FULL },
    { XML_SECTION,		drawing::CircleKind_SECTION },
    { XML_CUT,			drawing::CircleKind_CUT },
    { XML_ARC,			drawing::CircleKind_ARC },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_WritingMode_EnumMap[] =
{
    { XML_TB_RL,		text::WritingMode_TB_RL },
    { XML_LR_TB,		text::WritingMode_LR_TB },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_TextAnimation_Enum[] =
{
    { XML_NONE, 		drawing::TextAnimationKind_NONE	},
    { XML_BLINKING,	    drawing::TextAnimationKind_BLINK },	// will be filtered
    { XML_SCROLL,		drawing::TextAnimationKind_SCROLL },
    { XML_ALTERNATE,	drawing::TextAnimationKind_ALTERNATE },
    { XML_SLIDE,		drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_TextAnimation_Blinking_Enum[] =
{
    { XML_FALSE,		drawing::TextAnimationKind_NONE	},
    { XML_TRUE, 		drawing::TextAnimationKind_BLINK },
    { XML_FALSE,		drawing::TextAnimationKind_SCROLL },
    { XML_FALSE,		drawing::TextAnimationKind_ALTERNATE },
    { XML_FALSE,		drawing::TextAnimationKind_SLIDE },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_TextAnimationDirection_Enum[] =
{
    { XML_LEFT,	    	drawing::TextAnimationDirection_LEFT },
    { XML_RIGHT,		drawing::TextAnimationDirection_RIGHT },	// will be filtered
    { XML_UP,			drawing::TextAnimationDirection_UP },
    { XML_DOWN, 		drawing::TextAnimationDirection_DOWN },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_TextAlign_Enum[] =
{
    { XML_LEFT,		    drawing::TextHorizontalAdjust_LEFT },
    { XML_CENTER,		drawing::TextHorizontalAdjust_CENTER },
    { XML_RIGHT,		drawing::TextHorizontalAdjust_RIGHT },
    { XML_JUSTIFY,		drawing::TextHorizontalAdjust_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_VerticalAlign_Enum[] =
{
    { XML_TOP,			drawing::TextVerticalAdjust_TOP },
    { XML_MIDDLE,		drawing::TextVerticalAdjust_CENTER },
    { XML_BOTTOM,		drawing::TextVerticalAdjust_BOTTOM },
    { XML_JUSTIFY,		drawing::TextVerticalAdjust_BLOCK },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_FitToSize_Enum[] =
{
    { XML_FALSE,		drawing::TextFitToSizeType_NONE },
    { XML_TRUE, 		drawing::TextFitToSizeType_PROPORTIONAL },
    { XML_TRUE,	    	drawing::TextFitToSizeType_ALLLINES },
    { XML_TRUE,		    drawing::TextFitToSizeType_RESIZEATTR },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_MeasureUnit_Enum[] =
{
    { XML_AUTOMATIC,	0 },
    { XML_UNIT_MM,		1 },
    { XML_UNIT_CM,		2 },
    { XML_UNIT_M,		3 },
    { XML_UNIT_KM,		4 },
    { XML_UNIT_PT,		6 },
    { XML_UNIT_PC,		7 },
    { XML_UNIT_INCH,	8 },
    { XML_UNIT_FOOT,	9 },
    { XML_UNIT_MILES,	10 },
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Measure_HAlign_Enum[] =
{
    { XML_AUTOMATIC,		drawing::MeasureTextHorzPos_AUTO },
    { XML_LEFT_OUTSIDE, 	drawing::MeasureTextHorzPos_LEFTOUTSIDE },
    { XML_INSIDE,			drawing::MeasureTextHorzPos_INSIDE },
    { XML_RIGHT_OUTSIDE,	drawing::MeasureTextHorzPos_RIGHTOUTSIDE},
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Measure_VAlign_Enum[] =
{
    { XML_AUTOMATIC,	drawing::MeasureTextVertPos_AUTO },
    { XML_ABOVE,		drawing::MeasureTextVertPos_EAST },
    { XML_BELOW,		drawing::MeasureTextVertPos_WEST },
    { XML_CENTER,		drawing::MeasureTextVertPos_CENTERED },
    { XML_TOKEN_INVALID,0 }
};

// #FontWork#
SvXMLEnumMapEntry __READONLY_DATA pXML_Fontwork_Style_Enum[] =
{
    { XML_ROTATE,		0 }, //XFT_ROTATE, 
    { XML_UPRIGHT,		1 }, //XFT_UPRIGHT, 
    { XML_SLANT_X,		2 }, //XFT_SLANTX, 
    { XML_SLANT_Y,		3 }, //XFT_SLANTY, 
    { XML_NONE, 		4 }, //XFT_NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Fontwork_Adjust_Enum[] =
{
    { XML_LEFT, 		0 }, //XFT_LEFT, 
    { XML_RIGHT,		1 }, //XFT_RIGHT, 
    { XML_AUTOSIZE,	    2 }, //XFT_AUTOSIZE, 
    { XML_CENTER,		3 }, //XFT_CENTER
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Fontwork_Shadow_Enum[] =
{
    { XML_NORMAL,		0 }, //XFTSHADOW_NORMAL, 
    { XML_SLANT,		1 }, //XFTSHADOW_SLANT, 
    { XML_NONE, 		2 }, //XFTSHADOW_NONE
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Fontwork_Form_Enum[] =
{
    { XML_NONE,			    0 }, //XFTFORM_NONE, 
    { XML_TOPCIRCLE,		1 }, //XFTFORM_TOPCIRC, 
    { XML_BOTTOMCIRCLE, 	2 }, //XFTFORM_BOTCIRC,
    { XML_LEFTCIRCLE,		3 }, //XFTFORM_LFTCIRC, 
    { XML_RIGHTCIRCLE,		4 }, //XFTFORM_RGTCIRC, 
    { XML_TOPARC,			5 }, //XFTFORM_TOPARC,
    { XML_BOTTOMARC,		6 }, //XFTFORM_BOTARC, 
    { XML_LEFTARC,			7 }, //XFTFORM_LFTARC, 
    { XML_RIGHTARC, 		8 }, //XFTFORM_RGTARC,
    { XML_BUTTON1,			9 }, //XFTFORM_BUTTON1, 
    { XML_BUTTON2,			10 }, //XFTFORM_BUTTON2,
    { XML_BUTTON3,			11 }, //XFTFORM_BUTTON3, 
    { XML_BUTTON4,			12 }, //XFTFORM_BUTTON4
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Caption_Esc_Dir_Enum[] =
{
    { XML_HORIZONTAL,		0 }, //SDRCAPT_ESCHORIZONTAL, 
    { XML_VERTICAL,		    1 }, //SDRCAPT_ESCVERTICAL, 
    { XML_AUTO,			    2 }, //SDRCAPT_ESCBESTFIT, 
    { XML_TOKEN_INVALID,0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Caption_Type_Enum[] =
{
    { XML_STRAIGHT_LINE,			0 }, //SDRCAPT_TYPE1, 
    { XML_ANGLED_LINE,				1 }, //SDRCAPT_TYPE2, 
    { XML_ANGLED_CONNECTOR_LINE,	2 }, //SDRCAPT_TYPE3, 
    { XML_TOKEN_INVALID,0 }
};

//////////////////////////////////////////////////////////////////////////////

XMLSdPropHdlFactory::XMLSdPropHdlFactory( uno::Reference< frame::XModel > xModel )
: mxModel( xModel )
{
}

XMLSdPropHdlFactory::~XMLSdPropHdlFactory() 
{
}

const XMLPropertyHandler* XMLSdPropHdlFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    XMLPropertyHandler* pHdl = (XMLPropertyHandler*)XMLPropertyHandlerFactory::GetPropertyHandler( nType );		
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
                pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_CROSSEDOUT_SINGLE), GetXMLToken(XML_NONE) );
                break;
            }
            case XML_SD_TYPE_OPACITY :
            {
                pHdl = new XMLOpacityPropertyHdl();
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
                // This BOOL needs to be flipped, DoubleSided TRUE -> NO Backface culling
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
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Enum, ::getCppuType((const ::com::sun::star::drawing::TextAnimationKind*)0) ); 
                break;
            case XML_TYPE_TEXT_ANIMATION_BLINKING:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimation_Blinking_Enum, ::getCppuType((const ::com::sun::star::drawing::TextAnimationKind*)0) ); 
                break;
            case XML_TYPE_TEXT_ANIMATION_DIRECTION:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAnimationDirection_Enum, ::getCppuType((const ::com::sun::star::drawing::TextAnimationDirection*)0) ); 
                break;
            case XML_TYPE_TEXT_ANIMATION_STEPS:
                pHdl = new XMLTextAnimationStepPropertyHdl;
                break;
            case XML_SD_TYPE_TEXT_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_TextAlign_Enum, ::getCppuType((const ::com::sun::star::drawing::TextHorizontalAdjust*)0) );
                break;
            case XML_SD_TYPE_VERTICAL_ALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_VerticalAlign_Enum, ::getCppuType((const ::com::sun::star::drawing::TextVerticalAdjust*)0) );
                break;
            case XML_SD_TYPE_FITTOSIZE:
                pHdl = new XMLEnumPropertyHdl( pXML_FitToSize_Enum, ::getCppuType((const ::com::sun::star::drawing::TextFitToSizeType*)0) );
                break;
            case XML_SD_TYPE_MEASURE_UNIT:
                pHdl = new XMLEnumPropertyHdl( pXML_MeasureUnit_Enum, ::getCppuType((const sal_Int32*)0) );
                break;
            case XML_SD_TYPE_MEASURE_HALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_HAlign_Enum, ::getCppuType((const ::com::sun::star::drawing::MeasureTextHorzPos*)0) );
                break;
            case XML_SD_TYPE_MEASURE_VALIGN:
                pHdl = new XMLEnumPropertyHdl( pXML_Measure_VAlign_Enum, ::getCppuType((const ::com::sun::star::drawing::MeasureTextVertPos*)0) );
                break;
            case XML_SD_TYPE_MEASURE_PLACING:
                {
                    pHdl = new XMLNamedBoolPropertyHdl( GetXMLToken(XML_BELOW), GetXMLToken(XML_ABOVE) );
                }
                break;
            case XML_TYPE_TEXT_CLIP:
                pHdl = new XMLClipPropertyHandler;
                break;

            // #FontWork#
            case XML_SD_TYPE_FONTWORK_STYLE		:
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Style_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_ADJUST		:
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Adjust_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_SHADOW		:
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Shadow_Enum , ::getCppuType((const sal_Int32*)0));
                break;
            case XML_SD_TYPE_FONTWORK_FORM		:
                pHdl = new XMLEnumPropertyHdl( pXML_Fontwork_Form_Enum , ::getCppuType((const sal_Int32*)0));
                break;

            case XML_SD_TYPE_CONTROL_BORDER:
                pHdl = new xmloff::OControlBorderHandler;
                break;
            case XML_TYPE_CONTROL_TEXT_EMPHASIZE:
                pHdl = new ::binfilter::xmloff::OControlTextEmphasisHandler;
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
                pHdl = new XMLPercentOrMeasurePropertyHandler( sal_True );
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

#ifndef SVX_LIGHT
// ----------------------------------------

XMLShapeExportPropertyMapper::XMLShapeExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, XMLTextListAutoStylePool *pListAutoPool, SvXMLExport& rExport ) :
        SvXMLExportPropertyMapper( rMapper ),
        mpListAutoPool( pListAutoPool ),
        mrExport( rExport ),
        maNumRuleExp( rExport ),
        msCDATA( GetXMLToken(XML_CDATA)),
        msTrue( GetXMLToken(XML_TRUE)),
        msFalse( GetXMLToken(XML_FALSE)),
        mbIsInAutoStyles( sal_True )
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

    // filter properties
    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {	
        XMLPropertyState *property = &(*aIter);
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
                    text::WritingMode eWritingMode;
                    if( property->maValue >>= eWritingMode )
                    {
                        if( text::WritingMode_LR_TB == eWritingMode )
                            property->mnIndex = -1;
                    }
                }
                break;
            case CTF_REPEAT_OFFSET_X:
                pRepeatOffsetX = property;
                break;

            case CTF_REPEAT_OFFSET_Y:
                pRepeatOffsetY = property;
                break;

            case CTF_DASHNAME:
            case CTF_LINESTARTNAME:
            case CTF_LINEENDNAME:
            case CTF_FILLGRADIENTNAME:
            case CTF_FILLHATCHNAME:
            case CTF_FILLBITMAPNAME:
// #85953# take out this case to allow writing empty
// XML_TRANSPARENCY_NAME entries. This is used to represent
// disabled FillTransparencyItems.
//			case CTF_FILLTRANSNAME:
                {
                    if( !mbIsInAutoStyles )
                    {
                        OUString aStr;
                        if( (property->maValue >>= aStr) && 0 == aStr.getLength() )
                            property->mnIndex = -1;
                    }
                }
                break;
            case CTF_TEXTANIMATION_BLINKING:
                pTextAnimationBlinking = property;
                break;
            case CTF_TEXTANIMATION_KIND:
                pTextAnimationKind = property;
                break;

            // #FontWork#
            case CTF_FONTWORK_STYLE:				pFontWorkStyle = property;				break;
            case CTF_FONTWORK_ADJUST:				pFontWorkAdjust = property;				break;
            case CTF_FONTWORK_DISTANCE:				pFontWorkDistance = property;			break;
            case CTF_FONTWORK_START:				pFontWorkStart = property;				break;
            case CTF_FONTWORK_MIRROR:				pFontWorkMirror = property;				break;
            case CTF_FONTWORK_OUTLINE:				pFontWorkOutline = property;			break;
            case CTF_FONTWORK_SHADOW:				pFontWorkShadow = property;				break;
            case CTF_FONTWORK_SHADOWCOLOR:			pFontWorkShadowColor = property;		break;
            case CTF_FONTWORK_SHADOWOFFSETX:		pFontWorkShadowOffsetx = property;		break;
            case CTF_FONTWORK_SHADOWOFFSETY:		pFontWorkShadowOffsety = property;		break;
            case CTF_FONTWORK_FORM:					pFontWorkForm = property;				break;
            case CTF_FONTWORK_HIDEFORM:				pFontWorkHideform = property;			break;
            case CTF_FONTWORK_SHADOWTRANSPARENCE:	pFontWorkShadowTransparence = property;	break;

            // OLE
            case CTF_SD_OLE_VIS_AREA_LEFT:			pOLEVisAreaLeft = property;		break;
            case CTF_SD_OLE_VIS_AREA_TOP:			pOLEVisAreaTop = property;		break;
            case CTF_SD_OLE_VIS_AREA_WIDTH:			pOLEVisAreaWidth = property;	break;
            case CTF_SD_OLE_VIS_AREA_HEIGHT:		pOLEVisAreaHeight = property;	break;
            case CTF_SD_OLE_ISINTERNAL:				pOLEIsInternal = property;		break;
            case CTF_FRAME_DISPLAY_SCROLLBAR:
                {
                    if( !property->maValue.hasValue() )
                        property->mnIndex = -1;
                }
                break;
            case CTF_SD_MOVE_PROTECT:
                {
                    sal_Bool bProtected;
                    property->maValue >>= bProtected;
                    if( !bProtected )
                        property->mnIndex = -1;
                }
                break;
            case CTF_SD_SIZE_PROTECT:
                {
                    sal_Bool bProtected;
                    property->maValue >>= bProtected;
                    if( !bProtected )
                        property->mnIndex = -1;
                }
                break;
            case CTF_CAPTION_ISESCREL:				pCaptionIsEscRel = property;	break;
            case CTF_CAPTION_ESCREL:				pCaptionEscRel = property;		break;
            case CTF_CAPTION_ESCABS:				pCaptionEscAbs = property;		break;
        }
    }

    // do not export visual area for internal ole objects
    if( pOLEIsInternal )
    {
        sal_Bool bInternal;
        if( (pOLEIsInternal->maValue >>= bInternal) && bInternal )
        {
            if( pOLEVisAreaLeft ) pOLEVisAreaLeft->mnIndex = -1;
            if( pOLEVisAreaTop ) pOLEVisAreaTop->mnIndex = -1;
            if( pOLEVisAreaWidth ) pOLEVisAreaWidth->mnIndex = -1;
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
        sal_Int32 nOffset;
        if( ( pRepeatOffsetX->maValue >>= nOffset ) && ( nOffset == 0 ) )
            pRepeatOffsetX->mnIndex = -1;
        else
            pRepeatOffsetY->mnIndex = -1;
    }

    if(pFontWorkStyle)
    {
        // #FontWork#
        sal_Int32 nStyle;

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
        sal_Bool bIsRel;
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
                    OUString sName;
                    uno::Reference< container::XIndexReplace > xNumRule;
                    if( rProperty.maValue >>= xNumRule )
                        ((XMLShapeExportPropertyMapper*)this)->maNumRuleExp.exportNumberingRule( sName, xNumRule );
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

    // filter properties
    for( ::std::vector< XMLPropertyState >::iterator aIter = rProperties.begin();
         aIter != rProperties.end();
         ++aIter )
    {
        XMLPropertyState*property = &(*aIter);
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
                {
                    presentation::FadeEffect aEnum;
                    if( ((*property).maValue >>= aEnum) && aEnum == presentation::FadeEffect_NONE )
                        (*property).mnIndex = -1;
                }
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
                    sal_Bool bVisible;
                    (*property).maValue >>= bVisible;
                    if( bVisible )
                        (*property).mnIndex = -1;
                }
                break;
            case CTF_PAGE_TRANS_DURATION:
                pTransDuration = property;
                break;
        }
    }

    if( pRepeatOffsetX && pRepeatOffsetY )
    {
        sal_Int32 nOffset;
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
                if( (rProperty.maValue >>= aSoundURL) && aSoundURL.getLength() != 0 )
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

#endif // #ifndef SVX_LIGHT

}//end of namespace binfilter
