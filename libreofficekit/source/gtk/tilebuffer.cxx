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
cairo_surface_t* Tile::getBuffer()
{
    return m_pBuffer;
}

void Tile::setSurface(cairo_surface_t *buffer)
{
    if (m_pBuffer == buffer)
        return;
    if (m_pBuffer)
        cairo_surface_destroy(m_pBuffer);
    if (buffer != nullptr)
        cairo_surface_reference(buffer);
    m_pBuffer = buffer;
}

/* ----------------------------------
   TileBuffer class member functions
   ----------------------------------
*/
void TileBuffer::resetAllTiles()
{
    for (auto & tile : m_mTiles)
    {
        tile.second.valid = false;
    }
}

void TileBuffer::setInvalid(int x, int y, float fZoom, GTask* task,
                            GThreadPool* lokThreadPool)
{
    int index = x * m_nWidth + y;
    GError* error = nullptr;
    if (m_mTiles.find(index) == m_mTiles.end())
        return;

    m_mTiles[index].valid = false;

    LOEvent* pLOEvent = new LOEvent(LOK_PAINT_TILE);
    pLOEvent->m_nPaintTileX = x;
    pLOEvent->m_nPaintTileY = y;
    pLOEvent->m_fPaintTileZoom = fZoom;
    g_task_set_task_data(task, pLOEvent, LOEvent::destroy);
    g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
    if (error != nullptr)
    {
        g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
        g_clear_error(&error);
    }
}

Tile& TileBuffer::getTile(int x, int y, GTask* task,
                          GThreadPool* lokThreadPool)
{
    int index = x * m_nWidth + y;
    GError* error = nullptr;

    if (m_mTiles.find(index) != m_mTiles.end() && !m_mTiles[index].valid)
    {
        g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
            g_clear_error(&error);
        }
        return m_mTiles[index];
    }
    else if(m_mTiles.find(index) == m_mTiles.end())
    {
        g_thread_pool_push(lokThreadPool, g_object_ref(task), &error);
        if (error != nullptr)
        {
            g_warning("Unable to call LOK_PAINT_TILE: %s", error->message);
            g_clear_error(&error);
        }
        return m_DummyTile;
    }

    return m_mTiles[index];
}

void TileBuffer::setTile(int x, int y, cairo_surface_t *surface)
{
    int index = x * m_nWidth + y;

    m_mTiles[index].setSurface(surface);
    m_mTiles[index].valid = true;
}

bool TileBuffer::hasValidTile(int x, int y)
{
    int index = x * m_nWidth + y;
    auto it = m_mTiles.find(index);
    return (it != m_mTiles.end()) && it->second.valid;
}

void LOEvent::destroy(void* pMemory)
{
    LOEvent* pLOEvent = static_cast<LOEvent*>(pMemory);
    delete pLOEvent;
}

GQuark
LOKTileBufferErrorQuark()
{
    return g_quark_from_static_string("lok-tilebuffer-error");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
