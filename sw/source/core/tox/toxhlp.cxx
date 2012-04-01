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



#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <rtl/strbuf.hxx>
#include <tools/string.hxx>
#include <toxwrap.hxx>

using namespace ::com::sun::star;


IndexEntrySupplierWrapper::IndexEntrySupplierWrapper()
{
    uno::Reference<
            lang::XMultiServiceFactory > rxMSF =
                                    ::comphelper::getProcessServiceFactory();

    try {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xI =
            rxMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.i18n.IndexEntrySupplier")) );
        if( xI.is() )
        {
            ::com::sun::star::uno::Any x = xI->queryInterface( ::getCppuType(
                    (const uno::Reference< i18n::XExtendedIndexEntrySupplier>*)0) );
            x >>= xIES;
        }
    }
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("IndexEntrySupplierWrapper: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
#endif
    }
}

IndexEntrySupplierWrapper::~IndexEntrySupplierWrapper()
{
}

String IndexEntrySupplierWrapper::GetIndexKey( const String& rTxt,
                                               const String& rTxtReading,
                                               const ::com::sun::star::lang::Locale& rLocale ) const
{
    String sRet;
    try {
        sRet = xIES->getIndexKey( rTxt, rTxtReading, rLocale );
    }
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("getIndexKey: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
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
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("getIndexFollowPageWord: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
#endif
    }
    return sRet;
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
IndexEntrySupplierWrapper::GetAlgorithmList( const ::com::sun::star::lang::Locale& rLcl ) const
{
    uno::Sequence< ::rtl::OUString > sRet;

    try {
        sRet = xIES->getAlgorithmList( rLcl );
    }
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("getAlgorithmList: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
#endif
    }
    return sRet;
}

sal_Bool IndexEntrySupplierWrapper::LoadAlgorithm(
        const ::com::sun::star::lang::Locale& rLcl,
        const String& sSortAlgorithm, long nOptions ) const
{
    sal_Bool bRet = sal_False;
    try {
        bRet = xIES->loadAlgorithm( rLcl, sSortAlgorithm, nOptions );
    }
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("loadAlgorithm: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
#endif
    }
    return bRet;
}

sal_Int16 IndexEntrySupplierWrapper::CompareIndexEntry(
            const String& rTxt1, const String& rTxtReading1,
            const ::com::sun::star::lang::Locale& rLocale1,
            const String& rTxt2, const String& rTxtReading2,
            const ::com::sun::star::lang::Locale& rLocale2 ) const
{
    sal_Int16 nRet = 0;
    try {
        nRet = xIES->compareIndexEntry( rTxt1, rTxtReading1, rLocale1,
                                        rTxt2, rTxtReading2, rLocale2 );
    }
    catch (const ::com::sun::star::uno::Exception&
#if OSL_DEBUG_LEVEL > 0
        e
#endif
        )
    {
#if OSL_DEBUG_LEVEL > 0
        rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM("compareIndexEntry: Exception caught\n"));
        aMsg.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8));
        OSL_FAIL( aMsg.getStr() );
#endif
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
