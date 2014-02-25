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

#include "oox/drawingml/textcharacterproperties.hxx"
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
    maHyperlinkPropertyMap.insert( rSourceProps.maHyperlinkPropertyMap.begin(), rSourceProps.maHyperlinkPropertyMap.end() );
    maLatinFont.assignIfUsed( rSourceProps.maLatinFont );
    maAsianFont.assignIfUsed( rSourceProps.maAsianFont );
    maComplexFont.assignIfUsed( rSourceProps.maComplexFont );
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
}

void TextCharacterProperties::pushToPropMap( PropertyMap& rPropMap, const XmlFilterBase& rFilter, bool bUseOptional ) const
{
    OUString aFontName;
    sal_Int16 nFontPitch = 0;
    sal_Int16 nFontFamily = 0;

    if( maLatinFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter ) )
    {
        rPropMap[ PROP_CharFontName ] <<= aFontName;
        rPropMap[ PROP_CharFontPitch ] <<= nFontPitch;
        rPropMap[ PROP_CharFontFamily ] <<= nFontFamily;
    }

    if( maAsianFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter ) )
    {
        rPropMap[ PROP_CharFontNameAsian ] <<= aFontName;
        rPropMap[ PROP_CharFontPitchAsian ] <<= nFontFamily;
        rPropMap[ PROP_CharFontFamilyAsian ] <<= nFontPitch;
    }

    if( maComplexFont.getFontData( aFontName, nFontPitch, nFontFamily, rFilter ) )
    {
        rPropMap[ PROP_CharFontNameComplex ] <<= aFontName;
        rPropMap[ PROP_CharFontPitchComplex ] <<= nFontPitch;
        rPropMap[ PROP_CharFontFamilyComplex ] <<= nFontFamily;
    }

    // symbolfont, will now be ... textrun.cxx ... ausgewertet !!!i#113673

    if( maCharColor.isUsed() )
        rPropMap[ PROP_CharColor ] <<= maCharColor.getColor( rFilter.getGraphicHelper() );

    if( moLang.has() && !moLang.get().isEmpty() )
    {
        lang::Locale aLocale( LanguageTag( moLang.get()).getLocale());
        rPropMap[ PROP_CharLocale ] <<= aLocale;
        rPropMap[ PROP_CharLocaleAsian ] <<= aLocale;
        rPropMap[ PROP_CharLocaleComplex ] <<= aLocale;
    }

    if( moHeight.has() )
    {
        float fHeight = GetFontHeight( moHeight.get() );
        rPropMap[ PROP_CharHeight ] <<= fHeight;
        rPropMap[ PROP_CharHeightAsian ] <<= fHeight;
        rPropMap[ PROP_CharHeightComplex ] <<= fHeight;
    }

    rPropMap[ PROP_CharKerning ] <<= (sal_Int16) GetTextSpacingPoint( moSpacing.get( 0 ) );

    rPropMap[ PROP_CharUnderline ] <<= GetFontUnderline( moUnderline.get( XML_none ) );
    rPropMap[ PROP_CharStrikeout ] <<= GetFontStrikeout( moStrikeout.get( XML_noStrike ) );
    rPropMap[ PROP_CharCaseMap ] <<= GetCaseMap( moCaseMap.get( XML_none ) );

    if( moBaseline.has() ) {
        rPropMap[ PROP_CharEscapement ] <<= sal_Int16(moBaseline.get( 0 ) / 1000);
        rPropMap[ PROP_CharEscapementHeight ] <<= sal_Int8(DFLT_ESC_PROP);
    } else {
        rPropMap[ PROP_CharEscapement ] <<= sal_Int16(0);
        rPropMap[ PROP_CharEscapementHeight ] <<= sal_Int8(100); // 100%
    }

    if( !bUseOptional || moBold.has() ) {
        float fWeight = moBold.get( false ) ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL;
        rPropMap[ PROP_CharWeight ] <<= fWeight;
        rPropMap[ PROP_CharWeightAsian ] <<= fWeight;
        rPropMap[ PROP_CharWeightComplex ] <<= fWeight;
    }

    if( !bUseOptional || moItalic.has() ) {
        awt::FontSlant eSlant = moItalic.get( false ) ? awt::FontSlant_ITALIC : awt::FontSlant_NONE;
        rPropMap[ PROP_CharPosture ] <<= eSlant;
        rPropMap[ PROP_CharPostureAsian ] <<= eSlant;
        rPropMap[ PROP_CharPostureComplex ] <<= eSlant;
    }

    bool bUnderlineFillFollowText = moUnderlineFillFollowText.get( false );
    if( moUnderline.has() && maUnderlineColor.isUsed() && !bUnderlineFillFollowText )
    {
        rPropMap[ PROP_CharUnderlineHasColor ] <<= true;
        rPropMap[ PROP_CharUnderlineColor ] <<= maUnderlineColor.getColor( rFilter.getGraphicHelper() );
    }
}

    void TextCharacterProperties::pushToPropSet( PropertySet& rPropSet, const XmlFilterBase& rFilter, bool bUseOptional ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rFilter, bUseOptional );
    rPropSet.setProperties( aPropMap );
}

float TextCharacterProperties::getCharHeightPoints( float fDefault ) const
{
    return moHeight.has() ? GetFontHeight( moHeight.get() ) : fDefault;
}



} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
