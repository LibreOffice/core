/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// We know that VirtualDevices use a DPI of 96.
const int DPI = 96;
// Lets use a square of side 256 pixels for each tile.
const int nTileSizePixels = 256;

/**
   Converts the pixel value to zoom independent twip value.

   @param fInput value to convert
   @param zoom the current zoom level

   @return the pixels value corresponding to given twip value
*/
float pixelToTwip(float fInput, float zoom);

/**
   Converts the zoom independent twip value pixel value.

   @param fInput value to convert
   @param zoom the current zoom level

   @return the twip value corresponding to given pixel value
*/
float twipToPixel(float fInput, float zoom);

/**
   This class represents a single tile in the tile buffer.
   It encloses a reference to GdkPixBuf containing the pixel data of the tile.
*/
class Tile
{
 public:
    Tile() : valid(0) {}
    ~Tile() { }

    /**
       Tells if this tile is valid or not. Initialised to 0 (invalid) during
       object creation.
    */
    bool valid;

    /// Function to get the pointer to enclosing GdkPixbuf
    GdkPixbuf* getBuffer();
    /// Destroys the enclosing GdkPixbuf object pointed to by m_pBuffer
    void release();
    /// Used to set the pixel buffer of this object
    void setPixbuf(GdkPixbuf*);

private:
    /// Pixel buffer data for this tile
    GdkPixbuf *m_pBuffer;
};

/**
   This class represents the tile buffer which is responsible for managing,
   reusing and caching all the already rendered tiles. If the given tile is not
   present in the buffer, call to LOK Document's (m_pLOKDocument) paintTile
   method is made which fetches the rendered tile from LO core and store it in
   buffer for future reuse.
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
        , m_nWidth(columns)
        , m_nHeight(rows)
    {  }

    ~TileBuffer() {}

    /**
       Gets the underlying Tile object for given position. The position (0, 0)
       points to the left top most tile of the buffer.

       If the tile is not cached by the tile buffer, it makes a paintTile call
       to LO core asking to render the given tile. It then stores the tile for
       future reuse.

       @param x the tile along the x-axis of the buffer
       @param y the tile along the y-axis of the buffer

       @return the tile at the mentioned position (x, y)
     */
    Tile& getTile(int x, int y);
    /// Destroys all the tiles in the tile buffer; also frees the memory allocated
    /// for all the Tile objects.
    void resetAllTiles();
    /**
       Marks the tile as invalid. The tile (0, 0) is the left topmost tile in
       the tile buffer.

       @param x the position of tile along x-axis
       @param y the position of tile along y-axis
     */
    void setInvalid(int x, int y);

 private:
    /// Contains the reference to the LOK Document that this tile buffer is for.
    LibreOfficeKitDocument *m_pLOKDocument;
    /// The side of each squared tile in pixels.
    int m_nTileSize;
    /// Stores all the tiles cached by this tile buffer.
    std::map<int, Tile> m_mTiles;
    /// Width of the current tile buffer (number of columns)
    int m_nWidth;
    /// Height of the current tile buffer (numbero of rows)
    int m_nHeight;
};

#endif // INCLUDED_TILEBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
