//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import Foundation
import UIKit
import QuartzCore


/// The Document class represents one loaded document instance
/// Obtained through LibreOffice.documentLoad()
open class Document
{
    private let pDoc: UnsafeMutablePointer<LibreOfficeKitDocument>
    private let docClass: LibreOfficeKitDocumentClass

    internal init(pDoc: UnsafeMutablePointer<LibreOfficeKitDocument>)
    {
        self.pDoc = pDoc
        self.docClass = pDoc.pointee.pClass.pointee
    }

    /**
     * Stores the document's persistent data to a URL and
     * continues to be a representation of the old URL.
     *
     * @param pUrl the location where to store the document
     * @param pFormat the format to use while exporting, when omitted, then deducted from pURL's extension
     * @param pFilterOptions options for the export filter, e.g. SkipImages.
     *        Another useful FilterOption is "TakeOwnership".  It is consumed
     *        by the saveAs() itself, and when provided, the document identity
     *        changes to the provided pUrl - meaning that '.uno:ModifiedStatus'
     *        is triggered as with the "Save As..." in the UI.
     *        "TakeOwnership" mode must not be used when saving to PNG or PDF.
     */
    public func saveAs(url: String, format: String? = nil, filterOptions: String? = nil) -> Bool
    {
        return docClass.saveAs(pDoc, url, format, filterOptions) != 0
    }

    /**
     * Get document type.
     *
     * @since LibreOffice 6.0
     * @return an element of the LibreOfficeKitDocumentType enum.
     */
    public func getDocumentType() -> LibreOfficeKitDocumentType
    {
        return LibreOfficeKitDocumentType(rawValue: LibreOfficeKitDocumentType.RawValue(docClass.getDocumentType(pDoc)))
    }

    /**
     * Get number of part that the document contains.
     *
     * Part refers to either individual sheets in a Calc, or slides in Impress,
     * and has no relevance for Writer.
     */
    public func getParts() -> Int32
    {
        return docClass.getParts(pDoc);
    }

    public func initializeForRendering()
    {
        docClass.initializeForRendering(pDoc, "") // TODO: arguments??
    }

    /**
     * Get the logical rectangle of each part in the document.
     *
     * A part refers to an individual page in Writer and has no relevant for
     * Calc or Impress.
     *
     * @return a rectangle list, using the same format as
     * LOK_CALLBACK_TEXT_SELECTION.
     */
    public func getPartRectangles() -> String
    {
        return toString( docClass.getPartPageRectangles(pDoc) ) ?? ""

        // TODO: convert to CGRects? Comes out like "284, 284, 11906, 16838; 284, 17406, 11906, 16838; 284, 34528, 11906, 16838"

    }

    /// Get the current part of the document.
    public func getPart() -> Int32
    {
        return docClass.getPart(pDoc);
    }

    /// Set the current part of the document.
    public func setPart( nPart: Int32 )
    {
        docClass.setPart(pDoc, nPart);
    }

    /// Get the current part's name.
    public func getPartName( nPart: Int32) -> String?
    {
        return toString( docClass.getPartName(pDoc, nPart) )

    }

    /// Get the current part's hash.
    public func getPartHash( nPart: Int32 ) -> String?
    {
        return toString( docClass.getPartHash(pDoc, nPart) )

    }

    public func setPartMode( nMode: Int32 )
    {
        docClass.setPartMode( pDoc, nMode);
    }

    /**
     * Renders a subset of the document to a pre-allocated buffer.
     *
     * Note that the buffer size and the tile size implicitly supports
     * rendering at different zoom levels, as the number of rendered pixels and
     * the rendered rectangle of the document are independent.
     *
     * @param rCGContext Core Graphics context, cast to a UnsafeMutableRawPointer
     * @param nCanvasWidth number of pixels in a row of pBuffer.
     * @param nCanvasHeight number of pixels in a column of pBuffer.
     * @param nTilePosX logical X position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTilePosY logical Y position of the top left corner of the rendered rectangle, in TWIPs.
     * @param nTileWidth logical width of the rendered rectangle, in TWIPs.
     * @param nTileHeight logical height of the rendered rectangle, in TWIPs.
     */
    public func paintTileToCGContext( rCGContext: UnsafeMutableRawPointer,
        canvasWidth: Int32,
        canvasHeight: Int32,
        tilePosX: Int32,
        tilePosY: Int32,
        tileWidth: Int32,
        tileHeight: Int32)
    {
        print("paintTile canvasWidth=\(canvasWidth) canvasHeight=\(canvasHeight) tilePosX=\(tilePosX) tilePosY=\(tilePosY) tileWidth=\(tileWidth) tileHeight=\(tileHeight) ")
        return docClass.paintTileToCGContext(pDoc, rCGContext, canvasWidth, canvasHeight,
                                             tilePosX, tilePosY, tileWidth, tileHeight);
    }

    /**
     * Renders a window (dialog, popup, etc.) with give id
     *
     * @param nWindowId
     * @param pBuffer Buffer with enough memory allocated to render any dialog
     * @param x x-coordinate from where the dialog should start painting
     * @param y y-coordinate from where the dialog should start painting
     * @param width The width of the dialog image to be painted
     * @param height The height of the dialog image to be painted
     */
    public func paintWindow( nWindowId: UInt32,
        pBuffer: UnsafeMutablePointer<UInt8>,
        x: Int32,
        y: Int32,
        width: Int32,
        height: Int32)
    {
        return docClass.paintWindow(pDoc, nWindowId, pBuffer, x, y, width, height);
    }

    /**
     * Posts a command to the window (dialog, popup, etc.) with given id
     *
     * @param nWindowid
     */
    public func postWindow( nWindowId: UInt32, nAction: Int32, data: String)
    {
        return docClass.postWindow(pDoc, nWindowId, nAction, data);
    }

    /**
     * Gets the tile mode: the pixel format used for the pBuffer of paintTile().
     *
     * @return an element of the LibreOfficeKitTileMode enum.
     */
    public func getTileMode() -> LibreOfficeKitTileMode
    {
        return LibreOfficeKitTileMode(rawValue: LibreOfficeKitTileMode.RawValue(docClass.getTileMode(pDoc)));
    }

    /// Get the document sizes in TWIPs.
    public func getDocumentSize() -> (Int, Int)
    {
        print(Thread.isMainThread)
        // long* pWidth, long* pHeight
        var pWidth: Int = 0
        var pHeight: Int = 0
        docClass.getDocumentSize(pDoc, &pWidth, &pHeight);
        return (pWidth, pHeight)
    }

    /**
     * Initialize document for rendering.
     *
     * Sets the rendering and document parameters to default values that are
     * needed to render the document correctly using tiled rendering. This
     * method has to be called right after documentLoad() in case any of the
     * tiled rendering methods are to be used later.
     *
     * Example argument string for text documents:
     *
     * {
     *     ".uno:HideWhitespace":
     *     {
     *         "type": "boolean",
     *         "value": "true"
     *     }
     * }
     *
     * @param pArguments arguments of the rendering
     */
    public func initializeForRendering(arguments: String? = nil)
    {
        docClass.initializeForRendering(pDoc, arguments);
    }

    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    @discardableResult
    public func registerCallback( callback: @escaping LibreOfficeCallback ) -> Int
    {
        let ret = Callbacks.register(callback: callback)
        let pointer = UnsafeMutableRawPointer(bitPattern: ret)
        docClass.registerCallback(pDoc, callbackFromLibreOffice, pointer)
        return ret
    }

    /**
     * Posts a keyboard event to the focused frame.
     *
     * @param nType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    public func postKeyEvent(nType: Int32, nCharCode: Int32, nKeyCode: Int32)
    {
        docClass.postKeyEvent(pDoc, nType, nCharCode, nKeyCode);
    }

    /**
     * Posts a keyboard event to the dialog
     *
     * @param nWindowId
     * @param nType Event type, like press or release.
     * @param nCharCode contains the Unicode character generated by this event or 0
     * @param nKeyCode contains the integer code representing the key of the event (non-zero for control keys)
     */
    public func postWindowKeyEvent( nWindowId: UInt32, nType: Int32, nCharCode: Int32, nKeyCode: Int32)
    {
        docClass.postWindowKeyEvent(pDoc, nWindowId, nType, nCharCode, nKeyCode);
    }

    /**
     * Posts a mouse event to the document.
     *
     * @param nType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    public func postMouseEvent( nType: Int32, nX: Int32, nY: Int32, nCount: Int32, nButtons: Int32, nModifier: Int32)
    {
        docClass.postMouseEvent(pDoc, nType, nX, nY, nCount, nButtons, nModifier);
    }

    /**
     * Posts a mouse event to the window with given id.
     *
     * @param nWindowId
     * @param nType Event type, like down, move or up.
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     * @param nCount number of clicks: 1 for single click, 2 for double click
     * @param nButtons: which mouse buttons: 1 for left, 2 for middle, 4 right
     * @param nModifier: which keyboard modifier: (see include/vcl/vclenum.hxx for possible values)
     */
    public func postWindowMouseEvent(nWindowId: UInt32, nType: Int32, nX: Int32, nY: Int32, nCount: Int32, nButtons: Int32, nModifier: Int32)
    {
        docClass.postWindowMouseEvent(pDoc, nWindowId, nType, nX, nY, nCount, nButtons, nModifier);
    }

    /**
     * Posts a UNO command to the document.
     *
     * Example argument string:
     *
     * {
     *     "SearchItem.SearchString":
     *     {
     *         "type": "string",
     *         "value": "foobar"
     *     },
     *     "SearchItem.Backward":
     *     {
     *         "type": "boolean",
     *         "value": "false"
     *     }
     * }
     *
     * @param pCommand uno command to be posted to the document, like ".uno:Bold"
     * @param pArguments arguments of the uno command.
     */
    public func postUnoCommand(command: String, arguments: String? = nil, notifyWhenFinished: Bool = false)
    {
        docClass.postUnoCommand(pDoc, command, arguments, notifyWhenFinished);
    }

    /**
     * Sets the start or end of a text selection.
     *
     * @param nType @see LibreOfficeKitSetTextSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    public func setTextSelection( nType: Int32, nX: Int32, nY: Int32)
    {
        docClass.setTextSelection(pDoc, nType, nX, nY);
    }

    /**
     * Gets the currently selected text.
     *
     * @param pMimeType suggests the return format, for example text/plain;charset=utf-8.
     * @param pUsedMimeType output parameter to inform about the determined format (suggested one or plain text).
     */
    // FIXME - work out how to use an inout param for usedMimeType
    public func getTextSelection(mimeType: String, usedMimeType: UnsafeMutablePointer<UnsafeMutablePointer<Int8>?>? = nil) -> String?
    {
        return toString( docClass.getTextSelection(pDoc, mimeType, usedMimeType) );
    }

    /**
     * Pastes content at the current cursor position.
     *
     * @param pMimeType format of pData, for example text/plain;charset=utf-8.
     * @param pData the actual data to be pasted.
     * @return if the supplied data was pasted successfully.
     */
    public func paste(mimeType: String, data: String, size: Int) -> Bool
    {
        return docClass.paste(pDoc, mimeType, data, size);
    }

    /**
     * Adjusts the graphic selection.
     *
     * @param nType @see LibreOfficeKitSetGraphicSelectionType
     * @param nX horizontal position in document coordinates
     * @param nY vertical position in document coordinates
     */
    public func setGraphicSelection( nType: Int32, nX: Int32, nY: Int32)
    {
        docClass.setGraphicSelection(pDoc, nType, nX, nY);
    }

    /**
     * Gets rid of any text or graphic selection.
     */
    public func resetSelection()
    {
        docClass.resetSelection(pDoc);
    }

    /**
     * Returns a json mapping of the possible values for the given command
     * e.g. {commandName: ".uno:StyleApply", commandValues: {"familyName1" : ["list of style names in the family1"], etc.}}
     * @param pCommand a UNO command for which the possible values are requested
     * @return {commandName: unoCmd, commandValues: {possible_values}}
     */
    public func getCommandValues(command: String) -> String?
    {
        return toString(docClass.getCommandValues(pDoc, command));
    }

    /**
     * Save the client's view so that we can compute the right zoom level
     * for the mouse events. This only affects CALC.
     * @param nTilePixelWidth - tile width in pixels
     * @param nTilePixelHeight - tile height in pixels
     * @param nTileTwipWidth - tile width in twips
     * @param nTileTwipHeight - tile height in twips
     */
    public func setClientZoom(
        nTilePixelWidth: Int32,
        nTilePixelHeight: Int32,
        nTileTwipWidth: Int32,
        nTileTwipHeight: Int32)
    {
        docClass.setClientZoom(pDoc, nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);
    }

    /**
     * Inform core about the currently visible area of the document on the
     * client, so that it can perform e.g. page down (which depends on the
     * visible height) in a sane way.
     *
     * @param nX - top left corner horizontal position
     * @param nY - top left corner vertical position
     * @param nWidth - area width
     * @param nHeight - area height
     */
    public func setClientVisibleArea( nX: Int32, nY: Int32, nWidth: Int32, nHeight: Int32)
    {
        docClass.setClientVisibleArea(pDoc, nX, nY, nWidth, nHeight);
    }

    /**
     * Show/Hide a single row/column header outline for Calc documents.
     *
     * @param bColumn - if we are dealing with a column or row group
     * @param nLevel - the level to which the group belongs
     * @param nIndex - the group entry index
     * @param bHidden - the new group state (collapsed/expanded)
     */
    public func setOutlineState( column: Bool, level: Int32, index: Int32, hidden: Bool)
    {
        docClass.setOutlineState(pDoc, column, level, index, hidden);
    }

    /**
     * Create a new view for an existing document.
     * By default a loaded document has 1 view.
     * @return the ID of the new view.
     */
    public func createView() -> Int32
    {
        return docClass.createView(pDoc);
    }

    /**
     * Destroy a view of an existing document.
     * @param nId a view ID, returned by createView().
     */
    public func destroyView( id: Int32 )
    {
        docClass.destroyView(pDoc, id);
    }

    /**
     * Set an existing view of an existing document as current.
     * @param nId a view ID, returned by createView().
     */
    public func setView(id: Int32)
    {
        docClass.setView(pDoc, id);
    }

    /**
     * Get the current view.
     * @return a view ID, previously returned by createView().
     */
    public func getView() -> Int32
    {
        return docClass.getView(pDoc);
    }

    /**
     * Get number of views of this document.
     */
    public func getViewsCount() -> Int32
    {
        return docClass.getViewsCount(pDoc);
    }

    /**
     * Paints a font name or character if provided to be displayed in the font list
     * @param pFontName the font to be painted
     */
    // TODO
//    public func renderFont(fontName: String,
//        const char *pChar,
//        int *pFontWidth,
//        int *pFontHeight)
//    {
//        return docClass.renderFont(pDoc, pFontName, pChar, pFontWidth, pFontHeight);
//    }

    /**
     * Renders a subset of the document's part to a pre-allocated buffer.
     *
     * @param nPart the part number of the document of which the tile is painted.
     * @see paintTile.
     */
    public func paintPartTile(pBuffer: UnsafeMutablePointer<UInt8>,
        nPart: Int32,
        nCanvasWidth: Int32,
        nCanvasHeight: Int32,
        nTilePosX: Int32,
        nTilePosY: Int32,
        nTileWidth: Int32,
        nTileHeight: Int32)
    {
        return docClass.paintPartTile(pDoc, pBuffer, nPart,
            nCanvasWidth, nCanvasHeight,
            nTilePosX, nTilePosY,
            nTileWidth, nTileHeight);
    }

    /**
     * Returns the viewID for each existing view. Since viewIDs are not reused,
     * viewIDs are not the same as the index of the view in the view array over
     * time. Use getViewsCount() to know the minimal nSize that's large enough.
     *
     * @param pArray the array to write the viewIDs into
     * @param nSize the size of pArray
     * @returns true if pArray was large enough and result is written, false
     * otherwise.
     */
//    bool getViewIds(int* pArray,
//    size_t nSize)
//    {
//    return docClass.getViewIds(pDoc, pArray, nSize);
//    }

    /**
     * Set the language tag of the window with the specified nId.
     *
     * @param nId a view ID, returned by createView().
     * @param language Bcp47 languageTag, like en-US or so.
     */
    public func setViewLanguage( id: Int32, language: String)
    {
        docClass.setViewLanguage(pDoc, id, language);
    }

    public func invokeHandlers()
    {
        // The app seems to work even without this? Or maybe I
        // just didn't test hard enough?
        // temporaryHackToInvokeCallbackHandlers(pDoc)

    }

}

/**
 * iOS friendly extensions of Document.
 * TODO: move me back to the framework.
 */
public extension Document
{
    public func getDocumentSizeAsCGSize() -> CGSize
    {
        let (x,y) = self.getDocumentSize()
        return CGSize(width: x, height: y)
    }

    public func paintTileToCurrentContext(canvasSize: CGSize,
                                          tileRect: CGRect)
    {
        let ctx = UIGraphicsGetCurrentContext()
        //print(ctx!)
        let ptr = unsafeBitCast(ctx, to: UnsafeMutableRawPointer.self)
        //print(ptr)

        self.paintTileToCGContext(rCGContext:ptr,
                                  canvasWidth: Int32(canvasSize.width),
                                  canvasHeight: Int32(canvasSize.height),
                                  tilePosX: Int32(tileRect.minX),
                                  tilePosY: Int32(tileRect.minY),
                                  tileWidth: Int32(tileRect.size.width),
                                  tileHeight: Int32(tileRect.size.height))
    }

    public func paintTileToImage(canvasSize: CGSize,
                                 tileRect: CGRect) -> UIImage?
    {

        UIGraphicsBeginImageContextWithOptions(canvasSize, false, 1.0)
        let _ = UIGraphicsGetCurrentContext()!

        self.paintTileToCurrentContext(canvasSize: canvasSize, tileRect: tileRect)
        let image = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        return image
    }
}

