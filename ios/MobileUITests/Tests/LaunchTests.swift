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

final class LaunchTests: XCTestCase {
    override func setUpWithError() throws {
        continueAfterFailure = false
        XCUIDevice.shared.orientation = UIDeviceOrientation.landscapeLeft
    }
    
    @MainActor
    func testLaunchPerformance() throws {
        let app = XCUIApplication()

        measure(metrics: [XCTApplicationLaunchMetric()]) {
            Launch.fileBrowser(app: app)
            app.terminate()
        }
    }

    @MainActor
    func testOpenOdt() throws {
        let app = XCUIApplication()
        Launch.precopyTestFile(app: app, filename: "hello.odt")

        Launch.testFile(app: app, filename: "hello.odt")
            
        let attachment = XCTAttachment(screenshot: XCUIScreen.main.screenshot())
        attachment.name = "Document"
        attachment.lifetime = .keepAlways
        add(attachment)
        
        app.terminate()
    }
    
    @MainActor
    func testWriterOpenPerformance() throws {
        let app = XCUIApplication()
        Launch.precopyTestFile(app: app, filename: "hello.odt")
        
        measure(metrics: [XCTClockMetric()]) {
            Launch.testFile(app: app, filename: "hello.odt")
            app.terminate()
        }
    }
    
    @MainActor
    func testCalcOpenPerformance() throws {
        let app = XCUIApplication()
        Launch.precopyTestFile(app: app, filename: "hello.ods")
        
        measure(metrics: [XCTClockMetric()]) {
            Launch.testFile(app: app, filename: "hello.ods")
            app.terminate()
        }
    }
    
    @MainActor
    func testImpressOpenPerformance() throws {
        let app = XCUIApplication()
        Launch.precopyTestFile(app: app, filename: "hello.odp")
        
        measure(metrics: [XCTClockMetric()]) {
            Launch.testFile(app: app, filename: "hello.odp")
            app.terminate()
        }
    }
    
    @MainActor
    func testWriterReopenPerformance() throws {
        let app = XCUIApplication()
        Launch.precopyTestFile(app: app, filename: "hello.odt")
        
        let measureOptions = XCTMeasureOptions()
        measureOptions.invocationOptions = [.manuallyStart, .manuallyStop]
        
        measure(
            metrics: [XCTClockMetric()],
            options: measureOptions
        ) {
            Launch.testFile(app: app, filename: "hello.odt")
            Lifecycle.closeDocument(app: app)
            
            startMeasuring()
            
            Launch.testFile(app: app, filename: "hello.odt")

            let webview = app.webViews.containing(.other, identifier: "Online Editor").firstMatch;
            XCTAssert(webview.waitForExistence(timeout: 10), "App did not open file in time")
            let loading = webview.staticTexts["Loading…"]
            XCTAssert(loading.waitForNonExistence(timeout: 120), "App did not finish loading in time")
            
            stopMeasuring()
            
            app.terminate()
        }
    }
}
