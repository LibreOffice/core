/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "dudenimp.hxx"

#include <boost/property_tree/ptree.hpp>

#include <com/sun/star/text/TextMarkupType.hpp>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <svtools/strings.hrc>
#include <tools/json_writer.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/syslocaleoptions.hxx>

using namespace css::linguistic2;
using namespace lingucomponent::remote;

namespace
{
using DudenCfg = officecfg::Office::Linguistic::GrammarChecking::Duden;

constexpr OUString IMPLEMENTATION_NAME = u"cpo.lingu.Duden"_ustr;

OUString getBaseURL() { return DudenCfg::BaseURL::get().value_or(OUString()); }

// The language the server's messages are asked for: the language of the
// user interface, which is the reader's.
OUString getMessageLanguage()
{
    if (comphelper::COKit::isActive())
        return comphelper::COKit::getLanguageTag().getBcp47(false);
    return SvtSysLocaleOptions().GetRealUILanguageTag().getBcp47(false);
}

// A generic description when the server has no message for an error code.
TranslateId describeType(std::string_view rType)
{
    if (rType == "gram")
        return STR_DESCRIPTION_GRAMMAR_ERROR;
    if (rType == "style")
        return STR_DESCRIPTION_STYLE_ERROR;
    // "orth" and "term"
    return STR_DESCRIPTION_SPELLING_ERROR;
}

Color lineColorForType(std::string_view rType)
{
    if (rType == "orth" || rType == "term")
        return COL_REMOTE_SPELLING;
    if (rType == "style")
        return COL_REMOTE_STYLE;
    return COL_REMOTE_GRAMMAR;
}

void putNames(tools::JsonWriter& rJson, std::string_view rName,
              const cpo::uno::Sequence<OUString>& rNames)
{
    auto aArray = rJson.startArray(rName);
    for (const OUString& rItem : rNames)
    {
        if (!rItem.isEmpty())
            rJson.putSimpleValue(rItem);
    }
}
}

DudenChecker::DudenChecker()
    : CheckerBase(u"/org.openoffice.Office.Linguistic/GrammarChecking/Duden"_ustr)
{
}

bool DudenChecker::isEnabled()
{
    return DudenCfg::IsEnabled::get() && !DudenCfg::BaseURL::get().value_or(OUString()).isEmpty();
}

std::vector<OUString> DudenChecker::configuredLocales()
{
    cpo::uno::Sequence<OUString> aLocales;
    SvtLinguConfig().GetLocaleListFor(u"GrammarCheckers"_ustr, IMPLEMENTATION_NAME, aLocales);
    return std::vector<OUString>(aLocales.begin(), aLocales.end());
}

// The request skeleton: the server is reached with an API token in a header
// of its own.
Request DudenChecker::makeRequest(std::u16string_view rPath)
{
    Request aRequest;
    aRequest.aURL = getBaseURL() + rPath;
    aRequest.bVerifySSL = DudenCfg::SSLCertVerify::get();
    aRequest.aHeaders.push_back("Cache-Control: no-cache"_ostr);
    aRequest.aHeaders.push_back("Accept: application/json"_ostr);

    const OString aApiKey
        = OUStringToOString(DudenCfg::ApiKey::get().value_or(OUString()), RTL_TEXTENCODING_UTF8);
    if (!aApiKey.isEmpty())
        aRequest.aHeaders.push_back("access_token: " + aApiKey);
    return aRequest;
}

std::optional<Request> DudenChecker::buildRequest(const OUString& rText,
                                                  const css::lang::Locale& rLocale)
{
    if (getBaseURL().isEmpty())
        return std::nullopt;

    tools::JsonWriter aJson;
    aJson.put("text", rText);
    aJson.put("text-language", LanguageTag::convertToBcp47(rLocale, false));
    aJson.put("hyphenation", false);
    aJson.put("correction-proposals", true);
    aJson.put("spellchecking-level", DudenCfg::SpellcheckingLevel::get());
    const OUString aStandard = DudenCfg::OrthographyStandard::get();
    if (!aStandard.isEmpty())
        aJson.put("orthography-standard", aStandard);
    aJson.put("message-level", DudenCfg::MessageLevel::get());
    aJson.put("message-language", getMessageLanguage());
    aJson.put("style-foreign", DudenCfg::Style::Foreign::get());
    aJson.put("style-old", DudenCfg::Style::Old::get());
    aJson.put("style-regional", DudenCfg::Style::Regional::get());
    aJson.put("style-colloquial", DudenCfg::Style::Colloquial::get());
    aJson.put("style-fillers", DudenCfg::Style::Fillers::get());
    aJson.put("style-offensive", DudenCfg::Style::Offensive::get());
    aJson.put("style-gender-inclusive", DudenCfg::Style::GenderInclusive::get());
    aJson.put("style-complexity", DudenCfg::Style::Complexity::get());
    putNames(aJson, "property-sets", DudenCfg::PropertySets::get());
    putNames(aJson, "dictionaries", DudenCfg::Dictionaries::get());

    Request aRequest = makeRequest(u"/check");
    aRequest.aContentType = "application/json"_ostr;
    aRequest.aBody = aJson.finishAndGetAsOString();
    return aRequest;
}

std::vector<SingleProofreadingError> DudenChecker::parseResponse(const std::string& rBody,
                                                                 const OUString& /*rText*/,
                                                                 const css::lang::Locale& /*rLocale*/)
{
    std::vector<SingleProofreadingError> aErrors;
    const std::optional<boost::property_tree::ptree> oRoot = parseJson(rBody);
    if (!oRoot)
        return aErrors;
    const boost::optional<const boost::property_tree::ptree&> oPositions
        = oRoot->get_child_optional("check-positions");
    if (!oPositions)
        return aErrors;
    const boost::optional<const boost::property_tree::ptree&> oMessages
        = oRoot->get_child_optional("messages");

    // Worded in the language the server's messages were asked for, so the
    // comments on a paragraph all read in one language.
    const std::locale aLocale = Translate::Create("svt", LanguageTag(getMessageLanguage()));
    for (const std::pair<const std::string, boost::property_tree::ptree>& rItem : *oPositions)
    {
        const boost::property_tree::ptree& rPosition = rItem.second;
        SingleProofreadingError aError;
        aError.nErrorStart = rPosition.get<sal_Int32>("offset", 0);
        aError.nErrorLength = rPosition.get<sal_Int32>("length", 0);
        aError.nErrorType = css::text::TextMarkupType::PROOFREADING;

        const std::string aType = rPosition.get<std::string>("type", {});
        const std::string aCode = rPosition.get<std::string>("errorcode", {});
        aError.aRuleIdentifier = OStringToOUString(aCode, RTL_TEXTENCODING_UTF8);

        // The server's own explanation of the error code when it sent one,
        // else a word for the kind of error.
        std::string aMessage;
        if (oMessages && !aCode.empty())
            aMessage = oMessages->get<std::string>(aCode, {});
        aError.aShortComment = aMessage.empty()
                                   ? Translate::get(describeType(aType), aLocale)
                                   : OStringToOUString(aMessage, RTL_TEXTENCODING_UTF8);
        aError.aFullComment = aError.aShortComment;
        setLineColor(aError, lineColorForType(aType));

        if (const boost::optional<const boost::property_tree::ptree&> oProposals
            = rPosition.get_child_optional("proposals"))
            takeSuggestions(aError, *oProposals, nullptr);

        aErrors.push_back(aError);
    }
    return aErrors;
}

// The names in a list the server publishes: the objects' "name" members, or
// the plain strings when there is no member name to look for.
std::vector<OUString> DudenChecker::fetchNames(std::u16string_view rPath, const char* pNameKey)
{
    std::vector<OUString> aNames;
    const Response aResponse = get(makeRequest(rPath));
    if (aResponse.eCode != CURLE_OK)
        return aNames;
    const std::optional<boost::property_tree::ptree> oRoot = parseJson(aResponse.aBody);
    if (!oRoot)
        return aNames;
    for (const std::pair<const std::string, boost::property_tree::ptree>& rItem : *oRoot)
    {
        const std::string aName
            = pNameKey ? rItem.second.get<std::string>(pNameKey, {}) : rItem.second.data();
        if (!aName.empty())
            aNames.push_back(OStringToOUString(aName, RTL_TEXTENCODING_UTF8));
    }
    return aNames;
}

cpo::uno::Sequence<css::beans::PropertyValue> DudenChecker::probe()
{
    bool bReachable = false;
    if (!getBaseURL().isEmpty())
        bReachable = get(makeRequest(u"/version")).eCode == CURLE_OK;
    std::vector<OUString> aPropertySets;
    std::vector<OUString> aDictionaries;
    if (bReachable)
    {
        aPropertySets = fetchNames(u"/propset", "name");
        aDictionaries = fetchNames(u"/dictionaries", nullptr);
    }
    return { comphelper::makePropertyValue(u"IsReachable"_ustr, bReachable),
             comphelper::makePropertyValue(u"PropertySets"_ustr,
                                           comphelper::containerToSequence(aPropertySets)),
             comphelper::makePropertyValue(u"Dictionaries"_ustr,
                                           comphelper::containerToSequence(aDictionaries)) };
}

OUString DudenChecker::getServiceDisplayName(const css::lang::Locale& rLocale)
{
    return Translate::get(STR_DESCRIPTION_DUDEN, Translate::Create("svt", LanguageTag(rLocale)));
}

OUString DudenChecker::getImplementationName() { return IMPLEMENTATION_NAME; }

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
lingucomponent_Duden_get_implementation(cpo::uno::XComponentContext*,
                                        cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new DudenChecker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
