/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_default.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:39:34 $
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

#include <indexentrysupplier_default.hxx>
#include <localedata.hxx>
#include <i18nutil/unicode.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_Unicode::IndexEntrySupplier_Unicode(
    const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) :
    IndexEntrySupplier_Common(rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_Unicode";
    index = new Index(collator);
}

IndexEntrySupplier_Unicode::~IndexEntrySupplier_Unicode()
{
    delete index;
}

sal_Bool SAL_CALL IndexEntrySupplier_Unicode::loadAlgorithm( const lang::Locale& rLocale,
    const OUString& rAlgorithm, sal_Int32 collatorOptions ) throw (RuntimeException)
{
    index->init(rLocale, rAlgorithm);
    return IndexEntrySupplier_Common::loadAlgorithm(rLocale, rAlgorithm, collatorOptions);
}

OUString SAL_CALL IndexEntrySupplier_Unicode::getIndexKey( const OUString& rIndexEntry,
    const OUString& rPhoneticEntry, const lang::Locale& rLocale ) throw (RuntimeException)
{
    return index->getIndexDescription(getEntry(rIndexEntry, rPhoneticEntry, rLocale));
}

sal_Int16 SAL_CALL IndexEntrySupplier_Unicode::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const lang::Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const lang::Locale& rLocale2 )
    throw (RuntimeException)
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
    const lang::Locale& rLocale, const OUString& rAlgorithm ) throw (RuntimeException) {

    if (loadAlgorithm( rLocale, rAlgorithm, CollatorOptions::CollatorOptions_IGNORE_CASE))
        return index->getIndexDescription(rIndexEntry);
    else
        return IndexEntrySupplier_Common::getIndexCharacter(rIndexEntry, rLocale, rAlgorithm);
}

IndexTable::IndexTable()
{
    table = NULL;
}

IndexTable::~IndexTable()
{
    if (table) free(table);
}

void IndexTable::init(sal_Unicode start_, sal_Unicode end_, IndexKey *keys, sal_Int16 key_count, Index *index)
{
    start=start_;
    end=end_;
    table = (sal_uInt8*) malloc((end-start+1)*sizeof(sal_uInt8));
    for (sal_Unicode i = start; i <= end; i++) {
        sal_Int16 j;
        for (j = 0; j < key_count; j++) {
            if (i == keys[j].key || index->compare(i, keys[j].key) == 0) {
                table[i-start] = j;
                break;
            }
        }
        if (j == key_count)
            table[i-start] = 0xFF;
    }
}

Index::Index(CollatorImpl *col)
{
    collator = col;
}

sal_Int16 Index::compare(sal_Unicode c1, sal_Unicode c2)
{
    return collator->compareString(OUString(&c1, 1), OUString(&c2, 1));
}

sal_Int16 Index::getIndexWeight(const OUString& rIndexEntry)
{
    sal_Unicode code = rIndexEntry[0];
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
        if (keys[wgt].desc.getLength())
            return keys[wgt].desc;
        else
            return OUString(&keys[wgt].key, 1);
    }
    return rIndexEntry.copy(0, 1);
}

#define LOCALE_EN lang::Locale(OUString::createFromAscii("en"), OUString(), OUString())

void Index::makeIndexKeys(const lang::Locale &rLocale, const OUString &algorithm) throw (RuntimeException)
{
    OUString keyStr = LocaleData().getIndexKeysByAlgorithm(rLocale, algorithm);

    if (!keyStr.getLength()) {
        keyStr = LocaleData().getIndexKeysByAlgorithm(LOCALE_EN,
                    LocaleData().getDefaultIndexAlgorithm(LOCALE_EN));
        if (!keyStr)
            throw RuntimeException();
    }

    sal_Int16 j = 0, len = keyStr.getLength();

    for (sal_Int16 i = 0; i < len && j < MAX_KEYS; i++)
    {
        sal_Unicode curr = keyStr[i];

        if (unicode::isWhiteSpace(curr))
            continue;

        switch(curr) {
            case sal_Unicode('-'):
                if (j > 0 && i + 1 < len ) {
                    for (curr = keyStr[++i]; j < MAX_KEYS && keys[j-1].key < curr; j++) {
                        keys[j].key = keys[j-1].key+1;
                        keys[j].desc = OUString();
                    }
                } else
                    throw RuntimeException();
                break;
            case sal_Unicode('('):
                if (j > 0) {
                    sal_Int16 end = i+1;
                    while (end < len && keyStr[end] != sal_Unicode(')')) end++;

                    if (end >= len) // no found
                        throw RuntimeException();
                    keys[j-1].desc = keyStr.copy(i+1, end-i-1);
                    i=end+1;
                } else
                    throw RuntimeException();
                break;
            default:
                keys[j].key = curr;
                keys[j++].desc = OUString();
                break;
        }
    }
    key_count = j;
}

void Index::init(const lang::Locale &rLocale, const OUString& algorithm) throw (RuntimeException)
{
    makeIndexKeys(rLocale, algorithm);

    Sequence< UnicodeScript > scriptList = LocaleData().getUnicodeScripts( rLocale );

    if (scriptList.getLength() == 0) {
        scriptList = LocaleData().getUnicodeScripts(LOCALE_EN);
        if (scriptList.getLength() == 0)
            throw RuntimeException();
    }

    table_count = scriptList.getLength();
    if (table_count > MAX_TABLES)
        throw RuntimeException();

    collator->loadCollatorAlgorithm(algorithm, rLocale, CollatorOptions::CollatorOptions_IGNORE_CASE);
    sal_Int16 i, j=0;
    sal_Unicode start = unicode::getUnicodeScriptStart(scriptList[0]);
    sal_Unicode end = unicode::getUnicodeScriptEnd(scriptList[0]);
    for (i=1; i< scriptList.getLength(); i++) {
        if (unicode::getUnicodeScriptStart(scriptList[i]) != end+1) {
            tables[j++].init(start, end, keys, key_count, this);
            start = unicode::getUnicodeScriptStart(scriptList[i]);
        }
        end = unicode::getUnicodeScriptEnd(scriptList[i]);
    }
    tables[j++].init(start, end, keys, key_count, this);
    table_count = j;
}

} } } }
