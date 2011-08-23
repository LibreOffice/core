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

#include "oox/drawingml/textfont.hxx"
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include "oox/drawingml/theme.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"
#include "tokens.hxx"

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

