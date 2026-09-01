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
 * Unit test for user preset functionality.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <atomic>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

using namespace std::literals;

/// This test ensures that a document which has presets, but whose load is
/// canceled before the presets are installed, gracefully handles the case that
/// the document broker poll no longer exists when the response from async dns
/// arrives and the preset download attempt cannot be attached to the dead
/// poll.
class UnitEarlyDocDeath : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitDocPresetsInstallStart, DocPresetsInstallStart, WaitDocClose, ResumeDNS, Finish, Done) _phase;

    std::mutex _dns_mutex;
    std::condition_variable _dns_cv;
    bool _dnsResumed = false;

public:
    UnitEarlyDocDeath()
        : Base("UnitEarlyDocDeath")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Set to 0 to immediately discard any unused subforkits
        config.setUInt("serverside_config.idle_timeout_secs", 0);
    }

    // replace the preset asset uri so dns requests for them can
    // be identified when they are queried so we can delay their
    // resolution until the document load is abandoned.
    void filterRegisterPresetAsset(std::string& uri) override
    {
        TST_LOG("filterRegisterPresetAsset before: " << uri);
        uri = Util::replace(uri, "localhost", "presetasset");
        TST_LOG("filterRegisterPresetAsset after: " << uri);
    }

    // delay the resolution of these queries so we can cancel
    // the document load and let the dns complete when the
    // document has cancelled to test we don't crash under
    // this circumstance
    void filterResolveDNS(std::string& query) override
    {
        if (query == "presetasset")
        {
            query = "localhost";
            std::unique_lock<std::mutex> lock(_dns_mutex);
            // Block only the first preset-asset lookup, until the doc broker is
            // destroyed. There are several preset assets, so more lookups arrive
            // later; once resumed we must never block again, or a straggler would
            // re-stall the shared asyncdns thread and hang the next unit test.
            if (!_dnsResumed)
            {
                TST_LOG(
                    "delaying dns resolution of preset host until document broker is destroyed");
                _dns_cv.wait(lock, [this]() { return _dnsResumed; });
                TST_LOG("dns resumed after doc broker destruction");
                if (_phase == Phase::ResumeDNS)
                {
                    TRANSITION_STATE(_phase, Phase::Finish);
                }
            }
        }
    }

    // as soon as presets install starts, then cancel the load of the
    // document.
    void onDocBrokerPresetsInstallStart() override
    {
        TST_LOG("onDocBrokerPresetsInstallStart");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocPresetsInstallStart);
        TRANSITION_STATE(_phase, Phase::DocPresetsInstallStart);
        SocketPoll::wakeupWorld();
    }

    // presets install is delayed until document should be destroyed
    void onDocBrokerPresetsInstallEnd(bool /*success*/) override
    {
        failTest("Document should be destroyed before presets are installed");
    }

    void onDocBrokerDestroy(const std::string& /*docKey*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        TRANSITION_STATE(_phase, Phase::ResumeDNS);
        TST_LOG("resume dns resolution after doc broker was destroyed");
        {
            std::unique_lock<std::mutex> lock(_dns_mutex);
            _dnsResumed = true;
        }
        _dns_cv.notify_all();
        SocketPoll::wakeupWorld();
    }

    // document shouldn't get loaded until presets are installed,
    // and we delay preset installation via stalled dns lookup
    // and cancel the load, so the doc should never get loaded
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        failTest("Document should not get loaded.");
        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const Poco::URI uriReq(request.getURI());
        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() + "/wopi/settings/userconfig.json?testname=UnitEarlyDocDeath";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", "something");
        fileInfo->set("UserSettings", userSettings);
    }

    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& request,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& /*socket*/) override
    {
        std::string uri = Uri::decode(request.getURI());
        TST_LOG("parallelizeCheckInfo requested: " << uri);
        return std::map<std::string, std::string>{
            {"wopiSrc", "/wopi/files/0"},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", ""}
        };
    }

    // on loading this document, a new subforkit is needed, so that should
    // be created on demand, and then the document loaded via that subforkit
    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitDocPresetsInstallStart);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::DocPresetsInstallStart:
                TRANSITION_STATE(_phase, Phase::WaitDocClose);
                TST_LOG("Close document just after preset install starts");
                WSD_CMD_BY_CONNECTION_INDEX(0, "closedocument");
                break;
            case Phase::WaitDocPresetsInstallStart:
            case Phase::WaitDocClose:
            case Phase::ResumeDNS:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
            case Phase::Finish:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("Document load successfully abandoned");
            }
        }
    }
};

/// Advertises a single SPIF security-label policy in the UserSettings "spif"
/// group, serves it, and asserts the install machinery fetched it (so the host's
/// spif group is delivered into the jail's user config dir like the other groups).
class UnitSpifPreset : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitInstall, Done) _phase;

    bool _spifAssetRequested = false;

public:
    UnitSpifPreset()
        : Base("UnitSpifPreset")
        , _phase(Phase::Load)
    {
    }

    // Point the document's UserSettings at our settings JSON.
    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri =
            helpers::getTestServerURI() + "/wopi/settings/userconfig.json?testname=UnitSpifPreset";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", "spifstamp");
        fileInfo->set("UserSettings", userSettings);
    }

    // Serve the settings JSON (advertising the spif group) and the policy asset.
    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        const std::string path = uriReq.getPath();

        if (path == "/wopi/settings/userconfig.json")
        {
            const std::string assetUri =
                helpers::getTestServerURI()
                + "/wopi/settings/systemconfig/spif/spif-test.xml?testname=UnitSpifPreset";
            const std::string json =
                "{\"kind\":\"user\",\"spif\":[{\"uri\":\"" + assetUri + "\",\"stamp\":\"s1\"}]}";
            TST_LOG("Serving userconfig.json: " << json);
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(json, "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        if (path.find("/spif/") != std::string::npos)
        {
            _spifAssetRequested = true;
            TST_LOG("Serving SPIF policy asset: " << path);
            static constexpr auto spif =
                "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                "<spif:SPIF xmlns:spif=\"http://www.xmlspif.org/spif\" schemaVersion=\"1.0\" "
                "version=\"1\"><spif:securityPolicyId name=\"Test\" id=\"1.2.3\" />"
                "</spif:SPIF>"sv;
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(std::string(spif), "application/xml; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        return Base::handleHttpGetRequest(request, socket);
    }

    void onDocBrokerPresetsInstallEnd(bool success) override
    {
        TST_LOG("onDocBrokerPresetsInstallEnd: success=" << success);
        LOK_ASSERT_STATE(_phase, Phase::WaitInstall);
        LOK_ASSERT_MESSAGE("preset install should succeed", success);
        LOK_ASSERT_MESSAGE("the spif policy asset should have been fetched", _spifAssetRequested);
        TRANSITION_STATE(_phase, Phase::Done);
        passTest("spif preset group installed");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitInstall);
                TST_LOG("Creating connection and loading view");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitInstall:
            case Phase::Done:
                break;
        }
    }
};

/// Two people open the same document, each with settings of their own.
///
/// Both users' settings are fetched, because the browser settings are per
/// session, but the presets are installed once: one document is one kit and
/// one configuration, and it is the first user's. The second user's dialog
/// still shows what they saved, because it reads that back from the host
/// rather than from the kit, so their setting looks applied while the
/// document goes on checking with the first user's.
class UnitSecondUserPresets : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitFirstInstall, SecondView, WaitSecondView, Done) _phase;

    /// The users whose settings the server was asked for.
    std::set<std::string> _settingsAsked;
    int _installs = 0;
    int _viewsLoaded = 0;
    /// The userpresetsapplied answers the clients were given.
    std::vector<std::string> _presetsApplied;
    /// And the documentsettingslive ones, which change as people arrive.
    std::vector<std::string> _settingsLive;

public:
    UnitSecondUserPresets()
        : Base("UnitSecondUserPresets")
        , _phase(Phase::Load)
    {
    }

    static std::string userOf(const Poco::URI& uri, const std::string& name)
    {
        for (const auto& parameter : uri.getQueryParameters())
        {
            if (parameter.first == name)
                return parameter.second;
        }
        return std::string();
    }

    /// The access token names the user, so the two views are two people, each
    /// with a settings store of their own.
    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const std::string user = userOf(Poco::URI(request.getURI()), "access_token");
        fileInfo->set("UserId", user);
        fileInfo->set("UserFriendlyName", user);

        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() + "/wopi/settings/userconfig.json?user="
                          + user + "&testname=UnitSecondUserPresets";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", user);
        fileInfo->set("UserSettings", userSettings);
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (uriReq.getPath() == "/wopi/settings/userconfig.json")
        {
            const std::string user = userOf(uriReq, "user");
            TST_LOG("Settings asked for user [" << user << ']');
            _settingsAsked.insert(user);

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody("{\"kind\":\"user\"}", "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        return Base::handleHttpGetRequest(request, socket);
    }

    /// The dialog is told whether the document is running with this user's
    /// own settings, so it can say that a change will only show in the next
    /// document rather than leave them wondering.
    bool onFilterSendWebSocketMessage(std::string_view data, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        constexpr std::string_view prefix = "userpresetsapplied: ";
        if (data.rfind(prefix, 0) == 0)
        {
            const std::string value(data.substr(prefix.size()));
            TST_LOG("Client told userpresetsapplied: " << value);
            _presetsApplied.push_back(value);
        }

        constexpr std::string_view livePrefix = "documentsettingslive: ";
        if (data.rfind(livePrefix, 0) == 0)
        {
            const std::string value(data.substr(livePrefix.size()));
            TST_LOG("Client told documentsettingslive: " << value);
            _settingsLive.push_back(value);
        }
        return false;
    }

    void onDocBrokerPresetsInstallEnd(bool success) override
    {
        ++_installs;
        TST_LOG("onDocBrokerPresetsInstallEnd: success=" << success
                                                         << " install #" << _installs);
        LOK_ASSERT_MESSAGE("the presets should install", success);
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>&) override
    {
        ++_viewsLoaded;
        TST_LOG("onDocBrokerViewLoaded: " << _viewsLoaded << " view(s), "
                                          << _installs << " preset install(s)");

        if (_viewsLoaded == 1)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFirstInstall);
            LOK_ASSERT_EQUAL(1, _installs);
            TRANSITION_STATE(_phase, Phase::SecondView);
            return;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSecondView);

        // A document does not load until its presets are installed, so by the
        // time the second view is up a second install would have finished.
        // There is only ever one: the configuration this document checks with
        // is the one the first user brought.
        LOK_ASSERT_EQUAL(1, _installs);

        // Both users' settings were fetched all the same - the browser
        // settings are per session - so an untouched second user's store is
        // not what makes their options do nothing.
        LOK_ASSERT_MESSAGE("both users' settings should have been fetched",
                           _settingsAsked.count("first") == 1
                               && _settingsAsked.count("second") == 1);

        // And each is told which of the two they are, so the dialog can say
        // what a change will do.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), _presetsApplied.size());
        LOK_ASSERT_EQUAL(std::string("true"), _presetsApplied[0]);
        LOK_ASSERT_EQUAL(std::string("false"), _presetsApplied[1]);

        // Alone, the first user's change would have been felt here; once the
        // second arrives it would not, and both are told so.
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), _settingsLive.size());
        LOK_ASSERT_EQUAL(std::string("true"), _settingsLive[0]);
        LOK_ASSERT_EQUAL(std::string("false"), _settingsLive[1]);
        LOK_ASSERT_EQUAL(std::string("false"), _settingsLive[2]);

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("the second user's presets are not installed");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitFirstInstall);
                TST_LOG("First user opens the document");
                initWebsocket("/wopi/files/0?access_token=first");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::SecondView:
            {
                TRANSITION_STATE(_phase, Phase::WaitSecondView);
                TST_LOG("Second user joins the same document");
                // A connection of their own, with their own token: the doc key
                // is the WOPISrc path, so the token names the user without
                // making it a different document. initWebsocket puts the new
                // connection at index 0.
                initWebsocket("/wopi/files/0?access_token=second");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitFirstInstall:
            case Phase::WaitSecondView:
            case Phase::Done:
                break;
        }
    }
};

/// A user alone on a document asks for their settings to be read again, which
/// is what the dialog does after writing them: a document reads them when it
/// opens, so without this a change waits for the next one.
class UnitLoneUserReloadsPresets : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitInstall, Reload, WaitReload, Done) _phase;

    int _installs = 0;

public:
    UnitLoneUserReloadsPresets()
        : Base("UnitLoneUserReloadsPresets")
        , _phase(Phase::Load)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI()
                          + "/wopi/settings/userconfig.json?testname=UnitLoneUserReloadsPresets";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", "lonestamp");
        fileInfo->set("UserSettings", userSettings);
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (uriReq.getPath() == "/wopi/settings/userconfig.json")
        {
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody("{\"kind\":\"user\"}", "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        return Base::handleHttpGetRequest(request, socket);
    }

    void onDocBrokerPresetsInstallEnd(bool success) override
    {
        ++_installs;
        TST_LOG("onDocBrokerPresetsInstallEnd: success=" << success << " install #" << _installs);
        LOK_ASSERT_MESSAGE("the presets should install", success);

        if (_installs < 2)
            return;

        LOK_ASSERT_STATE(_phase, Phase::WaitReload);
        TRANSITION_STATE(_phase, Phase::Done);
        passTest("a lone session has its settings read again");
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>&) override
    {
        if (_phase != Phase::WaitInstall)
            return;

        LOK_ASSERT_EQUAL(1, _installs);
        TRANSITION_STATE(_phase, Phase::Reload);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitInstall);
                TST_LOG("Opening the document");
                initWebsocket("/wopi/files/0?access_token=lone");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::Reload:
            {
                TRANSITION_STATE(_phase, Phase::WaitReload);
                TST_LOG("The settings dialog has saved, so ask for them again");
                WSD_CMD_BY_CONNECTION_INDEX(0, "reloadconfig");
                break;
            }
            case Phase::WaitInstall:
            case Phase::WaitReload:
            case Phase::Done:
                break;
        }
    }
};

/// A host that has no settings for the user, as its CheckFileInfo gives no UserSettings, is sent
/// none when the document closes. Before, it was sent the user's standard.dic.
class UnitNoUserSettingsNoUpload : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDestroy, Done) _phase;

    std::atomic<int> _settingsUploads;

public:
    UnitNoUserSettingsNoUpload()
        : WopiTestServer("UnitNoUserSettingsNoUpload")
        , _phase(Phase::Load)
        , _settingsUploads(0)
    {
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        if (request.getURI().find("/settings/upload") != std::string::npos)
        {
            ++_settingsUploads;
            TST_LOG("Settings upload: " << request.getURI());
        }

        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);
        TRANSITION_STATE(_phase, Phase::WaitDestroy);

        // The user's dictionary, as the Kit writes it once the user adds a word.
        const std::string wordbook = FileUtil::buildLocalPathToJail(
            JailUtil::isMountNamespacesEnabled(), getJailRoot(), "/tmp/user/user/wordbook/");
        TST_LOG("Writing standard.dic in " << wordbook);
        std::ofstream(wordbook + "/standard.dic") << "OOoUserDict1\nlang: <none>\n---\nword\n";

        WSD_CMD("closedocument");
        return true;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected no settings uploaded to a host without settings", 0,
                                 _settingsUploads.load());
        passTest("Nothing was uploaded to the settings of a host without settings");
        WopiTestServer::onDocBrokerDestroy(docKey);
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitLoadStatus);
            initWebsocket("/wopi/files/0?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

namespace
{
/// The fileId under which the host receives the user dictionary.
constexpr std::string_view WordbookFileId = "/settings/userconfig/wordbook/standard.dic";

/// The command the spelling context menu sends, with the dictionary named in the query.
constexpr auto AddToWordbookCommand = "uno .uno:AddToWordbook?Wordbook:string=standard.dic";

/// The fileId of a preset upload, or an empty string for any other request.
std::string presetUploadFileId(const Poco::Net::HTTPRequest& request)
{
    const Poco::URI uri(request.getURI());
    if (uri.getPath() != "/wopi/settings/upload")
        return std::string();

    for (const auto& parameter : uri.getQueryParameters())
    {
        if (parameter.first == "fileId")
            return parameter.second;
    }
    return std::string();
}

/// The directory of the user configuration inside the jail of the document of the session, or
/// an empty string when the session has no document.
std::string jailConfigPath(const std::shared_ptr<ClientSession>& session)
{
    const std::shared_ptr<DocumentBroker> docBroker = session->getDocumentBroker();
    if (!docBroker)
        return std::string();
    return FileUtil::buildLocalPathToJail(JailUtil::isMountNamespacesEnabled(),
                                          docBroker->getJailRoot(), JAILED_CONFIG_ROOT);
}

/// Writes the user dictionary in the jail with the given words, in the format the engine
/// stores it in. The engine adds a word only once its background spell check has marked it,
/// so the tests write the dictionary themselves and send the command to report the change.
void writeJailWordbook(const std::string& configPath, const std::vector<std::string>& words)
{
    const std::string directory = configPath + "wordbook";
    std::filesystem::create_directories(directory);
    std::ofstream file(directory + "/standard.dic", std::ios::trunc);
    file << "OOoUserDict1\nlang: <none>\ntype: positive\n---\n";
    for (const std::string& word : words)
        file << word << '\n';
}
} // namespace

/// A word added to the user dictionary reaches the host while the document is still open. The
/// settings dialog reads the dictionary from the host, so it shows the word without a reopen.
class UnitWordbookLiveUpload : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoad, AddWord, WaitUpload, Done) _phase;

    std::string _jailConfigPath;

public:
    UnitWordbookLiveUpload()
        : Base("UnitWordbookLiveUpload")
        , _phase(Phase::Load)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Presets go only to a host that has settings for the user.
        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI()
                          + "/wopi/settings/userconfig.json?testname=UnitWordbookLiveUpload";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", "wordbookstamp");
        fileInfo->set("UserSettings", userSettings);
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (uriReq.getPath() == "/wopi/settings/userconfig.json")
        {
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody("{\"kind\":\"user\"}", "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }
        return Base::handleHttpGetRequest(request, socket);
    }

    bool handleHttpPostRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                               const std::shared_ptr<StreamSocket>& socket) override
    {
        const std::string fileId = presetUploadFileId(request);
        if (fileId.empty())
            return Base::handleHttpPostRequest(request, message, socket);

        const std::string body(std::istreambuf_iterator<char>(message), {});
        TST_LOG("Preset upload of [" << fileId << "]: " << body);
        http::Response httpResponse(http::StatusCode::OK);
        socket->sendAndShutdown(httpResponse);

        if (fileId == WordbookFileId)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitUpload);
            LOK_ASSERT_MESSAGE("the uploaded dictionary should hold the new word",
                               body.find("Plorkvane") != std::string::npos);
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("the dictionary reached the host while the document is open");
        }
        return true;
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);
        _jailConfigPath = jailConfigPath(session);
        LOK_ASSERT_MESSAGE("the session should have a document", !_jailConfigPath.empty());
        TRANSITION_STATE(_phase, Phase::AddWord);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoad);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::AddWord:
            {
                TRANSITION_STATE(_phase, Phase::WaitUpload);
                writeJailWordbook(_jailConfigPath, { "Plorkvane" });
                WSD_CMD_BY_CONNECTION_INDEX(0, AddToWordbookCommand);
                break;
            }
            case Phase::WaitLoad:
            case Phase::WaitUpload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[7]{ new UnitEarlyDocDeath(),
                             new UnitSpifPreset(),
                             new UnitSecondUserPresets(),
                             new UnitLoneUserReloadsPresets(),
                             new UnitNoUserSettingsNoUpload(),
                             new UnitWordbookLiveUpload(),
                             nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
