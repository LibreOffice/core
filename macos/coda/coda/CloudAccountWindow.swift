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
final class CloudAccountWindow: NSWindowController, NSWindowDelegate, WKNavigationDelegate {
    private static let defaultSize = NSSize(width: 1200, height: 800)
    private static let firstMacOSWithSafariVersion = 26
    private static let safariVersionsAheadOfMacOS = 3

    private static var openWindows: [String: CloudAccountWindow] = [:]

    private let accountId: String
    private var editorForm: Data?
    private var editorFormUrl: URL?

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

        let webView = WKWebView(frame: .zero, configuration: configuration)
#if DEBUG
        webView.isInspectable = true
#endif
        window.contentView = webView

        super.init(window: window)
        window.delegate = self
        webView.navigationDelegate = self

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
        Self.openWindows[accountId] = nil
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

        guard let form = request.httpBody else {
            COWrapper.LOG_DBG("No form was kept: the form posted into an editor frame has no body")
            decisionHandler(.allow)
            return
        }

        editorForm = form
        editorFormUrl = url
        COWrapper.LOG_DBG("Kept the \(form.count)-byte form posted into an editor frame")
        decisionHandler(.allow)
    }
}
