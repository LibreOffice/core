/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_asian.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:45:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <rtl/ustrbuf.hxx>
#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_alphanumeric.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier_asian::IndexEntrySupplier_asian(
    const Reference < XMultiServiceFactory >& rxMSF ) : IndexEntrySupplier_Common(rxMSF)
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier_asian";
#ifdef SAL_DLLPREFIX
    OUString lib=OUString::createFromAscii(SAL_DLLPREFIX"index_data"SAL_DLLEXTENSION);
#else
    OUString lib=OUString::createFromAscii("index_data"SAL_DLLEXTENSION);
#endif
    hModule = osl_loadModule( lib.pData, SAL_LOADMODULE_DEFAULT );
}

IndexEntrySupplier_asian::~IndexEntrySupplier_asian()
{
    if (hModule) osl_unloadModule(hModule);
}

OUString SAL_CALL
IndexEntrySupplier_asian::getIndexCharacter( const OUString& rIndexEntry,
    const Locale& rLocale, const OUString& rAlgorithm ) throw (RuntimeException)
{
    sal_Unicode ch = rIndexEntry.toChar();
    if (hModule) {
        OUString get=OUString::createFromAscii("get_indexdata_");
        int (*func)()=NULL;
        if (rLocale.Language.equalsAscii("zh") && OUString::createFromAscii("TW HK MO").indexOf(rLocale.Country) >= 0)
            func=(int (*)())osl_getFunctionSymbol(hModule, (get+rLocale.Language+OUString::createFromAscii("_TW_")+rAlgorithm).pData);
        if (!func)
            func=(int (*)())osl_getFunctionSymbol(hModule, (get+rLocale.Language+OUString('_')+rAlgorithm).pData);
        if (func) {
            sal_uInt16** idx=(sal_uInt16**)func();
            sal_uInt16 address=idx[0][ch >> 8];
            if (address != 0xFFFF) {
                address=idx[1][address+(ch & 0xFF)];
                return idx[2] ? OUString(&idx[2][address]) : OUString(address);
            }
        }
    }
    // using alphanumeric index for non-define stirng
    return OUString(&idxStr[(ch & 0xFF00) ? 0 : ch], 1);
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
        int (*func)()=NULL;
        const sal_Char *func_name=NULL;
        if (rLocale.Language.equalsAscii("zh"))
            func_name=(OUString::createFromAscii("TW HK MO").indexOf(rLocale.Country) >= 0) ?  "get_zh_zhuyin" : func_name="get_zh_pinyin";
        else if (rLocale.Language.equalsAscii("ko"))
            func_name="get_ko_phonetic";
        if (func_name)
            func=(int (*)())osl_getFunctionSymbol(hModule, OUString::createFromAscii(func_name).pData);
        if (func) {
            OUStringBuffer candidate;
            sal_uInt16** idx=(sal_uInt16**)func();
            for (sal_Int32 i=0; i < rIndexEntry.getLength(); i++) {
                sal_Unicode ch = rIndexEntry[i];
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
            return candidate.makeStringAndClear();
        }
    }
    return OUString();
}
} } } }
