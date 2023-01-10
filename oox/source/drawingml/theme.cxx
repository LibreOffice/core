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

#include <oox/drawingml/theme.hxx>
#include <oox/token/tokens.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <comphelper/propertyvalue.hxx>
#include <sal/log.hxx>
#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/ColorSets.hxx>
#include <svx/unoapi.hxx>

using namespace com::sun::star;

namespace oox::drawingml {

namespace {

template< typename Type >
const Type* lclGetStyleElement( const RefVector< Type >& rVector, sal_Int32 nIndex )
{
    return (rVector.empty() || (nIndex < 1)) ? nullptr :
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

const TextFont* Theme::resolveFont( std::u16string_view rName ) const
{
    const TextCharacterProperties* pCharProps = nullptr;
    /*  Resolves the following names:
        +mj-lt, +mj-ea, +mj-cs  --  major Latin, Asian, Complex font
        +mn-lt, +mn-ea, +mn-cs  --  minor Latin, Asian, Complex font
     */
    if( (rName.size() == 6) && (rName[ 0 ] == '+') && (rName[ 3 ] == '-') )
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
    if (rName == u"majorHAnsi" || rName == u"majorAscii" || rName == u"majorBidi" || rName == u"majorEastAsia")
        pCharProps = maFontScheme.get(XML_major).get();
    else if (rName == u"minorHAnsi" || rName == u"minorAscii" || rName == u"minorBidi" || rName == u"minorEastAsia")
        pCharProps = maFontScheme.get(XML_minor).get();
    if (pCharProps)
    {
        if (rName == u"majorAscii" || rName == u"majorHAnsi" || rName == u"minorAscii" || rName == u"minorHAnsi")
            return &pCharProps->maLatinFont;
        else if (rName == u"minorBidi" || rName == u"majorBidi")
            return &pCharProps->maComplexFont;
        else if (rName == u"minorEastAsia" || rName == u"majorEastAsia")
            return &pCharProps->maAsianFont;
    }
    return nullptr;
}

std::unique_ptr<svx::Theme> Theme::createSvxTheme() const
{
    auto pTheme = std::make_unique<svx::Theme>(maThemeName);
    auto pColorSet = std::make_unique<svx::ColorSet>(maClrScheme.GetName());
    maClrScheme.fill(*pColorSet);
    pTheme->SetColorSet(std::move(pColorSet));

    svx::FontScheme aFontScheme(maFontSchemeName);

    if (auto* pCharProps = getFontStyle(XML_minor))
    {
        svx::ThemeFont aMinorLatin;
        pCharProps->maLatinFont.fillThemeFont(aMinorLatin);
        aFontScheme.setMinorLatin(aMinorLatin);

        svx::ThemeFont aMinorAsian;
        pCharProps->maAsianFont.fillThemeFont(aMinorAsian);
        aFontScheme.setMinorAsian(aMinorAsian);

        svx::ThemeFont aMinorComplex;
        pCharProps->maComplexFont.fillThemeFont(aMinorComplex);
        aFontScheme.setMinorComplex(aMinorComplex);
    }

    if (auto* pCharProps = getFontStyle(XML_major))
    {
        svx::ThemeFont aMajorLatin;
        pCharProps->maLatinFont.fillThemeFont(aMajorLatin);
        aFontScheme.setMajorLatin(aMajorLatin);

        svx::ThemeFont aMajorAsian;
        pCharProps->maAsianFont.fillThemeFont(aMajorAsian);
        aFontScheme.setMajorAsian(aMajorAsian);

        svx::ThemeFont aMajorComplex;
        pCharProps->maComplexFont.fillThemeFont(aMajorComplex);
        aFontScheme.setMajorComplex(aMajorComplex);
    }

    if (maSupplementalFontMap.find(XML_minor) != maSupplementalFontMap.cend())
    {
        std::vector<svx::ThemeSupplementalFont> aList;
        for (auto const& [rScript, rTypeface] : maSupplementalFontMap.at(XML_minor))
        {
            aList.push_back(svx::ThemeSupplementalFont{rScript, rTypeface});
        }
        aFontScheme.setMinorSupplementalFontList(aList);
    }

    if (maSupplementalFontMap.find(XML_major) != maSupplementalFontMap.cend())
    {
        std::vector<svx::ThemeSupplementalFont> aList;
        for (auto const& [rScript, rTypeface] : maSupplementalFontMap.at(XML_major))
        {
            aList.push_back(svx::ThemeSupplementalFont{rScript, rTypeface});
        }
        aFontScheme.setMajorSupplementalFontList(aList);
    }

    pTheme->setFontScheme(aFontScheme);

    return pTheme;
}

void Theme::addTheme(const css::uno::Reference<css::drawing::XDrawPage>& xDrawPage) const
{
    SAL_WARN_IF(!xDrawPage.is(), "oox", "DrawPage is not set");

    SdrPage* pPage = GetSdrPageFromXDrawPage(xDrawPage);

    SAL_WARN_IF(!pPage, "oox", "Can't get SdrPage from XDrawPage");

    if (!pPage)
        return;

    std::unique_ptr<svx::Theme> pTheme = createSvxTheme();

    pPage->getSdrPageProperties().SetTheme(std::move(pTheme));
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
