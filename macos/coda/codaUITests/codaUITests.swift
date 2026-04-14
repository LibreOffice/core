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

final class codaUITests: XCTestCase {

    override func setUpWithError() throws {
        continueAfterFailure = false
    }

    @MainActor
    func testDocumentLoad() throws {
        let app = XCUIApplication()
        Launch.openTestFile(app: app, filename: "hello.odt")
        Launch.waitForDocumentLoad(app: app)

        let attachment = XCTAttachment(screenshot: app.screenshot())
        attachment.name = "Document Loaded"
        attachment.lifetime = .keepAlways
        add(attachment)

        XCTAssertTrue(app.windows.count > 0, "Expected a document window")
    }

    @MainActor
    func testTypingModifiesDocument() throws {
        let app = XCUIApplication()
        Launch.openTestFile(app: app, filename: "hello.odt")
        Launch.waitForDocumentLoad(app: app)

        JSBridge.switchToEditMode(app: app)
        JSBridge.focusWebView()
        app.typeText("some text")

        // Wait for the document to become modified
        MessageLog.waitForMessage(app: app,
            containing: "ModifiedStatus\",\"state\":\"true\"",
            timeout: 30)
    }

    @MainActor
    func testOpenCalcDocument() throws {
        let app = XCUIApplication()
        Launch.openTestFile(app: app, filename: "hello.ods")
        Launch.waitForDocumentLoad(app: app)

        let attachment = XCTAttachment(screenshot: app.screenshot())
        attachment.name = "Calc Document Loaded"
        attachment.lifetime = .keepAlways
        add(attachment)
    }
}
