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

#include "drawingml/textfont.hxx"
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include "oox/drawingml/theme.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/attributelist.hxx"

using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

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

}

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

void TextFont::setAttributes( const OUString& sFontName )
{
    maTypeface = sFontName;
    maPanose.clear();
    mnPitch = 0;
    mnCharset = WINDOWS_CHARSET_DEFAULT;
}

void TextFont::assignIfUsed( const TextFont& rTextFont )
{
    if( !rTextFont.maTypeface.isEmpty() )
        *this = rTextFont;
}

bool TextFont::getFontData( OUString& rFontName, sal_Int16& rnFontPitch, sal_Int16& rnFontFamily, const XmlFilterBase& rFilter ) const
{
    if( const Theme* pTheme = rFilter.getCurrentTheme() )
        if( const TextFont* pFont = pTheme->resolveFont( maTypeface ) )
            return pFont->implGetFontData( rFontName, rnFontPitch, rnFontFamily );
    return implGetFontData( rFontName, rnFontPitch, rnFontFamily );
}

bool TextFont::implGetFontData( OUString& rFontName, sal_Int16& rnFontPitch, sal_Int16& rnFontFamily ) const
{
    rFontName = maTypeface;
    rnFontPitch = lclGetFontPitch( extractValue< sal_Int16 >( mnPitch, 0, 4 ) );
    rnFontFamily = lclGetFontFamily( extractValue< sal_Int16 >( mnPitch, 4, 4 ) );
    return !rFontName.isEmpty();
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
