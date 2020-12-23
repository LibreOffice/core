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

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <sal/log.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <ordinalsuffix.hxx>

#include <unicode/rbnf.h>
#include <unicode/normlzr.h>
#include <memory>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace i18npool {


OrdinalSuffixService::OrdinalSuffixService()
{
}

OrdinalSuffixService::~OrdinalSuffixService()
{
}

namespace
{
    OUString mungeUnicodeStringToOUString(const icu::UnicodeString &rIn, UErrorCode &rCode)
    {
        // Apply NFKC normalization to get normal letters
        icu::UnicodeString normalized;
        icu::Normalizer::normalize(rIn, UNORM_NFKC, 0, normalized, rCode);
        // Convert the normalized UnicodeString to OUString
        OUString sRet = (U_SUCCESS(rCode))
            ? OUString(reinterpret_cast<const sal_Unicode *>(normalized.getBuffer()), normalized.length())
            : OUString();
        // replace any minus signs with hyphen-minus so that negative numbers
        // from the simple number formatter and heavy-duty pattern formatter
        // agree as to their negative number sign
        return sRet.replace(0x2212, '-');
    }
}

/*
 * For this method to properly return the ordinal suffix for other locales
 * than english ones, ICU 4.2+ has to be used.
 */
uno::Sequence< OUString > SAL_CALL OrdinalSuffixService::getOrdinalSuffix( sal_Int32 nNumber,
        const lang::Locale &rLocale )
{
    uno::Sequence< OUString > retValue;

    // Get the value from ICU
    UErrorCode nCode = U_ZERO_ERROR;
    const icu::Locale aIcuLocale( LanguageTagIcu::getIcuLocale( LanguageTag( rLocale)));

    icu::RuleBasedNumberFormat formatter(icu::URBNF_ORDINAL, aIcuLocale, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    std::unique_ptr<icu::NumberFormat> xNumberFormat(icu::NumberFormat::createInstance(aIcuLocale, nCode));
    if (!U_SUCCESS(nCode))
        return retValue;

    icu::UnicodeString sFormatWithNoOrdinal;
    icu::Formattable ftmNumber(static_cast<int32_t>(nNumber));
    icu::FieldPosition icuPosA;
    xNumberFormat->format(ftmNumber, sFormatWithNoOrdinal, icuPosA, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    OUString sValueWithNoOrdinal = mungeUnicodeStringToOUString(sFormatWithNoOrdinal, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    int32_t nRuleSets = formatter.getNumberOfRuleSetNames( );
    std::vector<OUString> retVec;
    retVec.reserve(nRuleSets);
    for (int32_t i = 0; i < nRuleSets; ++i)
    {
        icu::UnicodeString ruleSet = formatter.getRuleSetName(i);

        // format the string
        icu::UnicodeString sFormatWithOrdinal;
        icu::FieldPosition icuPosB;
        formatter.format(static_cast<int32_t>(nNumber), ruleSet, sFormatWithOrdinal, icuPosB, nCode);

        if (!U_SUCCESS(nCode))
            continue;

        OUString sValueWithOrdinal = mungeUnicodeStringToOUString(sFormatWithOrdinal, nCode);
        if (!U_SUCCESS(nCode))
            continue;

        // fdo#54486 lets make sure that the ordinal format and the non-ordinal
        // format match at the start, so that the expectation can be verified
        // that there is some trailing "ordinal suffix" which can be extracted
        bool bSimpleOrdinalSuffix = sValueWithOrdinal.startsWith(sValueWithNoOrdinal);

        SAL_WARN_IF(!bSimpleOrdinalSuffix, "i18npool", "ordinal " <<
            sValueWithOrdinal << " didn't start with expected " <<
            sValueWithNoOrdinal << " prefix");

        if (!bSimpleOrdinalSuffix)
            continue;

        // Remove the number to get the prefix
        sal_Int32 len = sValueWithNoOrdinal.getLength();
        retVec.push_back(sValueWithOrdinal.copy(len));
    }

    return comphelper::containerToSequence(retVec);
}

const OUStringLiteral cOrdinalSuffix = u"com.sun.star.i18n.OrdinalSuffix";

OUString SAL_CALL OrdinalSuffixService::getImplementationName()
{
    return cOrdinalSuffix;
}

sal_Bool SAL_CALL OrdinalSuffixService::supportsService( const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL OrdinalSuffixService::getSupportedServiceNames()
{
    return { cOrdinalSuffix };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_OrdinalSuffix_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::OrdinalSuffixService());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
