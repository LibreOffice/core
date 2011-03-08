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

#include <collatorImpl.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace com { namespace sun { namespace star { namespace i18n {

CollatorImpl::CollatorImpl( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF(rxMSF)
{
    if ( rxMSF.is()) {
        Reference < XInterface > xI =
            xMSF->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.LocaleData")));
        if ( xI.is() )
            xI->queryInterface(::getCppuType((const Reference< XLocaleData>*)0)) >>= localedata;
    }
    cachedItem = NULL;
}

CollatorImpl::~CollatorImpl()
{
    // Clear lookuptable
    for (size_t l = 0; l < lookupTable.size(); l++)
        delete lookupTable[l];
    lookupTable.clear();
}

sal_Int32 SAL_CALL
CollatorImpl::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException)
{
    if (cachedItem)
        return cachedItem->xC->compareSubstring(str1, off1, len1, str2, off2, len2);

    sal_Unicode *unistr1 = (sal_Unicode*) str1.getStr() + off1;
    sal_Unicode *unistr2 = (sal_Unicode*) str2.getStr() + off2;
    for (int i = 0; i < len1 && i < len2; i++)
        if (unistr1[i] != unistr2[i])
            return unistr1[i] < unistr2[i] ? -1 : 1;
    return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
}

sal_Int32 SAL_CALL
CollatorImpl::compareString( const OUString& in_str1, const OUString& in_str2) throw(RuntimeException)
{
    if (cachedItem)
        return cachedItem->xC->compareString(in_str1, in_str2);

    return CollatorImpl::compareSubstring(in_str1, 0, in_str1.getLength(), in_str2, 0, in_str2.getLength());
}


sal_Int32 SAL_CALL
CollatorImpl::loadDefaultCollator(const lang::Locale& rLocale, sal_Int32 collatorOptions) throw(RuntimeException)
{
    const Sequence< Implementation > &imp = localedata->getCollatorImplementations(rLocale);
    for (sal_Int16 i = 0; i < imp.getLength(); i++)
        if (imp[i].isDefault)
            return loadCollatorAlgorithm(imp[i].unoID, rLocale, collatorOptions);

    throw RuntimeException(); // not default is defined
    //return 0;
}

sal_Int32 SAL_CALL
CollatorImpl::loadCollatorAlgorithm(const OUString& impl, const lang::Locale& rLocale, sal_Int32 collatorOptions)
    throw(RuntimeException)
{
    if (! cachedItem || ! cachedItem->equals(rLocale, impl))
        loadCachedCollator(rLocale, impl);

    if (cachedItem)
        cachedItem->xC->loadCollatorAlgorithm(cachedItem->algorithm, nLocale = rLocale, collatorOptions);
    else
        throw RuntimeException(); // impl could not be loaded

    return 0;
}

void SAL_CALL
CollatorImpl::loadCollatorAlgorithmWithEndUserOption(const OUString& impl, const lang::Locale& rLocale,
    const Sequence< sal_Int32 >& collatorOptions) throw(RuntimeException)
{
    sal_Int32 options = 0;
    for (sal_Int32 i = 0; i < collatorOptions.getLength(); i++)
        options |= collatorOptions[i];
    loadCollatorAlgorithm(impl, rLocale, options);
}

Sequence< OUString > SAL_CALL
CollatorImpl::listCollatorAlgorithms( const lang::Locale& rLocale ) throw(RuntimeException)
{
    nLocale = rLocale;
    const Sequence< Implementation > &imp = localedata->getCollatorImplementations(rLocale);
    Sequence< OUString > list(imp.getLength());

    for (sal_Int32 i = 0; i < imp.getLength(); i++) {
        //if the current algorithm is default and the position is not on the first one, then switch
        if (imp[i].isDefault && i) {
            list[i] = list[0];
            list[0] = imp[i].unoID;
        }
        else
            list[i] = imp[i].unoID;
    }
    return list;
}

Sequence< sal_Int32 > SAL_CALL
CollatorImpl::listCollatorOptions( const OUString& /*collatorAlgorithmName*/ ) throw(RuntimeException)
{
    Sequence< OUString > option_str = localedata->getCollationOptions(nLocale);
    Sequence< sal_Int32 > option_int(option_str.getLength());

    for (sal_Int32 i = 0; i < option_str.getLength(); i++)
        option_int[i] =
            option_str[i].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IGNORE_CASE")) ?  CollatorOptions::CollatorOptions_IGNORE_CASE :
            option_str[i].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IGNORE_KANA")) ?  CollatorOptions::CollatorOptions_IGNORE_KANA :
            option_str[i].equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IGNORE_WIDTH")) ?  CollatorOptions::CollatorOptions_IGNORE_WIDTH : 0;

    return option_int;
}

sal_Bool SAL_CALL
CollatorImpl::createCollator(const lang::Locale& rLocale, const OUString& serviceName, const OUString& rSortAlgorithm)
    throw(RuntimeException)
{
    for (size_t l = 0; l < lookupTable.size(); l++) {
        cachedItem = lookupTable[l];
        if (cachedItem->service.equals(serviceName)) {// cross locale sharing
            lookupTable.push_back(cachedItem = new lookupTableItem(rLocale, rSortAlgorithm, serviceName, cachedItem->xC));
            return sal_True;
        }
    }
    if (xMSF.is()) {
        Reference < XInterface > xI =
            xMSF->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.Collator_")) + serviceName);

        if (xI.is()) {
            Reference < XCollator > xC;
            xI->queryInterface( getCppuType((const Reference< XCollator>*)0) ) >>= xC;
            if (xC.is()) {
                lookupTable.push_back(cachedItem = new lookupTableItem(rLocale, rSortAlgorithm, serviceName, xC));
                return sal_True;
            }
        }
        return sal_False;
    }
    throw RuntimeException();
}

void SAL_CALL
CollatorImpl::loadCachedCollator(const lang::Locale& rLocale, const OUString& rSortAlgorithm)
    throw(RuntimeException)
{
    for (size_t i = 0; i < lookupTable.size(); i++) {
        cachedItem = lookupTable[i];
        if (cachedItem->equals(rLocale, rSortAlgorithm)) {
            return;
        }
    }

    static sal_Unicode under = (sal_Unicode) '_';
    static OUString tw(RTL_CONSTASCII_USTRINGPARAM("TW"));
    static OUString unicode(RTL_CONSTASCII_USTRINGPARAM("Unicode"));

    sal_Int32 l = rLocale.Language.getLength();
    sal_Int32 c = rLocale.Country.getLength();
    sal_Int32 v = rLocale.Variant.getLength();
    sal_Int32 a = rSortAlgorithm.getLength();
    OUStringBuffer aBuf(l+c+v+a+4);

    if ((l > 0 && c > 0 && v > 0 && a > 0 &&
                // load service with name <base>_<lang>_<country>_<varian>_<algorithm>
                createCollator(rLocale, aBuf.append(rLocale.Language).append(under).append(rLocale.Country).append(
                        under).append(rLocale.Variant).append(under).append(rSortAlgorithm).makeStringAndClear(),
                    rSortAlgorithm)) ||
            (l > 0 && c > 0 && a > 0 &&
             // load service with name <base>_<lang>_<country>_<algorithm>
             createCollator(rLocale, aBuf.append(rLocale.Language).append(under).append(rLocale.Country).append(
                     under).append(rSortAlgorithm).makeStringAndClear(), rSortAlgorithm)) ||
            (l > 0 && c > 0 && a > 0 && rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("zh")) &&
             (rLocale.Country.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("HK")) ||
              rLocale.Country.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MO"))) &&
             // if the country code is HK or MO, one more step to try TW.
             createCollator(rLocale, aBuf.append(rLocale.Language).append(under).append(tw).append(under).append(
                     rSortAlgorithm).makeStringAndClear(), rSortAlgorithm)) ||
            (l > 0 && a > 0 &&
             // load service with name <base>_<lang>_<algorithm>
             createCollator(rLocale, aBuf.append(rLocale.Language).append(under).append(rSortAlgorithm).makeStringAndClear(),
                 rSortAlgorithm)) ||
            // load service with name <base>_<algorithm>
            (a > 0 &&
             createCollator(rLocale, rSortAlgorithm, rSortAlgorithm)) ||
            // load default service with name <base>_Unicode
            createCollator(rLocale, unicode, rSortAlgorithm)) {
                return;
            } else {
                cachedItem = NULL;
                throw RuntimeException(); // could not load any service
            }
}

const sal_Char cCollator[] = "com.sun.star.i18n.Collator";

OUString SAL_CALL
CollatorImpl::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cCollator);
}

sal_Bool SAL_CALL
CollatorImpl::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName.equalsAscii(cCollator);
}

Sequence< OUString > SAL_CALL
CollatorImpl::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cCollator);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
