/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include "languagetoolimp.hxx"

#include <boost/property_tree/ptree.hpp>

#include <com/sun/star/text/TextMarkupType.hpp>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <svtools/strings.hrc>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>

using namespace css::linguistic2;
using namespace lingucomponent::remote;

namespace
{
using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;

constexpr OUString IMPLEMENTATION_NAME = u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr;

OUString getBaseURL() { return LanguageToolCfg::BaseURL::get().value_or(OUString()); }

Color lineColorForCategory(std::string_view rCategoryId)
{
    if (rCategoryId == "TYPOS")
        return COL_REMOTE_SPELLING;
    if (rCategoryId == "STYLE")
        return COL_REMOTE_STYLE;
    // GRAMMAR, TYPOGRAPHY and the rest.
    return COL_REMOTE_GRAMMAR;
}
}

LanguageToolGrammarChecker::LanguageToolGrammarChecker()
    : CheckerBase(u"/org.openoffice.Office.Linguistic/GrammarChecking/LanguageTool"_ustr)
{
}

bool LanguageToolGrammarChecker::isEnabled() { return LanguageToolCfg::IsEnabled::get(); }

std::vector<OUString> LanguageToolGrammarChecker::configuredLocales()
{
    cpo::uno::Sequence<OUString> aLocales;
    SvtLinguConfig().GetLocaleListFor(u"GrammarCheckers"_ustr, IMPLEMENTATION_NAME, aLocales);
    return std::vector<OUString>(aLocales.begin(), aLocales.end());
}

std::optional<Request> LanguageToolGrammarChecker::buildRequest(const OUString& rText,
                                                                const css::lang::Locale& rLocale)
{
    const OUString aBaseURL = getBaseURL();
    if (aBaseURL.isEmpty())
        return std::nullopt;

    Request aRequest;
    aRequest.aURL = aBaseURL + "/check";
    aRequest.aContentType = "application/x-www-form-urlencoded"_ostr;
    aRequest.bVerifySSL = LanguageToolCfg::SSLCertVerify::get();
    aRequest.aBody = "text=" + percentEncode(rText) + "&language="
                     + LanguageTag::convertToBcp47(rLocale, false).toUtf8();

    // The premium API takes the account in the form, beside the text.
    const OUString aUsername = LanguageToolCfg::Username::get().value_or(OUString());
    const OUString aApiKey = LanguageToolCfg::ApiKey::get().value_or(OUString());
    if (!aUsername.isEmpty() && !aApiKey.isEmpty())
        aRequest.aBody += "&username=" + percentEncode(aUsername) + "&apiKey=" + percentEncode(aApiKey);
    return aRequest;
}

std::vector<SingleProofreadingError>
LanguageToolGrammarChecker::parseResponse(const std::string& rBody, const OUString& /*rText*/,
                                          const css::lang::Locale& /*rLocale*/)
{
    std::vector<SingleProofreadingError> aErrors;
    const std::optional<boost::property_tree::ptree> oRoot = parseJson(rBody);
    if (!oRoot)
        return aErrors;
    const boost::optional<const boost::property_tree::ptree&> oMatches
        = oRoot->get_child_optional("matches");
    if (!oMatches)
        return aErrors;

    for (const std::pair<const std::string, boost::property_tree::ptree>& rItem : *oMatches)
    {
        const boost::property_tree::ptree& rMatch = rItem.second;
        SingleProofreadingError aError;
        aError.nErrorStart = rMatch.get<sal_Int32>("offset", 0);
        aError.nErrorLength = rMatch.get<sal_Int32>("length", 0);
        aError.nErrorType = css::text::TextMarkupType::PROOFREADING;
        aError.aShortComment
            = OStringToOUString(rMatch.get<std::string>("message", {}), RTL_TEXTENCODING_UTF8);
        aError.aFullComment = OStringToOUString(rMatch.get<std::string>("shortMessage", {}),
                                                RTL_TEXTENCODING_UTF8);

        std::string aCategoryId;
        if (const boost::optional<const boost::property_tree::ptree&> oRule
            = rMatch.get_child_optional("rule"))
        {
            aError.aRuleIdentifier
                = OStringToOUString(oRule->get<std::string>("id", {}), RTL_TEXTENCODING_UTF8);
            if (const boost::optional<const boost::property_tree::ptree&> oCategory
                = oRule->get_child_optional("category"))
                aCategoryId = oCategory->get<std::string>("id", {});
        }
        setLineColor(aError, lineColorForCategory(aCategoryId));

        if (const boost::optional<const boost::property_tree::ptree&> oReplacements
            = rMatch.get_child_optional("replacements"))
            takeSuggestions(aError, *oReplacements, "value");

        aErrors.push_back(aError);
    }
    return aErrors;
}

cpo::uno::Sequence<css::beans::PropertyValue> LanguageToolGrammarChecker::probe()
{
    Request aRequest;
    aRequest.aURL = getBaseURL() + "/languages";
    aRequest.bVerifySSL = LanguageToolCfg::SSLCertVerify::get();
    const bool bReachable = !getBaseURL().isEmpty() && get(aRequest).eCode == CURLE_OK;
    return { comphelper::makePropertyValue(u"IsReachable"_ustr, bReachable) };
}

OUString LanguageToolGrammarChecker::getServiceDisplayName(const css::lang::Locale& rLocale)
{
    return Translate::get(STR_DESCRIPTION_LANGUAGETOOL,
                          Translate::Create("svt", LanguageTag(rLocale)));
}

OUString LanguageToolGrammarChecker::getImplementationName() { return IMPLEMENTATION_NAME; }

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
lingucomponent_LanguageToolGrammarChecker_get_implementation(
    cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new LanguageToolGrammarChecker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
