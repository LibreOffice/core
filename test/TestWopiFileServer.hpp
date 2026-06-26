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

#pragma once

#include <wsd/Exceptions.hpp>
#include <wsd/FileServer.hpp>
#include <wsd/RequestDetails.hpp>

#include <Poco/Util/Application.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#if !defined(ENABLE_DEBUG) || ENABLE_DEBUG == 0
#error This file should be excluded from non-debug builds
#endif

namespace
{

// Represents basic file's attributes.
// Used for localFile
class LocalFileInfo
{
public:
    // Path and name identify the file. The size and the modified time are
    // read live from the filesystem on each request, so a file that changes
    // on disk while a session is open reports its real current state.
    std::string localPath;
    std::string fileName;

    enum class COOLStatusCode : std::uint16_t
    {
        DocChanged = 1010 // Document changed externally in storage
    };

    std::string getSize() const { return std::to_string(FileUtil::Stat(localPath).size()); }

    std::chrono::system_clock::time_point getLastModifiedTimepoint() const
    {
        return FileUtil::Stat(localPath).modifiedTimepoint();
    }

    std::string getLastModifiedTime() const
    {
        return Util::getIso8601FracformatTime(getLastModifiedTimepoint());
    }

    LocalFileInfo() = delete;
    LocalFileInfo(std::string lPath, std::string fName)
        : localPath(std::move(lPath))
        , fileName(std::move(fName))
    {
    }

private:
    // A handle to each known file, so repeated requests for the same path
    // share one object. It holds only the path and name. The size and the
    // modified time are always read live from the filesystem.
    static std::vector<std::shared_ptr<LocalFileInfo>> fileInfoVec;

public:
    // Lookup a file in our file-list
    static const std::shared_ptr<LocalFileInfo>& getOrCreateFile(const std::string& lpath,
                                                                 const std::string& fname)
    {
        const auto it = std::find_if(fileInfoVec.begin(), fileInfoVec.end(),
                                     [&lpath](const std::shared_ptr<LocalFileInfo>& obj)
                                     { return obj->localPath == lpath; });

        if (it != fileInfoVec.end())
            return *it;

        return fileInfoVec.emplace_back(std::make_shared<LocalFileInfo>(lpath, fname));
    }
};

std::atomic<unsigned> lastLocalId;
std::vector<std::shared_ptr<LocalFileInfo>> LocalFileInfo::fileInfoVec;

/// Reads the content of `path`, returns an empty string on failure.
std::string readFileToString(const std::string& path)
{
    if (!FileUtil::Stat(path).exists())
    {
        return {};
    }

    std::ifstream stream(path);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

// Maps a user id to its on-disk preset directory (relative to FileServerRoot).
// A real WOPI host keeps each user's settings (browser/view settings,
// dictionaries, ...) separate, so mirror that here: two debug users with
// different &userid values get different stores and a theme/view change by one
// cannot leak into another's. The default debug user (empty or "test") keeps
// the historical shared location, which the cypress fixtures seed and read
// directly, so existing tests are unaffected.
std::string userPresetDir(const std::string& userId)
{
    if (userId.empty() || userId == "test")
        return "test/data/presets/user";

    // Keep the directory name filesystem-safe; debug user ids are simple
    // tokens, so reduce anything else to '_'.
    std::string safe;
    safe.reserve(userId.size());
    for (const char c : userId)
    {
        const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                        (c >= 'A' && c <= 'Z') || c == '-' || c == '_';
        safe.push_back(ok ? c : '_');
    }
    return "test/data/presets/user/u-" + safe;
}

//handles request starts with /wopi/files
void handleWopiRequest(const Poco::Net::HTTPRequest& request, const RequestDetails& requestDetails,
                       std::istream& message, const std::shared_ptr<StreamSocket>& socket)
{
    Poco::URI requestUri(request.getURI());
    const Poco::Path path = requestUri.getPath();
    constexpr std::string_view prefix = "/wopi/files";
    constexpr std::string_view suffix = "/contents";
    std::string localPath;
    if (path.toString().ends_with(suffix))
    {
        localPath = path.toString().substr(prefix.length(), path.toString().length() -
                                                                prefix.length() - suffix.length());
    }
    else
    {
        localPath = path.toString().substr(prefix.length());
    }

    if (!FileUtil::Stat(localPath).exists())
    {
        LOG_ERR("Local file URI [" << localPath << "] invalid or doesn't exist.");
        throw BadRequestException("Invalid URI: " + localPath);
    }

    if (request.getMethod() == "GET" && !path.toString().ends_with(suffix))
    {
        const std::shared_ptr<LocalFileInfo>& localFile =
            LocalFileInfo::getOrCreateFile(localPath, path.getFileName());

        // Identity used for settings/owner comes from the request (see UserId
        // below); the per-connection counter only keeps friendly names distinct
        // for multiple views, which some multi-view tests rely on.
        std::string userNameString = "LocalUser#" + std::to_string(lastLocalId++);
        Poco::JSON::Object::Ptr fileInfo = new Poco::JSON::Object();

        Poco::JSON::Object::Ptr wopi = new Poco::JSON::Object();
        // If there is matching WOPI data next to the file to be loaded, use it.
        std::string wopiString = readFileToString(localPath + ".wopi.json");
        if (!wopiString.empty())
        {
            if (JsonUtil::parseJSON(wopiString, wopi))
            {
                fileInfo = std::move(wopi);
            }
        }

        std::string postMessageOrigin;
        ConfigUtil::isSslEnabled() ? postMessageOrigin = "https://" : postMessageOrigin = "http://";
        postMessageOrigin += requestDetails.getHostUntrusted();

        fileInfo->set("BaseFileName", localFile->fileName);
        fileInfo->set("Size", localFile->getSize());
        fileInfo->set("Version", "1.0");
        fileInfo->set("OwnerId", "test");
        // The WOPI host identifies the user; mirror that here by taking the user
        // id from the request (debug.html sends &userid on the WOPISrc; a real
        // host resolves it from the access token). Same userid across tabs/reloads
        // is the same user, different userid is a different user. Falling back to
        // "test" keeps the default debug user matching OwnerId so it stays the
        // owner (owner-only actions such as the document-properties password
        // option require UserId == OwnerId).
        std::string userId = requestDetails.getParam("userid");
        if (userId.empty())
            userId = "test";
        fileInfo->set("UserId", userId);
        fileInfo->set("UserFriendlyName", userNameString);

        //allow &configid to override etag to force another subforkit
        std::string configId = requestDetails.getParam("configid");
        if (configId.empty())
            configId = "default";

        std::string etagString = "\"" COOLWSD_VERSION_HASH +
                                 ConfigUtil::getString("ver_suffix", "") + '-' + configId + "\"";

        // authentication token to get these settings??
        {
            Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
            std::string uri = COOLWSD::getServerURL() + "/wopi/settings/sharedconfig.json";
            sharedSettings->set("uri", Util::trim(uri));
            sharedSettings->set("stamp", etagString);
            fileInfo->set("SharedSettings", sharedSettings);
        }

#if defined(BUILDING_TESTS)
        // Cypress tests both assume that tests start in the default config, e.g.
        // spell checking and sidebar enabled, and some tests assume they can override
        // features by changing localStorage before loading a document.
        //
        // A test that needs real per-user settings persistence (e.g. the theme
        // isolation/reload tests) opts in by passing an explicit &userid: that
        // routes it to the full userconfig with its own per-user store (see
        // userPresetDir), so its browser settings are saved and served back on
        // reload. The default user (no userid -> "test") keeps the historical
        // cypressuserconfig.json, so the many tests that don't ask for
        // persistence are unaffected.
        const bool persistingTestUser =
            !userId.empty() && userId != "test";
        const bool cypressUserConfig =
            localPath.find("cypress_test") != std::string::npos && !persistingTestUser;
#else
        const bool cypressUserConfig(false);
#endif

        {
            Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
            std::string userConfig = !cypressUserConfig ? "/wopi/settings/userconfig.json"
                                                        : "/wopi/settings/cypressuserconfig.json";
            std::string uri = COOLWSD::getServerURL() + userConfig;
            Util::trim(uri);
            // Carry the user id so the settings handler can serve this user's
            // own preset store (see userPresetDir). It survives the round trip:
            // DocumentBroker fetches UserSettings.uri via getPathAndQuery().
            uri += "?userid=" + userId;
            userSettings->set("uri", uri);
            userSettings->set("stamp", etagString);
            fileInfo->set("UserSettings", userSettings);
        }

        fileInfo->set("UserCanWrite",
                      (requestDetails.getParam("permission") != "readonly") ? true : false);
        fileInfo->set("PostMessageOrigin", postMessageOrigin);
        fileInfo->set("LastModifiedTime", localFile->getLastModifiedTime());
        fileInfo->set("EnableOwnerTermination", true);

        std::ostringstream jsonStream;
        fileInfo->stringify(jsonStream);

        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        httpResponse.set("Last-Modified", Util::getHttpTime(localFile->getLastModifiedTimepoint()));
        httpResponse.setBody(jsonStream.str(), "application/json; charset=utf-8");
        socket->send(httpResponse);

        return;
    }

    if (request.getMethod() == "GET" && path.toString().ends_with(suffix))
    {
        const std::shared_ptr<LocalFileInfo>& localFile =
            LocalFileInfo::getOrCreateFile(localPath, path.getFileName());
        std::ostringstream ss;
        std::ifstream inputFile(localFile->localPath);
        ss << inputFile.rdbuf();

        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        httpResponse.set("Last-Modified", Util::getHttpTime(localFile->getLastModifiedTimepoint()));
        httpResponse.setBody(ss.str(), "text/plain; charset=utf-8");
        socket->send(httpResponse);
        return;
    }

    if (request.getMethod() == "POST" && path.toString().ends_with(suffix))
    {
        const std::shared_ptr<LocalFileInfo>& localFile =
            LocalFileInfo::getOrCreateFile(localPath, path.getFileName());
        std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());
        if (wopiTimestamp.empty())
            wopiTimestamp = request.get("X-LOOL-WOPI-Timestamp", std::string());

        if (!wopiTimestamp.empty())
        {
            if (wopiTimestamp != localFile->getLastModifiedTime())
            {
                http::Response httpResponse(http::StatusCode::Conflict);
                httpResponse.setBody("{\"COOLStatusCode\":" +
                                         std::to_string(static_cast<int>(
                                             LocalFileInfo::COOLStatusCode::DocChanged)) +
                                     ",\"stamp\": \"" + localFile->getLastModifiedTime() + "\"" +
                                     '}',
                                     "application/json; charset=utf-8");
                socket->send(httpResponse);
                return;
            }
        }

        std::ofstream outfile(localFile->localPath, std::ofstream::binary);
        std::copy_n(std::istreambuf_iterator<char>(message), request.getContentLength(),
                    std::ostreambuf_iterator<char>(outfile));
        outfile.close();

        // Report the modified time the file now has on disk, so the next
        // upload sends it back and matches.
        std::string body = R"({"LastModifiedTime": ")" + localFile->getLastModifiedTime() + "\" }";
        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        httpResponse.setBody(std::move(body), "application/json; charset=utf-8");
        socket->send(httpResponse);
        return;
    }
}

// pair consist of type and path of the item which usually resides test/data folder
using asset = std::pair<std::string, std::string>;

//handle requests for settings.json contents
void handlePresetRequest(const std::string& kind, const std::string& etagString,
                         const std::string& prefix, const std::shared_ptr<StreamSocket>& socket,
                         const std::vector<asset>& items, bool serveBrowserSetttings,
                         const std::string& unittest, const std::string& userId)
{
    Poco::JSON::Object::Ptr configInfo = new Poco::JSON::Object();
    configInfo->set("kind", kind);

    std::string fwd = COOLWSD::FileServerRoot;

    Poco::JSON::Array::Ptr configAutoTexts = new Poco::JSON::Array();
    Poco::JSON::Array::Ptr configDictionaries = new Poco::JSON::Array();
    Poco::JSON::Array::Ptr configXcu = new Poco::JSON::Array();
    Poco::JSON::Array::Ptr configTemplate = new Poco::JSON::Array();

    UnitWSD* const unitWsd = UnitWSD::isUnitTesting() ? &UnitWSD::get() : nullptr;
    for (const auto& item : items)
    {
        char queryDelim = '?';
        Poco::JSON::Object::Ptr configEntry = new Poco::JSON::Object();
        // There are two ways in practice that real integrations provide resources:
        // Most typical is some url like: settings/path/to/something.dic
        // Less typically is like: settings/path/to?file_name=something.dic
        // For testing, use the first for 'shared' and the second for 'user'
        std::string uri = COOLWSD::getServerURL().append(prefix + fwd);
        //COOLWSD::getServerURL tediously includes spaces at the start
        Util::trim(uri);
        if (kind == "shared")
            uri += item.second;
        else
        {
            Poco::Path filePath(item.second);
            uri += filePath.parent().toString();
            const std::string& fileName = filePath.getFileName();
            uri += "?file_name=" + fileName;
            queryDelim = '&';
        }
        //COOLWSD::getServerURL tediously includes spaces at the start
        Util::trim(uri);
        if (!unittest.empty())
            uri.append(1, queryDelim).append("testname=").append(unittest);
        if (unitWsd)
            unitWsd->filterRegisterPresetAsset(uri);
        configEntry->set("uri", uri);
        configEntry->set("stamp", etagString);
        if (item.first == "autotext")
            configAutoTexts->add(configEntry);
        else if (item.first == "wordbook")
            configDictionaries->add(configEntry);
        else if (item.first == "xcu")
            configXcu->add(configEntry);
        else if (item.first == "template")
            configTemplate->add(configEntry);
    }
    configInfo->set("autotext", configAutoTexts);
    configInfo->set("wordbook", configDictionaries);
    configInfo->set("xcu", configXcu);
    configInfo->set("template", configTemplate);

    if (serveBrowserSetttings)
    {
        assert(kind == "user");
        // browsersetting.json and viewsetting.json are mutable: the client
        // re-uploads them at runtime (e.g. a changed view mode). Their stamp
        // must therefore change with the file content.
        const std::string browserSettingPath =
            COOLWSD::FileServerRoot + userPresetDir(userId) + "/browsersetting.json";
        const FileUtil::Stat browserSettingStat(browserSettingPath);
        if (browserSettingStat.exists())
        {
            Poco::JSON::Array::Ptr browsersettingArray = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr configEntry = new Poco::JSON::Object();
            std::string uri = COOLWSD::getServerURL().append(prefix + browserSettingPath);
            Util::trim(uri);
            configEntry->set("uri", uri);
            configEntry->set("stamp", etagString + '-' +
                                         std::to_string(browserSettingStat.modifiedTimeUs()));
            browsersettingArray->add(configEntry);
            configInfo->set("browsersetting", browsersettingArray);
        }
        else
        {
            LOG_WRN("preset file [" << browserSettingPath << "] doesn't exist");
        }

        const std::string viewSettingPath =
            COOLWSD::FileServerRoot + userPresetDir(userId) + "/viewsetting.json";
        const FileUtil::Stat viewSettingStat(viewSettingPath);
        if (viewSettingStat.exists())
        {
            Poco::JSON::Array::Ptr viewsettingArray = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr configEntry = new Poco::JSON::Object();
            std::string uri = COOLWSD::getServerURL().append(prefix + viewSettingPath);
            Util::trim(uri);
            configEntry->set("uri", uri);
            configEntry->set("stamp", etagString + '-' +
                                          std::to_string(viewSettingStat.modifiedTimeUs()));
            viewsettingArray->add(configEntry);
            configInfo->set("viewsetting", viewsettingArray);
        }
        else
        {
            LOG_WRN("preset file [" << viewSettingPath << "] doesn't exist");
        }
    }

    std::ostringstream jsonStream;
    configInfo->stringify(jsonStream);

    http::Response httpResponse(http::StatusCode::OK);
    FileServerRequestHandler::hstsHeaders(httpResponse);
    httpResponse.set("Last-Modified", Util::getHttpTime(std::chrono::system_clock::now()));
    httpResponse.setBody(jsonStream.str(), "application/json; charset=utf-8");
    socket->send(httpResponse);
}

enum class PresetType : std::uint8_t
{
    Shared,
    User,
};

// search for presets file in test/data/presets directory
static std::vector<asset> getAssetVec(PresetType type, const std::string& userId = std::string())
{
    std::string searchDir = "test/data/presets";
    std::vector<asset> assetVec;
    if (!FileUtil::Stat(Poco::Path(COOLWSD::FileServerRoot, searchDir).toString()).exists())
    {
        LOG_ERR("preset directory[" << searchDir << "] doesn't exist");
        return assetVec;
    }

    if (type == PresetType::Shared)
        searchDir.append("/shared");
    else if (type == PresetType::User)
        searchDir = userPresetDir(userId);

    auto searchInDir = [&assetVec](const std::string& directory)
    {
        const auto fileNames =
            FileUtil::getDirEntries(Poco::Path(COOLWSD::FileServerRoot, directory).toString());
        for (const auto& fileName : fileNames)
        {
            const std::string ext = FileUtil::extractFileExtension(fileName);
            if (ext.empty())
                continue;
            std::string filePath = '/' + directory + '/';
            filePath.append(fileName);
            if (ext == "bau")
                assetVec.push_back(asset("autotext", filePath));
            else if (ext == "dic")
                assetVec.push_back(asset("wordbook", filePath));
            else if (ext == "xcu")
                assetVec.push_back(asset("xcu", filePath));
            else if (ext == "otp")
                assetVec.push_back(asset("template", filePath));
            LOG_TRC("Found preset file[" << filePath << ']');
        }
    };

    LOG_DBG("Looking for preset files in directory[" << searchDir << ']');
    searchInDir(searchDir);
    return assetVec;
}

//handles request starts with /wopi/settings
void handleSettingsRequest(const Poco::Net::HTTPRequest& request, const std::string& etagString,
                           std::istream& message, const std::shared_ptr<StreamSocket>& socket)
{
    Poco::URI requestUri(request.getURI());
    const Poco::Path path = requestUri.getPath();
    const std::string prefix = "/wopi/settings";
    std::string configPath = path.toString().substr(prefix.length());

    // When called via fetch-settings-config, the type is passed as a query
    // parameter and the path has no config filename suffix.  Map the type
    // parameter to the corresponding config path so the existing handler
    // logic works for both the direct and proxied request styles.
    if (configPath.empty() || configPath == "/")
    {
        const auto params = requestUri.getQueryParameters();
        for (const auto& param : params)
        {
            if (param.first == "type")
            {
                if (param.second == "userconfig")
                    configPath = "/userconfig.json";
                else if (param.second == "systemconfig")
                    configPath = "/sharedconfig.json";
                break;
            }
        }
    }

    if (request.getMethod() == "GET" && configPath.ends_with("config.json"))
    {
        // For unittests ensure there is a testname="whatever" on these responses
        std::string unittest;
        const auto params = requestUri.getQueryParameters();
        const auto testnameIt = std::find_if(params.begin(), params.end(),
                                             [](const std::pair<std::string, std::string>& pair)
                                             { return pair.first == "testname"; });
        if (testnameIt != params.end())
            unittest = testnameIt->second;

        if (configPath == "/userconfig.json" || configPath == "/cypressuserconfig.json")
        {
            const bool serveBrowerSettings = configPath != "/cypressuserconfig.json";
            // Route to the requesting user's own preset store, except for the
            // cypress config, which must keep using the shared fixture dir its
            // tests seed and read directly.
            std::string presetUser;
            if (serveBrowerSettings)
            {
                const auto useridIt =
                    std::find_if(params.begin(), params.end(),
                                 [](const std::pair<std::string, std::string>& pair)
                                 { return pair.first == "userid"; });
                if (useridIt != params.end())
                    presetUser = useridIt->second;
            }
            auto items = getAssetVec(PresetType::User, presetUser);
            handlePresetRequest("user", etagString, prefix, socket, items, serveBrowerSettings,
                                unittest, presetUser);
        }
        else if (configPath == "/sharedconfig.json")
        {
            auto items = getAssetVec(PresetType::Shared);
            handlePresetRequest("shared", etagString, prefix, socket, items, false, unittest,
                                std::string());
        }
        else
            throw BadRequestException("Invalid Config Request: " + configPath);
    }
    else if (request.getMethod() == "GET")
    {
        // See handlePresetRequest, for testing purposes this debug mode settings
        // code supports the two typical ways settings files tend to be supplied
        const Poco::URI::QueryParameters params = requestUri.getQueryParameters();
        std::string fileName;
        for (const auto& param : params)
        {
            if (param.first == "file_name")
                fileName = param.second;
        }

        if (!fileName.empty())
            configPath += fileName;

        if (!FileUtil::Stat(configPath).exists())
        {
            LOG_ERR("Local file URI [" << configPath << "] invalid or doesn't exist.");
            throw BadRequestException("Invalid URI: " + configPath);
        }

        const std::shared_ptr<LocalFileInfo>& localFile =
            LocalFileInfo::getOrCreateFile(configPath, path.getFileName());
        std::ostringstream ss;
        std::ifstream inputFile(localFile->localPath);
        ss << inputFile.rdbuf();

        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        httpResponse.set("Last-Modified", Util::getHttpTime(localFile->getLastModifiedTimepoint()));
        httpResponse.setBody(ss.str(), "text/plain; charset=utf-8");
        socket->send(httpResponse);
    }
    else if (request.getMethod() == "POST")
    {
        const Poco::URI::QueryParameters params = requestUri.getQueryParameters();
        std::string filePath;
        for (const auto& param : params)
        {
            if (param.first == "fileId")
                filePath = param.second;
        }

        // executed when file uploaded from ClientSession
        if (!filePath.empty())
        {
            std::streamsize size = request.getContentLength();
            if (size == 0)
            {
                http::Response httpResponse(http::StatusCode::BadRequest);
                socket->send(httpResponse);
                LOG_ERR("Failed to save the file, file content doesn't exist");
                return;
            }

            std::vector<std::string> splitStr = Util::splitStringToVector(filePath, '/');
            if (splitStr.size() != 4)
            {
                http::Response httpResponse(http::StatusCode::BadRequest);
                socket->send(httpResponse);
                LOG_ERR("Failed to save the file, invalid filPath[" << filePath << ']');
                return;
            }
            // ignoring category for local wopiserver
            const std::string& type = splitStr[1];
            const std::string& fileName = splitStr[3];

            // The user id (carried on the upload URI via _uriPublic, preserved
            // by getPresetUploadBaseUrl) selects this user's own store, so the
            // GET listing reads back what this user uploaded (see userPresetDir).
            std::string userId;
            for (const auto& param : params)
            {
                if (param.first == "userid")
                    userId = param.second;
            }

            // Write under FileServerRoot so the location matches where the
            // GET/index handlers read presets from (they use
            // COOLWSD::FileServerRoot + "test/data/presets/..."). A relative
            // path would resolve against coolwsd's CWD, which differs from
            // FileServerRoot under cypress (CWD is cypress_test/), so uploaded
            // settings would be written somewhere they are never read back.
            std::string dirPath = COOLWSD::FileServerRoot;
            if (type == "userconfig")
                dirPath += userPresetDir(userId);
            else if (type == "systemconfig")
                dirPath += "test/data/presets/shared";

            // The default cypress user (no &userid -> "test") must not persist
            // its browser settings: tests rely on a clean starting config, and a
            // persisted file (e.g. a recent color picked by one test) would leak
            // into later tests and runs. Only a test that opted in with an
            // explicit &userid (any value, including 1) gets its own persisted
            // store. Skip the write for the default user but still respond OK so
            // the client's upload does not error. Shared/systemconfig is
            // unaffected.
            const bool skipPersist =
                (type == "userconfig" && (userId.empty() || userId == "test"));
            if (!skipPersist)
            {
                Poco::File(dirPath).createDirectories();

                LOG_DBG("Saving uploaded file [" << fileName << "] to directory [" << dirPath << ']');
                dirPath.push_back('/');
                dirPath.append(fileName);

                std::ofstream outfile(dirPath, std::ofstream::binary);
                std::copy_n(std::istreambuf_iterator<char>(message), size,
                            std::ostreambuf_iterator<char>(outfile));
                outfile.close();
            }

            std::string timestamp =
                Util::getIso8601FracformatTime(std::chrono::system_clock::now());
            std::string body = R"({"LastModifiedTime": ")" + timestamp + "\" }";
            http::Response httpResponse(http::StatusCode::OK);
            FileServerRequestHandler::hstsHeaders(httpResponse);
            httpResponse.setBody(std::move(body), "application/json; charset=utf-8");
            socket->send(httpResponse);
            return;
        }

        // executed when file uploaded from admin panel
        // TODO: we don't need the following code once we fix the requesting to own server stuck problem
        FilePartHandler partHandler;
        Poco::Net::HTMLForm form(request, message, partHandler);
        const std::string& fileName = partHandler.getFileName();
        const std::string& uploadedFilePath = partHandler.getFilePath();

        if (fileName.empty() || uploadedFilePath.empty())
        {
            http::Response httpResponse(http::StatusCode::BadRequest);
            socket->send(httpResponse);
            LOG_ERR("No valid file uploaded.");
        }

        LOG_INF("File uploaded: " << fileName << " from [" << uploadedFilePath << ']');

        // TODO: hardcoded save to shared directory, add support for user directory
        // when adminIntegratorSettings allow it
        const std::string testSharedDir = COOLWSD::FileServerRoot + "test/data/presets/shared";
        Poco::File(testSharedDir).createDirectories();
        LOG_DBG("Saving uploaded file[" << fileName << "] to directory[" << testSharedDir << ']');

        FileUtil::copyFileTo(uploadedFilePath, testSharedDir + '/' + fileName);

        std::string timestamp = Util::getIso8601FracformatTime(std::chrono::system_clock::now());
        std::string body = R"({"LastModifiedTime": ")" + timestamp + "\" }";
        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        httpResponse.setBody(std::move(body), "application/json; charset=utf-8");
        socket->send(httpResponse);
    }
    else if (request.getMethod() == "DELETE")
    {
        const Poco::URI::QueryParameters params = requestUri.getQueryParameters();
        std::string fileId;
        for (const auto& param : params)
        {
            if (param.first == "fileId")
                fileId = param.second;
        }

        if (fileId.empty())
        {
            http::Response httpResponse(http::StatusCode::BadRequest);
            socket->send(httpResponse);
            LOG_ERR("Failed to delete the file, missing fileId parameter");
            return;
        }

        std::vector<std::string> splitStr = Util::splitStringToVector(fileId, '/');
        if (splitStr.size() != 4)
        {
            http::Response httpResponse(http::StatusCode::BadRequest);
            socket->send(httpResponse);
            LOG_ERR("Failed to delete the file, invalid fileId[" << fileId << ']');
            return;
        }

        const std::string& type = splitStr[1];
        const std::string& fileName = splitStr[3];

        std::string userId;
        for (const auto& param : params)
        {
            if (param.first == "userid")
                userId = param.second;
        }

        // Match the read/upload path (COOLWSD::FileServerRoot-based, keyed by
        // user id); see the upload handler above for why a relative path is
        // wrong under cypress.
        std::string dirPath = COOLWSD::FileServerRoot;
        if (type == "userconfig")
            dirPath += userPresetDir(userId);
        else if (type == "systemconfig")
            dirPath += "test/data/presets/shared";

        dirPath.push_back('/');
        dirPath.append(fileName);

        Poco::File file(dirPath);
        if (file.exists())
        {
            file.remove();
            LOG_DBG("Deleted preset file [" << dirPath << ']');
        }
        else
        {
            LOG_WRN("Preset file [" << dirPath << "] doesn't exist, nothing to delete");
        }

        http::Response httpResponse(http::StatusCode::OK);
        FileServerRequestHandler::hstsHeaders(httpResponse);
        socket->send(httpResponse);
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
