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
 * Unit test for WOPI watermark functionality.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <Poco/Net/HTTPRequest.h>

class UnitWOPIWatermark : public WopiTestServer
{
    STATE_ENUM(Phase, Load, TileRequest, Done)
    _phase;

public:
    UnitWOPIWatermark()
        : WopiTestServer("UnitWOPIWatermark")
        , _phase(Phase::Load)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const Poco::URI uriReq(request.getURI());
        const std::string fileName(uriReq.getPath() == "/wopi/files/3" ? "he%llo.txt"
                                                                       : "hello.txt");
        fileInfo->set("BaseFileName", fileName);
        fileInfo->set("WatermarkText", "WatermarkTest");
    }

    void invokeWSDTest() override
    {
        constexpr char testName[] = "UnitWOPIWatermark";

        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::TileRequest);

                initWebsocket("/wopi/files/5?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::Done:
            {
                exitTest(TestResult::Ok);
                break;
            }
            case Phase::TileRequest:
            {
                WSD_CMD("tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840 "
                        "tileposy=0,0 tilewidth=3840 tileheight=3840");
                const std::string tile =
                    helpers::getResponseString(getWs()->getWebSocket(), "tile:", testName);

                if (!tile.empty())
                {
                    StringVector tokens(StringVector::tokenize(tile, ' '));
                    std::string nviewid = tokens[1].substr(std::string_view("nviewid=").size());
                    if (!nviewid.empty() && nviewid != "0")
                    {
                        LOG_INF(
                            "Watermark is hashed into integer successfully nviewid=" << nviewid);
                        TRANSITION_STATE(_phase, Phase::Done);
                    }
                }
                break;
            }
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWOPIWatermark(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
