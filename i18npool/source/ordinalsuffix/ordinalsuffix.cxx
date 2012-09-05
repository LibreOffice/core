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

#include <boost/scoped_ptr.hpp>
#include <comphelper/processfactory.hxx>
#include <string.h>
#include "ordinalsuffix.hxx"

#include <unicode/rbnf.h>
#include <unicode/normlzr.h>

#define CSTR( ouStr ) rtl::OUStringToOString( ouStr, RTL_TEXTENCODING_UTF8 ).getStr( )

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {


OrdinalSuffix::OrdinalSuffix(
        const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) :
    _xServiceManager( rxMSF )
{
}

OrdinalSuffix::~OrdinalSuffix()
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
uno::Sequence< OUString > SAL_CALL OrdinalSuffix::getOrdinalSuffix( sal_Int32 nNumber,
        const lang::Locale &aLocale ) throw( RuntimeException )
{
    uno::Sequence< OUString > retValue;

    // Get the value from ICU
    UErrorCode nCode = U_ZERO_ERROR;
    const icu::Locale rIcuLocale(
            CSTR( aLocale.Language ),
            CSTR( aLocale.Country ),
            CSTR( aLocale.Variant ) );

    icu::RuleBasedNumberFormat formatter(icu::URBNF_ORDINAL, rIcuLocale, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    boost::scoped_ptr<NumberFormat> xNumberFormat(icu::NumberFormat::createInstance(rIcuLocale, nCode));
    if (!U_SUCCESS(nCode))
        return retValue;

    icu::UnicodeString sFormatWithNoOrdinal;
    xNumberFormat->format((int32_t)nNumber, sFormatWithNoOrdinal, NULL, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    OUString sValueWithNoOrdinal = mungeUnicodeStringToOUString(sFormatWithNoOrdinal, nCode);
    if (!U_SUCCESS(nCode))
        return retValue;

    int32_t nRuleSets = formatter.getNumberOfRuleSetNames( );
    for (int32_t i = 0; i < nRuleSets; ++i)
    {
        icu::UnicodeString ruleSet = formatter.getRuleSetName(i);

        // format the string
        icu::UnicodeString sFormatWithOrdinal;
        icu::FieldPosition icuPos;
        formatter.format( (int32_t)nNumber, ruleSet, sFormatWithOrdinal, icuPos, nCode );

        if (!U_SUCCESS(nCode))
            continue;

        OUString sValueWithOrdinal = mungeUnicodeStringToOUString(sFormatWithOrdinal, nCode);
        if (!U_SUCCESS(nCode))
            continue;

        // fdo#54486 lets make sure that the ordinal format and the non-ordinal
        // format match at the start, so that the expectation can be verified
        // that there is some trailing "ordinal suffix" which can be extracted
        bool bSimpleOrdinalSuffix = sValueWithOrdinal.match(sValueWithNoOrdinal);

        SAL_WARN_IF(!bSimpleOrdinalSuffix, "i18npool", "ordinal " <<
            sValueWithOrdinal << " didn't start with expected " <<
            sValueWithNoOrdinal << " prefix");

        if (!bSimpleOrdinalSuffix)
            continue;

        // Remove the number to get the prefix
        sal_Int32 len = sValueWithNoOrdinal.getLength();

        sal_Int32 newLength = retValue.getLength() + 1;
        retValue.realloc( newLength );
        retValue[ newLength - 1 ] = sValueWithOrdinal.copy( len );
    }

    return retValue;
}

const sal_Char cOrdinalSuffix[] = "com.sun.star.i18n.OrdinalSuffix";

OUString SAL_CALL OrdinalSuffix::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(cOrdinalSuffix);
}

sal_Bool SAL_CALL OrdinalSuffix::supportsService( const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cOrdinalSuffix);
}

Sequence< OUString > SAL_CALL OrdinalSuffix::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cOrdinalSuffix);
    return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
