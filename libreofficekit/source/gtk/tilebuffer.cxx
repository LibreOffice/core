/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tilebuffer.hxx"

#if !GLIB_CHECK_VERSION(2,40,0)
#define g_info(...) g_log(G_LOG_DOMAIN, G_LOG_LEVEL_INFO, __VA_ARGS__)
#endif

/* ------------------
   Utility functions
   ------------------
*/
float pixelToTwip(float fInput, float zoom)
{
    return (fInput / DPI / zoom) * 1440.0f;
}

float twipToPixel(float fInput, float zoom)
{
    return fInput / 1440.0f * DPI * zoom;
}

/* ----------------------------
   Tile class member functions
   ----------------------------
*/
GdkPixbuf* Tile::getBuffer()
{
    return m_pBuffer;
}

void Tile::release()
{
    g_object_unref (m_pBuffer);
    m_pBuffer = NULL;
}

void Tile::setPixbuf(GdkPixbuf *buffer)
{
    m_pBuffer = buffer;
}

/* ----------------------------------
   TileBuffer class member functions
   ----------------------------------
*/
void TileBuffer::resetAllTiles()
{
    std::map<int, Tile>::iterator it = m_mTiles.begin();
    for (; it != m_mTiles.end(); it++)
    {
        it->second.release();
    }
    m_mTiles.clear();
}

void TileBuffer::setInvalid(int x, int y)
{
    int index = x * m_nWidth + y;
    g_info("Setting tile invalid (%d, %d)", x, y);
    if (m_mTiles.find(index) != m_mTiles.end())
    {
        m_mTiles[index].valid = 0;
        m_mTiles[index].release();
        m_mTiles.erase(index);
    }
}

Tile& TileBuffer::getTile(int x, int y, float aZoom)
{
    int index = x * m_nWidth + y;
    if(m_mTiles.find(index) == m_mTiles.end() || !m_mTiles[index].valid)
    {

        GdkPixbuf* pPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, m_nTileSize, m_nTileSize);
        if (!pPixBuf)
        {
            g_info ("Error allocating memory to pixbuf");
            return m_mTiles[index];
        }

        unsigned char* pBuffer = gdk_pixbuf_get_pixels(pPixBuf);
        GdkRectangle aTileRectangle;
        aTileRectangle.x = pixelToTwip(m_nTileSize, aZoom) * y;
        aTileRectangle.y = pixelToTwip(m_nTileSize, aZoom) * x;

        g_info ("Rendering (%d, %d)", x, y);
        m_pLOKDocument->pClass->paintTile(m_pLOKDocument,
                                          pBuffer,
                                          m_nTileSize, m_nTileSize,
                                          aTileRectangle.x, aTileRectangle.y,
                                          pixelToTwip(m_nTileSize, aZoom),
                                          pixelToTwip(m_nTileSize, aZoom));

        //create a mapping for it
        m_mTiles[index].setPixbuf(pPixBuf);
        m_mTiles[index].valid = 1;
    }

    return m_mTiles[index];
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
