/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#include <unotools/charclass.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define CHARCLASS_LIBRARYNAME "i18n"
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
    if ( xSMgr.is() )
    {
        try
        {
            xCC = Reference< XCharacterClassification > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CHARCLASS_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& )
        {
            DBG_ERRORFILE( "CharClass ctor: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        getComponentInstance();
    }
}


CharClass::CharClass(
            const ::com::sun::star::lang::Locale& rLocale )
{
    setLocale( rLocale );
    getComponentInstance();
}


CharClass::~CharClass()
{
}


void CharClass::getComponentInstance()
{
    try
    {
        // CharClass may be needed by "small tools" like the Setup
        // => maybe no service manager => loadLibComponentFactory
        Reference < XInterface > xI = ::comphelper::getComponentInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( CHARCLASS_LIBRARYNAME ) ) ),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( CHARCLASS_SERVICENAME ) ) );
        if ( xI.is() )
        {
            Any x = xI->queryInterface( ::getCppuType((const Reference< XCharacterClassification >*)0) );
            x >>= xCC;
        }
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
    }
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
    const sal_Unicode* p = rStr.GetBuffer();
    const sal_Unicode* const pStop = p + rStr.Len();
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
    const sal_Unicode* p = rStr.GetBuffer();
    const sal_Unicode* const pStop = p + rStr.Len();
    do
    {
        if ( !isAsciiAlpha( *p ) )
            return sal_False;
    } while ( ++p < pStop );
    return sal_True;
}


// static
sal_Bool CharClass::isAsciiAlphaNumeric( const String& rStr )
{
    if ( !rStr.Len() )
        return sal_False;
    const sal_Unicode* p = rStr.GetBuffer();
    const sal_Unicode* const pStop = p + rStr.Len();
    do
    {
        if ( !isAsciiAlphaNumeric( *p ) )
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isAlpha: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isAlpha( const String& rStr ) const
{
    try
    {
        if ( xCC.is() )
            return isAlphaType( xCC->getStringType( rStr, 0, rStr.Len(), getLocale() ) );
        else
            return sal_False;
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isAlpha: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isLetter: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isLetter: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isDigit: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isNumeric: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isAlphaNumeric: Exception caught!" );
        return sal_False;
    }
}


sal_Bool CharClass::isAlphaNumeric( const String& rStr ) const
{
    try
    {
        if ( xCC.is() )
            return isAlphaNumericType( xCC->getStringType( rStr, 0, rStr.Len(), getLocale() ) );
        else
            return sal_False;
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isAlphaNumeric: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isLetterNumeric: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "isLetterNumeric: Exception caught!" );
        return sal_False;
    }
}


String CharClass::toUpper( const String& rStr, xub_StrLen nPos, xub_StrLen nCount ) const
{
    return toUpper_rtl(rStr, nPos, nCount);
}


String CharClass::toLower( const String& rStr, xub_StrLen nPos, xub_StrLen nCount ) const
{
    return toLower_rtl(::rtl::OUString(rStr), nPos, nCount);
}


String CharClass::toTitle( const String& rStr, xub_StrLen nPos, xub_StrLen nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toTitle( rStr, nPos, nCount, getLocale() );
        else
            return rStr.Copy( nPos, nCount );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "toTitle: Exception caught!" );
        return rStr.Copy( nPos, nCount );
    }
}


::rtl::OUString CharClass::toUpper_rtl( const ::rtl::OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toUpper( rStr, nPos, nCount, getLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "toUpper: Exception caught!" );
        return rStr.copy( nPos, nCount );
    }
}


::rtl::OUString CharClass::toLower_rtl( const ::rtl::OUString& rStr, sal_Int32 nPos, sal_Int32 nCount ) const
{
    try
    {
        if ( xCC.is() )
            return xCC->toLower( rStr, nPos, nCount, getLocale() );
        else
            return rStr.copy( nPos, nCount );
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "toLower: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getType: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getCharacterDirection: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getScript: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getCharacterType: Exception caught!" );
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
    catch ( Exception& )
    {
        DBG_ERRORFILE( "getStringType: Exception caught!" );
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
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "parseAnyToken: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
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
    catch ( Exception& e )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "parsePredefinedToken: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#else
        (void)e;
#endif
        return ParseResult();
    }
}



