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

static void getTileFunc(GTask*, gpointer, gpointer task_data, GCancellable*)
{
    GdkPixbuf* pPixBuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, nTileSizePixels, nTileSizePixels);
    GetTileCallbackData* pCallback = static_cast<GetTileCallbackData*>(task_data);
    TileBuffer* buffer = pCallback->m_pBuffer;
    int index = pCallback->m_nX * buffer->m_nWidth + pCallback->m_nY;
    if (!pPixBuf)
    {
        g_info ("Error allocating memory to pixbuf");
        return;
    }

    unsigned char* pBuffer = gdk_pixbuf_get_pixels(pPixBuf);
    GdkRectangle aTileRectangle;
    aTileRectangle.x = pixelToTwip(nTileSizePixels, pCallback->m_fZoom) * pCallback->m_nY;
    aTileRectangle.y = pixelToTwip(nTileSizePixels, pCallback->m_fZoom) * pCallback->m_nX;

    g_test_timer_start();
    buffer->m_pLOKDocument->pClass->paintTile(buffer->m_pLOKDocument,
                                      pBuffer,
                                      nTileSizePixels, nTileSizePixels,
                                      aTileRectangle.x, aTileRectangle.y,
                                      pixelToTwip(nTileSizePixels, pCallback->m_fZoom),
                                      pixelToTwip(nTileSizePixels, pCallback->m_fZoom));

    double elapsedTime = g_test_timer_elapsed();
    g_info ("Rendered (%d, %d) in %f seconds",
            pCallback->m_nX,
            pCallback->m_nY,
            elapsedTime);

    //create a mapping for it
    buffer->m_mTiles[index].setPixbuf(pPixBuf);
    buffer->m_mTiles[index].valid = true;
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
    for (; it != m_mTiles.end(); ++it)
    {
        it->second.valid = false;
    }
}

void TileBuffer::setInvalid(int x, int y, float fZoom)
{
    int index = x * m_nWidth + y;
    g_info("Setting tile invalid (%d, %d)", x, y);
    if (m_mTiles.find(index) != m_mTiles.end())
    {
        m_mTiles[index].valid = false;
        GTask* task = g_task_new(this, NULL, NULL, NULL);
        GetTileCallbackData* pCallback = new GetTileCallbackData(x, y, fZoom, this);
        g_task_set_task_data(task, pCallback, g_free);
        g_task_run_in_thread(task, getTileFunc);
    }
}

Tile& TileBuffer::getTile(int x, int y, float aZoom, GTask* task)
{
    int index = x * m_nWidth + y;

    if (m_mTiles.find(index) != m_mTiles.end() && !m_mTiles[index].valid)
    {
        GetTileCallbackData* pCallback = new GetTileCallbackData(x, y, aZoom, this);
        g_task_set_task_data(task, pCallback, g_free);
        g_task_run_in_thread(task, getTileFunc);
        return m_mTiles[index];
    }
    else if(m_mTiles.find(index) == m_mTiles.end())
    {
        GetTileCallbackData* pCallback = new GetTileCallbackData(x, y, aZoom, this);
        g_task_set_task_data(task, pCallback, g_free);
        g_info ("running in thread new tile");
        g_task_run_in_thread(task, getTileFunc);
        return m_DummyTile;
    }

    return m_mTiles[index];
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
