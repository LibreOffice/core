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

/**
 * Presents a WKWebView-based “New…” dialog and forwards the user’s choice back to the controller.
 */
final class BackstageViewController: NSViewController, WKScriptMessageHandlerWithReply {

    /** Callback to be invoked when the Backstage window should be closed. */
    var onClose: (() -> Void)?

    /** Embedded web view hosting the HTML UI for the 3-button chooser. */
    private var webView: WKWebView!

    /** Handle this webview registered under in WebDriverManager (for UI testing). */
    private var webDriverHandle: String?

    /** Creates an empty root view to host the web view. */
    override func loadView() {
        self.view = NSView()
    }

    /**
     * Builds the WKWebView, installs the “newDoc” JS bridge, and loads the HTML from the app bundle.
     */
    override func viewDidLoad() {
        super.viewDidLoad()

        // Setup jsHandler as the entry point to call back from JavaScript
        let contentController = WKUserContentController()
        contentController.addScriptMessageHandler(self, contentWorld: .page, name: "lok")

        let cfg = WKWebViewConfiguration()
        cfg.userContentController = contentController

        webView = WKWebView(frame: .zero, configuration: cfg)
        webView.translatesAutoresizingMaskIntoConstraints = false
        webView.setAccessibilityIdentifier("CODA.BackstageWindow.WebView")

#if DEBUG
        // Enable possibility to debug the webview from Safari
        webView.isInspectable = true
#endif

        view.addSubview(webView)
        NSLayoutConstraint.activate([
            webView.leadingAnchor.constraint(equalTo: view.leadingAnchor),
            webView.trailingAnchor.constraint(equalTo: view.trailingAnchor),
            webView.topAnchor.constraint(equalTo: view.topAnchor),
            webView.bottomAnchor.constraint(equalTo: view.bottomAnchor)
        ])

        // Load create-new-document.html from the bundle, allowing read access to its folder for relative assets.
        guard let url = Bundle.main.url(forResource: "cool", withExtension: "html") else {
            NSLog("BackstageViewController: cool.html not found in bundle")
            webView.loadHTMLString("<p>Missing <code>cool.html</code> in the bundle.</p>", baseURL: nil)
            return
        }

        var components = URLComponents(url: url, resolvingAgainstBaseURL: false)!

        // Make sure cool.html starts as the startup Backstage
        components.queryItems = [
            URLQueryItem(name: "starterMode", value: "true")
        ]

        // And also add common parameters, like "lang" or "darkTheme"
        Document.addCommonCOOLQueryItems(to: &components)

        // Allow access for additional resources
        let dir = url.deletingLastPathComponent()
        let finalURL = components.url!
        webView.loadFileURL(finalURL, allowingReadAccessTo: dir)

        webDriverHandle = WebDriverManager.shared.register(webView: webView)
    }

    /**
     * Receives “lok” messages from cool.html buttons and handles them.
     */
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) async -> (Any?, String?) {
        guard message.name == "lok", let body = message.body as? String else { return (nil, nil) }

        // Handle, and potentially close the window with the Backstage
        if let result = ViewController.handleBackstageMessage(body, onClose: { self.onClose?() }) {
            return result
        }

        return (nil, nil)
    }

    /**
     * Removes the message handler to avoid leaks or lingering callbacks after deallocation.
     */
    deinit {
        webView?.configuration.userContentController.removeScriptMessageHandler(forName: "lok")
        if let h = webDriverHandle {
            WebDriverManager.shared.unregister(handle: h)
        }
    }
}
