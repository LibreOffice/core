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

#include <drawingml/textfont.hxx>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <oox/drawingml/theme.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/tokens.hxx>
#include <docmodel/theme/Theme.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

using ::oox::core::XmlFilterBase;

namespace oox::drawingml {

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

TextFont::TextFont() :
    mnPitchFamily(0),
    mnCharset( WINDOWS_CHARSET_ANSI )
{
}

void TextFont::setAttributes( const AttributeList& rAttribs )
{
    auto device = Application::GetDefaultDevice();
    maTypeface = rAttribs.getStringDefaulted( XML_typeface);
    if (!device->IsFontAvailable(maTypeface)) {
        // If the font is not available, try to find the best match
        maTypeface = device->FindBestMatchingFont(maTypeface);
    }
    maPanose = rAttribs.getStringDefaulted( XML_panose);
    mnPitchFamily = rAttribs.getInteger( XML_pitchFamily, 0 );
    mnCharset = rAttribs.getInteger( XML_charset, WINDOWS_CHARSET_DEFAULT );
}

void TextFont::setAttributes( const OUString& sFontName )
{
    auto device = Application::GetDefaultDevice();
    maTypeface = sFontName;
    if (!device->IsFontAvailable(maTypeface)) {
        // If the font is not available, try to find the best match
        maTypeface = device->FindBestMatchingFont(maTypeface);
    }
    maPanose.clear();
    mnPitchFamily = 0;
    mnCharset = WINDOWS_CHARSET_DEFAULT;
}

void TextFont::assignIfUsed( const TextFont& rTextFont )
{
    if( !rTextFont.maTypeface.isEmpty() )
        *this = rTextFont;
}

bool TextFont::getFontData( OUString& rFontName, sal_Int16& rnFontPitch, sal_Int16& rnFontFamily, bool* pbSymbol, const XmlFilterBase& rFilter ) const
{
    if( const Theme* pTheme = rFilter.getCurrentTheme() )
        if( const TextFont* pFont = pTheme->resolveFont( maTypeface ) )
            return pFont->implGetFontData( rFontName, rnFontPitch, rnFontFamily, pbSymbol );
    return implGetFontData( rFontName, rnFontPitch, rnFontFamily, pbSymbol );
}

bool TextFont::implGetFontData( OUString& rFontName, sal_Int16& rnFontPitch, sal_Int16& rnFontFamily, bool* pbSymbol ) const
{
    rFontName = maTypeface;
    resolvePitch(mnPitchFamily, rnFontPitch, rnFontFamily);
    if (pbSymbol)
        *pbSymbol = mnCharset == WINDOWS_CHARSET_SYMBOL;
    return !rFontName.isEmpty();
}

void TextFont::fillThemeFont(model::ThemeFont& rThemeFont) const
{
    rThemeFont.maTypeface = maTypeface;
    rThemeFont.maPanose = maPanose;
    rThemeFont.maCharset = mnCharset;
    resolvePitch(mnPitchFamily, rThemeFont.maPitch, rThemeFont.maFamily);
}

void TextFont::resolvePitch(sal_Int32 nOoxPitchFamily, sal_Int16& rnFontPitch, sal_Int16& rnFontFamily)
{
    rnFontPitch = lclGetFontPitch(extractValue<sal_Int16>(nOoxPitchFamily, 0, 4));
    rnFontFamily = lclGetFontFamily(extractValue<sal_Int16>(nOoxPitchFamily, 4, 4));
}


} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
