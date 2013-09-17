/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/IndexEntrySupplier.hpp>
#include <tools/string.hxx>
#include <toxwrap.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;


IndexEntrySupplierWrapper::IndexEntrySupplierWrapper()
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    try {
        xIES = i18n::IndexEntrySupplier::create(xContext);
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "IndexEntrySupplierWrapper: Caught exception: " << e.Message );
    }
}

IndexEntrySupplierWrapper::~IndexEntrySupplierWrapper()
{
}

OUString IndexEntrySupplierWrapper::GetIndexKey( const OUString& rTxt,
                                                 const OUString& rTxtReading,
                                                 const ::com::sun::star::lang::Locale& rLocale ) const
{
    OUString sRet;
    try {
        sRet = xIES->getIndexKey( rTxt, rTxtReading, rLocale );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "getIndexKey: Caught exception: " << e.Message );
    }
    return sRet;
}

OUString IndexEntrySupplierWrapper::GetFollowingText( sal_Bool bMorePages ) const
{
    OUString sRet;
    try {
        sRet = xIES->getIndexFollowPageWord( bMorePages, aLcl );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "getIndexFollowPageWord: Caught exception: " << e.Message );
    }
    return sRet;
}

::com::sun::star::uno::Sequence< OUString >
IndexEntrySupplierWrapper::GetAlgorithmList( const ::com::sun::star::lang::Locale& rLcl ) const
{
    uno::Sequence< OUString > sRet;

    try {
        sRet = xIES->getAlgorithmList( rLcl );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "getAlgorithmList: Caught exception: " << e.Message );
    }
    return sRet;
}

sal_Bool IndexEntrySupplierWrapper::LoadAlgorithm(
        const ::com::sun::star::lang::Locale& rLcl,
        const OUString& sSortAlgorithm, long nOptions ) const
{
    sal_Bool bRet = sal_False;
    try {
        bRet = xIES->loadAlgorithm( rLcl, sSortAlgorithm, nOptions );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "loadAlgorithm: Caught exception: " << e.Message );
    }
    return bRet;
}

sal_Int16 IndexEntrySupplierWrapper::CompareIndexEntry(
            const OUString& rTxt1, const OUString& rTxtReading1,
            const ::com::sun::star::lang::Locale& rLocale1,
            const OUString& rTxt2, const OUString& rTxtReading2,
            const ::com::sun::star::lang::Locale& rLocale2 ) const
{
    sal_Int16 nRet = 0;
    try {
        nRet = xIES->compareIndexEntry( rTxt1, rTxtReading1, rLocale1,
                                        rTxt2, rTxtReading2, rLocale2 );
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN( "sw.core", "compareIndexEntry: Caught exception: " << e.Message );
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
