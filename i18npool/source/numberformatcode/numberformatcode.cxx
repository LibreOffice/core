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

#include <numberformatcode.hxx>
#include <com/sun/star/i18n/KNumberFormatUsage.hpp>
#include <com/sun/star/i18n/KNumberFormatType.hpp>
#include <com/sun/star/i18n/LocaleData2.hpp>
#include <cppuhelper/supportsservice.hxx>

NumberFormatCodeMapper::NumberFormatCodeMapper(
            const css::uno::Reference < css::uno::XComponentContext >& rxContext )
{
        m_xLocaleData.set( css::i18n::LocaleData2::create( rxContext ) );
}


NumberFormatCodeMapper::~NumberFormatCodeMapper()
{
}


css::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getDefault( sal_Int16 formatType, sal_Int16 formatUsage, const css::lang::Locale& rLocale )
{

    OUString elementType = mapElementTypeShortToString(formatType);
    OUString elementUsage = mapElementUsageShortToString(formatUsage);

    osl::MutexGuard g(maMutex);
    const css::uno::Sequence< css::i18n::FormatElement > &aFormatSeq = getFormats( rLocale );

    for (sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if (aFormatSeq[i].isDefault && aFormatSeq[i].formatType == elementType &&
            aFormatSeq[i].formatUsage == elementUsage) {
            css::i18n::NumberFormatCode anumberFormatCode(formatType,
                                                                    formatUsage,
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    true);
            return anumberFormatCode;
        }
    }
    css::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;
}


css::i18n::NumberFormatCode SAL_CALL
NumberFormatCodeMapper::getFormatCode( sal_Int16 formatIndex, const css::lang::Locale& rLocale )
{
    osl::MutexGuard g(maMutex);
    const css::uno::Sequence< css::i18n::FormatElement > &aFormatSeq = getFormats( rLocale );

    for (sal_Int32 i = 0; i < aFormatSeq.getLength(); i++) {
        if (aFormatSeq[i].formatIndex == formatIndex) {
            css::i18n::NumberFormatCode anumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                                    mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                                    aFormatSeq[i].formatCode,
                                                                    aFormatSeq[i].formatName,
                                                                    aFormatSeq[i].formatKey,
                                                                    aFormatSeq[i].formatIndex,
                                                                    aFormatSeq[i].isDefault);
            return anumberFormatCode;
        }
    }
    css::i18n::NumberFormatCode defaultNumberFormatCode;
    return defaultNumberFormatCode;
}


css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCode( sal_Int16 formatUsage, const css::lang::Locale& rLocale )
{
    osl::MutexGuard g(maMutex);
    const css::uno::Sequence< css::i18n::FormatElement > &aFormatSeq = getFormats( rLocale );

    sal_Int32 i, count;
    count = 0;
    for (i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if ( elementUsage == formatUsage )
            count++;
    }

    css::uno::Sequence<css::i18n::NumberFormatCode> seq(count);
    sal_Int32 j = 0;
    for (i = 0; i < aFormatSeq.getLength(); i++) {
        sal_Int16 elementUsage = mapElementUsageStringToShort(aFormatSeq[i].formatUsage);
        if ( elementUsage == formatUsage ) {
            seq[j] = css::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                            formatUsage,
                                                            aFormatSeq[i].formatCode,
                                                            aFormatSeq[i].formatName,
                                                            aFormatSeq[i].formatKey,
                                                            aFormatSeq[i].formatIndex,
                                                            aFormatSeq[i].isDefault);
            j++;
        }
    }
    return seq;
}


css::uno::Sequence< css::i18n::NumberFormatCode > SAL_CALL
NumberFormatCodeMapper::getAllFormatCodes( const css::lang::Locale& rLocale )
{
    osl::MutexGuard g(maMutex);
    const css::uno::Sequence< css::i18n::FormatElement > &aFormatSeq = getFormats( rLocale );

    css::uno::Sequence<css::i18n::NumberFormatCode> seq(aFormatSeq.getLength());
    for (sal_Int32 i = 0; i < aFormatSeq.getLength(); i++)
    {
        seq[i] = css::i18n::NumberFormatCode(mapElementTypeStringToShort(aFormatSeq[i].formatType),
                                                        mapElementUsageStringToShort(aFormatSeq[i].formatUsage),
                                                        aFormatSeq[i].formatCode,
                                                        aFormatSeq[i].formatName,
                                                        aFormatSeq[i].formatKey,
                                                        aFormatSeq[i].formatIndex,
                                                        aFormatSeq[i].isDefault);
    }
    return seq;
}


// --- private implementation -----------------------------------------

const css::uno::Sequence< css::i18n::FormatElement >& NumberFormatCodeMapper::getFormats( const css::lang::Locale& rLocale )
{
    /* Find the FormatElement Sequence in the cache */
    for (const FormatElementCacheItem& item : m_aFormatElementCache)
    {
        if ( item.first == rLocale )
            return item.second;
    }

    /* Not found; Get the FormatElement Sequence for the given Locale */
    css::uno::Sequence< css::i18n::FormatElement > aFormatElementSequence;
    if ( m_xLocaleData.is() )
        aFormatElementSequence = m_xLocaleData->getAllFormats( rLocale );

    /* Add the FormatElement Sequence to the cache */
    const int FORMATELEMENTCACHE_SIZE = 3;
    if ( m_aFormatElementCache.size() > FORMATELEMENTCACHE_SIZE )
        m_aFormatElementCache.pop_front();
    m_aFormatElementCache.emplace_back( rLocale, aFormatElementSequence );
    return m_aFormatElementCache.back().second;
}


OUString
NumberFormatCodeMapper::mapElementTypeShortToString(sal_Int16 formatType)
{

    switch ( formatType )
    {
        case css::i18n::KNumberFormatType::SHORT :
            return "short";
        case css::i18n::KNumberFormatType::MEDIUM :
            return "medium";
        case css::i18n::KNumberFormatType::LONG :
            return "long";
    }
    return OUString();
}

sal_Int16
NumberFormatCodeMapper::mapElementTypeStringToShort(const OUString& formatType)
{
    if ( formatType == "short" )
        return css::i18n::KNumberFormatType::SHORT;
    if ( formatType == "medium" )
        return css::i18n::KNumberFormatType::MEDIUM;
    if ( formatType == "long" )
        return css::i18n::KNumberFormatType::LONG;

    return css::i18n::KNumberFormatType::SHORT;
}

OUString
NumberFormatCodeMapper::mapElementUsageShortToString(sal_Int16 formatUsage)
{
    switch ( formatUsage )
    {
        case css::i18n::KNumberFormatUsage::DATE :
            return "DATE";
        case css::i18n::KNumberFormatUsage::TIME :
            return "TIME";
        case css::i18n::KNumberFormatUsage::DATE_TIME :
            return "DATE_TIME";
        case css::i18n::KNumberFormatUsage::FIXED_NUMBER :
            return "FIXED_NUMBER";
        case css::i18n::KNumberFormatUsage::FRACTION_NUMBER :
            return "FRACTION_NUMBER";
        case css::i18n::KNumberFormatUsage::PERCENT_NUMBER :
            return "PERCENT_NUMBER";
        case css::i18n::KNumberFormatUsage::CURRENCY :
            return "CURRENCY";
        case css::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER :
            return "SCIENTIFIC_NUMBER";
    }
    return OUString();
}


sal_Int16
NumberFormatCodeMapper::mapElementUsageStringToShort(const OUString& formatUsage)
{
    if ( formatUsage == "DATE" )
        return css::i18n::KNumberFormatUsage::DATE;
    if ( formatUsage == "TIME" )
        return css::i18n::KNumberFormatUsage::TIME;
    if ( formatUsage == "DATE_TIME" )
        return css::i18n::KNumberFormatUsage::DATE_TIME;
    if ( formatUsage == "FIXED_NUMBER" )
        return css::i18n::KNumberFormatUsage::FIXED_NUMBER;
    if ( formatUsage == "FRACTION_NUMBER" )
        return css::i18n::KNumberFormatUsage::FRACTION_NUMBER;
    if ( formatUsage == "PERCENT_NUMBER" )
        return  css::i18n::KNumberFormatUsage::PERCENT_NUMBER;
    if ( formatUsage == "CURRENCY" )
        return css::i18n::KNumberFormatUsage::CURRENCY;
    if ( formatUsage == "SCIENTIFIC_NUMBER" )
        return css::i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER;

    return 0;
}


OUString SAL_CALL
NumberFormatCodeMapper::getImplementationName()
{
    return "com.sun.star.i18n.NumberFormatCodeMapper";
}

sal_Bool SAL_CALL NumberFormatCodeMapper::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL
NumberFormatCodeMapper::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > aRet { "com.sun.star.i18n.NumberFormatMapper" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_NumberFormatCodeMapper_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new NumberFormatCodeMapper(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
