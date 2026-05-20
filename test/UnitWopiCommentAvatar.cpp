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

#include <config.h>

#include <StringVector.hpp>
#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpRequest.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

/// Verifies that the comment_avatar coolwsd.xml setting reaches the client both via the wopi:
/// websocket frame and via the iframe's Content-Security-Policy img-src directive.
class UnitWopiCommentAvatar : public WopiTestServer
{
    STATE_ENUM(Phase, CheckCsp, Load, WaitWopiInfo, Done) _phase;

    static constexpr const char* kAvatarUrl = "https://avatars.example.com/test/avatar.png";

public:
    UnitWopiCommentAvatar()
        : WopiTestServer("UnitWopiCommentAvatar")
        , _phase(Phase::CheckCsp)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setString("comment_avatar", kAvatarUrl);
    }

    bool onFilterSendWebSocketMessage(const std::string_view data, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (_phase != Phase::WaitWopiInfo)
            return false;

        constexpr std::string_view prefix = "wopi: ";
        if (!data.starts_with(prefix))
            return false;

        const std::string json{ data.substr(prefix.size()) };
        TST_LOG("wopi info: " << json);

        Poco::JSON::Parser parser;
        const Poco::Dynamic::Var parsed = parser.parse(json);
        const Poco::JSON::Object::Ptr obj = parsed.extract<Poco::JSON::Object::Ptr>();
        if (!obj->has("CommentAvatarUrl"))
        {
            failTest("wopi info is missing CommentAvatarUrl");
            return false;
        }
        const std::string actual = obj->getValue<std::string>("CommentAvatarUrl");
        if (actual != kAvatarUrl)
        {
            failTest("CommentAvatarUrl mismatch; expected [" + std::string(kAvatarUrl)
                     + "] got [" + actual + ']');
            return false;
        }

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("comment_avatar reached the client via wopi info and CSP img-src");
        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::CheckCsp:
            {
                // The CSP check is independent of any document being loaded; do it first so the
                // remaining flow only exercises the wopi-info wire side.
                verifyCspImgSrc();
                TRANSITION_STATE(_phase, Phase::Load);
                // fall through to start the load in the next invocation
                break;
            }
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitWopiInfo);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitWopiInfo:
            case Phase::Done:
                break;
        }
    }

private:
    void verifyCspImgSrc()
    {
        const std::shared_ptr<const http::Response> response = http::get(
            helpers::getTestServerURI(), "/browser/dist/cool.html?access_token=anything");
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());

        const std::string csp = response->header().get("Content-Security-Policy", std::string());
        TST_LOG("CSP: " << csp);

        const Poco::URI avatarUri{ kAvatarUrl };
        const std::string expectedOrigin = avatarUri.getScheme() + "://" + avatarUri.getHost();

        StringVector directives = StringVector::tokenize(csp, ';');
        for (std::size_t i = 0; i < directives.size(); ++i)
        {
            if (!directives.startsWith(i, " img-src") && !directives.startsWith(i, "img-src"))
                continue;
            if (directives[i].find(expectedOrigin) != std::string::npos)
                return;
            failTest("CSP img-src does not include the configured avatar origin [" + expectedOrigin
                     + "]; img-src was [" + directives[i] + ']');
            return;
        }
        failTest("CSP has no img-src directive at all: [" + csp + ']');
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWopiCommentAvatar(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
