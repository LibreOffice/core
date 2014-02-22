/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "CharacterProperties.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/text/FontRelief.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/RubyAdjust.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <comphelper/InlineContainer.hxx>


#include <unotools/lingucfg.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/outdev.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void CharacterProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    
    rOutProperties.push_back(
        Property( "CharFontName",
                  PROP_CHAR_FONT_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "CharFontStyleName",
                  PROP_CHAR_FONT_STYLE_NAME,
                  ::getCppuType( reinterpret_cast< const  OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharFontFamily",
                  PROP_CHAR_FONT_FAMILY,
                  ::getCppuType( reinterpret_cast< const  sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontCharSet",
                  PROP_CHAR_FONT_CHAR_SET,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontPitch",
                  PROP_CHAR_FONT_PITCH,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharColor",
                  PROP_CHAR_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharEscapement",
                  PROP_CHAR_ESCAPEMENT,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharHeight",
                  PROP_CHAR_CHAR_HEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharUnderline",
                  PROP_CHAR_UNDERLINE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharUnderlineColor",
                  PROP_CHAR_UNDERLINE_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharUnderlineHasColor",
                  PROP_CHAR_UNDERLINE_HAS_COLOR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharOverline",
                  PROP_CHAR_OVERLINE,
                  ::getCppuType( reinterpret_cast< const sal_Int16* >( 0 ) ),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ) );
    
    rOutProperties.push_back(
        Property( "CharOverlineColor",
                  PROP_CHAR_OVERLINE_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32* >( 0 ) ),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ) );
    
    rOutProperties.push_back(
        Property( "CharOverlineHasColor",
                  PROP_CHAR_OVERLINE_HAS_COLOR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ) );
    
    rOutProperties.push_back(
        Property( "CharWeight",
                  PROP_CHAR_WEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharPosture",
                  PROP_CHAR_POSTURE,
                  ::getCppuType( reinterpret_cast< const awt::FontSlant * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "CharAutoKerning",
                  PROP_CHAR_AUTO_KERNING,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "CharKerning",
                  PROP_CHAR_KERNING,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharEscapementHeight",
                  PROP_CHAR_ESCAPEMENT_HEIGHT,
                  ::getCppuType( reinterpret_cast< const sal_Int8 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharStrikeout",
                  PROP_CHAR_STRIKE_OUT,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharWordMode",
                  PROP_CHAR_WORD_MODE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharLocale",
                  PROP_CHAR_LOCALE,
                  ::getCppuType( reinterpret_cast< const lang::Locale * >(0)),
                  
                  beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharShadowed",
                  PROP_CHAR_SHADOWED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharContoured",
                  PROP_CHAR_CONTOURED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharRelief",
                  PROP_CHAR_RELIEF,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    
    rOutProperties.push_back(
        Property( "CharEmphasis",
                  PROP_CHAR_EMPHASIS,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    
    
    rOutProperties.push_back(
        Property( "CharFontNameAsian",
                  PROP_CHAR_ASIAN_FONT_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontStyleNameAsian",
                  PROP_CHAR_ASIAN_FONT_STYLE_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharFontFamilyAsian",
                  PROP_CHAR_ASIAN_FONT_FAMILY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontCharSetAsian",
                  PROP_CHAR_ASIAN_CHAR_SET,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontPitchAsian",
                  PROP_CHAR_ASIAN_FONT_PITCH,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharHeightAsian",
                  PROP_CHAR_ASIAN_CHAR_HEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharWeightAsian",
                  PROP_CHAR_ASIAN_WEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharPostureAsian",
                  PROP_CHAR_ASIAN_POSTURE,
                  ::getCppuType( reinterpret_cast< const awt::FontSlant * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharLocaleAsian",
                  PROP_CHAR_ASIAN_LOCALE,
                  ::getCppuType( reinterpret_cast< const lang::Locale * >(0)),
                  
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    
    
    
    rOutProperties.push_back(
        Property( "CharFontNameComplex",
                  PROP_CHAR_COMPLEX_FONT_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontStyleNameComplex",
                  PROP_CHAR_COMPLEX_FONT_STYLE_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    
    rOutProperties.push_back(
        Property( "CharFontFamilyComplex",
                  PROP_CHAR_COMPLEX_FONT_FAMILY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontCharSetComplex",
                  PROP_CHAR_COMPLEX_CHAR_SET,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharFontPitchComplex",
                  PROP_CHAR_COMPLEX_FONT_PITCH,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharHeightComplex",
                  PROP_CHAR_COMPLEX_CHAR_HEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharWeightComplex",
                  PROP_CHAR_COMPLEX_WEIGHT,
                  ::getCppuType( reinterpret_cast< const float * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharPostureComplex",
                  PROP_CHAR_COMPLEX_POSTURE,
                  ::getCppuType( reinterpret_cast< const awt::FontSlant * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    
    rOutProperties.push_back(
        Property( "CharLocaleComplex",
                  PROP_CHAR_COMPLEX_LOCALE,
                  ::getCppuType( reinterpret_cast< const lang::Locale * >(0)),
                  
                  beans::PropertyAttribute::MAYBEDEFAULT ));

    
    rOutProperties.push_back(
        Property( "WritingMode",
                  PROP_WRITING_MODE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)), /*com::sun::star::text::WritingMode2*/
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ParaIsCharacterDistance",
                  PROP_PARA_IS_CHARACTER_DISTANCE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void CharacterProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    const float fDefaultFontHeight = 13.0;

    SvtLinguConfig aLinguConfig;
    lang::Locale aDefaultLocale;
    aLinguConfig.GetProperty("DefaultLocale") >>= aDefaultLocale;
    lang::Locale aDefaultLocale_CJK;
    aLinguConfig.GetProperty("DefaultLocale_CJK") >>= aDefaultLocale_CJK;
    lang::Locale aDefaultLocale_CTL;
    aLinguConfig.GetProperty("DefaultLocale_CTL") >>= aDefaultLocale_CTL;

    using namespace ::com::sun::star::i18n::ScriptType;
    LanguageType nLang;
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale, false), LATIN);
    Font aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_SPREADSHEET, nLang, DEFAULTFONT_FLAGS_ONLYONE, 0 );
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale_CJK, false), ASIAN);
    Font aFontCJK = OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_SPREADSHEET, nLang, DEFAULTFONT_FLAGS_ONLYONE, 0 );
    nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType( aDefaultLocale_CTL, false), COMPLEX);
    Font aFontCTL = OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_SPREADSHEET, nLang, DEFAULTFONT_FLAGS_ONLYONE, 0 );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_NAME, OUString( aFont.GetName() ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_STYLE_NAME, OUString(aFont.GetStyleName()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_FAMILY, sal_Int16(aFont.GetFamily()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_CHAR_SET, sal_Int16(aFont.GetCharSet()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_FONT_PITCH, sal_Int16(aFont.GetPitch()) );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_CHAR_COLOR, -1 ); 
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_UNDERLINE, awt::FontUnderline::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_CHAR_UNDERLINE_COLOR, -1 ); 
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_UNDERLINE_HAS_COLOR, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_OVERLINE, awt::FontUnderline::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_CHAR_OVERLINE_COLOR, -1 ); 
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_OVERLINE_HAS_COLOR, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_AUTO_KERNING, true );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_CHAR_KERNING, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_CHAR_STRIKE_OUT, awt::FontStrikeout::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_WORD_MODE, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_LOCALE, aDefaultLocale );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_SHADOWED, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_CONTOURED, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_RELIEF, text::FontRelief::NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_EMPHASIS, text::FontEmphasis::NONE );

    
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_LOCALE, aDefaultLocale_CJK );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_NAME, OUString( aFontCJK.GetName() ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_STYLE_NAME, OUString(aFontCJK.GetStyleName()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_FAMILY, sal_Int16(aFontCJK.GetFamily()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_CHAR_SET, sal_Int16(aFontCJK.GetCharSet()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_ASIAN_FONT_PITCH, sal_Int16(aFontCJK.GetPitch()) );

    
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultFontHeight );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_WEIGHT, awt::FontWeight::NORMAL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_POSTURE, awt::FontSlant_NONE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_LOCALE, aDefaultLocale_CTL );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_NAME, OUString( aFontCTL.GetName() ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_STYLE_NAME, OUString(aFontCTL.GetStyleName()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_FAMILY, sal_Int16(aFontCTL.GetFamily()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_CHAR_SET, sal_Int16(aFontCTL.GetCharSet()) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_CHAR_COMPLEX_FONT_PITCH, sal_Int16(aFontCTL.GetPitch()) );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_WRITING_MODE, sal_Int16( com::sun::star::text::WritingMode2::PAGE ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_PARA_IS_CHARACTER_DISTANCE, sal_True );
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
    
    ::comphelper::MakeVector< OUString > aPropNames
        ( "CharFontCharSet");                
    aPropNames
        ( "CharFontFamily")                  
        ( "CharFontName")                    
        ( "CharFontPitch")                   
        ( "CharFontStyleName")               
        ( "CharHeight")                      
        ( "CharPosture")                     
        ( "CharStrikeout")                   
        ( "CharUnderline")                   
        ( "CharWeight")                      
        ( "CharWordMode")                    
        ;

    uno::Sequence< OUString > aPropNameSeq( ContainerHelper::ContainerToSequence( aPropNames ));
    uno::Sequence< uno::Any > aValues( xMultiPropSet->getPropertyValues( aPropNameSeq ));

    sal_Int32 i=0;
    
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
