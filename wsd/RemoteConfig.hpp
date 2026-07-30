/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Remote configuration fetching and management.
 * Classes: RemoteConfigPoll
 */

#pragma once

#if defined(MOBILEAPP) && MOBILEAPP
#error This file should be excluded from Mobile App builds
#endif // MOBILEAPP

#include <common/ConfigUtil.hpp>
#include <common/JsonUtil.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <string>

/**
  A custom socket poll to fetch remote config every 60 seconds.
  If config changes it applies the new config using LayeredConfiguration.

  The URI to fetch is read from the configuration too - it is either the
  remote config itself, or the font configuration.  It si passed via the
  uriConfigKey param.
*/
class RemoteJSONPoll : public SocketPoll
{
public:
    RemoteJSONPoll(Poco::Util::LayeredConfiguration& config, const std::string& uriConfigKey,
                   const std::string& name, const std::string& kind)
        : SocketPoll(name)
        , _conf(config)
        , _configKey(uriConfigKey)
        , _expectedKind(kind)
    {
    }

    virtual void handleJSON(const Poco::JSON::Object::Ptr& json) = 0;

    virtual void handleUnchangedJSON() {}

    void start();

    void pollingThread();

protected:
    Poco::Util::LayeredConfiguration& _conf;
    std::string _eTagValue;

private:
    std::string _configKey;
    std::string _expectedKind;
};

class RemoteConfigPoll : public RemoteJSONPoll
{
public:
    RemoteConfigPoll(Poco::Util::LayeredConfiguration& config)
        : RemoteJSONPoll(config, "remote_config.remote_url", "remoteconfig_poll", "configuration")
    {
        constexpr int PRIO_JSON = -200; // highest priority
        _persistConfig = new ConfigUtil::AppConfigMap;
        _conf.addWriteable(_persistConfig, PRIO_JSON);
    }

    void handleJSON(const Poco::JSON::Object::Ptr& remoteJson) override;

    void fetchLockedHostPatterns(std::map<std::string, std::string>& newAppConfig,
                                 Poco::JSON::Object::Ptr remoteJson);

    void fetchAliasGroups(std::map<std::string, std::string>& newAppConfig,
                          const Poco::JSON::Object::Ptr& remoteJson);

    static void fetchRemoteFontConfig(std::map<std::string, std::string>& newAppConfig,
                               const Poco::JSON::Object::Ptr& remoteJson);

    void fetchLockedTranslations(std::map<std::string, std::string>& newAppConfig,
                                 const Poco::JSON::Object::Ptr& remoteJson);

    static void fetchUnlockImageUrl(std::map<std::string, std::string>& newAppConfig,
                             const Poco::JSON::Object::Ptr& remoteJson);

    void fetchIndirectionEndpoint(std::map<std::string, std::string>& newAppConfig,
                                  const Poco::JSON::Object::Ptr& remoteJson);

    void fetchMonitors(std::map<std::string, std::string>& newAppConfig,
                       const Poco::JSON::Object::Ptr& remoteJson);

    static void handleOptions(const Poco::JSON::Object::Ptr& remoteJson);

    //sets property to false if it is missing from JSON
    //and returns std::string
    static std::string booleanToString(Poco::Dynamic::Var& booleanFlag)
    {
        if (booleanFlag.isEmpty())
        {
            booleanFlag = "false";
        }
        return booleanFlag.toString();
    }

private:
    // keeps track of remote config layer
    Poco::AutoPtr<ConfigUtil::AppConfigMap> _persistConfig = nullptr;
};

class RemoteFontConfigPoll : public RemoteJSONPoll
{
public:
    RemoteFontConfigPoll(Poco::Util::LayeredConfiguration& config)
        : RemoteJSONPoll(config, "remote_font_config.url", "remotefontconfig_poll",
                         "fontconfiguration")
    {
    }

    void handleJSON(const Poco::JSON::Object::Ptr& remoteJson) override;

    void handleUnchangedJSON() override;

private:
    bool downloadPlain(const std::string& uri);

    static bool eTagUnchanged(const std::string& uri, const std::string& oldETag);

    bool downloadWithETag(const std::string& uri, const std::string& oldETag);

    bool finishDownload(const std::string& uri,
                        const std::shared_ptr<const http::Response>& httpResponse);

    void restartForKitAndReDownloadConfigFile();

    struct FontData
    {
        // Each font can have a "stamp" in the JSON that we treat just as a string. In practice it
        // can be some timestamp, but we don't parse it. If the stamp is changed, we re-download the
        // font file.
        std::string stamp;

        // If the font has no "stamp" property, we use the ETag mechanism to see if the font file
        // needs to be re-downloaded.
        std::string eTag;

        // Where the font has been stored
        std::string pathName;

        // Flag that tells whether the font is mentioned in the JSON file that is being handled.
        // Used only in handleJSON() when the JSON has been (re-)downloaded, not when the JSON was
        // unchanged in handleUnchangedJSON().
        bool active;
    };

    // The key of this map is the download URI of the font.
    std::map<std::string, FontData> fonts;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
