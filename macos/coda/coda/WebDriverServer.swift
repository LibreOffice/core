/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Foundation
import Network

/**
 * W3C WebDriver protocol server for the WKWebView side.
 *
 * Implements the subset of the W3C WebDriver protocol sufficient for
 * WebDriverIO to execute JavaScript, manage sessions, and track window
 * handles.  This allows the same test specs to run on macOS (this
 * server), Linux (WebEngineDriver), and Windows (EdgeDriver).
 *
 * Additionally supports POST /focus as a custom extension to make
 * the WKWebView the macOS first responder for XCUITest typing.
 */
final class WebDriverServer: WebDriverHTTPServerBase {

    private let jsExecutor: (String, @escaping (Any?, Error?) -> Void) -> Void
    private let focusHandler: (@escaping () -> Void) -> Void
    private let handlesProvider: () -> [String]
    private let switchHandler: (String) -> Bool

    /**
     * Create a WebDriver server.
     *
     * - Parameters:
     *   - port: TCP port to listen on.
     *   - jsExecutor: Closure that evaluates JavaScript on the active webview.
     *   - focusHandler: Closure that makes the active webview the first responder.
     *   - handlesProvider: Returns the current list of window handles.
     *   - switchHandler: Set the active webview by handle; returns true on success.
     */
    init(port: UInt16,
         jsExecutor: @escaping (String, @escaping (Any?, Error?) -> Void) -> Void,
         focusHandler: @escaping (@escaping () -> Void) -> Void,
         handlesProvider: @escaping () -> [String],
         switchHandler: @escaping (String) -> Bool) throws {
        self.jsExecutor = jsExecutor
        self.focusHandler = focusHandler
        self.handlesProvider = handlesProvider
        self.switchHandler = switchHandler
        try super.init(port: port, label: "WebDriverServer")
    }

    override func routeRequest(_ request: HTTPRequest, connection: NWConnection) {
        let segments = request.path.split(separator: "/").map(String.init)

        // GET /status
        if request.method == "GET" && request.path == "/status" {
            sendW3C(connection: connection, value: ["ready": true, "message": "coda-macos"])
            return
        }

        // POST /session
        if request.method == "POST" && segments == ["session"] {
            let newId = UUID().uuidString.lowercased()
            sessionIds.insert(newId)
            sendW3C(connection: connection, value: [
                "sessionId": newId,
                "capabilities": [String: Any]()
            ] as [String: Any])
            return
        }

        // DELETE /session/{id}
        if request.method == "DELETE" && segments.count == 2 && segments[0] == "session" {
            sessionIds.remove(segments[1])
            sendW3C(connection: connection, value: NSNull())
            return
        }

        // Routes that require a valid session: /session/{id}/...
        if segments.count >= 3 && segments[0] == "session" {
            guard sessionIds.contains(segments[1]) else {
                sendW3CError(connection: connection, error: "invalid session id",
                             message: "No active session with id '\(segments[1])'")
                return
            }

            let subpath = Array(segments.dropFirst(2))

            // POST /session/{id}/execute/sync
            // POST /session/{id}/execute (WebDriverIO may use either)
            if request.method == "POST" && (subpath == ["execute", "sync"] || subpath == ["execute"]) {
                handleExecuteSync(request, connection: connection)
                return
            }

            // GET /session/{id}/source
            if request.method == "GET" && subpath == ["source"] {
                jsExecutor("document.documentElement.outerHTML") { [weak self] result, _ in
                    if let html = result as? String {
                        self?.sendW3C(connection: connection, value: html)
                    } else {
                        self?.sendW3C(connection: connection, value: "")
                    }
                }
                return
            }

            // GET /session/{id}/window/handles  -- W3C
            // GET /session/{id}/window_handles  -- JSON Wire (WebDriverIO 8 still uses this)
            if request.method == "GET"
                && (subpath == ["window", "handles"] || subpath == ["window_handles"]) {
                sendW3C(connection: connection, value: handlesProvider())
                return
            }

            // GET /session/{id}/window  -- current window handle
            if request.method == "GET" && subpath == ["window"] {
                sendW3C(connection: connection, value: handlesProvider().last ?? "")
                return
            }

            // POST /session/{id}/window  -- switch to window
            //   W3C body:        {"handle": "<id>"}
            //   JSON Wire body:  {"name":   "<id>"}
            if request.method == "POST" && subpath == ["window"] {
                let json = (try? JSONSerialization.jsonObject(with: request.body) as? [String: Any]) ?? [:]
                guard let handle = (json["handle"] as? String) ?? (json["name"] as? String) else {
                    sendW3CError(connection: connection, error: "invalid argument",
                                 message: "Missing 'handle' or 'name' in body")
                    return
                }
                if switchHandler(handle) {
                    sendW3C(connection: connection, value: NSNull())
                } else {
                    sendW3CError(connection: connection, error: "no such window",
                                 message: "Window handle '\(handle)' not found")
                }
                return
            }
        }

        // POST /focus (custom extension for XCUITest)
        if request.method == "POST" && request.path == "/focus" {
            focusHandler { [weak self] in
                self?.sendW3C(connection: connection, value: NSNull())
            }
            return
        }

        sendW3CError(connection: connection, error: "unknown command",
                     message: "\(request.method) \(request.path) not implemented")
    }

    // MARK: - Execute sync

    private func handleExecuteSync(_ request: HTTPRequest, connection: NWConnection) {
        guard let json = try? JSONSerialization.jsonObject(with: request.body) as? [String: Any],
              let script = json["script"] as? String else {
            sendW3CError(connection: connection, error: "invalid argument",
                         message: "Missing 'script' field in request body")
            return
        }

        let args = json["args"] as? [Any] ?? []

        // WKWebView.evaluateJavaScript expects an expression, not a
        // program with "return".  WebDriverIO sends scripts like
        // "return (function(){...}).apply(null,arguments)" so we need
        // to strip the leading "return " and let evaluateJavaScript
        // evaluate the expression directly.
        var expression: String
        if script.hasPrefix("return ") {
            expression = String(script.dropFirst("return ".count))
        } else {
            expression = script
        }

        // WebDriverIO uses "arguments" to reference the args array.
        // Since evaluateJavaScript runs at the top level where
        // "arguments" is not defined, replace it with the actual
        // serialized args array.
        if let argsData = try? JSONSerialization.data(withJSONObject: args),
           let argsJSON = String(data: argsData, encoding: .utf8) {
            expression = expression.replacingOccurrences(of: "arguments", with: argsJSON)
        }

        jsExecutor(expression) { [weak self] result, error in
            if let error = error {
                // Extract the actual JS exception message from the
                // NSError userInfo, not the generic localizedDescription.
                let nsError = error as NSError
                let jsMessage = nsError.userInfo["WKJavaScriptExceptionMessage"] as? String
                    ?? nsError.localizedDescription
                self?.sendW3CError(connection: connection, error: "javascript error",
                                   message: jsMessage)
            } else {
                self?.sendW3C(connection: connection, value: result ?? NSNull())
            }
        }
    }
}
