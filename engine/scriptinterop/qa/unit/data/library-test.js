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

// The script declares the TestLibrary library in library-test/, which declares the Inner library in
// turn, with the rules that GAS showed for the published apps-script-oauth2 library.  (Unlike the
// other tests here, it cannot be run against GAS, where a library has to be a published script
// project.)

function test() {
    console.assert(typeof TestLibrary === 'object');
    // A library exposes its top-level functions and variables, which see each other across files:
    console.assert(TestLibrary.greet('you') === 'Hello, you!');
    console.assert(TestLibrary.shout('you') === 'HELLO, YOU!');
    console.assert(TestLibrary.GREETING === 'Hello');
    console.assert(TestLibrary.secret() === 'secret');
    // Names that end in "_" stay private to the library:
    console.assert(TestLibrary.SECRET_ === undefined);
    console.assert(TestLibrary.punctuation_ === undefined);
    // Nothing of a library is visible in the script, and a library that it declares in turn is
    // only visible inside it:
    console.assert(typeof greet === 'undefined');
    console.assert(typeof GREETING === 'undefined');
    console.assert(typeof punctuation_ === 'undefined');
    console.assert(typeof Inner === 'undefined');
    console.assert(TestLibrary.innerValue() === 42);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
