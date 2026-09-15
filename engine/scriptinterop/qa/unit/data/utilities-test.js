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

function utilitiesTest() {
    const b1 = Utilities.newBlob([1, 2, 3]);
    console.assert(b1.getContentType() === null);
    console.assert(b1.getName() === null);
    const b1b = b1.getBytes();
    console.assert(b1b.length === 3);
    console.assert(b1b[0] === 1 && b1b[1] === 2 && b1b[2] === 3);
    const b2 = Utilities.newBlob([1, 2, 3], 'application/octet-stream');
    console.assert(b2.getContentType() === 'application/octet-stream');
    console.assert(b2.getName() === null);
    const b2n = Utilities.newBlob([1, 2, 3], null);
    console.assert(b2n.getContentType() === null);
    console.assert(b2n.getName() === null);
    const b3 = Utilities.newBlob([1, 2, 3], 'application/octet-stream', 'data');
    console.assert(b3.getContentType() === 'application/octet-stream');
    console.assert(b3.getName() === 'data');
    const b4 = Utilities.newBlob('hello');
    const b4b = b4.getBytes();
    console.assert(b4b.length === 5);
    console.assert(
        b4b[0] === 0x68 && b4b[1] === 0x65 && b4b[2] === 0x6c && b4b[3] === 0x6c
            && b4b[4] === 0x6f);
    console.assert(b4.getContentType() === 'text/plain');
    console.assert(b4.getName() === null);
    const b5 = Utilities.newBlob('hello', 'text/plain');
    console.assert(b5.getContentType() === 'text/plain');
    console.assert(b5.getName() === null);
    const b6 = Utilities.newBlob('hello', 'text/plain', 'greet.txt');
    console.assert(b6.getContentType() === 'text/plain');
    console.assert(b6.getName() === 'greet.txt');
    const b6n = Utilities.newBlob('hello', null, 'greet.txt');
    console.assert(b6n.getContentType() === null);
    console.assert(b6n.getName() === 'greet.txt');
}
