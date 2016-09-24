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

#include "oox/drawingml/theme.hxx"
#include <oox/token/tokens.hxx>
#include <drawingml/textcharacterproperties.hxx>

namespace oox {
namespace drawingml {

Theme::Theme()
{
}

Theme::~Theme()
{
}

namespace {

template< typename Type >
const Type* lclGetStyleElement( const RefVector< Type >& rVector, sal_Int32 nIndex )
{
    return (rVector.empty() || (nIndex < 1)) ? 0 :
        rVector.get( ::std::min( static_cast< sal_Int32 >( nIndex - 1 ), static_cast< sal_Int32 >( rVector.size() - 1 ) ) ).get();
}

} // namespace

const FillProperties* Theme::getFillStyle( sal_Int32 nIndex ) const
{
    return (nIndex >= 1000) ?
        lclGetStyleElement( maBgFillStyleList, nIndex - 1000 ) :
        lclGetStyleElement( maFillStyleList, nIndex );
}

const LineProperties* Theme::getLineStyle( sal_Int32 nIndex ) const
{
     return lclGetStyleElement( maLineStyleList, nIndex );
}

const EffectProperties* Theme::getEffectStyle( sal_Int32 nIndex ) const
{
    return lclGetStyleElement( maEffectStyleList, nIndex );
}

const TextCharacterProperties* Theme::getFontStyle( sal_Int32 nSchemeType ) const
{
    return maFontScheme.get( nSchemeType ).get();
}

const TextFont* Theme::resolveFont( const OUString& rName ) const
{
    const TextCharacterProperties* pCharProps = nullptr;
    /*  Resolves the following names:
        +mj-lt, +mj-ea, +mj-cs  --  major Latin, Asian, Complex font
        +mn-lt, +mn-ea, +mn-cs  --  minor Latin, Asian, Complex font
     */
    if( (rName.getLength() == 6) && (rName[ 0 ] == '+') && (rName[ 3 ] == '-') )
    {
        if( (rName[ 1 ] == 'm') && (rName[ 2 ] == 'j') )
            pCharProps = maFontScheme.get( XML_major ).get();
        else if( (rName[ 1 ] == 'm') && (rName[ 2 ] == 'n') )
            pCharProps = maFontScheme.get( XML_minor ).get();
        if( pCharProps )
        {
            if( (rName[ 4 ] == 'l') && (rName[ 5 ] == 't') )
                return &pCharProps->maLatinFont;
            if( (rName[ 4 ] == 'e') && (rName[ 5 ] == 'a') )
                return &pCharProps->maAsianFont;
            if( (rName[ 4 ] == 'c') && (rName[ 5 ] == 's') )
                return &pCharProps->maComplexFont;
        }
    }

    // See writerfilter::dmapper::ThemeTable::getFontNameForTheme().
    if (rName == "majorHAnsi" || rName == "majorAscii" || rName == "majorBidi" || rName == "majorEastAsia")
        pCharProps = maFontScheme.get(XML_major).get();
    else if (rName == "minorHAnsi" || rName == "minorAscii" || rName == "minorBidi" || rName == "minorEastAsia")
        pCharProps = maFontScheme.get(XML_minor).get();
    if (pCharProps)
    {
        if (rName == "majorAscii" || rName == "majorHAnsi" || rName == "minorAscii" || rName == "minorHAnsi")
            return &pCharProps->maLatinFont;
        else if (rName == "minorBidi" || rName == "majorBidi")
            return &pCharProps->maComplexFont;
        else if (rName == "minorEastAsia" || rName == "majorEastAsia")
            return &pCharProps->maAsianFont;
    }
    return nullptr;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
