/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:08:56 $
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
#include <indexentrysupplier.hxx>
#include <localedata.hxx>
#include <data/zh_pinyin.h>
#include <data/zh_zhuyin.h>
#include <data/ko_phonetic.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

static const sal_Unicode under = sal_Unicode('_');

namespace com { namespace sun { namespace star { namespace i18n {

IndexEntrySupplier::IndexEntrySupplier( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
}

Sequence < Locale > SAL_CALL IndexEntrySupplier::getLocaleList() throw (RuntimeException)
{
        return LocaleData().getAllInstalledLocaleNames();
}

Sequence < OUString > SAL_CALL IndexEntrySupplier::getAlgorithmList( const Locale& rLocale ) throw (RuntimeException)
{
        return LocaleData().getIndexAlgorithm(rLocale);
}

sal_Bool SAL_CALL IndexEntrySupplier::loadAlgorithm( const Locale& rLocale, const OUString& SortAlgorithm,
        sal_Int32 collatorOptions ) throw (RuntimeException)
{
        Sequence < OUString > algorithmList = getAlgorithmList( rLocale );
        for (sal_Int32 i = 0; i < algorithmList.getLength(); i++) {
            if (algorithmList[i] == SortAlgorithm) {
                if (getLocaleSpecificIndexEntrySupplier(rLocale, SortAlgorithm).is())
                    return xIES->loadAlgorithm(rLocale, SortAlgorithm, collatorOptions);
            }
        }
        return sal_False;
}

sal_Bool SAL_CALL IndexEntrySupplier::usePhoneticEntry( const Locale& rLocale ) throw (RuntimeException)
{
        return LocaleData().hasPhonetic(rLocale);
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
                    candidate += OUString(&address, 1);
                else
                    candidate += space;
            }
        }
        return candidate;
}

OUString SAL_CALL IndexEntrySupplier::getIndexKey( const OUString& rIndexEntry,
        const OUString& rPhoneticEntry, const Locale& rLocale ) throw (RuntimeException)
{
        if (xIES.is())
            return xIES->getIndexKey(rIndexEntry, rPhoneticEntry, rLocale);
        else
            throw RuntimeException();
}

sal_Int16 SAL_CALL IndexEntrySupplier::compareIndexEntry(
        const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const Locale& rLocale1,
        const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const Locale& rLocale2 )
        throw (com::sun::star::uno::RuntimeException)
{
        if (xIES.is())
            return xIES->compareIndexEntry(rIndexEntry1, rPhoneticEntry1, rLocale1,
                                        rIndexEntry2, rPhoneticEntry2, rLocale2);
        else
            throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier::getIndexCharacter( const OUString& rIndexEntry,
        const Locale& rLocale, const OUString& rSortAlgorithm )
        throw (RuntimeException)
{
        return getLocaleSpecificIndexEntrySupplier(rLocale, rSortAlgorithm)->
                        getIndexCharacter( rIndexEntry, rLocale, rSortAlgorithm );
}

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
        if (xIES.is() && rSortAlgorithm == aSortAlgorithm && rLocale.Language == aLocale.Language &&
                rLocale.Country == aLocale.Country && rLocale.Variant == aLocale.Variant)
            return xIES;
        else if (xMSF.is()) {
            aLocale = rLocale;
            if (rSortAlgorithm.getLength() == 0)
                aSortAlgorithm = LocaleData().getDefaultIndexAlgorithm( rLocale );
            else
                aSortAlgorithm = rSortAlgorithm;

            sal_Int32 l = rLocale.Language.getLength();
            sal_Int32 c = rLocale.Country.getLength();
            sal_Int32 v = rLocale.Variant.getLength();
            sal_Int32 a = aSortAlgorithm.getLength();
            OUStringBuffer aBuf(l+c+v+a+4);

            if ((l > 0 && c > 0 && v > 0 && a > 0 &&
                        // load service with name <base>_<lang>_<country>_<varian>_<algorithm>
                        createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).append(under).append(rLocale.Variant).append(under).append(
                                    aSortAlgorithm).makeStringAndClear())) ||
                (l > 0 && c > 0 && a > 0 &&
                        // load service with name <base>_<lang>_<country>_<algorithm>
                        createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).append(under).append(aSortAlgorithm).makeStringAndClear())) ||
                (l > 0 && c > 0 && a > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                                            (rLocale.Country.compareToAscii("HK") == 0 ||
                                            rLocale.Country.compareToAscii("MO") == 0) &&
                        // if the country code is HK or MO, one more step to try TW.
                        createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).appendAscii(
                                    "TW").append(under).append(aSortAlgorithm).makeStringAndClear())) ||
                (l > 0 && a > 0 &&
                        // load service with name <base>_<lang>_<algorithm>
                        createLocaleSpecificIndexEntrySupplier(aBuf.append(rLocale.Language).append(under).append(
                                    aSortAlgorithm).makeStringAndClear())) ||
                        // load service with name <base>_<algorithm>
                (a > 0 && createLocaleSpecificIndexEntrySupplier(aSortAlgorithm)) ||
                        // load default service with name <base>_Unicode
                        createLocaleSpecificIndexEntrySupplier(OUString::createFromAscii("Unicode"))) {
                return xIES;
            }
        }
        throw RuntimeException();
}

OUString SAL_CALL IndexEntrySupplier::getIndexFollowPageWord( sal_Bool bMorePages,
        const Locale& rLocale ) throw (RuntimeException)
{
        Sequence< OUString > aFollowPageWords = LocaleData().getFollowPageWords(rLocale);

        return (bMorePages && aFollowPageWords.getLength() > 1) ?
            aFollowPageWords[1] : (aFollowPageWords.getLength() > 0 ?
            aFollowPageWords[0] : OUString());
}

#define implementationName "com.sun.star.i18n.IndexEntrySupplier"

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
