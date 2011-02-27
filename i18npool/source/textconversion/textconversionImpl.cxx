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

#include <assert.h>
#include <textconversionImpl.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

TextConversionResult SAL_CALL
TextConversionImpl::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
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
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
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
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    getLocaleSpecificTextConversion(rLocale);

    sal_Int32 len = aText.getLength() - nStartPos;
    if (nLength > len)
        nLength = len > 0 ? len : 0;
    return xTC->getConversionWithOffset(aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions, offset);
}

sal_Bool SAL_CALL
TextConversionImpl::interactiveConversion( const Locale& rLocale, sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions )
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    getLocaleSpecificTextConversion(rLocale);

    return xTC->interactiveConversion(rLocale, nTextConversionType, nTextConversionOptions);
}

static inline sal_Bool operator != (const Locale& l1, const Locale& l2) {
    return l1.Language != l2.Language || l1.Country != l2.Country || l1.Variant != l2.Variant;
}

void SAL_CALL
TextConversionImpl::getLocaleSpecificTextConversion(const Locale& rLocale) throw( NoSupportException )
{
    if (xMSF.is() && rLocale != aLocale) {
        aLocale = rLocale;

        Reference < XInterface > xI;

        xI = xMSF->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.TextConversion_")) + aLocale.Language);

        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.TextConversion_")) + aLocale.Language +
                OUString(RTL_CONSTASCII_USTRINGPARAM("_")) + aLocale.Country);
        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.TextConversion_")) + aLocale.Language +
                OUString(RTL_CONSTASCII_USTRINGPARAM("_")) + aLocale.Country +
                OUString(RTL_CONSTASCII_USTRINGPARAM("_")) + aLocale.Variant);

        if (xI.is())
            xI->queryInterface( getCppuType((const Reference< XTextConversion>*)0) ) >>= xTC;
        else if (xTC.is())
            xTC.clear();
    }
    if (! xTC.is())
        throw NoSupportException(); // aLocale is not supported
}

const sal_Char cTextConversion[] = "com.sun.star.i18n.TextConversion";

OUString SAL_CALL
TextConversionImpl::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cTextConversion);
}

sal_Bool SAL_CALL
TextConversionImpl::supportsService(const OUString& rServiceName)
                throw( RuntimeException )
{
    return rServiceName.equalsAscii(cTextConversion);
}

Sequence< OUString > SAL_CALL
TextConversionImpl::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cTextConversion);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
