/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "kittranslation.hxx"
#include "kitremotechecker.hxx"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <comphelper/configuration.hxx>
#include <linguistic/translate.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>

namespace desktop::translation
{
namespace
{
using TranslationCfg = officecfg::Office::Linguistic::Translation;
using DeeplCfg = officecfg::Office::Linguistic::Translation::Deepl;
using LibreTranslateCfg = officecfg::Office::Linguistic::Translation::LibreTranslate;

// The kit's environment prefix of a provider, by the provider's id.
const char* getEnvPrefix(std::u16string_view rId)
{
    if (rId == u"deepl")
        return "DEEPL";
    if (rId == u"libretranslate")
        return "LIBRETRANSLATE";
    return nullptr;
}

// The settings dialog's view of a provider's settings. The keys stay here:
// the dialog learns that there is one.
void putProviderSettings(tools::JsonWriter& rJson, std::u16string_view rId)
{
    if (rId == u"deepl")
    {
        rJson.put("apiURL", DeeplCfg::ApiURL::get().value_or(OUString()));
        rJson.put("hasCredentials", !DeeplCfg::AuthKey::get().value_or(OUString()).isEmpty());
    }
    else if (rId == u"libretranslate")
    {
        rJson.put("baseURL", LibreTranslateCfg::BaseURL::get().value_or(OUString()));
        rJson.put("hasCredentials",
                  !LibreTranslateCfg::ApiKey::get().value_or(OUString()).isEmpty());
        rJson.put("sslVerify", LibreTranslateCfg::SSLCertVerify::get());
    }
    else
        rJson.put("hasCredentials", false);
}
} // namespace

bool allowsUserSettings(const char* pPrefix)
{
    // The same rule as for the remote checkers: the server says, and a
    // provider the server switched off stays off for everyone.
    return desktop::remotechecker::allowsUserSettings(pPrefix);
}

// What the server set up for the translation: from the environment it set
// up, with the defaults of the schema where it says nothing, as in the
// apps, which set no variable at all.
namespace
{
struct ServerSettings
{
    OUString aProvider;
    bool bDeeplEnabled = true;
    std::optional<OUString> oDeeplApiURL;
    std::optional<OUString> oDeeplAuthKey;
    bool bLibreTranslateEnabled = false;
    std::optional<OUString> oLibreTranslateBaseURL;
    std::optional<OUString> oLibreTranslateApiKey;
    bool bLibreTranslateVerifySSL = true;
};

ServerSettings getServerSettings()
{
    using desktop::remotechecker::envBool;
    using desktop::remotechecker::envString;
    ServerSettings aSettings;
    aSettings.aProvider = envString("TRANSLATION", "PROVIDER").value_or(OUString());
    aSettings.bDeeplEnabled = envBool("DEEPL", "ENABLED").value_or(true);
    aSettings.oDeeplApiURL = envString("DEEPL", "API_URL");
    aSettings.oDeeplAuthKey = envString("DEEPL", "AUTH_KEY");
    aSettings.bLibreTranslateEnabled = envBool("LIBRETRANSLATE", "ENABLED").value_or(false);
    aSettings.oLibreTranslateBaseURL = envString("LIBRETRANSLATE", "BASE_URL");
    aSettings.oLibreTranslateApiKey = envString("LIBRETRANSLATE", "API_KEY");
    aSettings.bLibreTranslateVerifySSL = envBool("LIBRETRANSLATE", "SSL_VERIFICATION").value_or(true);
    return aSettings;
}

// What the server set up for one provider, for the dialog to show when the
// user drops their own settings; the key stays here too.
void putServerSettings(tools::JsonWriter& rJson, std::u16string_view rId,
                       const ServerSettings& rServer)
{
    tools::JsonWriter::ScopedJsonWriterNode<'}'> aNode = rJson.startNode("server");
    if (rId == u"deepl")
    {
        rJson.put("enabled", rServer.bDeeplEnabled);
        rJson.put("apiURL", rServer.oDeeplApiURL.value_or(OUString()));
        rJson.put("hasCredentials", !rServer.oDeeplAuthKey.value_or(OUString()).isEmpty());
    }
    else if (rId == u"libretranslate")
    {
        rJson.put("enabled", rServer.bLibreTranslateEnabled);
        rJson.put("baseURL", rServer.oLibreTranslateBaseURL.value_or(OUString()));
        rJson.put("hasCredentials", !rServer.oLibreTranslateApiKey.value_or(OUString()).isEmpty());
        rJson.put("sslVerify", rServer.bLibreTranslateVerifySSL);
    }
}
} // namespace

// Applies what the server set up. This runs before a user's own settings
// are applied, so a user who drops a setting of their own is back at the
// server's.
void setTranslationConfig()
{
    try
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());

        const ServerSettings aServer = getServerSettings();
        TranslationCfg::Provider::set(aServer.aProvider, batch);
        DeeplCfg::IsEnabled::set(aServer.bDeeplEnabled, batch);
        DeeplCfg::ApiURL::set(aServer.oDeeplApiURL, batch);
        DeeplCfg::AuthKey::set(aServer.oDeeplAuthKey, batch);
        LibreTranslateCfg::IsEnabled::set(aServer.bLibreTranslateEnabled, batch);
        LibreTranslateCfg::BaseURL::set(aServer.oLibreTranslateBaseURL, batch);
        LibreTranslateCfg::ApiKey::set(aServer.oLibreTranslateApiKey, batch);
        LibreTranslateCfg::SSLCertVerify::set(aServer.bLibreTranslateVerifySSL, batch);

        batch->commit();
    }
    catch (const cpo::uno::Exception& rException)
    {
        SAL_WARN("kit", "Failed to set the translation settings: " << rException.Message);
    }
}

// The translation providers as the settings dialog needs them: which one
// translates, and how each is set up. The keys stay here. The services are
// not asked here: the languages are what they answered when they were asked.
std::string getTranslationProviders()
{
    tools::JsonWriter aJson;
    aJson.put("commandName", ".uno:TranslationProviders");
    {
        tools::JsonWriter::ScopedJsonWriterNode<'}'> aValues = aJson.startNode("commandValues");
        aJson.put("provider", TranslationCfg::Provider::get());
        const ServerSettings aServer = getServerSettings();
        aJson.put("serverProvider", aServer.aProvider);
        const std::shared_ptr<linguistic::TranslationProvider> pActive
            = linguistic::getActiveTranslationProvider();
        aJson.put("active", pActive ? pActive->getId() : OString());
        tools::JsonWriter::ScopedJsonWriterNode<']'> aProviders = aJson.startArray("providers");
        for (const std::shared_ptr<linguistic::TranslationProvider>& pProvider :
             linguistic::getTranslationProviders())
        {
            tools::JsonWriter::ScopedJsonWriterNode<'}'> aProvider = aJson.startStruct();
            const OString aId = pProvider->getId();
            const OUString aIdU = OStringToOUString(aId, RTL_TEXTENCODING_UTF8);
            aJson.put("id", aId);
            aJson.put("name", pProvider->getDisplayName());
            aJson.put("enabled", pProvider->isEnabled());
            aJson.put("configured", pProvider->isConfigured());
            const char* pEnvPrefix = getEnvPrefix(aIdU);
            aJson.put("allowUserSettings", pEnvPrefix ? allowsUserSettings(pEnvPrefix) : false);
            putProviderSettings(aJson, aIdU);
            putServerSettings(aJson, aIdU, aServer);
            {
                tools::JsonWriter::ScopedJsonWriterNode<']'> aLanguages
                    = aJson.startArray("targetLanguages");
                for (const OString& rLanguage :
                     pProvider->getTargetLanguages(/*bAskService=*/false))
                    aJson.putSimpleValue(OStringToOUString(rLanguage, RTL_TEXTENCODING_UTF8));
            }
        }
    }
    return aJson.finishAndGetAsStdString();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
