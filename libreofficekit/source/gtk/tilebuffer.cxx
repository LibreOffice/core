/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tilebuffer.hxx"


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

void Tile::setPixbuf(GdkPixbuf *buffer)
{
    if (m_pBuffer == buffer)
        return;
    g_clear_object(&m_pBuffer);
    if (buffer != NULL)
        g_object_ref(buffer);
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

void TileBuffer::setInvalid(int x, int y, float fZoom, GTask* task,
                            GThreadPool* lokThreadPool)
{
    int index = x * m_nWidth + y;
    GError* error = NULL;
    if (m_mTiles.find(index) != m_mTiles.end())
    {
        m_mTiles[index].valid = false;

        LOEvent* pLOEvent = new LOEvent(LOK_PAINT_TILE);
        pLOEvent->m_nPaintTileX = x;
        pLOEvent->m_nPaintTileY = y;
        pLOEvent->m_fPaintTileZoom = fZoom;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
        g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
        if (error != NULL)
        {
            g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
            g_clear_error(&error);
        }
    }
}

Tile& TileBuffer::getTile(int x, int y, float fZoom, GTask* task,
                          GThreadPool* lokThreadPool)
{
    int index = x * m_nWidth + y;
    GError* error = NULL;

    if (m_mTiles.find(index) != m_mTiles.end() && !m_mTiles[index].valid)
    {
        LOEvent* pLOEvent = new LOEvent(LOK_PAINT_TILE);
        pLOEvent->m_nPaintTileX = x;
        pLOEvent->m_nPaintTileY = y;
        pLOEvent->m_fPaintTileZoom = fZoom;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
        g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
        if (error != NULL)
        {
            g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
            g_clear_error(&error);
        }
        return m_mTiles[index];
    }
    else if(m_mTiles.find(index) == m_mTiles.end())
    {
        LOEvent* pLOEvent = new LOEvent(LOK_PAINT_TILE);
        pLOEvent->m_nPaintTileX = x;
        pLOEvent->m_nPaintTileY = y;
        pLOEvent->m_fPaintTileZoom = fZoom;
        g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
        g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
        if (error != NULL)
        {
            g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
            g_clear_error(&error);
        }
        return m_DummyTile;
    }

    return m_mTiles[index];
}

void LOEvent::destroy(void* pMemory)
{
    LOEvent* pLOEvent = static_cast<LOEvent*>(pMemory);
    delete pLOEvent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
