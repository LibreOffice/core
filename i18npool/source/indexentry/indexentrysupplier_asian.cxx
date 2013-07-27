/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <rtl/ustrbuf.hxx>
#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_alphanumeric.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

extern "C" { static void SAL_CALL thisModule() {} }

IndexEntrySupplier_asian::IndexEntrySupplier_asian(
    const Reference < XMultiServiceFactory >& rxMSF ) : IndexEntrySupplier_Common(rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_asian";
#ifdef SAL_DLLPREFIX
    OUString lib=OUString::createFromAscii( SAL_DLLPREFIX "index_data" SAL_DLLEXTENSION);
#else
    OUString lib=OUString::createFromAscii("index_data" SAL_DLLEXTENSION);
#endif
    hModule = osl_loadModuleRelative(
        &thisModule, lib.pData, SAL_LOADMODULE_DEFAULT );
}

IndexEntrySupplier_asian::~IndexEntrySupplier_asian()
{
    if (hModule) osl_unloadModule(hModule);
}

OUString SAL_CALL
IndexEntrySupplier_asian::getIndexCharacter( const OUString& rIndexEntry,
    const Locale& rLocale, const OUString& rAlgorithm ) throw (RuntimeException)
{
    sal_Int32 i=0;
    sal_uInt32 ch = rIndexEntry.iterateCodePoints(&i, 0);
    if (hModule) {
        OUString get=OUString::createFromAscii("get_indexdata_");
        sal_uInt16** (*func)(sal_Int16*)=NULL;
        if (rLocale.Language.equalsAscii("zh") && OUString::createFromAscii("TW HK MO").indexOf(rLocale.Country) >= 0)
            func=(sal_uInt16** (*)(sal_Int16*))osl_getFunctionSymbol(hModule, (get+rLocale.Language+OUString::createFromAscii("_TW_")+rAlgorithm).pData);
        if (!func)
            func=(sal_uInt16** (*)(sal_Int16*))osl_getFunctionSymbol(hModule, (get+rLocale.Language+OUString('_')+rAlgorithm).pData);
        if (func) {
            sal_Int16 max_index;
            sal_uInt16** idx=func(&max_index);
            if (((sal_Int16)(ch >> 8)) <= max_index) {
                sal_uInt16 address=idx[0][ch >> 8];
                if (address != 0xFFFF) {
                    address=idx[1][address+(ch & 0xFF)];
                    return idx[2] ? OUString(&idx[2][address]) : OUString(address);
                }
            }
        }
    }
    // using alphanumeric index for non-define stirng
    return OUString(&idxStr[(ch & 0xFFFFFF00) ? 0 : ch], 1);
}

OUString SAL_CALL
IndexEntrySupplier_asian::getIndexKey( const OUString& rIndexEntry,
    const OUString& rPhoneticEntry, const Locale& rLocale) throw (RuntimeException)
{
    return getIndexCharacter(getEntry(rIndexEntry, rPhoneticEntry, rLocale), rLocale, aAlgorithm);
}

sal_Int16 SAL_CALL
IndexEntrySupplier_asian::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const Locale& rLocale2 )
    throw (RuntimeException)
{
    sal_Int32 result = collator->compareString(getEntry(rIndexEntry1, rPhoneticEntry1, rLocale1),
                                    getEntry(rIndexEntry2, rPhoneticEntry2, rLocale2));

    // equivalent of phonetic entries does not mean equivalent of index entries.
    // we have to continue comparing index entry here.
    if (result == 0 && usePhonetic && rPhoneticEntry1.getLength() > 0 &&
            rLocale1.Language == rLocale2.Language && rLocale1.Country == rLocale2.Country &&
            rLocale1.Variant == rLocale2.Variant)
        result = collator->compareString(rIndexEntry1, rIndexEntry2);
    return sal::static_int_cast< sal_Int16 >(result); // result in { -1, 0, 1 }
}

OUString SAL_CALL
IndexEntrySupplier_asian::getPhoneticCandidate( const OUString& rIndexEntry,
        const Locale& rLocale ) throw (RuntimeException)
{
    if (hModule) {
        sal_uInt16 **(*func)(sal_Int16*)=NULL;
        const sal_Char *func_name=NULL;
        if (rLocale.Language.equalsAscii("zh"))
            func_name=(OUString::createFromAscii("TW HK MO").indexOf(rLocale.Country) >= 0) ?  "get_zh_zhuyin" : "get_zh_pinyin";
        else if (rLocale.Language.equalsAscii("ko"))
            func_name="get_ko_phonetic";
        if (func_name)
            func=(sal_uInt16 **(*)(sal_Int16*))osl_getFunctionSymbol(hModule, OUString::createFromAscii(func_name).pData);
        if (func) {
            OUStringBuffer candidate;
            sal_Int16 max_index;
            sal_uInt16** idx=func(&max_index);
            OUString aIndexEntry=rIndexEntry;
            for (sal_Int32 i=0,j=0; i < rIndexEntry.getLength(); i=j) {
                sal_uInt32 ch = rIndexEntry.iterateCodePoints(&j, 1);
                if (((sal_Int16)(ch>>8)) <= max_index) {
                    sal_uInt16 address = idx[0][ch>>8];
                    if (address != 0xFFFF) {
                        address = idx[1][address + (ch & 0xFF)];
                        if (i > 0 && rLocale.Language.equalsAscii("zh"))
                            candidate.appendAscii(" ");
                        if (idx[2])
                            candidate.append(&idx[2][address]);
                        else
                            candidate.append(address);
                    } else
                        candidate.appendAscii(" ");
                }
            }
            return candidate.makeStringAndClear();
        }
    }
    return OUString();
}
} } } }
