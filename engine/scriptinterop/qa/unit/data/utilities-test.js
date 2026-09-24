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

    // A string is encoded as US-ASCII, with "?" for each code point outside it, unless UTF-8 is
    // asked for:
    console.assert(Utilities.base64Encode('hi') === 'aGk=');
    console.assert(Utilities.base64Encode('') === '');
    console.assert(Utilities.base64Encode('aéb') === 'YT9i');
    console.assert(Utilities.base64Encode('€') === 'Pw==');
    console.assert(Utilities.base64Encode('😀') === 'Pw==');
    console.assert(Utilities.base64Encode('é', Utilities.Charset.US_ASCII) === 'Pw==');
    console.assert(Utilities.base64Encode('é', Utilities.Charset.UTF_8) === 'w6k=');
    console.assert(Utilities.base64Encode('€', Utilities.Charset.UTF_8) === '4oKs');
    console.assert(Utilities.base64Encode('😀', Utilities.Charset.UTF_8) === '8J+YgA==');
    // Bytes are encoded as they are, whether given signed or unsigned:
    console.assert(Utilities.base64Encode([0x68, 0x69]) === 'aGk=');
    console.assert(Utilities.base64Encode([-1, 0, 127, -128]) === '/wB/gA==');
    console.assert(Utilities.base64Encode([0xff, 0, 0x7f, 0x80]) === '/wB/gA==');

    // Decoding gives signed bytes, ignoring white space and any missing or extra padding:
    const decoded = (...args) => JSON.stringify(Utilities.base64Decode(...args));
    console.assert(decoded('aGk=') === '[104,105]');
    console.assert(decoded('/wB/gA==') === '[-1,0,127,-128]');
    console.assert(decoded('w6k=') === '[-61,-87]');
    console.assert(decoded('w6k=', Utilities.Charset.UTF_8) === '[-61,-87]');
    console.assert(decoded('') === '[]');
    console.assert(decoded('aGk') === '[104,105]');
    console.assert(decoded('aGk==') === '[104,105]');
    console.assert(decoded('aG k=') === '[104,105]');
    console.assert(decoded('aG\nk=') === '[104,105]');
    // Anything else is not base64, including the web-safe alphabet:
    for (const text of ['_wB_gA==', '!!!!', 'a']) {
        let message = null;
        try {
            Utilities.base64Decode(text);
        } catch (e) {
            message = e.message;
        }
        console.assert(message === 'Could not decode string.');
    }

    // A charset argument that is given has to be a charset:
    for (const f of [
        () => Utilities.base64Encode('hi', null), () => Utilities.base64Encode('hi', undefined),
        () => Utilities.base64Decode('aGk=', null), () => Utilities.base64Decode('aGk=', undefined)])
    {
        let message = null;
        try {
            f();
        } catch (e) {
            message = e.message;
        }
        console.assert(message === 'Argument cannot be null: charset');
    }

    // A UUID is a random (version 4) one, new on each call:
    const uuid = Utilities.getUuid();
    console.assert(
        /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/.test(uuid));
    console.assert(Utilities.getUuid() !== uuid);
}
