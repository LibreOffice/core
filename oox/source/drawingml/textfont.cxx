/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/drawingml/textfont.hxx"
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include "oox/drawingml/theme.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

// ============================================================================

namespace {

sal_Int16 lclGetFontPitch( sal_Int32 nOoxValue )
{
    using namespace ::com::sun::star::awt::FontPitch;
    static const sal_Int16 spnFontPitch[] = { DONTKNOW, FIXED, VARIABLE };
    return STATIC_ARRAY_SELECT( spnFontPitch, nOoxValue, DONTKNOW );
}

sal_Int16 lclGetFontFamily( sal_Int32 nOoxValue )
{
    using namespace ::com::sun::star::awt::FontFamily;
    static const sal_Int16 spnFontFamily[] = { DONTKNOW, ROMAN, SWISS, MODERN, SCRIPT, DECORATIVE };
    return STATIC_ARRAY_SELECT( spnFontFamily, nOoxValue, DONTKNOW );
}

} // namespace

// ============================================================================

TextFont::TextFont() :
    mnPitch( 0 ),
    mnCharset( WINDOWS_CHARSET_ANSI )
{
}

void TextFont::setAttributes( const AttributeList& rAttribs )
{
    maTypeface = rAttribs.getString( XML_typeface, OUString() );
    maPanose   = rAttribs.getString( XML_panose, OUString() );
    mnPitch    = rAttribs.getInteger( XML_pitchFamily, 0 );
    mnCharset  = rAttribs.getInteger( XML_charset, WINDOWS_CHARSET_DEFAULT );
}

void TextFont::assignIfUsed( const TextFont& rTextFont )
{
    if( rTextFont.maTypeface.getLength() > 0 )
        *this = rTextFont;
}

bool TextFont::getFontData( OUString& rFontName, sal_Int16 rnFontPitch, sal_Int16& rnFontFamily, const XmlFilterBase& rFilter ) const
{
    if( const Theme* pTheme = rFilter.getCurrentTheme() )
        if( const TextFont* pFont = pTheme->resolveFont( maTypeface ) )
            return pFont->implGetFontData( rFontName, rnFontPitch, rnFontFamily );
    return implGetFontData( rFontName, rnFontPitch, rnFontFamily );
}

bool TextFont::implGetFontData( OUString& rFontName, sal_Int16 rnFontPitch, sal_Int16& rnFontFamily ) const
{
    rFontName = maTypeface;
    rnFontPitch = lclGetFontPitch( extractValue< sal_Int16 >( mnPitch, 0, 4 ) );
    rnFontFamily = lclGetFontFamily( extractValue< sal_Int16 >( mnPitch, 4, 4 ) );
    return rFontName.getLength() > 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

