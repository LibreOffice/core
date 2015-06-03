/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tilebuffer.hxx"

static const int DPI = 96;

static float pixelToTwip(float fInput, float zoom)
{
    return (fInput / DPI / zoom) * 1440.0f;
}

static float twipToPixel(float fInput, float zoom)
{
    return fInput / 1440.0f * DPI * zoom;
}

GdkPixbuf* Tile::tile_get_buffer()
{
    return m_pBuffer;
}

void Tile::tile_release()
{
    g_object_unref (m_pBuffer);
    m_pBuffer = NULL;
}

void TileBuffer::tile_buffer_set_zoom(float newZoomFactor, int rows, int columns)
{
    m_fZoomFactor = newZoomFactor;

    tile_buffer_reset_all_tiles();

    // set new buffer width and height
    m_nWidth = columns;
    m_nHeight = rows;
}

void TileBuffer::tile_buffer_reset_all_tiles()
{
    std::map<int, Tile>::iterator it = m_mTiles.begin();
    for (; it != m_mTiles.end(); it++)
        {
            it->second.tile_release();
        }
    m_mTiles.clear();
}

void TileBuffer::tile_buffer_set_invalid(int x, int y)
{
    int index = x * m_nWidth + y;
    g_info("setting invalid : %d %d",x, y);
    if (m_mTiles.find(index) != m_mTiles.end())
        {
            m_mTiles[index].valid = 0;
            m_mTiles[index].tile_release();
            m_mTiles.erase(index);
        }
}

Tile& TileBuffer::tile_buffer_get_tile(int x, int y)
{
    int index = x * m_nWidth + y;
    if(m_mTiles.find(index) == m_mTiles.end() || !m_mTiles[index].valid)
        {

            GdkPixbuf* pPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, m_nTileSize, m_nTileSize);
            if (!pPixBuf){
                g_info ("error allocating memory to pixbuf");
            }
            unsigned char* pBuffer = gdk_pixbuf_get_pixels(pPixBuf);
            GdkRectangle aTileRectangle;
            aTileRectangle.x = pixelToTwip(m_nTileSize, m_fZoomFactor) * y;
            aTileRectangle.y = pixelToTwip(m_nTileSize, m_fZoomFactor) * x;

            g_info ("rendering (%d %d)", x, y);
            m_pLOKDocument->pClass->paintTile(m_pLOKDocument,
                                              // Buffer and its size, depends on the position only.
                                              pBuffer,
                                              m_nTileSize, m_nTileSize,
                                              // Position of the tile.
                                              aTileRectangle.x, aTileRectangle.y,
                                              // Size of the tile, depends on the zoom factor and the tile position only.
                                              pixelToTwip(m_nTileSize, m_fZoomFactor), pixelToTwip(m_nTileSize, m_fZoomFactor));

            //create a mapping for it
            m_mTiles[index].tile_set_pixbuf(pPixBuf);
            m_mTiles[index].valid = 1;
        }

    return m_mTiles[index];
}

void Tile::tile_set_pixbuf(GdkPixbuf *buffer)
{
    m_pBuffer = buffer;
}
