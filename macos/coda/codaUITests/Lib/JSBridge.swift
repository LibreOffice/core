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

    /// Port for the embedded test HTTP server.
    static let port: UInt16 = 4567

    /**
     * Execute JavaScript in the WKWebView via the embedded HTTP test driver.
     *
     * Sends a POST /execute request with the JS command and returns the
     * parsed JSON result, or nil on error.
     *
     * - Parameters:
     *   - js: The JavaScript to execute.
     * - Returns: The result from evaluateJavaScript, or nil.
     */
    @discardableResult
    static func execute(js: String) -> Any? {
        let url = URL(string: "http://localhost:\(port)/execute")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.httpBody = try? JSONSerialization.data(withJSONObject: ["js": js])
        request.timeoutInterval = 30

        var result: Any?
        let semaphore = DispatchSemaphore(value: 0)

        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            defer { semaphore.signal() }
            if let error = error {
                XCTFail("JSBridge HTTP error: \(error.localizedDescription)")
                return
            }
            guard let data = data,
                  let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] else {
                return
            }
            if let errorMsg = json["error"] as? String {
                XCTFail("JSBridge JS error: \(errorMsg)")
                return
            }
            result = json["result"]
        }
        task.resume()
        semaphore.wait()

        return result
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
     * This calls _proceedEditMode() via the HTTP bridge, which is the same
     * function the Viewing/Editing dropdown invokes.
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
        let url = URL(string: "http://localhost:\(port)/focus")!
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.timeoutInterval = 10

        let semaphore = DispatchSemaphore(value: 0)
        let task = URLSession.shared.dataTask(with: request) { _, _, _ in
            semaphore.signal()
        }
        task.resume()
        semaphore.wait()
    }
}
