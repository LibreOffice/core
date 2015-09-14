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

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitTypes.h>
#include <tools/gen.hxx>
#include <vcl/virdev.hxx>

class SfxViewShell;

namespace vcl
{

class VCL_DLLPUBLIC ITiledRenderable
{
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

    /// @see lok::Document::setPartMode().
    virtual void setPartMode(int nPartMode)
    {
        (void) nPartMode;
    }

    /**
     * Setup various document properties that are needed for the document to
     * be renderable via tiled rendering.
     */
    virtual void initializeForTiledRendering() = 0;

    /**
     * Registers a callback that will be invoked whenever the tiled renderer
     * wants to notify the client about an event.
     *
     * @param pCallBack is the callback function
     * @param pData is private data of the client that will be sent back when the callback is invoked
     */
    virtual void registerCallback(LibreOfficeKitCallback pCallback, void* pData) = 0;

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
    virtual void postMouseEvent(int nType, int nX, int nY, int nCount) = 0;

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

    /// Get the currently active view shell of the document.
    virtual SfxViewShell* getCurrentViewShell()
    {
        return 0;
    }
};

} // namespace vcl

#endif // INCLUDED_VCL_ITILEDRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
