/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import XCTest

final class JSBridge {

    /// Port for the embedded WebDriver server.
    static let port: UInt16 = 4567

    /// Base URL for the WebDriver server.
    private static let baseURL = "http://localhost:4567"

    /// Session ID, created lazily on first use.
    private static var sessionId: String = {
        guard let json = postJSON(path: "/session", body: [:]),
              let value = json["value"] as? [String: Any],
              let id = value["sessionId"] as? String else {
            XCTFail("JSBridge: failed to create WebDriver session")
            return "unknown"
        }
        return id
    }()

    /**
     * Execute JavaScript in the WKWebView via the W3C WebDriver protocol.
     *
     * Sends POST /session/{id}/execute/sync with the script.
     *
     * - Parameters:
     *   - js: The JavaScript to execute.
     * - Returns: The result from evaluateJavaScript, or nil.
     */
    @discardableResult
    static func execute(js: String) -> Any? {
        let body: [String: Any] = [
            "script": "return (\(js))",
            "args": [] as [Any]
        ]
        guard let json = postJSON(path: "/session/\(sessionId)/execute/sync", body: body) else {
            return nil
        }

        // W3C error response: {"value": {"error": "...", "message": "..."}}
        if let value = json["value"] as? [String: Any], value["error"] != nil {
            let msg = value["message"] as? String ?? "unknown error"
            XCTFail("JSBridge JS error: \(msg)")
            return nil
        }

        return json["value"]
    }

    /**
     * Execute JS and wait for a specific message in the log.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     *   - js: The JavaScript to execute.
     *   - expectMessage: Substring to wait for in the message log.
     *   - timeout: Maximum seconds for the message to appear.
     */
    static func executeAndWait(app: XCUIApplication, js: String,
                                expectMessage: String, timeout: TimeInterval = 30) {
        execute(js: js)
        MessageLog.waitForMessage(app: app, containing: expectMessage, timeout: timeout)
    }

    /**
     * Switch the document from Viewing mode to Editing mode.
     *
     * On CO Desktop, non-new documents start in read-only (Viewing) mode.
     * This calls _proceedEditMode() via the WebDriver bridge, which is the
     * same function the Viewing/Editing dropdown invokes.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     *   - timeout: Maximum seconds to wait for the mode switch.
     */
    static func switchToEditMode(app: XCUIApplication, timeout: TimeInterval = 30) {
        executeAndWait(
            app: app,
            js: "app.map._proceedEditMode()",
            expectMessage: ".uno:EditDoc",
            timeout: timeout
        )
    }

    /**
     * Make the WKWebView the first responder and bring the app to front.
     *
     * Call this before app.typeText() to ensure OS-level keystrokes
     * reach the WKWebView.
     */
    static func focusWebView() {
        _ = postJSON(path: "/focus", body: [:])
    }

    // MARK: - HTTP helpers

    /**
     * Send a POST request with a JSON body and return the parsed response.
     */
    private static func postJSON(path: String, body: [String: Any]) -> [String: Any]? {
        let url = URL(string: "\(baseURL)\(path)")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.httpBody = try? JSONSerialization.data(withJSONObject: body)
        request.timeoutInterval = 30

        var result: [String: Any]?
        let semaphore = DispatchSemaphore(value: 0)

        let task = URLSession.shared.dataTask(with: request) { data, _, error in
            defer { semaphore.signal() }
            if let error = error {
                XCTFail("JSBridge HTTP error: \(error.localizedDescription)")
                return
            }
            guard let data = data else { return }
            result = try? JSONSerialization.jsonObject(with: data) as? [String: Any]
        }
        task.resume()
        semaphore.wait()

        return result
    }

    /**
     * Send a GET request and return the parsed response.
     */
    static func getJSON(path: String) -> [String: Any]? {
        let url = URL(string: "\(baseURL)\(path)")!
        var request = URLRequest(url: url)
        request.httpMethod = "GET"
        request.timeoutInterval = 30

        var result: [String: Any]?
        let semaphore = DispatchSemaphore(value: 0)

        let task = URLSession.shared.dataTask(with: request) { data, _, error in
            defer { semaphore.signal() }
            if let error = error {
                XCTFail("JSBridge HTTP error: \(error.localizedDescription)")
                return
            }
            guard let data = data else { return }
            result = try? JSONSerialization.jsonObject(with: data) as? [String: Any]
        }
        task.resume()
        semaphore.wait()

        return result
    }
}
