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


#include "atktextattributes.hxx"

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <com/sun/star/text/WritingMode2.hpp>

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleComponent.hpp>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star;

typedef gchar* (* AtkTextAttrFunc)       ( const uno::Any& rAny );
typedef bool   (* TextPropertyValueFunc) ( uno::Any& rAny, const gchar * value );

#define STRNCMP_PARAM( s )  s,sizeof( s )-1


/*****************************************************************************/

static AtkTextAttribute atk_text_attribute_paragraph_style = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_font_effect = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_decoration = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_line_height = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_rotation = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_shadow = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_tab_interval = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_tab_stops = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_writing_mode = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_vertical_align = ATK_TEXT_ATTR_INVALID;
static AtkTextAttribute atk_text_attribute_misspelled = ATK_TEXT_ATTR_INVALID;
// #i92232#
static AtkTextAttribute atk_text_attribute_tracked_change = ATK_TEXT_ATTR_INVALID;
// #i92233#
static AtkTextAttribute atk_text_attribute_mm_to_pixel_ratio = ATK_TEXT_ATTR_INVALID;

/*****************************************************************************/

/**
  * !! IMPORTANT NOTE !! : when adding items to this list, KEEP THE LIST SORTED
  *                        and re-arrange the enum values accordingly.
  */

enum ExportedAttribute
{
    TEXT_ATTRIBUTE_BACKGROUND_COLOR = 0,
    TEXT_ATTRIBUTE_CASEMAP,
    TEXT_ATTRIBUTE_FOREGROUND_COLOR,
    TEXT_ATTRIBUTE_CONTOURED,
    TEXT_ATTRIBUTE_CHAR_ESCAPEMENT,
    TEXT_ATTRIBUTE_BLINKING,
    TEXT_ATTRIBUTE_FONT_NAME,
    TEXT_ATTRIBUTE_HEIGHT,
    TEXT_ATTRIBUTE_HIDDEN,
    TEXT_ATTRIBUTE_KERNING,
    TEXT_ATTRIBUTE_LOCALE,
    TEXT_ATTRIBUTE_POSTURE,
    TEXT_ATTRIBUTE_RELIEF,
    TEXT_ATTRIBUTE_ROTATION,
    TEXT_ATTRIBUTE_SCALE,
    TEXT_ATTRIBUTE_SHADOWED,
    TEXT_ATTRIBUTE_STRIKETHROUGH,
    TEXT_ATTRIBUTE_UNDERLINE,
    TEXT_ATTRIBUTE_WEIGHT,
    // #i92233#
    TEXT_ATTRIBUTE_MM_TO_PIXEL_RATIO,
    TEXT_ATTRIBUTE_JUSTIFICATION,
    TEXT_ATTRIBUTE_BOTTOM_MARGIN,
    TEXT_ATTRIBUTE_FIRST_LINE_INDENT,
    TEXT_ATTRIBUTE_LEFT_MARGIN,
    TEXT_ATTRIBUTE_LINE_SPACING,
    TEXT_ATTRIBUTE_RIGHT_MARGIN,
    TEXT_ATTRIBUTE_STYLE_NAME,
    TEXT_ATTRIBUTE_TAB_STOPS,
    TEXT_ATTRIBUTE_TOP_MARGIN,
    TEXT_ATTRIBUTE_WRITING_MODE,
    TEXT_ATTRIBUTE_LAST
};

static const char * ExportedTextAttributes[TEXT_ATTRIBUTE_LAST] =
{
    "CharBackColor",        // TEXT_ATTRIBUTE_BACKGROUND_COLOR
    "CharCaseMap",          // TEXT_ATTRIBUTE_CASEMAP
    "CharColor",            // TEXT_ATTRIBUTE_FOREGROUND_COLOR
    "CharContoured",        // TEXT_ATTRIBUTE_CONTOURED
    "CharEscapement",       // TEXT_ATTRIBUTE_CHAR_ESCAPEMENT
    "CharFlash",            // TEXT_ATTRIBUTE_BLINKING
    "CharFontName",         // TEXT_ATTRIBUTE_FONT_NAME
    "CharHeight",           // TEXT_ATTRIBUTE_HEIGHT
    "CharHidden",           // TEXT_ATTRIBUTE_HIDDEN
    "CharKerning",          // TEXT_ATTRIBUTE_KERNING
    "CharLocale",           // TEXT_ATTRIBUTE_LOCALE
    "CharPosture",          // TEXT_ATTRIBUTE_POSTURE
    "CharRelief",           // TEXT_ATTRIBUTE_RELIEF
    "CharRotation",         // TEXT_ATTRIBUTE_ROTATION
    "CharScaleWidth",       // TEXT_ATTRIBUTE_SCALE
    "CharShadowed",         // TEXT_ATTRIBUTE_SHADOWED
    "CharStrikeout",        // TEXT_ATTRIBUTE_STRIKETHROUGH
    "CharUnderline",        // TEXT_ATTRIBUTE_UNDERLINE
    "CharWeight",           // TEXT_ATTRIBUTE_WEIGHT
    // #i92233#
    "MMToPixelRatio",       // TEXT_ATTRIBUTE_MM_TO_PIXEL_RATIO
    "ParaAdjust",           // TEXT_ATTRIBUTE_JUSTIFICATION
    "ParaBottomMargin",     // TEXT_ATTRIBUTE_BOTTOM_MARGIN
    "ParaFirstLineIndent",  // TEXT_ATTRIBUTE_FIRST_LINE_INDENT
    "ParaLeftMargin",       // TEXT_ATTRIBUTE_LEFT_MARGIN
    "ParaLineSpacing",      // TEXT_ATTRIBUTE_LINE_SPACING
    "ParaRightMargin",      // TEXT_ATTRIBUTE_RIGHT_MARGIN
    "ParaStyleName",        // TEXT_ATTRIBUTE_STYLE_NAME
    "ParaTabStops",         // TEXT_ATTRIBUTE_TAB_STOPS
    "ParaTopMargin",        // TEXT_ATTRIBUTE_TOP_MARGIN
    "WritingMode"           // TEXT_ATTRIBUTE_WRITING_MODE
};


/*****************************************************************************/

static gchar*
get_value( const uno::Sequence< beans::PropertyValue >& rAttributeList,
           sal_Int32 nIndex, AtkTextAttrFunc func )
{
    if( nIndex != -1 )
        return func(rAttributeList[nIndex].Value);

    return NULL;
}

#define get_bool_value( list, index ) get_value( list, index, Bool2String )
#define get_height_value( list, index ) get_value( list, index, Float2String )
#define get_justification_value( list, index ) get_value( list, index, Adjust2Justification )
#define get_cmm_value( list, index ) get_value( list, index, CMM2UnitString )
#define get_scale_width( list, index ) get_value( list, index, Scale2String )
#define get_strikethrough_value( list, index ) get_value( list, index, Strikeout2String )
#define get_string_value( list, index ) get_value( list, index, GetString )
#define get_style_value( list, index ) get_value( list, index, FontSlant2Style )
#define get_underline_value( list, index ) get_value( list, index, Underline2String )
#define get_variant_value( list, index ) get_value( list, index, CaseMap2String )
#define get_weight_value( list, index ) get_value( list, index, Weight2String )
#define get_language_string( list, index ) get_value( list, index, Locale2String )

static inline
double toPoint(sal_Int16 n)
{
    // 100th mm -> pt
    return (double) (n * 72) / 2540;
}


/*****************************************************************************/

static bool
InvalidValue( uno::Any&, const gchar * )
{
    return false;
}

/*****************************************************************************/

static gchar*
Float2String(const uno::Any& rAny)
{
    return g_strdup_printf( "%g", rAny.get<float>() );
}

static bool
String2Float( uno::Any& rAny, const gchar * value )
{
    float fval;

    if( 1 != sscanf( value, "%g", &fval ) )
        return false;

    rAny = uno::makeAny( fval );
    return true;
}

/*****************************************************************************/

static accessibility::XAccessibleComponent*
    getComponent( AtkText *pText ) throw (uno::RuntimeException)
{
    AtkObjectWrapper *pWrap = ATK_OBJECT_WRAPPER( pText );
    if( pWrap )
    {
        if( !pWrap->mpComponent && pWrap->mpContext )
        {
            uno::Any any = pWrap->mpContext->queryInterface( accessibility::XAccessibleComponent::static_type(NULL) );
            pWrap->mpComponent = reinterpret_cast< accessibility::XAccessibleComponent * > (any.pReserved);
            pWrap->mpComponent->acquire();
        }

        return pWrap->mpComponent;
    }

    return NULL;
}

static gchar*
get_color_value(const uno::Sequence< beans::PropertyValue >& rAttributeList,
                const sal_Int32 * pIndexArray,
                ExportedAttribute attr,
                AtkText * text)
{
    sal_Int32 nColor = -1; // AUTOMATIC
    sal_Int32 nIndex = pIndexArray[attr];

    if( nIndex != -1 )
        nColor = rAttributeList[nIndex].Value.get<sal_Int32>();

    /*
     * Check for color value for 100% alpha white, which means
     * "automatic". Grab the RGB value from XAccessibleComponent
     * in this case.
     */

    if( (nColor == -1) && text )
    {
        try
        {
            accessibility::XAccessibleComponent *pComponent = getComponent( text );
            if( pComponent )
            {
                switch( attr )
                {
                    case TEXT_ATTRIBUTE_BACKGROUND_COLOR:
                        nColor = pComponent->getBackground();
                        break;
                    case TEXT_ATTRIBUTE_FOREGROUND_COLOR:
                        nColor = pComponent->getForeground();
                        break;
                    default:
                        break;
                }
            }
        }

        catch(const uno::Exception& e) {
            g_warning( "Exception in get[Fore|Back]groundColor()" );
        }
    }

    if( nColor != -1 )
    {
        sal_uInt8 blue  = nColor & 0xFF;
        sal_uInt8 green = (nColor >> 8) & 0xFF;
        sal_uInt8 red   = (nColor >> 16) & 0xFF;

        return g_strdup_printf( "%u,%u,%u", red, green, blue );
    }

    return NULL;
}

static bool
String2Color( uno::Any& rAny, const gchar * value )
{
    int red, green, blue;

    if( 3 != sscanf( value, "%d,%d,%d", &red, &green, &blue ) )
        return false;

    sal_Int32 nColor = (sal_Int32) blue | ( (sal_Int32) green << 8 ) | ( ( sal_Int32 ) red << 16 );
    rAny = uno::makeAny( nColor );
    return true;
}

/*****************************************************************************/

static gchar*
FontSlant2Style(const uno::Any& rAny)
{
    const gchar * value = NULL;

    switch( rAny.get<awt::FontSlant>() )
    {
        case awt::FontSlant_NONE:
            value = "normal";
            break;

        case awt::FontSlant_OBLIQUE:
            value = "oblique";
            break;

        case awt::FontSlant_ITALIC:
            value = "italic";
            break;

        case awt::FontSlant_REVERSE_OBLIQUE:
            value = "reverse oblique";
            break;

        case awt::FontSlant_REVERSE_ITALIC:
            value = "reverse italic";
            break;

        default:
            break;
    }

    if( value )
         return g_strdup( value );

    return NULL;
}

static bool
Style2FontSlant( uno::Any& rAny, const gchar * value )
{
    awt::FontSlant aFontSlant;

    if( strncmp( value, STRNCMP_PARAM( "normal" ) ) )
        aFontSlant = awt::FontSlant_NONE;
    else if( strncmp( value, STRNCMP_PARAM( "oblique" ) ) )
        aFontSlant = awt::FontSlant_OBLIQUE;
    else if( strncmp( value, STRNCMP_PARAM( "italic" ) ) )
        aFontSlant = awt::FontSlant_ITALIC;
    else if( strncmp( value, STRNCMP_PARAM( "reverse oblique" ) ) )
        aFontSlant = awt::FontSlant_REVERSE_OBLIQUE;
    else if( strncmp( value, STRNCMP_PARAM( "reverse italic" ) ) )
        aFontSlant = awt::FontSlant_REVERSE_ITALIC;
    else
        return false;

    rAny = uno::makeAny( aFontSlant );
    return true;
}

/*****************************************************************************/

static gchar*
Weight2String(const uno::Any& rAny)
{
    return g_strdup_printf( "%g", rAny.get<float>() * 4 );
}

static bool
String2Weight( uno::Any& rAny, const gchar * value )
{
    float weight;

    if( 1 != sscanf( value, "%g", &weight ) )
        return false;

    rAny = uno::makeAny( weight / 4 );
    return true;
}


/*****************************************************************************/

static gchar*
Adjust2Justification(const uno::Any& rAny)
{
    const gchar * value = NULL;

    switch( rAny.get<short>() )
    {
        case style::ParagraphAdjust_LEFT:
            value = "left";
            break;

        case style::ParagraphAdjust_RIGHT:
            value = "right";
            break;

        case style::ParagraphAdjust_BLOCK:
        case style::ParagraphAdjust_STRETCH:
            value = "fill";
            break;

        case style::ParagraphAdjust_CENTER:
            value = "center";
            break;

        default:
            break;
    }

    if( value )
        return g_strdup( value );

    return NULL;
}

static bool
Justification2Adjust( uno::Any& rAny, const gchar * value )
{
    short nParagraphAdjust;

    if( strncmp( value, STRNCMP_PARAM( "left" ) ) )
        nParagraphAdjust = style::ParagraphAdjust_LEFT;
    else if( strncmp( value, STRNCMP_PARAM( "right" ) ) )
        nParagraphAdjust = style::ParagraphAdjust_RIGHT;
    else if( strncmp( value, STRNCMP_PARAM( "fill" ) ) )
        nParagraphAdjust = style::ParagraphAdjust_BLOCK;
    else if( strncmp( value, STRNCMP_PARAM( "center" ) ) )
        nParagraphAdjust = style::ParagraphAdjust_CENTER;
    else
        return false;

    rAny = uno::makeAny( nParagraphAdjust );
    return true;
}

/*****************************************************************************/

const gchar * font_strikethrough[] = {
    "none",   // FontStrikeout::NONE
    "single", // FontStrikeout::SINGLE
    "double", // FontStrikeout::DOUBLE
    NULL,     // FontStrikeout::DONTKNOW
    "bold",   // FontStrikeout::BOLD
    "with /", // FontStrikeout::SLASH
    "with X"  // FontStrikeout::X
};

const sal_Int16 n_strikeout_constants = sizeof(font_strikethrough) / sizeof(gchar*);

static gchar*
Strikeout2String(const uno::Any& rAny)
{
    sal_Int16 n = rAny.get<sal_Int16>();

    if( n >= 0 && n < n_strikeout_constants )
        return g_strdup( font_strikethrough[n] );

    return NULL;
}

static bool
String2Strikeout( uno::Any& rAny, const gchar * value )
{
    for( sal_Int16 n=0; n < n_strikeout_constants; ++n )
    {
        if( ( NULL != font_strikethrough[n] ) &&
            0 == strncmp( value, font_strikethrough[n], strlen( font_strikethrough[n] ) ) )
        {
            rAny = uno::makeAny( n );
            return true;
        }
    }

    return false;
}

/*****************************************************************************/

static gchar*
Underline2String(const uno::Any& rAny)
{
    const gchar * value = NULL;

    switch( rAny.get<sal_Int16>() )
    {
        case awt::FontUnderline::NONE:
            value = "none";
            break;

        case awt::FontUnderline::SINGLE:
            value = "single";
            break;

        case awt::FontUnderline::DOUBLE:
            value = "double";
            break;

        default:
            break;
    }

    if( value )
        return g_strdup( value );

    return NULL;
}

static bool
String2Underline( uno::Any& rAny, const gchar * value )
{
    short nUnderline;

    if( strncmp( value, STRNCMP_PARAM( "none" ) ) )
        nUnderline = awt::FontUnderline::NONE;
    else if( strncmp( value, STRNCMP_PARAM( "single" ) ) )
        nUnderline = awt::FontUnderline::SINGLE;
    else if( strncmp( value, STRNCMP_PARAM( "double" ) ) )
        nUnderline = awt::FontUnderline::DOUBLE;
    else
        return false;

    rAny = uno::makeAny( nUnderline );
    return true;
}

/*****************************************************************************/

static gchar*
GetString(const uno::Any& rAny)
{
    OString aFontName = OUStringToOString( rAny.get< OUString > (), RTL_TEXTENCODING_UTF8 );

    if( !aFontName.isEmpty() )
        return g_strdup( aFontName.getStr() );

    return NULL;
}

static bool
SetString( uno::Any& rAny, const gchar * value )
{
    OString aFontName( value );

    if( !aFontName.isEmpty() )
    {
        rAny = uno::makeAny( OStringToOUString( aFontName, RTL_TEXTENCODING_UTF8 ) );
        return true;
    }

    return false;
}

/*****************************************************************************/

// @see http://developer.gnome.org/doc/API/2.0/atk/AtkText.html#AtkTextAttribute

// CMM = 100th of mm
static gchar*
CMM2UnitString(const uno::Any& rAny)
{
    double fValue = rAny.get<sal_Int32>();
    fValue = fValue * 0.01;

    return g_strdup_printf( "%gmm", fValue );
}

static bool
UnitString2CMM( uno::Any& rAny, const gchar * value )
{
    float fValue = 0.0; // pb: dont use double here because of warning on linux

    if( 1 != sscanf( value, "%gmm", &fValue ) )
        return false;

    fValue = fValue * 100;

    rAny = uno::makeAny( (sal_Int32) fValue);
    return true;
}

/*****************************************************************************/

static const gchar * bool_values[] = { "true", "false" };

static gchar *
Bool2String( const uno::Any& rAny )
{
    int n = 1;

    if( rAny.get<sal_Bool>() )
        n = 0;

    return g_strdup( bool_values[n] );
}

static bool
String2Bool( uno::Any& rAny, const gchar * value )
{
    sal_Bool bValue;

    if( strncmp( value, STRNCMP_PARAM( "true" ) ) )
        bValue = sal_True;
    else if( strncmp( value, STRNCMP_PARAM( "false" ) ) )
        bValue = sal_False;
    else
        return false;

    rAny = uno::makeAny(bValue);
    return true;
}

/*****************************************************************************/

static gchar*
Scale2String( const uno::Any& rAny )
{
    return g_strdup_printf( "%g", (double) (rAny.get< sal_Int16 > ()) / 100 );
}

static bool
String2Scale( uno::Any& rAny, const gchar * value )
{
    double dval;

    if( 1 != sscanf( value, "%lg", &dval ) )
        return false;

    rAny = uno::makeAny((sal_Int16) (dval * 100));
    return true;
}

/*****************************************************************************/

static gchar *
CaseMap2String( const uno::Any& rAny )
{
    const gchar * value = NULL;

    switch( rAny.get<short>() )
    {
        case style::CaseMap::SMALLCAPS:
            value = "small_caps";
            break;

        default:
            value = "normal";
            break;
    }

    if( value )
        return g_strdup( value );

    return NULL;
}

static bool
String2CaseMap( uno::Any& rAny, const gchar * value )
{
    short nCaseMap;

    if( strncmp( value, STRNCMP_PARAM( "normal" ) ) )
        nCaseMap = style::CaseMap::NONE;
    else if( strncmp( value, STRNCMP_PARAM( "small_caps" ) ) )
        nCaseMap = style::CaseMap::SMALLCAPS;
    else
        return false;

    rAny = uno::makeAny( nCaseMap );
    return true;
}

/*****************************************************************************/

const gchar * font_stretch[] = {
    "ultra_condensed",
    "extra_condensed",
    "condensed",
    "semi_condensed",
    "normal",
    "semi_expanded",
    "expanded",
    "extra_expanded",
    "ultra_expanded"
};

static gchar*
Kerning2Stretch(const uno::Any& rAny)
{
    sal_Int16 n = rAny.get<sal_Int16>();
    int i = 4;

    // No good idea for a mapping - just return the basic info
    if( n < 0 )
        i=2;
    else if( n > 0 )
        i=6;

    return g_strdup(font_stretch[i]);
}

/*****************************************************************************/

static gchar*
Locale2String(const uno::Any& rAny)
{
    /* FIXME-BCP47: support language tags? And why is country lowercase? */
    lang::Locale aLocale = rAny.get<lang::Locale> ();
    LanguageTag aLanguageTag( aLocale);
    return g_strdup_printf( "%s-%s",
        OUStringToOString( aLanguageTag.getLanguage(), RTL_TEXTENCODING_ASCII_US).getStr(),
        OUStringToOString( aLanguageTag.getCountry(), RTL_TEXTENCODING_ASCII_US).toAsciiLowerCase().getStr() );
}

static bool
String2Locale( uno::Any& rAny, const gchar * value )
{
    /* FIXME-BCP47: support language tags? */
    bool ret = false;

    gchar ** str_array = g_strsplit_set( value, "-.@", -1 );
    if( str_array[0] != NULL )
    {
        ret = true;

        lang::Locale aLocale;

        aLocale.Language = OUString::createFromAscii(str_array[0]);
        if( str_array[1] != NULL )
        {
            gchar * country = g_ascii_strup(str_array[1], -1);
            aLocale.Country = OUString::createFromAscii(country);
            g_free(country);
        }

        rAny = uno::makeAny(aLocale);
    }

    g_strfreev(str_array);
    return ret;
}

/*****************************************************************************/

// @see http://www.w3.org/TR/2002/WD-css3-fonts-20020802/#font-effect-prop
static const gchar * relief[] = { "none", "emboss", "engrave" };
static const gchar * outline  = "outline";

static gchar *
get_font_effect(const uno::Sequence< beans::PropertyValue >& rAttributeList,
                sal_Int32 nContourIndex, sal_Int32 nReliefIndex)
{
    if( nContourIndex != -1 )
    {
        if( rAttributeList[nContourIndex].Value.get<sal_Bool>() )
            return g_strdup(outline);
    }

    if( nReliefIndex != -1 )
    {
        sal_Int16 n = rAttributeList[nReliefIndex].Value.get<sal_Int16>();
        if( n <  3)
            return g_strdup(relief[n]);
    }

    return NULL;
}

/*****************************************************************************/

// @see http://www.w3.org/TR/REC-CSS2/text.html#lining-striking-props


enum
{
    DECORATION_NONE = 0,
    DECORATION_BLINK,
    DECORATION_UNDERLINE,
    DECORATION_LINE_THROUGH
};


static const gchar * decorations[] = { "none", "blink", "underline", "line-through" };

static gchar *
get_text_decoration(const uno::Sequence< beans::PropertyValue >& rAttributeList,
                    sal_Int32 nBlinkIndex, sal_Int32 nUnderlineIndex,
                    sal_Int16 nStrikeoutIndex)
{
    gchar * value_list[4] = { NULL, NULL, NULL, NULL };
    gint count = 0;

    // no property value found
    if( ( nBlinkIndex == -1 ) && (nUnderlineIndex == -1 ) && (nStrikeoutIndex == -1))
        return NULL;

    if( nBlinkIndex != -1 )
    {
        if( rAttributeList[nBlinkIndex].Value.get<sal_Bool>() )
            value_list[count++] = const_cast <gchar *> (decorations[DECORATION_BLINK]);
    }
    if( nUnderlineIndex != -1 )
    {
        sal_Int16 n = rAttributeList[nUnderlineIndex].Value.get<sal_Int16> ();
        if( n != awt::FontUnderline::NONE )
            value_list[count++] = const_cast <gchar *> (decorations[DECORATION_UNDERLINE]);
    }
    if( nStrikeoutIndex != -1 )
    {
        sal_Int16 n = rAttributeList[nStrikeoutIndex].Value.get<sal_Int16> ();
        if( n != awt::FontStrikeout::NONE && n != awt::FontStrikeout::DONTKNOW )
            value_list[count++] = const_cast <gchar *> (decorations[DECORATION_LINE_THROUGH]);
    }

    if( count == 0 )
        value_list[count++] = const_cast <gchar *> (decorations[DECORATION_NONE]);

    return g_strjoinv(" ", value_list);
}


/*****************************************************************************/

// @see http://www.w3.org/TR/REC-CSS2/text.html#propdef-text-shadow

static const gchar * shadow_values[] = { "none", "black" };

static gchar *
Bool2Shadow( const uno::Any& rAny )
{
    int n = 0;

    if( rAny.get<sal_Bool>() )
        n = 1;

    return g_strdup( shadow_values[n] );
}

/*****************************************************************************/

static gchar *
Short2Degree( const uno::Any& rAny )
{
    float f = rAny.get<sal_Int16>() / 10;
    return g_strdup_printf( "%g", f );
}

/*****************************************************************************/

const gchar * directions[] = { "ltr", "rtl", "rtl", "ltr", "none" };

static gchar *
WritingMode2Direction( const uno::Any& rAny )
{
    sal_Int16 n = rAny.get<sal_Int16>();

    if( 0 <= n && n <= text::WritingMode2::PAGE )
        return g_strdup(directions[n]);

    return NULL;
}

// @see http://www.w3.org/TR/2001/WD-css3-text-20010517/#PrimaryTextAdvanceDirection

const gchar * writing_modes[] = { "lr-tb", "rl-tb", "tb-rl", "tb-lr", "none" };
static gchar *
WritingMode2String( const uno::Any& rAny )
{
    sal_Int16 n = rAny.get<sal_Int16>();

    if( 0 <= n && n <= text::WritingMode2::PAGE )
        return g_strdup(writing_modes[n]);

    return NULL;
}

/*****************************************************************************/

const char * baseline_values[] = { "baseline", "sub", "super" };

// @see http://www.w3.org/TR/REC-CSS2/visudet.html#propdef-vertical-align
static gchar *
Escapement2VerticalAlign( const uno::Any& rAny )
{
    sal_Int16 n = rAny.get<sal_Int16>();
    gchar * ret = NULL;

    // Values are in %, 101% means "automatic"
    if( n == 0 )
        ret = g_strdup(baseline_values[0]);
    else if( n == 101 )
        ret = g_strdup(baseline_values[2]);
    else if( n == -101 )
        ret = g_strdup(baseline_values[1]);
    else
        ret = g_strdup_printf( "%d%%", n );

    return ret;
}

/*****************************************************************************/

// @see http://www.w3.org/TR/REC-CSS2/visudet.html#propdef-line-height
static gchar *
LineSpacing2LineHeight( const uno::Any& rAny )
{
    style::LineSpacing ls;
    gchar * ret = NULL;

    if( rAny >>= ls )
    {
        if( ls.Mode == style::LineSpacingMode::PROP )
            ret = g_strdup_printf( "%d%%", ls.Height );
        else if( ls.Mode == style::LineSpacingMode::FIX )
            ret = g_strdup_printf( "%.3gpt", toPoint(ls.Height) );
    }

    return ret;
}

/*****************************************************************************/

// @see http://www.w3.org/People/howcome/t/970224HTMLERB-CSS/WD-tabs-970117.html
static gchar *
TabStopList2String( const uno::Any& rAny, bool default_tabs )
{
    uno::Sequence< style::TabStop > theTabStops;
    gchar * ret = NULL;

    if( rAny >>= theTabStops)
    {
        sal_Int32 indexOfTab = 0;
        sal_Int32 numberOfTabs = theTabStops.getLength();
        sal_Unicode lastFillChar = (sal_Unicode) ' ';

        for( ; indexOfTab < numberOfTabs; ++indexOfTab )
        {
            bool is_default_tab = (style::TabAlign_DEFAULT == theTabStops[indexOfTab].Alignment);

            if( is_default_tab != default_tabs )
                continue;

            double fValue = theTabStops[indexOfTab].Position;
            fValue = fValue * 0.01;

            const gchar * tab_align = "";
            switch( theTabStops[indexOfTab].Alignment )
            {
                case style::TabAlign_LEFT :
                    tab_align = "left ";
                    break;
                case style::TabAlign_CENTER :
                    tab_align = "center ";
                    break;
                case style::TabAlign_RIGHT :
                    tab_align = "right ";
                    break;
                case style::TabAlign_DECIMAL :
                    tab_align = "decimal ";
                    break;
                default:
                    break;
            }

            const gchar * lead_char = "";

            if( theTabStops[indexOfTab].FillChar != lastFillChar )
            {
                lastFillChar = theTabStops[indexOfTab].FillChar;
                switch (lastFillChar)
                {
                    case (sal_Unicode) ' ':
                        lead_char = "blank ";
                        break;

                    case (sal_Unicode) '.':
                        lead_char = "dotted ";
                        break;

                    case (sal_Unicode) '-':
                        lead_char = "dashed ";
                        break;

                    case (sal_Unicode) '_':
                        lead_char = "lined ";
                        break;

                    default:
                        lead_char = "custom ";
                        break;
                }
            }

            gchar * tab_str = g_strdup_printf( "%s%s%gmm", lead_char, tab_align, fValue );

            if( ret )
            {
                gchar * old_tab_str = ret;
                ret = g_strconcat(old_tab_str, " ", tab_str, (const char*)NULL);
                g_free( old_tab_str );
            }
            else
                ret = tab_str;
        }
    }

    return ret;
}

static gchar *
TabStops2String( const uno::Any& rAny )
{
    return TabStopList2String(rAny, false);
}

static gchar *
DefaultTabStops2String( const uno::Any& rAny )
{
    return TabStopList2String(rAny, true);
}

/*****************************************************************************/

extern "C" int
attr_compare(const void *p1,const void *p2)
{
    const rtl_uString * pustr = (const rtl_uString *) p1;
    const char * pc = *((const char **) p2);

    return rtl_ustr_ascii_compare_WithLength(pustr->buffer, pustr->length, pc);
}

static void
find_exported_attributes( sal_Int32 *pArray,
    const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rAttributeList )
{
    for( sal_Int32 i = 0; i < rAttributeList.getLength(); i++ )
    {
        const char ** pAttr = (const char **) bsearch(rAttributeList[i].Name.pData,
            ExportedTextAttributes, TEXT_ATTRIBUTE_LAST, sizeof(const char *),
            attr_compare);

        if( pAttr )
        {
            sal_Int32 nIndex = pAttr - ExportedTextAttributes;
            pArray[nIndex] = i;
        }
    }
}

/*****************************************************************************/

static AtkAttributeSet*
attribute_set_prepend( AtkAttributeSet* attribute_set,
                       AtkTextAttribute attribute,
                       gchar * value )
{
    if( value )
    {
        AtkAttribute *at = (AtkAttribute *) g_malloc( sizeof (AtkAttribute) );
        at->name  = g_strdup( atk_text_attribute_get_name( attribute ) );
        at->value = value;

        return g_slist_prepend(attribute_set, at);
    }

    return attribute_set;
}

/*****************************************************************************/

AtkAttributeSet*
attribute_set_new_from_property_values(
    const uno::Sequence< beans::PropertyValue >& rAttributeList,
    bool run_attributes_only,
    AtkText *text)
{
    AtkAttributeSet* attribute_set = NULL;

    sal_Int32 aIndexList[TEXT_ATTRIBUTE_LAST] = { -1 };

    // Initialize index array with -1
    for( sal_Int32 attr = 0; attr < TEXT_ATTRIBUTE_LAST; ++attr )
        aIndexList[attr] = -1;

    find_exported_attributes(aIndexList, rAttributeList);

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_BG_COLOR,
        get_color_value(rAttributeList, aIndexList, TEXT_ATTRIBUTE_BACKGROUND_COLOR, run_attributes_only ? NULL : text ) );

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_FG_COLOR,
        get_color_value(rAttributeList, aIndexList, TEXT_ATTRIBUTE_FOREGROUND_COLOR, run_attributes_only ? NULL : text) );

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_INVISIBLE,
        get_bool_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_HIDDEN]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_UNDERLINE,
        get_underline_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_UNDERLINE]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_STRIKETHROUGH,
        get_strikethrough_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_STRIKETHROUGH]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_SIZE,
        get_height_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_HEIGHT]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_WEIGHT,
        get_weight_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_WEIGHT]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_FAMILY_NAME,
        get_string_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_FONT_NAME]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_VARIANT,
        get_variant_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_CASEMAP]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_STYLE,
        get_style_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_POSTURE]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_SCALE,
        get_scale_width(rAttributeList, aIndexList[TEXT_ATTRIBUTE_SCALE]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_LANGUAGE,
        get_language_string(rAttributeList, aIndexList[TEXT_ATTRIBUTE_LOCALE]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_DIRECTION,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_WRITING_MODE], WritingMode2Direction));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_STRETCH,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_KERNING], Kerning2Stretch));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_font_effect )
        atk_text_attribute_font_effect = atk_text_attribute_register("font-effect");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_font_effect,
        get_font_effect(rAttributeList, aIndexList[TEXT_ATTRIBUTE_CONTOURED], aIndexList[TEXT_ATTRIBUTE_RELIEF]));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_decoration )
        atk_text_attribute_decoration = atk_text_attribute_register("text-decoration");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_decoration,
        get_text_decoration(rAttributeList, aIndexList[TEXT_ATTRIBUTE_BLINKING],
            aIndexList[TEXT_ATTRIBUTE_UNDERLINE], aIndexList[TEXT_ATTRIBUTE_STRIKETHROUGH]));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_rotation )
        atk_text_attribute_rotation = atk_text_attribute_register("text-rotation");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_rotation,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_ROTATION], Short2Degree));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_shadow )
        atk_text_attribute_shadow = atk_text_attribute_register("text-shadow");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_shadow,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_SHADOWED], Bool2Shadow));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_writing_mode )
        atk_text_attribute_writing_mode = atk_text_attribute_register("writing-mode");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_writing_mode,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_WRITING_MODE], WritingMode2String));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_vertical_align )
        atk_text_attribute_vertical_align = atk_text_attribute_register("vertical-align");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_vertical_align,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_CHAR_ESCAPEMENT], Escapement2VerticalAlign));

    if( run_attributes_only )
        return attribute_set;

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_LEFT_MARGIN,
        get_cmm_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_LEFT_MARGIN]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_RIGHT_MARGIN,
        get_cmm_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_RIGHT_MARGIN]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_INDENT,
        get_cmm_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_FIRST_LINE_INDENT]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_PIXELS_ABOVE_LINES,
        get_cmm_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_TOP_MARGIN]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_PIXELS_BELOW_LINES,
        get_cmm_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_BOTTOM_MARGIN]));

    attribute_set = attribute_set_prepend(attribute_set, ATK_TEXT_ATTR_JUSTIFICATION,
        get_justification_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_JUSTIFICATION]));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_paragraph_style )
        atk_text_attribute_paragraph_style = atk_text_attribute_register("paragraph-style");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_paragraph_style,
        get_string_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_STYLE_NAME]));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_line_height )
        atk_text_attribute_line_height = atk_text_attribute_register("line-height");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_line_height,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_LINE_SPACING], LineSpacing2LineHeight));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_tab_interval )
        atk_text_attribute_tab_interval = atk_text_attribute_register("tab-interval");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_tab_interval,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_TAB_STOPS], DefaultTabStops2String));

    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_tab_stops )
        atk_text_attribute_tab_stops = atk_text_attribute_register("tab-stops");

    attribute_set = attribute_set_prepend(attribute_set, atk_text_attribute_tab_stops,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_TAB_STOPS], TabStops2String));

    // #i92233#
    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_mm_to_pixel_ratio )
        atk_text_attribute_mm_to_pixel_ratio = atk_text_attribute_register("mm-to-pixel-ratio");

    attribute_set = attribute_set_prepend( attribute_set, atk_text_attribute_mm_to_pixel_ratio,
        get_value(rAttributeList, aIndexList[TEXT_ATTRIBUTE_MM_TO_PIXEL_RATIO], Float2String));

    return attribute_set;
}


AtkAttributeSet* attribute_set_prepend_misspelled( AtkAttributeSet* attribute_set )
{
    if( ATK_TEXT_ATTR_INVALID == atk_text_attribute_misspelled )
        atk_text_attribute_misspelled = atk_text_attribute_register( "text-spelling" );

    attribute_set = attribute_set_prepend( attribute_set, atk_text_attribute_misspelled,
        g_strdup_printf( "misspelled" ) );

    return attribute_set;
}

// #i92232#
AtkAttributeSet* attribute_set_prepend_tracked_change_insertion( AtkAttributeSet* attribute_set )
{
    if ( ATK_TEXT_ATTR_INVALID == atk_text_attribute_tracked_change )
    {
        atk_text_attribute_tracked_change = atk_text_attribute_register( "text-tracked-change" );
    }

    attribute_set = attribute_set_prepend( attribute_set,
                                           atk_text_attribute_tracked_change,
                                           g_strdup_printf( "insertion" ) );

    return attribute_set;
}

AtkAttributeSet* attribute_set_prepend_tracked_change_deletion( AtkAttributeSet* attribute_set )
{
    if ( ATK_TEXT_ATTR_INVALID == atk_text_attribute_tracked_change )
    {
        atk_text_attribute_tracked_change = atk_text_attribute_register( "text-tracked-change" );
    }

    attribute_set = attribute_set_prepend( attribute_set,
                                           atk_text_attribute_tracked_change,
                                           g_strdup_printf( "deletion" ) );

    return attribute_set;
}

AtkAttributeSet* attribute_set_prepend_tracked_change_formatchange( AtkAttributeSet* attribute_set )
{
    if ( ATK_TEXT_ATTR_INVALID == atk_text_attribute_tracked_change )
    {
        atk_text_attribute_tracked_change = atk_text_attribute_register( "text-tracked-change" );
    }

    attribute_set = attribute_set_prepend( attribute_set,
                                           atk_text_attribute_tracked_change,
                                           g_strdup_printf( "attribute-change" ) );

    return attribute_set;
}

/*****************************************************************************/

struct AtkTextAttrMapping
{
    const char *          name;
    TextPropertyValueFunc toPropertyValue;
};

const AtkTextAttrMapping g_TextAttrMap[] =
{
    { "", InvalidValue },                       // ATK_TEXT_ATTR_INVALID = 0
    { "ParaLeftMargin", UnitString2CMM },       // ATK_TEXT_ATTR_LEFT_MARGIN
    { "ParaRightMargin", UnitString2CMM },      // ATK_TEXT_ATTR_RIGHT_MARGIN
    { "ParaFirstLineIndent", UnitString2CMM },  // ATK_TEXT_ATTR_INDENT
    { "CharHidden", String2Bool },              // ATK_TEXT_ATTR_INVISIBLE
    { "", InvalidValue },                       // ATK_TEXT_ATTR_EDITABLE
    { "ParaTopMargin", UnitString2CMM },        // ATK_TEXT_ATTR_PIXELS_ABOVE_LINES
    { "ParaBottomMargin", UnitString2CMM },     // ATK_TEXT_ATTR_PIXELS_BELOW_LINES
    { "", InvalidValue },                       // ATK_TEXT_ATTR_PIXELS_INSIDE_WRAP
    { "", InvalidValue },                       // ATK_TEXT_ATTR_BG_FULL_HEIGHT
    { "", InvalidValue },                       // ATK_TEXT_ATTR_RISE
    { "CharUnderline", String2Underline },      // ATK_TEXT_ATTR_UNDERLINE
    { "CharStrikeout", String2Strikeout },      // ATK_TEXT_ATTR_STRIKETHROUGH
    { "CharHeight", String2Float },             // ATK_TEXT_ATTR_SIZE
    { "CharScaleWidth", String2Scale },         // ATK_TEXT_ATTR_SCALE
    { "CharWeight", String2Weight },            // ATK_TEXT_ATTR_WEIGHT
    { "CharLocale", String2Locale },            // ATK_TEXT_ATTR_LANGUAGE
    { "CharFontName",  SetString },             // ATK_TEXT_ATTR_FAMILY_NAME
    { "CharBackColor", String2Color },          // ATK_TEXT_ATTR_BG_COLOR
    { "CharColor", String2Color },              // ATK_TEXT_ATTR_FG_COLOR
    { "", InvalidValue },                       // ATK_TEXT_ATTR_BG_STIPPLE
    { "", InvalidValue },                       // ATK_TEXT_ATTR_FG_STIPPLE
    { "", InvalidValue },                       // ATK_TEXT_ATTR_WRAP_MODE
    { "", InvalidValue },                       // ATK_TEXT_ATTR_DIRECTION
    { "ParaAdjust", Justification2Adjust },     // ATK_TEXT_ATTR_JUSTIFICATION
    { "", InvalidValue },                       // ATK_TEXT_ATTR_STRETCH
    { "CharCaseMap", String2CaseMap },          // ATK_TEXT_ATTR_VARIANT
    { "CharPosture", Style2FontSlant }          // ATK_TEXT_ATTR_STYLE
};

static const sal_Int32 g_TextAttrMapSize = sizeof( g_TextAttrMap ) / sizeof( AtkTextAttrMapping );

/*****************************************************************************/

bool
attribute_set_map_to_property_values(
    AtkAttributeSet* attribute_set,
    uno::Sequence< beans::PropertyValue >& rValueList )
{
    // Ensure enough space ..
    uno::Sequence< beans::PropertyValue > aAttributeList (g_TextAttrMapSize);

    sal_Int32 nIndex = 0;
    for( GSList * item = attribute_set; item != NULL; item = g_slist_next( item ) )
    {
        AtkAttribute* attribute = (AtkAttribute *) item;

        AtkTextAttribute text_attr = atk_text_attribute_for_name( attribute->name );
        if( text_attr < g_TextAttrMapSize )
        {
            if( g_TextAttrMap[text_attr].name[0] != '\0' )
            {
                if( ! g_TextAttrMap[text_attr].toPropertyValue( aAttributeList[nIndex].Value, attribute->value) )
                    return false;

                aAttributeList[nIndex].Name = OUString::createFromAscii( g_TextAttrMap[text_attr].name );
                aAttributeList[nIndex].State = beans::PropertyState_DIRECT_VALUE;
                ++nIndex;
            }
        }
        else
        {
            // Unsupported text attribute
            return false;
        }
    }

    aAttributeList.realloc( nIndex );
    rValueList = aAttributeList;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
