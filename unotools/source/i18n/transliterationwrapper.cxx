/*************************************************************************
 *
 *  $RCSfile: transliterationwrapper.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:15:18 $
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

#include <unotools/transliterationwrapper.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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


#define TRANSLIT_LIBRARYNAME "i18n"
#define TRANSLIT_SERVICENAME "com.sun.star.i18n.Transliteration"

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::drafts::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::utl;

TransliterationWrapper::TransliterationWrapper(
                    const Reference< XMultiServiceFactory > & xSF,
                    sal_uInt32 nTyp )
    : xSMgr( xSF ), nType( nTyp ), nLanguage( 0 ), bFirstCall( sal_True )
{
    if( xSMgr.is() )
    {
        try {
            xTrans = Reference< XExtendedTransliteration > (
                    xSMgr->createInstance( ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                TRANSLIT_SERVICENAME))), UNO_QUERY );
        }
        catch ( Exception&  )
        {
            DBG_ERRORFILE( "TransliterationWrapper: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "TransliterationWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME(
                                TRANSLIT_LIBRARYNAME ))),
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            TRANSLIT_SERVICENAME)));
            if ( xI.is() )
            {
                Any x = xI->queryInterface(
                    ::getCppuType((const Reference< XExtendedTransliteration>*)0) );
                x >>= xTrans ;
            }
        }
        catch ( Exception&  )
        {
            DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
        }
    }
    DBG_ASSERT( xTrans.is(), "TransliterationWrapper: no Transliteraion available" );
}


TransliterationWrapper::~TransliterationWrapper()
{
}


String TransliterationWrapper::transliterate(
                                const String& rStr, sal_uInt16 nLang,
                                xub_StrLen nStart, xub_StrLen nLen,
                                Sequence <long>* pOffset )
{
    String sRet;
    if( xTrans.is() )
    {
        try
        {
            loadModuleIfNeeded( nLang );

            if ( pOffset )
                sRet = xTrans->transliterate( rStr, nStart, nLen, *pOffset );
            else
                sRet = xTrans->transliterateString2String( rStr, nStart, nLen);
        }
        catch( Exception&  )
        {
            DBG_ERRORFILE( "transliterate: Exception caught!" );
        }
    }
    return sRet;
}


String TransliterationWrapper::transliterate(
                                const String& rStr,
                                xub_StrLen nStart, xub_StrLen nLen,
                                Sequence <long>* pOffset ) const
{
    String sRet( rStr );
    if( xTrans.is() )
    {
        try
        {
            if ( pOffset )
                sRet = xTrans->transliterate( rStr, nStart, nLen, *pOffset );
            else
                sRet = xTrans->transliterateString2String( rStr, nStart, nLen);
        }
        catch( Exception&  )
        {
            DBG_ERRORFILE( "transliterate: Exception caught!" );
        }
    }
    return sRet;
}

sal_Bool TransliterationWrapper::needLanguageForTheMode() const
{
    return TransliterationModules_UPPERCASE_LOWERCASE == nType ||
           TransliterationModules_LOWERCASE_UPPERCASE == nType ||
           TransliterationModules_IGNORE_CASE == nType;
}


void TransliterationWrapper::setLanguageLocaleImpl( sal_uInt16 nLang )
{
    nLanguage = nLang;
    String aLangStr, aCtryStr;
    if( LANGUAGE_NONE == nLanguage )
        nLanguage = LANGUAGE_SYSTEM;
    ConvertLanguageToIsoNames( nLanguage, aLangStr, aCtryStr );
    aLocale.Language = aLangStr;
    aLocale.Country = aCtryStr;
}


void TransliterationWrapper::loadModuleIfNeeded( sal_uInt16 nLang )
{
    sal_Bool bLoad = bFirstCall;
    bFirstCall = sal_False;

    if( nLanguage != nLang )
    {
        setLanguageLocaleImpl( nLang );
        if( !bLoad )
            bLoad = needLanguageForTheMode();
    }
    if( bLoad )
        loadModuleImpl();
}


void TransliterationWrapper::loadModuleImpl() const
{
    if ( bFirstCall )
        ((TransliterationWrapper*)this)->setLanguageLocaleImpl( LANGUAGE_SYSTEM );

    try
    {
        if ( xTrans.is() )
            xTrans->loadModule( (TransliterationModules)nType, aLocale );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "loadModuleImpl: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }

    bFirstCall = sal_False;
}


void TransliterationWrapper::loadModuleByImplName(
        const String& rModuleName, sal_uInt16 nLang )
{
    try
    {
        setLanguageLocaleImpl( nLang );
        // Reset LanguageType, so the next call to loadModuleIfNeeded() forces
        // new settings.
        nLanguage = LANGUAGE_DONTKNOW;
        if ( xTrans.is() )
            xTrans->loadModuleByImplName( rModuleName, aLocale );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "loadModuleByImplName: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }

    bFirstCall = sal_False;
}


sal_Bool TransliterationWrapper::equals(
    const String& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const String& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) const
{
    try
    {
        if( bFirstCall )
            loadModuleImpl();
        if ( xTrans.is() )
            return xTrans->equals( rStr1, nPos1, nCount1, nMatch1, rStr2, nPos2, nCount2, nMatch2 );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "equals: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sal_False;
}


sal_Int32 TransliterationWrapper::compareSubstring(
    const String& rStr1, sal_Int32 nOff1, sal_Int32 nLen1,
    const String& rStr2, sal_Int32 nOff2, sal_Int32 nLen2 ) const
{
    try
    {
        if( bFirstCall )
            loadModuleImpl();
        if ( xTrans.is() )
            return xTrans->compareSubstring( rStr1, nOff1, nLen1, rStr2, nOff2, nLen2 );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "compareSubstring: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


sal_Int32 TransliterationWrapper::compareString( const String& rStr1, const String& rStr2 ) const
{
    try
    {
        if( bFirstCall )
            loadModuleImpl();
        if ( xTrans.is() )
            return xTrans->compareString( rStr1, rStr2 );
    }
    catch ( Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "compareString: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return 0;
}


// --- helpers --------------------------------------------------------

sal_Bool TransliterationWrapper::isEqual( const String& rStr1, const String& rStr2 ) const
{
    sal_Int32 nMatch1, nMatch2;
    sal_Bool bMatch = equals(
        rStr1, 0, rStr1.Len(), nMatch1,
        rStr2, 0, rStr2.Len(), nMatch2 );
    return bMatch;
}


sal_Bool TransliterationWrapper::isMatch( const String& rStr1, const String& rStr2 ) const
{
    sal_Int32 nMatch1, nMatch2;
    sal_Bool bMatch = equals(
        rStr1, 0, rStr1.Len(), nMatch1,
        rStr2, 0, rStr2.Len(), nMatch2 );
    return (nMatch1 <= nMatch2) && (nMatch1 == rStr1.Len());
}
