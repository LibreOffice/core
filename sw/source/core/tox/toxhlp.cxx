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
#include "precompiled_sw.hxx"


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#ifndef _COM_SUN_STAR_I18N_XINDEXENTRYSUPPLIER_HPP_
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#endif
#include <tools/string.hxx>
#include <tools/debug.hxx>
#include <toxwrap.hxx>

using namespace ::com::sun::star;


IndexEntrySupplierWrapper::IndexEntrySupplierWrapper()
{
    uno::Reference<
            lang::XMultiServiceFactory > rxMSF =
                                    ::comphelper::getProcessServiceFactory();

    try {
        STAR_REFERENCE( uno::XInterface ) xI =
            rxMSF->createInstance( ::rtl::OUString::createFromAscii(
                                "com.sun.star.i18n.IndexEntrySupplier" ) );
        if( xI.is() )
        {
            UNO_NMSPC::Any x = xI->queryInterface( ::getCppuType(
                    (const uno::Reference< i18n::XExtendedIndexEntrySupplier>*)0) );
            x >>= xIES;
        }
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "IndexEntrySupplierWrapper: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}

IndexEntrySupplierWrapper::~IndexEntrySupplierWrapper()
{
}

String IndexEntrySupplierWrapper::GetIndexKey( const String& rTxt,
                                               const String& rTxtReading,
                                               const STAR_NMSPC::lang::Locale& rLocale ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexKey( rTxt, rTxtReading, rLocale );
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getIndexKey: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

String IndexEntrySupplierWrapper::GetFollowingText( sal_Bool bMorePages ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexFollowPageWord( bMorePages, aLcl );
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getIndexFollowPageWord: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

STAR_NMSPC::uno::Sequence< ::rtl::OUString >
IndexEntrySupplierWrapper::GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const
{
    uno::Sequence< ::rtl::OUString > sRet;

    try {
        sRet = xIES->getAlgorithmList( rLcl );
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "getAlgorithmList: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

sal_Bool IndexEntrySupplierWrapper::LoadAlgorithm(
        const STAR_NMSPC::lang::Locale& rLcl,
        const String& sSortAlgorithm, long nOptions ) const
{
    sal_Bool bRet = sal_False;
    try {
        bRet = xIES->loadAlgorithm( rLcl, sSortAlgorithm, nOptions );
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "loadAlgorithm: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return bRet;
}

sal_Int16 IndexEntrySupplierWrapper::CompareIndexEntry(
            const String& rTxt1, const String& rTxtReading1,
            const STAR_NMSPC::lang::Locale& rLocale1,
            const String& rTxt2, const String& rTxtReading2,
            const STAR_NMSPC::lang::Locale& rLocale2 ) const
{
    sal_Int16 nRet = 0;
    try {
        nRet = xIES->compareIndexEntry( rTxt1, rTxtReading1, rLocale1,
                                        rTxt2, rTxtReading2, rLocale2 );
    }
    catch ( UNO_NMSPC::Exception&
#ifdef DBG_UTIL
        e
#endif
        )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "compareIndexEntry: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return nRet;
}
