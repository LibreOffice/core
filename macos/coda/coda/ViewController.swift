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
import UniformTypeIdentifiers
import WebKit

final class ConsoleController: NSWindowController {
    let webView: WKWebView

    init(webView: WKWebView) {
        self.webView = webView
        let style: NSWindow.StyleMask = [.titled, .closable, .resizable, .miniaturizable]
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 1000, height: 640),
                              styleMask: style, backing: .buffered, defer: false)
        super.init(window: window)

        // put the webView into the window
        webView.translatesAutoresizingMaskIntoConstraints = false
        window.contentView?.addSubview(webView)

        if let content = window.contentView {
            NSLayoutConstraint.activate([
                webView.leadingAnchor.constraint(equalTo: content.leadingAnchor),
                webView.trailingAnchor.constraint(equalTo: content.trailingAnchor),
                webView.topAnchor.constraint(equalTo: content.topAnchor),
                webView.bottomAnchor.constraint(equalTo: content.bottomAnchor),
            ])
        }
    }

    required init?(coder: NSCoder) { fatalError("doesn't seem to matter") }
}

class ViewController: NSViewController, WKScriptMessageHandlerWithReply, WKNavigationDelegate, WKUIDelegate {

    /// Access to the NSDocument (document loading & saving infrastructure).
    var document: Document!

    /// The actual webview holding the document.
    var webView: WKWebView!

    var consoleController: ConsoleController!

    /// Whether UI testing mode is active.
    private lazy var isUITesting: Bool = {
        ProcessInfo.processInfo.arguments.contains("--uitesting")
    }()

    /// Offscreen text view that accumulates all "lok" messages for XCUITest assertions.
    private var testMessageLog: NSTextView?

    /// HTTP server for executing JS commands from XCUITest.
    private var testHTTPServer: TestHTTPServer?

    var savedViewFrame: NSRect!
    var savedConsoleViewFrame: NSRect!

    var displayConnectionObserver: AnyObject!
    var screenCount: Int = 0
    var needRearrange: Bool = false
    var mainFullScreenActive: Bool = false
    var consoleFullScreenActive: Bool = false
    var mainWindowExitFSObserver: AnyObject!
    var mainMonitorExchangeObserver: AnyObject!
    var consoleWindowExitFSObserver: AnyObject!
    var consoleMonitorExchangeObserver: AnyObject!

    override func viewDidLoad() {
        super.viewDidLoad()

        // Setup jsHandler as the entry point to call back from JavaScript
        let contentController = WKUserContentController()
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "debug")
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "lok")
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "error")
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "clipboard")

        let config = WKWebViewConfiguration()
        config.preferences.isElementFullscreenEnabled = true
        config.userContentController = contentController

        // Create the web view
        webView = WKWebView(frame: .zero, configuration: config)
        webView.navigationDelegate = self
        webView.uiDelegate = self

#if DEBUG
        // Enable possibility to debug the webview from Safari
        webView.isInspectable = true
#endif

        // Add it to the view controller's view
        self.view.addSubview(webView)

        webView.setAccessibilityIdentifier("CODA.DocumentWebView")

        webView.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            webView.leadingAnchor.constraint(equalTo: self.view.leadingAnchor),
            webView.trailingAnchor.constraint(equalTo: self.view.trailingAnchor),
            webView.topAnchor.constraint(equalTo: self.view.topAnchor),
            webView.bottomAnchor.constraint(equalTo: self.view.bottomAnchor)
        ])

        if isUITesting {
            let scrollView = NSScrollView(frame: NSRect(x: -10000, y: 0, width: 100, height: 100))
            let textView = NSTextView(frame: NSRect(x: 0, y: 0, width: 100, height: 100))
            textView.isEditable = false
            textView.setAccessibilityIdentifier("CODA.TestMessageLog")
            scrollView.documentView = textView
            self.view.addSubview(scrollView)
            testMessageLog = textView

            // Start the test HTTP server for JS execution
            let args = ProcessInfo.processInfo.arguments
            let portString = args.lazy
                .compactMap { $0.hasPrefix("--testDriverPort=") ? String($0.dropFirst("--testDriverPort=".count)) : nil }
                .first
            if let portString, let port = UInt16(portString) {
                do {
                    let server = try TestHTTPServer(port: port,
                        jsExecutor: { [weak self] js, completion in
                            DispatchQueue.main.async {
                                guard let webView = self?.webView else {
                                    completion(nil, NSError(domain: "TestHTTPServer", code: 1,
                                                            userInfo: [NSLocalizedDescriptionKey: "webView not available"]))
                                    return
                                }
                                webView.evaluateJavaScript(js, completionHandler: completion)
                            }
                        },
                        focusHandler: { [weak self] done in
                            DispatchQueue.main.async {
                                if let webView = self?.webView {
                                    webView.window?.makeKeyAndOrderFront(nil)
                                    NSApp.activate(ignoringOtherApps: true)
                                    webView.window?.makeFirstResponder(webView)
                                }
                                done()
                            }
                        }
                    )
                    server.start()
                    testHTTPServer = server
                } catch {
                    NSLog("TestHTTPServer: failed to start: %@", error.localizedDescription)
                }
            }
        }
    }

    /**
     * Load the the document; to be called from the Document (NSDocument) instance.
     */
    func loadDocument(_ document: Document) {
        self.document = document
        let permission = document.isWelcome ? "view" : "edit"
        self.document.loadDocumentInWebView(webView: webView, permission: permission, isWelcome: document.isWelcome)
    }

    /**
     * Make the picking of images work in our webView (via the WKUIDelegate).
     */
    func webView(_ webView: WKWebView,
                 runOpenPanelWith parameters: WKOpenPanelParameters,
                 initiatedByFrame frame: WKFrameInfo,
                 completionHandler: @escaping ([URL]?) -> Void) {

        let panel = NSOpenPanel()
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = parameters.allowsMultipleSelection

        // Let the use pick just images
        panel.allowedContentTypes = [UTType.image]

        panel.beginSheetModal(for: view.window!) { result in
            if result == .OK {
                completionHandler(panel.urls)
            }
            else {
                completionHandler(nil)
            }
        }
    }

    /**
     * Receive message from JavaScript, with the possibility to reply
     */
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) async -> (Any?, String?) {
        switch message.name {

        case "error":
            if let body = message.body as? String {
                COWrapper.LOG_ERR("Error from WebView: \(body)")
            }

        case "debug":
            if let body = message.body as? String {
                print("==> \(body)")
            }

        case "clipboard":
            if let body = message.body as? String {
                switch body {

                case "read":
                    COWrapper.setClipboard(document, from: .general)
                    return ("(internal)", nil);

                case "write":
                    guard let content = COWrapper.getClipboard(document) else {
                        COWrapper.LOG_ERR("Failed to get clipboard contents")
                        return (nil, nil)
                    }
                    NSPasteboard.general.clearContents()
                    NSPasteboard.general.writeObjects(content)

                case let s where s.hasPrefix("sendToInternal "):
                    if !COWrapper.sendToInternalClipboard(document, content: String(s.dropFirst("sendToInternal ".count))) {
                        COWrapper.LOG_ERR("set clipboard returned failure");
                        return (nil, "set clipboard returned failure");
                    }

                default:
                    COWrapper.LOG_ERR("Invalid clipboard action \(body)")
                }
            }

        case "lok":
            if let body = message.body as? String {
                COWrapper.LOG_DBG("To Online: '\(message.body)'")

                if isUITesting, let log = testMessageLog {
                    log.string += body + "\n"
                }

                if body == "HULLO" {
                    // Now we know that the JS has started completely
                    COWrapper.handleHULLO(with: document)
                    return (nil, nil)
                }
                else if body == "BYE" {
                    COWrapper.LOG_TRC("Document window terminating on JavaScript side. Closing our end of the socket.")
                    view.window?.performClose(nil)
                    return (nil, nil)
                }
                else if body.hasPrefix("COMMANDSTATECHANGED ") {
                    if let brace = body.firstIndex(of: "{") {
                        // substring that shares storage with the original string
                        let jsonSlice = body[brace...]

                        // convert directly to Data and decode.
                        let data = Data(jsonSlice.utf8)
                        do {
                            let state = try JSONDecoder().decode(CommandStateChange.self, from: data)

                            // Has the modification state of the document changed?
                            // This is smportant for saving which has to copy the document from the temporary location.
                            if state.commandName == ".uno:ModifiedStatus" {
                                document?.isModified = (state.state == "true")
                            }

                            // remember states of the commands for app menu handling
                            if let windowController = view.window?.windowController as? WindowController {
                                windowController.handleCommandStateChange(state)
                            }
                        } catch {}
                    }
                }
                else if body.hasPrefix("COMMANDRESULT ") {
                    if let brace = body.firstIndex(of: "{") {
                        // substring that shares storage with the original string
                        let jsonSlice = body[brace...]

                        // convert directly to Data and decode.
                        let data = Data(jsonSlice.utf8)
                        do {
                            let result = try JSONDecoder().decode(CommandResult.self, from: data)

                            // Was it a successful save?
                            if result.commandName == ".uno:Save" && result.success == true && result.wasModified == true {
                                document.triggerSave()
                            }
                        } catch {}
                    }
                }
                else if body == "PRINT" {
                    document.printDocument(self)
                    return (nil, nil)
                }
                else if body == "EXCHANGEMONITORS" {
                    exchangeMonitors()
                }
                else if body.hasPrefix("FULLSCREENPRESENTATION ") {
                    let fullScreen = body.dropFirst("FULLSCREENPRESENTATION ".count) == "true"
                    // this type of full screen window is under our control
                    // so it can be moved to another monitor
                    if (fullScreen) {

                        installDisplayConnectionMonitor()
                        let mainWindow = view.window!
                        self.savedViewFrame = mainWindow.frame
                        arrangePresentationWindows()

                        arrangePresentationWindows()
                    } else {
                        endPresentation();
                    }
                }
                else if body == "FOCUSIFHWKBD" {
                    COWrapper.LOG_ERR("TODO: Implement FOCUSIFHWKBD")
                    /*
                    if isExternalKeyboardAttached() {
                        let hwKeyboardMagic = """
                                {
                                    if (window.MagicToGetHWKeyboardWorking) {
                                        window.MagicToGetHWKeyboardWorking();
                                    }
                                }
                                """
                        self.webView.evaluateJavaScript(hwKeyboardMagic) { (result, error) in
                            if let error = error {
                                COOLWrapper.LOG_ERR("Error after \(hwKeyboardMagic): \(error.localizedDescription)")
                                if let jsException = (error as NSError).userInfo["WKJavaScriptExceptionMessage"] as? String {
                                    COOLWrapper.LOG_ERR("JavaScript exception: \(jsException)")
                                }
                            }
                        }
                    }
                    */
                    return (nil, nil)
                }
                else if body.hasPrefix("HYPERLINK") {
                    let messageBodyItems = body.components(separatedBy: " ")
                    if messageBodyItems.count >= 2 {
                        if let url = URL(string: messageBodyItems[1]) {
                            NSWorkspace.shared.open(url)
                            return (nil, nil)
                        }
                    }
                    return (nil, nil)
                }
                else if body == "FONTPICKER" {
                    COWrapper.LOG_ERR("TODO: Implement FONTPICKER")
                    /*
                    // Font picker is not available on macOS like on iOS, but you can use NSFontPanel
                    let fontManager = NSFontManager.shared
                    fontManager.target = self
                    fontManager.action = #selector(changeFont(_:))
                    fontManager.orderFrontFontPanel(self)
                    */
                    return (nil, nil)
                }
                else if body == "WELCOME" {
                    (NSDocumentController.shared as? DocumentController)?.openWelcome()
                    return (nil, nil)
                }
                else if body == "LICENSE" {
                    guard let url = Bundle.main.url(forResource: "LICENSE", withExtension: "html") else {
                        COWrapper.LOG_ERR("LICENSE.html not found in bundle")
                        return (nil, nil)
                     }

                     NSWorkspace.shared.open(url)
                     return (nil, nil)
                }
                else if body == "SYNCSETTINGS" {
                    // TODO implement
                    return (nil, nil)
                }
                else if body.hasPrefix("downloadas ") {
                    let messageBodyItems = body.components(separatedBy: " ")
                    var format: String?
                    if messageBodyItems.count >= 2 {
                        for item in messageBodyItems[1...] {
                            if item.hasPrefix("format=") {
                                format = String(item.dropFirst("format=".count))
                            }
                        }
                        guard let format = format else { return (nil, nil) }

                        // Handle special "direct-" formats
                        var adjustedFormat = format
                        if adjustedFormat.hasPrefix("direct-") {
                            adjustedFormat = String(adjustedFormat.dropFirst("direct-".count))
                        }

                        // Save the document in the requested format
                        let tmpFileDirectory = FileManager.default.temporaryDirectory.appendingPathComponent("export")
                        do {
                            try FileManager.default.createDirectory(at: tmpFileDirectory, withIntermediateDirectories: true, attributes: nil)
                        } catch {
                            COWrapper.LOG_ERR("Could not create directory \(tmpFileDirectory.path)")
                            return (nil, nil)
                        }

                        // Remove the original extension from the file name and add the adjustedFormat
                        guard let tmpFileName = self.document?.tempFileURL?.deletingPathExtension().appendingPathExtension(adjustedFormat).lastPathComponent else { return (nil, nil) }
                        let downloadAsTmpURL = tmpFileDirectory.appendingPathComponent(tmpFileName)

                        // Remove any existing file
                        do {
                            try FileManager.default.removeItem(at: downloadAsTmpURL)
                        } catch {
                            // File may not exist, ignore error
                        }

                        // Perform the actual Save As
                        COWrapper.saveAs(with: document, url: downloadAsTmpURL.absoluteString, format: adjustedFormat, filterOptions: nil)

                        // Verify the file was saved
                        let fileExists = FileManager.default.fileExists(atPath: downloadAsTmpURL.path)
                        if !fileExists {
                            COWrapper.LOG_ERR("Could not save to '\(downloadAsTmpURL.path)'")
                            return (nil, nil)
                        }

                        // Present a save panel to let the user choose where to save the file
                        let savePanel = NSSavePanel()
                        savePanel.directoryURL = FileManager.default.homeDirectoryForCurrentUser
                        savePanel.nameFieldStringValue = tmpFileName
                        savePanel.begin { (result) in
                            if result == .OK, let url = savePanel.url {
                                do {
                                    try FileManager.default.copyItem(at: downloadAsTmpURL, to: url)
                                    // Remove the temporary file
                                    try FileManager.default.removeItem(at: downloadAsTmpURL)
                                } catch {
                                    COWrapper.LOG_ERR("Error during file save: \(error)")
                                }
                            }
                        }
                    }
                    return (nil, nil)
                }
                else if body == "uno .uno:SaveAs" {
                    // Trigger the NSDocument Save As… panel
                    document.saveAs(nil)
                    return (nil, nil)
                }
                else if body.hasPrefix("TEXTCLIPBOARD ") {
                    let text = String(body.dropFirst("TEXTCLIPBOARD ".count))
                    NSPasteboard.general.clearContents()
                    NSPasteboard.general.setString(text, forType: .string)
                    return (nil, nil)
                }
                else if let result = ViewController.handleBackstageMessage(body) {
                    return result
                }
                else {
                    // Just send the message
                    COWrapper.handleMessage(with: document, message: body)
                }
            }

        default:
            if let body = message.body as? String {
                COWrapper.LOG_ERR("Unrecognized kind of message received from WebView: \(message.name):\(body)")
            }
            else {
                COWrapper.LOG_ERR("Unrecognized kind of message received from WebView: \(message.name)")
            }
        }

        return (nil, nil)
    }

    /**
     * Functionality shared by the "normal" backstage (that opens when the user chooses "File" in the notebookbar) and the "startup" backstage (that has the templates etc.).
     *
     * @param body: The message (command) that we want to handle
     * @param onClose: In case this is called from backstage that is a separate window & should close the backstage itself, this is what will be called to do that.
     *
     * The return value is either nil (means the message was not handled), or pair that would be returned from the handler.
     */
    static func handleBackstageMessage(_ body: String, onClose: (() -> Void)? = nil) -> (Any?, String?)? {
        if body == "GETRECENTDOCS" {
            return (RecentFiles.serialize(), nil)
        }
        else if body.hasPrefix("newdoc ") {
            let messageBodyItems = body.components(separatedBy: " ")
            var type: String?
            var templatePath: String?
            if messageBodyItems.count >= 2 {
                for item in messageBodyItems[1...] {
                    if item.hasPrefix("type=") {
                        type = String(item.dropFirst("type=".count))
                    } else if item.hasPrefix("template=") {
                        templatePath = String(item.dropFirst("template=".count)).removingPercentEncoding
                    }
                }

                let kind: DocumentController.NewKind
                switch type {
                    case "calc": kind = .spreadsheet
                    case "impress": kind = .presentation
                    default : kind = .text
                }

                (NSDocumentController.shared as? DocumentController)?.createDocument(fromTemplateFor: kind, templatePath: templatePath)
            }

            onClose?()
            return (nil, nil)
        }
        else if body.hasPrefix("opendoc ") {
            let messageBodyItems = body.components(separatedBy: " ")
            var fileURLString: String?
            if messageBodyItems.count >= 2 {
                for item in messageBodyItems[1...] {
                    if item.hasPrefix("file=") {
                        fileURLString = String(item.dropFirst("file=".count)).removingPercentEncoding
                    }
                }
            }

            guard let fileURLString, let url = URL(string: fileURLString) else {
                return (nil, "Invalid opendoc message (missing/invalid file=...)")
            }

            // Open via NSDocument / NSDocumentController
            NSDocumentController.shared.openDocument(withContentsOf: url, display: true) { _, _, error in
                if let error {
                    NSLog("openDocument failed for \(url): \(error)")
                }
            }

            onClose?()
            return (nil, nil)
        }
        else if body == "uno .uno:Open" {
            // FIXME A real message would be preferred over intercepting a uno command; but this is what the backstage currently uses
            (NSDocumentController.shared as? DocumentController)?.focusOrPresentOpenPanel()

            onClose?()
            return (nil, nil)
        }

        return nil // not handled
    }

    func exchangeMonitors() {
        let screens = NSScreen.screens
        if (screens.count < 2) {
            return
        }

        let mainWindow = view.window!
        let consoleWindow = self.consoleController != nil ? self.consoleController.window : nil

        var origConsoleScreen = 0
        var origPresentationScreen = 0
        for i in 0...screens.count-1 {
            if (consoleWindow != nil && NSContainsRect(screens[i].frame, consoleWindow!.frame)) {
                origConsoleScreen = i
            }
            if NSContainsRect(screens[i].frame, mainWindow.frame) {
                origPresentationScreen = i
            }
        }

        let newConsoleScreen = (origConsoleScreen + 1) % screens.count
        var newPresentationScreen = origPresentationScreen

        if (consoleWindow != nil) {
            // Rotate the console screen and rotate the presentation screen
            // every time the console catches up to it for the case there
            // are more than two screens. Typically there's just two screens
            // and they just swap.
            if (newConsoleScreen == newPresentationScreen) {
                newPresentationScreen = (newPresentationScreen + 1) % screens.count
            }
        } else {
            newPresentationScreen = (newPresentationScreen + 1) % screens.count
        }

        NotificationCenter.default.removeObserver(self.mainWindowExitFSObserver!)
        installChangeMainMonitor(window: mainWindow, frame: screens[newPresentationScreen].frame)
        // toggle to normal to trigger restore full screen elsewhere
        showNormal(window: mainWindow)

        if (consoleWindow != nil) {
            if (self.consoleWindowExitFSObserver != nil) {
                NotificationCenter.default.removeObserver(self.consoleWindowExitFSObserver!)
            }
            installChangeConsoleMonitor(window: consoleWindow!, frame: screens[newConsoleScreen].frame)
            showNormal(window: consoleWindow!)
        }
    }

    func startPresentation() {
        let mainWindow = self.view.window!

        installDisplayConnectionMonitor()

        self.savedViewFrame = mainWindow.frame

        arrangePresentationWindows()
    }

    func webView(_ webView: WKWebView, createWebViewWith configuration: WKWebViewConfiguration, for navigationAction: WKNavigationAction, windowFeatures: WKWindowFeatures) -> WKWebView? {

        let consoleWebView = WKWebView(frame: .zero, configuration: configuration)
        consoleWebView.uiDelegate = self

        self.consoleController = ConsoleController(webView: consoleWebView)
        let consoleWindow = consoleController.window
        if (consoleWindow != nil) {
            consoleWindow!.collectionBehavior.insert(.fullScreenPrimary)
            // If forced to share screen with presentation allow it to participate in full screen mode
            consoleWindow!.collectionBehavior.insert(.fullScreenAuxiliary)
            // Float over the presentation in full screen mode if they share a screen (auxiliary mode)
            consoleWindow!.level = .floating
            self.savedConsoleViewFrame = consoleWindow!.frame
            self.savedConsoleViewFrame = NSRect(x: 0, y: 0, width: 640, height: 640)
        }

        self.startPresentation()

        return consoleWebView
    }

    func showFullScreen(window: NSWindow) {
        if (!window.styleMask.contains(NSWindow.StyleMask.fullScreen)) {
            window.toggleFullScreen(nil)
        }
    }

    func showNormal(window: NSWindow) {
        if (window.styleMask.contains(NSWindow.StyleMask.fullScreen)) {
            window.toggleFullScreen(nil)
        }
    }

    func arrangePresentationWindows() {
        // The windows should not be full screen at this point
        self.needRearrange = false

        let screens = NSScreen.screens

        var laptopScreen: NSScreen! = nil
        var externalScreen: NSScreen! = nil

        // Lets see if there is are two monitors where one is built-in and one is not.
        for screen in screens {
            let viewDisplayID = screen.deviceDescription[NSDeviceDescriptionKey(rawValue: "NSScreenNumber")] as! CGDirectDisplayID
            if (CGDisplayIsBuiltin(viewDisplayID) != 0) {
                if (laptopScreen == nil) {
                    laptopScreen = screen
                }
            } else {
                if (externalScreen == nil) {
                    externalScreen = screen
                }
            }
        }

        // If not then assume the main screen, which is just where the current activity is,
        // is the laptop screen and pick another to be the external
        if (laptopScreen == nil || externalScreen == nil) {
            laptopScreen = NSScreen.main
            externalScreen = nil
            for screen in screens {
                if (screen != laptopScreen) {
                    externalScreen = screen
                    break
                }
            }
        }

        let consoleWindow = self.consoleController != nil ? self.consoleController.window : nil
        if (consoleWindow != nil) {
            consoleWindow!.setIsVisible(false)
        }

        let presenterScreen: NSScreen = externalScreen != nil ? externalScreen : laptopScreen
        let mainWindow = self.view.window!
        mainWindow.setIsVisible(false)

        installRestorePresOnFullScreenExit(mainWindow: mainWindow, frame: self.savedViewFrame)

        mainWindow.setFrame(presenterScreen.frame, display: true, animate: false)
        showFullScreen(window: mainWindow)
        mainWindow.makeKeyAndOrderFront(nil)
        self.mainFullScreenActive = true

        if (consoleWindow != nil) {
            installRestoreConsoleOnFullScreenExit(consoleWindow: consoleWindow!, frame: self.savedConsoleViewFrame)

            if (externalScreen != nil) {
                consoleWindow!.setFrame(laptopScreen.frame, display: true, animate: false)
                showFullScreen(window: consoleWindow!)
                self.consoleFullScreenActive = true
            } else {
                consoleWindow!.setFrame(self.savedConsoleViewFrame, display: true, animate: false)
            }

            consoleWindow!.makeKeyAndOrderFront(nil)
        }
    }

    func maybeDispatchRearrange() {
        if (!self.needRearrange) {
            return
        }
        if (self.mainFullScreenActive) {
            // considering rearrange, main is still fullscreen, defer
            return
        }
        if (self.consoleFullScreenActive) {
            // considering rearrange, console is still fullscreen, defer
            return
        }
        DispatchQueue.main.async {
            self.arrangePresentationWindows()
        }
    }

    func installRestorePresOnFullScreenExit(mainWindow: NSWindow, frame: NSRect) {
        // Observe full-screen exit, and at that point dispatch the attempt to restore
        // original monitor, size & position. Otherwise we remain on the monitor we are
        // presenting to.
        let center = NotificationCenter.default
        // didExitFullScreenNotification may not be fired on removing/adding screens
        // but the window will have lost its fullscreen bit
        if (self.mainWindowExitFSObserver != nil) {
            center.removeObserver(self.mainWindowExitFSObserver!)
        }
        self.mainWindowExitFSObserver = center.addObserver(
            forName: NSWindow.didExitFullScreenNotification,
            object: mainWindow,
            queue: OperationQueue.main) { _ in

                DispatchQueue.main.async {
                    mainWindow.setFrame(frame, display: true, animate: false)
                    mainWindow.makeKeyAndOrderFront(nil)

                    self.mainFullScreenActive = false
                    self.maybeDispatchRearrange()
                }

                center.removeObserver(self.mainWindowExitFSObserver!)
                self.mainWindowExitFSObserver = nil
        }
    }

    func installRestoreConsoleOnFullScreenExit(consoleWindow: NSWindow, frame: NSRect) {
        // Observe full-screen exit, and at that point dispatch the attempt to restore
        // original monitor, size & position. Otherwise we remain on the monitor we are
        // presenting to.
        let center = NotificationCenter.default
        // didExitFullScreenNotification may not be fired on removing/adding screens
        // but the window will have lost its fullscreen bit
        if (self.consoleWindowExitFSObserver != nil) {
            center.removeObserver(self.consoleWindowExitFSObserver!)
        }
        self.consoleWindowExitFSObserver = center.addObserver(
            forName: NSWindow.didExitFullScreenNotification,
            object: consoleWindow,
            queue: OperationQueue.main) { _ in

                DispatchQueue.main.async {
                    consoleWindow.setFrame(frame, display: true, animate: false)
                    consoleWindow.makeKeyAndOrderFront(nil)

                    self.consoleFullScreenActive = false
                    self.maybeDispatchRearrange()
                }

                center.removeObserver(self.consoleWindowExitFSObserver!)
                self.consoleWindowExitFSObserver = nil
        }
    }

    func installChangeMainMonitor(window: NSWindow, frame: NSRect) {
        let center = NotificationCenter.default
        self.mainMonitorExchangeObserver = center.addObserver(
            forName: NSWindow.didExitFullScreenNotification,
            object: window,
            queue: OperationQueue.main) { _ in

                DispatchQueue.main.async {
                    window.setFrame(frame, display: true, animate: false)
                    self.showFullScreen(window: window)
                }

                self.installRestorePresOnFullScreenExit(mainWindow: window, frame: self.savedViewFrame)
                center.removeObserver(self.mainMonitorExchangeObserver!)
                self.mainMonitorExchangeObserver = nil
        }
    }

    func installDisplayConnectionMonitor() {
        self.screenCount = NSScreen.screens.count
        let center = NotificationCenter.default
        self.displayConnectionObserver = center.addObserver(
            forName: NSApplication.didChangeScreenParametersNotification,
            object: nil,
            queue: OperationQueue.main) { _ in
                if (self.screenCount != NSScreen.screens.count) {
                    self.screenCount = NSScreen.screens.count
                    if (!self.needRearrange) {
                        self.needRearrange = true
                        // dispatch setting windows back to normal and rearrange,
                        // which will recreate at least the presentation window
                        // as fullscreen, when that is completed
                        DispatchQueue.main.async {
                            self.showNormal(window: self.view.window!)
                            let consoleWindow = self.consoleController != nil ? self.consoleController.window : nil
                            if (consoleWindow != nil) {
                                self.showNormal(window: consoleWindow!)
                            }
                        }
                    }
                }
        }
    }

    func installChangeConsoleMonitor(window: NSWindow, frame: NSRect) {
        let center = NotificationCenter.default
        self.consoleMonitorExchangeObserver = center.addObserver(
            forName: NSWindow.didExitFullScreenNotification,
            object: window,
            queue: OperationQueue.main) { _ in

                DispatchQueue.main.async {
                    window.setFrame(frame, display: true, animate: false)
                    self.showFullScreen(window: window)
                    window.makeKeyAndOrderFront(nil)
                }

                self.installRestoreConsoleOnFullScreenExit(consoleWindow: window, frame: self.savedConsoleViewFrame)
                center.removeObserver(self.consoleMonitorExchangeObserver!)
                self.consoleMonitorExchangeObserver = nil
        }
    }

    func endPresentation() {
            if (self.displayConnectionObserver != nil) {
                NotificationCenter.default.removeObserver(self.displayConnectionObserver!)
            }
            if (self.mainMonitorExchangeObserver != nil) {
                NotificationCenter.default.removeObserver(self.mainMonitorExchangeObserver!)
            }
            if (self.consoleMonitorExchangeObserver != nil) {
                NotificationCenter.default.removeObserver(self.consoleMonitorExchangeObserver!)
            }
            if (self.consoleWindowExitFSObserver != nil) {
                NotificationCenter.default.removeObserver(self.consoleWindowExitFSObserver!)
            }

            // this will trigger the restoration of original location/size
            // via the convoluted observer stuff.
            showNormal(window: self.view.window!)
    }

    func webViewDidClose(_ webView: WKWebView) {
        if (self.consoleController != nil && webView == self.consoleController.webView) {
            self.consoleController.close()
            self.consoleController = nil
            self.consoleFullScreenActive = false

            self.endPresentation();
        }
    }
}
