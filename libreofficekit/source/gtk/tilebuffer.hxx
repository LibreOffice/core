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

#define LOK_TILEBUFFER_ERROR (LOKTileBufferErrorQuark())

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
   Gets GQuark identifying this tile buffer errors
*/
GQuark LOKTileBufferErrorQuark(void);

/**
   This class represents a single tile in the tile buffer.
   It encloses a reference to GdkPixBuf containing the pixel data of the tile.
*/
class Tile
{
 public:
    Tile() : valid(false), m_pBuffer(nullptr) {}
    ~Tile()
    {
        if (m_pBuffer)
            cairo_surface_destroy(m_pBuffer);
    }

    /**
       Tells if this tile is valid or not. Initialised to 0 (invalid) during
       object creation.
    */
    bool valid;

    /// Function to get the pointer to enclosing cairo_surface_t
    cairo_surface_t* getBuffer();
    /// Used to set the pixel buffer of this object
    void setSurface(cairo_surface_t*);

private:
    /// Pixel buffer data for this tile
    cairo_surface_t *m_pBuffer;
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
 TileBuffer(LibreOfficeKitDocument *document = nullptr,
            int columns = 0)
     : m_pLOKDocument(document)
     , m_nWidth(columns)
    {
        cairo_surface_t *pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nTileSizePixels, nTileSizePixels);
        m_DummyTile.setSurface(pSurface);
        cairo_surface_destroy(pSurface);
    }

    ~TileBuffer() {}

    /**
       Gets the underlying Tile object for given position. The position (0, 0)
       points to the left top most tile of the buffer.

       If the tile is not cached by the tile buffer, it makes a paintTile call
       to LO core asking to render the given tile. It then stores the tile for
       future reuse.

       @param x the tile along the x-axis of the buffer
       @param y the tile along the y-axis of the buffer
       @param task GTask object containing the necessary data
       @param pool GThreadPool managed by the widget instance used for all the
       LOK calls made by widget. It is needed here because getTile invokes one
       of the LOK call : paintTile.

       @return the tile at the mentioned position (x, y)
     */
    Tile& getTile(int x, int y, GTask* task, GThreadPool* pool);
    /// Destroys all the tiles in the tile buffer; also frees the memory allocated
    /// for all the Tile objects.
    void resetAllTiles();
    /**
       Marks the tile as invalid. The tile (0, 0) is the left topmost tile in
       the tile buffer.

       @param x the position of tile along x-axis
       @param y the position of tile along y-axis
       @param zoom zoom factor of the document
       @param task GTask object containing the necessary data
       @param pool GThreadPool managed by the widget instance used for all the
       LOK calls made by widget. It is needed here because setInvalid() invokes one
       of the LOK call : paintTile.
     */
    void setInvalid(int x, int y, float zoom, GTask* task, GThreadPool*);

    /// Contains the reference to the LOK Document that this tile buffer is for.
    LibreOfficeKitDocument *m_pLOKDocument;
    /// Stores all the tiles cached by this tile buffer.
    std::map<int, Tile> m_mTiles;
    /// Width of the current tile buffer (number of columns)
    int m_nWidth;
    /// Dummy tile
    Tile m_DummyTile;
};

enum
{
    LOK_LOAD_DOC,
    LOK_POST_COMMAND,
    LOK_SET_EDIT,
    LOK_SET_PARTMODE,
    LOK_SET_PART,
    LOK_POST_KEY,
    LOK_PAINT_TILE,
    LOK_POST_MOUSE_EVENT,
    LOK_SET_GRAPHIC_SELECTION,
    LOK_SET_CLIENT_ZOOM
};

enum
{
    LOK_TILEBUFFER_CHANGED,
    LOK_TILEBUFFER_MEMORY
};

/**
   A struct that we use to store the data about the LOK call.

   Object of this type is passed with all the LOK calls,
   so that they can be idenitified. Additionally, it also contains
   the data that LOK call needs.
*/
struct LOEvent
{
    /// To identify the type of LOK call
    int m_nType;

    /// @name post_command parameters
    ///@{
    const gchar* m_pCommand;
    gchar* m_pArguments;
    gboolean m_bNotifyWhenFinished;
    ///@}

    /// @name open_document parameter
    ///@{
    const gchar* m_pPath;
    ///@}

    /// set_edit parameter
    gboolean m_bEdit;

    /// set_partmode parameter
    int m_nPartMode;

    /// set_part parameter
    int m_nPart;

    /// @name postKeyEvent parameters
    ///@{
    int m_nKeyEvent;
    int m_nCharCode;
    int m_nKeyCode;
    ///@}

    /// @name paintTile parameters
    ///@{
    int m_nPaintTileX;
    int m_nPaintTileY;
    float m_fPaintTileZoom;
    TileBuffer* m_pTileBuffer;
    ///@}

    /// @name postMouseEvent parameters
    ///@{
    int m_nPostMouseEventType;
    int m_nPostMouseEventX;
    int m_nPostMouseEventY;
    int m_nPostMouseEventCount;
    int m_nPostMouseEventButton;
    int m_nPostMouseEventModifier;
    ///@}

    /// @name setGraphicSelection parameters
    ///@{
    int m_nSetGraphicSelectionType;
    int m_nSetGraphicSelectionX;
    int m_nSetGraphicSelectionY;
    ///@}

    /// @name setClientView parameters
    ///@{
    int m_nTilePixelWidth;
    int m_nTilePixelHeight;
    int m_nTileTwipWidth;
    int m_nTileTwipHeight;
    ///@}

    /// Constructor to instantiate an object of type `type`.
    LOEvent(int type)
        : m_nType(type)
        , m_pCommand(nullptr)
        , m_pArguments(nullptr)
        , m_bNotifyWhenFinished(false)
        , m_pPath(nullptr)
        , m_bEdit(false)
        , m_nPartMode(0)
        , m_nPart(0)
        , m_nKeyEvent(0)
        , m_nCharCode(0)
        , m_nKeyCode(0)
        , m_nPaintTileX(0)
        , m_nPaintTileY(0)
        , m_fPaintTileZoom(0)
        , m_pTileBuffer(nullptr)
        , m_nPostMouseEventType(0)
        , m_nPostMouseEventX(0)
        , m_nPostMouseEventY(0)
        , m_nPostMouseEventCount(0)
        , m_nPostMouseEventButton(0)
        , m_nPostMouseEventModifier(0)
        , m_nSetGraphicSelectionType(0)
        , m_nSetGraphicSelectionX(0)
        , m_nSetGraphicSelectionY(0)
    {
    }

    /// Wrapper around delete to help GLib.
    static void destroy(void* pMemory);
};

#endif // INCLUDED_TILEBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
