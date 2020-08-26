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

/**
 * Holds the document object so to enforce access in a thread safe way.
 */
public class DocumentHolder
{
    private let doc: Document

    public weak var delegate: DocumentUIDelegate? = nil
    public weak var searchDelegate: SearchDelegate? = nil

    private let cache = RenderCache()

    public let documentType: LibreOfficeKitDocumentType
    public let documentSize: CGSize
    public let views: Int32
    public let parts: Int32
    public let partNames: [String]

    public private(set) var currentPart: Int32 = 0

    init(doc: Document)
    {
        self.doc = doc

        // we go and get a bunch of document properties and store them in properties
        // this allows easy access to these without threading issues
        // when we get to editing they will have to be invalidated

        self.documentType = doc.getDocumentType()
        documentSize = doc.getDocumentSizeAsCGSize()
        views = doc.getViewsCount()
        parts = doc.getParts()

        var partNames = [String]()
        for i in 0..<parts
        {
            let n = doc.getPartName(nPart: i) ?? ""
            partNames.append(n)
        }
        self.partNames = partNames

        doc.registerCallback() {
            [weak self] typ, payload in
            self?.onDocumentEvent(type: typ, payload: payload)
        }
    }

    public var isPresentation: Bool
    {
        return documentType == LOK_DOCTYPE_PRESENTATION
    }
    public var isText: Bool
    {
        return documentType == LOK_DOCTYPE_TEXT
    }
    public var isDrawing: Bool
    {
        return documentType == LOK_DOCTYPE_DRAWING
    }
    public var isSpreadsheet: Bool
    {
        return documentType == LOK_DOCTYPE_SPREADSHEET
    }

    /// Gives async access to the document
    public func async(_ closure: @escaping (Document) -> ())
    {
        LOKitThread.instance.async
        {
            closure(self.doc)
        }
        self.invokeHandlers()
    }

    public func invokeHandlers()
    {
        LOKitThread.instance.async
        {
            self.doc.invokeHandlers()
        }
    }

    /// Gives sync access to the document - blocks until the closure runs.
    /// Careful of deadlocks.
    public func sync<R>( _ closure: @escaping (Document) -> R ) -> R
    {
        return LOKitThread.instance.sync
        {
            self.invokeHandlers()
            return closure(self.doc)
        }
    }

    /// Paints a tile and return synchronously, using a cached version if it can
    public func paintTileToImage(canvasSize: CGSize,
                                 tileRect: CGRect) -> UIImage?
    {
        if let cached = cache.get(part: currentPart, canvasSize: canvasSize, tileRect: tileRect)
        {
            return cached
        }

        let img = sync {
            $0.paintTileToImage(canvasSize: canvasSize, tileRect: tileRect)
        }
        if let image = img
        {
            cache.add(cachedRender: CachedRender(part: currentPart, canvasSize: canvasSize, tileRect: tileRect, image: image))
        }

        return img
    }

    private func onDocumentEvent(type: LibreOfficeKitCallbackType, payload: String?)
    {
        print("onDocumentEvent type:\(type) payload:\(payload ?? "")")

        switch type
        {
        case LOK_CALLBACK_INVALIDATE_TILES:
            runOnMain {
                self.delegate?.invalidateTiles( rects: decodeRects(payload) )
            }
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            runOnMain {
                self.delegate?.invalidateVisibleCursor( rects: decodeRects(payload) )
            }
        case LOK_CALLBACK_TEXT_SELECTION:
            runOnMain {
                self.delegate?.textSelection( rects: decodeRects(payload) )
            }
        case LOK_CALLBACK_TEXT_SELECTION_START:
            runOnMain {
                self.delegate?.textSelectionStart( rects: decodeRects(payload) )
            }
        case LOK_CALLBACK_TEXT_SELECTION_END:
            runOnMain {
                self.delegate?.textSelectionEnd( rects: decodeRects(payload) )
            }

        case LOK_CALLBACK_SEARCH_NOT_FOUND:
            runOnMain {
                self.searchDelegate?.searchNotFound()
            }
        case LOK_CALLBACK_SEARCH_RESULT_SELECTION:
            runOnMain {
                self.searchResults(payload: payload)
            }

        case LOK_CALLBACK_SET_PART:
            if let p = payload, let newPart = Int32(p)
            {
                self.currentPart = newPart
                // TODO: callback?
            }

        case LOK_CALLBACK_STATE_CHANGED:
            // TODO: call backback
            print("onDocumentEvent type: LOK_CALLBACK_STATE_CHANGED: \(payload ?? "")")

        default:
            print("onDocumentEvent type:\(type) not handled!")
        }
    }

    private func searchResults(payload: String?)
    {
        if let d = payload, let data = d.data(using: .utf8)
        {
            let decoder = JSONDecoder()
            do
            {
                let searchResults = try decoder.decode(SearchResults.self, from: data )
                self.searchDelegate?.searchResultSelection(searchResults: searchResults)
            }
            catch
            {
                print("Error decoding payload: \(error)")
            }
        }
    }

    public func search(searchString: String, forwardDirection: Bool = true, from: CGPoint)
    {
        var rootJson = JSONObject()
        addProperty(&rootJson, "SearchItem.SearchString", "string", searchString);
        addProperty(&rootJson, "SearchItem.Backward", "boolean", String(!forwardDirection) );
        addProperty(&rootJson, "SearchItem.SearchStartPointX", "long", String(describing: from.x) );
        addProperty(&rootJson, "SearchItem.SearchStartPointY", "long", String(describing: from.y) );
        addProperty(&rootJson, "SearchItem.Command", "long", "0") // String.valueOf(0)); // search all == 1

        if let jsonStr = encode(json: rootJson)
        {
            async {
                $0.postUnoCommand(command: ".uno:ExecuteSearch", arguments: jsonStr, notifyWhenFinished: true)
            }
        }
    }

    public func incrementPart(amount: Int32)
    {
        async {
            document in
            let currentPart = document.getPart()
            let numParts = document.getParts()
            let newPart = currentPart + amount
            if (newPart < numParts && newPart > 0)
            {
                document.setPart(nPart: newPart)
            }
        }
    }
}

public typealias JSONObject = Dictionary<String, AnyObject>
public func addProperty( _ json: inout JSONObject, _ parentValue: String, _ type: String, _ value: String)
{
    var child = JSONObject();
    child["type"] = type as AnyObject
    child["value"] = value as AnyObject
    json[parentValue] = child as AnyObject
}

func encode(json: JSONObject) -> String?
{
    if let data = try? JSONSerialization.data(withJSONObject: json, options: .prettyPrinted)
    {
        return String(data: data, encoding: String.Encoding.utf8)
    }
    return nil
}

/// Decodes a series of rectangles in the form: "x, y, width, height; x, y, width, height"
public func decodeRects(_ payload: String?) -> [CGRect]?
{
    guard var pl = payload else { return nil }
    pl = pl.trimmingCharacters(in: .whitespacesAndNewlines )
    if pl == "EMPTY" || pl.count == 0
    {
        return nil
    }
    var ret = [CGRect]()
    for rectStr in pl.split(separator: ";")
    {
        let coords = rectStr.split(separator: ",").compactMap { Double($0.trimmingCharacters(in: .whitespacesAndNewlines)) }
        if coords.count == 4
        {
            let rect = CGRect(x: coords[0],
                              y: coords[1],
                              width: coords[2],
                              height: coords[3])
            ret.append( rect )
        }
    }
    return ret
}

// MARK :- Delegates

public protocol DocumentUIDelegate: class
{
    func invalidateTiles(rects: [CGRect]? )
    func invalidateVisibleCursor(rects: [CGRect]? )

    func textSelection(rects: [CGRect]? )
    func textSelectionStart(rects: [CGRect]? )
    func textSelectionEnd(rects: [CGRect]? )
}

public protocol SearchDelegate: class
{
    func searchNotFound()
    func searchResultSelection(searchResults: SearchResults)
}

/**
 Encodes this example json:
 {
 "searchString": "Office",
 "highlightAll": "true",
 "searchResultSelection": [
 {
 "part": "0",
 "rectangles": "1951, 10743, 627, 239"
 },
 {
 "part": "0",
 "rectangles": "5343, 9496, 627, 287"
 },
 {
 "part": "0",
 "rectangles": "1951, 9256, 627, 239"
 },
 {
 "part": "0",
 "rectangles": "6502, 5946, 626, 287"
 },
 {
 "part": "0",
 "rectangles": "6686, 5658, 627, 287"
 },
 {
 "part": "0",
 "rectangles": "4103, 5418, 573, 239"
 },
 {
 "part": "0",
 "rectangles": "1951, 5418, 627, 239"
 },
 {
 "part": "0",
 "rectangles": "4934, 1658, 1586, 559"
 }
 ]
 }
 */
public struct SearchResults: Codable
{
    public var searchString: String?
    public var highlightAll: String?
    public var searchResultSelection: Array<PartAndRectangles>?
}

public struct PartAndRectangles: Codable
{
    public var part: String?
    public var rectangles: String?

    public var rectsAsCGRects: [CGRect]? {
        return decodeRects(self.rectangles)
    }
}

