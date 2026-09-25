/* -*- fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// GAS doesn't have console.assert:
if (!globalThis.cool) {
    console.assert = console.assert || (cond => { if (!cond) throw new Error('failed: ' + cond); });
}

function test() {
    // The locale is a language code, followed by an underscore and a region code where there is
    // one, or empty in GAS when the script runs through the Apps Script API:
    const locale = Session.getActiveUserLocale();
    console.assert(/^([a-z]{2,3}(_[A-Z]{2})?)?$/.test(locale), locale);
    if (globalThis.cool) {
        // The test's kit has the language "sr-Latn-RS":
        console.assert(locale === 'sr_RS', locale);
    }
}
