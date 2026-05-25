/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import Foundation

/**
 * Tiny XPath parser for the selector subset the shared specs use:
 *
 *   //*[@accessibility-id="ID"]
 *   //*[@title="Title"]
 *   //*[@role="AXButton"]
 *   //*[@default-button="true"]
 *
 * [Now "close" XPaths misinterpreted as comments: */*/*/*/]
 */
enum SimpleXPath {
    struct Predicate {
        let attribute: String
        let value: String
    }

    static func parse(_ xpath: String) -> Predicate? {
        // Match: //*[@<attr>="<value>"]
        let pattern = #"^//\*\[@([\w-]+)\s*=\s*"([^"]*)"\]$"#
        guard let regex = try? NSRegularExpression(pattern: pattern),
              let m = regex.firstMatch(in: xpath, range: NSRange(xpath.startIndex..., in: xpath)),
              m.numberOfRanges == 3,
              let attrR = Range(m.range(at: 1), in: xpath),
              let valR = Range(m.range(at: 2), in: xpath) else {
            return nil
        }
        return Predicate(attribute: String(xpath[attrR]), value: String(xpath[valR]))
    }
}
