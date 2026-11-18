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

final class Devices {
    /// Only continue running the test if the device we're running on is a phone
    static func skipUnlessIPhone() throws {
        try XCTSkipUnless(UIDevice.current.userInterfaceIdiom == .phone, "This test may only be run on an iPhone")
    }
    
    /// Only continue running the test if the device we're running on is an iPad
    static func skipUnlessIPad() throws {
        try XCTSkipUnless(UIDevice.current.userInterfaceIdiom == .pad, "This test may only be run on an iPad")
    }
}
