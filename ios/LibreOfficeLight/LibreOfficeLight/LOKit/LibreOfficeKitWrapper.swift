//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

import Foundation


public struct LibreOfficeError: Error
{
    let message: String
    public init(_ message: String)
    {
        self.message = message
    }
}

public typealias LibreOfficeCallback = (_ type: LibreOfficeKitCallbackType, _ payload: String?) -> ()

func callbackFromLibreOffice(nType: Int32, payload: UnsafePointer<Int8>?, pData: UnsafeMutableRawPointer?)
{
    if let val = pData?.hashValue
    {
        if let theFunc = Callbacks.callbackRegister[val]
        {
            let payString = toString(payload)
            theFunc(LibreOfficeKitCallbackType(rawValue: LibreOfficeKitCallbackType.RawValue(nType)), payString)
        }
        else
        {
            print("Unknown callback: \(val)")
        }
    }
    else
    {
        print("callbackFromLibreOffice, but pData was nil")
    }
}


internal struct Callbacks
{
    static var count = 0
    static var callbackRegister: Dictionary<Int, LibreOfficeCallback> = [:]

    static func register(callback: @escaping LibreOfficeCallback) -> Int
    {
        count += 1
        let id = count
        callbackRegister[id] = callback
        return id

    }
}


open class LibreOffice
{
    private let pLok: UnsafeMutablePointer<LibreOfficeKit>
    private let lokClass: LibreOfficeKitClass

    public init() throws
    {
        let b = Bundle.init(for: LibreOffice.self)
        let path = b.bundlePath // not Bundle.main.bundlePath
        let docsDir = getDocumentsDirectory()
        let userDir = docsDir.appendingPathComponent("user")
        print("Calling BridgeLOkit_Init with \(path), \(userDir.path)" )
        BridgeLOkit_Init(path, userDir.path)
        let pLok = BridgeLOkit_getLOK()
        if let lokClass = pLok?.pointee.pClass?.pointee
        {
            self.pLok = pLok!
            self.lokClass = lokClass
            print("Loaded LibreOfficeKit: \(self.getVersionInfo() ?? "")")
            return
        }
        throw LibreOfficeError("Unable to init LibreOfficeKit")
    }

    /**
     * Get version information of the LOKit process
     *
     * @since LibreOffice 6.0
     * @returns JSON string containing version information in format:
     * {ProductName: <>, ProductVersion: <>, ProductExtension: <>, BuildId: <>}
     *
     * Eg: {"ProductName": "LibreOffice",
     * "ProductVersion": "5.3",
     * "ProductExtension": ".0.0.alpha0",
     * "BuildId": "<full 40 char git hash>"}
     */
    public func getVersionInfo() -> String?
    {
        if let pRet = lokClass.getVersionInfo(pLok)
        {
            return String(cString: pRet) // TODO: convert JSON
        }
        return nil
    }

    /**
     * Loads a document from an URL.
     *
     * @param pUrl the URL of the document to load
     * @param pFilterOptions options for the import filter, e.g. SkipImages.
     *        Another useful FilterOption is "Language=...".  It is consumed
     *        by the documentLoad() itself, and when provided, LibreOfficeKit
     *        switches the language accordingly first.
     * @since pFilterOptions argument added in LibreOffice 5.0
     */
    public func documentLoad(url: String) throws -> Document
    {
        if let pDoc = lokClass.documentLoad(pLok, url)
        {
            return Document(pDoc: pDoc)
        }
        throw LibreOfficeError("Unable to load document")
    }



    /// Returns the last error as a string
    public func getError() -> String?
    {
        if let cstr = lokClass.getError(pLok)
        {
            let ret = String(cString: cstr)
            lokClass.freeError(cstr)
            return ret
        }
        return nil
    }


    /**
     * Registers a callback. LOK will invoke this function when it wants to
     * inform the client about events.
     *
     * @since LibreOffice 6.0
     * @param pCallback the callback to invoke
     * @param pData the user data, will be passed to the callback on invocation
     */
    @discardableResult
    public func registerCallback( callback: @escaping LibreOfficeCallback ) -> Int
    {
        let ret = Callbacks.register(callback: callback)
        let pointer = UnsafeMutableRawPointer(bitPattern: ret)
        lokClass.registerCallback(pLok, callbackFromLibreOffice, pointer)
        return ret
    }

    /**
     * Returns details of filter types.
     *
     * Example returned string:
     *
     * {
     *     "writer8": {
     *         "MediaType": "application/vnd.oasis.opendocument.text"
     *     },
     *     "calc8": {
     *         "MediaType": "application/vnd.oasis.opendocument.spreadsheet"
     *     }
     * }
     *
     * @since LibreOffice 6.0
     */
    public func getFilterTypes() -> String?
    {
        return toString(lokClass.getFilterTypes(pLok));
    }

    /**
     * Set bitmask of optional features supported by the client.
     *
     * @since LibreOffice 6.0
     * @see LibreOfficeKitOptionalFeatures
     */
    public func setOptionalFeatures(features: UInt64)
    {
        return lokClass.setOptionalFeatures(pLok, features);
    }

    /**
     * Set password required for loading or editing a document.
     *
     * Loading the document is blocked until the password is provided.
     *
     * @param pURL      the URL of the document, as sent to the callback
     * @param pPassword the password, nullptr indicates no password
     *
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD, a valid password
     * will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD request,
     * and a NULL password will abort loading the document.
     *
     * In response to LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY, a valid
     * password will continue loading the document, an invalid password will
     * result in another LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY request,
     * and a NULL password will continue loading the document in read-only
     * mode.
     *
     * @since LibreOffice 6.0
     */
    public func setDocumentPassword(URL: String, password: String)
    {
        lokClass.setDocumentPassword(pLok, URL, password);
    }




    /**
     * Run a macro.
     *
     * Same syntax as on command line is permissible (ie. the macro:// URI forms)
     *
     * @since LibreOffice 6.0
     * @param pURL macro url to run
     */

    public func runMacro( URL: String ) -> Bool
    {
        return lokClass.runMacro( pLok, URL ) != 0;
    }
}

