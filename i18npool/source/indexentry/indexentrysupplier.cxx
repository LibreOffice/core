/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:07:14 $
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

#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>
#include <tools/intn.hxx>
#include <tools/isolang.hxx>
#include <indexentrysupplier.hxx>
#include <data/zh_pinyin.h>
#include <data/zh_zhuyin.h>
#include <data/ko_phonetic.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

static const sal_Unicode under = sal_Unicode('_');

static const struct {
    const char* pLocale;
    const char* pAlgorithms;
    sal_Int16 pAlgorithmCount;
} aLocaleList[] = {
    { "ar",  "alphanumeric", 1 },
    { "bg",  "alphanumeric", 1 },
    { "ca",  "alphanumeric", 1 },
    { "cs",  "alphanumeric", 1 },
    { "da",  "alphanumeric", 1 },
    { "de",  "alphanumeric", 1 },
    { "el",  "alphanumeric", 1 },
    { "en",  "alphanumeric", 1 },
    { "es",  "alphanumeric", 1 },
    { "fi",  "alphanumeric", 1 },
    { "fr",  "alphanumeric", 1 },
    { "he",  "alphanumeric", 1 },
    { "hi",  "alphanumeric", 1 },
    { "hu",  "alphanumeric", 1 },
    { "is",  "alphanumeric", 1 },
    { "it",  "alphanumeric", 1 },
    { "ja",  "phonetic (alphanumeric first) (grouped by syllable),phonetic (alphanumeric first) (grouped by consonant),phonetic (alphanumeric last) (grouped by syllable),phonetic (alphanumeric last) (grouped by consonant)", 4 },
    { "ko",  "dict", 1 },
    { "nb",  "alphanumeric", 1 },
    { "nl",  "alphanumeric", 1 },
    { "nn",  "alphanumeric", 1 },
    { "no",  "alphanumeric", 1 },
    { "pl",  "alphanumeric", 1 },
    { "pt",  "alphanumeric", 1 },
    { "ru",  "alphanumeric", 1 },
    { "sv",  "alphanumeric", 1 },
    { "tr",  "alphanumeric", 1 },
    { "th",  "alphanumeric", 1 },
    { "zh_CN",  "pinyin,stroke,radical", 3 },
    { "zh_TW",  "stroke,radical,zhuyin,pinyin", 4 },
};

static const sal_Int16 nbOfLocales = sizeof(aLocaleList) / sizeof(aLocaleList[0]);

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier::IndexEntrySupplier( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
    implementationName = "com.sun.star.i18n.IndexEntrySupplier";
}


Sequence < Locale > SAL_CALL IndexEntrySupplier::getLocaleList() throw (RuntimeException)
{
    Sequence < Locale > localeList(nbOfLocales);

    for( sal_Int16 i=0; i<nbOfLocales; i++ ) {
        OUString name = OUString::createFromAscii( aLocaleList[i].pLocale );
        sal_Int32 index = 0;
        localeList[i].Language = name.getToken(0, under, index);
        if (index >= 0) {
        localeList[i].Country = name.getToken(0, under, index);
        if (index >= 0)
            localeList[i].Variant = name.getToken(0, under, index);
        }
    }

    return localeList;
}

Sequence < OUString > SAL_CALL IndexEntrySupplier::getAlgorithmList( const Locale& rLocale ) throw (RuntimeException)
{
    Sequence < OUString > algorithmList(1);
    algorithmList[0] = OUString::createFromAscii("alphanumeric");

    for( sal_Int16 i=0; i<nbOfLocales; i++ ) {
        OUString name = OUString::createFromAscii( aLocaleList[i].pLocale );
        sal_Int32 index = 0;

        if (rLocale.Language.equals(name.getToken(0, under, index))) {
        if (index >= 0 && rLocale.Language.compareToAscii("zh") == 0) {

            OUString country = name.getToken(0, under, index);

            // make sure Simplified and Traditional Chinese use right list.
            if ((country.compareToAscii("TW") == 0 &&
                rLocale.Country.compareToAscii("TW") != 0 &&
                rLocale.Country.compareToAscii("HK") != 0 &&
                rLocale.Country.compareToAscii("MO") != 0)
            || (country.compareToAscii("CN") == 0 &&
                rLocale.Country.getLength() > 0 &&
                rLocale.Country.compareToAscii("CN") != 0 &&
                rLocale.Country.compareToAscii("SG") != 0)) {
            continue;
            }
        }

        OUString algorithms = OUString::createFromAscii( aLocaleList[i].pAlgorithms );

        if (aLocaleList[i].pAlgorithmCount > 1) {
            algorithmList.realloc(aLocaleList[i].pAlgorithmCount);
            index = 0;
            for (sal_Int16 j=0; j<aLocaleList[i].pAlgorithmCount; j++)
            algorithmList[j] = algorithms.getToken(0, sal_Unicode(','), index);
        } else
            algorithmList[0] = algorithms;

        break;
        }
    }
    return algorithmList;
}

sal_Bool SAL_CALL IndexEntrySupplier::loadAlgorithm( const Locale& rLocale, const OUString& SortAlgorithm,
    sal_Int32 collatorOptions ) throw (RuntimeException)
{
    if (getLocaleSpecificIndexEntrySupplier(rLocale, SortAlgorithm).is())
        return xIES->loadAlgorithm(rLocale, SortAlgorithm, collatorOptions);
    return sal_False;
}

sal_Bool SAL_CALL IndexEntrySupplier::usePhoneticEntry( const Locale& rLocale ) throw (RuntimeException)
{
    // First implementation only turns the feature on for Japanese language.
    return rLocale.Language.equalsAscii("zh")
        || rLocale.Language.equalsAscii("ja")
        || rLocale.Language.equalsAscii("ko");
}

OUString SAL_CALL IndexEntrySupplier::getPhoneticCandidate( const OUString& rIndexEntry,
    const Locale& rLocale ) throw (RuntimeException)
{
    static OUString space(OUString::createFromAscii(" "));
    OUString candidate;
    // TODO: the phonetic candidate will be provided by language engine for CJK.
    if (rLocale.Language.equalsAscii("zh")) {
        sal_Unicode *Str;
        sal_uInt16 *Index1, *Index2;
        if (rLocale.Country.equalsAscii("TW") ||
        rLocale.Country.equalsAscii("HK") ||
        rLocale.Country.equalsAscii("MO")) {
            Str = ZhuYinStr_zh;
            Index1 = ZhuYinIndex1_zh;
            Index2 = ZhuYinIndex2_zh;
        } else {
            Str = PinYinStr_zh;
            Index1 = PinYinIndex1_zh;
            Index2 = PinYinIndex2_zh;
        }

        for (sal_Int32 i=0; i < rIndexEntry.getLength(); i++) {
        sal_Unicode ch = rIndexEntry[i];
        sal_uInt16 address = Index1[ch>>8];
        if (address != 0xFFFF)
            address = Index2[address + (ch & 0xFF)];
        if (i > 0)
            candidate += space;
        if (address != 0xFFFF)
            candidate += OUString(&Str[address]);
        }
    } else if (rLocale.Language.equalsAscii("ja")) {
        ; // TODO
    } else if (rLocale.Language.equalsAscii("ko")) {
        for (sal_Int32 i=0; i < rIndexEntry.getLength(); i++) {
        sal_Unicode ch = rIndexEntry[i];
        sal_uInt16 address = PhoneticIndex_ko[ch>>8];
        if (address != 0xFFFF)
            address = PhoneticCharacter_ko[address + (ch & 0xFF)];
        if (address != 0xFFFF)
            candidate += OUString(address);
        else
            candidate += space;
        }
    }
    return candidate;
}

OUString SAL_CALL IndexEntrySupplier::getIndexKey( const OUString& rIndexEntry,
    const OUString& rPhoneticEntry, const Locale& rLocale ) throw (RuntimeException)
{
    return xIES->getIndexKey(rIndexEntry, rPhoneticEntry, rLocale);
}

sal_Int16 SAL_CALL IndexEntrySupplier::compareIndexEntry(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const Locale& rLocale2 )
    throw (com::sun::star::uno::RuntimeException)
{
    return xIES->compareIndexEntry(rIndexEntry1, rPhoneticEntry1, rLocale1,
                    rIndexEntry2, rPhoneticEntry2, rLocale2);
}

OUString SAL_CALL IndexEntrySupplier::getIndexCharacter( const OUString& rIndexEntry,
    const Locale& rLocale, const OUString& rSortAlgorithm )
    throw (RuntimeException)
{
    return getLocaleSpecificIndexEntrySupplier(rLocale, rSortAlgorithm)->
            getIndexCharacter( rIndexEntry, rLocale, rSortAlgorithm );
}

#if (_MSC_VER < 1300)
sal_Bool SAL_CALL operator == (const Locale& l1, const Locale& l2) {
    return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
}
#else
extern sal_Bool SAL_CALL operator == (const Locale& l1, const Locale& l2);
#endif

sal_Bool SAL_CALL IndexEntrySupplier::createLocaleSpecificIndexEntrySupplier(const OUString& name) throw( RuntimeException )
{
    Reference < XInterface > xI = xMSF->createInstance(
        OUString::createFromAscii("com.sun.star.i18n.IndexEntrySupplier_") + name);

    if ( xI.is() ) {
        xI->queryInterface( ::getCppuType((const Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier>*)0) ) >>= xIES;
        return xIES.is();
    }
    return sal_False;
}

Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > SAL_CALL
IndexEntrySupplier::getLocaleSpecificIndexEntrySupplier(const Locale& rLocale, const OUString& rSortAlgorithm) throw (RuntimeException)
{
    if (xIES.is() && rLocale == aLocale && rSortAlgorithm == aSortAlgorithm)
        return xIES;
    else if (xMSF.is()) {
        aLocale = rLocale;
        aSortAlgorithm = rSortAlgorithm;

        static OUString tw(OUString::createFromAscii("TW"));
        static OUString unicode(OUString::createFromAscii("Unicode"));

        sal_Int32 l = rLocale.Language.getLength();
        sal_Int32 c = rLocale.Country.getLength();
        sal_Int32 v = rLocale.Variant.getLength();
        sal_Int32 a = rSortAlgorithm.getLength();
        OUStringBuffer aBuf(l+c+v+a+4);

        if ((l > 0 && c > 0 && v > 0 && a > 0 &&
            // load service with name <base>_<lang>_<country>_<varian>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).append(under).append(rLocale.Variant).append(under).append(
                    rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && c > 0 && a > 0 &&
            // load service with name <base>_<lang>_<country>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rLocale.Country).append(under).append(rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && c > 0 && a > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                        (rLocale.Country.compareToAscii("HK") == 0 ||
                        rLocale.Country.compareToAscii("MO") == 0) &&
            // if the country code is HK or MO, one more step to try TW.
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    tw).append(under).append(rSortAlgorithm).makeStringAndClear())) ||
        (l > 0 && a > 0 &&
            // load service with name <base>_<lang>_<algorithm>
            createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                    rSortAlgorithm).makeStringAndClear())) ||
            // load service with name <algorithm>
        (a > 0 &&
            createLocaleSpecificIndexEntrySupplier(rSortAlgorithm)) ||
            // load default service with name <base>_Unicode
            createLocaleSpecificIndexEntrySupplier(unicode)) {
        return xIES;
        }
    }
    throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier::getIndexFollowPageWord( sal_Bool bMorePages,
    const Locale& rLocale ) throw (RuntimeException)
{
    static const struct {
        const sal_Char *pLang, *pFollowPage, *pFollowPages;
    } aFollowPageArr[] = {
        { "en", "p.", "pp." },
        { "de", "f.", "ff." },
        { "es", " s.", " ss." },
        { "it", " e seg.", " e segg." },
        { "fr", " sv", " sv" },
        { "sv", "f.", "ff." },
        { "zh", "", "" },
        { "ja", "p.", "pp." },
        { "ko", "", "" },
        { 0, 0, 0 }
    };

    int n;
    for( n = 0; aFollowPageArr[ n ].pLang; ++n )
        if( 0 == rLocale.Language.compareToAscii( aFollowPageArr[ n ].pLang ))
        break;

    if( !aFollowPageArr[ n ].pLang )
        n = 0;      //the default for unknow languages

    return OUString::createFromAscii( bMorePages ? aFollowPageArr[ n ].pFollowPages
                            : aFollowPageArr[ n ].pFollowPage );
}

OUString SAL_CALL
IndexEntrySupplier::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
IndexEntrySupplier::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

Sequence< OUString > SAL_CALL
IndexEntrySupplier::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( implementationName );
    return aRet;
}

} } } }
