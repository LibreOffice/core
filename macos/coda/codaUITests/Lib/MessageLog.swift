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

final class MessageLog {

    /**
     * Wait until the message log contains a specific substring.
     *
     * The message log is an offscreen text view that accumulates every
     * message the JS sends to native via the "lok" handler.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     *   - text: The substring to search for.
     *   - timeout: Maximum seconds to wait (default 30).
     */
    static func waitForMessage(app: XCUIApplication, containing text: String,
                                timeout: TimeInterval = 30) {
        let messageLog = app.textViews["CODA.TestMessageLog"]
        XCTAssertTrue(messageLog.waitForExistence(timeout: 10),
                      "Message log element not found")

        let predicate = NSPredicate(format: "value CONTAINS %@", text)
        let expectation = XCTNSPredicateExpectation(predicate: predicate,
                                                     object: messageLog)
        let result = XCTWaiter().wait(for: [expectation], timeout: timeout)
        XCTAssertEqual(result, .completed,
                       "Message containing '\(text)' not found within \(timeout)s")
    }

    /**
     * Check whether the message log currently contains a specific substring.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     *   - text: The substring to search for.
     * - Returns: `true` if the substring is present.
     */
    static func containsMessage(app: XCUIApplication, containing text: String) -> Bool {
        let messageLog = app.textViews["CODA.TestMessageLog"]
        guard messageLog.exists else { return false }
        guard let value = messageLog.value as? String else { return false }
        return value.contains(text)
    }
}
