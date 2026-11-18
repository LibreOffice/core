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

final class DocumentBrowser {
    /// Detect if the sidebar is open in the document browser view.
    ///
    /// This assumes that you are in the document browser, and will always return false if you are not
    ///
    /// There is no sidebar on iPhones, so in that case this will always return false
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    ///
    /// - Returns: whether the document browser's sidebar is currently open
    static func getSidebarShown(app: XCUIApplication) -> Bool {
        if (UIDevice.current.userInterfaceIdiom == .phone) {
            return false
        }

        return app.cells["DOC.sidebar.item.On My iPad"].exists
    }

    /// Hide the sidebar if it's not already hidden. If it is already hidden, do nothing
    ///
    /// On phones, this method is always a no-op
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    static func ensureSidebarHidden(app: XCUIApplication) {
        if (UIDevice.current.userInterfaceIdiom == .phone) {
            return // The sidebar doesn't exist on phones, so it's always hidden...
        }
        
        XCTAssert(app.buttons["Search"].waitForExistence(timeout: 5), "Document browser did not appear while ensuring sidebar state")

        if (!getSidebarShown(app: app)) {
            return
        }
        app.buttons["ToggleSidebar"].tap() // Yes, one of these has a capital B and the other doesn't
    }
    
    /// Show the sidebar if it's not already shown. If it is already shown, do nothing
    ///
    /// On phones, there is no sidebar so - as running this is an indication that you have a test that is only made for iPads - this method will skip your test
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    static func ensureSidebarShown(app: XCUIApplication) throws {
        try Devices.skipUnlessIPad()
        
        XCTAssert(app.buttons["Search"].waitForExistence(timeout: 5), "Document browser did not appear while ensuring sidebar state")

        if (getSidebarShown(app: app)) {
            return
        }
        app.buttons["ToggleSideBar"].tap() // Yes, one of these has a capital B and the other doesn't
    }
}
