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

#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/ptrstyle.hxx>
#include <vcl/vclptr.hxx>
#include <map>
#include <com/sun/star/datatransfer/XTransferable.hpp>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::datatransfer { namespace clipboard { class XClipboard; } }
namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace com::sun::star::uno { template <typename > class Sequence; }
namespace vcl { class Window; }

class VirtualDevice;

namespace vcl
{
    /*
     * Map directly to css cursor styles to avoid further mapping in the client.
     * Gtk (via gdk_cursor_new_from_name) also supports the same css cursor styles.
     *
     * This was created partially with help of the mappings in gtkdata.cxx.
     * The list is incomplete as some cursor style simply aren't supported
     * by css, it might turn out to be worth mapping some of these missing cursors
     * to available cursors?
     */
    extern const std::map <PointerStyle, OString> gaLOKPointerMap;


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
    virtual void setPart( int ) {}

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
    virtual OUString getPartName(int)
    {
        return OUString();
    }

    /**
     * Get the vcl::Window for the document being edited
     */
    virtual VclPtr<vcl::Window> getDocWindow() = 0;

    /**
     * Get the hash of the currently displayed part, i.e. sheet in a spreadsheet
     * or slide in a presentation.
     */
    virtual OUString getPartHash(int nPart) = 0;

    /// @see lok::Document::setPartMode().
    virtual void setPartMode(int) {}

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
     * Gets the selection as a transferable for later processing
     */
    virtual css::uno::Reference<css::datatransfer::XTransferable> getSelection() = 0;

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
    virtual OUString getRowColumnHeaders(const tools::Rectangle& /*rRectangle*/)
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

    virtual PointerStyle getPointer() = 0;

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
    {}

    /// @see lok::Document::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& /*rRectangle*/)
    {
    }

    /**
     * Show/Hide a single row/column header outline for Calc documents.
     *
     * @param bColumn - if we are dealing with a column or row group
     * @param nLevel - the level to which the group belongs
     * @param nIndex - the group entry index
     * @param bHidden - the new group state (collapsed/expanded)
     */
    virtual void setOutlineState(bool /*bColumn*/, int /*nLevel*/, int /*nIndex*/, bool /*bHidden*/)
    {
        return;
    }

    /// Implementation for
    /// lok::Document::getCommandValues(".uno:AcceptTrackedChanges") when there
    /// is no matching UNO API.
    virtual OUString getTrackedChanges()
    {
        return OUString();
    }

    /// Implementation for
    /// lok::Document::getCommandValues(".uno:TrackedChangeAuthors").
    virtual OUString getTrackedChangeAuthors()
    {
        return OUString();
    }

    /// Implementation for
    /// lok::Document::getCommandValues(".uno:ViewAnnotations");
    virtual OUString getPostIts()
    {
        return OUString();
    }

    /// Implementation for
    /// lok::Document::getCommandValues(".uno:ViewAnnotationsPosition");
    virtual OUString getPostItsPos()
    {
        return OUString();
    }

    /// Implementation for
    /// lok::Document::getCommandValues(".uno:RulerState");
    virtual OUString getRulerState()
    {
        return OUString();
    }

    /*
     * Used for sheets in spreadsheet documents.
     */
    virtual OUString getPartInfo(int /*nPart*/)
    {
        return OUString();
    }
};
} // namespace vcl

#endif // INCLUDED_VCL_ITILEDRENDERABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
