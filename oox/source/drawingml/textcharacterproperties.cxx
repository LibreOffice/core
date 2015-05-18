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

#include "drawingml/textcharacterproperties.hxx"
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <editeng/escapementitem.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/token/tokens.hxx"

using ::oox::core::XmlFilterBase;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace oox {
namespace drawingml {

void TextCharacterProperties::assignUsed( const TextCharacterProperties& rSourceProps )
{
    // overwrite all properties exisiting in rSourceProps
    maHyperlinkPropertyMap.assignUsed( rSourceProps.maHyperlinkPropertyMap );
    maLatinFont.assignIfUsed( rSourceProps.maLatinFont );
    maLatinThemeFont.assignIfUsed( rSourceProps.maLatinThemeFont );
    maAsianFont.assignIfUsed( rSourceProps.maAsianFont );
    maAsianThemeFont.assignIfUsed( rSourceProps.maAsianThemeFont );
    maComplexFont.assignIfUsed( rSourceProps.maComplexFont );
    maComplexThemeFont.assignIfUsed( rSourceProps.maComplexThemeFont );
    maSymbolFont.assignIfUsed( rSourceProps.maSymbolFont );
    maCharColor.assignIfUsed( rSourceProps.maCharColor );
    maHighlightColor.assignIfUsed( rSourceProps.maHighlightColor );
    maUnderlineColor.assignIfUsed( rSourceProps.maUnderlineColor );
    moHeight.assignIfUsed( rSourceProps.moHeight );
    moSpacing.assignIfUsed( rSourceProps.moSpacing );
    moUnderline.assignIfUsed( rSourceProps.moUnderline );
    moBaseline.assignIfUsed( rSourceProps.moBaseline );
    moStrikeout.assignIfUsed( rSourceProps.moStrikeout );
    moCaseMap.assignIfUsed( rSourceProps.moCaseMap );
    moBold.assignIfUsed( rSourceProps.moBold );
    moItalic.assignIfUsed( rSourceProps.moItalic );
    moUnderlineLineFollowText.assignIfUsed( rSourceProps.moUnderlineLineFollowText );
    moUnderlineFillFollowText.assignIfUsed( rSourceProps.moUnderlineFillFollowText );

    maTextEffectsProperties = rSourceProps.maTextEffectsProperties;
    maGradientProps.assignUsed( rSourceProps.maGradientProps );
}

void TextCharacterProperties::pushToPropMap( PropertyMap& rPropMap, const XmlFilterBase& rFilter, bool bUseOptional ) const
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

    // symbolfont, will now be ... textrun.cxx ... ausgewertet !!!i#113673

    if( maCharColor.isUsed() )
        rPropMap.setProperty( PROP_CharColor, maCharColor.getColor( rFilter.getGraphicHelper() ));
    if( maGradientProps.maGradientStops.size() > 0 )
        rPropMap.setProperty( PROP_CharColor, maGradientProps.maGradientStops.begin()->second.getColor( rFilter.getGraphicHelper() ));

    if( moLang.has() && !moLang.get().isEmpty() )
    {
        lang::Locale aLocale( LanguageTag( moLang.get()).getLocale());
        rPropMap.setProperty( PROP_CharLocale, aLocale);
        rPropMap.setProperty( PROP_CharLocaleAsian, aLocale);
        rPropMap.setProperty( PROP_CharLocaleComplex, aLocale);
    }

    if( moHeight.has() )
    {
        float fHeight = GetFontHeight( moHeight.get() );
        rPropMap.setProperty( PROP_CharHeight, fHeight);
        rPropMap.setProperty( PROP_CharHeightAsian, fHeight);
        rPropMap.setProperty( PROP_CharHeightComplex, fHeight);
    }

    rPropMap.setProperty( PROP_CharKerning, (sal_Int16) GetTextSpacingPoint( moSpacing.get( 0 ) ));

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

    if( !bUseOptional || moBold.has() ) {
        float fWeight = moBold.get( false ) ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL;
        rPropMap.setProperty( PROP_CharWeight, fWeight);
        rPropMap.setProperty( PROP_CharWeightAsian, fWeight);
        rPropMap.setProperty( PROP_CharWeightComplex, fWeight);
    }

    if( !bUseOptional || moItalic.has() ) {
        awt::FontSlant eSlant = moItalic.get( false ) ? awt::FontSlant_ITALIC : awt::FontSlant_NONE;
        rPropMap.setProperty( PROP_CharPosture, eSlant);
        rPropMap.setProperty( PROP_CharPostureAsian, eSlant);
        rPropMap.setProperty( PROP_CharPostureComplex, eSlant);
    }

    bool bUnderlineFillFollowText = moUnderlineFillFollowText.get( false );
    if( moUnderline.has() && maUnderlineColor.isUsed() && !bUnderlineFillFollowText )
    {
        rPropMap.setProperty( PROP_CharUnderlineHasColor, true);
        rPropMap.setProperty( PROP_CharUnderlineColor, maUnderlineColor.getColor( rFilter.getGraphicHelper() ));
    }
}

void pushToGrabBag( PropertySet& rPropSet, const std::vector<PropertyValue>& aVectorOfProperyValues )
{
    if (!rPropSet.hasProperty(PROP_CharInteropGrabBag) || aVectorOfProperyValues.empty())
        return;
    Sequence<PropertyValue> aGrabBag;
    Any aAnyGrabBag = rPropSet.getAnyProperty(PROP_CharInteropGrabBag);
    aAnyGrabBag >>= aGrabBag;

    sal_Int32 nLength = aGrabBag.getLength();
    aGrabBag.realloc(nLength + aVectorOfProperyValues.size());

    for (size_t i = 0; i < aVectorOfProperyValues.size(); i++)
    {
        PropertyValue aPropertyValue = aVectorOfProperyValues[i];
        aGrabBag[nLength + i] = aPropertyValue;
    }

    rPropSet.setAnyProperty(PROP_CharInteropGrabBag, makeAny(aGrabBag));
}

void TextCharacterProperties::pushToPropSet( PropertySet& rPropSet, const XmlFilterBase& rFilter, bool bUseOptional ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rFilter, bUseOptional );
    rPropSet.setProperties( aPropMap );
    pushToGrabBag(rPropSet, maTextEffectsProperties);
}

float TextCharacterProperties::getCharHeightPoints( float fDefault ) const
{
    return moHeight.has() ? GetFontHeight( moHeight.get() ) : fDefault;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
