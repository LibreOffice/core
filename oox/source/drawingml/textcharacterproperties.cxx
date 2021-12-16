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

#include <drawingml/textcharacterproperties.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/sequence.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/escapementitem.hxx>
#include <oox/helper/helper.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

using ::oox::core::XmlFilterBase;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace oox::drawingml {

void TextCharacterProperties::assignUsed( const TextCharacterProperties& rSourceProps )
{
    // overwrite all properties existing in rSourceProps
    maHyperlinkPropertyMap.assignUsed( rSourceProps.maHyperlinkPropertyMap );
    maLatinFont.assignIfUsed( rSourceProps.maLatinFont );
    maLatinThemeFont.assignIfUsed( rSourceProps.maLatinThemeFont );
    maAsianFont.assignIfUsed( rSourceProps.maAsianFont );
    maAsianThemeFont.assignIfUsed( rSourceProps.maAsianThemeFont );
    maComplexFont.assignIfUsed( rSourceProps.maComplexFont );
    maComplexThemeFont.assignIfUsed( rSourceProps.maComplexThemeFont );
    maSymbolFont.assignIfUsed( rSourceProps.maSymbolFont );
    maHighlightColor.assignIfUsed( rSourceProps.maHighlightColor );
    maUnderlineColor.assignIfUsed( rSourceProps.maUnderlineColor );
    moLang.assignIfUsed( rSourceProps.moLang );
    moHeight.assignIfUsed( rSourceProps.moHeight );
    moFontScale.assignIfUsed(rSourceProps.moFontScale);
    moSpacing.assignIfUsed( rSourceProps.moSpacing );
    moUnderline.assignIfUsed( rSourceProps.moUnderline );
    moBaseline.assignIfUsed( rSourceProps.moBaseline );
    moStrikeout.assignIfUsed( rSourceProps.moStrikeout );
    moCaseMap.assignIfUsed( rSourceProps.moCaseMap );
    moBold.assignIfUsed( rSourceProps.moBold );
    moItalic.assignIfUsed( rSourceProps.moItalic );
    moUnderlineLineFollowText.assignIfUsed( rSourceProps.moUnderlineLineFollowText );
    moUnderlineFillFollowText.assignIfUsed( rSourceProps.moUnderlineFillFollowText );
    moTextOutlineProperties.assignIfUsed(rSourceProps.moTextOutlineProperties);

    maTextEffectsProperties = rSourceProps.maTextEffectsProperties;
    maFillProperties.assignUsed( rSourceProps.maFillProperties );
}

void TextCharacterProperties::pushToPropMap( PropertyMap& rPropMap, const XmlFilterBase& rFilter ) const
{
    OUString aFontName;
    sal_Int16 nFontPitch = 0;
    sal_Int16 nFontFamily = 0;

    bool bRet = maLatinFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (!bRet)
        // In case there is no direct font, try to look it up as a theme reference.
        bRet = maLatinThemeFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (bRet)
    {
        rPropMap.setProperty( PROP_CharFontName, aFontName);
        rPropMap.setProperty( PROP_CharFontPitch, nFontPitch);
        rPropMap.setProperty( PROP_CharFontFamily, nFontFamily);
    }

    bRet = maAsianFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (!bRet)
        bRet = maAsianThemeFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (bRet)
    {
        rPropMap.setProperty( PROP_CharFontNameAsian, aFontName);
        rPropMap.setProperty( PROP_CharFontPitchAsian, nFontFamily);
        rPropMap.setProperty( PROP_CharFontFamilyAsian, nFontPitch);
    }

    bRet = maComplexFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (!bRet)
        bRet = maComplexThemeFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter );
    if (bRet)
    {
        rPropMap.setProperty( PROP_CharFontNameComplex, aFontName);
        rPropMap.setProperty( PROP_CharFontPitchComplex, nFontPitch);
        rPropMap.setProperty( PROP_CharFontFamilyComplex, nFontFamily);
    }

    if ( maFillProperties.moFillType.has() )
    {
        Color aColor = maFillProperties.getBestSolidColor();
        bool bContoured = false;

        // noFill doesn't exist for characters. Map noFill to 99% transparency
        if (maFillProperties.moFillType.get() == XML_noFill)
            aColor.addTransformation(XML_alpha, 1000);

        // tdf#137438 Emulate text outline color/transparency.
        // If the outline color dominates, then use it as the text color.
        if (moTextOutlineProperties.has()
            && moTextOutlineProperties.get().maLineFill.moFillType.has()
            && moTextOutlineProperties.get().maLineFill.moFillType.get() != XML_noFill)
        {
            Color aLineColor = moTextOutlineProperties.get().maLineFill.getBestSolidColor();
            sal_Int16 nLineTransparency = aLineColor.getTransparency();

            // tdf#127696 If the text color is white (and the outline color doesn't dominate),
            //            then this is contoured text in LO.
            if (nLineTransparency < aColor.getTransparency()
                || (bContoured = aColor.getColor(rFilter.getGraphicHelper()) == COL_WHITE))
                aColor = aLineColor;
        }
        rPropMap.setProperty(PROP_CharColor, aColor.getColor(rFilter.getGraphicHelper()));
        // set color theme index
        rPropMap.setProperty(PROP_CharColorTheme, aColor.getSchemeColorIndex());
        rPropMap.setProperty(PROP_CharColorTintOrShade, aColor.getTintOrShade());
        rPropMap.setProperty(PROP_CharColorLumMod, aColor.getLumMod());
        rPropMap.setProperty(PROP_CharColorLumOff, aColor.getLumOff());
        rPropMap.setProperty(PROP_CharContoured, bContoured);

        if (aColor.hasTransparency())
        {
            rPropMap.setProperty(PROP_CharTransparence, aColor.getTransparency());
        }
    }

    if( moLang.has() && !moLang.get().isEmpty() )
    {
        LanguageTag aTag(moLang.get());
        lang::Locale aLocale(aTag.getLocale());
        switch(MsLangId::getScriptType(aTag.getLanguageType()))
        {
            case css::i18n::ScriptType::LATIN:
                rPropMap.setProperty( PROP_CharLocale, aLocale);break;
            case css::i18n::ScriptType::ASIAN:
                rPropMap.setProperty( PROP_CharLocaleAsian, aLocale);break;
            case css::i18n::ScriptType::COMPLEX:
                rPropMap.setProperty( PROP_CharLocaleComplex, aLocale);break;
        }
    }

    if( moHeight.has() )
    {
        float fHeight = GetFontHeight( moHeight.get() );
        if (moFontScale.has())
            fHeight *= (moFontScale.get() / 100000);
        rPropMap.setProperty( PROP_CharHeight, fHeight);
        rPropMap.setProperty( PROP_CharHeightAsian, fHeight);
        rPropMap.setProperty( PROP_CharHeightComplex, fHeight);
    }

    rPropMap.setProperty( PROP_CharKerning, static_cast<sal_Int16>(GetTextSpacingPoint( moSpacing.get( 0 ) )));

    rPropMap.setProperty( PROP_CharUnderline, GetFontUnderline( moUnderline.get( XML_none ) ));
    rPropMap.setProperty( PROP_CharStrikeout, GetFontStrikeout( moStrikeout.get( XML_noStrike ) ));
    rPropMap.setProperty( PROP_CharCaseMap, GetCaseMap( moCaseMap.get( XML_none ) ));

    if( moBaseline.has() ) {
        rPropMap.setProperty( PROP_CharEscapement, sal_Int16(moBaseline.get( 0 ) / 1000));
        rPropMap.setProperty( PROP_CharEscapementHeight, sal_Int8(DFLT_ESC_PROP));
    } else {
        rPropMap.setProperty( PROP_CharEscapement, sal_Int16(0));
        rPropMap.setProperty( PROP_CharEscapementHeight, sal_Int8(100)); // 100%
    }

    float fWeight = moBold.get( false ) ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL;
    rPropMap.setProperty( PROP_CharWeight, fWeight);
    rPropMap.setProperty( PROP_CharWeightAsian, fWeight);
    rPropMap.setProperty( PROP_CharWeightComplex, fWeight);

    awt::FontSlant eSlant = moItalic.get( false ) ? awt::FontSlant_ITALIC : awt::FontSlant_NONE;
    rPropMap.setProperty( PROP_CharPosture, eSlant);
    rPropMap.setProperty( PROP_CharPostureAsian, eSlant);
    rPropMap.setProperty( PROP_CharPostureComplex, eSlant);

    bool bUnderlineFillFollowText = moUnderlineFillFollowText.get( false );
    if( moUnderline.has() && maUnderlineColor.isUsed() && !bUnderlineFillFollowText )
    {
        rPropMap.setProperty( PROP_CharUnderlineHasColor, true);
        rPropMap.setProperty( PROP_CharUnderlineColor, maUnderlineColor.getColor( rFilter.getGraphicHelper() ));
    }
    // TODO If bUnderlineFillFollowText uFillTx (CT_TextUnderlineFillFollowText) is set, fill color of the underline should be the same color as the text

    if (maHighlightColor.isUsed() && maHighlightColor.getTransparency() != 100)
        rPropMap.setProperty( PROP_CharBackColor, maHighlightColor.getColor( rFilter.getGraphicHelper() ));
    else
        rPropMap.setProperty( PROP_CharBackColor, sal_Int32(-1));
}

static void pushToGrabBag( PropertySet& rPropSet, const std::vector<PropertyValue>& aVectorOfPropertyValues )
{
    if (!rPropSet.hasProperty(PROP_CharInteropGrabBag) || aVectorOfPropertyValues.empty())
        return;
    Sequence<PropertyValue> aGrabBag;
    Any aAnyGrabBag = rPropSet.getAnyProperty(PROP_CharInteropGrabBag);
    aAnyGrabBag >>= aGrabBag;

    rPropSet.setAnyProperty(PROP_CharInteropGrabBag, makeAny(comphelper::concatSequences(aGrabBag, aVectorOfPropertyValues)));
}

void TextCharacterProperties::pushToPropSet( PropertySet& rPropSet, const XmlFilterBase& rFilter ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rFilter );
    rPropSet.setProperties( aPropMap );
    pushToGrabBag(rPropSet, maTextEffectsProperties);
}

float TextCharacterProperties::getCharHeightPoints( float fDefault ) const
{
    return moHeight.has() ? GetFontHeight( moHeight.get() ) : fDefault;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
