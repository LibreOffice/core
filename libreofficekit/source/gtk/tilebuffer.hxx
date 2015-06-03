/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TILEBUFFER_HXX
#define INCLUDED_TILEBUFFER_HXX

#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <map>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitGtk.h>

/*
  This class represents a single tile in the tile buffer.
  TODO: Extend it to support features like double buffering
*/
class Tile
{
public:
  Tile() : valid(0) {}
  ~Tile() {
    tile_release();
  }

  GdkPixbuf* tile_get_buffer();
  void tile_release();
  void tile_set_pixbuf(GdkPixbuf*);
  bool valid;
private:
  GdkPixbuf *m_pBuffer;
};

/*
  TileBuffer is the buffer caching all the recently rendered tiles.
  The buffer is set to invalid when zoom factor changes.
*/
class TileBuffer
{
public:
  TileBuffer(LibreOfficeKitDocument *document,
             int tileSize,
             int rows,
             int columns)
    : m_pLOKDocument(document)
    , m_nTileSize(tileSize)
    , m_fZoomFactor(1)
    , m_nWidth(columns)
    , m_nHeight(rows)
    {  }

  ~TileBuffer() {}

  void tile_buffer_set_zoom(float zoomFactor, int rows, int columns);
  Tile& tile_buffer_get_tile(int x, int y);
  void tile_buffer_update();
  void tile_buffer_reset_all_tiles();
private:
  LibreOfficeKitDocument *m_pLOKDocument;
  int m_nTileSize;
  float m_fZoomFactor;
  std::map<int, Tile> m_mTiles;
  //TODO: Also set width and height when document size changes
  int m_nWidth;
  int m_nHeight;
};

#endif // INCLUDED_TILEBUFFER_HXX
