/*************************************************************************
 *
 *  $RCSfile: sdpropls.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: mib $ $Date: 2000-12-18 13:25:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

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

#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include <EnumPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include <NamedBoolPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_NUMRULE_HXX
#include "numithdl.hxx"
#endif

#ifndef _XMLOFF_XMLBACKGROUNDREPEATPROPERTYHANDLER_HXX
#include "XMLBackgroundRepeatPropertyHandler.hxx"
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmlkywd.hxx>
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

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

const XMLPropertyMapEntry aXMLSDProperties[] =
{
    // stroke attributes
    { "LineStyle",      XML_NAMESPACE_DRAW, sXML_stroke,                XML_SD_TYPE_STROKE, 0 },
    { "LineDashName",   XML_NAMESPACE_DRAW, sXML_stroke_dash,           XML_TYPE_STRING, 0 },
    { "LineWidth",      XML_NAMESPACE_SVG,  sXML_stroke_width,          XML_TYPE_MEASURE, 0 },
    { "LineColor",      XML_NAMESPACE_SVG,  sXML_stroke_color,          XML_TYPE_COLOR, 0 },
    { "LineStartName",  XML_NAMESPACE_DRAW, sXML_marker_start,          XML_TYPE_STRING, 0 },
    { "LineStartWidth", XML_NAMESPACE_DRAW, sXML_marker_start_width,    XML_TYPE_MEASURE, 0 },
    { "LineStartCenter",XML_NAMESPACE_DRAW, sXML_marker_start_center,   XML_TYPE_BOOL, 0 },
    { "LineEndName",    XML_NAMESPACE_DRAW, sXML_marker_end,            XML_TYPE_STRING, 0 },
    { "LineEndWidth",   XML_NAMESPACE_DRAW, sXML_marker_end_width,      XML_TYPE_MEASURE, 0 },
    { "LineEndCenter",  XML_NAMESPACE_DRAW, sXML_marker_end_center,     XML_TYPE_BOOL, 0 },
    { "LineTransparence", XML_NAMESPACE_SVG,sXML_stroke_opacity,        XML_SD_TYPE_OPACITY, 0 },
    { "LineJoint",      XML_NAMESPACE_SVG,  sXML_stroke_linejoin,       XML_SD_TYPE_LINEJOIN, 0 },

    // fill attributes
    { "FillStyle",      XML_NAMESPACE_DRAW, sXML_fill,                  XML_SD_TYPE_FILLSTYLE, 0 },
    { "FillColor",      XML_NAMESPACE_DRAW, sXML_fill_color,            XML_TYPE_COLOR, 0 },
    { "FillGradientName",   XML_NAMESPACE_DRAW, sXML_fill_gradient_name,XML_TYPE_STRING, 0 },
    { "FillGradientStepCount",  XML_NAMESPACE_DRAW, sXML_gradient_step_count,   XML_TYPE_NUMBER, 0 },
    { "FillHatchName",      XML_NAMESPACE_DRAW, sXML_fill_hatch_name,   XML_TYPE_STRING, 0 },
    { "FillBitmapName",     XML_NAMESPACE_DRAW, sXML_fill_image_name,   XML_TYPE_STRING, 0 },
    { "FillTransparence",   XML_NAMESPACE_DRAW, sXML_transparency,      XML_TYPE_PERCENT16, 0 },
    { "FillTransparenceGradientName",   XML_NAMESPACE_DRAW, sXML_transparency_name, XML_TYPE_STRING, 0 },

    { "FillBitmapSizeX",            XML_NAMESPACE_DRAW, sXML_fill_image_width,  XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 },
    { "FillBitmapLogicalSize",      XML_NAMESPACE_DRAW, sXML_fill_image_width,  XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MULTI_ATTRIBUTE, 0 },
    { "FillBitmapSizeY",            XML_NAMESPACE_DRAW, sXML_fill_image_height, XML_SD_TYPE_FILLBITMAPSIZE|MID_FLAG_MULTI_PROPERTY, 0 },
    { "FillBitmapLogicalSize",      XML_NAMESPACE_DRAW, sXML_fill_image_height, XML_SD_TYPE_LOGICAL_SIZE|MID_FLAG_MULTI_PROPERTY|MID_FLAG_MULTI_ATTRIBUTE, 0 },
    { "FillBitmapTile",             XML_NAMESPACE_STYLE, sXML_repeat,           XML_SD_TYPE_BITMAP_TILE|MID_FLAG_MULTI_PROPERTY, CTF_BITMAP_TILE },
    { "FillBitmapStretch",          XML_NAMESPACE_STYLE, sXML_repeat,           XML_SD_TYPE_BITMAP_STRETCH|MID_FLAG_MULTI_PROPERTY, CTF_BITMAP_STRETCH },
    { "FillBitmapPositionOffsetX",  XML_NAMESPACE_DRAW, sXML_fill_image_ref_point_x,                XML_TYPE_PERCENT, 0 },
    { "FillBitmapPositionOffsetY",  XML_NAMESPACE_DRAW, sXML_fill_image_ref_point_y,                XML_TYPE_PERCENT, 0 },
    { "FillBitmapRectanglePoint",   XML_NAMESPACE_DRAW, sXML_fill_image_ref_point, XML_SD_TYPE_BITMAP_REFPOINT, 0 },
    { "FillBitmapOffsetX",          XML_NAMESPACE_DRAW, sXML_tile_repeat_offset,XML_SD_TYPE_BITMAPREPOFFSETX|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_X },
    { "FillBitmapOffsetY",          XML_NAMESPACE_DRAW, sXML_tile_repeat_offset,XML_SD_TYPE_BITMAPREPOFFSETY|MID_FLAG_MULTI_PROPERTY, CTF_REPEAT_OFFSET_Y },

    // text frame attributes
    { "TextWritingMode",    XML_NAMESPACE_FO,   sXML_writing_mode,      XML_SD_TYPE_WRITINGMODE, CTF_WRITINGMODE },

    // shadow attributes
    { "Shadow",         XML_NAMESPACE_DRAW, sXML_shadow,                XML_SD_TYPE_SHADOW, 0 },
    { "ShadowXDistance",XML_NAMESPACE_DRAW, sXML_shadow_offset_x,       XML_TYPE_MEASURE, 0 },
    { "ShadowYDistance",XML_NAMESPACE_DRAW, sXML_shadow_offset_y,       XML_TYPE_MEASURE, 0 },
    { "ShadowColor",    XML_NAMESPACE_DRAW, sXML_shadow_color,          XML_TYPE_COLOR, 0 },
    { "ShadowTransparence", XML_NAMESPACE_DRAW, sXML_shadow_transparency, XML_TYPE_PERCENT, 0 },

    // graphic attributes
    { "GraphicColorMode", XML_NAMESPACE_DRAW, sXML_color_mode,          XML_TYPE_COLOR_MODE, 0 },
    { "AdjustLuminance",  XML_NAMESPACE_DRAW, sXML_luminance,           XML_TYPE_PERCENT16, 0 },        // signed?
    { "AdjustContrast", XML_NAMESPACE_DRAW, sXML_contrast,              XML_TYPE_PERCENT16, 0 },        // signed?
    { "Gamma",          XML_NAMESPACE_DRAW, sXML_gamma,                 XML_TYPE_DOUBLE, 0 },           // signed?
    { "AdjustRed",      XML_NAMESPACE_DRAW, sXML_red,                   XML_TYPE_PERCENT16, 0 },        // signed?
    { "AdjustGreen",    XML_NAMESPACE_DRAW, sXML_green,                 XML_TYPE_PERCENT16, 0 },        // signed?
    { "AdjustBlue",     XML_NAMESPACE_DRAW, sXML_blue,                  XML_TYPE_PERCENT16, 0 },        // signed?

    // animation text attributes

    // connector attributes
    { "EdgeNode1HorzDist",  XML_NAMESPACE_DRAW, sXML_start_line_spacing_horizontal, XML_TYPE_MEASURE, 0 },
    { "EdgeNode1VertDist",  XML_NAMESPACE_DRAW, sXML_start_line_spacing_vertical,   XML_TYPE_MEASURE, 0 },
    { "EdgeNode2HorzDist",  XML_NAMESPACE_DRAW, sXML_end_line_spacing_horizontal,   XML_TYPE_MEASURE, 0 },
    { "EdgeNode2VertDist",  XML_NAMESPACE_DRAW, sXML_end_line_spacing_vertical,     XML_TYPE_MEASURE, 0 },

    // measure attributes
    { "MeasureLineDistance",        XML_NAMESPACE_DRAW, sXML_line_distance,             XML_TYPE_MEASURE, 0 },
    { "MeasureHelpLineOverhang",    XML_NAMESPACE_DRAW, sXML_guide_overhang,            XML_TYPE_MEASURE, 0 },
    { "MeasureHelpLineDistance",    XML_NAMESPACE_DRAW, sXML_guide_distance,            XML_TYPE_MEASURE, 0 },
    { "MeasureHelpLine1Length",     XML_NAMESPACE_DRAW, sXML_start_guide,               XML_TYPE_MEASURE, 0 },
    { "MeasureHelpLine2Length",     XML_NAMESPACE_DRAW, sXML_end_guide,                 XML_TYPE_MEASURE, 0 },
//  { "MeasureUnit",                XML_NAMESPACE_DRAW, sXML_unit,                      XML_TYPE_MEASURE_UNIT|MID_FLAG_MULTI_PROPERTY, 0 },
//  { "MeasureShowUnit",            XML_NAMESPACE_DRAW, sXML_unit,                      XML_TYPE_MEASURE_UNIT|MID_FLAG_MULTI_PROPERTY, 0 },
//  { "MeasureBelowReferenceEdge",  XML_NAMESPACE_DRAW, sXML_placing,                   XML_TYPE_MEASURE_PLACING, 0 },
    { "MeasureTextRotate90",        XML_NAMESPACE_DRAW, sXML_parallel,                  XML_TYPE_BOOL, 0 },


/*
    // text attributes
    { "CharColor",              XML_NAMESPACE_FO,       sXML_color,                     XML_TYPE_COLOR, 0 },
    { "CharCrossedOut",         XML_NAMESPACE_STYLE,    sXML_text_crossing_out,         XML_SD_TYPE_TEXT_CROSSEDOUT,    0},
    { "CharEscapement",         XML_NAMESPACE_STYLE,    sXML_text_position,             XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 },
    { "CharEscapementHeight",   XML_NAMESPACE_STYLE,    sXML_text_position,             XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE|MID_FLAG_MULTI_PROPERTY, 0 },
    { "CharFontName",           XML_NAMESPACE_STYLE,    sXML_font_name,                 XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTNAME },
    { "CharFontName",           XML_NAMESPACE_FO,       sXML_font_family,               XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME },
    { "CharFontStyleName",      XML_NAMESPACE_STYLE,    sXML_font_style_name,           XML_TYPE_STRING, CTF_FONTSTYLENAME },
    { "CharFontFamily",         XML_NAMESPACE_STYLE,    sXML_font_family_generic,       XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY },
    { "CharFontPitch",          XML_NAMESPACE_STYLE,    sXML_font_pitch,                XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH },
    { "CharFontCharSet",        XML_NAMESPACE_STYLE,    sXML_font_charset,              XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET },
    { "CharHeight",             XML_NAMESPACE_FO,       sXML_font_size,                 XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT },
    { "CharLocale",             XML_NAMESPACE_FO,       sXML_language,                  XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharLocale",             XML_NAMESPACE_FO,       sXML_country,                   XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharPosture",            XML_NAMESPACE_FO,       sXML_font_style,                XML_TYPE_TEXT_POSTURE, 0 },
    { "CharShadowed",           XML_NAMESPACE_FO,       sXML_text_shadow,               XML_TYPE_TEXT_SHADOWED, 0 },
    { "CharUnderline",          XML_NAMESPACE_STYLE,    sXML_text_underline,            XML_TYPE_TEXT_UNDERLINE, CTF_UNDERLINE },
    { "CharWeight",             XML_NAMESPACE_FO,       sXML_font_weight,               XML_TYPE_TEXT_WEIGHT, 0 },
// ??   { "WordMode",           XML_NAMESPACE_STYLE,    sXML_decorate_words_only,XML_TYPE_BOOL, 0 },
// ??   { "CharAutoKerning",    XML_NAMESPACE_STYLE,    sXML_letter_kerning,        XML_TYPE_BOOL, 0 },
// ??   { "CharKerning",        XML_NAMESPACE_FO,       sXML_letter_spacing,        XML_TYPE_TEXT_KERNING, 0 },

    { "CharFontNameAsian",      XML_NAMESPACE_STYLE,    sXML_font_name_asian,           XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CJK },
    { "CharFontNameAsian",      XML_NAMESPACE_STYLE,    sXML_font_family_asian,         XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CJK },
    { "CharFontStyleNameAsian", XML_NAMESPACE_STYLE,    sXML_font_style_name_asian,     XML_TYPE_STRING, CTF_FONTSTYLENAME_CJK },
    { "CharFontFamilyAsian",    XML_NAMESPACE_STYLE,    sXML_font_family_generic_asian, XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CJK },
    { "CharFontPitchAsian",     XML_NAMESPACE_STYLE,    sXML_font_pitch_asian,          XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CJK },
    { "CharFontCharSetAsian",   XML_NAMESPACE_STYLE,    sXML_font_charset_asian,        XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CJK },
    { "CharHeightAsian",        XML_NAMESPACE_STYLE,    sXML_font_size_asian,           XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CJK },
    { "CharLocaleAsian",        XML_NAMESPACE_STYLE,    sXML_language_asian,            XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharLocaleAsian",        XML_NAMESPACE_STYLE,    sXML_country_asian,             XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharPostureAsian",       XML_NAMESPACE_STYLE,    sXML_font_style_asian,          XML_TYPE_TEXT_POSTURE, 0 },
    { "CharWeightAsian",        XML_NAMESPACE_STYLE,    sXML_font_weight_asian,         XML_TYPE_TEXT_WEIGHT, 0 },

    { "CharFontNameComplex",    XML_NAMESPACE_STYLE,    sXML_font_name_complex,         XML_TYPE_STRING|MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_FONTNAME_CTL },
    { "CharFontNameComplex",    XML_NAMESPACE_STYLE,    sXML_font_family_complex,       XML_TYPE_TEXT_FONTFAMILYNAME, CTF_FONTFAMILYNAME_CTL },
    { "CharFontStyleNameComplex",XML_NAMESPACE_STYLE,   sXML_font_style_name_complex,   XML_TYPE_STRING, CTF_FONTSTYLENAME_CTL },
    { "CharFontFamilyComplex",  XML_NAMESPACE_STYLE,    sXML_font_family_generic_complex,XML_TYPE_TEXT_FONTFAMILY, CTF_FONTFAMILY_CTL },
    { "CharFontPitchComplex",   XML_NAMESPACE_STYLE,    sXML_font_pitch_complex,        XML_TYPE_TEXT_FONTPITCH, CTF_FONTPITCH_CTL },
    { "CharFontCharSetComplex", XML_NAMESPACE_STYLE,    sXML_font_charset_complex,      XML_TYPE_TEXT_FONTENCODING, CTF_FONTCHARSET_CTL },
    { "CharHeightComplex",      XML_NAMESPACE_STYLE,    sXML_font_size_complex,         XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_CTL },
    { "CharLocaleComplex",      XML_NAMESPACE_STYLE,    sXML_language_complex,          XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharLocaleComplex",      XML_NAMESPACE_STYLE,    sXML_country_complex,           XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 },
    { "CharPostureComplex",     XML_NAMESPACE_STYLE,    sXML_font_style_complex,        XML_TYPE_TEXT_POSTURE, 0 },
    { "CharWeightComplex",      XML_NAMESPACE_STYLE,    sXML_font_weight_complex,       XML_TYPE_TEXT_WEIGHT, 0 },

    { "ParaLineSpacing",        XML_NAMESPACE_FO,       sXML_line_height,               XML_TYPE_LINE_SPACE_FIXED, 0 },
    { "ParaLineSpacing",        XML_NAMESPACE_STYLE,    sXML_line_height_at_least,      XML_TYPE_LINE_SPACE_MINIMUM, 0 },
    { "ParaLineSpacing",        XML_NAMESPACE_STYLE,    sXML_line_spacing,              XML_TYPE_LINE_SPACE_DISTANCE, 0 },
    { "ParaAdjust",             XML_NAMESPACE_FO,       sXML_text_align,                XML_TYPE_TEXT_ADJUST, 0 },
    { "ParaLastLineAdjust",     XML_NAMESPACE_STYLE,    sXML_text_align_last,           XML_TYPE_TEXT_ADJUSTLAST, 0 },
    { "ParaIsHyphenation",      XML_NAMESPACE_FO,       sXML_hyphenate,                 XML_TYPE_BOOL, 0 },
    { "ParaLeftMargin",         XML_NAMESPACE_FO,       sXML_margin_left,               XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARALEFTMARGIN },
    { "ParaRightMargin",        XML_NAMESPACE_FO,       sXML_margin_right,              XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARARIGHTMARGIN },
    { "ParaFirstLineIndent",    XML_NAMESPACE_FO,       sXML_text_indent,               XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARAFIRSTLINE },
    { "ParaLastLineAdjust",     XML_NAMESPACE_FO,       sXML_text_align_last,           XML_TYPE_TEXT_ADJUSTLAST, 0 },

    { "ParaTopMargin",          XML_NAMESPACE_FO,       sXML_margin_top,                XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARATOPMARGIN },
    { "ParaBottomMargin",       XML_NAMESPACE_FO,       sXML_margin_bottom,             XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARABOTTOMMARGIN },
    { "ParaIsCharacterDistance",XML_NAMESPACE_STYLE,    sXML_text_autospace,            XML_TYPE_TEXT_AUTOSPACE, 0 },
    { "ParaIsHangingPunctuation",XML_NAMESPACE_STYLE,   sXML_punctuation_wrap,          XML_TYPE_TEXT_PUNCTUATION_WRAP, 0 },
    { "ParaIsForbiddenRules",   XML_NAMESPACE_STYLE,    sXML_line_break,                XML_TYPE_TEXT_LINE_BREAK, 0 },
    { "IsNumbering",            XML_NAMESPACE_TEXT,     sXML_enable_numbering,          XML_TYPE_BOOL, 0 },
*/
    { "NumberingRules",         XML_NAMESPACE_TEXT,     sXML_list_style,                XML_SD_TYPE_NUMBULLET|MID_FLAG_ELEMENT_ITEM, CTF_NUMBERINGRULES },
    { "NumberingRules",         XML_NAMESPACE_TEXT,     sXML_list_style_name,           XML_TYPE_STRING, CTF_NUMBERINGRULES_NAME },

    // 3D geometry attributes
    { "D3DHorizontalSegments",          XML_NAMESPACE_DR3D, sXML_horizontal_segments,   XML_TYPE_NUMBER, 0 },
    { "D3DVerticalSegments",            XML_NAMESPACE_DR3D, sXML_vertical_segments,     XML_TYPE_NUMBER, 0 },
    { "D3DPercentDiagonal",             XML_NAMESPACE_DR3D, sXML_edge_rounding,         XML_TYPE_PERCENT, 0 },
    { "D3DBackscale",                   XML_NAMESPACE_DR3D, sXML_back_scale,            XML_TYPE_PERCENT, 0 },
    { "D3DEndAngle",                    XML_NAMESPACE_DR3D, sXML_end_angle,             XML_TYPE_NUMBER, 0 },
    { "D3DDepth",                       XML_NAMESPACE_DR3D, sXML_depth,                 XML_TYPE_MEASURE, 0 },
    { "D3DDoubleSided",                 XML_NAMESPACE_DR3D, sXML_backface_culling,      XML_SD_TYPE_BACKFACE_CULLING, 0 },

    // 3D lighting attributes
    { "D3DNormalsKind",                 XML_NAMESPACE_DR3D, sXML_normals_kind,          XML_SD_TYPE_NORMALS_KIND, 0 },
    { "D3DNormalsInvert",               XML_NAMESPACE_DR3D, sXML_normals_direction,     XML_SD_TYPE_NORMALS_DIRECTION, 0 },

    // 3D texture attributes
    { "D3DTextureProjectionX",          XML_NAMESPACE_DR3D, sXML_tex_generation_mode_x, XML_SD_TYPE_TEX_GENERATION_MODE_X, 0 },
    { "D3DTextureProjectionY",          XML_NAMESPACE_DR3D, sXML_tex_generation_mode_y, XML_SD_TYPE_TEX_GENERATION_MODE_Y, 0 },
    { "D3DTextureKind",                 XML_NAMESPACE_DR3D, sXML_tex_kind,              XML_SD_TYPE_TEX_KIND, 0 },
    { "D3DTextureMode",                 XML_NAMESPACE_DR3D, sXML_tex_mode,              XML_SD_TYPE_TEX_MODE, 0 },
    { "D3DTextureFilter",               XML_NAMESPACE_DR3D, sXML_tex_filter,            XML_TYPE_BOOL, 0 },

    // 3D material attributes
    { "D3DMaterialColor",               XML_NAMESPACE_DR3D, sXML_diffuse_color,         XML_TYPE_COLOR, 0 },
    { "D3DMaterialEmission",            XML_NAMESPACE_DR3D, sXML_emissive_color,        XML_TYPE_COLOR, 0 },
    { "D3DMaterialSpecular",            XML_NAMESPACE_DR3D, sXML_specular_color,        XML_TYPE_COLOR, 0 },
    { "D3DMaterialSpecularIntensity",   XML_NAMESPACE_DR3D, sXML_shininess,             XML_TYPE_PERCENT, 0 },

    // 3D shadow attributes
    { "D3DShadow3D",                    XML_NAMESPACE_DR3D, sXML_shadow,                XML_TYPE_BOOL, 0 },

    { 0L }
};

//////////////////////////////////////////////////////////////////////////////
// entry list for presentation page properties

const XMLPropertyMapEntry aXMLSDPresPageProps[] =
{
    { "Change", XML_NAMESPACE_PRESENTATION, "transition-type", XML_SD_TYPE_PRESPAGE_TYPE, 0 },
    { "Effect", XML_NAMESPACE_PRESENTATION, "transition-style", XML_SD_TYPE_PRESPAGE_STYLE, 0 },
    { "Speed", XML_NAMESPACE_PRESENTATION, "transition-speed", XML_SD_TYPE_PRESPAGE_SPEED, 0 },
    { "Duration", XML_NAMESPACE_PRESENTATION, "duration", XML_SD_TYPE_PRESPAGE_DURATION, 0 },
// still missing:
//  { "", XML_NAMESPACE_PRESENTATION, "visibility", XML_SD_TYPE_PRESPAGE_VISIBILITY, 0 },
//  { "", XML_NAMESPACE_PRESENTATION, "sound", XML_SD_TYPE_PRESPAGE_SOUND, 0 },

    { "FillStyle",              XML_NAMESPACE_DRAW, sXML_fill,                  XML_SD_TYPE_FILLSTYLE, 0 },
    { "FillColor",              XML_NAMESPACE_DRAW, sXML_fill_color,            XML_TYPE_COLOR, 0 },
    { "FillGradientName",       XML_NAMESPACE_DRAW, sXML_fill_gradient_name,    XML_TYPE_STRING, 0 },
    { "FillGradientStepCount",  XML_NAMESPACE_DRAW, sXML_gradient_step_count,   XML_TYPE_NUMBER, 0 },
    { "FillHatchName",          XML_NAMESPACE_DRAW, sXML_fill_hatch_name,       XML_TYPE_STRING, 0 },
    { "FillBitmapName",         XML_NAMESPACE_DRAW, sXML_fill_image_name,       XML_TYPE_STRING, 0 },
    { "FillTransparenceName",   XML_NAMESPACE_DRAW, sXML_transparency_name,     XML_TYPE_STRING, 0 },

    { 0L }
};

//////////////////////////////////////////////////////////////////////////////
// implementation of factory for own graphic properties

SvXMLEnumMapEntry aXML_LineStyle_EnumMap[] =
{
    { sXML_none, drawing::LineStyle_NONE },
    { sXML_solid, drawing::LineStyle_SOLID },
    { sXML_dash, drawing::LineStyle_DASH },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_LineJoint_EnumMap[] =
{
    { sXML_none,    drawing::LineJoint_NONE },
    { sXML_miter,   drawing::LineJoint_MITER },
    { sXML_round,   drawing::LineJoint_ROUND },
    { sXML_bevel,   drawing::LineJoint_BEVEL },
    { sXML_middle,  drawing::LineJoint_MIDDLE },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_FillStyle_EnumMap[] =
{
    { sXML_none,    drawing::FillStyle_NONE },
    { sXML_solid,   drawing::FillStyle_SOLID },
    { sXML_bitmap,  drawing::FillStyle_BITMAP },
    { sXML_gradient,drawing::FillStyle_GRADIENT },
    { sXML_hatch,   drawing::FillStyle_HATCH },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_PresChange_EnumMap[] =
{
    { sXML_manual,  0 },
    { sXML_automatic, 1 },
    { sXML_semi_automatic, 2 },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_TransSpeed_EnumMap[] =
{
    { sXML_slow,      presentation::AnimationSpeed_FAST },
    { sXML_medium,    presentation::AnimationSpeed_MEDIUM },
    { sXML_fast,      presentation::AnimationSpeed_FAST },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_FadeEffect_EnumMap[] =
{
    { sXML_none,                presentation::FadeEffect_NONE },
    { sXML_fade_from_left,      presentation::FadeEffect_FADE_FROM_LEFT },
    { sXML_fade_from_top,       presentation::FadeEffect_FADE_FROM_TOP },
    { sXML_fade_from_right,     presentation::FadeEffect_FADE_FROM_RIGHT },
    { sXML_fade_from_bottom,    presentation::FadeEffect_FADE_FROM_BOTTOM },
    { sXML_fade_to_center,      presentation::FadeEffect_FADE_TO_CENTER },
    { sXML_fade_from_center,    presentation::FadeEffect_FADE_FROM_CENTER },
    { sXML_move_from_left,      presentation::FadeEffect_MOVE_FROM_LEFT },
    { sXML_move_from_top,       presentation::FadeEffect_MOVE_FROM_TOP },
    { sXML_move_from_right,     presentation::FadeEffect_MOVE_FROM_RIGHT },
    { sXML_move_from_bottom,    presentation::FadeEffect_MOVE_FROM_BOTTOM },
    { sXML_roll_from_left,      presentation::FadeEffect_ROLL_FROM_LEFT },
    { sXML_roll_from_right,     presentation::FadeEffect_ROLL_FROM_RIGHT },
    { sXML_roll_from_bottom,    presentation::FadeEffect_ROLL_FROM_BOTTOM },
    { sXML_vertical_stripes,    presentation::FadeEffect_VERTICAL_STRIPES },
    { sXML_horizontal_stripes,  presentation::FadeEffect_HORIZONTAL_STRIPES },
    { sXML_clockwise,           presentation::FadeEffect_CLOCKWISE },
    { sXML_counterclockwise,    presentation::FadeEffect_COUNTERCLOCKWISE },
    { sXML_fade_from_upperleft, presentation::FadeEffect_FADE_FROM_UPPERLEFT },
    { sXML_fade_from_upperright,presentation::FadeEffect_FADE_FROM_UPPERRIGHT },
    { sXML_fade_from_lowerleft, presentation::FadeEffect_FADE_FROM_LOWERLEFT },
    { sXML_fade_from_lowerright,presentation::FadeEffect_FADE_FROM_LOWERRIGHT },
    { sXML_close_vertical,      presentation::FadeEffect_CLOSE_VERTICAL },
    { sXML_close_horizontal,    presentation::FadeEffect_CLOSE_HORIZONTAL },
    { sXML_open_vertical,       presentation::FadeEffect_OPEN_VERTICAL },
    { sXML_open_horizontal,     presentation::FadeEffect_OPEN_HORIZONTAL },
    { sXML_spiralin_left,       presentation::FadeEffect_SPIRALIN_LEFT },
    { sXML_spiralin_right,      presentation::FadeEffect_SPIRALIN_RIGHT },
    { sXML_spiralout_left,      presentation::FadeEffect_SPIRALOUT_LEFT },
    { sXML_spiralout_right,     presentation::FadeEffect_SPIRALOUT_RIGHT },
    { sXML_dissolve,            presentation::FadeEffect_DISSOLVE },
    { sXML_wavyline_from_left,  presentation::FadeEffect_WAVYLINE_FROM_LEFT },
    { sXML_wavyline_from_top,   presentation::FadeEffect_WAVYLINE_FROM_TOP },
    { sXML_wavyline_from_right, presentation::FadeEffect_WAVYLINE_FROM_RIGHT },
    { sXML_wavyline_from_bottom,presentation::FadeEffect_WAVYLINE_FROM_BOTTOM },
    { sXML_random,              presentation::FadeEffect_RANDOM },
    { sXML_stretch_from_left,   presentation::FadeEffect_STRETCH_FROM_LEFT },
    { sXML_stretch_from_top,    presentation::FadeEffect_STRETCH_FROM_TOP },
    { sXML_stretch_from_right,  presentation::FadeEffect_STRETCH_FROM_RIGHT },
    { sXML_stretch_from_bottom, presentation::FadeEffect_STRETCH_FROM_BOTTOM },
    { sXML_vertical_lines,      presentation::FadeEffect_VERTICAL_LINES },
    { sXML_horizontal_lines,    presentation::FadeEffect_HORIZONTAL_LINES },
    { NULL, 0 }
};

SvXMLEnumMapEntry   aXML_ConnectionKind_EnumMap[] =
{
    { sXML_standard,    drawing::ConnectorType_STANDARD },
    { sXML_curve,       drawing::ConnectorType_CURVE },
    { sXML_line,        drawing::ConnectorType_LINE },
    { sXML_lines,       drawing::ConnectorType_LINES },
    { NULL, 0 }
};

//////////////////////////////////////////////////////////////////////////////
// 3D EnumMaps

SvXMLEnumMapEntry  aXML_NormalsKind_EnumMap[] =
{
    { sXML_object,      drawing::NormalsKind_SPECIFIC },
    { sXML_flat,        drawing::NormalsKind_FLAT },
    { sXML_sphere,      drawing::NormalsKind_SPHERE },
    { NULL, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationX_EnumMap[] =
{
    { sXML_object,      drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { sXML_parallel,    drawing::TextureProjectionMode_PARALLEL },
    { sXML_sphere,      drawing::TextureProjectionMode_SPHERE },
    { NULL, 0 }
};

SvXMLEnumMapEntry  aXML_TexGenerationY_EnumMap[] =
{
    { sXML_object,      drawing::TextureProjectionMode_OBJECTSPECIFIC },
    { sXML_parallel,    drawing::TextureProjectionMode_PARALLEL },
    { sXML_sphere,      drawing::TextureProjectionMode_SPHERE },
    { NULL, 0 }
};

SvXMLEnumMapEntry  aXML_TexKind_EnumMap[] =
{
    { sXML_luminance,   drawing::TextureKind_LUMINANCE },
//    { sXML_intensity, drawing::TextureKind_INTENSITY },
    { sXML_color,       drawing::TextureKind_COLOR },
    { NULL, 0 }
};

SvXMLEnumMapEntry  aXML_TexMode_EnumMap[] =
{
    { sXML_replace,     drawing::TextureMode_REPLACE },
    { sXML_modulate,    drawing::TextureMode_MODULATE },
    { sXML_blend,       drawing::TextureMode_BLEND },
    { NULL, 0 }
};

SvXMLEnumMapEntry aXML_RefPoint_EnumMap[] =
{
    { sXML_top_left,    drawing::RectanglePoint_LEFT_TOP },
    { sXML_top,         drawing::RectanglePoint_MIDDLE_TOP },
    { sXML_top_right,   drawing::RectanglePoint_RIGHT_TOP },
    { sXML_left,        drawing::RectanglePoint_LEFT_MIDDLE },
    { sXML_center,      drawing::RectanglePoint_MIDDLE_MIDDLE },
    { sXML_right,       drawing::RectanglePoint_RIGHT_MIDDLE },
    { sXML_bottom_left, drawing::RectanglePoint_LEFT_BOTTOM },
    { sXML_bottom,      drawing::RectanglePoint_MIDDLE_BOTTOM },
    { sXML_bottom_right,drawing::RectanglePoint_RIGHT_BOTTOM },
    { NULL, 0 }
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
            case XML_SD_TYPE_DASHARRAY :
            {
                pHdl = new XMLDashArrayPropertyHdl();
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
                const OUString aTrueStr( OUString::createFromAscii(sXML_visible) );
                const OUString aFalseStr( OUString::createFromAscii(sXML_hidden) );
                pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
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
                const OUString aTrueStr( OUString::createFromAscii(sXML_crossedout_single) );
                const OUString aFalseStr( OUString::createFromAscii(sXML_none) );
                pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
                break;
            }
            case XML_SD_TYPE_OPACITY :
            {
                pHdl = new XMLOpacityPropertyHdl();
                break;
            }
            case XML_SD_TYPE_WRITINGMODE :
            {
                const OUString aTrueStr( OUString::createFromAscii(sXML_tb_rl) );
                const OUString aFalseStr( OUString::createFromAscii(sXML_lr_tb) );
                pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
                break;
            }
// still missing:
//          case XML_SD_TYPE_PRESPAGE_VISIBILITY :
//          {
//              break;
//          }
//          case XML_SD_TYPE_PRESPAGE_SOUND :
//          {
//              break;
//          }

            //////////////////////////////////////////////////////////////////
            // 3D Properties

            case XML_SD_TYPE_BACKFACE_CULLING:
            {
                const OUString aTrueStr( OUString::createFromAscii(sXML_enabled) );
                const OUString aFalseStr( OUString::createFromAscii(sXML_disabled) );
                pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
                break;
            }

            case XML_SD_TYPE_NORMALS_KIND:
            {
                pHdl = new XMLEnumPropertyHdl( aXML_NormalsKind_EnumMap, ::getCppuType((const drawing::NormalsKind*)0) );
                break;
            }
            case XML_SD_TYPE_NORMALS_DIRECTION:
            {
                const OUString aTrueStr( OUString::createFromAscii(sXML_normal) );
                const OUString aFalseStr( OUString::createFromAscii(sXML_inverse) );
                pHdl = new XMLNamedBoolPropertyHdl( aTrueStr, aFalseStr );
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
            case XML_SD_TYPE_BITMAP_TILE:
            case XML_SD_TYPE_BITMAP_STRETCH:
            {
                pHdl = new XMLBackgroundRepeatPropertyHandler( nType == XML_SD_TYPE_BITMAP_TILE );
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

XMLShapeExportPropertyMapper::XMLShapeExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, XMLTextListAutoStylePool *pListAutoPool, SvXMLExport& rExport ) :
        SvXMLExportPropertyMapper( rMapper ),
        mpListAutoPool( pListAutoPool ),
        mrExport( rExport ),
        maNumRuleExp( rExport ),
        msCDATA( rtl::OUString::createFromAscii( sXML_CDATA )),
        msTrue( rtl::OUString::createFromAscii( sXML_true )),
        msFalse( rtl::OUString::createFromAscii( sXML_false )),
        mbIsInAutoStyles( sal_False )
{
}

XMLShapeExportPropertyMapper::~XMLShapeExportPropertyMapper()
{
}

void XMLShapeExportPropertyMapper::ContextFilter(
    std::vector< XMLPropertyState >& rProperties,
    uno::Reference< beans::XPropertySet > rPropSet ) const
{
    XMLPropertyState* pBitmapTile = NULL;
    XMLPropertyState* pBitmapStretch = NULL;
    XMLPropertyState* pRepeatOffsetX = NULL;
    XMLPropertyState* pRepeatOffsetY = NULL;

    // filter properties
    for( std::vector< XMLPropertyState >::iterator property = rProperties.begin();
         property != rProperties.end();
         property++ )
    {
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
            case CTF_NUMBERINGRULES_NAME:
                {
                    if( mbIsInAutoStyles )
                    {
                        uno::Reference< container::XIndexReplace > xNumRule;
                        if( property->maValue >>= xNumRule )
                        {
                            const OUString sName = ((XMLTextListAutoStylePool*)&mrExport.GetTextParagraphExport()->GetListAutoStylePool())->Add( xNumRule );
                            property->maValue <<= sName;
                        }
                    }
                    else
                    {
                        property->mnIndex = -1;
                    }
                }
                break;
            case CTF_WRITINGMODE:
                {
                    sal_Bool bWritingMode;
                    if( property->maValue >>= bWritingMode )
                    {
                        if( !bWritingMode )
                            property->mnIndex = -1;
                    }
                }
                break;
            case CTF_BITMAP_TILE:
                pBitmapTile = property;
                break;

            case CTF_BITMAP_STRETCH:
                pBitmapStretch = property;
                break;

            case CTF_REPEAT_OFFSET_X:
                pRepeatOffsetX = property;
                break;

            case CTF_REPEAT_OFFSET_Y:
                pRepeatOffsetY = property;
                break;
        }
    }

    if( pBitmapTile && pBitmapStretch )
    {
        sal_Bool bTile;
        if( ( pBitmapTile->maValue >>= bTile ) && bTile )
            pBitmapStretch->mnIndex = -1;
        else
            pBitmapTile->mnIndex = -1;
    }

    if( pRepeatOffsetX && pRepeatOffsetY )
    {
        sal_Int32 nOffset;
        if( ( pRepeatOffsetX->maValue >>= nOffset ) && ( nOffset == 0 ) )
            pRepeatOffsetX->mnIndex = -1;
        else
            pRepeatOffsetY->mnIndex = -1;
    }

    SvXMLExportPropertyMapper::ContextFilter(rProperties, rPropSet);
}

void XMLShapeExportPropertyMapper::handleElementItem(
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > & rHandler,
    const XMLPropertyState& rProperty, const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap, sal_uInt16 nFlags,
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
            SvXMLExportPropertyMapper::handleElementItem( rHandler, rProperty, rUnitConverter, rNamespaceMap, nFlags, pProperties, nIdx );
            break;
    }
}
