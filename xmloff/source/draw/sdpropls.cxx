/*************************************************************************
 *
 *  $RCSfile: sdpropls.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pw $ $Date: 2000-10-26 14:31:15 $
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

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#ifndef _XMLOFF_ENUMPROPERTYHANDLER_HXX
#include <EnumPropertyHdl.hxx>
#endif

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include <NamedBoolPropertyHdl.hxx>
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

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmlnmspe.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

#define CTF_CHARHEIGHT               1
#define CTF_CHARHEIGHT_REL           2
#define CTF_PARALEFTMARGIN           3
#define CTF_PARALEFTMARGIN_REL       4
#define CTF_PARARIGHTMARGIN          5
#define CTF_PARARIGHTMARGIN_REL      6
#define CTF_PARAFIRSTLINE            7
#define CTF_PARAFIRSTLINE_REL        8
#define CTF_PARATOPMARGIN            9
#define CTF_PARATOPMARGIN_REL       10
#define CTF_PARABOTTOMMARGIN        11
#define CTF_PARABOTTOMMARGIN_REL    12

const XMLPropertyMapEntry aXMLSDProperties[] =
{
    // stroke attributes
    { "LineStyle",      XML_NAMESPACE_DRAW, sXML_stroke, XML_SD_TYPE_STROKE, 0 },
    { "LineDashName",   XML_NAMESPACE_DRAW, sXML_stroke_dasharray,      XML_TYPE_STRING, 0 },
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
    { "FillTransparenceName",   XML_NAMESPACE_DRAW, sXML_transparency_name, XML_TYPE_STRING, 0 },

    // text frame attributes

    // shadow attributes
    { "Shadow",         XML_NAMESPACE_DRAW, sXML_shadow,                XML_SD_TYPE_SHADOW, 0 },
    { "ShadowXDistance",XML_NAMESPACE_DRAW, sXML_shadow_offset_x,       XML_TYPE_MEASURE, 0 },
    { "ShadowYDistance",XML_NAMESPACE_DRAW, sXML_shadow_offset_y,       XML_TYPE_MEASURE, 0 },
    { "ShadowColor",    XML_NAMESPACE_DRAW, sXML_shadow_color,          XML_TYPE_COLOR, 0 },
    { "ShadowTransparence", XML_NAMESPACE_DRAW, sXML_shadow_transparency, XML_TYPE_PERCENT, 0 },

    // graphic attributes
    { "GraphicColorMode", XML_NAMESPACE_DRAW, sXML_color_mode,          XML_SD_TYPE_COLORMODE, 0 },
    { "AdjustLuminance",  XML_NAMESPACE_DRAW, sXML_luminance,           XML_TYPE_PERCENT, 0 },      // signed?
    { "AdjustContrast", XML_NAMESPACE_DRAW, sXML_contrast,              XML_TYPE_PERCENT, 0 },      // signed?
    { "Gamma",          XML_NAMESPACE_DRAW, sXML_gamma,                 XML_TYPE_DOUBLE, 0 },       // signed?
    { "AdjustRed",      XML_NAMESPACE_DRAW, sXML_red,                   XML_TYPE_PERCENT, 0 },      // signed?
    { "AdjustGreen",    XML_NAMESPACE_DRAW, sXML_green,                 XML_TYPE_PERCENT, 0 },      // signed?
    { "AdjustBlue",     XML_NAMESPACE_DRAW, sXML_blue,                  XML_TYPE_PERCENT, 0 },      // signed?

    // animation text attributes

    // text attributes
    { "CharColor",      XML_NAMESPACE_FO,       sXML_color,                 XML_TYPE_COLOR, 0 },
    { "CharCrossedOut", XML_NAMESPACE_STYLE,    sXML_text_crossing_out,     XML_SD_TYPE_TEXT_CROSSEDOUT,    0},
//  { "CharEscapement",      XML_NAMESPACE_STYLE, sXML_text_position,   XML_TYPE_TEXT_ESCAPEMENT|MID_FLAG_MERGE_ATTRIBUTE, 0 }, BugID #76842#
//  { "CharEscapementHeight", XML_NAMESPACE_STYLE, sXML_text_position,  XML_TYPE_TEXT_ESCAPEMENT_HEIGHT|MID_FLAG_MERGE_ATTRIBUTE, 0 },
// ??   { "CharContoured",  XML_NAMESPACE_STYLE,    sXML_text_outline,      XML_TYPE_BOOL,  0 },
/// { "CharFontName",   XML_NAMESPACE_FO,       sXML_font_family,       XML_TYPE_TEXT_FONTFAMILYNAME, 0 },
    { "CharFontStyleName",XML_NAMESPACE_STYLE,  sXML_font_style_name,   XML_TYPE_STRING, 0 },
    { "CharFontFamily", XML_NAMESPACE_STYLE,    sXML_font_family_generic,XML_TYPE_TEXT_FONTFAMILY, 0 },
// ??   { "CharFontPitch",  XML_NAMESPACE_STYLE,    sXML_font_pitch,            XML_TYPE_TEXT_FONTPITCH, 0 },
    { "CharFontCharSet",    XML_NAMESPACE_STYLE,    sXML_font_charset,      XML_TYPE_TEXT_FONTENCODING, 0 },
    { "CharHeight",       XML_NAMESPACE_FO, sXML_font_size,         XML_TYPE_CHAR_HEIGHT|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT },
// ??   { "CharPropFontHeight",XML_NAMESPACE_FO,    sXML_font_size,         XML_TYPE_CHAR_HEIGHT_PROP|MID_FLAG_MULTI_PROPERTY, CTF_CHARHEIGHT_REL },
// ??   { "CharKerning",        XML_NAMESPACE_FO,       sXML_letter_spacing,        XML_TYPE_TEXT_KERNING, 0 },
//  { "CharLocale",     XML_NAMESPACE_FO,       sXML_language,          XML_TYPE_CHAR_LANGUAGE|MID_FLAG_MERGE_PROPERTY, 0 }, not supported yet
//  { "CharLocale",     XML_NAMESPACE_FO,       sXML_country,           XML_TYPE_CHAR_COUNTRY|MID_FLAG_MERGE_PROPERTY, 0 }, not supported yet
    { "CharPosture",        XML_NAMESPACE_FO,       sXML_font_style,            XML_TYPE_TEXT_POSTURE, 0 },
/// { "CharShadowed",   XML_NAMESPACE_FO,       sXML_text_shadow,       XML_TYPE_TEXT_SHADOWED, 0 },
    { "CharUnderline",  XML_NAMESPACE_STYLE,    sXML_text_underline,        XML_TYPE_TEXT_UNDERLINE, 0 },
    { "CharWeight",     XML_NAMESPACE_FO,       sXML_font_weight,       XML_TYPE_TEXT_WEIGHT, 0 },
// ??   { "WordMode",       XML_NAMESPACE_STYLE,    sXML_decorate_words_only,XML_TYPE_BOOL, 0 },
// ??   { "CharAutoKerning",    XML_NAMESPACE_STYLE,    sXML_letter_kerning,        XML_TYPE_BOOL, 0 },
    { "ParaLineSpacing",        XML_NAMESPACE_FO,       sXML_line_height,           XML_TYPE_LINE_SPACE_FIXED, 0 },
    { "ParaLineSpacing",        XML_NAMESPACE_STYLE,    sXML_line_height_at_least,  XML_TYPE_LINE_SPACE_MINIMUM, 0 },
    { "ParaLineSpacing",        XML_NAMESPACE_STYLE,    sXML_line_spacing,          XML_TYPE_LINE_SPACE_DISTANCE, 0 },
    { "ParaAdjust",         XML_NAMESPACE_FO,       sXML_text_align,            XML_TYPE_TEXT_ADJUST, 0 },
    { "ParaLastLineAdjust", XML_NAMESPACE_STYLE,    sXML_text_align_last,   XML_TYPE_TEXT_ADJUSTLAST, 0 },
// ??   { "ParaExpandSingleWord",XML_NAMESPACE_STYLE,   sXML_justify_single_word,XML_TYPE_BOOL, 0 },
    { "ParaIsHyphenation",  XML_NAMESPACE_FO,       sXML_hyphenate,         XML_TYPE_BOOL, 0 },
// ??   { "ParaHyphenationMaxLeadingChars", XML_NAMESPACE_FO, sXML_hyphenation_remain_char_count, XML_TYPE_NUMBER, 0 },
// ??   { "ParaHyphenationMaxTrailingChars",XML_NAMESPACE_FO, sXML_hyphenation_push_char_count, XML_TYPE_NUMBER, 0 },
// ??   { "ParaHyphenationMaxHyphens",  XML_NAMESPACE_FO, sXML_hyphenation_ladder_count, XML_TYPE_NUMBER_NONE, 0 },
    { "ParaLeftMargin",         XML_NAMESPACE_FO,   sXML_margin_left,       XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARALEFTMARGIN },
// ??   { "ParaLeftMarginRelative", XML_NAMESPACE_FO,   sXML_margin_left,       XML_TYPE_PERCENT, CTF_PARALEFTMARGIN_REL },
    { "ParaRightMargin",            XML_NAMESPACE_FO,   sXML_margin_right,      XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARARIGHTMARGIN },
// ??   { "ParaRightMarginRelative",    XML_NAMESPACE_FO,   sXML_margin_right,      XML_TYPE_PERCENT, CTF_PARARIGHTMARGIN_REL },
// ??   { "ParaFirstLineIndent",        XML_NAMESPACE_FO,   sXML_text_indent,       XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARAFIRSTLINE },
// ??   { "ParaFirstLineIndentRelative",    XML_NAMESPACE_FO,   sXML_text_indent,   XML_TYPE_PERCENT, CTF_PARAFIRSTLINE_REL },
// ??   { "ParaLastLineAdjust", XML_NAMESPACE_STYLE, sXML_auto_text_indent,     XML_TYPE_BOOL, 0 },

    { "ParaTopMargin",          XML_NAMESPACE_FO,   sXML_margin_top,            XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARATOPMARGIN },
// ??   { "ParaTopMarginRelative",  XML_NAMESPACE_FO,   sXML_margin_top,            XML_TYPE_PERCENT, CTF_PARATOPMARGIN_REL },
    { "ParaBottomMargin",       XML_NAMESPACE_FO,   sXML_margin_bottom,     XML_TYPE_MEASURE|MID_FLAG_MULTI_PROPERTY, CTF_PARABOTTOMMARGIN },
// ??   { "ParaBottomMarginRelative",XML_NAMESPACE_FO,  sXML_margin_bottom,     XML_TYPE_PERCENT, CTF_PARABOTTOMMARGIN_REL },
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

SvXMLEnumMapEntry aXML_ColorMode_EnumMap[] =
{
    { sXML_greyscale, drawing::ColorMode_GREYS },
    { sXML_mono,      drawing::ColorMode_MONO },
    { sXML_watermark, drawing::ColorMode_WATERMARK },
    { sXML_standard,  drawing::ColorMode_STANDARD },
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
            case XML_SD_TYPE_COLORMODE :
            {
                pHdl = new XMLEnumPropertyHdl( aXML_ColorMode_EnumMap, ::getCppuType((const drawing::ColorMode*)0) );
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
// still missing:
//          case XML_SD_TYPE_PRESPAGE_VISIBILITY :
//          {
//              break;
//          }
//          case XML_SD_TYPE_PRESPAGE_SOUND :
//          {
//              break;
//          }
        }

        if(pHdl)
            PutHdlCache(nType, pHdl);
    }

    return pHdl;
}

//////////////////////////////////////////////////////////////////////////////


