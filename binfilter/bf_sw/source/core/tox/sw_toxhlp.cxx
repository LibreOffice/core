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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <tools/string.hxx>
#include <tools/debug.hxx>

#include <toxwrap.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
namespace binfilter {

 IndexEntrySupplierWrapper::IndexEntrySupplierWrapper()
 {
     ::com::sun::star::uno::Reference<
             ::com::sun::star::lang::XMultiServiceFactory > rxMSF =
                                     ::legacy_binfilters::getLegacyProcessServiceFactory();
 
    try {
        STAR_REFERENCE( uno::XInterface ) xI =
            rxMSF->createInstance( ::rtl::OUString::createFromAscii(
                                 "com.sun.star.i18n.IndexEntrySupplier" ) );
        if( xI.is() )
        {
            UNO_NMSPC::Any x = xI->queryInterface( ::getCppuType(
                     (const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XExtendedIndexEntrySupplier>*)0) );
            x >>= xIES;
        }
    }
    catch ( UNO_NMSPC::Exception& e )
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

 STAR_NMSPC::uno::Sequence< ::rtl::OUString >
 IndexEntrySupplierWrapper::GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const
 {
     ::com::sun::star::uno::Sequence< ::rtl::OUString > sRet;
 
    try {
         sRet = xIES->getAlgorithmList( rLcl );
    }
    catch ( UNO_NMSPC::Exception& e )
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
    catch ( UNO_NMSPC::Exception& e )
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
    catch ( UNO_NMSPC::Exception& e )
    {
 #ifdef DBG_UTIL
         ByteString aMsg( "compareIndexEntry: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
 #endif
    }
     return nRet;
 }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
