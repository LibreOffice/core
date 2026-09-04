/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sfx2/viewsh.hxx>
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/ptrstyle.hxx>
#include <vcl/vclptr.hxx>
#include <map>
#include <string_view>
#include <vector>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <basegfx/range/b2drange.hxx>

#include <COKit/COKit.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::datatransfer::clipboard { class XClipboard; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace cpo::uno { template <typename > class Sequence; }
namespace vcl { class Window; }
namespace tools { class JsonWriter; }

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
    extern const std::map <PointerStyle, OString> gaKitPointerMap;


class VCL_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") ITiledRenderable
{
public:

    typedef std::map<OUString, OUString>  StringMap;

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
                            tools::Long nTileWidth,
                            tools::Long nTileHeight ) = 0;

    /**
     * Get the document size in twips.
     */
    virtual Size getDocumentSize() = 0;

    /**
     * Gets the part size in TWIPs.
     *
     * nPart is the part number in 0-based indexing.
     */
    virtual Size getPartSize(int /*nPart*/)
    {
        return Size(1,1);
    }

    /**
     * Gets all parts' size in TWIPs.
     */
    virtual void getAllPartSize(::tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /**
     * Get the data area size (in Calc last column and row).
     */
    virtual Size getDataArea(long /*nPart*/)
    {
        return Size(1, 1);
    }

    virtual std::string getPrintRanges()
    {
        return {};
    }

    /**
     * Set the document "part", i.e. slide for a slideshow, and
     * tab for a spreadsheet.
     * bool bAllowChangeFocus - used to not disturb other users while editing when
     *                          setPart is used for tile rendering only
     */
    virtual void setPart( int /*nPart*/, bool /*bAllowChangeFocus*/ = true ) {}

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

    /// @see COKitDocument::setPartMode().
    virtual void setPartMode(COKitPartMode) {}

    /**
     * Get the currently used EditMode (supported in Impress).
     */
    virtual int getEditMode()
    {
        return 0;
    }

    /**
     * Set the currently used EditMode (supported in Impress).
     */
    virtual void setEditMode(int) {}

    /**
     * Say whether anything still draws from the model rather than from the
     * view object contacts.
     */
    virtual void setDrawnFromModel(bool /*bDrawnFromModel*/) {}

    /**
     * Setup various document properties that are needed for the document to
     * be renderable via tiled rendering.
     */
    virtual void initializeForTiledRendering(const cpo::uno::Sequence<css::beans::PropertyValue>& rArguments) = 0;

    /**
     * Posts a keyboard event on the document.
     *
     * @see COKitDocument::postKeyEvent().
     */
    virtual void postKeyEvent(COKitKeyEventType eType, int nCharCode, int nKeyCode) = 0;

    /**
     * Posts a mouse event on the document.
     *
     * @see COKitDocument::postMouseEvent().
     */
    virtual void postMouseEvent(COKitMouseEventType eType, int nX, int nY, int nCount, int nButtons,
                                int nModifier) = 0;

    /**
     * Sets the start or end of a text selection.
     *
     * @see COKitDocument::setTextSelection().
     */
    virtual void setTextSelection(COKitSetTextSelectionType eType, int nX, int nY) = 0;

    /**
     * Gets the selection as a transferable for later processing
     */
    virtual css::uno::Reference<css::datatransfer::XTransferable> getSelection() = 0;

    /**
     * Adjusts the graphic selection.
     *
     * @see COKitDocument::setGraphicSelection().
     */
    virtual void setGraphicSelection(COKitSetGraphicSelectionType eType, int nX, int nY) = 0;

    /**
     * @see COKitDocument::resetSelection().
     */
    virtual void resetSelection() = 0;

    /**
     * @see COKitDocument::getWriterPageRectangles().
     */
    virtual std::string getWriterPageRectangles()
    {
        return {};
    }

    /**
     * Get position and content of row/column headers of Calc documents.
     *
     * @param rRectangle - if not empty, then limit the output only to the area of this rectangle
     * @return a JSON describing position/content of rows/columns
     */
    virtual void getRowColumnHeaders(const tools::Rectangle& /*rRectangle*/, tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /**
     * Generates a serialization of the active (Calc document) sheet's geometry data.
     *
     * @param bColumns - if true, the column widths/hidden/filtered/groups data
     *     are included depending on the settings of the flags bSizes, bHidden,
     *     bFiltered and bGroups.
     * @param bRows - if true, the row heights/hidden/filtered/groups data
     *     are included depending on the settings of the flags bSizes, bHidden,
     *     bFiltered and bGroups.
     * @bSizes - if true, the column-widths and/or row-heights data (represented as a list of spans)
     *     are included depending on the settings of the flags bColumns and bRows.
     * @bHidden - if true, the hidden columns and/or rows data (represented as a list of spans)
     *     are included depending on the settings of the flags bColumns and bRows.
     * @bFiltered - if true, the filtered columns and/or rows data (represented as a list of spans)
     *     are included depending on the settings of the flags bColumns and bRows.
     * @bGroups - if true, the column grouping and/or row grouping data
     *     are included depending on the settings of the flags bColumns and bRows.
     * @return serialization of the active sheet's geometry data as OString.
     */
    virtual std::string getSheetGeometryData(bool /*bColumns*/, bool /*bRows*/, bool /*bSizes*/,
                                         bool /*bHidden*/, bool /*bFiltered*/, bool /*bGroups*/)
    {
        return {};
    }

    /**
     * Get position and size of cell cursor in Calc - as JSON in the
     * current' views' co-ordinate system.
     * (This could maybe also be used for tables in Writer/Impress in future?)
     */
    virtual void getCellCursor(tools::JsonWriter& /*rJsonWriter*/)
    {
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

    /**
     * Provide the zoom level that will used during save/export
     *
     * @param nExportZoom - the zoom level as a percent
     */
    virtual void setExportZoom(int /*nExportZoom*/)
    {
    }

    /// @see COKitDocument::setClientVisibleArea().
    virtual void setClientVisibleArea(const tools::Rectangle& /*rRectangle*/)
    {
    }

    /**
     * Remember the zoom level as a percent for canvas page and
     * non-canvas page
     *
     * @param nZoom - the zoom level as a percent
     */
    virtual void setPageZoom(int /*nPageZoom*/)
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
    /// COKitDocument::getCommandValues(".uno:AcceptTrackedChanges") when there
    /// is no matching UNO API.
    virtual void getTrackedChanges(tools::JsonWriter&)
    {
    }

    /// Implementation for
    /// COKitDocument::getCommandValues(".uno:TrackedChangeAuthors").
    virtual void getTrackedChangeAuthors(tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /// Implementation for
    /// COKitDocument::getCommandValues(".uno:ViewAnnotations");
    virtual void getPostIts(tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /// Implementation for
    /// COKitDocument::getCommandValues(".uno:ViewAnnotationsPosition");
    virtual void getPostItsPos(tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /// Implementation for
    /// COKitDocument::getCommandValues(".uno:RulerState");
    virtual void getRulerState(tools::JsonWriter& /*rJsonWriter*/)
    {
    }

    /*
     * Used for sheets in spreadsheet documents,
     * and slides in presentation documents.
     */
    virtual std::string getPartInfo(int /*nPart*/)
    {
        return {};
    }

    /*
     * The stable identifier of the part at the given index. For a presentation or
     * drawing document that is the page's GUID as a braced string like
     * {1BE1A269-4A03-4202-ACFE-0204C5E9BE1F}, kept over part moves, insertions and
     * deletions of other parts, and persistent across sessions when the file format
     * stores it; nMode selects the page list the index addresses: 0 for the standard
     * pages, 1 for the master pages, 2 for the notes pages, 3 for the notes master
     * pages, 4 for the handout master page. For the other document types the
     * identifier is the index itself in decimal form. Empty when there is no such
     * part.
     */
    virtual OString getPartId(int nPart, int /*nMode*/)
    {
        if (nPart < 0)
            return OString();

        return OString::number(nPart);
    }

    /*
     * The index the part with the given identifier holds now in the part list nMode
     * selects, or -1 when no part carries that identifier. For a document whose part
     * identifiers are decimal indexes the identifier converts back to the index it
     * spells.
     */
    virtual int getPartIndex(std::string_view rPartId, int /*nMode*/)
    {
        // Nine digits keep the converted value inside the int range.
        if (rPartId.empty() || rPartId.size() > 9)
            return -1;

        int nIndex = 0;
        for (const char c : rPartId)
        {
            if (c < '0' || c > '9')
                return -1;
            nIndex = nIndex * 10 + (c - '0');
        }
        return nIndex;
    }

    /**
     * Select/Unselect a document "part", i.e. slide for a slideshow, and
     * tab for a spreadsheet(?).
     * nSelect: 0 to deselect, 1 to select, and 2 to toggle.
     */
    virtual void selectPart(int /*nPart*/, int /*nSelect*/) {}

    /**
     * Move selected pages/slides to a new position.
     * nPosition: the new position to move to.
     * bDuplicate: to copy (true), or to move (false).
     * nIntoSection: when >= 0, the section at that index will be re-anchored
     * to the first moved slide; pass -1 to keep default section anchoring.
     */
    virtual void moveSelectedParts(int /*nPosition*/, bool /*bDuplicate*/, int /*nIntoSection*/) {}

    /// @see COKitDocument::completeFunction().
    virtual void completeFunction(const OUString& /*rFunctionName*/)
    {
    }

    /**
     * It can happen that the underlying implementation is being disposed, but
     * somebody is trying to access the data...
     */
    virtual bool isDisposed() const
    {
        return false;
    }

    /**
     * Execute a form field event in the document.
     * E.g. select an item from a drop down field's list.
     */
    virtual void executeFromFieldEvent(const StringMap&)
    {
    }

    /**
     * Returns the rectangles of the input search result JSON
     */
    virtual std::vector<basegfx::B2DRange> getSearchResultRectangles(const char* /*pPayload*/)
    {
        return std::vector<basegfx::B2DRange>();
    }

    /**
     * Execute a content control event in the document.
     * E.g. select a list item from a drop down content control.
     */
    virtual void executeContentControlEvent(const StringMap&) {}

    /**
     *  Allow / disable drawing current text edit (used in Impress for slide previews)
     */
    virtual void setPaintTextEdit(bool) {}

    /// Decides if it's OK to call getCommandValues(rCommand).
    virtual bool supportsCommand(std::u16string_view /*rCommand*/) { return false; }

    /// Returns a json mapping of the possible values for the given command.
    virtual void getCommandValues(tools::JsonWriter& /*rJsonWriter*/, std::string_view /*rCommand*/)
    {
    }

    /**
     * Returns an opaque string reflecting the render state of a component
     * eg. 'PD' - P for non-printing-characters, D for dark-mode.
     * @param pViewShell the view to get the options from, if nullptr the current view shell is used
     */
    virtual OString getViewRenderState(const SfxViewShell* = nullptr) { return rtl::OString(); }

    /** Return JSON structure filled with the information about the presentation (Impress only function) */
    virtual std::string getPresentationInfo(bool = false /*bAllyState*/) const
    {
        return {};
    }
    /** Creates a slide show renderer (Impress only function) */
    virtual bool createSlideRenderer(
        const OString& /*rSlideHash*/,
        sal_Int32 /*nSlideNumber*/, sal_Int32& /*nViewWidth*/, sal_Int32& /*nViewHeight*/,
        bool /*bRenderBackground*/, bool /*bRenderMasterPage*/)
   {
        return false;
   }

    /** render slideshow layer*/
    virtual bool renderNextSlideLayer(unsigned char* /*pBuffer*/, bool& /*bIsBitmapLayer*/, double& /*rScale*/, std::string& /*rJsonMsg*/)
    {
        return true;
    }

    /** Inserts pages of the presentation at rFileUrl into this document. rJsonOptions names
        the pages to take as "slides", an empty list taking every page, the slide to insert
        before as "at", whether the pages keep the design of the file they came from as
        "keepDesign", whether they stay linked to the document named by "source" as "link",
        and the time that source was last modified as "lastModifiedTime", recorded on each
        linked page (Impress only function) */
    virtual bool insertPagesFromFile(const OUString& /*rFileUrl*/, const OString& /*rJsonOptions*/)
    {
        return false;
    }

    /** Writes the pages of the document that are linked to a source document, grouped by the
        source they were made from (Impress only function) */
    virtual bool getSlideLinks(tools::JsonWriter& /*rJsonWriter*/) { return false; }

    /** Refreshes the pages linked to one source document from a local file holding the source
        pages, and returns how many pages were refreshed; -1 when no page is linked to that source
        or the file could not be read. rLastModifiedTime is the time the source was last modified
        now, recorded on each refreshed page. pNotUpdated, when it is given, takes the
        identifier of every page the file held no slide for (Impress only function) */
    virtual sal_Int32 refreshSlideLinks(const OUString& /*rSourceName*/,
                                        const OUString& /*rFileUrl*/,
                                        const OUString& /*rLastModifiedTime*/,
                                        std::vector<OString>* /*pNotUpdated*/ = nullptr)
    {
        return -1;
    }

    /** Takes the source document off the page at nIndex in the page list of the standard pages,
        which keeps the content it holds, and returns whether that page was linked to a source
        (Impress only function) */
    virtual bool breakSlideLink(sal_Int32 /*nIndex*/) { return false; }

    /** Writes the given pages out as a document of their own, in the order they are given.
        The pages are named by their index in the page list of the standard pages, and an
        empty list writes every page. rFileUrl names the file to write and its extension
        chooses the filter (Impress only function) */
    virtual bool exportPages(const std::vector<sal_Int32>& /*rPages*/,
                             const OUString& /*rFileUrl*/)
    {
        return false;
    }
};
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
