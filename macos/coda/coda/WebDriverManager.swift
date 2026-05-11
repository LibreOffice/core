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
 * Tracks all WKWebViews in the app for the WebDriver server.
 *
 * Each WKWebView is assigned a stable handle when registered.  The
 * "active" handle is the one that `POST /session/{id}/execute/sync`
 * runs against; tests change it via `POST /session/{id}/window`.
 *
 * The first registered webview automatically becomes active.  When the
 * active webview is unregistered (e.g. the backstage closes after a
 * document opens), the most recently registered remaining webview
 * becomes active automatically.  This matches the Qt test pattern:
 * `getWindowHandles()` -> `switchToWindow(newHandle)`.
 */
final class WebDriverManager {
    static let shared = WebDriverManager()

    private var server: WebDriverServer?

    /// Insertion-ordered handles -> weak references to webviews.
    private var entries: [(handle: String, webView: Weak<WKWebView>)] = []

    /// Currently active handle for execute/sync.
    private var activeHandle: String?

    private struct Weak<T: AnyObject> {
        weak var value: T?
    }

    private init() {}

    /**
     * Start the embedded WebDriver server if `--testDriverPort=<port>`
     * is present in the launch arguments.  Idempotent.
     */
    func startIfRequested() {
        guard server == nil else { return }

        let args = ProcessInfo.processInfo.arguments
        let portString = args.lazy
            .compactMap { $0.hasPrefix("--testDriverPort=") ? String($0.dropFirst("--testDriverPort=".count)) : nil }
            .first
        guard let portString, let port = UInt16(portString) else { return }

        do {
            let s = try WebDriverServer(port: port,
                jsExecutor: { [weak self] js, completion in
                    self?.execute(js: js, completion: completion)
                },
                focusHandler: { [weak self] done in
                    self?.focusActiveWebView(done: done)
                },
                handlesProvider: { [weak self] in
                    self?.liveHandles() ?? []
                },
                switchHandler: { [weak self] handle in
                    self?.switchTo(handle: handle) ?? false
                }
            )
            s.start()
            server = s
        } catch {
            NSLog("WebDriverManager: failed to start server: %@", error.localizedDescription)
        }
    }

    /**
     * Register a WKWebView and return its handle.  The manager keeps a
     * weak reference, so the caller must call `unregister(handle:)`
     * when the view is going away (or just let it be cleaned up next
     * time `liveHandles()` runs).
     */
    func register(webView: WKWebView) -> String {
        let handle = UUID().uuidString
        entries.append((handle: handle, webView: Weak(value: webView)))
        if activeHandle == nil {
            activeHandle = handle
        } else {
            // A new webview replaces the active one (the typical
            // pattern is backstage -> document).
            activeHandle = handle
        }
        return handle
    }

    func unregister(handle: String) {
        entries.removeAll { $0.handle == handle }
        if activeHandle == handle {
            activeHandle = entries.last?.handle
        }
    }

    /// Drop entries whose WKWebView has been deallocated.
    private func compact() {
        entries.removeAll { $0.webView.value == nil }
        if let h = activeHandle, !entries.contains(where: { $0.handle == h }) {
            activeHandle = entries.last?.handle
        }
    }

    func liveHandles() -> [String] {
        compact()
        return entries.map { $0.handle }
    }

    func switchTo(handle: String) -> Bool {
        compact()
        guard entries.contains(where: { $0.handle == handle }) else {
            return false
        }
        activeHandle = handle
        return true
    }

    private func activeWebView() -> WKWebView? {
        compact()
        guard let h = activeHandle else { return nil }
        return entries.first(where: { $0.handle == h })?.webView.value
    }

    // MARK: - Operations on the active webview

    private func execute(js: String, completion: @escaping (Any?, Error?) -> Void) {
        DispatchQueue.main.async { [weak self] in
            guard let webView = self?.activeWebView() else {
                completion(nil, NSError(domain: "WebDriverManager", code: 1,
                                        userInfo: [NSLocalizedDescriptionKey: "no active webview"]))
                return
            }
            webView.evaluateJavaScript(js, completionHandler: completion)
        }
    }

    private func focusActiveWebView(done: @escaping () -> Void) {
        DispatchQueue.main.async { [weak self] in
            if let webView = self?.activeWebView() {
                webView.window?.makeKeyAndOrderFront(nil)
                NSApp.activate(ignoringOtherApps: true)
                webView.window?.makeFirstResponder(webView)
            }
            done()
        }
    }
}
