//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import Foundation
import UIKit




/// Serves the same purpose as the LOKitThread in the Android project - sequentialises all access to LOKit on a background thread, off the UI thread.
/// It's a singleton, and keeps a single instance of LibreOfficeKit
/// Public methods may be called from any thread, and will dispatch their work onto the held sequential queue.
/// TODO: move me to framework
public class LOKitThread
{
    public static let instance = LOKitThread() // statics are lazy and thread safe in swift, so no need for anything more complex


    fileprivate let queue = SingleThreadedQueue(name: "LOKitThread.queue")

    /// singleton LibreOffice instance. Can only be accessed through the queue.
    var libreOffice: LibreOffice! = nil // initialised in didFinishLaunchingWithOptions

    public weak var delegate: LOKitUIDelegate? = nil
    public weak var progressDelegate: ProgressDelegate? = nil

    private init()
    {

        async {
            self.libreOffice = try! LibreOffice() // will blow up the app if it throws, but fair enough

            // hook up event handler
            self.libreOffice.registerCallback(callback: self.onLOKEvent)

        }
    }

    private func onLOKEvent(type: LibreOfficeKitCallbackType, payload: String?)
    {
        //LibreOfficeLight.LibreOfficeKitKeyEventType.
        print("onLOKEvent type:\(type) payload:\(payload ?? "")")

        switch type
        {
        case LOK_CALLBACK_STATUS_INDICATOR_START:
            runOnMain {
                self.progressDelegate?.statusIndicatorStart()
            }

        case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
            runOnMain {
                if let doub = Double(payload ?? "")
                {
                    self.progressDelegate?.statusIndicatorSetValue(value: doub)
                }
            }

        case LOK_CALLBACK_STATUS_INDICATOR_FINISH:
            runOnMain {
                self.progressDelegate?.statusIndicatorFinish()
            }
        default:
             print("onLOKEvent type:\(type) not handled!")
        }
    }

    /// Run the task on the serial queue, and return immediately
    public func async(_ runnable: @escaping Runnable)
    {
        queue.async( runnable)
    }

    /// Run the task on the serial queue, and block to get the result
    /// Careful of deadlocking!
    public func sync<R>( _ closure: @escaping () -> R ) -> R
    {
        let ret = queue.sync( closure )
        return ret
    }

    public func withLibreOffice( _ closure: @escaping (LibreOffice) -> ())
    {
        async {
            closure(self.libreOffice)
        }
    }

    /// Loads a document, and calls the callback with a wrapper if successful, or an error if not.
    public func documentLoad(url: String, callback: @escaping (DocumentHolder?, Error?) -> ())
    {
        withLibreOffice
        {
            lo in

            do
            {
                // this is trying to avoid null context errors which pop up on doc init
                // doesn't seem to fix
                UIGraphicsBeginImageContext(CGSize(width:1,height:1))
                let doc = try lo.documentLoad(url: url)
                print("Opened document: \(url)")
                doc.initializeForRendering()
                UIGraphicsEndImageContext()

                callback(DocumentHolder(doc: doc), nil)
            }
            catch
            {
                print("Failed to load document: \(error)")
                callback(nil, error)
            }
        }
    }
}


open class CachedRender
{
    open let canvasSize: CGSize
    open let tileRect: CGRect
    open let image: UIImage

    public init(canvasSize: CGSize, tileRect: CGRect, image: UIImage)
    {
        self.canvasSize = canvasSize
        self.tileRect = tileRect
        self.image = image
    }
}

class RenderCache
{
    let CACHE_LOWMEM = 4
    let CACHE_NORMAL = 20

    var cachedRenders: [CachedRender] = []
    var hasReceivedMemoryWarning = false

    let lock = NSRecursiveLock()

    func emptyCache()
    {
        lock.lock(); defer { lock.unlock() }

        cachedRenders.removeAll()

    }

    func pruneCache()
    {
        lock.lock(); defer { lock.unlock() }

        let max = hasReceivedMemoryWarning ? CACHE_LOWMEM : CACHE_NORMAL
        while cachedRenders.count > max
        {
            cachedRenders.remove(at: 0)
        }
    }

    func add(cachedRender: CachedRender)
    {
        lock.lock(); defer { lock.unlock() }

        cachedRenders.append(cachedRender)
        pruneCache()
    }

    func get(canvasSize: CGSize, tileRect: CGRect) -> UIImage?
    {
        lock.lock(); defer { lock.unlock() }

        if let cr = cachedRenders.first(where: { $0.canvasSize == canvasSize && $0.tileRect == tileRect })
        {
            return cr.image
        }
        return nil
    }

}

/**
 * Holds the document object so to enforce access in a thread safe way.
 */
public class DocumentHolder
{
    private let doc: Document

    public weak var delegate: DocumentUIDelegate? = nil
    public weak var searchDelegate: SearchDelegate? = nil

    private let cache = RenderCache()

    init(doc: Document)
    {
        self.doc = doc
        doc.registerCallback() {
            [weak self] typ, payload in
            self?.onDocumentEvent(type: typ, payload: payload)
        }
    }

    /// Gives async access to the document
    public func async(_ closure: @escaping (Document) -> ())
    {
        LOKitThread.instance.async
        {
            closure(self.doc)
        }
    }

    /// Gives sync access to the document - blocks until the closure runs.
    /// Careful of deadlocks.
    public func sync<R>( _ closure: @escaping (Document) -> R ) -> R
    {
        return LOKitThread.instance.sync
        {
            return closure(self.doc)
        }
    }

    /// Paints a tile and return synchronously, using a cached version if it can
    public func paintTileToImage(canvasSize: CGSize,
                                 tileRect: CGRect) -> UIImage?
    {
        if let cached = cache.get(canvasSize: canvasSize, tileRect: tileRect)
        {
            return cached
        }

        let img = sync {
            $0.paintTileToImage(canvasSize: canvasSize, tileRect: tileRect)
        }
        if let image = img
        {
            cache.add(cachedRender: CachedRender(canvasSize: canvasSize, tileRect: tileRect, image: image))
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

                /*
                if let srs = searchResults.searchResultSelection
                {
                    for par in srs
                    {
                        print("\(par.rectsAsCGRects)")
                    }
                }
                */

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
    //let encoder = JSONEncoder()

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
        let coords = rectStr.split(separator: ",").flatMap { Double($0.trimmingCharacters(in: .whitespacesAndNewlines)) }
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

/**
 * Delegate methods for global events emitted from LOKit.
 * Mostly dispatched on the main thread unless noted.
 */
public protocol LOKitUIDelegate: class
{
    // Nothing ATM..
}

public protocol ProgressDelegate: class
{
    func statusIndicatorStart()

    func statusIndicatorFinish()

    func statusIndicatorSetValue(value: Double)
}


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
