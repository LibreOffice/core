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


#include <indexentrysupplier_ja_phonetic.hxx>
#include <collatorImpl.hxx>
#include "data/indexdata_alphanumeric.h"
#include "data/indexdata_ja_phonetic.h"
#include <string.h>

using namespace ::com::sun::star::i18n;

namespace i18npool {

OUString SAL_CALL IndexEntrySupplier_ja_phonetic::getIndexCharacter( const OUString& rIndexEntry,
    const css::lang::Locale& /*rLocale*/, const OUString& /*rSortAlgorithm*/ )
{
    sal_Unicode ch=rIndexEntry.toChar();
    sal_uInt16 first = idx[ ch >> 8 ];
    if (first == 0xFFFF) {
        // using alphanumeric index for non-define string
        return OUString(&idxStr[(ch & 0xFF00) ? 0 : ch], 1);
    } else {
        const sal_Unicode *idx2 = strstr(implementationName, "syllable") ? syllable : consonant;
        return OUString(&idx2[ first + (ch & 0xff) ], 1);
    }
}

OUString SAL_CALL IndexEntrySupplier_ja_phonetic::getIndexKey( const OUString& IndexEntry,
    const OUString& PhoneticEntry, const css::lang::Locale& rLocale )
{
    return getIndexCharacter( PhoneticEntry.isEmpty() ? IndexEntry : PhoneticEntry , rLocale, OUString());
}

sal_Int16 SAL_CALL IndexEntrySupplier_ja_phonetic::compareIndexEntry(
    const OUString& IndexEntry1, const OUString& PhoneticEntry1, const css::lang::Locale& rLocale1,
    const OUString& IndexEntry2, const OUString& PhoneticEntry2, const css::lang::Locale& rLocale2 )
{
    sal_Int16 result = sal::static_int_cast<sal_Int16>( collator->compareString(
        IndexEntrySupplier_ja_phonetic::getIndexKey(IndexEntry1, PhoneticEntry1, rLocale1),
        IndexEntrySupplier_ja_phonetic::getIndexKey(IndexEntry2, PhoneticEntry2, rLocale2)) );

    if (result == 0)
        return IndexEntrySupplier_Common::compareIndexEntry(
                    IndexEntry1, PhoneticEntry1, rLocale1,
                    IndexEntry2, PhoneticEntry2, rLocale2);
    return result;
}

const OUStringLiteral first = u"phonetic (alphanumeric first)";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_syllable::loadAlgorithm(
    const css::lang::Locale& rLocale, const OUString& /*SortAlgorithm*/,
    sal_Int32 collatorOptions )
{
    return collator->loadCollatorAlgorithm(first, rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_first_by_consonant::loadAlgorithm(
    const css::lang::Locale& rLocale, const OUString& /*SortAlgorithm*/,
    sal_Int32 collatorOptions )
{
    return collator->loadCollatorAlgorithm(first, rLocale, collatorOptions) == 0;
}

const OUStringLiteral last = u"phonetic (alphanumeric last)";
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_syllable::loadAlgorithm(
    const css::lang::Locale& rLocale, const OUString& /*SortAlgorithm*/,
    sal_Int32 collatorOptions )
{
    return collator->loadCollatorAlgorithm(last, rLocale, collatorOptions) == 0;
}
sal_Bool SAL_CALL IndexEntrySupplier_ja_phonetic_alphanumeric_last_by_consonant::loadAlgorithm(
    const css::lang::Locale& rLocale, const OUString& /*SortAlgorithm*/,
    sal_Int32 collatorOptions )
{
    return collator->loadCollatorAlgorithm(last, rLocale, collatorOptions) == 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
