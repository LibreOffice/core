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



