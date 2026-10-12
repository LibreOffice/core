/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "grammalecteimp.hxx"

#include <boost/property_tree/ptree.hpp>

#include <com/sun/star/text/TextMarkupType.hpp>
#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <svtools/strings.hrc>
#include <unotools/lingucfg.hxx>
#include <unotools/resmgr.hxx>

using namespace css::linguistic2;
using namespace lingucomponent::remote;

namespace
{
using GrammalecteCfg = officecfg::Office::Linguistic::GrammarChecking::Grammalecte;

constexpr OUString IMPLEMENTATION_NAME = u"cpo.lingu.Grammalecte"_ustr;

OUString getBaseURL() { return GrammalecteCfg::BaseURL::get().value_or(OUString()); }

// The checker's options, each a rule family the server can be told to run
// or to skip. The ids are the server's.
struct Option
{
    const char* pId;
    bool (*pGet)();
};

using Options = GrammalecteCfg::Options;

constexpr Option aOptions[] = {
    { "typo", [] { return Options::typo::get(); } },
    { "apos", [] { return Options::apos::get(); } },
    { "eepi", [] { return Options::eepi::get(); } },
    { "esp", [] { return Options::esp::get(); } },
    { "tab", [] { return Options::tab::get(); } },
    { "nbsp", [] { return Options::nbsp::get(); } },
    { "unit", [] { return Options::unit::get(); } },
    { "tu", [] { return Options::tu::get(); } },
    { "maj", [] { return Options::maj::get(); } },
    { "minis", [] { return Options::minis::get(); } },
    { "num", [] { return Options::num::get(); } },
    { "nf", [] { return Options::nf::get(); } },
    { "virg", [] { return Options::virg::get(); } },
    { "poncfin", [] { return Options::poncfin::get(); } },
    { "ocr", [] { return Options::ocr::get(); } },
    { "chim", [] { return Options::chim::get(); } },
    { "liga", [] { return Options::liga::get(); } },
    { "mapos", [] { return Options::mapos::get(); } },
    { "conf", [] { return Options::conf::get(); } },
    { "loc", [] { return Options::loc::get(); } },
    { "gn", [] { return Options::gn::get(); } },
    { "infi", [] { return Options::infi::get(); } },
    { "conj", [] { return Options::conj::get(); } },
    { "ppas", [] { return Options::ppas::get(); } },
    { "imp", [] { return Options::imp::get(); } },
    { "inte", [] { return Options::inte::get(); } },
    { "vmode", [] { return Options::vmode::get(); } },
    { "bs", [] { return Options::bs::get(); } },
    { "pleo", [] { return Options::pleo::get(); } },
    { "eleu", [] { return Options::eleu::get(); } },
    { "neg", [] { return Options::neg::get(); } },
    { "redon1", [] { return Options::redon1::get(); } },
    { "redon2", [] { return Options::redon2::get(); } },
    { "date", [] { return Options::date::get(); } },
    { "mc", [] { return Options::mc::get(); } },
};

// The options as the server takes them: a JSON object of booleans. The
// text is plain, which the three format switches say.
OString buildOptions()
{
    OStringBuffer aJson("{");
    for (const Option& rOption : aOptions)
        aJson.append(OString::Concat("\"") + rOption.pId + (rOption.pGet() ? "\":true," : "\":false,"));
    aJson.append("\"html\":false,\"latex\":false,\"md\":false}");
    return aJson.makeStringAndClear();
}

// Which rule families underline in which colour: the typographic and the
// stylistic ones in blue, the grammatical ones in orange, as the other
// remote checkers do for their categories.
Color lineColorForType(std::string_view rType)
{
    static constexpr std::string_view aBlue[]
        = { "typo", "apos", "eepi", "esp",  "tab",    "nbsp",   "unit", "tu",   "maj",  "minis",
            "num",  "nf",   "virg", "poncfin", "ocr", "chim",   "liga", "mapos", "bs",  "pleo",
            "eleu", "neg",  "redon1", "redon2", "date", "mc" };
    for (const std::string_view& rBlue : aBlue)
    {
        if (rType == rBlue)
            return COL_REMOTE_STYLE;
    }
    return COL_REMOTE_GRAMMAR;
}
}

GrammalecteChecker::GrammalecteChecker()
    : CheckerBase(u"/org.openoffice.Office.Linguistic/GrammarChecking/Grammalecte"_ustr)
{
}

bool GrammalecteChecker::isEnabled() { return GrammalecteCfg::IsEnabled::get(); }

std::vector<OUString> GrammalecteChecker::configuredLocales()
{
    cpo::uno::Sequence<OUString> aLocales;
    SvtLinguConfig().GetLocaleListFor(u"GrammarCheckers"_ustr, IMPLEMENTATION_NAME, aLocales);
    return std::vector<OUString>(aLocales.begin(), aLocales.end());
}

std::optional<Request> GrammalecteChecker::buildRequest(const OUString& rText,
                                                        const css::lang::Locale& /*rLocale*/)
{
    const OUString aBaseURL = getBaseURL();
    if (aBaseURL.isEmpty())
        return std::nullopt;

    Request aRequest;
    aRequest.aURL = aBaseURL + "/gc_text/fr";
    aRequest.aContentType = "application/x-www-form-urlencoded"_ostr;
    aRequest.bVerifySSL = GrammalecteCfg::SSLCertVerify::get();
    // The text goes as it is: the server splits it into lines itself, and
    // the answer is read back against the same text.
    aRequest.aBody = "text=" + percentEncode(rText) + "&options="
                     + percentEncode(OStringToOUString(buildOptions(), RTL_TEXTENCODING_ASCII_US));
    return aRequest;
}

std::vector<SingleProofreadingError>
GrammalecteChecker::parseResponse(const std::string& rBody, const OUString& rText,
                                  const css::lang::Locale& /*rLocale*/)
{
    std::vector<SingleProofreadingError> aErrors;
    const std::optional<boost::property_tree::ptree> oRoot = parseJson(rBody);
    if (!oRoot)
        return aErrors;

    const std::string aServerError = oRoot->get<std::string>("error", {});
    if (!aServerError.empty())
    {
        SAL_WARN("lingucomponent.remote", "Grammalecte: " << aServerError);
        return aErrors;
    }
    const boost::optional<const boost::property_tree::ptree&> oData
        = oRoot->get_child_optional("data");
    if (!oData)
        return aErrors;

    // The server counts in code points and from the start of each line,
    // the document in UTF-16 units and from the start of the paragraph.
    // One table maps a code point index to the unit index it starts at,
    // another says at which code point each line starts.
    std::vector<sal_Int32> aUnitAt;
    std::vector<sal_Int32> aLineStart{ 0 };
    for (sal_Int32 nIndex = 0; nIndex < rText.getLength();)
    {
        aUnitAt.push_back(nIndex);
        const sal_uInt32 nCodePoint = rText.iterateCodePoints(&nIndex);
        if (nCodePoint == '\n')
            aLineStart.push_back(static_cast<sal_Int32>(aUnitAt.size()));
    }
    aUnitAt.push_back(rText.getLength());
    const sal_Int32 nCodePoints = static_cast<sal_Int32>(aUnitAt.size()) - 1;

    for (const std::pair<const std::string, boost::property_tree::ptree>& rEntry : *oData)
    {
        const boost::property_tree::ptree& rParagraph = rEntry.second;
        const sal_Int32 nLine = rParagraph.get<sal_Int32>("iParagraph", 0);
        if (nLine < 1 || nLine > static_cast<sal_Int32>(aLineStart.size()))
            continue;
        const sal_Int32 nOffset = aLineStart[nLine - 1];

        // The spelling errors are left to the spell checker, which has the
        // suggestions this answer does not carry.
        const boost::optional<const boost::property_tree::ptree&> oGrammar
            = rParagraph.get_child_optional("lGrammarErrors");
        if (!oGrammar)
            continue;
        for (const std::pair<const std::string, boost::property_tree::ptree>& rItem : *oGrammar)
        {
            const boost::property_tree::ptree& rMatch = rItem.second;
            const sal_Int32 nStart = nOffset + rMatch.get<sal_Int32>("nStart", -1);
            const sal_Int32 nEnd = nOffset + rMatch.get<sal_Int32>("nEnd", -1);
            if (nStart < 0 || nEnd <= nStart || nEnd > nCodePoints)
                continue;

            SingleProofreadingError aError;
            aError.nErrorStart = aUnitAt[nStart];
            aError.nErrorLength = aUnitAt[nEnd] - aUnitAt[nStart];
            aError.nErrorType = css::text::TextMarkupType::PROOFREADING;
            aError.aRuleIdentifier
                = OStringToOUString(rMatch.get<std::string>("sRuleId", {}), RTL_TEXTENCODING_UTF8);
            aError.aShortComment
                = OStringToOUString(rMatch.get<std::string>("sMessage", {}), RTL_TEXTENCODING_UTF8);
            aError.aFullComment = aError.aShortComment;
            setLineColor(aError, lineColorForType(rMatch.get<std::string>("sType", {})));

            const std::string aURL = rMatch.get<std::string>("URL", {});
            if (!aURL.empty())
                setFullCommentURL(aError, OStringToOUString(aURL, RTL_TEXTENCODING_UTF8));

            if (const boost::optional<const boost::property_tree::ptree&> oSuggestions
                = rMatch.get_child_optional("aSuggestions"))
                takeSuggestions(aError, *oSuggestions, nullptr);

            aErrors.push_back(aError);
        }
    }
    return aErrors;
}

cpo::uno::Sequence<css::beans::PropertyValue> GrammalecteChecker::probe()
{
    Request aRequest;
    aRequest.aURL = getBaseURL() + "/get_options/fr";
    aRequest.bVerifySSL = GrammalecteCfg::SSLCertVerify::get();
    const bool bReachable = !getBaseURL().isEmpty() && get(aRequest).eCode == CURLE_OK;
    return { comphelper::makePropertyValue(u"IsReachable"_ustr, bReachable) };
}

OUString GrammalecteChecker::getServiceDisplayName(const css::lang::Locale& rLocale)
{
    return Translate::get(STR_DESCRIPTION_GRAMMALECTE,
                          Translate::Create("svt", LanguageTag(rLocale)));
}

OUString GrammalecteChecker::getImplementationName() { return IMPLEMENTATION_NAME; }

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
lingucomponent_Grammalecte_get_implementation(cpo::uno::XComponentContext*,
                                              cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new GrammalecteChecker());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
