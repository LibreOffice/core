/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Cocoa
#if canImport(CryptoKit)
import CryptoKit
#endif
import PDFKit
import WebKit

/**
 * Represents a document in the application.
 */
class Document: NSDocument {

    // MARK: - Properties

    /// For the COWrapper to send the messages to the right file descriptor.
    @objc
    var fakeClientFd: Int32 = -1

    /// ID to identify the document to be able to get access to lok::Document (eg. for printing)
    @objc
    var appDocId: Int32 = -1

    /// The webview that contains the document.
    var webView: WKWebView!

    /// The URL of the temporary directory where the document's working files are stored.
    private var tempDirectoryURL: URL?

    /// The URL of the temporary file that represents the "live" version of the document.
    @objc
    var tempFileURL: URL?

    /// This is the welcome slideshow (that needs a special parameter when opening)
    var isWelcome = false

    /// This is a newly created document (from a template).
    var isNewDocument = false

    /// The document type (UTI) for this document, set explicitly for new documents
    var documentType: String?

    /** Parameters for a deferred Save / Save As… request. */
    private struct PendingSave {
        let url: URL
        let typeName: String
        let operation: NSDocument.SaveOperationType
        let completion: (((any Error)?) -> Void)
        /** Change-count token captured when the save started; used to clear only if nothing new happened. */
        let token: Any
    }

    /** Stashed Save / Save As… request while edits are still dirty (nil when none). */
    private var pendingSave: PendingSave?

    /// Make sure the isModified access can be atomic.
    private var modifiedLock = NSLock()
    private var _isModified: Bool = false

    /// Observe the fileURL changes, so that we can remember the window size per document.
    private var fileURLObservation: NSKeyValueObservation?

    /**
     * Modified status mirrored from the core.
     */
    var isModified: Bool {
        get {
            modifiedLock.lock()
            let value = _isModified
            modifiedLock.unlock()

            return value
        }
        set {
            // schedule UI updates after we release the lock
            var changeUpdate: (() -> Void)?

            modifiedLock.lock()

            let oldValue = _isModified
            _isModified = newValue

            // non-modified -> modified: Mark that the document became edited
            if !oldValue && newValue {
                updateChangeCount(.changeDone)
            }
            // modified -> non-modified: Clear the mark unless there is an ongoing saving operation
            else if oldValue && !newValue && pendingSave == nil {
                changeUpdate = { self.updateChangeCount(.changeCleared) }
            }

            modifiedLock.unlock()

            // change the "Edited" mark
            if let changeUpdate {
                DispatchQueue.main.async(execute: changeUpdate)
            }
        }
    }

    /**
     * Atomically checks dirty state and, if dirty, stashes a pending save target. Returns true if pending was set.
     */
    private func setPendingIfModified(url: URL,
                                      typeName: String,
                                      saveOperation: NSDocument.SaveOperationType,
                                      completionHandler: @escaping ((any Error)?) -> Void) -> Bool {
        modifiedLock.lock()
        defer { modifiedLock.unlock() }
        if _isModified {
            let t = self.changeCountToken(for: saveOperation)
            pendingSave = PendingSave(url: url, typeName: typeName, operation: saveOperation, completion: completionHandler, token: t)
            return true
        }

        return false
    }

    /**
     * Called by the COOL once it flushed edits to `tempFileURL` (based on .uno:Save command result); completes any pending Save / Save As…
     */
    func triggerSave() {
        // Grab and clear the pending request atomically.
        var ps: PendingSave?

        modifiedLock.lock()
        ps = pendingSave
        pendingSave = nil
        modifiedLock.unlock()

        if let ps {
            // Finish the original AppKit request: write the bytes and call its original completion
            DispatchQueue.main.async {
                self.performPendingSave(ps)
            }
        }
        else {
            // No pending AppKit request → internal/webview save: ask AppKit to Save (or show Save As… if untitled)
            DispatchQueue.main.async {
                self.performImplicitSave()
            }
        }
    }

    // MARK: - deinit

    deinit {
        fileURLObservation?.invalidate()
    }


    // MARK: - NSDocument Overrides

    /**
     * Enables autosaving.
     */
    override class var autosavesInPlace: Bool {
        return true
    }

    /**
     * Creates the window controllers for the document.
     *
     * Default to a nicer size when opening the document the 1st time, or update
     * the window size according to what we remember (per document).
     */
    override func makeWindowControllers() {
        // Load the storyboard and get the window controller.
        let storyboard = NSStoryboard(name: "Main", bundle: nil)
        let identifier = NSStoryboard.SceneIdentifier("DocumentWindowController")
        guard let windowController = storyboard.instantiateController(withIdentifier: identifier) as? WindowController else {
            fatalError("Unable to find DocumentWindowController in storyboard.")
        }

        // Assign a per-document autosave name early
        let initialName = FrameAutosaveHelper.keyName(for: self)
        windowController.windowFrameAutosaveName = initialName
        windowController.shouldCascadeWindows = false

        addWindowController(windowController)

        if let viewController = windowController.contentViewController as? ViewController {
            viewController.loadDocument(self)
        }

        // Ensure the window exists so we can apply a default if no saved frame yet
        windowController.loadWindow()
        if let win = windowController.window {
            if isWelcome {
                // Border-less Welcome screen; we can't do it truly border-less (using .borderless),
                // because then it doesn't auto-close when the slideshow finishes.
                win.styleMask = [.titled, .resizable, .closable, .miniaturizable, .fullSizeContentView]
                win.titleVisibility = .hidden
                win.titlebarAppearsTransparent = true
                win.isMovableByWindowBackground = true

                // Hide traffic lights
                win.standardWindowButton(.closeButton)?.isHidden = true
                win.standardWindowButton(.miniaturizeButton)?.isHidden = true
                win.standardWindowButton(.zoomButton)?.isHidden = true
            }
            else {
                // Prefer tabbed approach for documents
                win.tabbingMode = .preferred
                win.tabbingIdentifier = "CollaboraDocumentTab"
            }

            // Set minimum window size
            win.minSize = NSSize(width: 800, height: 480)

            if isWelcome {
                win.maxSize = NSSize(width: 1280, height: 720)
                FrameAutosaveHelper.applyDefaultFrame(win, widthFraction: 0.4, heightFraction: 0.4, ratio: 16.0/9.0)
            }
            else if !win.setFrameUsingName(initialName) {
                FrameAutosaveHelper.applyDefaultFrame(win, widthFraction: 0.95, heightFraction: 0.95)
            }

            // Set window icon based on document type
            let type = self.documentType ?? self.fileType ?? "org.oasis-open.opendocument.text"

            // For new/untitled documents, we need to set a represented URL to make the icon button appear
            if self.fileURL == nil && win.representedURL == nil {
                // Create a temporary represented URL so the icon button appears
                // Use the temp file URL if available, otherwise create a dummy one
                if let tempURL = self.tempFileURL {
                    win.representedURL = tempURL
                } else {
                    // Create a dummy URL with the appropriate extension
                    let ext: String
                    switch type {
                    case let t where t.contains("spreadsheet"):
                        ext = "ods"
                    case let t where t.contains("presentation"):
                        ext = "odp"
                    case let t where t.contains("text"):
                        ext = "odt"
                    default:
                        ext = "odt"
                    }
                    let dummyURL = URL(fileURLWithPath: "/tmp/Untitled.\(ext)")
                    win.representedURL = dummyURL
                }
            }

            // Set branded icon based on document type
            if let icon = Document.iconForType(typeName: type) {
                // General icon for the entire window
                win.standardWindowButton(.documentIconButton)?.image = icon

                // Show the icon at the right side of the tab in the tabbed view too
                let view = NSImageView(image: icon)
                NSLayoutConstraint.activate([
                    view.widthAnchor.constraint(equalToConstant: 24), // add some breathing space to the left and right, it's centered inside that
                    view.heightAnchor.constraint(equalToConstant: 16),
                ])
                win.tab.accessoryView = view
            }
        }

        // Observe fileURL so we can switch/migrate the autosave key after first save/rename.
        fileURLObservation = observe(\.fileURL, options: [.new]) { [weak windowController] doc, _ in
            guard let wc = windowController else { return }
            let oldName = wc.windowFrameAutosaveName
            let newName = FrameAutosaveHelper.keyName(for: doc)
            if newName == oldName { return }

            FrameAutosaveHelper.migrateSavedFrame(from: oldName, to: newName)
            wc.windowFrameAutosaveName = newName

            // If there’s already a window, try to apply the stored frame for the new key.
            if let win = wc.window { _ = win.setFrameUsingName(newName) }
        }
    }

    /**
     * Called by the system when it wants to save or autosave the document.
     */
    override func data(ofType typeName: String) throws -> Data {
        guard let tempFileURL = self.tempFileURL else {
            // FIXME: handle error?
            return Data()
        }

        // Read the latest data from the temp file
        let data = try Data(contentsOf: tempFileURL)
        return data
    }

    /**
     * We save asynchronously, so that COOL can first write the file, and we can then copy it to the right location.
     */
    override func canAsynchronouslyWrite(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) -> Bool {
        return true
    }

    /**
     * Make sure that we first save by COOL when the user chooses to Save, and only then copy the content to the resulting place.
     */
    override func save(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType, completionHandler: @escaping ((any Error)?) -> Void) {

        if setPendingIfModified(url: url, typeName: typeName, saveOperation: saveOperation, completionHandler: completionHandler) {
            // we have to wait for COOL to save first
            DispatchQueue.main.async {
                COWrapper.handleMessage(with: self, message: "save dontTerminateEdit=1 dontSaveIfUnmodified=1")
            }
        }
        else {
            // all is good, we can proceed with copying the data from COOL
            super.save(to: url, ofType: typeName, for: saveOperation, completionHandler: completionHandler)
        }
    }

    /**
     * Calls super.save(...) to complete a pending Save / Save As…
     */
    private func performPendingSave(_ ps: PendingSave) {
        do {
            // Perform the actual write using NSDocument’s writing pipeline (uses data(ofType:)).
            try self.write(to: ps.url, ofType: ps.typeName, for: ps.operation, originalContentsURL: self.fileURL)

            // For Save As / first Save, adopt the new URL/type. Save To must NOT change fileURL.
            if ps.operation == .saveAsOperation || (ps.operation == .saveOperation && self.fileURL == nil) {
                self.fileURL = ps.url
                self.fileType = ps.typeName
            }

            // Clear edited state only if nothing new happened since this save began.
            self.updateChangeCount(withToken: ps.token, for: ps.operation)

            // Tell AppKit the *original* save request completed (this unblocks the close button flow).
            DispatchQueue.main.async { ps.completion(nil) }
        }
        catch {
            DispatchQueue.main.async { ps.completion(error) }
        }
    }

    /**
     * Performs an AppKit-driven save (or Save As… if the document is untitled) after a COOL-initiated flush.
     */
    private func performImplicitSave() {
        // This will show the Save panel if fileURL == nil, otherwise it saves in place.
        self.save(self)
    }

    /**
     * Restrict Save/Save As… formats to the document's current type, we currently can't change the type (yet).
     */
    override func writableTypes(for saveOperation: NSDocument.SaveOperationType) -> [String] {
        // Ask super for the default list (from Info.plist)
        let all = super.writableTypes(for: saveOperation)

        // Determine our current type name (best effort)
        guard let current = currentTypeNameForSavePanel() else {
            return all  // fallback: keep default list if we can't determine
        }

        // If super’s list includes it, return only that; otherwise still force just current.
        return all.contains(current) ? [current] : [current]
    }

    /**
     * Best-effort detection of this document’s current type name.
     */
    private func currentTypeNameForSavePanel() -> String? {
        // If the doc already knows its type, use it.
        if let t = self.fileType { return t }

        // If we were opened from a URL, ask the controller to infer it from contents.
        if let url = self.fileURL {
            return try? NSDocumentController.shared.typeForContents(of: url)
        }

        // Or try the tempFile
        if let url = self.tempFileURL {
            return try? NSDocumentController.shared.typeForContents(of: url)
        }

        // Fall back to the controller’s defaultType (may be nil).
        return NSDocumentController.shared.defaultType
    }

    /**
     * Called by the system when the document is opened. The system provides the file contents as `Data`.
     * We create a non-predictable temporary directory using a UUID, and store the `data` there.
     */
    override func read(from data: Data, ofType typeName: String) throws {
        // Create a unique temp directory
        let tempDirBase = FileManager.default.temporaryDirectory
        let uniqueDirName = UUID().uuidString
        let tempDir = tempDirBase.appendingPathComponent(uniqueDirName, isDirectory: true)

        try FileManager.default.createDirectory(at: tempDir, withIntermediateDirectories: true, attributes: nil)

        self.tempDirectoryURL = tempDir

        // If fileURL is available (document opened from a file), preserve the original filename.
        // If not available, use a generic name.
        let fileName: String
        if let fileURL = self.fileURL {
            fileName = fileURL.lastPathComponent
        }
        else {
            fileName = "Document-\(UUID().uuidString)"
        }

        let tempFile = tempDir.appendingPathComponent(fileName)
        try data.write(to: tempFile, options: .atomic)

        self.tempFileURL = tempFile
    }

    /**
     * Implement printing.
     */
    override func printOperation(withSettings printSettings: [NSPrintInfo.AttributeKey : Any]) throws -> NSPrintOperation {
        // export to a temporary PDF file
        let tmpURL = FileManager.default
            .temporaryDirectory
            .appendingPathComponent(UUID().uuidString)
            .appendingPathExtension("pdf")

        COWrapper.saveAs(with: self, url: tmpURL.absoluteString, format: "pdf", filterOptions: nil)

        // load the PDF into a PDFView
        guard let pdfDocument = PDFDocument(url: tmpURL) else {
            throw CocoaError(.fileReadCorruptFile, userInfo: [NSURLErrorKey: tmpURL])
        }

        // we no longer need the file
        try? FileManager.default.removeItem(at: tmpURL)

        guard let op = pdfDocument.printOperation(for: self.printInfo, scalingMode: .pageScaleNone, autoRotate: true) else {
            throw CocoaError(.fileReadCorruptFile, userInfo: [NSURLErrorKey: tmpURL])
        }

        return op
    }

    /*
     * Guard against double call of close().
     */
    private var isClosing = false

    /**
     * Clean up the temporary directory when the document closes.
     */
    override func close() {
        // there is no guarrantee that close() is called just once, see
        // https://stackoverflow.com/questions/5627267/nsdocument-subclass-close-method-called-twice
        if isClosing { return }
        isClosing = true

        NSLog("CollaboraOffice: Closing document")
        COWrapper.bye(self)
        super.close()
        if let tempDir = self.tempDirectoryURL {
            try? FileManager.default.removeItem(at: tempDir)
        }
    }

    /**
     * Common items like "lang" or "darkTheme" and alike, that are useful for the Backstage invocation too.
     */
    static func addCommonCOOLQueryItems(to components: inout URLComponents) {
        let lang = Locale.preferredLanguages.first ?? "en-US"
        components.queryItems?.append(URLQueryItem(name: "lang", value: lang))
        components.queryItems?.append(URLQueryItem(name: "dir", value: COWrapper.isRtlLanguage(lang) ? "rtl" : ""))

        // Add darkTheme parameter if user has dark mode enabled
        if NSApp.effectiveAppearance.bestMatch(from: [.darkAqua, .aqua]) == .darkAqua {
            components.queryItems?.append(URLQueryItem(name: "darkTheme", value: "true"))
        }
    }

    /**
     * Initiate loading of cool.html, which also triggers loading of the document via lokit.
     */
    func loadDocumentInWebView(webView: WKWebView, permission: String, isWelcome: Bool) {
        self.webView = webView

        self.appDocId = COWrapper.generateNewAppDocId()
        self.fakeClientFd = COWrapper.fakeSocketSocket()

        guard let url = Bundle.main.url(forResource: "cool", withExtension: "html") else {
            fatalError("Resource 'cool.html' not found in the main bundle.")
        }

        var components = URLComponents(url: url, resolvingAgainstBaseURL: false)!
        let lang = Locale.preferredLanguages.first ?? "en-US"

        components.queryItems = [
            URLQueryItem(name: "file_path", value: tempFileURL!.absoluteString),
            URLQueryItem(name: "permission", value: permission),
            URLQueryItem(name: "appdocid", value: "\(self.appDocId)"),
            URLQueryItem(name: "userinterfacemode", value: "notebookbar"),
        ]

        // Add lang and (potentially) darkTheme
        Document.addCommonCOOLQueryItems(to: &components)

        if !isNewDocument {
            components.queryItems?.append(URLQueryItem(name: "startreadonly", value: "true"))
        }

        if isWelcome {
            components.queryItems?.append(URLQueryItem(name: "welcome", value: "true"))
        }

        let finalURL = components.url!
        let urlDir = url.deletingLastPathComponent()

        guard finalURL.isFileURL else {
            NSLog("cool.html not found in bundle")
            webView.loadHTMLString("<p>Couldn't access <code>cool.html</code>, this build is seriously broken.</p>", baseURL: nil)
            return
        }

        webView.loadFileURL(finalURL, allowingReadAccessTo: urlDir)
    }

    /**
     * Abbreviated message for debugging.
     */
    private func abbreviatedMessage(buffer: UnsafePointer<CChar>, length: Int) -> String {
        // Implement your logic or return a placeholder:
        let msgData = Data(bytes: buffer, count: length)
        let msgStr = String(data: msgData, encoding: .utf8) ?? "<non-UTF8 message>"
        return msgStr.prefix(100) + (msgStr.count > 100 ? "..." : "")
    }

    @objc
    func send2JS(_ buffer: UnsafePointer<CChar>, length: Int) {
        let abbrMsg = abbreviatedMessage(buffer: buffer, length: length)
        COWrapper.LOG_TRC("To JS: \(abbrMsg)")

        let binaryMessage = COWrapper.isBinaryMessage(buffer, length: length)

        let pretext = binaryMessage
            ? "window.TheFakeWebSocket.onmessage({'data': window.atob('"
            : "window.TheFakeWebSocket.onmessage({'data': window.b64d('"
        let posttext = "')});"

        // Convert the buffer to Data
        let payloadData = Data(bytes: buffer, count: length)
        let encodedPayload = payloadData.base64EncodedString(options: [])

        // Construct the full JavaScript string
        let js = pretext + encodedPayload + posttext

        // Truncate for logging
        let truncatedJS = js.count > 100 ? (js.prefix(100) + "...") : js[...]
        COWrapper.LOG_TRC("Evaluating JavaScript: \(truncatedJS)")

        // Evaluate on main queue
        DispatchQueue.main.async {
            self.webView.evaluateJavaScript(js) { (obj, error) in
                if let error = error as NSError? {
                    COWrapper.LOG_ERR("Error after \(truncatedJS): \(error.localizedDescription)")
                    if let jsException = error.userInfo["WKJavaScriptExceptionMessage"] as? String {
                        COWrapper.LOG_ERR("JavaScript exception: \(jsException)")
                    }
                }
            }
        }
    }

    /// Returns the correct icon for a document type
    static func iconForType(typeName: String) -> NSImage? {
        // Use branded icons from Assets catalog
        let iconName: String?
        switch typeName {
        case "org.oasis-open.opendocument.text",
             "org.openoffice.text",
             "com.microsoft.word.doc",
             "org.openxmlformats.wordprocessingml.document",
             "org.openxmlformats.wordprocessingml.document.macroEnabled",
             "org.openxmlformats.wordprocessingml.template",
             "org.openxmlformats.wordprocessingml.template.macroEnabled",
             "public.rtf":
            iconName = "DocumentIcon"
        case "org.oasis-open.opendocument.spreadsheet",
             "org.openoffice.spreadsheet",
             "com.microsoft.excel.xls",
             "org.openxmlformats.spreadsheetml.sheet",
             "org.openxmlformats.spreadsheetml.sheet.macroEnabled",
             "com.microsoft.excel.sheet.binary.macroEnabled",
             "org.openxmlformats.spreadsheetml.template",
             "org.openxmlformats.spreadsheetml.template.macroEnabled":
            iconName = "SpreadsheetIcon"
        case "org.oasis-open.opendocument.presentation",
             "org.openoffice.presentation",
             "com.microsoft.powerpoint.ppt",
             "org.openxmlformats.presentationml.presentation",
             "org.openxmlformats.presentationml.presentation.macroEnabled",
             "org.openxmlformats.presentationml.template",
             "org.openxmlformats.presentationml.template.macroEnabled":
            iconName = "PresentationIcon"
        case "org.oasis-open.opendocument.graphics",
             "org.openoffice.graphics",
             "org.libreoffice.visio-document":
            iconName = "DrawingIcon"
        default:
            iconName = nil
        }

        // Load from Assets catalog
        if let name = iconName, let image = NSImage(named: name) {
            return image
        }

        // Fallback to app icon
        return NSApp.applicationIconImage
    }
}

/**
 * Helper class containing functions for handling autosaving the size of the window.
 */
private class FrameAutosaveHelper {


    /**
     * Helper function for handling Window's maxSize.
     */
    @inline(__always) private static func effMax(_ v: CGFloat) -> CGFloat { v > 0 ? v : .greatestFiniteMagnitude }

    /**
     * Resize window to fractions of visible width & height, and center it on the target screen.
     */
    static func applyDefaultFrame(_ window: NSWindow, widthFraction: CGFloat, heightFraction: CGFloat, ratio: CGFloat? = nil) {
        // Choose the screen the window will appear on (fallback to main/first if unknown).
        let screen = window.screen ?? NSScreen.main ?? NSScreen.screens.first!
        let vf = screen.visibleFrame

        // Fraction for width and height (ideal target before constraints)
        let targetW = floor(vf.width * widthFraction)
        let targetH = floor(vf.height * heightFraction)

        let minSize = window.minSize
        let maxSize = window.maxSize

        let clampedW: CGFloat
        let clampedH: CGFloat
        if let r = ratio, r > 0 {
            // Maintain aspect ratio r = width/height, drive by width primarily.
            // Derive allowed width interval from BOTH width and height constraints.
            let minAllowedW = max(minSize.width, r * minSize.height)
            let maxAllowedW = min(effMax(maxSize.width), effMax(maxSize.height) * r)

            // Start from target width; clamp into feasible interval.
            var w = max(minAllowedW, min(maxAllowedW, targetW))
            var h = w / r

            // Defensive: after rounding, enforce height limits by nudging width accordingly (keep ratio).
            if h < minSize.height {
                h = minSize.height
                w = r * h
            }
            else if maxSize.height > 0 && h > maxSize.height {
                h = maxSize.height
                w = r * h
            }

            clampedW = w
            clampedH = h
        }
        else {
            // No ratio requested -> independent clamping
            clampedW = max(minSize.width, min(effMax(maxSize.width), targetW))
            clampedH = max(minSize.height, min(effMax(maxSize.height), targetH))
        }

        // Position so that the window is centered
        let x = vf.origin.x + (vf.width - clampedW) / 2.0
        let y = vf.origin.y + (vf.height - clampedH) / 2.0

        let clampedFrame = NSRect(x: x, y: y, width: clampedW, height: clampedH)

        window.setFrame(clampedFrame, display: false, animate: false)
    }

    /// Stable per-doc name. Untitled docs share one bucket so they all use the default layout.
    static func keyName(for doc: NSDocument) -> String {
        guard let url = doc.fileURL else { return "DocWindow-untitled" }

        if let stable = stableIdentityKey(for: url) {
            return "DocWindow-\(stable)"
        } else {
            // Fallback: based on standardized path (won’t survive moves/renames, but deterministic)
            return "DocWindow-path-\(hashHex(url.standardizedFileURL.path))"
        }
    }

    /// Try to build a move/rename-resistant identity from the file + volume IDs.
    private static func stableIdentityKey(for url: URL) -> String? {
        do {
            let vals = try url.resourceValues(forKeys: [.fileResourceIdentifierKey, .volumeIdentifierKey])
            guard let fid = vals.fileResourceIdentifier, let vid = vals.volumeIdentifier else { return nil }
            return "id-\(hashHex("\(vid)|\(fid)"))"
        } catch { return nil }
    }

    static func migrateSavedFrame(from old: String, to new: String) {
        let defaults = UserDefaults.standard
        let oldKey = "NSWindow Frame \(old)"
        let newKey = "NSWindow Frame \(new)"
        if let v = defaults.string(forKey: oldKey) {
            defaults.set(v, forKey: newKey)
        }
    }

    private static func hashHex(_ s: String) -> String {
        let data = Data(s.utf8)
        #if canImport(CryptoKit)
        let digest = SHA256.hash(data: data)
        return digest.map { String(format: "%02x", $0) }.joined()
        #else
        // Deterministic FNV-1a 64-bit fallback
        var h: UInt64 = 0xcbf29ce484222325
        for b in data { h ^= UInt64(b); h &*= 0x100000001b3 }
        return String(format: "%016llx", h)
        #endif
    }
}
