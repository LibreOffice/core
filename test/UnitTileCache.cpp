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
 * Unit test for tile cache functionality.
 */

#include <config.h>

#include <fstream>
#include <thread>

#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>
#include <WopiTestServer.hpp>

using namespace helpers;

class UnitTileCache: public WopiTestServer
{
    STATE_ENUM(Phase,
               Load, // load the document
               Tile, // lookup tile method
    )
    _phase;
public:
    UnitTileCache()
        : WopiTestServer("UnitTileCache")
        , _phase(Phase::Load)
    {
    }

    virtual void lookupTile(const std::string& part, int mode, int width, int height,
                            int tilePosX, int tilePosY, int tileWidth, int tileHeight,
                            std::shared_ptr<TileData> &tile)
    {
        // Call base to fire events.
        UnitWSD::lookupTile(part, mode, width, height, tilePosX, tilePosY, tileWidth, tileHeight, tile);

        // Fail the lookup to force subscription and rendering.
        tile.reset();

        // FIXME: push through to the right place to exercise this.
        exitTest(TestResult::Ok);
    }

    virtual void invokeWSDTest()
    {
        switch (_phase)
        {
        case Phase::Load:
        {
            TRANSITION_STATE(_phase, Phase::Tile);

            TST_LOG("Load: initWebsocket");
            initWebsocket("/wopi/files/0?access_token=anything");

            WSD_CMD("load url=" + getWopiSrc());

            // FIXME: need to invoke the tile lookup ...
            exitTest(TestResult::Ok);
            break;
        }
        case Phase::Tile:
            break;
        }
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitTileCache();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
