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

#include <comphelper/processfactory.hxx>
#include <unotools/charclass.hxx>
#include <rtl/character.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

CharClass::CharClass(
            const Reference< uno::XComponentContext > & rxContext,
            LanguageTag aLanguageTag
            )
    : maLanguageTag(std::move( aLanguageTag))
{
    xCC = CharacterClassification::create( rxContext );
}

CharClass::CharClass( LanguageTag aLanguageTag )
    : maLanguageTag(std::move( aLanguageTag))
{
    xCC = CharacterClassification::create( comphelper::getProcessComponentContext() );
}

CharClass::~CharClass()
{
}

const LanguageTag& CharClass::getLanguageTag() const
{
    return maLanguageTag;
}

const css::lang::Locale& CharClass::getMyLocale() const
{
    return maLanguageTag.getLocale();
}

// static
bool CharClass::isAsciiNumeric( std::u16string_view rStr )
{
    if ( rStr.empty() )
        return false;
    const sal_Unicode* p = rStr.data();
    const sal_Unicode* const pStop = p + rStr.size();

    do
    {
        if ( !rtl::isAsciiDigit( *p ) )
            return false;
    }
    while ( ++p < pStop );

    return true;
}

// static
bool CharClass::isAsciiAlpha( std::u16string_view rStr )
{
    if ( rStr.empty() )
        return false;
    const sal_Unicode* p = rStr.data();
    const sal_Unicode* const pStop = p + rStr.size();

    do
    {
        if ( !rtl::isAsciiAlpha( *p ) )
            return false;
    }
    while ( ++p < pStop );

    return true;
}

bool CharClass::isAlpha( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiAlpha( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                 nCharClassAlphaType) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isLetter( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiAlpha( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                 nCharClassLetterType) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isLetter( const OUString& rStr ) const
{
    if (rStr.isEmpty())
        return false;

    try
    {
        sal_Int32 nPos = 0;
        while (nPos < rStr.getLength())
        {
            if (!isLetter( rStr, nPos))
                return false;
            rStr.iterateCodePoints( &nPos);
        }
        return true;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isDigit( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[ nPos ];
    if ( c < 128 )
        return rtl::isAsciiDigit( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                 KCharacterType::DIGIT) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isNumeric( const OUString& rStr ) const
{
    if (rStr.isEmpty())
        return false;

    try
    {
        sal_Int32 nPos = 0;
        while (nPos < rStr.getLength())
        {
            if (!isDigit( rStr, nPos))
                return false;
            rStr.iterateCodePoints( &nPos);
        }
        return true;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isAlphaNumeric( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiAlphanumeric( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                (nCharClassAlphaType | nCharClassNumericType)) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isLetterNumeric( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiAlphanumeric( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                (nCharClassLetterType | nCharClassNumericType)) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isLetterNumeric( const OUString& rStr ) const
{
    if (rStr.isEmpty())
        return false;

    try
    {
        sal_Int32 nPos = 0;
        while (nPos < rStr.getLength())
        {
            if (!isLetterNumeric( rStr, nPos))
                return false;
            rStr.iterateCodePoints( &nPos);
        }
        return true;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isBase( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiAlphanumeric( c );

    try
    {
        return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) & nCharClassBaseType ) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

bool CharClass::isUpper( const OUString& rStr, sal_Int32 nPos ) const
{
    sal_Unicode c = rStr[nPos];
    if ( c < 128 )
        return rtl::isAsciiUpperCase(c);

    try
    {
        return (xCC->getCharacterType( rStr, nPos, getMyLocale()) &
                KCharacterType::UPPER) != 0;
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return false;
}

OUString CharClass::titlecase(const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount) const
{
    try
    {
        return xCC->toTitle( rStr, nPos, nCount, getMyLocale() );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return rStr.copy( nPos, nCount );
}

OUString CharClass::uppercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        return xCC->toUpper( rStr, nPos, nCount, getMyLocale() );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return rStr.copy( nPos, nCount );
}

OUString CharClass::lowercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        return xCC->toLower( rStr, nPos, nCount, getMyLocale() );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return rStr.copy( nPos, nCount );
}

sal_Int16 CharClass::getType( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        return xCC->getType( rStr, nPos );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return 0;
}

css::i18n::DirectionProperty CharClass::getCharacterDirection( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        return static_cast<css::i18n::DirectionProperty>(xCC->getCharacterDirection( rStr, nPos ));
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return css::i18n::DirectionProperty_LEFT_TO_RIGHT;
}

css::i18n::UnicodeScript CharClass::getScript( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        return static_cast<css::i18n::UnicodeScript>(xCC->getScript( rStr, nPos ));
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return UnicodeScript_kBasicLatin;
}

sal_Int32 CharClass::getCharacterType( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        return xCC->getCharacterType( rStr, nPos, getMyLocale() );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("unotools.i18n", "" );
    }
    return 0;
}

css::i18n::ParseResult CharClass::parseAnyToken(
            const OUString& rStr,
            sal_Int32 nPos,
            sal_Int32 nStartCharFlags,
            const OUString& userDefinedCharactersStart,
            sal_Int32 nContCharFlags,
            const OUString& userDefinedCharactersCont ) const
{
    try
    {
        return xCC->parseAnyToken( rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "parseAnyToken" );
    }
    return ParseResult();
}

css::i18n::ParseResult CharClass::parsePredefinedToken(
            sal_Int32 nTokenType,
            const OUString& rStr,
            sal_Int32 nPos,
            sal_Int32 nStartCharFlags,
            const OUString& userDefinedCharactersStart,
            sal_Int32 nContCharFlags,
            const OUString& userDefinedCharactersCont ) const
{
    try
    {
        return xCC->parsePredefinedToken( nTokenType, rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
    }
    catch ( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "unotools.i18n", "parsePredefinedToken" );
    }
    return ParseResult();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
