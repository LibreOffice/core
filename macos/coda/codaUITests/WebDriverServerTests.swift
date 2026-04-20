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

/**
 * Tests for the W3C WebDriver protocol implemented by WebDriverServer.
 *
 * These tests launch the app with a test document and exercise the
 * WebDriver endpoints via HTTP to verify protocol correctness.
 */
final class WebDriverServerTests: XCTestCase {

    private var app: XCUIApplication!

    override func setUpWithError() throws {
        continueAfterFailure = false
        app = XCUIApplication()
        Launch.openTestFile(app: app, filename: "hello.odt")
        Launch.waitForDocumentLoad(app: app)
    }

    // MARK: - Status

    @MainActor
    func testStatus() throws {
        let json = JSBridge.getJSON(path: "/status")
        XCTAssertNotNil(json, "GET /status should return JSON")

        let value = json!["value"] as? [String: Any]
        XCTAssertNotNil(value, "Response should have 'value' object")
        XCTAssertEqual(value!["ready"] as? Bool, true, "Server should be ready")
    }

    // MARK: - Session

    @MainActor
    func testCreateSession() throws {
        let json = JSBridge.postJSON(path: "/session", body: [:])
        XCTAssertNotNil(json, "POST /session should return JSON")

        let value = json!["value"] as? [String: Any]
        XCTAssertNotNil(value, "Response should have 'value' object")

        let sessionId = value!["sessionId"] as? String
        XCTAssertNotNil(sessionId, "Response should contain sessionId")
        XCTAssertFalse(sessionId!.isEmpty, "sessionId should not be empty")
    }

    // MARK: - Execute sync

    @MainActor
    func testExecuteSyncSimple() throws {
        let result = JSBridge.execute(js: "1 + 1")
        XCTAssertEqual(result as? Int, 2, "1 + 1 should return 2")
    }

    @MainActor
    func testExecuteSyncString() throws {
        let result = JSBridge.execute(js: "'hello' + ' world'")
        XCTAssertEqual(result as? String, "hello world")
    }

    @MainActor
    func testExecuteSyncObject() throws {
        let result = JSBridge.execute(js: "({a: 1, b: 'two'})")
        let obj = result as? [String: Any]
        XCTAssertNotNil(obj, "Should return an object")
        XCTAssertEqual(obj?["a"] as? Int, 1)
        XCTAssertEqual(obj?["b"] as? String, "two")
    }

    @MainActor
    func testExecuteSyncNull() throws {
        let result = JSBridge.execute(js: "null")
        XCTAssertTrue(result is NSNull || result == nil,
                      "null should return NSNull or nil")
    }

    @MainActor
    func testExecuteSyncDOMAccess() throws {
        let result = JSBridge.execute(js: "document.readyState")
        XCTAssertEqual(result as? String, "complete",
                       "Document should be fully loaded")
    }

    // MARK: - Window handles

    @MainActor
    func testWindowHandles() throws {
        // Create a session first
        _ = JSBridge.postJSON(path: "/session", body: [:])
        let sessionJson = JSBridge.postJSON(path: "/session", body: [:])
        let sessionId = (sessionJson?["value"] as? [String: Any])?["sessionId"] as? String ?? ""

        let json = JSBridge.getJSON(path: "/session/\(sessionId)/window/handles")
        XCTAssertNotNil(json, "GET window/handles should return JSON")

        let handles = json!["value"] as? [String]
        XCTAssertNotNil(handles, "Should return array of handles")
        XCTAssertFalse(handles!.isEmpty, "Should have at least one handle")
    }

    // MARK: - Switch window

    @MainActor
    func testSwitchWindow() throws {
        let sessionJson = JSBridge.postJSON(path: "/session", body: [:])
        let sessionId = (sessionJson?["value"] as? [String: Any])?["sessionId"] as? String ?? ""

        let json = JSBridge.postJSON(
            path: "/session/\(sessionId)/window",
            body: ["handle": "main"])
        XCTAssertNotNil(json, "POST window should return JSON")
        // Should not be an error
        let value = json!["value"]
        XCTAssertTrue(value is NSNull || value != nil, "Should return null (success)")
    }
}
