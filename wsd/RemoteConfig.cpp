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
 * Implementation of remote configuration polling.
 * Classes: RemoteConfigPoll
 */

#include <config.h>

#if defined(MOBILEAPP) && MOBILEAPP
#error This file should be excluded from Mobile App builds
#endif // MOBILEAPP

#include "RemoteConfig.hpp"

#include <common/CommandControl.hpp>
#include <common/HexUtil.hpp>
#include <common/JsonUtil.hpp>
#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>
#include <wsd/Admin.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/HostUtil.hpp>
#include <wsd/wopi/StorageConnectionManager.hpp>

#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/MapConfiguration.h>

#include <string>

void RemoteJSONPoll::start()
{
    Poco::URI remoteServerURI(_conf.getString(_configKey));

    if (_expectedKind == "configuration")
    {
        if (remoteServerURI.empty())
        {
            LOG_INF("Remote " << _expectedKind << " is not specified in coolwsd.xml");
            return; // no remote config server setup.
        }

        if constexpr (!Util::isDebugEnabled())
        {
            if (Util::iequal(remoteServerURI.getScheme(), "http"))
            {
                LOG_ERR("Remote config url should only use HTTPS protocol: "
                        << remoteServerURI.toString());
                return;
            }
        }
    }

    startThread();
}

void RemoteJSONPoll::pollingThread()
{
    while (!isStop() && !SigUtil::getShutdownRequestFlag())
    {
        Poco::URI remoteServerURI(_conf.getString(_configKey));

        // don't try to fetch from an empty URI
        bool valid = !remoteServerURI.empty();

        if constexpr (!Util::isDebugEnabled())
        {
            if (Util::iequal(remoteServerURI.getScheme(), "http"))
            {
                LOG_ERR("Remote config url should only use HTTPS protocol: "
                        << remoteServerURI.toString());
                valid = false;
            }
        }

        if (valid)
        {
            try
            {
                std::shared_ptr<http::Session> httpSession(
                    StorageConnectionManager::getHttpSession(remoteServerURI));
                http::Request request(remoteServerURI.getPathAndQuery());

                //we use ETag header to check whether JSON is modified or not
                if (!_eTagValue.empty())
                {
                    request.set("If-None-Match", _eTagValue);
                }

                const std::shared_ptr<const http::Response> httpResponse =
                    httpSession->syncRequest(request);

                const http::StatusCode statusCode = httpResponse->statusLine().statusCode();

                if (statusCode == http::StatusCode::OK)
                {
                    _eTagValue = httpResponse->get("ETag");

                    const std::string& body = httpResponse->getBody();

                    LOG_DBG("Got " << body.size() << " bytes for " << remoteServerURI.toString());

                    Poco::JSON::Object::Ptr remoteJson;
                    if (JsonUtil::parseJSON(body, remoteJson))
                    {
                        std::string kind;
                        JsonUtil::findJSONValue(remoteJson, "kind", kind);
                        if (kind == _expectedKind)
                        {
                            handleJSON(remoteJson);
                        }
                        else
                        {
                            LOG_ERR("Make sure that " << remoteServerURI.toString()
                                                      << " contains a property 'kind' with "
                                                         "value '"
                                                      << _expectedKind << "'");
                        }
                    }
                    else
                    {
                        LOG_ERR("Could not parse the remote config JSON");
                    }
                }
                else if (statusCode == http::StatusCode::NotModified)
                {
                    LOG_DBG("Not modified since last time: " << remoteServerURI.toString());
                    handleUnchangedJSON();
                }
                else
                {
                    LOG_ERR("Remote config server has response status code: " << statusCode);
                }
            }
            catch (...)
            {
                LOG_ERR("Failed to fetch remote config JSON, Please check JSON format");
            }
        }
        poll(std::chrono::seconds(60));
    }
}

void RemoteConfigPoll::handleJSON(const Poco::JSON::Object::Ptr& remoteJson)
{
    std::map<std::string, std::string> newAppConfig;

    fetchAliasGroups(newAppConfig, remoteJson);

#if ENABLE_FEATURE_LOCK
    fetchLockedHostPatterns(newAppConfig, remoteJson);
    fetchLockedTranslations(newAppConfig, remoteJson);
    fetchUnlockImageUrl(newAppConfig, remoteJson);
#endif

    fetchIndirectionEndpoint(newAppConfig, remoteJson);

    fetchMonitors(newAppConfig, remoteJson);

    fetchRemoteFontConfig(newAppConfig, remoteJson);

    // before resetting get monitors list
    std::vector<std::pair<std::string, int>> oldMonitors = Admin::instance().getMonitorList();

    _persistConfig->reset(newAppConfig);

#if ENABLE_FEATURE_LOCK
    CommandControl::LockManager::parseLockedHost(_conf);
#endif
    Admin::instance().updateMonitors(oldMonitors);

    HostUtil::parseAliases(_conf);

    HostUtil::parseAllowedWSOrigins();

    COOLWSD::setLokitEnvironmentVariables(_conf);

    COOLWSD::IndirectionServerEnabled =
        !ConfigUtil::getConfigValue<std::string>(_conf, "indirection_endpoint.url", "").empty();
    COOLWSD::GeolocationSetup =
        ConfigUtil::getConfigValue("indirection_endpoint.geolocation_setup.enable", false);

    handleOptions(remoteJson);
}

void RemoteConfigPoll::fetchLockedHostPatterns(std::map<std::string, std::string>& newAppConfig,
                                               Poco::JSON::Object::Ptr remoteJson)
{
    try
    {
        Poco::JSON::Object::Ptr lockedHost;
        Poco::JSON::Array::Ptr lockedHostPatterns;
        try
        {
            lockedHost = remoteJson->getObject("feature_locking")->getObject("locked_hosts");
            lockedHostPatterns = lockedHost->getArray("hosts");
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding locked_hosts failed because "
                    "feature_locking->locked_hosts->hosts array does not exist");
            return;
        }

        if (lockedHostPatterns.isNull() || lockedHostPatterns->size() == 0)
        {
            LOG_INF("Overriding locked_hosts failed because locked_hosts->hosts array is empty "
                    "or null");
            return;
        }

        //use feature_lock.locked_hosts[@allow] entry from coolwsd.xml if feature_lock.locked_hosts.allow key does not exist in json
        Poco::Dynamic::Var allow =
            !lockedHost->has("allow")
                ? Poco::Dynamic::Var(_conf.getBool("feature_lock.locked_hosts[@allow]"))
                : lockedHost->get("allow");
        newAppConfig.insert(
            std::make_pair("feature_lock.locked_hosts[@allow]", booleanToString(allow)));

        if (booleanToString(allow) == "false")
        {
            LOG_INF("locked_hosts feature is disabled, set feature_lock->locked_hosts->allow "
                    "to true to enable");
            return;
        }

        std::size_t i;
        for (i = 0; i < lockedHostPatterns->size(); i++)
        {
            std::string host;
            Poco::JSON::Object::Ptr subObject = lockedHostPatterns->getObject(i);
            JsonUtil::findJSONValue(subObject, "host", host);
            Poco::Dynamic::Var readOnly = subObject->get("read_only");
            Poco::Dynamic::Var disabledCommands = subObject->get("disabled_commands");

            const std::string path = "feature_lock.locked_hosts.host[" + std::to_string(i) + "]";
            newAppConfig.insert(std::make_pair(path, host));
            newAppConfig.insert(std::make_pair(path + "[@read_only]", booleanToString(readOnly)));
            newAppConfig.insert(
                std::make_pair(path + "[@disabled_commands]", booleanToString(disabledCommands)));
        }

        //if number of locked wopi host patterns defined in coolwsd.xml are greater than number of host
        //fetched from json, overwrite the remaining host from config file to empty strings and
        //set read_only and disabled_commands to false
        for (;; ++i)
        {
            const std::string path = "feature_lock.locked_hosts.host[" + std::to_string(i) + "]";
            if (!_conf.has(path))
            {
                break;
            }
            newAppConfig.insert(std::make_pair(path, ""));
            newAppConfig.insert(std::make_pair(path + "[@read_only]", "false"));
            newAppConfig.insert(std::make_pair(path + "[@disabled_commands]", "false"));
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to fetch locked_hosts, please check JSON format: " << exc.what());
    }
}

void RemoteConfigPoll::fetchAliasGroups(std::map<std::string, std::string>& newAppConfig,
                                        const Poco::JSON::Object::Ptr& remoteJson)
{
    try
    {
        Poco::JSON::Object::Ptr aliasGroups;
        Poco::JSON::Array::Ptr groups;

        try
        {
            aliasGroups =
                remoteJson->getObject("storage")->getObject("wopi")->getObject("alias_groups");
            groups = aliasGroups->getArray("groups");
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding alias_groups failed because "
                    "storage->wopi->alias_groups->groups array does not exist");
            return;
        }

        if (groups.isNull() || groups->size() == 0)
        {
            LOG_INF("Overriding alias_groups failed because alias_groups->groups array is "
                    "empty or null");
            return;
        }

        std::string mode = "first";
        JsonUtil::findJSONValue(aliasGroups, "mode", mode);
        newAppConfig.insert(std::make_pair("storage.wopi.alias_groups[@mode]", mode));

        std::size_t i;
        for (i = 0; i < groups->size(); i++)
        {
            Poco::JSON::Object::Ptr group = groups->getObject(i);
            std::string host;
            JsonUtil::findJSONValue(group, "host", host);
            Poco::Dynamic::Var allow = group->get("allow");
            const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';

            newAppConfig.insert(std::make_pair(path + ".host", host));
            newAppConfig.insert(std::make_pair(path + ".host[@allow]", booleanToString(allow)));
#if ENABLE_FEATURE_LOCK
            std::string unlockLink;
            JsonUtil::findJSONValue(group, "unlock_link", unlockLink);
            newAppConfig.insert(std::make_pair(path + ".unlock_link", unlockLink));
#endif
            Poco::JSON::Array::Ptr aliases = group->getArray("aliases");

            size_t j = 0;
            if (aliases)
            {
                auto it = aliases->begin();
                for (; j < aliases->size(); j++)
                {
                    const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
                    newAppConfig.insert(std::make_pair(aliasPath, it->toString()));
                    it++;
                }
            }
            for (;; j++)
            {
                const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
                if (!_conf.has(aliasPath))
                {
                    break;
                }
                newAppConfig.insert(std::make_pair(aliasPath, ""));
            }
        }

        //if number of alias_groups defined in configuration are greater than number of alias_group
        //fetched from json, overwrite the remaining alias_groups from config file to empty strings and
        for (;; i++)
        {
            const std::string path =
                "storage.wopi.alias_groups.group[" + std::to_string(i) + "].host";
            if (!_conf.has(path))
            {
                break;
            }
            newAppConfig.insert(std::make_pair(path, ""));
            newAppConfig.insert(std::make_pair(path + "[@allowed]", "false"));
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Fetching of alias groups failed with error: " << exc.what()
                                                               << ", please check JSON format");
    }
}

void RemoteConfigPoll::fetchRemoteFontConfig(std::map<std::string, std::string>& newAppConfig,
                                             const Poco::JSON::Object::Ptr& remoteJson)
{
    try
    {
        Poco::JSON::Object::Ptr remoteFontConfig = remoteJson->getObject("remote_font_config");

        std::string url;
        if (JsonUtil::findJSONValue(remoteFontConfig, "url", url))
            newAppConfig.insert(std::make_pair("remote_font_config.url", url));
    }
    catch (const Poco::NullPointerException&)
    {
        LOG_INF("Overriding the remote font config URL failed because the remote_font_config "
                "entry does not exist");
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to fetch remote_font_config, please check JSON format: " << exc.what());
    }
}

void RemoteConfigPoll::fetchLockedTranslations(std::map<std::string, std::string>& newAppConfig,
                                               const Poco::JSON::Object::Ptr& remoteJson)
{
    try
    {
        Poco::JSON::Array::Ptr lockedTranslations;
        try
        {
            lockedTranslations = remoteJson->getObject("feature_locking")->getArray("translations");
        }
        catch (const Poco::NullPointerException&)
        {
            LOG_INF("Overriding translations failed because feature_locking->translations array "
                    "does not exist");
            return;
        }

        if (lockedTranslations.isNull() || lockedTranslations->size() == 0)
        {
            LOG_INF("Overriding feature_locking->translations failed because array is empty or "
                    "null");
            return;
        }

        std::size_t i;
        for (i = 0; i < lockedTranslations->size(); i++)
        {
            Poco::JSON::Object::Ptr translation = lockedTranslations->getObject(i);
            std::string language;
            //default values if the one of the entry is missing in json
            std::string title = _conf.getString("feature_lock.unlock_title", "");
            std::string description = _conf.getString("feature_lock.unlock_description", "");
            std::string writerHighlights =
                _conf.getString("feature_lock.writer_unlock_highlights", "");
            std::string impressHighlights =
                _conf.getString("feature_lock.impress_unlock_highlights", "");
            std::string calcHighlights = _conf.getString("feature_lock.calc_unlock_highlights", "");
            std::string drawHighlights = _conf.getString("feature_lock.draw_unlock_highlights", "");

            JsonUtil::findJSONValue(translation, "language", language);
            JsonUtil::findJSONValue(translation, "unlock_title", title);
            JsonUtil::findJSONValue(translation, "unlock_description", description);
            JsonUtil::findJSONValue(translation, "writer_unlock_highlights", writerHighlights);
            JsonUtil::findJSONValue(translation, "calc_unlock_highlights", calcHighlights);
            JsonUtil::findJSONValue(translation, "impress_unlock_highlights", impressHighlights);
            JsonUtil::findJSONValue(translation, "draw_unlock_highlights", drawHighlights);

            const std::string path =
                "feature_lock.translations.language[" + std::to_string(i) + ']';

            newAppConfig.insert(std::make_pair(path + "[@name]", language));
            newAppConfig.insert(std::make_pair(path + ".unlock_title", title));
            newAppConfig.insert(std::make_pair(path + ".unlock_description", description));
            newAppConfig.insert(
                std::make_pair(path + ".writer_unlock_highlights", writerHighlights));
            newAppConfig.insert(std::make_pair(path + ".calc_unlock_highlights", calcHighlights));
            newAppConfig.insert(
                std::make_pair(path + ".impress_unlock_highlights", impressHighlights));
            newAppConfig.insert(std::make_pair(path + ".draw_unlock_highlights", drawHighlights));
        }

        //if number of translations defined in configuration are greater than number of translation
        //fetched from json, overwrite the remaining translations from config file to empty strings
        for (;; i++)
        {
            const std::string path =
                "feature_lock.translations.language[" + std::to_string(i) + "][@name]";
            if (!_conf.has(path))
            {
                break;
            }
            newAppConfig.insert(std::make_pair(path, ""));
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to fetch feature_locking->translations, please check JSON format: "
                << exc.what());
    }
}

void RemoteConfigPoll::fetchUnlockImageUrl(std::map<std::string, std::string>& newAppConfig,
                                           const Poco::JSON::Object::Ptr& remoteJson)
{
    try
    {
        Poco::JSON::Object::Ptr featureLocking = remoteJson->getObject("feature_locking");

        std::string unlockImage;
        if (JsonUtil::findJSONValue(featureLocking, "unlock_image", unlockImage))
        {
            newAppConfig.insert(std::make_pair("feature_lock.unlock_image", unlockImage));
        }
    }
    catch (const Poco::NullPointerException&)
    {
        LOG_INF("Overriding unlock_image URL failed because the unlock_image entry does not "
                "exist");
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Failed to fetch unlock_image, please check JSON format: " << exc.what());
    }
}

void RemoteConfigPoll::fetchIndirectionEndpoint(std::map<std::string, std::string>& newAppConfig,
                                                const Poco::JSON::Object::Ptr& remoteJson)
{
    const std::string indrectionEndpointKey = "indirection_endpoint";
    if (!remoteJson || !remoteJson->isObject(indrectionEndpointKey))
    {
        LOG_DBG(indrectionEndpointKey << " doesn't exist, ignoring");
        return;
    }

    Poco::JSON::Object::Ptr indirectionEndpoint = remoteJson->getObject(indrectionEndpointKey);

    std::string url;
    if (JsonUtil::findJSONValue(indirectionEndpoint, "url", url))
        newAppConfig.insert(std::make_pair(indrectionEndpointKey + ".url", url));

    std::string serverName;
    if (JsonUtil::findJSONValue(indirectionEndpoint, indrectionEndpointKey + ".server_name",
                                serverName))
        newAppConfig.insert(std::make_pair(indrectionEndpointKey + ".server_name", serverName));

    const std::string geolocationKey = indrectionEndpointKey + ".geolocation_setup";
    if (!indirectionEndpoint->isObject("geolocation_setup"))
    {
        LOG_DBG(geolocationKey << " doesn't exist, ignoring");
        return;
    }

    Poco::JSON::Object::Ptr geolocationSetupObj =
        indirectionEndpoint->getObject("geolocation_setup");

    bool geolocationEnabled;
    if (JsonUtil::findJSONValue(geolocationSetupObj, "enable", geolocationEnabled))
        newAppConfig.insert(
            std::make_pair(geolocationKey + ".enable", std::to_string(geolocationEnabled)));

    std::string geolocationTimezone;
    if (JsonUtil::findJSONValue(geolocationSetupObj, "timezone", geolocationTimezone))
        newAppConfig.insert(std::make_pair(geolocationKey + ".timezone", geolocationTimezone));

    Poco::JSON::Array::Ptr allowedOrigins =
        geolocationSetupObj->getArray("allowed_websocket_origins");

    const std::string allowedWebsocketKey = geolocationKey + ".allowed_websocket_origins.origin";
    if (allowedOrigins.isNull() || allowedOrigins->size() == 0)
    {
        LOG_INF("Overriding " << allowedWebsocketKey << " failed because array is empty or null");
        return;
    }

    size_t i;
    for (i = 0; i < allowedOrigins->size(); i++)
    {
        newAppConfig.insert(std::make_pair(allowedWebsocketKey + '[' + std::to_string(i) + ']',
                                           allowedOrigins->get(i).toString()));
    }

    //if number of allowed_websocket_origins defined in configuration are greater than number of allowed_websocket_origins
    //fetched from json or if the number of monitors shrinks with new json,
    //overwrite the remaining allowed_websocket_origins from config file to empty strings
    bool haveMorePaths = true;
    do
    {
        const std::string path = allowedWebsocketKey + '[' + std::to_string(i) + ']';
        haveMorePaths = _conf.has(path);
        if (haveMorePaths)
            newAppConfig.insert(std::make_pair(path, ""));
        i++;
    } while (haveMorePaths);
}

void RemoteConfigPoll::fetchMonitors(std::map<std::string, std::string>& newAppConfig,
                                     const Poco::JSON::Object::Ptr& remoteJson)
{
    Poco::JSON::Array::Ptr monitors;
    try
    {
        monitors = remoteJson->getArray("monitors");
    }
    catch (const Poco::NullPointerException&)
    {
        LOG_INF("Overriding monitor failed because array "
                "does not exist");
        return;
    }

    if (monitors.isNull() || monitors->size() == 0)
    {
        LOG_INF("Overriding monitors failed because array is empty or "
                "null");
        return;
    }
    std::size_t i;
    for (i = 0; i < monitors->size(); i++)
        newAppConfig.insert(std::make_pair("monitors.monitor[" + std::to_string(i) + ']',
                                           monitors->get(i).toString()));

    //if number of monitors defined in configuration are greater than number of monitors
    //fetched from json or if the number of monitors shrinks with new json,
    //overwrite the remaining monitors from config file to empty strings
    for (;; i++)
    {
        const std::string path = "monitors.monitor[" + std::to_string(i) + ']';
        if (!_conf.has(path))
        {
            break;
        }
        newAppConfig.insert(std::make_pair(path, ""));
    }
}

void RemoteConfigPoll::handleOptions(const Poco::JSON::Object::Ptr& remoteJson)
{
    try
    {
        std::string buyProduct;
        JsonUtil::findJSONValue(remoteJson, "buy_product_url", buyProduct);
        Poco::URI buyProductUri(buyProduct);
        {
            std::lock_guard<std::mutex> lock(COOLWSD::RemoteConfigMutex);
            COOLWSD::BuyProductUrl = buyProductUri.toString();
        }
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("handleOptions: Exception " << exc.what());
    }
}

void RemoteFontConfigPoll::handleJSON(const Poco::JSON::Object::Ptr& remoteJson)
{
    // First mark all fonts we have downloaded previously as "inactive" to be able to check if
    // some font gets deleted from the list in the JSON file.
    for (auto& it : fonts)
        it.second.active = false;

    // Just pick up new fonts.
    auto fontsPtr = remoteJson->getArray("fonts");
    if (!fontsPtr)
    {
        LOG_WRN("The 'fonts' property does not exist or is not an array");
        return;
    }

    for (std::size_t i = 0; i < fontsPtr->size(); i++)
    {
        if (!fontsPtr->isObject(i))
            LOG_WRN("Element " << i << " in fonts array is not an object");
        else
        {
            const auto fontPtr = fontsPtr->getObject(i);
            const auto uriPtr = fontPtr->get("uri");
            if (uriPtr.isEmpty() || !uriPtr.isString())
                LOG_WRN("Element in fonts array does not have an 'uri' property or it is not a "
                        "string");
            else
            {
                const std::string uri = uriPtr.toString();
                const auto stampPtr = fontPtr->get("stamp");

                if (!stampPtr.isEmpty() && !stampPtr.isString())
                    LOG_WRN("Element in fonts array with uri '"
                            << uri << "' has a stamp property that is not a string, ignored");
                else if (fonts.count(uri) == 0)
                {
                    // First case: This font has not been downloaded.
                    if (!stampPtr.isEmpty())
                    {
                        if (downloadPlain(uri))
                        {
                            fonts[uri].stamp = stampPtr.toString();
                            fonts[uri].active = true;
                        }
                    }
                    else
                    {
                        if (downloadWithETag(uri, ""))
                        {
                            fonts[uri].active = true;
                        }
                    }
                }
                else if (!stampPtr.isEmpty() && stampPtr.toString() != fonts[uri].stamp)
                {
                    // Second case: Font has been downloaded already, has a "stamp" property,
                    // and that has been changed in the JSON since it was downloaded.
                    restartForKitAndReDownloadConfigFile();
                    break;
                }
                else if (!stampPtr.isEmpty())
                {
                    // Third case: Font has been downloaded already, has a "stamp" property, and
                    // that has *not* changed in the JSON since it was downloaded.
                    fonts[uri].active = true;
                }
                else
                {
                    // Last case: Font has been downloaded but does not have a "stamp" property.
                    // Use ETag.
                    if (!eTagUnchanged(uri, fonts[uri].eTag))
                    {
                        restartForKitAndReDownloadConfigFile();
                        break;
                    }
                    fonts[uri].active = true;
                }
            }
        }
    }

    // Any font that has been deleted from the JSON needs to be removed on this side, too.
    for (const auto& it : fonts)
    {
        if (!it.second.active)
        {
            LOG_DBG("Font no longer mentioned in the remote font config: " << it.first);
            restartForKitAndReDownloadConfigFile();
            break;
        }
    }
}

void RemoteFontConfigPoll::handleUnchangedJSON()
{
    // Iterate over the fonts that were mentioned in the JSON file when it was last downloaded.
    for (const auto& it : fonts)
    {
        // If the JSON has a "stamp" for the font, and we have already downloaded it, by
        // definition we don't need to do anything when the JSON file has not changed.
        if (it.second.stamp != "" && it.second.pathName != "")
            continue;

        // If the JSON has a "stamp" it must have been downloaded already. Should we even
        // assert() that?
        if (it.second.stamp != "" && it.second.pathName == "")
        {
            LOG_WRN("Font at " << it.first << " was not downloaded, should have been");
            continue;
        }

        // Otherwise use the ETag to check if the font file needs re-downloading.
        if (!eTagUnchanged(it.first, it.second.eTag))
        {
            restartForKitAndReDownloadConfigFile();
            break;
        }
    }
}

bool RemoteFontConfigPoll::downloadPlain(const std::string& uri)
{
    const Poco::URI fontUri{ uri };
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(fontUri));
    http::Request request(fontUri.getPathAndQuery());

    request.set("User-Agent", http::getAgentString());

    const std::shared_ptr<const http::Response> httpResponse = httpSession->syncRequest(request);

    return finishDownload(uri, httpResponse);
}

bool RemoteFontConfigPoll::eTagUnchanged(const std::string& uri, const std::string& oldETag)
{
    const Poco::URI fontUri{ uri };
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(fontUri));
    http::Request request(fontUri.getPathAndQuery());

    if (!oldETag.empty())
    {
        request.set("If-None-Match", oldETag);
    }

    request.set("User-Agent", http::getAgentString());

    const std::shared_ptr<const http::Response> httpResponse = httpSession->syncRequest(request);

    if (httpResponse->statusLine().statusCode() == http::StatusCode::NotModified)
    {
        LOG_DBG("Not modified since last time: " << uri);
        return true;
    }

    return false;
}

bool RemoteFontConfigPoll::downloadWithETag(const std::string& uri, const std::string& oldETag)
{
    const Poco::URI fontUri{ uri };
    std::shared_ptr<http::Session> httpSession(StorageConnectionManager::getHttpSession(fontUri));
    http::Request request(fontUri.getPathAndQuery());

    if (!oldETag.empty())
    {
        request.set("If-None-Match", oldETag);
    }

    request.set("User-Agent", http::getAgentString());

    const std::shared_ptr<const http::Response> httpResponse = httpSession->syncRequest(request);

    if (httpResponse->statusLine().statusCode() == http::StatusCode::NotModified)
    {
        LOG_DBG("Not modified since last time: " << uri);
        return true;
    }

    if (!finishDownload(uri, httpResponse))
        return false;

    fonts[uri].eTag = httpResponse->get("ETag");
    return true;
}

bool RemoteFontConfigPoll::finishDownload(const std::string& uri,
                                          const std::shared_ptr<const http::Response>& httpResponse)
{
    if (httpResponse->statusLine().statusCode() != http::StatusCode::OK)
    {
        LOG_WRN("Could not fetch " << uri);
        return false;
    }

    const std::string& body = httpResponse->getBody();

    // We intentionally use a new file name also when an updated version of a font is
    // downloaded. It causes trouble to rewrite the same file, in case it is in use in some Kit
    // process at the moment.

    // We don't remove the old file either as that also causes problems.

    // And in reality, it is a bit unclear how likely it even is that fonts downloaded through
    // this mechanism even will be updated.
    const std::string fontFile =
        COOLWSD::TmpFontDir + '/' + HexUtil::encodeId(Util::rng::getNext()) + ".ttf";

    std::ofstream fontStream(fontFile);
    fontStream.write(body.data(), body.size());
    if (!fontStream.good())
    {
        LOG_ERR("Could not write " << body.size() << " bytes to [" << fontFile << ']');
        return false;
    }

    LOG_DBG("Got " << body.size() << " bytes from [" << uri << "] and wrote to [" << fontFile
                   << ']');
    fonts[uri].pathName = fontFile;

    COOLWSD::sendMessageToForKit("addfont " + fontFile);

    COOLWSD::requestTerminateSpareKits();

    return true;
}

void RemoteFontConfigPoll::restartForKitAndReDownloadConfigFile()
{
    LOG_DBG("Downloaded font has been updated or a font has been removed. ForKit must be "
            "restarted.");
    fonts.clear();
    // Clear the saved ETag of the remote font configuration file so that it will be
    // re-downloaded, and all fonts mentioned in it re-downloaded and fed to ForKit.
    _eTagValue.clear();
    COOLWSD::sendMessageToForKit("exit");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
