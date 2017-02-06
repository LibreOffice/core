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

#include <assert.h>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <textconversionImpl.hxx>
#include <localedata.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

TextConversionResult SAL_CALL
TextConversionImpl::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
{
    getLocaleSpecificTextConversion(rLocale);

    sal_Int32 len = aText.getLength() - nStartPos;
    if (nLength > len)
        nLength = len > 0 ? len : 0;
    return xTC->getConversions(aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions);
}

OUString SAL_CALL
TextConversionImpl::getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
{
    getLocaleSpecificTextConversion(rLocale);

    sal_Int32 len = aText.getLength() - nStartPos;
    if (nLength > len)
        nLength = len > 0 ? len : 0;
    return xTC->getConversion(aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions);
}

OUString SAL_CALL
TextConversionImpl::getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions, Sequence< sal_Int32>& offset)
{
    getLocaleSpecificTextConversion(rLocale);

    sal_Int32 len = aText.getLength() - nStartPos;
    if (nLength > len)
        nLength = len > 0 ? len : 0;
    return xTC->getConversionWithOffset(aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions, offset);
}

sal_Bool SAL_CALL
TextConversionImpl::interactiveConversion( const Locale& rLocale, sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
{
    getLocaleSpecificTextConversion(rLocale);

    return xTC->interactiveConversion(rLocale, nTextConversionType, nTextConversionOptions);
}

void SAL_CALL
TextConversionImpl::getLocaleSpecificTextConversion(const Locale& rLocale)
{
    if (rLocale != aLocale) {
        aLocale = rLocale;

        OUString aPrefix("com.sun.star.i18n.TextConversion_");
        Reference < XInterface > xI;

        xI = m_xContext->getServiceManager()->createInstanceWithContext(
                aPrefix + LocaleDataImpl::getFirstLocaleServiceName( aLocale), m_xContext);
        if (!xI.is())
        {
            ::std::vector< OUString > aFallbacks( LocaleDataImpl::getFallbackLocaleServiceNames( aLocale));
            for (::std::vector< OUString >::const_iterator it( aFallbacks.begin()); it != aFallbacks.end(); ++it)
            {
                xI = m_xContext->getServiceManager()->createInstanceWithContext( aPrefix + *it, m_xContext);
                if (xI.is())
                    break;
            }
        }
        if (xI.is())
            xTC.set( xI, UNO_QUERY );
        else if (xTC.is())
            xTC.clear();
    }
    if (! xTC.is())
        throw NoSupportException(); // aLocale is not supported
}

OUString SAL_CALL
TextConversionImpl::getImplementationName()
{
    return OUString("com.sun.star.i18n.TextConversion");
}

sal_Bool SAL_CALL
TextConversionImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
TextConversionImpl::getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.i18n.TextConversion" };
    return aRet;
}

} } } }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_TextConversion_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new css::i18n::TextConversionImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
