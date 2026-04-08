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

final class Input {

    /**
     * Tap an element, waiting a short time for it to appear first.
     *
     * - Parameters:
     *   - element: The element to tap.
     *   - timeout: Maximum seconds to wait for the element to exist.
     */
    static func tapWithTimeout(element: XCUIElement, timeout: TimeInterval) {
        XCTAssert(element.waitForExistence(timeout: timeout),
                  "Did not find an element before its tap timeout expired")
        element.tap()
    }

    /**
     * Click the document webview to give it focus.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     */
    static func clickWebView(app: XCUIApplication) {
        let webView = app.webViews.firstMatch
        XCTAssertTrue(webView.waitForExistence(timeout: 10),
                      "Document webview not found")
        webView.click()
    }
}
