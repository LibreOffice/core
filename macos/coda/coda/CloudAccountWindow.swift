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
                                WKScriptMessageHandler {
    private static let defaultSize = NSSize(width: 1200, height: 800)
    private static let world = WKContentWorld.world(name: "coda")
    private static let firstMacOSWithSafariVersion = 26
    private static let safariVersionsAheadOfMacOS = 3

    private static var openWindows: [String: CloudAccountWindow] = [:]

    private let accountId: String
    private let webView: WKWebView
    private var blockedOrigins: Set<String> = []
    private var editorForm: Data?
    private var editorFormUrl: URL?
    private var editorFrameInfo: WKFrameInfo?

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
        webView.configuration.userContentController.add(
            self,
            contentWorld: Self.world,
            name: "frameLoaded"
        )

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
        webView.configuration.userContentController.removeScriptMessageHandler(
            forName: "frameLoaded",
            contentWorld: Self.world
        )

        Self.openWindows[accountId] = nil
    }

    // WebKit fills in frameInfo, so a page cannot pass itself off as another frame.
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let frameInfo = message.frameInfo
        let isEditorFrame = !frameInfo.isMainFrame && frameInfo.request.url == editorFormUrl

        if !isEditorFrame {
            COWrapper.LOG_TRC("A frame other than the editor frame loaded")
            return
        }

        editorFrameInfo = frameInfo
        COWrapper.LOG_DBG("The editor frame loaded")
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
