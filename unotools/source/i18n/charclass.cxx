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
#include <tools/string.hxx>
#include <tools/debug.hxx>

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


const ::com::sun::star::lang::Locale& CharClass::getMyLocale() const
{
    ::osl::MutexGuard aGuard( aMutex );
    return maLanguageTag.getLocale();
}


// static
sal_Bool CharClass::isAsciiNumeric( const String& rStr )
{
    if ( !rStr.Len() )
        return sal_False;
    register const sal_Unicode* p = rStr.GetBuffer();
    register const sal_Unicode* const pStop = p + rStr.Len();
    do
    {
        if ( !isAsciiDigit( *p ) )
            return sal_False;
    } while ( ++p < pStop );
    return sal_True;
}


// static
sal_Bool CharClass::isAsciiAlpha( const String& rStr )
{
    if ( !rStr.Len() )
        return sal_False;
    register const sal_Unicode* p = rStr.GetBuffer();
    register const sal_Unicode* const pStop = p + rStr.Len();
    do
    {
        if ( !isAsciiAlpha( *p ) )
            return sal_False;
    } while ( ++p < pStop );
    return sal_True;
}



sal_Bool CharClass::isAlpha( const String& rStr, xub_StrLen nPos ) const
{
    sal_Unicode c = rStr.GetChar( nPos );
    if ( c < 128 )
        return isAsciiAlpha( c );

    try
    {
        if ( xCC.is() )
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                nCharClassAlphaType) != 0;
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isAlpha: Exception caught!" );
        return sal_False;
    }
}



sal_Bool CharClass::isLetter( const String& rStr, xub_StrLen nPos ) const
{
    sal_Unicode c = rStr.GetChar( nPos );
    if ( c < 128 )
        return isAsciiAlpha( c );

    try
    {
        if ( xCC.is() )
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                nCharClassLetterType) != 0;
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetter: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isLetter( const String& rStr ) const
{
    try
    {
        if ( xCC.is() )
            return isLetterType( xCC->getStringType( rStr, 0, rStr.Len(), getMyLocale() ) );
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetter: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isDigit( const String& rStr, xub_StrLen nPos ) const
{
    sal_Unicode c = rStr.GetChar( nPos );
    if ( c < 128 )
        return isAsciiDigit( c );

    try
    {
        if ( xCC.is() )
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                KCharacterType::DIGIT) != 0;
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isDigit: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isNumeric( const String& rStr ) const
{
    try
    {
        if ( xCC.is() )
            return isNumericType( xCC->getStringType( rStr, 0, rStr.Len(), getMyLocale() ) );
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isNumeric: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isAlphaNumeric( const String& rStr, xub_StrLen nPos ) const
{
    sal_Unicode c = rStr.GetChar( nPos );
    if ( c < 128 )
        return isAsciiAlphaNumeric( c );

    try
    {
        if ( xCC.is() )
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                (nCharClassAlphaType | KCharacterType::DIGIT)) != 0;
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isAlphaNumeric: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isLetterNumeric( const String& rStr, xub_StrLen nPos ) const
{
    sal_Unicode c = rStr.GetChar( nPos );
    if ( c < 128 )
        return isAsciiAlphaNumeric( c );

    try
    {
        if ( xCC.is() )
            return  (xCC->getCharacterType( rStr, nPos, getMyLocale() ) &
                (nCharClassLetterType | KCharacterType::DIGIT)) != 0;
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetterNumeric: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isLetterNumeric( const String& rStr ) const
{
    try
    {
        if ( xCC.is() )
            return isLetterNumericType( xCC->getStringType( rStr, 0, rStr.Len(), getMyLocale() ) );
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetterNumeric: Exception caught!" );
        return sal_False;
    }
}

OUString CharClass::titlecase(const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toTitle( rStr, nPos, nCount, getMyLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "titlecase: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}

OUString CharClass::uppercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toUpper( rStr, nPos, nCount, getMyLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "uppercase: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}

OUString CharClass::lowercase( const OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toLower( rStr, nPos, nCount, getMyLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "lowercase: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}

sal_Int16 CharClass::getType( const String& rStr, xub_StrLen nPos ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->getType( rStr, nPos );
        else
            return 0;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getType: Exception caught!" );
        return 0;
    }
}


sal_Int16 CharClass::getCharacterDirection( const String& rStr, xub_StrLen nPos ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->getCharacterDirection( rStr, nPos );
        else
            return 0;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getCharacterDirection: Exception caught!" );
        return 0;
    }
}


sal_Int16 CharClass::getScript( const String& rStr, xub_StrLen nPos ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->getScript( rStr, nPos );
        else
            return 0;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getScript: Exception caught!" );
        return 0;
    }
}


sal_Int32 CharClass::getCharacterType( const String& rStr, xub_StrLen nPos ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->getCharacterType( rStr, nPos, getMyLocale() );
        else
            return 0;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getCharacterType: Exception caught!" );
        return 0;
    }
}


sal_Int32 CharClass::getStringType( const String& rStr, xub_StrLen nPos, xub_StrLen nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->getStringType( rStr, nPos, nCount, getMyLocale() );
        else
            return 0;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "getStringType: Exception caught!" );
        return 0;
    }
}


::com::sun::star::i18n::ParseResult CharClass::parseAnyToken(
            const String& rStr,
            sal_Int32 nPos,
            sal_Int32 nStartCharFlags,
            const String& userDefinedCharactersStart,
            sal_Int32 nContCharFlags,
            const String& userDefinedCharactersCont ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->parseAnyToken( rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
        else
            return ParseResult();
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "parseAnyToken: Exception caught " << e.Message );
        return ParseResult();
    }
}


::com::sun::star::i18n::ParseResult CharClass::parsePredefinedToken(
            sal_Int32 nTokenType,
            const String& rStr,
            sal_Int32 nPos,
            sal_Int32 nStartCharFlags,
            const String& userDefinedCharactersStart,
            sal_Int32 nContCharFlags,
            const String& userDefinedCharactersCont ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->parsePredefinedToken( nTokenType, rStr, nPos, getMyLocale(),
                nStartCharFlags, userDefinedCharactersStart,
                nContCharFlags, userDefinedCharactersCont );
        else
            return ParseResult();
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "parsePredefinedToken: Exception caught " << e.Message );
        return ParseResult();
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
