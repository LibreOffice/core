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

final class Launch {

    /**
     * Launch the app with a test document opened directly, skipping the backstage.
     *
     * The file is located in the test bundle's resources and its path is passed
     * to the app via the `--testFile` launch argument.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance.
     *   - filename: The basename of the file (e.g. "hello.odt").
     */
    static func openTestFile(app: XCUIApplication, filename: String) {
        let bundle = Bundle(for: Launch.self)
        let name = (filename as NSString).deletingPathExtension
        let ext = (filename as NSString).pathExtension

        guard let path = bundle.path(forResource: name, ofType: ext) else {
            XCTFail("Test file '\(filename)' not found in test bundle")
            return
        }

        app.launchArguments = ["--uitesting", "--testFile", path]
        app.launch()
    }

    /**
     * Wait until the document has finished loading.
     *
     * Watches the message log for a COMMANDSTATECHANGED message containing
     * `.uno:ModifiedStatus`, which arrives once the document is fully loaded.
     *
     * - Parameters:
     *   - app: The XCUIApplication instance (must already be launched).
     *   - timeout: Maximum seconds to wait (default 60).
     */
    static func waitForDocumentLoad(app: XCUIApplication, timeout: TimeInterval = 60) {
        let messageLog = app.textViews["CODA.TestMessageLog"]
        XCTAssertTrue(messageLog.waitForExistence(timeout: 10),
                      "Message log element not found")

        let predicate = NSPredicate(format: "value CONTAINS %@", ".uno:ModifiedStatus")
        let expectation = XCTNSPredicateExpectation(predicate: predicate,
                                                     object: messageLog)
        let result = XCTWaiter().wait(for: [expectation], timeout: timeout)
        XCTAssertEqual(result, .completed,
                       "Document did not finish loading within \(timeout)s")
    }
}
