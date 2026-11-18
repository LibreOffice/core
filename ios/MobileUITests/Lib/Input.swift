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

final class Input {
    /// Tap an element, waiting a little bit if it isn't already present
    ///
    /// This can be particularly useful if you have animations/etc. that might obscure an element for a short amount of time
    ///
    /// If the element doesn't appear within the timeout, the test will be failed - the same as if you tried to tap on an element that didn't exist without using this method
    ///
    /// - Parameters:
    ///   - element: The element you want to tap
    //    - timeout: The maximum amount of time to wait before failing the test
    static func tapWithTimeout(element: XCUIElement, timeout: TimeInterval) {
        XCTAssert(element.waitForExistence(timeout: timeout), "Did not find an element before its tap timeout expired")
        element.tap()
    }
}
