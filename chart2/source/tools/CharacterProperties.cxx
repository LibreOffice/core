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

#include <CharacterProperties.hxx>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <osl/diagnose.h>
#include <unotools/lingucfg.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void CharacterProperties::AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    // CharacterProperties
    rOutProperties.emplace_back( "CharFontName",
                  PROP_CHAR_FONT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "CharFontStyleName",
                  PROP_CHAR_FONT_STYLE_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharFontFamily (see awt.FontFamily)
    rOutProperties.emplace_back( "CharFontFamily",
                  PROP_CHAR_FONT_FAMILY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontCharSet (see awt.CharSet)
    rOutProperties.emplace_back( "CharFontCharSet",
                  PROP_CHAR_FONT_CHAR_SET,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontPitch (see awt.FontPitch)
    rOutProperties.emplace_back( "CharFontPitch",
                  PROP_CHAR_FONT_PITCH,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharColor
    rOutProperties.emplace_back( "CharColor",
                  PROP_CHAR_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharEscapement
    rOutProperties.emplace_back( "CharEscapement",
                  PROP_CHAR_ESCAPEMENT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharHeight
    rOutProperties.emplace_back( "CharHeight",
                  PROP_CHAR_CHAR_HEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharUnderline (see awt.FontUnderline)
    rOutProperties.emplace_back( "CharUnderline",
                  PROP_CHAR_UNDERLINE,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharUnderlineColor
    rOutProperties.emplace_back( "CharUnderlineColor",
                  PROP_CHAR_UNDERLINE_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharUnderlineHasColor
    rOutProperties.emplace_back( "CharUnderlineHasColor",
                  PROP_CHAR_UNDERLINE_HAS_COLOR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharOverline (see awt.FontUnderline)
    rOutProperties.emplace_back( "CharOverline",
                  PROP_CHAR_OVERLINE,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharOverlineColor
    rOutProperties.emplace_back( "CharOverlineColor",
                  PROP_CHAR_OVERLINE_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharOverlineHasColor
    rOutProperties.emplace_back( "CharOverlineHasColor",
                  PROP_CHAR_OVERLINE_HAS_COLOR,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharWeight (see awt.FontWeight)
    rOutProperties.emplace_back( "CharWeight",
                  PROP_CHAR_WEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharPosture
    rOutProperties.emplace_back( "CharPosture",
                  PROP_CHAR_POSTURE,
                  cppu::UnoType<awt::FontSlant>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "CharAutoKerning",
                  PROP_CHAR_AUTO_KERNING,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "CharKerning",
                  PROP_CHAR_KERNING,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharEscapementHeight
    rOutProperties.emplace_back( "CharEscapementHeight",
                  PROP_CHAR_ESCAPEMENT_HEIGHT,
                  cppu::UnoType<sal_Int8>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharStrikeout (see awt.FontStrikeout)
    rOutProperties.emplace_back( "CharStrikeout",
                  PROP_CHAR_STRIKE_OUT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharWordMode
    rOutProperties.emplace_back( "CharWordMode",
                  PROP_CHAR_WORD_MODE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharLocale
    rOutProperties.emplace_back( "CharLocale",
                  PROP_CHAR_LOCALE,
                  cppu::UnoType<lang::Locale>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );
    // CharShadowed
    rOutProperties.emplace_back( "CharShadowed",
                  PROP_CHAR_SHADOWED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharContoured
    rOutProperties.emplace_back( "CharContoured",
                  PROP_CHAR_CONTOURED,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharRelief (see text.FontRelief)
    rOutProperties.emplace_back( "CharRelief",
                  PROP_CHAR_RELIEF,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // CharEmphasize (see text.FontEmphasis)
    rOutProperties.emplace_back( "CharEmphasis",
                  PROP_CHAR_EMPHASIS,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharacterPropertiesAsian

    // CharFontNameAsian
    rOutProperties.emplace_back( "CharFontNameAsian",
                  PROP_CHAR_ASIAN_FONT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontStyleNameAsian
    rOutProperties.emplace_back( "CharFontStyleNameAsian",
                  PROP_CHAR_ASIAN_FONT_STYLE_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharFontFamilyAsian (see awt.FontFamily)
    rOutProperties.emplace_back( "CharFontFamilyAsian",
                  PROP_CHAR_ASIAN_FONT_FAMILY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontCharSetAsian (see awt.CharSet)
    rOutProperties.emplace_back( "CharFontCharSetAsian",
                  PROP_CHAR_ASIAN_CHAR_SET,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontPitchAsian (see awt.FontPitch)
    rOutProperties.emplace_back( "CharFontPitchAsian",
                  PROP_CHAR_ASIAN_FONT_PITCH,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharHeightAsian
    rOutProperties.emplace_back( "CharHeightAsian",
                  PROP_CHAR_ASIAN_CHAR_HEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharWeightAsian
    rOutProperties.emplace_back( "CharWeightAsian",
                  PROP_CHAR_ASIAN_WEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharPostureAsian
    rOutProperties.emplace_back( "CharPostureAsian",
                  PROP_CHAR_ASIAN_POSTURE,
                  cppu::UnoType<awt::FontSlant>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharLocaleAsian
    rOutProperties.emplace_back( "CharLocaleAsian",
                  PROP_CHAR_ASIAN_LOCALE,
                  cppu::UnoType<lang::Locale>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );

    // CharacterPropertiesComplex

    // CharFontNameComplex
    rOutProperties.emplace_back( "CharFontNameComplex",
                  PROP_CHAR_COMPLEX_FONT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontStyleNameComplex
    rOutProperties.emplace_back( "CharFontStyleNameComplex",
                  PROP_CHAR_COMPLEX_FONT_STYLE_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    // CharFontFamilyComplex (see awt.FontFamily)
    rOutProperties.emplace_back( "CharFontFamilyComplex",
                  PROP_CHAR_COMPLEX_FONT_FAMILY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontCharSetComplex (see awt.CharSet)
    rOutProperties.emplace_back( "CharFontCharSetComplex",
                  PROP_CHAR_COMPLEX_CHAR_SET,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharFontPitchComplex (see awt.FontPitch)
    rOutProperties.emplace_back( "CharFontPitchComplex",
                  PROP_CHAR_COMPLEX_FONT_PITCH,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharHeightComplex
    rOutProperties.emplace_back( "CharHeightComplex",
                  PROP_CHAR_COMPLEX_CHAR_HEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharWeightComplex
    rOutProperties.emplace_back( "CharWeightComplex",
                  PROP_CHAR_COMPLEX_WEIGHT,
                  cppu::UnoType<float>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharPostureComplex
    rOutProperties.emplace_back( "CharPostureComplex",
                  PROP_CHAR_COMPLEX_POSTURE,
                  cppu::UnoType<awt::FontSlant>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // CharLocaleComplex
    rOutProperties.emplace_back( "CharLocaleComplex",
                  PROP_CHAR_COMPLEX_LOCALE,
                  cppu::UnoType<lang::Locale>::get(),
                  //#i111967# no PropertyChangeEvent is fired on change so far
                  beans::PropertyAttribute::MAYBEDEFAULT );

    // Writing Mode left to right vs right to left
    rOutProperties.emplace_back( "WritingMode",
                  PROP_WRITING_MODE,
                  cppu::UnoType<sal_Int16>::get(), /*css::text::WritingMode2*/
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ParaIsCharacterDistance",
                  PROP_PARA_IS_CHARACTER_DISTANCE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void CharacterProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    SolarMutexGuard aGuard;
    const float fDefaultFontHeight = 13.0;

    SvtLinguConfig aLinguConfig;
    lang::Locale aDefaultLocale;
    aLinguConfig.GetProperty(u"DefaultLocale") >>= aDefaultLocale;
    lang::Locale aDefaultLocale_CJK;
    aLinguConfig.GetProperty(u"DefaultLocale_CJK") >>= aDefaultLocale_CJK;
    lang::Locale aDefaultLocale_CTL;
    aLinguConfig.GetProperty(u"DefaultLocale_CTL") >>= aDefaultLocale_CTL;

    using namespace ::com::sun::star::i18n::ScriptType;
    LanguageType nLang;
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale, false), LATIN);
    vcl::Font aFont = OutputDevice::GetDefaultFont( DefaultFontType::LATIN_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne );
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale_CJK, false), ASIAN);
    vcl::Font aFontCJK = OutputDevice::GetDefaultFont( DefaultFontType::CJK_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne );
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale_CTL, false), COMPLEX);
    vcl::Font aFontCTL = OutputDevice::GetDefaultFont( DefaultFontType::CTL_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_NAME, aFont.GetFamilyName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_STYLE_NAME, aFont.GetStyleName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_FAMILY, sal_Int16(aFont.GetFamilyType()) );//awt::FontFamily::SWISS
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_CHAR_SET, sal_Int16(aFont.GetCharSet()) );//use awt::CharSet::DONTKNOW instead of SYSTEM to avoid assertion issue 50249
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_PITCH, sal_Int16(aFont.GetPitch()) );//awt::FontPitch::VARIABLE
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COLOR, COL_AUTO ); //automatic color
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_UNDERLINE, awt::FontUnderline::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_UNDERLINE_COLOR, COL_AUTO ); //automatic color
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_UNDERLINE_HAS_COLOR, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_OVERLINE, awt::FontUnderline::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_OVERLINE_COLOR, COL_AUTO ); //automatic color
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_OVERLINE_HAS_COLOR, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_AUTO_KERNING, true );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_KERNING, sal_Int16(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_STRIKE_OUT, awt::FontStrikeout::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_WORD_MODE, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_LOCALE, aDefaultLocale );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_SHADOWED, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_CONTOURED, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_RELIEF, text::FontRelief::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_EMPHASIS, text::FontEmphasis::NONE );

    // Asian (com.sun.star.style.CharacterPropertiesAsian)
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_LOCALE, aDefaultLocale_CJK );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_NAME, aFontCJK.GetFamilyName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_STYLE_NAME, aFontCJK.GetStyleName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_FAMILY, sal_Int16(aFontCJK.GetFamilyType()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_CHAR_SET, sal_Int16(aFontCJK.GetCharSet()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_PITCH, sal_Int16(aFontCJK.GetPitch()) );

    // Complex Text Layout (com.sun.star.style.CharacterPropertiesComplex)
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_LOCALE, aDefaultLocale_CTL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_NAME, aFontCTL.GetFamilyName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_STYLE_NAME, aFontCTL.GetStyleName() );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_FAMILY, sal_Int16(aFontCTL.GetFamilyType()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_CHAR_SET, sal_Int16(aFontCTL.GetCharSet()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_PITCH, sal_Int16(aFontCTL.GetPitch()) );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_WRITING_MODE, sal_Int16( css::text::WritingMode2::PAGE ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PARA_IS_CHARACTER_DISTANCE, true );
}

bool CharacterProperties::IsCharacterPropertyHandle( sal_Int32 nHandle )
{
    return ( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
             nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );
}

awt::FontDescriptor CharacterProperties::createFontDescriptorFromPropertySet(
    const uno::Reference< beans::XMultiPropertySet > & xMultiPropSet )
{
    awt::FontDescriptor aResult;
    // Note: keep this sorted!
    uno::Sequence< OUString > aPropNameSeq{
        u"CharFontCharSet"_ustr,   // CharSet
        u"CharFontFamily"_ustr,    // Family
        u"CharFontName"_ustr,      // Name
        u"CharFontPitch"_ustr,     // Pitch
        u"CharFontStyleName"_ustr, // StyleName
        u"CharHeight"_ustr,        // Height
        u"CharPosture"_ustr,       // Slant
        u"CharStrikeout"_ustr,     // Strikeout
        u"CharUnderline"_ustr,     // Underline
        u"CharWeight"_ustr,        // Weight
        u"CharWordMode"_ustr};     // WordLineMode
    uno::Sequence< uno::Any > aValues( xMultiPropSet->getPropertyValues( aPropNameSeq ));

    sal_Int32 i=0;
    // Note keep this sorted according to the list above (comments are the fieldnames)
    aValues[ i++ ]  >>= aResult.CharSet;
    aValues[ i++ ]  >>= aResult.Family;
    aValues[ i++ ]  >>= aResult.Name;
    aValues[ i++ ]  >>= aResult.Pitch;
    aValues[ i++ ]  >>= aResult.StyleName;
    float fCharHeight = 0;
    aValues[ i++ ]  >>= fCharHeight;
    aResult.Height = static_cast< sal_Int16 >( fCharHeight );
    aValues[ i++ ]  >>= aResult.Slant;
    aValues[ i++ ]  >>= aResult.Strikeout;
    aValues[ i++ ]  >>= aResult.Underline;
    aValues[ i++ ]  >>= aResult.Weight;
    aValues[ i++ ]  >>= aResult.WordLineMode;
    OSL_ASSERT( i == aValues.getLength());

    return aResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
