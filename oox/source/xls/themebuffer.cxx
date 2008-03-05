/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: themebuffer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:07:19 $
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

#include "oox/xls/themebuffer.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/stylespropertyhelper.hxx"

using ::oox::drawingml::ClrScheme;
using ::oox::drawingml::Theme;
using ::oox::drawingml::ThemePtr;

namespace oox {
namespace xls {

// ============================================================================

namespace {

/** Specifies default theme fonts for a specific locale. */
struct BuiltinThemeFont
{
    const sal_Char*     mpcLocale;          /// The locale for this font setting.
    const sal_Char*     mpcHeadFont;        /// Default heading font.
    const sal_Char*     mpcBodyFont;        /// Default body font.
};

#define FONT_JA "\357\274\255\357\274\263 \357\274\260\343\202\264\343\202\267\343\203\203\343\202\257"
#define FONT_KO "\353\247\221\354\235\200 \352\263\240\353\224\225"
#define FONT_CS "\345\256\213\344\275\223"
#define FONT_CT "\346\226\260\347\264\260\346\230\216\351\253\224"

static const BuiltinThemeFont spBuiltinThemeFonts[] =
{ //  locale    headings font           body font
    { "*",      "Cambria",              "Calibri"           },  // Default
    { "ar",     "Times New Roman",      "Arial"             },  // Arabic
    { "bn",     "Vrinda",               "Vrinda"            },  // Bengali
    { "div",    "MV Boli",              "MV Boli"           },  // Divehi
    { "fa",     "Times New Roman",      "Arial"             },  // Farsi
    { "gu",     "Shruti",               "Shruti"            },  // Gujarati
    { "he",     "Times New Roman",      "Arial"             },  // Hebrew
    { "hi",     "Mangal",               "Mangal"            },  // Hindi
    { "ja",     FONT_JA,                FONT_JA             },  // Japanese
    { "kn",     "Tunga",                "Tunga"             },  // Kannada
    { "ko",     FONT_KO,                FONT_KO             },  // Korean
    { "kok",    "Mangal",               "Mangal"            },  // Konkani
    { "ml",     "Kartika",              "Kartika"           },  // Malayalam
    { "mr",     "Mangal",               "Mangal"            },  // Marathi
    { "pa",     "Raavi",                "Raavi"             },  // Punjabi
    { "sa",     "Mangal",               "Mangal"            },  // Sanskrit
    { "syr",    "Estrangelo Edessa",    "Estrangelo Edessa" },  // Syriac
    { "ta",     "Latha",                "Latha"             },  // Tamil
    { "te",     "Gautami",              "Gautami"           },  // Telugu
    { "th",     "Tahoma",               "Tahoma"            },  // Thai
    { "ur",     "Times New Roman",      "Arial"             },  // Urdu
    { "vi",     "Times New Roman",      "Arial"             },  // Vietnamese
    { "zh",     FONT_CS,                FONT_CS             },  // Chinese, Simplified
    { "zh-HK",  FONT_CT,                FONT_CT             },  // Chinese, Hong Kong
    { "zh-MO",  FONT_CT,                FONT_CT             },  // Chinese, Macau
    { "zh-TW",  FONT_CT,                FONT_CT             }   // Chinese, Taiwan
};

} // namespace

// ----------------------------------------------------------------------------

ThemeBuffer::ThemeBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mxDefFontData( new OoxFontData )
{
    switch( getFilterType() )
    {
        case FILTER_OOX:
            //! TODO: locale dependent font name
            mxDefFontData->maName = CREATE_OUSTRING( "Cambria" );
            mxDefFontData->mfHeight = 11.0;
        break;
        case FILTER_BIFF:
            //! TODO: BIFF dependent font name
            mxDefFontData->maName = CREATE_OUSTRING( "Arial" );
            mxDefFontData->mfHeight = 10.0;
        break;
        case FILTER_UNKNOWN: break;
    }
}

ThemeBuffer::~ThemeBuffer()
{
}

Theme& ThemeBuffer::getOrCreateCoreTheme()
{
    if( !mxTheme )
        mxTheme.reset( new Theme );
    return *mxTheme;
}

sal_Int32 ThemeBuffer::getColorByToken( sal_Int32 nToken ) const
{
    sal_Int32 nColor = 0;
    if( mxTheme.get() )
        if( const ClrScheme* pClrScheme = mxTheme->getClrScheme().get() )
            if( pClrScheme->getColor( nToken, nColor ) )
                return nColor;
    return API_RGB_TRANSPARENT;
}

sal_Int32 ThemeBuffer::getColorByIndex( sal_Int32 nIndex ) const
{
    static const sal_Int32 spnColorTokens[] = {
        XML_lt1, XML_dk1, XML_lt2, XML_dk2, XML_accent1, XML_accent2,
        XML_accent3, XML_accent4, XML_accent5, XML_accent6, XML_hlink, XML_folHlink };

    sal_Int32 nColor = 0;
    if( mxTheme.get() )
        if( const ClrScheme* pClrScheme = mxTheme->getClrScheme().get() )
            if( pClrScheme->getColor( STATIC_ARRAY_SELECT( spnColorTokens, nIndex, XML_TOKEN_INVALID ), nColor ) )
                return nColor;
    return API_RGB_TRANSPARENT;
}

sal_Int32 ThemeBuffer::getSystemColor( sal_Int32 nElement, sal_Int32 nDefaultColor )
{
    sal_Int32 nColor = 0;
    return ClrScheme::getSystemColor( nColor, nElement ) ? nColor : nDefaultColor;
}

sal_Int32 ThemeBuffer::getSystemWindowColor()
{
    return getSystemColor( XML_window, 0xFFFFFF );
}

sal_Int32 ThemeBuffer::getSystemWindowTextColor()
{
    return getSystemColor( XML_windowText, 0x000000 );
}

// ============================================================================

} // namespace xls
} // namespace oox

