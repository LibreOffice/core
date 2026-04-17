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
 * Unit test for proxy functionality.
 */

#include <config.h>

#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <net/HttpRequest.hpp>
#include <test/helpers.hpp>

using namespace std::literals;

class UnitProxyProtocol : public UnitWSD
{
    http::Request _req;
    bool _sentRequest;

public:
    UnitProxyProtocol()
        : UnitWSD("UnitProxyProtocol")
        , _sentRequest(false)
    {
        setTimeout(10s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        // Enable the unusual proxy prefix mode. Must be done on the first test.
        config.setBool("net.proxy_prefix", true);
    }

    std::shared_ptr<http::Session> openSession()
    {
        auto httpSession = http::Session::create(helpers::getTestServerURI());
        httpSession->setConnectFailHandler([this](const std::shared_ptr<http::Session>&)
                                           { LOK_ASSERT_FAIL("Unexpected connection failure"); });
        httpSession->setTimeout(9s);
        return httpSession;
    }

    void invokeWSDTest() override
    {
        if (_sentRequest)
            return; // be more patient.
        _sentRequest = true;

        std::string documentPath =
            helpers::getTempFileCopyPath(TDOC, "empty.odt", "UnitProxyProtocol");
        std::string encodedPath;
        Poco::URI::encode(Poco::Path(documentPath).makeAbsolute().toString(), ":/?", encodedPath);
        std::string wopiSRC = encodedPath;

        auto httpSession = openSession();

        http::Request httpRequest("cool/" + wopiSRC + "/ws?WOPISrc=/wopi/files" + wopiSRC +
                                  "&compat=/ws/open/open/1");
        httpRequest.set("ProxyPrefix", "https://substituted-into-cool.html-from-here/");
        auto httpResponse = httpSession->syncRequest(httpRequest);
        LOK_ASSERT_EQUAL(true, httpResponse->done());
        LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusCode());

        std::string accessToken = httpResponse->getBody();
        TST_LOG("Got access token for proxy document URL " << httpSession->getUrl() << " which is "
                                                           << accessToken);

        std::string loadCmd = wopiSRC;
        loadCmd += "lang=en deviceFormFactor=desktop timezone=Europe/London "
                   "darkTheme=false darkBackground=false accessibilityState=false "
                   "clientvisiblearea=0;0;10980;2250";
        http::Request loadRequest("cool/" + wopiSRC + "/ws?WOPISrc=/wopi/files" + wopiSRC +
                                  "&compat=/ws/" + accessToken + "/write/2");
        loadRequest.set("ProxyPrefix", "https://substituted-into-cool.html-from-here/");
        std::ostringstream os;
        os << "B" << std::hex << "0x" << 2 << "\n0x" << loadCmd.size() << '\n';
        const std::string str = os.str();

        loadRequest.setBody("B0x1\n0x2f\ncoolclient 0.1 1758919064472 1325.9000000953674\n" +
                            os.str() + loadCmd + "\n" + ".");
        httpResponse = httpSession->syncRequest(loadRequest);
        LOK_ASSERT_EQUAL(true, httpResponse->done());
        LOK_ASSERT_EQUAL(http::Response::State::Complete, httpResponse->state());
        LOK_ASSERT_EQUAL(http::StatusCode::OK, httpResponse->statusCode());

        TST_LOG("Attempt proxy URL fetch " << httpSession->getUrl() << _req.getUrl());

        exitTest(TestResult::Ok);
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitProxyProtocol(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
