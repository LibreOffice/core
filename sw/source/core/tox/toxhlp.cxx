/*************************************************************************
 *
 *  $RCSfile: toxhlp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 10:54:23 $
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


#pragma hdrstop

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XINDEXENTRYSUPPLIER_HPP_
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _TOXWRAP_HXX
#include <toxwrap.hxx>
#endif


IndexEntrySupplierWrapper::IndexEntrySupplierWrapper()
{
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > rxMSF =
                                    ::comphelper::getProcessServiceFactory();

    try {
        STAR_REFERENCE( uno::XInterface ) xI =
            rxMSF->createInstance( ::rtl::OUString::createFromAscii(
                                "com.sun.star.i18n.IndexEntrySupplier" ) );
        if( xI.is() )
        {
            UNO_NMSPC::Any x = xI->queryInterface( ::getCppuType(
                    (const com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier>*)0) );
            x >>= xIES;
        }
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
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
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getIndexKey: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

String IndexEntrySupplierWrapper::GetFollowingText( BOOL bMorePages ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexFollowPageWord( bMorePages, aLcl );
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
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
    ::com::sun::star::uno::Sequence< ::rtl::OUString > sRet;

    try {
        sRet = xIES->getAlgorithmList( rLcl );
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getAlgorithmList: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return sRet;
}

STAR_NMSPC::uno::Sequence < com::sun::star::lang::Locale >
IndexEntrySupplierWrapper::GetLocaleList() const
{
    ::com::sun::star::uno::Sequence< com::sun::star::lang::Locale > sRet;

    try {
        sRet = xIES->getLocaleList();
    }
    catch ( UNO_NMSPC::Exception& e )
    {
#ifndef PRODUCT
        ByteString aMsg( "getLocaleList: Exception caught\n" );
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
#ifndef PRODUCT
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
#ifndef PRODUCT
        ByteString aMsg( "compareIndexEntry: Exception caught\n" );
        aMsg += ByteString( String( e.Message ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
    return nRet;
}
