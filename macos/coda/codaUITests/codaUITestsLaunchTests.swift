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

final class codaUITestsLaunchTests: XCTestCase {

    override class var runsForEachTargetApplicationUIConfiguration: Bool {
        false // FIXME: Currently test just one configuration, instead all the languages & light/dark theme combinations
    }

    override func setUpWithError() throws {
        continueAfterFailure = false
    }

    @MainActor
    func testBackstageAfterStartup() throws {
        let app = XCUIApplication()
        app.launchArguments = ["-ApplePersistenceIgnoreState", "YES"]
        app.launch()

        // Wait until the main window exists
        let window = app.windows["CODA.BackstageWindow"]
        XCTAssertTrue(window.waitForExistence(timeout: 5), "The backstage window did not appear")

        // Find the webview
        let webView = window.groups["CODA.BackstageWindow.WebView"]
        XCTAssertTrue(webView.waitForExistence(timeout: 5), "The webview did not appear")

        // Screenshot just the window
        let attachment = XCTAttachment(screenshot: window.screenshot())
        attachment.name = "Backstage Window"
        attachment.lifetime = .keepAlways
        add(attachment)
    }
}
