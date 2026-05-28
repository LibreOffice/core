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

#include <common/Png.hpp>
#include <common/Rectangle.hpp>
#include <common/ThreadPool.hpp>
#include <kit/Delta.hpp>
#include <wsd/TileDesc.hpp>

#include <COKit/COKit.hxx>

#include <cassert>
#include <memory>
#include <vector>
namespace RenderTiles
{
    struct Buffer {
        unsigned char *_data;
        Buffer()
        {
            _data = nullptr;
        }
        Buffer(size_t x, size_t y) :
            Buffer()
        {
            allocate(x, y);
        }
        void allocate(size_t x, size_t y)
        {
            assert(!_data);
            _data = static_cast<unsigned char *>(calloc(x * y, 4));
        }
        ~Buffer()
        {
            if (_data)
                free (_data);
        }
        unsigned char *data() { return _data; }
    };

    // FIXME: we should perhaps increment only on a plausible edit
    static TileWireId getCurrentWireId(bool increment = false)
    {
        static TileWireId nextId = 0;
        if (increment)
            nextId++;
        return nextId;
    }

    bool doRender(
        const std::shared_ptr<kit::Document>& document, DeltaGenerator& deltaGen,
        TileCombined& tileCombined, ThreadPool& pngPool,
        const std::function<void(unsigned char* data, int offsetX, int offsetY, size_t pixmapWidth,
                                 size_t pixmapHeight, int pixelWidth, int pixelHeight,
                                 COKitTileMode mode)>& blendWatermark,
        const std::function<void(const char* buffer, size_t length)>& outputMessage,
        const std::function<void(std::string_view msg)>& errorMessage,
        [[maybe_unused]] unsigned mobileAppDocId, CanonicalViewId canonicalViewId, bool dumpTiles)
    {
        const auto& tiles = tileCombined.getTiles();

        // Otherwise our delta-building & threading goes badly wrong
        // external sources of tilecombine are checked at the perimeter
        assert(!tileCombined.hasDuplicates());

        // Calculate the area we cover
        Util::Rectangle renderArea;
        std::vector<Util::Rectangle> tileRecs;
        tileRecs.reserve(tiles.size());

        for (const auto& tile : tiles)
        {
            Util::Rectangle rectangle(tile.getTilePosX(), tile.getTilePosY(),
                                      tileCombined.getTileWidth(), tileCombined.getTileHeight());

            if (tileRecs.empty())
            {
                renderArea = rectangle;
            }
            else
            {
                renderArea.extend(rectangle);
            }

            tileRecs.push_back(rectangle);
        }

        assert(tiles.size() == tileRecs.size());

        const int areaWidth = renderArea.getWidth();
        const int areaHeight = renderArea.getHeight();
        if (areaWidth <= 0 || areaHeight <= 0)
        {
            LOG_ERR("Invalid render area " << areaWidth << 'x' << areaHeight);
            errorMessage("error: cmd=tile kind=invalidarea");
            return false;
        }

        const size_t tilesByX = static_cast<size_t>(areaWidth) / tileCombined.getTileWidth();
        const size_t tilesByY = static_cast<size_t>(areaHeight) / tileCombined.getTileHeight();
        const int pixelWidth = tileCombined.getWidth();
        const int pixelHeight = tileCombined.getHeight();
        assert (pixelWidth > 0 && pixelHeight > 0);
        const size_t pixmapWidth = tilesByX * static_cast<size_t>(pixelWidth);
        const size_t pixmapHeight = tilesByY * static_cast<size_t>(pixelHeight);

        if (pixmapWidth > 4096 || pixmapHeight > 4096)
            LOG_WRN("Unusual extremely large tile combine of size " << pixmapWidth << 'x' << pixmapHeight
                    << " (" << tilesByX << 'x' << tilesByY << " tiles to serve " << tiles.size() << " tiles: "
                    << (tiles.size() * 100)/(tilesByX * tilesByY) << "% in " << (tilesByX*tilesByY*0.25) << "MB");

        RenderTiles::Buffer pixmap(pixmapWidth, pixmapHeight);

        // Render the whole area
        const double area = pixmapWidth * pixmapHeight;
        const auto start = std::chrono::steady_clock::now();
        LOG_TRC("Calling paintPartTile(" << (void*)pixmap.data() << ')');
        document->paintPartTile(pixmap.data(),
                                tileCombined.getPart(),
                                tileCombined.getEditMode(),
                                pixmapWidth, pixmapHeight,
                                renderArea.getLeft(), renderArea.getTop(),
                                renderArea.getWidth(), renderArea.getHeight());
        auto duration = std::chrono::steady_clock::now() - start;
        const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        LOG_DBG("paintPartTile      " << tileRecs.size() << " tiles at ("
                << renderArea.getLeft() << ", " << renderArea.getTop() << "), ("
                << renderArea.getWidth() << ", " << renderArea.getHeight() << ") "
                << " took " << elapsedUs << " (" << area / elapsedUs.count() << " MP/s).");

        const auto mode = static_cast<COKitTileMode>(document->getTileMode());

        const size_t pixmapSize = 4 * pixmapWidth * pixmapHeight;
        std::vector<char> output;
        output.reserve(pixmapSize);

        // Compress the area as tiles
        TileCombinedBuilder renderedTiles;
        std::vector<TileWireId> renderingIds;

        size_t tileIndex = 0;

        std::mutex pngMutex;

        for (const Util::Rectangle& tileRect : tileRecs)
        {
            const size_t positionX = (tileRect.getLeft() - renderArea.getLeft()) / tileCombined.getTileWidth();
            const size_t positionY = (tileRect.getTop() - renderArea.getTop()) / tileCombined.getTileHeight();

            const int offsetX = positionX * pixelWidth;
            const int offsetY = positionY * pixelHeight;

            // FIXME: should this be in the delta / compression thread ?
            blendWatermark(pixmap.data(), offsetX, offsetY,
                           pixmapWidth, pixmapHeight,
                           pixelWidth, pixelHeight,
                           mode);

            // FIXME: prettify this.
            bool forceKeyframe = tiles[tileIndex].isForcedKeyFrame();

            // FIXME: share the same wireId for all tiles concurrently rendered.
            TileWireId wireId = getCurrentWireId(true);

            bool skipCompress = false;
            if (!skipCompress)
            {
                renderingIds.push_back(wireId);

                LOG_TRC("Queued encoding of tile #" << tileIndex << " at (" << positionX << ',' << positionY << ") with " <<
                        (forceKeyframe?"force keyframe" : "allow delta") << ", wireId: " << wireId);

                // Queue to be executed later in parallel inside 'run'
                pngPool.pushWork([=,&output,&pixmap,&tiles,&renderedTiles,
                                  &pngMutex,&deltaGen]()
                    {
                        std::vector< char > data;
                        data.reserve(pixmapWidth * pixmapHeight * 1);

                        // FIXME: don't try to store & create deltas for read-only documents.
                        if (!tiles[tileIndex].isPreview())
                        {
                            // Can we create a delta ?
                            LOG_TRC("Compress new tile #" << tileIndex);
                            assert(pixelWidth <= 256 && pixelHeight <= 256);
                            deltaGen.compressOrDelta(pixmap.data(), offsetX, offsetY,
                                                     pixelWidth, pixelHeight,
                                                     pixmapWidth, pixmapHeight,
                                                     TileLocation(
                                                         tileRect.getLeft(),
                                                         tileRect.getTop(),
                                                         tileRect.getWidth(),
                                                         tileCombined.getPart(),
                                                         canonicalViewId,
                                                         tileCombined.getEditMode()
                                                         ),
                                                     data, wireId, forceKeyframe, dumpTiles, mode);
                        }
                        else
                        {
                            // FIXME: write our own trivial PNG encoding code using deflate.
                            LOG_TRC("Encode a new png for tile #" << tileIndex);
                            if (!Png::encodeSubBufferToPNG(pixmap.data(), offsetX, offsetY, pixelWidth, pixelHeight,
                                                           pixmapWidth, pixmapHeight, data, mode))
                            {
                                LOG_ERR("Failed to encode tile into PNG.");
                                errorMessage("error: cmd=tile kind=pngencode");
                                return;
                            }
                        }

                        LOG_TRC("Tile " << tileIndex << " is " << data.size() << " bytes.");
                        std::unique_lock<std::mutex> pngLock(pngMutex);
                        output.insert(output.end(), data.begin(), data.end());
                        renderedTiles.pushRendered(tiles[tileIndex], wireId, data.size());
                    });
            }
            tileIndex++;
        }

        pngPool.run();

        duration = std::chrono::steady_clock::now() - start;
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        LOG_DBG("paintPartTile+comp " << tileRecs.size() << " tiles at ("
                << renderArea.getLeft() << ", " << renderArea.getTop()
                << "), (" << renderArea.getWidth() << ", "
                << renderArea.getHeight() << ") "
                << " took " << elapsed << " (" << area / elapsed.count() << " MP/s).");

        if (tileIndex == 0)
        {
            LOG_ERR("No tiles produced for render area " << areaWidth << 'x' << areaHeight);
            errorMessage("error: cmd=tile kind=empty");
            return false;
        }

        std::string tileMsg;
        if (tileCombined.getCombined())
        {
            tileMsg = renderedTiles.serialize("tilecombine:", "\n");

            LOG_TRC("Sending back painted tiles for " << tileMsg << " of size " << output.size() << " bytes) for: " << tileMsg);

            const size_t responseSize = tileMsg.size() + output.size();
            std::unique_ptr<char[]> response(std::make_unique<char[]>(responseSize));
            std::copy(tileMsg.begin(), tileMsg.end(), response.get());
            std::copy(output.begin(), output.end(), response.get() + tileMsg.size());
            outputMessage(response.get(), responseSize);
        }
        else
        {
            size_t outputOffset = 0;
            for (const auto &i : renderedTiles.getTiles())
            {
                tileMsg = i.serialize("tile:", "\n");
                const size_t responseSize = tileMsg.size() + i.getImgSize();
                std::unique_ptr<char[]> response(std::make_unique<char[]>(responseSize));
                std::copy(tileMsg.begin(), tileMsg.end(), response.get());
                std::copy(output.begin() + outputOffset, output.begin() + outputOffset + i.getImgSize(), response.get() + tileMsg.size());
                outputMessage(response.get(), responseSize);
                outputOffset += i.getImgSize();
            }
        }

        // Should we do this more frequently? and/or should we defer it?
        deltaGen.rebalanceDeltas();
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
