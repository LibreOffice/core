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


#include <indexentrysupplier_common.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <localedata.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_Common::IndexEntrySupplier_Common(const Reference < uno::XComponentContext >& rxContext)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Common";
    collator = new CollatorImpl(rxContext);
    usePhonetic = sal_False;
}

IndexEntrySupplier_Common::~IndexEntrySupplier_Common()
{
    delete collator;
}

Sequence < lang::Locale > SAL_CALL IndexEntrySupplier_Common::getLocaleList() throw (RuntimeException)
{
    throw RuntimeException();
}

Sequence < OUString > SAL_CALL IndexEntrySupplier_Common::getAlgorithmList( const lang::Locale& ) throw (RuntimeException)
{
    throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier_Common::getPhoneticCandidate( const OUString&,
    const lang::Locale& ) throw (RuntimeException)
{
    return OUString();
}

sal_Bool SAL_CALL IndexEntrySupplier_Common::usePhoneticEntry( const lang::Locale& ) throw (RuntimeException)
{
    throw RuntimeException();
}

sal_Bool SAL_CALL IndexEntrySupplier_Common::loadAlgorithm( const lang::Locale& rLocale,
    const OUString& rAlgorithm, sal_Int32 collatorOptions ) throw (RuntimeException)
{
    usePhonetic = LocaleDataImpl().isPhonetic(rLocale, rAlgorithm);
    collator->loadCollatorAlgorithm(rAlgorithm, rLocale, collatorOptions);
    aLocale = rLocale;
    aAlgorithm = rAlgorithm;
    return sal_True;
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexKey( const OUString& rIndexEntry,
    const OUString&, const lang::Locale& ) throw (RuntimeException)
{
    sal_Int32 nPos=0;
    sal_uInt32 indexChar=rIndexEntry.iterateCodePoints(&nPos, 0);
    return OUString(&indexChar, 1);
}

sal_Int16 SAL_CALL IndexEntrySupplier_Common::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString&, const lang::Locale&,
    const OUString& rIndexEntry2, const OUString&, const lang::Locale& )
    throw (RuntimeException)
{
    return sal::static_int_cast< sal_Int16 >(
        collator->compareString(rIndexEntry1, rIndexEntry2));
        // return value of compareString in { -1, 0, 1 }
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexCharacter( const OUString& rIndexEntry,
    const lang::Locale& rLocale, const OUString& ) throw (RuntimeException)
{
    return getIndexKey(rIndexEntry, rIndexEntry, rLocale);
}

OUString SAL_CALL IndexEntrySupplier_Common::getIndexFollowPageWord( sal_Bool,
    const lang::Locale& ) throw (RuntimeException)
{
    throw RuntimeException();
}

const OUString& SAL_CALL
IndexEntrySupplier_Common::getEntry( const OUString& IndexEntry,
    const OUString& PhoneticEntry, const lang::Locale& rLocale ) throw (RuntimeException)
{
    // The condition for using phonetic entry is:
    // usePhonetic is set for the algorithm;
    // rLocale for phonetic entry is same as aLocale for algorithm,
    // which means Chinese phonetic will not be used for Japanese algorithm;
    // phonetic entry is not blank.
    if (usePhonetic && !PhoneticEntry.isEmpty() && rLocale.Language == aLocale.Language &&
            rLocale.Country == aLocale.Country && rLocale.Variant == aLocale.Variant)
        return PhoneticEntry;
    else
        return IndexEntry;
}

OUString SAL_CALL
IndexEntrySupplier_Common::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier_Common::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

Sequence< OUString > SAL_CALL
IndexEntrySupplier_Common::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( implementationName );
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
