/*
 * Copyright the Collabora Office contributors.
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

/// A window that shows the integrator's site for one cloud account, as a browser would.
final class CloudAccountWindow: NSWindowController, NSWindowDelegate, WKNavigationDelegate,
                                WKUIDelegate, WKScriptMessageHandler, WKScriptMessageHandlerWithReply {
    private static let defaultSize = NSSize(width: 1200, height: 800)
    // CODA's scripts run in this "coda" world. The COOL server's scripts run in WebKit's default
    // page world, which has no access to CODA's message handlers.
    static let world = WKContentWorld.world(name: "coda")
    private static let firstMacOSWithSafariVersion = 26
    private static let safariVersionsAheadOfMacOS = 3
    // WebKit's error code for a navigation that decidePolicyFor cancelled:
    private static let frameLoadInterruptedByPolicyChange = 102

    /// window.bridge calls that native code does not answer yet. They are not engine messages.
    private static let unansweredBridgeCalls: Set<Substring> = [
        "getAllPrefs",
        "setPref"
    ]

    private static var openWindows: [String: CloudAccountWindow] = [:]

    private let accountId: String
    private let webView: WKWebView
    private var integratorOrigin: String?
    private var blockedOrigins: [String: WKContentRuleList] = [:]
    // The origins whose documents open in the server's own editor, with the reason of each:
    private var serverEditorReasons: [String: String] = [:]
    // The notice above the integrator's site, which says why the editor frame shows the server's
    // own editor:
    private let notice = NSStackView()
    private let noticeText = NSTextField(wrappingLabelWithString: "")
    // The space around the notice's text, in points:
    private static let noticeInsets = NSEdgeInsets(top: 8, left: 12, bottom: 8, right: 8)
    private static let noticeCloseWidth: CGFloat = 24
    private var editorForm: Data?
    private var editorFormUrl: URL?
    private var editorFrameInfo: WKFrameInfo?
    /// The local copy of the document open in the editor frame, edited by the engine.
    private var remoteDocument: Document?

    /// The remote document's name, like "calc.ods".
    private var remoteDocumentName: String?

    // The editor frame asked for the view settings before its document existed:
    private var syncSettingsPending = false

    private var hasShownPage = false

    static func open(accountId: String, url: URL) {
        if let controller = openWindows[accountId] {
            controller.showWindow(nil)
            return
        }

        let controller = CloudAccountWindow(accountId: accountId, url: url)
        openWindows[accountId] = controller
        controller.showWindow(nil)
    }

    private init(accountId: String, url: URL) {
        self.accountId = accountId
        self.integratorOrigin = Self.origin(of: url)

        let window = NSWindow(contentRect: NSRect(origin: .zero, size: Self.defaultSize),
                              styleMask: [.titled, .closable, .resizable, .miniaturizable],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.title = AppDelegate.getAppName()
        window.center()

        // Integrators refuse a browser they do not know. Safari always names WebKit 605.1.15.
        let configuration = WKWebViewConfiguration()
        configuration.applicationNameForUserAgent = "Version/\(Self.safariVersion) Safari/605.1.15"

        let frameLoaded = WKUserScript(
            source: "window.webkit.messageHandlers.frameLoaded.postMessage(null)",
            injectionTime: .atDocumentEnd,
            forMainFrameOnly: false,
            in: Self.world
        )

        // WebKit tells us when the main page has loaded, but not when an
        // iframe has, so every frame reports the end of its load itself.
        configuration.userContentController.addUserScript(frameLoaded)

        webView = WKWebView(frame: .zero, configuration: configuration)
        ViewController.applyUIZoom(to: webView)
#if DEBUG
        webView.isInspectable = true
#endif

        super.init(window: window)
        window.contentView = makeContent()
        window.delegate = self
        webView.navigationDelegate = self
        webView.uiDelegate = self

        let contentController = webView.configuration.userContentController
        contentController.add(self, contentWorld: Self.world, name: "frameLoaded")
        contentController.addScriptMessageHandler(self, contentWorld: Self.world, name: "lok")
        ViewController.addDiagnosticMessageHandlers(to: contentController, handler: self, world: Self.world)
        webView.load(URLRequest(url: url))
    }

    // Safari updates apart from macOS, so its own bundle has the version WebKit is at.
    private static var safariVersion: String {
        let safari = Bundle(path: "/Applications/Safari.app")
        if let version = safari?.infoDictionary?["CFBundleShortVersionString"] as? String {
            return version
        }

        let macOSVersion = ProcessInfo.processInfo.operatingSystemVersion
        var major = macOSVersion.majorVersion
        // Before macOS 26, Safari's version was 3 higher than the macOS version.
        if major < Self.firstMacOSWithSafariVersion {
            major += Self.safariVersionsAheadOfMacOS
        }

        return "\(major).\(macOSVersion.minorVersion)"
    }

    required init?(coder: NSCoder) {
        fatalError("CloudAccountWindow is created in code only")
    }

    static var openAccountWindows: [NSWindow] {
        return openWindows.values.compactMap { $0.window }
    }

    private static var appDelegate: AppDelegate? {
        return NSApp.delegate as? AppDelegate
    }

    // The page saves, then sends CLOSE_WINDOW.
    private static let saveAndCloseScript = "window.app.map._saveAndClose()"

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        guard remoteDocument != nil, let editorFrameInfo else {
            return true
        }

        webView.evaluateJavaScript(Self.saveAndCloseScript, in: editorFrameInfo, in: Self.world) { result in
            if case .failure = result {
                COWrapper.LOG_ERR("The editor page was not asked to save")
                self.close()
            }
        }

        return false
    }

    // The integrator's site, below the notice while that shows:
    private func makeContent() -> NSView {
        let close = NSButton(
            image: NSImage(systemSymbolName: "xmark", accessibilityDescription: "Close") ?? NSImage(),
            target: self, action: #selector(hideNotice))
        close.isBordered = false
        close.toolTip = "Close"
        close.widthAnchor.constraint(equalToConstant: Self.noticeCloseWidth).isActive = true
        noticeText.setContentCompressionResistancePriority(.defaultLow, for: .horizontal)
        notice.orientation = .horizontal
        notice.alignment = .top
        notice.edgeInsets = Self.noticeInsets
        notice.addArrangedSubview(noticeText)
        notice.addArrangedSubview(close)
        notice.setHuggingPriority(.defaultHigh, for: .vertical)
        notice.wantsLayer = true
        notice.isHidden = true

        let content = NSStackView(views: [notice, webView])
        content.orientation = .vertical
        content.spacing = 0
        for view in [notice, webView] {
            view.widthAnchor.constraint(equalTo: content.widthAnchor).isActive = true
        }
        webView.setContentHuggingPriority(.defaultLow, for: .vertical)
        return content
    }

    // Shows the text above the integrator's site, until the user closes it:
    private func showNotice(_ text: String) {
        noticeText.stringValue = text
        notice.layer?.backgroundColor = NSColor.systemYellow.withAlphaComponent(0.2).cgColor
        notice.isHidden = false
        fitNoticeText()
    }

    @objc private func hideNotice() {
        notice.isHidden = true
    }

    // The notice's text wraps at the width that the window leaves it:
    private func fitNoticeText() {
        let width = window?.contentView?.bounds.width ?? Self.defaultSize.width
        noticeText.preferredMaxLayoutWidth = max(
            0, width - Self.noticeInsets.left - Self.noticeInsets.right - notice.spacing - Self.noticeCloseWidth)
    }

    func windowDidResize(_ notification: Notification) {
        fitNoticeText()
    }

    func windowWillClose(_ notification: Notification) {
        let contentController = webView.configuration.userContentController
        contentController.removeScriptMessageHandler(forName: "frameLoaded", contentWorld: Self.world)
        contentController.removeScriptMessageHandler(forName: "lok", contentWorld: Self.world)
        ViewController.removeDiagnosticMessageHandlers(from: contentController, world: Self.world)
        closeRemoteDocument()
        Self.openWindows[accountId] = nil

        if Self.openWindows.isEmpty {
            Self.appDelegate?.lastAccountWindowClosed()
        }
    }

    // WebKit fills in frameInfo, so a page cannot pass itself off as another frame.
    private func isEditorFrame(_ frameInfo: WKFrameInfo) -> Bool {
        return !frameInfo.isMainFrame && frameInfo.request.url == editorFormUrl
    }

    // "https://host", with ":port" only when a port is given. WebKit gives port 0 for none.
    private static func origin(scheme: String?, host: String?, port: Int?) -> String? {
        guard let scheme = scheme?.lowercased(), let host = host?.lowercased(), !host.isEmpty else {
            return nil
        }

        guard let port, port != 0 else {
            return scheme + "://" + host
        }

        return scheme + "://" + host + ":\(port)"
    }

    private static func origin(of url: URL) -> String? {
        return origin(scheme: url.scheme, host: url.host, port: url.port)
    }

    private static func origin(of securityOrigin: WKSecurityOrigin) -> String? {
        return origin(scheme: securityOrigin.protocol, host: securityOrigin.host, port: securityOrigin.port)
    }

    // comes from WKScriptMessageHandler
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let frameInfo = message.frameInfo
        if !isEditorFrame(frameInfo) {
            COWrapper.LOG_TRC("A frame other than the editor frame loaded")
            return
        }

        editorFrameInfo = frameInfo
        COWrapper.LOG_DBG("The editor frame loaded")
        // The frame's earlier page ends together with its document and its pending SYNCSETTINGS
        // (also when that page sent no BYE):
        closeRemoteDocument()
        syncSettingsPending = false
        fillEditorFrame(frameInfo)
    }

    // CODA's page goes into the editor frame, unless the server allows no client document loading:
    private func fillEditorFrame(_ frameInfo: WKFrameInfo) {
        guard let editorFormUrl, let editorForm else {
            return
        }

        // A server without client document loading already shows its own editor in the frame (with
        // the notice shown only for its first document in the window):
        if let origin = Self.origin(of: editorFormUrl), serverEditorReasons[origin] != nil {
            COWrapper.LOG_DBG("The server allows no client document loading, so the frame shows the server's own editor")
            return
        }

        hideNotice()
        let script = "return (" + COWrapper.clientDocumentLoadingRefusalScript() + ")(editorUrl)"
        webView.callAsyncJavaScript(
            script,
            arguments: ["editorUrl": editorFormUrl.absoluteString],
            in: frameInfo,
            in: Self.world
        ) { result in
            if case .success(let value) = result, let reason = value as? String {
                self.useServerEditor(reason: reason, editorUrl: editorFormUrl, form: editorForm, in: frameInfo)
            } else {
                let accessToken = COWrapper.editorPageAccessToken(
                    formUrl: editorFormUrl.absoluteString, form: editorForm)
                self.putCodaPageIn(frameInfo, accessToken: accessToken)
            }
        }
    }

    // The frame gets the integrator's form posted again, with all of the server's files, for the
    // server's own editor, for one of the reasons in common/EditorFrameScripts.hpp:
    private func useServerEditor(reason: String, editorUrl: URL, form: Data, in frameInfo: WKFrameInfo) {
        guard COWrapper.isServerEditorReason(reason), let origin = Self.origin(of: editorUrl) else {
            COWrapper.LOG_ERR("The server's own editor was refused: an unknown reason")
            return
        }

        // A document that the page switched to the server goes to the server's own editor without a
        // notice, and the next document from that server goes to CODA's page again:
        if reason == COWrapper.switchReason() {
            COWrapper.LOG_DBG("The editor frame's page switched its document to the server, so the frame shows the server's own editor")
        } else {
            COWrapper.LOG_DBG("No client document loading (\(reason)), so the frame shows the server's own editor")
            serverEditorReasons[origin] = reason
            showServerEditorNotice(reason: reason, editorUrl: editorUrl)
        }
        unblockCoolServerFiles(of: editorUrl)
        postServerEditorForm(editorUrl: editorUrl, form: form, in: frameInfo)
    }

    private func postServerEditorForm(editorUrl: URL, form: Data, in frameInfo: WKFrameInfo) {
        webView.callAsyncJavaScript(
            "(" + COWrapper.serverEditorFormScript() + ")(editorUrl, formBody)",
            arguments: ["editorUrl": editorUrl.absoluteString, "formBody": String(decoding: form, as: UTF8.self)],
            in: frameInfo,
            in: Self.world
        ) { result in
            if case .failure = result {
                COWrapper.LOG_ERR("The form was not posted again for the server's own editor")
            }
        }
    }

    // Whether the frame shows the server's own editor for the kept form (with the fragment after a
    // fallback or a switch, or without it for a server that is known to refuse):
    private func showsServerEditor(_ frameInfo: WKFrameInfo) -> Bool {
        guard !frameInfo.isMainFrame, isKeptFormAddress(frameInfo.request.url), let editorFormUrl
        else {
            return false
        }

        if frameInfo.request.url?.fragment == COWrapper.serverEditorFragment() {
            return true
        }
        return Self.origin(of: editorFormUrl).map { serverEditorReasons[$0] != nil } == true
    }

    // Whether the address is that of the kept form, with or without a fragment (compared as text,
    // as the kept address can have a fragment of its own, and characters that URLComponents
    // changes):
    private func isKeptFormAddress(_ url: URL?) -> Bool {
        guard let url, let editorFormUrl else {
            return false
        }

        return Self.beforeFragment(url) == Self.beforeFragment(editorFormUrl)
    }

    private static func beforeFragment(_ url: URL) -> Substring {
        return url.absoluteString.prefix { $0 != "#" }
    }

    private func showServerEditorNotice(reason: String, editorUrl: URL) {
        let server = editorUrl.host ?? "The server"
        let app = AppDelegate.getAppName()
        if reason == COWrapper.disabledReason() {
            showNotice(
                "\(server) does not allow editing in \(app), so the document is open in the server's own editor. "
                + "To edit documents from \(server) in \(app), ask the server's administrator to allow that "
                + "(client_document_loading.enable in coolwsd.xml).")
        } else if reason == COWrapper.outdatedReason() {
            showNotice(
                "This version of \(app) is too old to edit documents from \(server), so the document is open in the "
                + "server's own editor. To edit documents from \(server) in \(app), update your copy of \(app).")
        } else {
            showNotice(
                "\(server) runs a version of Collabora Online that does not support editing in \(app), "
                + "so the document is open in the server's own editor.")
        }
    }

    private func showRemoteFailure(_ error: String, in frameInfo: WKFrameInfo) {
        COWrapper.LOG_ERR("The remote bootstrap failed in the editor frame")

        var server = "the Collabora Online server"
        if let editorFormUrl, let origin = Self.origin(of: editorFormUrl) {
            server = origin
        }

        let title = "Cannot open remote document"
        let paragraphs = [
            "Could not open the document from \(server).",
            "Details: \(error)",
        ]

        let script = COWrapper.failurePageScript(title: title, paragraphs: paragraphs)
        webView.evaluateJavaScript(script, in: frameInfo, in: Self.world)
    }

    // comes from WKScriptMessageHandlerWithReply
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage,
                               replyHandler: @escaping @MainActor @Sendable (Any?, String?) -> Void) {
        if let reply = handleScriptMessage(message, replyHandler: replyHandler) {
            replyHandler(reply.0, reply.1)
        }
    }

    // A nil result means that the reply comes later, through replyHandler:
    private func handleScriptMessage(_ message: WKScriptMessage,
                                     replyHandler: @escaping @MainActor @Sendable (Any?, String?) -> Void) -> (Any?, String?)? {
        if !isEditorFrame(message.frameInfo) {
            COWrapper.LOG_DBG("A message was dropped: it is not from the editor frame")
            return (nil, nil)
        }

        if ViewController.handleDiagnosticMessage(message) {
            return (nil, nil)
        }

        // Only "lok" messages reach this point.
        let body = message.body as? String ?? ""
        if !COWrapper.isAllowedFromEditorFrame(body) {
            return (nil, nil)
        }

        let kind = body.prefix { $0 != " " }
        if kind == "getBundledFile" {
            return (Self.bundledFile(body.dropFirst(kind.count + 1)), nil)
        }

        if kind == "writeRemoteDocFile" {
            return (writeRemoteDocFile(body.dropFirst(kind.count + 1)), nil)
        }

        if Self.unansweredBridgeCalls.contains(kind) {
            COWrapper.LOG_DBG("\(kind) is not answered yet")
            return (nil, nil)
        }

        // The page asks for the server's own editor in the frame, also after it switched its
        // document to the server and so ended the local document:
        if kind == "serverEditor" {
            if let editorFormUrl, let editorForm {
                useServerEditor(
                    reason: String(body.dropFirst(kind.count + 1)), editorUrl: editorFormUrl, form: editorForm,
                    in: message.frameInfo)
            }
            return (nil, nil)
        }

        if kind == "REMOTEBOOTSTRAPFAILED" {
            showRemoteFailure(String(body.dropFirst(kind.count + 1)), in: message.frameInfo)
            return (nil, nil)
        }

        guard let remoteDocument else {
            COWrapper.LOG_DBG("\(kind) came before the document")
            // The page asks for the view settings as it starts, before it fetches its document, so
            // the answer waits for the document:
            if body == "SYNCSETTINGS" {
                syncSettingsPending = true
            }
            return (nil, nil)
        }

        if body == "BYE" {
            closeRemoteDocument()
            return (nil, nil)
        }

        if kind == "readLocalDocBytes" {
            return (localCopyBytes(of: remoteDocument), nil)
        }

        // TODO: Keep a flag while a save is uploading, as Qt does, so closing the window waits for
        // the upload instead of losing it.
        let isSaveReport = kind == "saveCompleted" || kind == "SAVESTARTED" || kind == "SAVECOMPLETED"
            || kind == "SAVEFAILED"
        if isSaveReport {
            return (nil, nil)
        }

        if kind == "CLOSE_CANCELLED" {
            Self.appDelegate?.accountWindowKeptOpen()
            return (nil, nil)
        }

        if kind == "CLOSE_WINDOW" {
            close()
            return (nil, nil)
        }

        // The page edits on the COOL server from now on.
        if kind == "switchToServerMode" {
            closeRemoteDocument()
            return (nil, nil)
        }

        if ViewController.handleFetchAIModels(body, replyHandler: replyHandler) {
            return nil
        }

        if let result = ViewController.handleDocumentMessage(body, document: remoteDocument) {
            return result
        }

        COWrapper.handleMessage(with: remoteDocument, message: body)
        return (nil, nil)
    }

    // The account window stays open when its document closes.
    private func closeRemoteDocument() {
        remoteDocument?.removeWindowController(self)
        remoteDocument?.close()
        remoteDocument = nil
        remoteDocumentName = nil
        window?.title = AppDelegate.getAppName()
    }

    // The title shows the remote document's name, not the name of its temporary copy.
    override func windowTitle(forDocumentDisplayName displayName: String) -> String {
        guard let remoteDocumentName else {
            return AppDelegate.getAppName()
        }

        return AppDelegate.getAppName() + " - " + remoteDocumentName
    }

    // The arguments are a JSON array with the integrator's file name and the bytes in base64.
    private func writeRemoteDocFile(_ arguments: Substring) -> String {
        let parsed = try? JSONSerialization.jsonObject(with: Data(arguments.utf8))
        let values = parsed as? [String] ?? []
        let bytes = values.count == 2 ? Data(base64Encoded: values[1]) : nil
        guard let bytes else {
            COWrapper.LOG_ERR("writeRemoteDocFile has no name and bytes")
            return ""
        }

        // Only the extension is kept, because the engine picks its import filter by it.
        let fileExtension = (values[0] as NSString).pathExtension
        if !COWrapper.isPlainExtension(fileExtension) {
            COWrapper.LOG_ERR("writeRemoteDocFile has a name with no plain extension: \"\(fileExtension)\"")
            return ""
        }

        if !Self.documentExtensions.contains(fileExtension.lowercased()) {
            COWrapper.LOG_ERR("writeRemoteDocFile: extension \(fileExtension) not supported")
            return ""
        }

        guard let editorFrameInfo else {
            COWrapper.LOG_ERR("writeRemoteDocFile has no editor frame")
            return ""
        }

        let document = Document()
        do {
            try document.writeTemporaryCopy(bytes, named: "document." + fileExtension)
        } catch {
            COWrapper.LOG_ERR("writeRemoteDocFile could not write the local copy")
            return ""
        }

        // AppKit sets the type of a document it opens. Without one, NSDocument.fileType asks
        // writableTypes(for:), which asks fileType again leading to a stack overflow.
        let fileType = document.tempFileURL.flatMap { try? NSDocumentController.shared.typeForContents(of: $0) }
        guard let fileType else {
            COWrapper.LOG_ERR("writeRemoteDocFile has a file of no known type")
            return ""
        }

        document.fileType = fileType
        document.connect(webView: webView)
        document.editorFrame = editorFrameInfo
        COWrapper.setInEditorFrame(document.appDocId)

        remoteDocumentName = values[0]

        // The document shows its Save As, print and error dialogs on this window.
        document.addWindowController(self)
        remoteDocument = document
        if syncSettingsPending {
            syncSettingsPending = false
            _ = ViewController.handleDocumentMessage("SYNCSETTINGS", document: document)
        }
        return document.tempFileURL?.path ?? ""
    }

    // The engine saves into the local copy, and the page uploads these bytes to the integrator.
    private func localCopyBytes(of document: Document) -> String {
        guard let url = document.tempFileURL, let bytes = try? Data(contentsOf: url) else {
            COWrapper.LOG_ERR("readLocalDocBytes could not read the local copy")
            return ""
        }

        return bytes.base64EncodedString()
    }

    // App declared document extensions, like "odt" or "csv".
    private static let documentExtensions = Set(Document.readableTypes.flatMap {
        UTType($0)?.tags[.filenameExtension] ?? []
    })

    // The arguments are a JSON array with the path, like ["device-desktop.css"].
    private static func bundledFile(_ arguments: Substring) -> String {
        let parsed = try? JSONSerialization.jsonObject(with: Data(arguments.utf8))
        guard let path = (parsed as? [String])?.first else {
            COWrapper.LOG_ERR("getBundledFile has no path")
            return ""
        }

        guard let text = appFile(path) else {
            COWrapper.LOG_DBG("Bundled file not found: \(path)")
            return ""
        }

        return text
    }

    // WebKit cannot reply to a request in place of coolwsd, as WebView2 can, and
    // coolwsd's policy refuses scripts from an app scheme. So CODA's page goes in as text.
    private static let putCodaPageInScript = """
        window.codaRemotePage = { accessToken };

        const codaPage = new DOMParser().parseFromString(codaHtml, 'text/html');
        const scripts = Array.from(codaPage.scripts, (script) => script.getAttribute('src'));
        const styleLinks = codaPage.querySelectorAll('link[rel="stylesheet"]');
        const styles = Array.from(styleLinks, (link) => link.getAttribute('href'));
        codaPage.querySelectorAll('script, link[rel="stylesheet"]').forEach((element) => element.remove());

        document.documentElement.replaceWith(document.adoptNode(codaPage.documentElement));
        return { scripts, styles };
        """

    // As on Qt and Windows, a path may not leave the app's resources or name a hidden file.
    private static func appFile(_ path: String) -> String? {
        for segment in path.split(separator: "/", omittingEmptySubsequences: false) {
            let isHidden = segment.hasPrefix(".")
            let hasSeparator = segment.contains("\\") || segment.contains(":")
            if segment.isEmpty || isHidden || hasSeparator {
                COWrapper.LOG_ERR("A file from CODA's page was refused: its path leads out of the folder")
                return nil
            }
        }

        guard let resources = Bundle.main.resourceURL else {
            return nil
        }

        return try? String(contentsOf: resources.appendingPathComponent(path), encoding: .utf8)
    }

    private func putCodaPageIn(_ frameInfo: WKFrameInfo, accessToken: String) {
        guard let codaHtml = Self.appFile("cool.html") else {
            COWrapper.LOG_ERR("CODA's page was not put in: the app has no cool.html")
            return
        }

        webView.callAsyncJavaScript(
            Self.putCodaPageInScript,
            arguments: ["codaHtml": codaHtml, "accessToken": accessToken],
            in: frameInfo,
            in: Self.world
        ) { result in
            guard case .success(let value) = result, let files = value as? [String: [String]] else {
                COWrapper.LOG_ERR("CODA's page was not put in the editor frame")
                return
            }

            COWrapper.LOG_DBG("CODA's page is in the editor frame")
            self.adoptStyleSheets(files["styles"] ?? [], in: frameInfo)
            self.runScripts(files["scripts"] ?? [], in: frameInfo)
        }
    }

    // coolwsd's policy lets images in as data: URLs, so the app's images, about 15 MB, are kept
    // in a map like "images/lc_save.svg": "data:image/svg+xml;base64,...".
    private static let appImages = readAppImages()

    private static func readAppImages() -> [String: String] {
        guard let resources = Bundle.main.resourceURL else {
            return [:]
        }

        let folder = resources.appendingPathComponent("images")
        let mimeTypes = [
            "svg": "image/svg+xml",
            "png": "image/png",
            "gif": "image/gif"
        ]

        let files = FileManager.default.enumerator(
            at: folder,
            includingPropertiesForKeys: nil
        )

        var images: [String: String] = [:]

        while let file = files?.nextObject() as? URL {
            guard let mimeType = mimeTypes[file.pathExtension], let data = try? Data(contentsOf: file) else {
                continue
            }

            let path = String(file.path.dropFirst(resources.path.count + 1))
            images[path] = "data:\(mimeType);base64," + data.base64EncodedString()
        }

        return images
    }

    // coolwsd's policy refuses a <style> element, but not a style sheet made in script.
    private func adoptStyleSheets(_ paths: [String], in frameInfo: WKFrameInfo) {
        var sheets: [String] = []

        for path in paths {
            guard let css = Self.appFile(path) else {
                COWrapper.LOG_DBG("Style sheet not found: \(path)")
                continue
            }

            sheets.append(css)
        }

        // Each url(images/...) in a style sheet would load from coolwsd, so the script replaces it
        // with that image's data: URL from appImages. The quotes around the path are optional.
        let script = """
            window.codaRemotePage.images = images;

            const imageUrl = /url\\((['"]?)(images\\/[^'")]+)\\1\\)/g;
            const withImages = (css) => css.replace(imageUrl, (url, quote, path) =>
                images[path] ? `url("${images[path]}")` : url);

            document.adoptedStyleSheets = sheets.map((css) => {
                const sheet = new CSSStyleSheet();
                sheet.replaceSync(withImages(css));
                return sheet;
            });
            """

        webView.callAsyncJavaScript(
            script,
            arguments: ["sheets": sheets, "images": Self.appImages],
            in: frameInfo,
            in: Self.world
        ) { result in
            if case .failure = result {
                COWrapper.LOG_ERR("CODA's style sheets were not attached in the editor frame")
            }
        }
    }

    private func runScripts(_ paths: [String], in frameInfo: WKFrameInfo) {
        for path in paths {
            guard let source = Self.appFile(path) else {
                COWrapper.LOG_DBG("Script not found: \(path)")
                continue
            }

            // "void 0": the script ends on undefined, which WebKit can pass back.
            // "sourceURL": names the script in the Web Inspector.
            let program = source + "\n;void 0\n//# sourceURL=" + path
            webView.evaluateJavaScript(program, in: frameInfo, in: Self.world) { result in
                if case .failure = result {
                    COWrapper.LOG_ERR("Script failed: \(path)")
                }
            }
        }

        // Both events fired before the editor's scripts ran, so fire them again.
        let pageLoaded = """
            document.dispatchEvent(new Event('DOMContentLoaded'));
            window.dispatchEvent(new Event('load'));
        """
        webView.evaluateJavaScript(pageLoaded, in: frameInfo, in: Self.world)
    }

    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction,
                 decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
        // A request for a new window has no target frame. Allowing it makes WebKit call
        // createWebViewWith below, which opens only http and https addresses, in the browser.
        guard let targetFrame = navigationAction.targetFrame else {
            decisionHandler(.allow)
            return
        }

        if targetFrame.isMainFrame {
            decideMainFrameNavigation(navigationAction, decisionHandler: decisionHandler)
            return
        }

        decideFrameNavigation(navigationAction, decisionHandler: decisionHandler)
    }

    // The account window stays on the integrator's site. A link the user clicks to another site
    // opens in the browser, and anything else that would leave the site is refused.
    private func decideMainFrameNavigation(_ navigationAction: WKNavigationAction,
                                           decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
        guard let url = navigationAction.request.url else {
            decisionHandler(.allow)
            return
        }

        let targetOrigin = Self.origin(of: url)
        if targetOrigin != nil && targetOrigin == integratorOrigin {
            decisionHandler(.allow)
            return
        }

        // Before the first page shows, a navigation is a redirect of the account address.
        if !hasShownPage {
            var isWithinSite = false
            if let integratorOrigin, let targetOrigin {
                isWithinSite = COWrapper.isWithinSite(integratorOrigin, to: targetOrigin)
            }

            if isWithinSite {
                integratorOrigin = targetOrigin
                COWrapper.LOG_DBG("The account address redirects within its site")
                decisionHandler(.allow)
                return
            }

            decisionHandler(.cancel)
            COWrapper.LOG_ERR("The account window refused: the account address redirects to another site")
            refuseRedirect(to: targetOrigin ?? url.absoluteString)
            return
        }

        decisionHandler(.cancel)
        if navigationAction.navigationType == .linkActivated {
            Self.openInBrowser(url)
            return
        }

        COWrapper.LOG_ERR("The account window was kept from going to \(targetOrigin ?? "a page with no origin")")
    }

    private func refuseRedirect(to targetOrigin: String) {
        let accountOrigin = integratorOrigin ?? "The account address"
        let text = "\(accountOrigin) redirected to \(targetOrigin), "
            + "so the account was not opened. "
            + "Only a redirect within the same site is followed."

        guard let window else {
            close()
            return
        }

        let alert = NSAlert()
        alert.messageText = "Account not opened"
        alert.informativeText = text
        alert.beginSheetModal(for: window) { _ in
            self.close()
        }
    }

    // window.open and target="_blank" links: only web addresses are opened, in the browser.
    func webView(_ webView: WKWebView, createWebViewWith configuration: WKWebViewConfiguration,
                 for navigationAction: WKNavigationAction, windowFeatures: WKWindowFeatures) -> WKWebView? {
        if let url = navigationAction.request.url {
            Self.openInBrowser(url)
        }

        return nil
    }

    func webView(_ webView: WKWebView, didCommit navigation: WKNavigation!) {
        hasShownPage = true
    }

    func webView(
        _ webView: WKWebView, didFailProvisionalNavigation navigation: WKNavigation!,
        withError error: Error)
    {
        reportFailedLoad(error)
    }

    func webView(_ webView: WKWebView, didFail navigation: WKNavigation!, withError error: Error) {
        reportFailedLoad(error)
    }

    private func reportFailedLoad(_ error: Error) {
        let nsError = error as NSError
        if nsError.domain == NSURLErrorDomain && nsError.code == NSURLErrorCancelled {
            return
        }
        if nsError.domain == "WebKitErrorDomain"
            && nsError.code == Self.frameLoadInterruptedByPolicyChange
        {
            return
        }
        COWrapper.LOG_ERR("The account window could not load a page: \(nsError)")
        guard let window else {
            return
        }
        // A window that has not shown a page yet closes once the error is dismissed:
        NSAlert(error: error).beginSheetModal(for: window) { _ in
            if !self.hasShownPage {
                self.close()
            }
        }
    }

    private static func openInBrowser(_ url: URL) {
        guard url.scheme == "http" || url.scheme == "https" else {
            COWrapper.LOG_ERR("A link was not opened: URL scheme \(url.scheme ?? "none") not supported")
            return
        }

        NSWorkspace.shared.open(url)
    }

    // The integrator opens a document by posting a form with the token and settings into the
    // iframe that holds the editor. WebKit gives us the form's body here, and we keep a copy.
    private func decideFrameNavigation(_ navigationAction: WKNavigationAction,
                                       decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
        let request = navigationAction.request
        guard let url = request.url else {
            decisionHandler(.allow)
            return
        }

        let isPost = request.httpMethod == "POST"
        let isEditorPage = url.path.hasSuffix("/cool.html")

        // A reload of the server's own editor gets the kept form posted again (as WebKit reloads
        // the frame with a GET, which carries no access token):
        if navigationAction.navigationType == .reload && !isPost && isEditorPage,
           let frame = navigationAction.targetFrame, showsServerEditor(frame), let editorFormUrl,
           let editorForm
        {
            COWrapper.LOG_DBG("The server's own editor reloads, so its form is posted again")
            decisionHandler(.cancel)
            postServerEditorForm(editorUrl: editorFormUrl, form: editorForm, in: frame)
            return
        }

        if !isPost || !isEditorPage {
            decisionHandler(.allow)
            return
        }

        // The editor frame gets the integrator's form posted again for the server's own editor:
        let source = navigationAction.sourceFrame
        if url.fragment == COWrapper.serverEditorFragment() && isKeptFormAddress(url)
            && !source.isMainFrame && isKeptFormAddress(source.request.url)
        {
            decisionHandler(.allow)
            return
        }

        // Only the integrator's pages open an editor.
        let posterOrigin = Self.origin(of: navigationAction.sourceFrame.securityOrigin)
        if posterOrigin == nil || posterOrigin != integratorOrigin {
            COWrapper.LOG_ERR("An editor form was refused: it came from \(posterOrigin ?? "no origin")")
            decisionHandler(.cancel)
            return
        }

        // A server without client document loading shows its own editor, with all of its files:
        if let origin = Self.origin(of: url), serverEditorReasons[origin] != nil {
            decisionHandler(.allow)
        } else {
            blockCoolServerFiles(of: url) { decisionHandler(.allow) }
        }

        guard let form = request.httpBody else {
            COWrapper.LOG_DBG("No form was kept: the form posted into an editor frame has no body")
            return
        }

        editorForm = form
        editorFormUrl = url
        COWrapper.LOG_DBG("Kept the \(form.count)-byte form posted into an editor frame")
    }

    // CODA puts its own editor into the iframe, so coolwsd's scripts, style sheets, images and
    // fonts must not load there. WebKit checks these rules before it looks in its caches, so
    // cached copies are refused too. A worker's script counts as "other", not "script".
    // Fetches and WebSockets stay open, for /co/collab.
    private func blockCoolServerFiles(of url: URL, then proceed: @escaping () -> Void) {
        guard let origin = Self.coolServerOrigin(of: url) else {
            COWrapper.LOG_ERR("coolwsd's files were not blocked: the editor's address has no host")
            proceed()
            return
        }

        if blockedOrigins[origin] != nil {
            proceed()
            return
        }

        let trigger: [String: Any] = [
            "url-filter": "^" + NSRegularExpression.escapedPattern(for: origin + "/"),
            "resource-type": ["script", "style-sheet", "image", "font", "other"],
        ]

        let rules = [["trigger": trigger, "action": ["type": "block"]]]
        let json = String(data: try! JSONSerialization.data(withJSONObject: rules), encoding: .utf8)!

        // The navigation waits for the rules, so they are in place before the iframe loads.
        let store = WKContentRuleListStore.default()!
        let identifier = "CoolServerFiles " + origin
        store.compileContentRuleList(forIdentifier: identifier, encodedContentRuleList: json) { list, _ in
            defer { proceed() }
            guard let list else {
                COWrapper.LOG_ERR("coolwsd's files were not blocked: the content rules did not compile")
                return
            }

            self.webView.configuration.userContentController.add(list)
            self.blockedOrigins[origin] = list
        }
    }

    // The origin that the rules for coolwsd's files at the address are for:
    private static func coolServerOrigin(of url: URL) -> String? {
        guard let scheme = url.scheme, let host = url.host else {
            return nil
        }

        var origin = scheme + "://" + host
        if let port = url.port {
            origin += ":\(port)"
        }
        return origin
    }

    private func unblockCoolServerFiles(of url: URL) {
        guard let origin = Self.coolServerOrigin(of: url), let list = blockedOrigins[origin] else {
            return
        }

        webView.configuration.userContentController.remove(list)
        blockedOrigins[origin] = nil
    }
}
