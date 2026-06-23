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

// A borderless window cannot normally become key, so it would not get the keys
// that drive the slideshow. Allow it to.
final class KeyableWindow: NSWindow {
    override var canBecomeKey: Bool { return true }
    override var canBecomeMain: Bool { return true }
}

final class PresentationController: NSWindowController {
    let webView: WKWebView

    init(webView: WKWebView) {
        self.webView = webView
        // Borderless and floating, filled to a screen by setting its frame.
        let window = KeyableWindow(contentRect: NSRect(x: 0, y: 0, width: 800, height: 600),
                                   styleMask: .borderless, backing: .buffered, defer: false)
        window.level = .floating
        window.isReleasedWhenClosed = false
        // Black, so any moment before the slide shows or during teardown is not
        // a flash of white.
        window.backgroundColor = .black
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

    var presentationController: PresentationController!

    /// Whether UI testing mode is active.
    private lazy var isUITesting: Bool = {
        ProcessInfo.processInfo.arguments.contains("--uitesting")
    }()

    /// Offscreen text view that accumulates all "lok" messages for XCUITest assertions.
    private var testMessageLog: NSTextView?

    /// Handle this document webview is registered under in WebDriverManager.
    private var webDriverHandle: String?

    var displayConnectionObserver: AnyObject!
    var screenCount: Int = 0

    override func viewDidLoad() {
        super.viewDidLoad()

        // Setup jsHandler as the entry point to call back from JavaScript
        let contentController = WKUserContentController()
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "lok")
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "clipboard")
        ViewController.addDiagnosticMessageHandlers(to: contentController, handler: self)

        let config = WKWebViewConfiguration()
        config.preferences.isElementFullscreenEnabled = true
        config.userContentController = contentController
        ViewController.allowLocalCrossFrameAccess(config)

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
        }

        // Register this webview with the WebDriver manager (no-op when
        // the WebDriver server is not running).
        webDriverHandle = WebDriverManager.shared.register(webView: webView)
    }

    deinit {
        if let h = webDriverHandle {
            WebDriverManager.shared.unregister(handle: h)
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
        if ViewController.handleDiagnosticMessage(message) {
            return (nil, nil)
        }

        switch message.name {

        case "clipboard":
            if let body = message.body as? String {
                switch body {

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
                    COWrapper.syncSettings(with: document)
                    return (nil, nil)
                }
                else if body.hasPrefix("SETDARKMODE ") {
                    COWrapper.setDarkMode(body == "SETDARKMODE true")
                    return (nil, nil)
                }
                // Options dialog: AI model listing performs a network request, so
                // handle it asynchronously rather than via handleSettingsMessage.
                else if body.hasPrefix("FETCHAIMODELS ") {
                    let payload = String(body.dropFirst("FETCHAIMODELS ".count))
                    return (await ViewController.fetchAIModels(payload), nil)
                }
                // Other Options dialog native backend, shared with the Backstage
                // web view (see handleSettingsMessage).
                else if let settingsResult = ViewController.handleSettingsMessage(body) {
                    return settingsResult
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
                else if body.hasPrefix("exportfile ") {
                    // The kit's KIT_CALLBACK_EXPORT_FILE branch sends this for
                    // export flows like .uno:ExportToPDF (PDF with options).
                    // The file has already been written to a tmp URL; show a
                    // native NSSavePanel and copy it to the user's choice.
                    var fileUrlString: String?
                    for item in body.dropFirst("exportfile ".count).components(separatedBy: " ") {
                        if item.hasPrefix("url=") {
                            fileUrlString = String(item.dropFirst("url=".count))
                        }
                    }
                    guard let urlString = fileUrlString,
                          let srcURL = URL(string: urlString),
                          FileManager.default.fileExists(atPath: srcURL.path) else {
                        COWrapper.LOG_ERR("exportfile: source file missing: \(fileUrlString ?? "(no url)")")
                        return (nil, nil)
                    }

                    let suggestedName = srcURL.lastPathComponent
                    let savePanel = NSSavePanel()
                    savePanel.directoryURL = FileManager.default.homeDirectoryForCurrentUser
                    savePanel.nameFieldStringValue = suggestedName
                    savePanel.begin { result in
                        if result == .OK, let dstURL = savePanel.url {
                            do {
                                if FileManager.default.fileExists(atPath: dstURL.path) {
                                    try FileManager.default.removeItem(at: dstURL)
                                }
                                try FileManager.default.copyItem(at: srcURL, to: dstURL)
                            } catch {
                                COWrapper.LOG_ERR("exportfile: copy failed: \(error)")
                            }
                        }
                        try? FileManager.default.removeItem(at: srcURL)
                    }
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
     * The macOS JavaScript routes postMobileError and postMobileDebug to the "error" and "debug"
     * message handlers. Registers both on the given content controller.
     */
    static func addDiagnosticMessageHandlers(to contentController: WKUserContentController,
                                             handler: WKScriptMessageHandlerWithReply) {
        contentController.addScriptMessageHandler(handler, contentWorld: .page, name: "error")
        contentController.addScriptMessageHandler(handler, contentWorld: .page, name: "debug")
    }

    /** Counterpart to addDiagnosticMessageHandlers, for use when tearing a web view down. */
    static func removeDiagnosticMessageHandlers(from contentController: WKUserContentController?) {
        contentController?.removeScriptMessageHandler(forName: "error")
        contentController?.removeScriptMessageHandler(forName: "debug")
    }

    /** Logs an "error" or "debug" message from a web view. Returns true if it was one of those. */
    static func handleDiagnosticMessage(_ message: WKScriptMessage) -> Bool {
        switch message.name {
        case "error":
            COWrapper.LOG_ERR("Error from WebView: \(message.body as? String ?? "")")
            return true
        case "debug":
            COWrapper.LOG_DBG("Debug from WebView: \(message.body as? String ?? "")")
            return true
        default:
            return false
        }
    }

    /**
     * Functionality shared by the "normal" backstage (that opens when the user chooses "File" in the notebookbar) and the "startup" backstage (that has the templates etc.).
     *
     * @param body: The message (command) that we want to handle
     * @param onClose: In case this is called from backstage that is a separate window & should close the backstage itself, this is what will be called to do that.
     *
     * The return value is either nil (means the message was not handled), or pair that would be returned from the handler.
     */
    /**
     * The UI is loaded from file://, and WKWebView gives file:// frames unique
     * opaque "null" origins, so the Options dialog iframe is cross-origin to its
     * parent and the direct window.parent.postMobileCall() bridge is blocked
     * with a SecurityError (postMessage is exempt, which is why the handshake
     * works). Allow file:// documents to access other file:// documents, which
     * removes that barrier.
     *
     * This is the WKWebView equivalent of what the other apps do with the same
     * file:// UI: the Windows app passes Chromium's --allow-file-access-from-files
     * (CODA.cpp), and QtWebEngine enables LocalContentCanAccessFileUrls by
     * default. On all three the main UI is plain file://; the cool:// scheme only
     * serves embedded media. This is the narrow file->file relaxation, not the
     * broad file->any-origin allowUniversalAccessFromFileURLs. KVC-only setting,
     * applied to our own bundled content.
     */
    static func allowLocalCrossFrameAccess(_ configuration: WKWebViewConfiguration) {
        configuration.preferences.setValue(true, forKey: "allowFileAccessFromFileURLs")
    }

    /**
     * Native backend for the Options dialog (adminIntegratorSettings), shared by
     * the document web view and the Backstage web view. Bridges to the Desktop::
     * layer in common/SettingsStorage.cpp, mirroring the Windows (CODA.cpp) and
     * Qt (Bridge.cpp) apps. Returns nil if `body` is not a settings message.
     */
    static func handleSettingsMessage(_ body: String) -> (Any?, String?)? {
        if body == "FETCHSETTINGSCONFIG" {
            return (COWrapper.fetchSettingsConfig(), nil)
        }
        else if body.hasPrefix("FETCHSETTINGSFILE ") {
            let relPath = String(body.dropFirst("FETCHSETTINGSFILE ".count))
            // The JS caller reads `result.content`, so reply with an object.
            return (["content": COWrapper.fetchSettingsFile(relPath)], nil)
        }
        else if body.hasPrefix("UPLOADSETTINGS ") {
            COWrapper.uploadSettings(String(body.dropFirst("UPLOADSETTINGS ".count)))
            return (nil, nil)
        }
        // FETCHAIMODELS is handled separately (asynchronously) by the message
        // handlers, since it performs a network request; see fetchAIModels().
        return nil
    }

    /**
     * Fetch an AI provider's model list for the Options dialog. The desktop apps
     * have no server-side proxy, so the app issues the request itself, mirroring
     * Qt's Desktop::fetchAIModels(). The payload is {"provider","apiKey","baseUrl"};
     * returns the provider's JSON body verbatim ({"data":[...]} or its own error
     * JSON), or an {"error":...} JSON of our own.
     */
    static func fetchAIModels(_ payload: String) async -> String {
        guard let data = payload.data(using: .utf8),
              let obj = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any] else {
            return #"{"error":"Invalid payload"}"#
        }

        let provider = (obj["provider"] as? String) ?? ""
        let apiKey = (obj["apiKey"] as? String) ?? ""
        var baseUrl = (obj["baseUrl"] as? String) ?? ""

        if provider.isEmpty || apiKey.isEmpty {
            return #"{"error":"Missing provider or apiKey"}"#
        }

        if provider != "custom" {
            // Keep in sync with preCannedAIProviderBaseUrl() in wsd/FileServer.cpp
            // and the same map in qt/Application.cpp.
            let preCanned = [
                "openai": "https://api.openai.com",
                "groq": "https://api.groq.com/openai",
                "together": "https://api.together.xyz",
                "mistral": "https://api.mistral.ai",
            ]
            guard let mapped = preCanned[provider] else {
                return #"{"error":"Unknown provider"}"#
            }
            baseUrl = mapped
        } else if baseUrl.isEmpty {
            return #"{"error":"Missing baseUrl for custom provider"}"#
        }

        if baseUrl.hasSuffix("/") {
            baseUrl.removeLast()
        }
        guard let url = URL(string: baseUrl + "/v1/models") else {
            return #"{"error":"Invalid baseUrl"}"#
        }

        var request = URLRequest(url: url)
        request.setValue("Bearer \(apiKey)", forHTTPHeaderField: "Authorization")
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        do {
            let (body, _) = try await URLSession.shared.data(for: request)
            return String(data: body, encoding: .utf8)
                ?? #"{"error":"Non-text response from the AI provider"}"#
        } catch {
            return #"{"error":"Failed to reach the AI provider"}"#
        }
    }

    static func handleBackstageMessage(_ body: String, onClose: (() -> Void)? = nil) -> (Any?, String?)? {
        if let settingsResult = handleSettingsMessage(body) {
            return settingsResult
        }
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

        let presWindow = self.presentationController != nil ? self.presentationController.window : nil
        let consoleWindow = self.consoleController != nil ? self.consoleController.window : nil

        var origConsoleScreen = 0
        var origPresentationScreen = 0
        for i in 0...screens.count-1 {
            if (consoleWindow != nil && NSContainsRect(screens[i].frame, consoleWindow!.frame)) {
                origConsoleScreen = i
            }
            if (presWindow != nil && NSContainsRect(screens[i].frame, presWindow!.frame)) {
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

        if (presWindow != nil) {
            placeWindow(presWindow!, onScreen: screens[newPresentationScreen], fillScreen: true)
            presWindow!.makeFirstResponder(self.presentationController.webView)
        }
        if (consoleWindow != nil) {
            placeWindow(consoleWindow!, onScreen: screens[newConsoleScreen], fillScreen: true)
        }
    }

    func webView(_ webView: WKWebView, createWebViewWith configuration: WKWebViewConfiguration, for navigationAction: WKNavigationAction, windowFeatures: WKWindowFeatures) -> WKWebView? {

        // The fragment values are set where window.open is called, in
        // browser/src/slideshow/SlideShowPresenter.ts and PresenterConsole.js.
        // The # arrives percent encoded (about:blank%23coda-...), so the
        // fragment accessor is nil; match the token in the whole URL string
        // instead. Only the slideshow and presenter console are claimed here;
        // any other window.open is left to the default behaviour.
        let urlString = navigationAction.request.url?.absoluteString ?? ""
        let isConsole = urlString.contains("coda-console")
        let isPresentation = urlString.contains("coda-presentation")
        if (!isConsole && !isPresentation) {
            return nil
        }

        let childWebView = WKWebView(frame: .zero, configuration: configuration)
        childWebView.uiDelegate = self

        installDisplayConnectionMonitor()

        NSApp.presentationOptions = [.autoHideMenuBar, .autoHideDock]

        // Place only the new window. Re-framing the live slideshow window can
        // disturb its rendering.
        if (isConsole) {
            self.consoleController = ConsoleController(webView: childWebView)
            placeConsoleWindow()
            kickPresentationWindow()
        } else {
            self.presentationController = PresentationController(webView: childWebView)
            placePresentationWindow()
        }

        return childWebView
    }

    // Opening the console window can drop the slideshow window's first presented
    // frame. A static slide renders only once, so force the window to lay out and
    // present again. A frame that really changes is needed; the same frame can be
    // optimised away.
    func kickPresentationWindow() {
        if (self.presentationController == nil) {
            return
        }
        DispatchQueue.main.async {
            guard let presWindow = self.presentationController?.window else { return }
            let (laptopScreen, externalScreen) = self.pickScreens()
            let target = (externalScreen ?? laptopScreen).frame
            var nudge = target
            nudge.origin.y += 1
            presWindow.setFrame(nudge, display: false, animate: false)
            presWindow.setFrame(target, display: true, animate: false)
            presWindow.makeFirstResponder(self.presentationController?.webView)
        }
    }

    // The console stays on the document window's screen; the slideshow goes on
    // any other screen. Keying off the document window's screen is stable: it
    // does not shift when a window becomes key, unlike a main-screen test.
    func pickScreens() -> (NSScreen, NSScreen?) {
        let documentScreen = self.view.window?.screen ?? NSScreen.main ?? NSScreen.screens.first!
        var externalScreen: NSScreen? = nil
        for screen in NSScreen.screens {
            if (screen != documentScreen) {
                externalScreen = screen
                break
            }
        }
        return (documentScreen, externalScreen)
    }

    func placeWindow(_ window: NSWindow, onScreen screen: NSScreen, fillScreen: Bool) {
        if (fillScreen) {
            window.setFrame(screen.frame, display: true, animate: false)
        }
        window.setIsVisible(true)
        window.makeKeyAndOrderFront(nil)
    }

    func placePresentationWindow() {
        guard let presWindow = presentationController?.window else { return }
        let (laptopScreen, externalScreen) = pickScreens()
        let presenterScreen = externalScreen ?? laptopScreen
        placeWindow(presWindow, onScreen: presenterScreen, fillScreen: true)
        presWindow.makeFirstResponder(presentationController.webView)
    }

    func placeConsoleWindow() {
        guard let consoleWindow = consoleController?.window else { return }
        let (laptopScreen, externalScreen) = pickScreens()
        if (externalScreen != nil) {
            // Fill the screen but stay a titled window, because borderless stops
            // its web view rendering. Hide the title bar and buttons and extend
            // the content over them so it still looks full screen.
            consoleWindow.styleMask.insert(.fullSizeContentView)
            consoleWindow.titlebarAppearsTransparent = true
            consoleWindow.titleVisibility = .hidden
            consoleWindow.titlebarSeparatorStyle = .none
            consoleWindow.standardWindowButton(.closeButton)?.isHidden = true
            consoleWindow.standardWindowButton(.miniaturizeButton)?.isHidden = true
            consoleWindow.standardWindowButton(.zoomButton)?.isHidden = true
            placeWindow(consoleWindow, onScreen: laptopScreen, fillScreen: true)
        } else {
            // One screen: the console floats on top of the full screen
            // presentation. Give it most of the work area, but leave a margin
            // so the presentation stays visible around the edges and the user
            // can see it is running underneath.
            // The presentation window is at the floating level, so the console
            // has to be at least that level too, otherwise it would be drawn
            // behind the presentation no matter the front-to-back order.
            consoleWindow.level = .floating
            let area = laptopScreen.visibleFrame
            let size = NSSize(width: area.width * 17 / 20, height: area.height * 17 / 20)
            let origin = NSPoint(x: area.minX + (area.width - size.width) / 2,
                                 y: area.minY + (area.height - size.height) / 2)
            consoleWindow.setFrame(NSRect(origin: origin, size: size), display: true, animate: false)
            consoleWindow.makeKeyAndOrderFront(nil)
        }
    }

    // Re-place both windows, for when the screen layout changes.
    func arrangePresentationWindows() {
        placePresentationWindow()
        placeConsoleWindow()
    }

    func installDisplayConnectionMonitor() {
        if (self.displayConnectionObserver != nil) {
            return
        }
        self.screenCount = NSScreen.screens.count
        let center = NotificationCenter.default
        self.displayConnectionObserver = center.addObserver(
            forName: NSApplication.didChangeScreenParametersNotification,
            object: nil,
            queue: OperationQueue.main) { _ in
                if (self.screenCount != NSScreen.screens.count) {
                    self.screenCount = NSScreen.screens.count
                    self.arrangePresentationWindows()
                }
        }
    }

    func endPresentation() {
        if (self.displayConnectionObserver != nil) {
            NotificationCenter.default.removeObserver(self.displayConnectionObserver!)
            self.displayConnectionObserver = nil
        }

        if (self.presentationController != nil) {
            self.presentationController.window?.orderOut(nil)
            self.presentationController.close()
            self.presentationController = nil
        }
        if (self.consoleController != nil) {
            self.consoleController.window?.orderOut(nil)
            self.consoleController.close()
            self.consoleController = nil
        }

        // Bring back the menu bar and Dock.
        NSApp.presentationOptions = []
    }

    func webViewDidClose(_ webView: WKWebView) {
        // Closing either window ends the show and tears down the other too.
        let isPresentation = self.presentationController != nil && webView == self.presentationController.webView
        let isConsole = self.consoleController != nil && webView == self.consoleController.webView
        if (isPresentation || isConsole) {
            endPresentation()
        }
    }
}
