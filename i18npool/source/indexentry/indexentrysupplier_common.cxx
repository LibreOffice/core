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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <indexentrysupplier_common.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <localedata.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_Common::IndexEntrySupplier_Common(const Reference < lang::XMultiServiceFactory >& rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Common";
    collator = new CollatorImpl(rxMSF);
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
    usePhonetic = LocaleData().isPhonetic(rLocale, rAlgorithm);
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
    if (usePhonetic && PhoneticEntry.getLength() > 0 && rLocale.Language == aLocale.Language &&
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
