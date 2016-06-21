/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_ITILEDRENDERABLE_HXX
#define INCLUDED_VCL_ITILEDRENDERABLE_HXX

#include <LibreOfficeKit/LibreOfficeKitTypes.h>
#include <tools/gen.hxx>
#include <vcl/pointr.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>

namespace vcl
{

class VCL_DLLPUBLIC ITiledRenderable
{
protected:
    int mnTilePixelWidth, mnTilePixelHeight;
    int mnTileTwipWidth, mnTileTwipHeight;
public:
    virtual ~ITiledRenderable();

    /**
     * Paint a tile to a given VirtualDevice.
     *
     * Output parameters are measured in pixels, tile parameters are in
     * twips.
     */
    virtual void paintTile( VirtualDevice &rDevice,
                            int nOutputWidth,
                            int nOutputHeight,
                            int nTilePosX,
                            int nTilePosY,
                            long nTileWidth,
                            long nTileHeight ) = 0;

    /**
     * Get the document size in twips.
     */
    virtual Size getDocumentSize() = 0;

    /**
     * Set the document "part", i.e. slide for a slideshow, and
     * tab for a spreadsheet.
     */
    virtual void setPart( int nPart )
    {
        (void) nPart;
    }

    /**
     * Get the number of parts -- see setPart for further details.
     */
    virtual int getParts()
    {
        return 1;
    }

    /**
     * Get the currently displayed/selected part -- see setPart for further
     * details.
     */
    virtual int getPart()
    {
        return 0;
    }

    /**
     * Get the name of the currently displayed part, i.e. sheet in a spreadsheet
     * or slide in a presentation.
     */
    virtual OUString getPartName(int nPart)
    {
        (void) nPart;
        return OUString("");
    }

    /**
     * Get the hash of the currently displayed part, i.e. sheet in a spreadsheet
     * or slide in a presentation.
     */
    virtual OUString getPartHash(int nPart) = 0;

    /// @see lok::Document::setPartMode().
    virtual void setPartMode(int nPartMode)
    {
        (void) nPartMode;
    }

    /**
     * Setup various document properties that are needed for the document to
     * be renderable via tiled rendering.
     */
    virtual void initializeForTiledRendering(const css::uno::Sequence<css::beans::PropertyValue>& rArguments) = 0;

    /**
     * Posts a keyboard event on the document.
     *
     * @see lok::Document::postKeyEvent().
     */
    virtual void postKeyEvent(int nType, int nCharCode, int nKeyCode) = 0;

    /**
     * Posts a mouse event on the document.
     *
     * @see lok::Document::postMouseEvent().
     */
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount, int nButtons, int nModifier) = 0;

    /**
     * Sets the start or end of a text selection.
     *
     * @see lok::Document::setTextSelection().
     */
    virtual void setTextSelection(int nType, int nX, int nY) = 0;

    /**
     * Gets the text selection.
     *
     * @see lok::Document::getTextSelection().
     */
    virtual OString getTextSelection(const char* pMimeType, OString& rUsedMimeType) = 0;

    /**
     * Adjusts the graphic selection.
     *
     * @see lok::Document::setGraphicSelection().
     */
    virtual void setGraphicSelection(int nType, int nX, int nY) = 0;

    /**
     * @see lok::Document::resetSelection().
     */
    virtual void resetSelection() = 0;

    /**
     * @see lok::Document::getPartPageRectangles().
     */
    virtual OUString getPartPageRectangles()
    {
        return OUString();
    }

    /**
     * Get position and content of row/column headers of Calc documents.
     *
     * @param rRectangle - if not empty, then limit the output only to the area of this rectangle
     * @return a JSON describing position/content of rows/columns
     */
    virtual OUString getRowColumnHeaders(const Rectangle& /*rRectangle*/)
    {
        return OUString();
    }

    /**
     * Get position and size of cell cursor in Calc.
     * (This could maybe also be used for tables in Writer/Impress in future?)
     */
    virtual OString getCellCursor(int /*nOutputWidth*/,
                                  int /*nOutputHeight*/,
                                  long /*nTileWidth*/,
                                  long /*nTileHeight*/)
    {
        return OString();
    }

    virtual Pointer getPointer() = 0;

    /// Sets the clipboard of the component.
    virtual void setClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& xClipboard) = 0;

    /// If the current contents of the clipboard is something we can paste.
    virtual bool isMimeTypeSupported() = 0;

    /**
     * Save the client's view so that we can compute the right zoom level
     * for the mouse events.
     * @param nTilePixelWidth - tile width in pixels
     * @param nTilePixelHeight - tile height in pixels
     * @param nTileTwipWidth - tile width in twips
     * @param nTileTwipHeight - tile height in twips
     */
    virtual void setClientZoom(int /*nTilePixelWidth*/,
                               int /*nTilePixelHeight*/,
                               int /*nTileTwipWidth*/,
                               int /*nTileTwipHeight*/)
    {
    }

    /// @see lok::Document::setClientVisibleArea().
    virtual void setClientVisibleArea(const Rectangle& /*rRectangle*/)
    {
    }
};

} // namespace vcl

#endif // INCLUDED_VCL_ITILEDRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
