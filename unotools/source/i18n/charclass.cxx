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
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <rtl/character.hxx>

#include <com/sun/star/i18n/CharacterClassification.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;

CharClass::CharClass(
            const Reference< uno::XComponentContext > & rxContext,
            const LanguageTag& rLanguageTag
            )
    :
        maLanguageTag( rLanguageTag)
{
    xCC = CharacterClassification::create( rxContext );
}

CharClass::CharClass(
            const LanguageTag& rLanguageTag )
    :
        maLanguageTag( rLanguageTag)
{
    xCC = CharacterClassification::create( comphelper::getProcessComponentContext() );
}

CharClass::~CharClass()
{
}

void CharClass::setLanguageTag( const LanguageTag& rLanguageTag )
{
    ::osl::MutexGuard aGuard( aMutex );
    maLanguageTag = rLanguageTag;
}

const LanguageTag& CharClass::getLanguageTag() const
{
    ::osl::MutexGuard aGuard( aMutex );
    return maLanguageTag;
}

const css::lang::Locale& CharClass::getMyLocale() const
{
    // Mutex locked by callers.
    return maLanguageTag.getLocale();
}

// static
bool CharClass::isAsciiNumeric( const OUString& rStr )
{
    if ( rStr.isEmpty() )
        return false;
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* const pStop = p + rStr.getLength();

    do
    {
        if ( !rtl::isAsciiDigit( *p ) )
            return false;
    }
    while ( ++p < pStop );

    return true;
}

// static
bool CharClass::isAsciiAlpha( const OUString& rStr )
{
    if ( rStr.isEmpty() )
        return false;
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* const pStop = p + rStr.getLength();

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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                     nCharClassAlphaType) != 0;
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isAlpha: Exception caught!" );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                     nCharClassLetterType) != 0;
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetter: Exception caught!" );
    }
    return false;
}

bool CharClass::isLetter( const OUString& rStr ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return isLetterType( xCC->getStringType( rStr, 0, rStr.getLength(), getMyLocale() ) );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetter: Exception caught!" );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                     KCharacterType::DIGIT) != 0;
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isDigit: Exception caught!" );
    }
    return false;
}

bool CharClass::isNumeric( const OUString& rStr ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return isNumericType( xCC->getStringType( rStr, 0, rStr.getLength(), getMyLocale() ) );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isNumeric: Exception caught!" );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                (nCharClassAlphaType | KCharacterType::DIGIT)) != 0;
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isAlphaNumeric: Exception caught!" );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                     (nCharClassLetterType | KCharacterType::DIGIT)) != 0;
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetterNumeric: Exception caught!" );
    }
    return false;
}

bool CharClass::isLetterNumeric( const OUString& rStr ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return isLetterNumericType( xCC->getStringType( rStr, 0, rStr.getLength(), getMyLocale() ) );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetterNumeric: Exception caught!" );
    }
    return false;
}

OUString CharClass::titlecase(const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->toTitle( rStr, nPos, nCount, getMyLocale() );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "titlecase: Exception caught!" );
    }
    return rStr.copy( nPos, nCount );
}

OUString CharClass::uppercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->toUpper( rStr, nPos, nCount, getMyLocale() );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "uppercase: Exception caught!" );
    }
    return rStr.copy( nPos, nCount );
}

OUString CharClass::lowercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->toLower( rStr, nPos, nCount, getMyLocale() );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "lowercase: Exception caught!" );
    }
    return rStr.copy( nPos, nCount );
}

sal_Int16 CharClass::getType( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->getType( rStr, nPos );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getType: Exception caught!" );
    }
    return 0;
}

css::i18n::DirectionProperty CharClass::getCharacterDirection( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return static_cast<css::i18n::DirectionProperty>(xCC->getCharacterDirection( rStr, nPos ));
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getCharacterDirection: Exception caught!" );
    }
    return css::i18n::DirectionProperty_LEFT_TO_RIGHT;
}

css::i18n::UnicodeScript CharClass::getScript( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return static_cast<css::i18n::UnicodeScript>(xCC->getScript( rStr, nPos ));
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getScript: Exception caught!" );
    }
    return UnicodeScript_kBasicLatin;
}

sal_Int32 CharClass::getCharacterType( const OUString& rStr, sal_Int32 nPos ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->getCharacterType( rStr, nPos, getMyLocale() );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getCharacterType: Exception caught!" );
    }
    return 0;
}

sal_Int32 CharClass::getStringType( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->getStringType( rStr, nPos, nCount, getMyLocale() );
        }
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getStringType: Exception caught!" );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->parseAnyToken( rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
        }
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "parseAnyToken: Exception caught " << e );
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
        if ( xCC.is() )
        {
            ::osl::MutexGuard aGuard( aMutex );
            return xCC->parsePredefinedToken( nTokenType, rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
        }
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "parsePredefinedToken: Exception caught " << e );
    }
    return ParseResult();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
