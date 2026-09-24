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
    // An AuthMode value prints as its name and has the methods of a Java enum:
    const modes = ['NONE', 'CUSTOM_FUNCTION', 'LIMITED', 'FULL'];
    modes.forEach(function(name, ordinal) {
        const mode = ScriptApp.AuthMode[name];
        console.assert(typeof mode === 'object');
        console.assert(String(mode) === name);
        console.assert(`${mode}` === name);
        console.assert(JSON.stringify(mode) === '"' + name + '"');
        console.assert(mode.name() === name);
        console.assert(mode.ordinal() === ordinal);
        console.assert(mode === ScriptApp.AuthMode[name]);
    });
    console.assert(ScriptApp.AuthMode.FULL !== ScriptApp.AuthMode.LIMITED);
    console.assert(ScriptApp.AuthMode.FULL.compareTo(ScriptApp.AuthMode.NONE) > 0);
    console.assert(ScriptApp.AuthMode.NONE.compareTo(ScriptApp.AuthMode.FULL) < 0);
    console.assert(ScriptApp.AuthMode.FULL.compareTo(ScriptApp.AuthMode.FULL) === 0);
    console.assert(ScriptApp.AuthMode.CUSTOM === undefined);

    const id = ScriptApp.getScriptId();
    console.assert(typeof id === 'string' && id.length > 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
