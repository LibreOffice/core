/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <string>
#include <string_view>

#include <Poco/JSON/Object.h>

#include <UnitWSDClient.hpp>
#include <WopiTestServer.hpp>
#include <common/JsonUtil.hpp>
#include <common/StateEnum.hpp>
#include <common/Unit.hpp>
#include <lokassert.hpp>
#include <net/Socket.hpp>

// A page that sends its view settings with a secret left blank and flagged as kept gets them back
// the same way, while a secret that it typed in comes back as it was (and an AI key does not come
// back at all):
class UnitViewSettingSecrets: public WopiTestServer {
public:
    UnitViewSettingSecrets(): WopiTestServer("UnitViewSettingSecrets"), phase_(Phase::Load) {}

    bool onFilterSendWebSocketMessage(
        std::string_view const message, WSOpCode const, bool const, int &) override
    {
        std::string_view const prefix = "viewsetting: ";
        if (!message.starts_with(prefix)) {
            return false;
        }
        Poco::JSON::Object::Ptr settings;
        LOK_ASSERT(JsonUtil::parseJSON(std::string(message.substr(prefix.size())), settings));
        if (phase_ == Phase::WaitTypedEcho) {
            std::string zotero;
            JsonUtil::findJSONValue(settings, "zoteroAPIKey", zotero);
            LOK_ASSERT_EQUAL(std::string("typed-zotero-key"), zotero);
            std::string signature;
            JsonUtil::findJSONValue(settings, "signatureKey", signature);
            LOK_ASSERT_EQUAL(std::string("typed-signature-key"), signature);
            TRANSITION_STATE(phase_, Phase::SendKept);
        } else if (phase_ == Phase::WaitKeptEcho) {
            std::string zotero = "unset";
            JsonUtil::findJSONValue(settings, "zoteroAPIKey", zotero);
            LOK_ASSERT_EQUAL(std::string(), zotero);
            bool zoteroKept = false;
            JsonUtil::findJSONValue(settings, "zoteroAPIKeyStored", zoteroKept);
            LOK_ASSERT(zoteroKept);
            std::string signature = "unset";
            JsonUtil::findJSONValue(settings, "signatureKey", signature);
            LOK_ASSERT_EQUAL(std::string(), signature);
            bool signatureKept = false;
            JsonUtil::findJSONValue(settings, "signatureKeyStored", signatureKept);
            LOK_ASSERT(signatureKept);
            LOK_ASSERT(!settings->has("aiProviderAPIKey"));
            LOK_ASSERT(!settings->has("aiProviderAPIKeyStored"));
            TRANSITION_STATE(phase_, Phase::Done);
        }
        return false;
    }

    bool onDocumentLoaded(std::string const &) override {
        LOK_ASSERT_STATE(phase_, Phase::WaitLoaded);
        TRANSITION_STATE(phase_, Phase::WaitTypedEcho);
        WSD_CMD(
            "updateviewsettings {\"zoteroAPIKey\":\"typed-zotero-key\","
            "\"signatureKey\":\"typed-signature-key\"}");
        return true;
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Load:
            TRANSITION_STATE(phase_, Phase::WaitLoaded);
            initWebsocket("/wopi/files/0?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
            break;
        case Phase::SendKept:
            TRANSITION_STATE(phase_, Phase::WaitKeptEcho);
            WSD_CMD(
                "updateviewsettings {\"zoteroAPIKey\":\"\",\"zoteroAPIKeyStored\":true,"
                "\"signatureKey\":\"\",\"signatureKeyStored\":true,"
                "\"aiProviderAPIKey\":\"\",\"aiProviderAPIKeyStored\":true}");
            break;
        case Phase::Done:
            passTest("The view settings echo carried each secret as the page sent it");
            break;
        default:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Load, WaitLoaded, WaitTypedEcho, SendKept, WaitKeptEcho, Done) phase_;
};

UnitBase * unit_create_wsd() { return new UnitViewSettingSecrets(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
