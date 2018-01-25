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

#include <indexentrysupplier_default.hxx>
#include <localedata.hxx>
#include <i18nutil/unicode.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

IndexEntrySupplier_Unicode::IndexEntrySupplier_Unicode(
    const css::uno::Reference < css::uno::XComponentContext >& rxContext ) :
    IndexEntrySupplier_Common(rxContext)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Unicode";
    index.reset( new Index(rxContext) );
}

IndexEntrySupplier_Unicode::~IndexEntrySupplier_Unicode()
{
}

sal_Bool SAL_CALL IndexEntrySupplier_Unicode::loadAlgorithm( const lang::Locale& rLocale,
    const OUString& rAlgorithm, sal_Int32 collatorOptions )
{
    index->init(rLocale, rAlgorithm);
    return IndexEntrySupplier_Common::loadAlgorithm(rLocale, rAlgorithm, collatorOptions);
}

OUString SAL_CALL IndexEntrySupplier_Unicode::getIndexKey( const OUString& rIndexEntry,
    const OUString& rPhoneticEntry, const lang::Locale& rLocale )
{
    return index->getIndexDescription(getEntry(rIndexEntry, rPhoneticEntry, rLocale));
}

sal_Int16 SAL_CALL IndexEntrySupplier_Unicode::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const lang::Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const lang::Locale& rLocale2 )
{
    sal_Int16 result =
            index->getIndexWeight(getEntry(rIndexEntry1, rPhoneticEntry1, rLocale1)) -
            index->getIndexWeight(getEntry(rIndexEntry2, rPhoneticEntry2, rLocale2));
    if (result == 0)
        return IndexEntrySupplier_Common::compareIndexEntry(
                    rIndexEntry1, rPhoneticEntry1, rLocale1,
                    rIndexEntry2, rPhoneticEntry2, rLocale2);
    return result > 0 ? 1 : -1;
}

OUString SAL_CALL IndexEntrySupplier_Unicode::getIndexCharacter( const OUString& rIndexEntry,
    const lang::Locale& rLocale, const OUString& rAlgorithm ) {

    if (loadAlgorithm( rLocale, rAlgorithm, CollatorOptions::CollatorOptions_IGNORE_CASE_ACCENT))
        return index->getIndexDescription(rIndexEntry);
    else
        return IndexEntrySupplier_Common::getIndexCharacter(rIndexEntry, rLocale, rAlgorithm);
}

IndexTable::IndexTable()
    : start(0)
    , end(0)
    , table(nullptr)
{
}

IndexTable::~IndexTable()
{
    if (table) free(table);
}

void IndexTable::init(sal_Unicode start_, sal_Unicode end_, IndexKey const *keys, sal_Int16 key_count, Index *index)
{
    start=start_;
    end=end_;
    table = static_cast<sal_uInt8*>(malloc((end-start+1)*sizeof(sal_uInt8)));
    for (sal_Unicode i = start; i <= end; i++) {
        sal_Int16 j;
        for (j = 0; j < key_count; j++) {
            if (keys[j].key > 0 && (i == keys[j].key || index->compare(i, keys[j].key) == 0)) {
                table[i-start] = sal::static_int_cast<sal_uInt8>(j);
                break;
            }
        }
        if (j == key_count)
            table[i-start] = 0xFF;
    }
}

Index::Index(const css::uno::Reference < css::uno::XComponentContext >& rxContext)
    : table_count(0)
    , key_count(0)
    , mkey_count(0)
    , collator( new CollatorImpl(rxContext) )
{
}

Index::~Index()
{
}

sal_Int16 Index::compare(sal_Unicode c1, sal_Unicode c2)
{
    return sal::static_int_cast<sal_Int16>( collator->compareString(OUString(&c1, 1), OUString(&c2, 1)) );
}

sal_Int16 Index::getIndexWeight(const OUString& rIndexEntry)
{
    sal_Int32 startPos=0;
    if (!skipping_chars.isEmpty())
        while (skipping_chars.indexOf(rIndexEntry[startPos]) >= 0)
            startPos++;
    if (mkey_count > 0) {
        for (sal_Int16 i = 0; i < mkey_count; i++) {
            sal_Int32 len = keys[mkeys[i]].mkey.getLength();
            if (collator->compareSubstring(rIndexEntry, startPos, len,
                                    keys[mkeys[i]].mkey, 0, len) == 0)
                return mkeys[i];
        }
    }
    sal_Unicode code = startPos < rIndexEntry.getLength() ? rIndexEntry[startPos] : 0;
    for (sal_Int16 i = 0; i < table_count; i++) {
        if (tables[i].start <= code && code <= tables[i].end)
            return tables[i].table[code-tables[i].start];
    }
    return 0xFF;
}

OUString Index::getIndexDescription(const OUString& rIndexEntry)
{
    sal_Int16 wgt = getIndexWeight(rIndexEntry);
    if (wgt < MAX_KEYS) {
        if (!keys[wgt].desc.isEmpty())
            return keys[wgt].desc;
        else if (keys[wgt].key > 0)
            return OUString(&keys[wgt].key, 1);
        else
            return keys[wgt].mkey;
    }
    sal_Int32 nPos=0;
    sal_uInt32 indexChar=rIndexEntry.iterateCodePoints(&nPos, 0);
    return OUString(&indexChar, 1);
}

#define LOCALE_EN lang::Locale("en", OUString(), OUString())

void Index::makeIndexKeys(const lang::Locale &rLocale, const OUString &algorithm)
{
    OUString keyStr = LocaleDataImpl::get()->getIndexKeysByAlgorithm(rLocale, algorithm);

    if (keyStr.isEmpty()) {
        keyStr = LocaleDataImpl::get()->getIndexKeysByAlgorithm(LOCALE_EN,
                    LocaleDataImpl::get()->getDefaultIndexAlgorithm(LOCALE_EN));
        if (keyStr.isEmpty())
            throw RuntimeException();
    }

    sal_Int16 len = sal::static_int_cast<sal_Int16>( keyStr.getLength() );
    mkey_count=key_count=0;
    skipping_chars=OUString();
    sal_Int16 i, j;

    for (i = 0; i < len && key_count < MAX_KEYS; i++)
    {
        sal_Unicode curr = keyStr[i];
        sal_Unicode close = ')';

        if (unicode::isWhiteSpace(curr))
            continue;

        switch(curr) {
            case u'-': {
                    if (key_count <= 0 || i + 1 >= len)
                        throw RuntimeException();
                    for (curr = keyStr[++i]; key_count < MAX_KEYS && keys[key_count-1].key < curr; key_count++) {
                        keys[key_count].key = keys[key_count-1].key+1;
                        keys[key_count].desc.clear();
                    }
                    break;
                }
            case u'[':
                for (i++; i < len && keyStr[i] != ']'; i++) {
                    if (unicode::isWhiteSpace(keyStr[i])) {
                        continue;
                    } else if (keyStr[i] == '_') {
                        for (curr=keyStr[i-1]+1;  curr <= keyStr[i+1]; curr++)
                            skipping_chars+=OUStringLiteral1(curr);
                        i+=2;
                    } else {
                        skipping_chars+=OUStringLiteral1(keyStr[i]);
                    }
                }
                break;
            case u'{':
                close = '}';
                SAL_FALLTHROUGH;
            case u'(': {
                    if (key_count <= 0)
                        throw RuntimeException();

                    sal_Int16 end = i+1;
                    for (; end < len && keyStr[end] != close; end++) ;

                    if (end >= len) // no found
                        throw RuntimeException();
                    if (close == ')')
                        keys[key_count-1].desc = keyStr.copy(i+1, end-i-1);
                    else {
                        mkeys[mkey_count++]=key_count;
                        keys[key_count].key = 0;
                        keys[key_count].mkey = keyStr.copy(i+1, end-i-1);
                        keys[key_count++].desc.clear();
                    }
                    i=end+1;
                    break;
                }
            default:
                keys[key_count].key = curr;
                keys[key_count++].desc.clear();
                break;
        }
    }
    for (i = 0; i < mkey_count; i++) {
        for (j=i+1; j < mkey_count; j++) {
            if (keys[mkeys[i]].mkey.getLength() < keys[mkeys[j]].mkey.getLength()) {
                sal_Int16 k = mkeys[i];
                mkeys[i] = mkeys[j];
                mkeys[j] = k;
            }
        }
    }
}

void Index::init(const lang::Locale &rLocale, const OUString& algorithm)
{
    makeIndexKeys(rLocale, algorithm);

    Sequence< UnicodeScript > scriptList = LocaleDataImpl::get()->getUnicodeScripts( rLocale );

    if (scriptList.getLength() == 0) {
        scriptList = LocaleDataImpl::get()->getUnicodeScripts(LOCALE_EN);
        if (scriptList.getLength() == 0)
            throw RuntimeException();
    }

    table_count = sal::static_int_cast<sal_Int16>( scriptList.getLength() );
    if (table_count > MAX_TABLES)
        throw RuntimeException();

    collator->loadCollatorAlgorithm(algorithm, rLocale, CollatorOptions::CollatorOptions_IGNORE_CASE_ACCENT);
    sal_Int16 j=0;
    sal_Unicode start = unicode::getUnicodeScriptStart((UnicodeScript)0);
    sal_Unicode end = unicode::getUnicodeScriptEnd((UnicodeScript)0);
    for (sal_Int32 i= (scriptList[0] == (UnicodeScript)0) ? 1 : 0; i< scriptList.getLength(); i++) {
        if (unicode::getUnicodeScriptStart(scriptList[i]) != end+1) {
            tables[j++].init(start, end, keys, key_count, this);
            start = unicode::getUnicodeScriptStart(scriptList[i]);
        }
        end = unicode::getUnicodeScriptEnd(scriptList[i]);
    }
    tables[j++].init(start, end, keys, key_count, this);
    table_count = j;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
