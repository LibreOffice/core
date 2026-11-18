// -*- Mode: swift; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import XCTest

final class Lifecycle {
    /// Close an open document
    ///
    /// This assumes you have a document open and, if on iOS, that you are not in edit mode
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    static func closeDocument(app: XCUIApplication) {
        if (UIDevice.current.userInterfaceIdiom == .pad) {
            let closeButton = app.otherElements["navigation"].buttons.firstMatch
            XCTAssert(closeButton.exists, "Didn't find close button when trying to close file")
            closeButton.tap()
            
            XCTAssert(app.buttons["Search"].waitForExistence(timeout: 5), "Document browser did not appear after closing file")
        } else {
            // There's no close indicator on iPhone, so we have to blindly tap coordinates...
            let webview = app.webViews.containing(.other, identifier: "Online Editor").firstMatch;
            
            XCTAssert(webview.waitForExistence(timeout: 1), "Tried to close document without it being open")
            
            webview.coordinate(withNormalizedOffset: CGVector(dx: 0, dy: 0)).withOffset(CGVector(dx: 10, dy: 10)).tap()
            
            XCTAssert(app.buttons["Recents"].waitForExistence(timeout: 5), "Document browser did not appear after closing file")
        }
    }
}
