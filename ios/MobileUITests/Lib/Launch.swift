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

import Foundation
import XCTest
import SafariServices

final class Launch {   
    /// Copy a test file to the correct places for `Launch.testFile` to find it
    ///
    /// To copy, this will launch the app with some specially-crafted parameters, and both terminates the app and resets parameters after running. Therefore, it's advisable to run this early on as it'll destroy your test state when it runs
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    ///   - filename: The basename of a file
    static func precopyTestFile(app: XCUIApplication, filename: String) {
        app.launchArguments = ["-copyTestFile", filename]
        app.launch()
       
        app.launchArguments = [String]()
        app.terminate()
    }

    /// Open a specified test file
    ///
    /// Open the `"hello.odt"` file, launching the app if it's not open, or reusing an existing session (which must be on the document browser) if there is one:
    ///
    /// ```swift
    /// Launch.testFile(app: app, filename: "hello.odt")
    /// ```
    ///
    /// It's **required** to precopy your test file with `Launch.precopyTestFile`. Omitting this will make your test fail on clean devices
    ///
    /// ```swift
    /// Launch.precopyTestFile(filename: "hello.odt")
    /// ```
    ///
    /// - Parameters:
    ///   - app: The XCUI app object
    ///   - filename: The basename of a file
    static func testFile(app: XCUIApplication, filename: String) {
        app.activate()
        
        let mainAppName = app.label

        if (UIDevice.current.userInterfaceIdiom == .phone) {
            let browseButton = app.buttons
                .matching(NSPredicate(format: #"label == "Browse""#))
                .matching(NSPredicate(format: #"identifier != "BackButton""#))
                .element
            
            Input.tapWithTimeout(element: browseButton, timeout: 0.5)
            Input.tapWithTimeout(element: browseButton, timeout: 0.5) // We tap it twice to end back on the home page, otherwise we might be in some folder...
                            
            app.cells["DOC.sidebar.item.On My iPhone"].tap()
        } else {
            try! DocumentBrowser.ensureSidebarShown(app: app)
            app.cells["DOC.sidebar.item.On My iPad"].tap()
        }

        Input.tapWithTimeout(element: app.cells["\(mainAppName), Container"], timeout: 0.5)
        Input.tapWithTimeout(element: app.cells["TestFiles, Folder"], timeout: 0.5)
        
        let filenameParts = filename.split(separator: ".")
        let name = filenameParts.dropLast().joined(separator: ".")
        let ext = filenameParts.last!
        let testFile = app.cells.matching(NSPredicate(format: "identifier == \"\(name).\(ext), \(ext)\" || identifier == \"\(name), \(ext)\"")).element // iOS has two ways of displaying file names (with extensions or without) - so let's check for either ... this definitely breaks on some weird cases of, say, hello.odt.odt, but it's "unlikely" that we actually call a test file that
        Input.tapWithTimeout(element: testFile, timeout: 0.5)

        let webview = app.webViews.containing(.other, identifier: "Online Editor").firstMatch;
        XCTAssert(webview.waitForExistence(timeout: 30), "App did not open editor in time")
        let loading = webview.staticTexts["Loading…"]
        XCTAssert(loading.waitForNonExistence(timeout: 30), "App did not finish loading in time")
    }

    /// Launch the app to the filebrowser screen
    ///
    /// If the app is already open, this method will relaunch it to the filebrowser screen
    ///
    /// - Parameters:
    ///     - app: The XCUI app object
    static func fileBrowser(app: XCUIApplication) {
        app.launch()
    }
}
