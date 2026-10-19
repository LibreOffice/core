/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "kitremotechecker.hxx"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cpo/uno/XComponentContext.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <linguistic/misc.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>
#include <unotools/lingucfg.hxx>
#include <vcl/svapp.hxx>

using namespace css;
using namespace ::cpo;

namespace desktop::remotechecker
{
namespace
{
// What the settings dialog and the kit know about a remote checker: the
// configuration group, the kit's environment variables, and whether it
// marks misspellings too, in which case the local spell checker gives way
// to it for its languages.
struct RemoteChecker
{
    const char* pId;
    OUString aImplementationName;
    const char* pEnvPrefix;
    // The group under GrammarChecking in the linguistic configuration that
    // holds the checker's settings.
    OUString aConfigGroup;
    bool bChecksSpelling;
};

using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;
using DudenCfg = officecfg::Office::Linguistic::GrammarChecking::Duden;
using GrammalecteCfg = officecfg::Office::Linguistic::GrammarChecking::Grammalecte;

const RemoteChecker aRemoteCheckers[] = {
    { "languagetool", u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr, "LANGUAGETOOL",
      u"LanguageTool"_ustr, true },
    { "duden", u"cpo.lingu.Duden"_ustr, "DUDEN", u"Duden"_ustr, true },
    { "grammalecte", u"cpo.lingu.Grammalecte"_ustr, "GRAMMALECTE",
      u"Grammalecte"_ustr, false },
};

std::optional<OUString> envString(const char* pPrefix, const char* pName)
{
    const char* pValue = ::getenv(OString(OString::Concat(pPrefix) + "_" + pName).getStr());
    if (!pValue)
        return std::nullopt;
    return OStringToOUString(pValue, RTL_TEXTENCODING_UTF8);
}

std::optional<bool> envBool(const char* pPrefix, const char* pName)
{
    const std::optional<OUString> oValue = envString(pPrefix, pName);
    if (!oValue)
        return std::nullopt;
    return *oValue == "true";
}

// A list of names separated by commas; a name may have spaces in it.
std::optional<cpo::uno::Sequence<OUString>> envList(const char* pPrefix, const char* pName)
{
    const std::optional<OUString> oValue = envString(pPrefix, pName);
    if (!oValue)
        return std::nullopt;
    std::vector<OUString> aItems;
    for (sal_Int32 nIndex = 0; nIndex >= 0;)
    {
        const OUString aItem(o3tl::trim(o3tl::getToken(*oValue, 0, ',', nIndex)));
        if (!aItem.isEmpty())
            aItems.push_back(aItem);
    }
    return comphelper::containerToSequence(aItems);
}

} // namespace

// Whether a user's own settings for a remote checker are applied from their
// document settings. The server says; where there is no server, as in the
// desktop app, there is nobody else on the document and the answer is yes.
// A checker the server switched off stays off for everyone: that switch
// keeps the text in house, and is nobody else's to undo.
bool allowsUserSettings(const char* pPrefix)
{
    return envBool(pPrefix, "ENABLED").value_or(true)
           && envBool(pPrefix, "ALLOW_USER_SETTINGS").value_or(true);
}

namespace
{
constexpr OUString REMOTE_CHECKER_CONFIG_PATH = u"/org.openoffice.Office.Linguistic/GrammarChecking"_ustr;

// A checker's settings group, read-only and live: each call sees what the
// configuration holds at that moment. Empty when the configuration cannot
// be reached, which the readers below take as the defaults.
cpo::uno::Reference<css::container::XNameAccess> getRemoteCheckerConfig(const RemoteChecker& rChecker)
{
    try
    {
        cpo::uno::Reference<css::lang::XMultiServiceFactory> xProvider(
            css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext()));
        const css::beans::NamedValue aPath(
            u"nodepath"_ustr, cpo::uno::Any(REMOTE_CHECKER_CONFIG_PATH + "/" + rChecker.aConfigGroup));
        return cpo::uno::Reference<css::container::XNameAccess>(
            xProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationAccess"_ustr, { cpo::uno::Any(aPath) }),
            uno::UNO_QUERY);
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "cannot read the settings of " << rChecker.pId << ": " << rException.Message);
        return {};
    }
}

// A setting of the group, or the default where the group has no such
// setting or it is unset.
template <typename T>
T getRemoteCheckerSetting(const cpo::uno::Reference<css::container::XNameAccess>& xGroup,
                          const OUString& rName, const T& rDefault)
{
    T aValue = rDefault;
    if (xGroup.is() && xGroup->hasByName(rName))
        xGroup->getByName(rName) >>= aValue;
    return aValue;
}

// Switched on and told where its server is: without the server there is
// nothing to hand a language to.
bool isRemoteCheckerEnabled(const RemoteChecker& rChecker)
{
    const cpo::uno::Reference<css::container::XNameAccess> xGroup = getRemoteCheckerConfig(rChecker);
    return getRemoteCheckerSetting(xGroup, u"IsEnabled"_ustr, false)
           && !getRemoteCheckerSetting(xGroup, u"BaseURL"_ustr, OUString()).isEmpty();
}

// What a remote checker's server answered when it was asked about itself,
// and when it was asked.
struct RemoteCheckerProbe
{
    cpo::uno::Sequence<css::beans::PropertyValue> aAnswer;
    bool bReachable = false;
    std::chrono::steady_clock::time_point aAskedAt;
};

// A server that did not answer is asked again after this long: it may have
// been starting up.
constexpr std::chrono::seconds REMOTE_CHECKER_RETRY(30);

// The answers by checker, server and key: another server, or another key
// for it, set since is another server to ask. The listener asks without the
// solar mutex, hence a mutex of its own.
std::mutex gRemoteCheckerProbeMutex;
std::map<std::string, RemoteCheckerProbe> gRemoteCheckerProbes;

std::string probeKey(const RemoteChecker& rChecker)
{
    const cpo::uno::Reference<css::container::XNameAccess> xGroup = getRemoteCheckerConfig(rChecker);
    const OUString aURL = getRemoteCheckerSetting(xGroup, u"BaseURL"_ustr, OUString());
    const OUString aKey = getRemoteCheckerSetting(xGroup, u"ApiKey"_ustr, OUString());
    return std::string(rChecker.pId) + "|" + OUStringToOString(aURL, RTL_TEXTENCODING_UTF8).getStr()
           + "|" + OUStringToOString(aKey, RTL_TEXTENCODING_UTF8).getStr();
}

// An answer that said the server is there stands; a no stands for a while.
bool isCurrent(const RemoteCheckerProbe& rProbe)
{
    return rProbe.bReachable
           || std::chrono::steady_clock::now() - rProbe.aAskedAt < REMOTE_CHECKER_RETRY;
}

// The answer on record for the checker's server, current or not; nothing
// when the server has not been asked.
std::optional<RemoteCheckerProbe> knownProbe(const RemoteChecker& rChecker)
{
    const std::string aKey = probeKey(rChecker);
    std::lock_guard<std::mutex> aGuard(gRemoteCheckerProbeMutex);
    const std::map<std::string, RemoteCheckerProbe>::const_iterator aKnown
        = gRemoteCheckerProbes.find(aKey);
    if (aKnown == gRemoteCheckerProbes.end())
        return std::nullopt;
    return aKnown->second;
}

// Asks the checker's server about itself, unless a current answer is on
// record. Waits for the round trip, which the probe's limit keeps short;
// a server that did not answer is not asked again for a while, so a dead
// one holds up a caller at most once in that while.
RemoteCheckerProbe probeRemoteChecker(const RemoteChecker& rChecker)
{
    if (const std::optional<RemoteCheckerProbe> oKnown = knownProbe(rChecker))
    {
        if (isCurrent(*oKnown))
            return *oKnown;
    }

    // The key goes with the answer: the settings may change while the
    // server is being asked, and then the answer is about the old server.
    const std::string aKey = probeKey(rChecker);
    RemoteCheckerProbe aProbe;
    try
    {
        cpo::uno::Reference<css::beans::XPropertyAccess> xChecker(
            comphelper::getProcessComponentContext()->getServiceManager()->createInstanceWithContext(
                rChecker.aImplementationName, comphelper::getProcessComponentContext()),
            uno::UNO_QUERY_THROW);
        aProbe.aAnswer = xChecker->getPropertyValues();
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "cannot ask " << rChecker.pId << " about its server: " << rException.Message);
    }
    for (const css::beans::PropertyValue& rValue : aProbe.aAnswer)
    {
        if (rValue.Name == "IsReachable")
            rValue.Value >>= aProbe.bReachable;
    }
    aProbe.aAskedAt = std::chrono::steady_clock::now();
    if (!aProbe.bReachable)
        std::cerr << "Remote checker " << rChecker.pId
                  << " has not answered; keeping the local spell checker\n";

    std::lock_guard<std::mutex> aGuard(gRemoteCheckerProbeMutex);
    gRemoteCheckerProbes[aKey] = aProbe;
    return aProbe;
}

// The languages the local spell checker gave up, as BCP 47 tags, with the
// spell checkers it had for each: the remote checker that took them may be
// switched off again, and then it gets them back as they were. Under the
// solar mutex, which every caller of the hand-over takes.
std::map<OUString, cpo::uno::Sequence<OUString>> gLocalesHandedOver;

void watchRemoteCheckerConfig();

// Asks the servers of the remote checkers that spell check and are switched
// on, where no current answer is on record.
void askRemoteCheckers()
{
    for (const RemoteChecker& rChecker : aRemoteCheckers)
    {
        if (rChecker.bChecksSpelling && isRemoteCheckerEnabled(rChecker))
            probeRemoteChecker(rChecker);
    }
}

} // namespace

// Switches the local spell checker off for the languages a reachable remote
// checker spell checks itself, and back on for the languages no remote
// checker holds any more. One that has not answered keeps the local spell
// checker: a language without any spell checking would be worse than one
// with two. A server without a current answer on record is asked here.
void handLanguagesToRemoteCheckers()
{
    watchRemoteCheckerConfig();

    try
    {
        cpo::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv =
            css::linguistic2::LinguServiceManager::create(comphelper::getProcessComponentContext());
        std::set<OUString> aClaimed;

        for (const RemoteChecker& rChecker : aRemoteCheckers)
        {
            if (!rChecker.bChecksSpelling || !isRemoteCheckerEnabled(rChecker))
                continue;
            if (!probeRemoteChecker(rChecker).bReachable)
                continue;

            cpo::uno::Reference<css::linguistic2::XSupportedLocales> xRemote(
                comphelper::getProcessComponentContext()->getServiceManager()->createInstanceWithContext(
                    rChecker.aImplementationName, comphelper::getProcessComponentContext()),
                uno::UNO_QUERY_THROW);
            for (const css::lang::Locale& rLocale : xRemote->getLocales())
                aClaimed.insert(LanguageTag::convertToBcp47(rLocale));
        }

        for (const OUString& rTag : aClaimed)
        {
            if (gLocalesHandedOver.count(rTag))
                continue;
            const css::lang::Locale aLocale = LanguageTag::convertToLocale(rTag);
            // Only a language the local spell checker has is taken from it,
            // and what it had is kept for when it gets the language back.
            const cpo::uno::Sequence<OUString> aLocal
                = xLangSrv->getConfiguredServices(SN_SPELLCHECKER, aLocale);
            if (!aLocal.hasElements())
                continue;
            xLangSrv->setConfiguredServices(SN_SPELLCHECKER, aLocale, {});
            gLocalesHandedOver[rTag] = aLocal;
        }
        for (std::map<OUString, cpo::uno::Sequence<OUString>>::iterator aIt = gLocalesHandedOver.begin();
             aIt != gLocalesHandedOver.end();)
        {
            if (aClaimed.count(aIt->first))
            {
                ++aIt;
                continue;
            }
            xLangSrv->setConfiguredServices(SN_SPELLCHECKER,
                                            LanguageTag::convertToLocale(aIt->first), aIt->second);
            aIt = gLocalesHandedOver.erase(aIt);
        }
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "Failed to hand languages to the remote checkers: " << rException.Message);
    }
}

namespace
{
// Watches the remote checkers' settings, so a checker switched on or off
// while a document is open takes its languages, or gives them back, at once.
// A settings file handed in through setOption arrives without the solar
// mutex: the servers are asked before it is taken, and the hand-over, which
// changes the spell checker lists the documents hear of, takes it, as the
// unotools listener does. The watch is set up by the first hand-over.
class RemoteCheckerConfigListener final : public cppu::WeakImplHelper<css::util::XChangesListener>
{
public:
    void changesOccurred(const css::util::ChangesEvent&) override
    {
        askRemoteCheckers();
        SolarMutexGuard aGuard;
        handLanguagesToRemoteCheckers();
    }

    void disposing(const css::lang::EventObject&) override {}
};

cpo::uno::Reference<css::util::XChangesNotifier> gRemoteCheckerConfigNode;

void watchRemoteCheckerConfig()
{
    if (gRemoteCheckerConfigNode.is())
        return;
    try
    {
        cpo::uno::Reference<css::lang::XMultiServiceFactory> xProvider(
            css::configuration::theDefaultProvider::get(comphelper::getProcessComponentContext()));
        const css::beans::NamedValue aPath(
            u"nodepath"_ustr, cpo::uno::Any(u"/org.openoffice.Office.Linguistic/GrammarChecking"_ustr));
        gRemoteCheckerConfigNode.set(
            xProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationAccess"_ustr, { cpo::uno::Any(aPath) }),
            uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::util::XChangesListener> xListener(new RemoteCheckerConfigListener);
        gRemoteCheckerConfigNode->addChangesListener(xListener);
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "cannot watch the remote checker settings: " << rException.Message);
    }
}

} // namespace

// Takes the remote checker settings from the environment the server set up.
// The languages are handed over separately: by the forkit, whose kits
// inherit the servers' answers, and by a kit when it loads a document.
void setRemoteCheckerConfig()
{
    try
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

        // A profile that drove the Duden Korrekturserver through the
        // LanguageTool group, with RestProtocol set to "duden", has its server
        // moved to the Duden group, once; the environment below, where there
        // is one, has the last word.
        if (LanguageToolCfg::RestProtocol::get().value_or(OUString()) == "duden")
        {
            if (DudenCfg::BaseURL::get().value_or(OUString()).isEmpty())
            {
                DudenCfg::BaseURL::set(LanguageToolCfg::BaseURL::get().value_or(OUString()), batch);
                DudenCfg::ApiKey::set(LanguageToolCfg::ApiKey::get().value_or(OUString()), batch);
                DudenCfg::SSLCertVerify::set(LanguageToolCfg::SSLCertVerify::get(), batch);
                DudenCfg::IsEnabled::set(LanguageToolCfg::IsEnabled::get(), batch);
            }
            LanguageToolCfg::IsEnabled::set(false, batch);
            LanguageToolCfg::RestProtocol::set(OUString(), batch);
        }

        if (const std::optional<bool> oEnabled = envBool("LANGUAGETOOL", "ENABLED"))
            LanguageToolCfg::IsEnabled::set(*oEnabled, batch);
        if (const std::optional<OUString> oValue = envString("LANGUAGETOOL", "BASEURL"))
            LanguageToolCfg::BaseURL::set(*oValue, batch);
        if (const std::optional<bool> oValue = envBool("LANGUAGETOOL", "SSL_VERIFICATION"))
            LanguageToolCfg::SSLCertVerify::set(*oValue, batch);
        if (const std::optional<OUString> oValue = envString("LANGUAGETOOL", "USERNAME"))
            LanguageToolCfg::Username::set(*oValue, batch);
        if (const std::optional<OUString> oValue = envString("LANGUAGETOOL", "APIKEY"))
            LanguageToolCfg::ApiKey::set(*oValue, batch);

        if (const std::optional<bool> oEnabled = envBool("DUDEN", "ENABLED"))
            DudenCfg::IsEnabled::set(*oEnabled, batch);
        if (const std::optional<OUString> oValue = envString("DUDEN", "BASEURL"))
            DudenCfg::BaseURL::set(*oValue, batch);
        if (const std::optional<bool> oValue = envBool("DUDEN", "SSL_VERIFICATION"))
            DudenCfg::SSLCertVerify::set(*oValue, batch);
        if (const std::optional<OUString> oValue = envString("DUDEN", "APIKEY"))
            DudenCfg::ApiKey::set(*oValue, batch);
        if (const std::optional<OUString> oValue = envString("DUDEN", "SPELLCHECKING_LEVEL"))
            DudenCfg::SpellcheckingLevel::set(oValue->toInt32(), batch);
        if (const std::optional<OUString> oValue = envString("DUDEN", "ORTHOGRAPHY_STANDARD"))
            DudenCfg::OrthographyStandard::set(*oValue, batch);
        if (const std::optional<OUString> oValue = envString("DUDEN", "MESSAGE_LEVEL");
            oValue && !oValue->isEmpty())
            DudenCfg::MessageLevel::set(oValue->toInt32(), batch);
        if (const std::optional<cpo::uno::Sequence<OUString>> oValue = envList("DUDEN", "PROPERTY_SETS"))
            DudenCfg::PropertySets::set(*oValue, batch);
        if (const std::optional<cpo::uno::Sequence<OUString>> oValue = envList("DUDEN", "DICTIONARIES"))
            DudenCfg::Dictionaries::set(*oValue, batch);

        if (const std::optional<bool> oEnabled = envBool("GRAMMALECTE", "ENABLED"))
            GrammalecteCfg::IsEnabled::set(*oEnabled, batch);
        if (const std::optional<OUString> oValue = envString("GRAMMALECTE", "BASEURL"))
            GrammalecteCfg::BaseURL::set(*oValue, batch);
        if (const std::optional<bool> oValue = envBool("GRAMMALECTE", "SSL_VERIFICATION"))
            GrammalecteCfg::SSLCertVerify::set(*oValue, batch);

        batch->commit();
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "Failed to set the remote checker settings: " << rException.Message);
    }
}

namespace
{
// The settings dialog's name for a setting: the configuration's, with a
// small first letter, except for the switch and the certificate check, which
// keep the names the dialog has had for them.
std::string getSettingName(std::u16string_view rName)
{
    if (rName == u"IsEnabled")
        return "enabled";
    if (rName == u"SSLCertVerify")
        return "sslVerify";
    const OUString aName = OUString(rName.substr(0, 1)).toAsciiLowerCase() + rName.substr(1);
    return std::string(OUStringToOString(aName, RTL_TEXTENCODING_UTF8));
}

// Writes a configuration value as JSON: a group as an object holding its
// settings under their own names, a string list as an array, an unset
// string as an empty one.
void putConfigValue(tools::JsonWriter& rJson, const std::string& rName,
                           const cpo::uno::Any& rValue)
{
    bool bValue = false;
    sal_Int32 nValue = 0;
    OUString aString;
    cpo::uno::Sequence<OUString> aStrings;
    cpo::uno::Reference<css::container::XNameAccess> xGroup;
    if (rValue >>= bValue)
        rJson.put(rName, bValue);
    else if (rValue >>= nValue)
        rJson.put(rName, nValue);
    else if (rValue >>= aString)
        rJson.put(rName, aString);
    else if (rValue >>= aStrings)
    {
        auto aArray = rJson.startArray(rName);
        for (const OUString& rItem : aStrings)
            rJson.putSimpleValue(rItem);
    }
    else if (rValue >>= xGroup)
    {
        auto aNode = rJson.startNode(rName);
        for (const OUString& rChild : xGroup->getElementNames())
            putConfigValue(rJson, std::string(OUStringToOString(rChild, RTL_TEXTENCODING_UTF8)),
                           xGroup->getByName(rChild));
    }
    else if (!rValue.hasValue())
        rJson.put(rName, "");
}

// A checker's settings group as the dialog takes it. The API key stays
// here: the dialog learns that there is one.
void putRemoteCheckerSettings(tools::JsonWriter& rJson,
                                     const cpo::uno::Reference<css::container::XNameAccess>& xGroup)
{
    bool bKeySeen = false;
    if (xGroup.is())
    {
        for (const OUString& rName : xGroup->getElementNames())
        {
            const cpo::uno::Any aValue = xGroup->getByName(rName);
            if (rName == "ApiKey")
            {
                OUString aKey;
                aValue >>= aKey;
                rJson.put("hasCredentials", !aKey.isEmpty());
                bKeySeen = true;
                continue;
            }
            putConfigValue(rJson, getSettingName(rName), aValue);
        }
    }
    if (!bKeySeen)
        rJson.put("hasCredentials", false);
}

} // namespace

// The remote sentence checkers as the settings dialog needs them: how each
// is set up, which languages it serves, and what its server offers to
// choose from. The API keys stay here. The servers are not asked here:
// what they answered when they were asked is reported.
std::string getRemoteGrammarCheckers()
{
    tools::JsonWriter aJson;
    aJson.put("commandName", ".uno:RemoteGrammarCheckers");
    {
        auto aCheckers = aJson.startArray("commandValues");
        for (const RemoteChecker& rChecker : aRemoteCheckers)
        {
            auto aChecker = aJson.startStruct();
            aJson.put("id", rChecker.pId);
            aJson.put("allowUserSettings", allowsUserSettings(rChecker.pEnvPrefix));
            aJson.put("checksSpelling", rChecker.bChecksSpelling);

            {
                cpo::uno::Sequence<OUString> aLocales;
                SvtLinguConfig().GetLocaleListFor(u"GrammarCheckers"_ustr, rChecker.aImplementationName, aLocales);
                auto aLocaleArray = aJson.startArray("locales");
                for (const OUString& rLocale : aLocales)
                    aJson.putSimpleValue(rLocale);
            }

            putRemoteCheckerSettings(aJson, getRemoteCheckerConfig(rChecker));

            // What the server offers, when it has been asked and answered:
            // each list in its answer, as "available" and the list's name.
            if (!isRemoteCheckerEnabled(rChecker))
                continue;
            const std::optional<RemoteCheckerProbe> oProbe = knownProbe(rChecker);
            if (!oProbe)
                continue;
            for (const css::beans::PropertyValue& rValue : oProbe->aAnswer)
            {
                cpo::uno::Sequence<OUString> aNames;
                if (rValue.Name != "IsReachable" && (rValue.Value >>= aNames))
                    putConfigValue(aJson, "available" + std::string(OUStringToOString(rValue.Name, RTL_TEXTENCODING_UTF8)),
                                   rValue.Value);
            }
        }
    }
    return aJson.finishAndGetAsStdString();
}

std::vector<css::lang::Locale> getRemoteSpellingLocales()
{
    std::vector<css::lang::Locale> aLocales;
    const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext
        = comphelper::getProcessComponentContext();
    for (const RemoteChecker& rChecker : aRemoteCheckers)
    {
        if (!rChecker.bChecksSpelling || !isRemoteCheckerEnabled(rChecker))
            continue;
        uno::Reference< linguistic2::XSupportedLocales > xRemote(
                xContext->getServiceManager()->createInstanceWithContext(rChecker.aImplementationName, xContext),
                uno::UNO_QUERY );
        for (const css::lang::Locale& rLocale : xRemote->getLocales())
            aLocales.push_back(rLocale);
    }
    return aLocales;
}

void logRemoteCheckerLanguages()
{
    for (const RemoteChecker& rChecker : aRemoteCheckers)
    {
        if (!isRemoteCheckerEnabled(rChecker))
            continue;
        try
        {
            cpo::uno::Reference<css::linguistic2::XSupportedLocales> xRemote(
                comphelper::getProcessComponentContext()->getServiceManager()->createInstanceWithContext(
                    rChecker.aImplementationName, comphelper::getProcessComponentContext()),
                uno::UNO_QUERY_THROW);
            std::cerr << "Remote checker " << rChecker.pId << " languages: ";
            for (const css::lang::Locale& rLocale : xRemote->getLocales())
                std::cerr << LanguageTag::convertToBcp47(rLocale) << " ";
            std::cerr << "\n";
        }
        catch (const cpo::uno::Exception& rException)
        {
            SAL_WARN("kit", "cannot create " << rChecker.pId << ": " << rException.Message);
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
