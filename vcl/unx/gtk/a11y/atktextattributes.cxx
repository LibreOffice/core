/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atktextattributes.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:28:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "atktextattributes.hxx"

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

#include <svapp.hxx>
#include <outdev.hxx>

#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star;

typedef gchar* (* AtkTextAttrFunc)       ( const uno::Any& rAny );
typedef bool   (* TextPropertyValueFunc) ( uno::Any& rAny, const gchar * value );

#define STRNCMP_PARAM( s )  s,sizeof( s )-1

/*****************************************************************************/

static gchar*
NullString(const uno::Any&)
{
    return NULL;
}

static bool
InvalidValue( uno::Any&, const gchar * )
{
    return false;
}

/*****************************************************************************/

static gchar*
Float2String(const uno::Any& rAny)
{
    return g_strdup_printf( "%.0f", rAny.get<float>() );
}

static bool
String2Float( uno::Any& rAny, const gchar * value )
{
    float fval;

    if( 1 != sscanf( value, "%f", &fval ) )
        return false;

    rAny = uno::makeAny( fval );
    return true;
}

/*****************************************************************************/

static gchar*
Long2String(const uno::Any& rAny)
{
    return g_strdup_printf( "%ld", rAny.get<sal_Int32>() );
}

static bool
String2Long( uno::Any& rAny, const gchar * value )
{
    sal_Int32 lval;

    if( 1 != sscanf( value, "%ld", &lval ) )
        return false;

    rAny = uno::makeAny( lval );
    return true;
}

/*****************************************************************************/

static gchar*
Color2String(const uno::Any& rAny)
{
    sal_Int32 nColor = rAny.get<sal_Int32>();

    if( nColor != -1 ) // AUTOMATIC
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
    return g_strdup_printf( "%.0f", rAny.get<float>() * 4 );
}

static bool
String2Weight( uno::Any& rAny, const gchar * value )
{
    float weight;

    if( 1 != sscanf( value, "%f", &weight ) )
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

static gchar*
Strikeout2String(const uno::Any& rAny)
{
    sal_Int16 nStrikeout = rAny.get<sal_Int16>();

    if( nStrikeout != 3 ) // DONTKNOW
        return g_strdup( nStrikeout ? "true" : "false" );

    return NULL;
}

static bool
String2Strikeout( uno::Any& rAny, const gchar * value )
{
    sal_Int16 nStrikeout;

    if( strncmp( value, STRNCMP_PARAM( "true" ) ) )
        nStrikeout = awt::FontStrikeout::SINGLE;
    else if( strncmp( value, STRNCMP_PARAM( "false" ) ) )
        nStrikeout = awt::FontStrikeout::NONE;
    else
        return false;

    rAny = uno::makeAny( nStrikeout );
    return true;
}

/*****************************************************************************/

static gchar*
Underline2String(const uno::Any& rAny)
{
    const gchar * value = NULL;

    switch( rAny.get<short>() )
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

static inline Size
LogicToPixel( long margin_x, long margin_y )
{
    OutputDevice * pOutputDevice = Application::GetDefaultDevice();
    return pOutputDevice->LogicToPixel( Size( margin_x, margin_y ), MapMode( MAP_100TH_MM ) );
}

static inline Size
PixelToLogic( long margin_x, long margin_y )
{
    OutputDevice * pOutputDevice = Application::GetDefaultDevice();
    return pOutputDevice->PixelToLogic( Size( margin_x, margin_y ), MapMode( MAP_100TH_MM ) );
}

static gchar*
MarginHeight2Pixel(const uno::Any& rAny)
{
    Size aMargin = LogicToPixel( 0, rAny.get<sal_Int32>() );
    return g_strdup_printf( "%ld",  aMargin.getHeight() );
}

static bool
Pixel2MarginHeight( uno::Any& rAny, const gchar * value )
{
    sal_Int32 nHeight;

    if( 1 != sscanf( value, "%ld", &nHeight ) )
        return false;

    Size aMargin = PixelToLogic( 0, nHeight );
    rAny = uno::makeAny( (sal_Int32) aMargin.getHeight() );
    return true;
}

static gchar*
MarginWidth2Pixel(const uno::Any& rAny)
{
    Size aMargin = LogicToPixel( rAny.get<sal_Int32>(), 0 );
    return g_strdup_printf( "%ld",  aMargin.getWidth() );
}

static bool
Pixel2MarginWidth( uno::Any& rAny, const gchar * value )
{
    sal_Int32 nWidth;

    if( 1 != sscanf( value, "%ld", &nWidth ) )
        return false;

    Size aMargin = PixelToLogic( nWidth, 0 );
    rAny = uno::makeAny( (sal_Int32) aMargin.getWidth() );
    return true;
}

/*****************************************************************************/

static gchar *
Bool2String( const uno::Any& rAny )
{
    const gchar * value = NULL;

    if( rAny.get<sal_Bool>() )
        value = "true";
    else
        value = "false";

    if( value )
        return g_strdup( value );

    return NULL;
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

struct AtkTextAttrMapping
{
    const char *          name;
    AtkTextAttrFunc       toAtkTextAttr;
    TextPropertyValueFunc toPropertyValue;
};

const AtkTextAttrMapping g_TextAttrMap[] =
{
    { "", NullString,   InvalidValue },                             // ATK_TEXT_ATTR_INVALID = 0
    { "ParaLeftMargin", MarginWidth2Pixel, Pixel2MarginWidth },     // ATK_TEXT_ATTR_LEFT_MARGIN
    { "ParaRightMargin", MarginWidth2Pixel, Pixel2MarginWidth },    // ATK_TEXT_ATTR_RIGHT_MARGIN
    { "ParaFirstLineIndent", Long2String, String2Long },            // ATK_TEXT_ATTR_INDENT
    { "CharHidden", Bool2String, String2Bool },                     // ATK_TEXT_ATTR_INVISIBLE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_EDITABLE
    { "ParaTopMargin", MarginHeight2Pixel, Pixel2MarginHeight },    // ATK_TEXT_ATTR_PIXELS_ABOVE_LINES
    { "ParaBottomMargin", MarginHeight2Pixel, Pixel2MarginHeight }, // ATK_TEXT_ATTR_PIXELS_BELOW_LINES
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_PIXELS_INSIDE_WRAP
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_BG_FULL_HEIGHT
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_RISE
    { "CharUnderline", Underline2String, String2Underline },        // ATK_TEXT_ATTR_UNDERLINE
    { "CharStrikeout", Strikeout2String, String2Strikeout },        // ATK_TEXT_ATTR_STRIKETHROUGH
    { "CharHeight", Float2String, String2Float },                   // ATK_TEXT_ATTR_SIZE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_SCALE
    { "CharWeight", Weight2String, String2Weight },                 // ATK_TEXT_ATTR_WEIGHT
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_LANGUAGE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_FAMILY_NAME
    { "CharBackColor", Color2String, String2Color },                // ATK_TEXT_ATTR_BG_COLOR
    { "CharColor", Color2String, String2Color },                    // ATK_TEXT_ATTR_FG_COLOR
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_BG_STIPPLE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_FG_STIPPLE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_WRAP_MODE
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_DIRECTION
    { "ParaAdjust", Adjust2Justification, Justification2Adjust },   // ATK_TEXT_ATTR_JUSTIFICATION
    { "", NullString,   InvalidValue },      // ATK_TEXT_ATTR_STRETCH
    { "CharCaseMap", CaseMap2String, String2CaseMap },              // ATK_TEXT_ATTR_VARIANT
    { "CharPosture", FontSlant2Style, Style2FontSlant }             // ATK_TEXT_ATTR_STYLE
};

//
//        FIXME !!
//        "ParaLineSpacing" => ATK_TEXT_ATTR_PIXELS_INSIDE_WRAP ?

static const sal_Int32 g_TextAttrMapSize = sizeof( g_TextAttrMap ) / sizeof( AtkTextAttrMapping );

/*****************************************************************************/

static AtkAttributeSet*
attribute_set_prepend( AtkAttributeSet* attribute_set,
                       AtkTextAttribute attribute,
                       gchar * value )
{
    AtkAttribute *at = (AtkAttribute *) g_malloc( sizeof (AtkAttribute) );
    at->name  = g_strdup( atk_text_attribute_get_name( attribute ) );
    at->value = value;

    return g_slist_prepend(attribute_set, at);
}

/*****************************************************************************/

AtkAttributeSet*
attribute_set_new_from_property_values(
    const uno::Sequence< beans::PropertyValue >& rAttributeList )
{
    AtkAttributeSet* attribute_set = NULL;

    for( sal_Int32 i = 0; i < rAttributeList.getLength(); i++ )
    {
        gint j = 0;
        for( ; j < g_TextAttrMapSize; ++j )
        {
            if( 0 == rAttributeList[i].Name.compareToAscii( g_TextAttrMap[j].name ) )
            {
                gchar * value = g_TextAttrMap[j].toAtkTextAttr(rAttributeList[i].Value);
                if( value )
                    attribute_set = attribute_set_prepend( attribute_set, static_cast < AtkTextAttribute > (j), value );

                break;
            }
        }
    }

    return attribute_set;
}

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

                aAttributeList[nIndex].Name = rtl::OUString::createFromAscii( g_TextAttrMap[text_attr].name );
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

