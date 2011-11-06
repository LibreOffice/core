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

#include <unotools/numberformatcodewrapper.hxx>
#include <tools/debug.hxx>

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#define LOCALEDATA_LIBRARYNAME "i18npool"
#define LOCALEDATA_SERVICENAME "com.sun.star.i18n.NumberFormatMapper"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;


NumberFormatCodeWrapper::NumberFormatCodeWrapper(
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
            xNFC = Reference< XNumberFormatCode > ( xSMgr->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) ),
                uno::UNO_QUERY );
        }
        catch ( Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "NumberFormatCodeWrapper ctor: Exception caught!" );
        }
    }
    else
    {   // try to get an instance somehow
        DBG_ERRORFILE( "NumberFormatCodeWrapper: no service manager, trying own" );
        try
        {
            Reference< XInterface > xI = ::comphelper::getComponentInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LLCF_LIBNAME( LOCALEDATA_LIBRARYNAME ) ) ),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( LOCALEDATA_SERVICENAME ) ) );
            if ( xI.is() )
            {
                Any x = xI->queryInterface( ::getCppuType((const Reference< XNumberFormatCode >*)0) );
                x >>= xNFC;
            }
        }
        catch ( Exception& e )
        {
            (void)e;
            DBG_ERRORFILE( "getComponentInstance: Exception caught!" );
        }
    }
    DBG_ASSERT( xNFC.is(), "NumberFormatCodeWrapper: no NumberFormatMapper" );
}


NumberFormatCodeWrapper::~NumberFormatCodeWrapper()
{
}


void NumberFormatCodeWrapper::setLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    aLocale = rLocale;
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getDefault( formatType, formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getDefault: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::i18n::NumberFormatCode
NumberFormatCodeWrapper::getFormatCode( sal_Int16 formatIndex ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getFormatCode( formatIndex, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getFormatCode: Exception caught!" );
    }
    return ::com::sun::star::i18n::NumberFormatCode();
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCode( sal_Int16 formatUsage ) const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCode( formatUsage, aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCode: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}


::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >
NumberFormatCodeWrapper::getAllFormatCodes() const
{
    try
    {
        if ( xNFC.is() )
            return xNFC->getAllFormatCodes( aLocale );
    }
    catch ( Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getAllFormatCodes: Exception caught!" );
    }
    return ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode > (0);
}
