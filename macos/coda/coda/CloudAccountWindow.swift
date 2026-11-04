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
import WebKit

/// A window that shows the integrator's site for one cloud account, as a browser would.
final class CloudAccountWindow: NSWindowController, NSWindowDelegate, WKNavigationDelegate,
                                WKScriptMessageHandler, WKScriptMessageHandlerWithReply {
    private static let defaultSize = NSSize(width: 1200, height: 800)
    private static let world = WKContentWorld.world(name: "coda")
    private static let firstMacOSWithSafariVersion = 26
    private static let safariVersionsAheadOfMacOS = 3

    /// window.bridge calls that native code does not answer yet. They are not engine messages.
    private static let unansweredBridgeCalls: Set<Substring> = [
        // TODO: coolwsd should send the file info along with the "authenticated" message. The
        // native code need not query the integrator for it.
        "checkFileInfo",
        "getAllPrefs",
        "setPref"
    ]

    private static var openWindows: [String: CloudAccountWindow] = [:]

    private let accountId: String
    private let webView: WKWebView
    private var blockedOrigins: Set<String> = []
    private var editorForm: Data?
    private var editorFormUrl: URL?
    private var editorFrameInfo: WKFrameInfo?
    /// The local copy of the document open in the editor frame, edited by the engine.
    private var remoteDocument: Document?

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

        let window = NSWindow(contentRect: NSRect(origin: .zero, size: Self.defaultSize),
                              styleMask: [.titled, .closable, .resizable, .miniaturizable],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        window.title = url.host ?? url.absoluteString
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
#if DEBUG
        webView.isInspectable = true
#endif
        window.contentView = webView

        super.init(window: window)
        window.delegate = self
        webView.navigationDelegate = self

        let contentController = webView.configuration.userContentController
        contentController.add(self, contentWorld: Self.world, name: "frameLoaded")
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "lok")
        ViewController.addDiagnosticMessageHandlers(to: contentController, handler: self)
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

    func windowWillClose(_ notification: Notification) {
        let contentController = webView.configuration.userContentController
        contentController.removeScriptMessageHandler(forName: "frameLoaded", contentWorld: Self.world)
        contentController.removeScriptMessageHandler(forName: "lok")
        ViewController.removeDiagnosticMessageHandlers(from: contentController)
        closeRemoteDocument()
        Self.openWindows[accountId] = nil
    }

    // WebKit fills in frameInfo, so a page cannot pass itself off as another frame.
    private func isEditorFrame(_ frameInfo: WKFrameInfo) -> Bool {
        return !frameInfo.isMainFrame && frameInfo.request.url == editorFormUrl
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
        putCodaPageIn(frameInfo)
    }

    // comes from WKScriptMessageHandlerWithReply
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) async -> (Any?, String?) {
        if !isEditorFrame(message.frameInfo) {
            COWrapper.LOG_DBG("A message was dropped: it is not from the editor frame")
            return (nil, nil)
        }

        if ViewController.handleDiagnosticMessage(message) {
            return (nil, nil)
        }

        // Only "lok" messages reach this point.
        let body = message.body as? String ?? ""
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

        guard let remoteDocument else {
            COWrapper.LOG_DBG("\(kind) came before the document")
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
            || kind == "SAVEFAILED" || kind == "CLOSE_CANCELLED"
        if isSaveReport {
            return (nil, nil)
        }

        if let result = await ViewController.handleDocumentMessage(body, document: remoteDocument) {
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
        if !Self.isPlainExtension(fileExtension) {
            COWrapper.LOG_ERR("writeRemoteDocFile has a name with no plain extension: \"\(fileExtension)\"")
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

        // The document shows its Save As, print and error dialogs on this window.
        document.addWindowController(self)
        remoteDocument = document
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

    // Only ASCII letters and digits, like "odt" or "docx".
    private static func isPlainExtension(_ fileExtension: String) -> Bool {
        let isPlainCharacter = { (character: Character) in
            character.isASCII && (character.isLetter || character.isNumber)
        }

        return !fileExtension.isEmpty && fileExtension.allSatisfy(isPlainCharacter)
    }

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
        window.codaRemotePage = {};

        // coolwsd writes the values for its editor into these two elements.
        const variables = document.getElementById('initial-variables');
        const cssVariables = document.getElementById('init-css-vars');

        window.codaRemotePage.serverVariables = variables ? { ...variables.dataset } : {};
        window.codaRemotePage.cssVariables = cssVariables ? cssVariables.value : '';

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

    private func putCodaPageIn(_ frameInfo: WKFrameInfo) {
        guard let codaHtml = Self.appFile("cool.html") else {
            COWrapper.LOG_ERR("CODA's page was not put in: the app has no cool.html")
            return
        }

        webView.callAsyncJavaScript(
            Self.putCodaPageInScript,
            arguments: ["codaHtml": codaHtml],
            in: frameInfo,
            in: .page
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
            in: .page
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
            webView.evaluateJavaScript(program, in: frameInfo, in: .page) { result in
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
        webView.evaluateJavaScript(pageLoaded, in: frameInfo, in: .page)
    }

    // The integrator opens a document by posting a form with the token and settings into the
    // iframe that holds the editor. WebKit gives us the form's body here, and we keep a copy.
    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction,
                 decisionHandler: @escaping (WKNavigationActionPolicy) -> Void) {
        let request = navigationAction.request
        guard let url = request.url else {
            decisionHandler(.allow)
            return
        }

        let isFrame = navigationAction.targetFrame?.isMainFrame == false
        let isPost = request.httpMethod == "POST"
        let isEditorPage = url.path.hasSuffix("/cool.html")
        if !isFrame || !isPost || !isEditorPage {
            decisionHandler(.allow)
            return
        }

        blockCoolServerFiles(of: url) { decisionHandler(.allow) }

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
        guard let scheme = url.scheme, let host = url.host else {
            COWrapper.LOG_ERR("coolwsd's files were not blocked: the editor's address has no host")
            proceed()
            return
        }

        var origin = scheme + "://" + host
        if let port = url.port {
            origin += ":\(port)"
        }

        if blockedOrigins.contains(origin) {
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
            self.blockedOrigins.insert(origin)
        }
    }
}
