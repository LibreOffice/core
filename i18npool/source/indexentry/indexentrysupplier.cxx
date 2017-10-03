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

#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <indexentrysupplier.hxx>
#include <localedata.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace i18npool {

IndexEntrySupplier::IndexEntrySupplier( const Reference < XComponentContext >& rxContext ) : m_xContext( rxContext )
{
}

Sequence < Locale > SAL_CALL IndexEntrySupplier::getLocaleList()
{
    return LocaleDataImpl::get()->getAllInstalledLocaleNames();
}

Sequence < OUString > SAL_CALL IndexEntrySupplier::getAlgorithmList( const Locale& rLocale )
{
    return LocaleDataImpl::get()->getIndexAlgorithm(rLocale);
}

sal_Bool SAL_CALL IndexEntrySupplier::loadAlgorithm( const Locale& rLocale, const OUString& SortAlgorithm,
        sal_Int32 collatorOptions )
{
    Sequence < OUString > algorithmList = getAlgorithmList( rLocale );
    for (sal_Int32 i = 0; i < algorithmList.getLength(); i++) {
        if (algorithmList[i] == SortAlgorithm) {
            if (getLocaleSpecificIndexEntrySupplier(rLocale, SortAlgorithm).is())
                return xIES->loadAlgorithm(rLocale, SortAlgorithm, collatorOptions);
        }
    }
    return false;
}

sal_Bool SAL_CALL IndexEntrySupplier::usePhoneticEntry( const Locale& rLocale )
{
    return LocaleDataImpl::get()->hasPhonetic(rLocale);
}

OUString SAL_CALL IndexEntrySupplier::getPhoneticCandidate( const OUString& rIndexEntry,
        const Locale& rLocale )
{
    if (!getLocaleSpecificIndexEntrySupplier(rLocale, OUString()).is())
        throw RuntimeException();
    return xIES->getPhoneticCandidate(rIndexEntry, rLocale);
}

OUString SAL_CALL IndexEntrySupplier::getIndexKey( const OUString& rIndexEntry,
        const OUString& rPhoneticEntry, const Locale& rLocale )
{
    if (!xIES.is())
        throw RuntimeException();
    return xIES->getIndexKey(rIndexEntry, rPhoneticEntry, rLocale);
}

sal_Int16 SAL_CALL IndexEntrySupplier::compareIndexEntry(
        const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const Locale& rLocale1,
        const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const Locale& rLocale2 )
{
    if (!xIES.is())
        throw RuntimeException();
    return xIES->compareIndexEntry(rIndexEntry1, rPhoneticEntry1, rLocale1,
            rIndexEntry2, rPhoneticEntry2, rLocale2);
}

OUString SAL_CALL IndexEntrySupplier::getIndexCharacter( const OUString& rIndexEntry,
        const Locale& rLocale, const OUString& rSortAlgorithm )
{
    return getLocaleSpecificIndexEntrySupplier(rLocale, rSortAlgorithm)->
        getIndexCharacter( rIndexEntry, rLocale, rSortAlgorithm );
}

bool SAL_CALL IndexEntrySupplier::createLocaleSpecificIndexEntrySupplier(const OUString& name)
{
    Reference < XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.i18n.IndexEntrySupplier_" + name, m_xContext);

    if ( xI.is() ) {
        xIES.set( xI, UNO_QUERY );
        return xIES.is();
    }
    return false;
}

Reference < css::i18n::XExtendedIndexEntrySupplier > const & SAL_CALL
IndexEntrySupplier::getLocaleSpecificIndexEntrySupplier(const Locale& rLocale, const OUString& rSortAlgorithm)
{
    if (xIES.is() && rSortAlgorithm == aSortAlgorithm && rLocale.Language == aLocale.Language &&
            rLocale.Country == aLocale.Country && rLocale.Variant == aLocale.Variant)
        return xIES;
    else {
        rtl::Reference<LocaleDataImpl> ld(new LocaleDataImpl);
        aLocale = rLocale;
        if (rSortAlgorithm.isEmpty())
            aSortAlgorithm = ld->getDefaultIndexAlgorithm( rLocale );
        else
            aSortAlgorithm = rSortAlgorithm;

        OUString module = ld->getIndexModuleByAlgorithm(rLocale, aSortAlgorithm);
        if (!module.isEmpty() && createLocaleSpecificIndexEntrySupplier(module))
            return xIES;

        bool bLoaded = false;
        if (!aSortAlgorithm.isEmpty())
        {
            // Load service with name <base>_<lang>_<country>_<algorithm>
            // or <base>_<bcp47>_<algorithm> and fallbacks.
            bLoaded = createLocaleSpecificIndexEntrySupplier(
                    LocaleDataImpl::getFirstLocaleServiceName( rLocale) + "_" + aSortAlgorithm);
            if (!bLoaded)
            {
                ::std::vector< OUString > aFallbacks( LocaleDataImpl::getFallbackLocaleServiceNames( rLocale));
                for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
                {
                    bLoaded = createLocaleSpecificIndexEntrySupplier( *it + "_" + aSortAlgorithm);
                    if (bLoaded)
                        break;
                }
                if (!bLoaded)
                {
                    // load service with name <base>_<algorithm>
                    bLoaded = createLocaleSpecificIndexEntrySupplier( aSortAlgorithm);
                }
            }
        }
        if (!bLoaded)
        {
            // load default service with name <base>_Unicode
            bLoaded = createLocaleSpecificIndexEntrySupplier( "Unicode");
            if (!bLoaded)
            {
                throw RuntimeException();   // could not load any service
            }
        }
        return xIES;
    }
}

OUString SAL_CALL IndexEntrySupplier::getIndexFollowPageWord( sal_Bool bMorePages,
        const Locale& rLocale )
{
    Sequence< OUString > aFollowPageWords = LocaleDataImpl::get()->getFollowPageWords(rLocale);

    return (bMorePages && aFollowPageWords.getLength() > 1) ?
        aFollowPageWords[1] : (aFollowPageWords.getLength() > 0 ?
                aFollowPageWords[0] : OUString());
}

#define implementationName "com.sun.star.i18n.IndexEntrySupplier"

OUString SAL_CALL
IndexEntrySupplier::getImplementationName()
{
    return OUString( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
IndexEntrySupplier::getSupportedServiceNames()
{
    Sequence< OUString > aRet { implementationName };
    return aRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
