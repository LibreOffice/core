/*************************************************************************
 *
 *  $RCSfile: charclass.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: er $ $Date: 2002-08-05 16:27:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <unotools/charclass.hxx>

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#pragma hdrstop


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
    ::utl::ReadWriteGuard aGuard( aMutex, ::utl::ReadWriteGuardMode::nCriticalChange );
    aLocale.Language = rLocale.Language;
    aLocale.Country = rLocale.Country;
    aLocale.Variant = rLocale.Variant;
}


const ::com::sun::star::lang::Locale& CharClass::getLocale() const
{
    ::utl::ReadWriteGuard aGuard( aMutex );
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


// static
sal_Bool CharClass::isAsciiAlphaNumeric( const String& rStr )
{
    if ( !rStr.Len() )
        return sal_False;
    register const sal_Unicode* p = rStr.GetBuffer();
    register const sal_Unicode* const pStop = p + rStr.Len();
    do
    {
        if ( !isAsciiAlphaNumeric( *p ) )
            return sal_False;
    } while ( ++p < pStop );
    return sal_True;
}


sal_Bool CharClass::isAlpha( const String& rStr, xub_StrLen nPos ) const
{
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
#ifndef PRODUCT
        ByteString aMsg( "parseAnyToken: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
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
#ifndef PRODUCT
        ByteString aMsg( "parsePredefinedToken: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
        return ParseResult();
    }
}



