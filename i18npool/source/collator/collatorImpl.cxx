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

#include <collatorImpl.hxx>
#include <localedata.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/i18n/LocaleData.hpp>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

CollatorImpl::CollatorImpl( const Reference < XComponentContext >& rxContext ) : m_xContext(rxContext)
{
    mxLocaleData.set( LocaleData::create(rxContext) );
    cachedItem = nullptr;
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
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException, std::exception)
{
    if (cachedItem)
        return cachedItem->xC->compareSubstring(str1, off1, len1, str2, off2, len2);

    sal_Unicode *unistr1 = const_cast<sal_Unicode*>(str1.getStr()) + off1;
    sal_Unicode *unistr2 = const_cast<sal_Unicode*>(str2.getStr()) + off2;
    for (int i = 0; i < len1 && i < len2; i++)
        if (unistr1[i] != unistr2[i])
            return unistr1[i] < unistr2[i] ? -1 : 1;
    return len1 == len2 ? 0 : (len1 < len2 ? -1 : 1);
}

sal_Int32 SAL_CALL
CollatorImpl::compareString( const OUString& in_str1, const OUString& in_str2) throw(RuntimeException, std::exception)
{
    if (cachedItem)
        return cachedItem->xC->compareString(in_str1, in_str2);

    return CollatorImpl::compareSubstring(in_str1, 0, in_str1.getLength(), in_str2, 0, in_str2.getLength());
}


sal_Int32 SAL_CALL
CollatorImpl::loadDefaultCollator(const lang::Locale& rLocale, sal_Int32 collatorOptions) throw(RuntimeException, std::exception)
{
    const Sequence< Implementation > &imp = mxLocaleData->getCollatorImplementations(rLocale);
    for (sal_Int32 i = 0; i < imp.getLength(); i++)
        if (imp[i].isDefault)
            return loadCollatorAlgorithm(imp[i].unoID, rLocale, collatorOptions);

    throw RuntimeException(); // not default is defined
    //return 0;
}

sal_Int32 SAL_CALL
CollatorImpl::loadCollatorAlgorithm(const OUString& impl, const lang::Locale& rLocale, sal_Int32 collatorOptions)
    throw(RuntimeException, std::exception)
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
    const Sequence< sal_Int32 >& collatorOptions) throw(RuntimeException, std::exception)
{
    sal_Int32 options = 0;
    for (sal_Int32 i = 0; i < collatorOptions.getLength(); i++)
        options |= collatorOptions[i];
    loadCollatorAlgorithm(impl, rLocale, options);
}

Sequence< OUString > SAL_CALL
CollatorImpl::listCollatorAlgorithms( const lang::Locale& rLocale ) throw(RuntimeException, std::exception)
{
    nLocale = rLocale;
    const Sequence< Implementation > &imp = mxLocaleData->getCollatorImplementations(rLocale);
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
CollatorImpl::listCollatorOptions( const OUString& /*collatorAlgorithmName*/ ) throw(RuntimeException, std::exception)
{
    Sequence< OUString > option_str = mxLocaleData->getCollationOptions(nLocale);
    Sequence< sal_Int32 > option_int(option_str.getLength());

    for (sal_Int32 i = 0; i < option_str.getLength(); i++)
        option_int[i] =
            option_str[i] == "IGNORE_CASE" ?  CollatorOptions::CollatorOptions_IGNORE_CASE :
            option_str[i] == "IGNORE_KANA" ?  CollatorOptions::CollatorOptions_IGNORE_KANA :
            option_str[i] == "IGNORE_WIDTH" ?  CollatorOptions::CollatorOptions_IGNORE_WIDTH : 0;

    return option_int;
}

bool SAL_CALL
CollatorImpl::createCollator(const lang::Locale& rLocale, const OUString& serviceName, const OUString& rSortAlgorithm)
    throw(RuntimeException)
{
    for (size_t l = 0; l < lookupTable.size(); l++) {
        cachedItem = lookupTable[l];
        if (cachedItem->service.equals(serviceName)) {// cross locale sharing
            lookupTable.push_back(cachedItem = new lookupTableItem(rLocale, rSortAlgorithm, serviceName, cachedItem->xC));
            return true;
        }
    }
    Reference < XInterface > xI =
        m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.i18n.Collator_" + serviceName, m_xContext );

    if (xI.is()) {
        Reference < XCollator > xC;
        xC.set( xI, UNO_QUERY );
        if (xC.is()) {
            lookupTable.push_back(cachedItem = new lookupTableItem(rLocale, rSortAlgorithm, serviceName, xC));
            return true;
        }
    }
    return false;
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

    bool bLoaded = false;
    if (!rSortAlgorithm.isEmpty())
    {
        // Load service with name <base>_<lang>_<country>_<algorithm> or
        // <base>_<bcp47>_<algorithm> and fallbacks.
        bLoaded = createCollator( rLocale,
                LocaleDataImpl::getFirstLocaleServiceName( rLocale) + "_" + rSortAlgorithm, rSortAlgorithm);
        if (!bLoaded)
        {
            ::std::vector< OUString > aFallbacks( LocaleDataImpl::getFallbackLocaleServiceNames( rLocale));
            for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
            {
                bLoaded = createCollator( rLocale, *it + "_" + rSortAlgorithm, rSortAlgorithm);
                if (bLoaded)
                    break;
            }
            if (!bLoaded)
            {
                // load service with name <base>_<algorithm>
                bLoaded = createCollator( rLocale, rSortAlgorithm, rSortAlgorithm);
            }
        }
    }
    if (!bLoaded)
    {
        // load default service with name <base>_Unicode
        bLoaded = createCollator( rLocale, "Unicode", rSortAlgorithm);
        if (!bLoaded)
        {
            cachedItem = nullptr;
            throw RuntimeException();   // could not load any service
        }
    }
}

OUString SAL_CALL CollatorImpl::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString("com.sun.star.i18n.Collator");
}

sal_Bool SAL_CALL CollatorImpl::supportsService(const OUString& rServiceName)
                throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
CollatorImpl::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    Sequence< OUString > aRet { "com.sun.star.i18n.Collator" };
    return aRet;
}

} } } }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_Collator_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new css::i18n::CollatorImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
