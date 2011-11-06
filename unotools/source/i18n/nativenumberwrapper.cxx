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

#include <unotools/nativenumberwrapper.hxx>
#include <tools/debug.hxx>

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define LOCALEDATA_LIBRARYNAME "i18npool"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.NativeNumberSupplier"

using namespace ::com::sun::star;


NativeNumberWrapper::NativeNumberWrapper(
            const uno::Reference< lang::XMultiServiceFactory > & xSF
            )
        :
        xSMgr( xSF )
{
    if ( xSMgr.is() )
    {
        try
        {
            xNNS = uno::Reference< i18n::XNativeNumberSupplier > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( uno::Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "NativeNumberWrapper ctor: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "NativeNumberWrapper: no service manager, trying own" );
        try
        {
            uno::Reference< uno::XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( LOCALEDATA_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) );
            if ( xI.is() )
            {
                uno::Any x = xI->queryInterface( ::getCppuType((const uno::Reference< i18n::XNativeNumberSupplier >*)0) );
                x >>= xNNS;
            }
        }
        catch ( uno::Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
        }
    }
    DBG_ASSERT( xNNS.is(), "NativeNumberWrapper: no NativeNumberSupplier" );
}


NativeNumberWrapper::~NativeNumberWrapper()
{
}


::rtl::OUString
NativeNumberWrapper::getNativeNumberString(
                    const ::rtl::OUString& rNumberString,
                    const ::com::sun::star::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->getNativeNumberString( rNumberString, rLocale, nNativeNumberMode );
    }
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getNativeNumberString: Exception caught!" );
    }
    return ::rtl::OUString();
}


sal_Bool
NativeNumberWrapper::isValidNatNum(
                    const ::com::sun::star::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->isValidNatNum( rLocale, nNativeNumberMode );
    }
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "isValidNatNum: Exception caught!" );
    }
    return sal_False;
}


i18n::NativeNumberXmlAttributes
NativeNumberWrapper::convertToXmlAttributes(
                    const ::com::sun::star::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->convertToXmlAttributes( rLocale, nNativeNumberMode );
    }
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "convertToXmlAttributes: Exception caught!" );
    }
    return i18n::NativeNumberXmlAttributes();
}


sal_Int16
NativeNumberWrapper::convertFromXmlAttributes(
                    const i18n::NativeNumberXmlAttributes& rAttr ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->convertFromXmlAttributes( rAttr );
    }
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "convertFromXmlAttributes: Exception caught!" );
    }
    return 0;
}


