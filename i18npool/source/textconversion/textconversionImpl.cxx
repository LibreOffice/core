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

#include <assert.h>
#include <textconversionImpl.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace rtl;

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
            OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language);

        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language +
                OUString::createFromAscii("_") + aLocale.Country);
        if ( ! xI.is() )
            xI = xMSF->createInstance(
                OUString::createFromAscii("com.sun.star.i18n.TextConversion_") + aLocale.Language +
                OUString::createFromAscii("_") + aLocale.Country +
                OUString::createFromAscii("_") + aLocale.Variant);

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
