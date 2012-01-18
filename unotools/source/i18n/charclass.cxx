/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <unotools/charclass.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#include "instance.hxx"

#define CHARCLASS_SERVICENAME "com.sun.star.i18n.CharacterClassification"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


CharClass::CharClass(
            const Reference< lang::XMultiServiceFactory > & xSF,
            const lang::Locale& rLocale
            )
        :
        xSMgr( xSF )
{
    setLocale( rLocale );
    xCC = Reference< XCharacterClassification > ( intl_createInstance( xSMgr, CHARCLASS_SERVICENAME, "CharClass" ), uno::UNO_QUERY );
}


CharClass::CharClass(
            const ::com::sun::star::lang::Locale& rLocale )
{
    setLocale( rLocale );
    Reference< lang::XMultiServiceFactory > xNil;
    xCC = Reference< XCharacterClassification > ( intl_createInstance( xNil, CHARCLASS_SERVICENAME, "CharClass" ), uno::UNO_QUERY );
}


CharClass::~CharClass()
{
}


void CharClass::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    ::osl::MutexGuard aGuard( aMutex );
    aLocale.Language = rLocale.Language;
    aLocale.Country = rLocale.Country;
    aLocale.Variant = rLocale.Variant;
}


const ::com::sun::star::lang::Locale& CharClass::getLocale() const
{
    ::osl::MutexGuard aGuard( aMutex );
    return aLocale;
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
            return  (xCC->getCharacterType( rStr, nPos, getLocale() ) &
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
            return  (xCC->getCharacterType( rStr, nPos, getLocale() ) &
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
            return isLetterType( xCC->getStringType( rStr, 0, rStr.Len(), getLocale() ) );
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
            return  (xCC->getCharacterType( rStr, nPos, getLocale() ) &
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
            return isNumericType( xCC->getStringType( rStr, 0, rStr.Len(), getLocale() ) );
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
            return  (xCC->getCharacterType( rStr, nPos, getLocale() ) &
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
            return  (xCC->getCharacterType( rStr, nPos, getLocale() ) &
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
            return isLetterNumericType( xCC->getStringType( rStr, 0, rStr.Len(), getLocale() ) );
        else
            return sal_False;
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "isLetterNumeric: Exception caught!" );
        return sal_False;
    }
}

rtl::OUString CharClass::titlecase(const rtl::OUString& rStr, sal_Int32 nPos, sal_Int32 nCount) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toTitle( rStr, nPos, nCount, getLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "titlecase: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}

::rtl::OUString CharClass::uppercase( const ::rtl::OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toUpper( rStr, nPos, nCount, getLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( const Exception& )
    {
        SAL_WARN( "unotools.i18n", "uppercase: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}

::rtl::OUString CharClass::lowercase( const ::rtl::OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toLower( rStr, nPos, nCount, getLocale() );
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
            return xCC->getCharacterType( rStr, nPos, getLocale() );
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
            return xCC->getStringType( rStr, nPos, nCount, getLocale() );
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
            return xCC->parseAnyToken( rStr, nPos, getLocale(),
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
            return xCC->parsePredefinedToken( nTokenType, rStr, nPos, getLocale(),
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
