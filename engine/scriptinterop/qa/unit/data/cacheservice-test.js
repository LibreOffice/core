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

function throws(f) {
    try {
        f();
    } catch (e) {
        return e.message;
    }
    return null;
}

function test() {
    // The user cache keeps its entries from one script execution to the next, so each run uses keys
    // of its own:
    const p = 'test-' + Utilities.getUuid() + '-';
    const cache = CacheService.getUserCache();

    console.assert(cache.get(p + 'missing') === null);
    cache.put(p + 'a', '1');
    console.assert(cache.get(p + 'a') === '1');
    cache.put(p + 'e', '');
    console.assert(cache.get(p + 'e') === '');
    // A value is stored as a string, and a null value leaves no entry:
    cache.put(p + 'n', 42);
    console.assert(cache.get(p + 'n') === '42');
    cache.put(p + 'o', {x: 1});
    console.assert(cache.get(p + 'o') === '[object Object]');
    cache.put(p + 'null', null);
    console.assert(cache.get(p + 'null') === null);

    // Any number is an expiration that stores the entry:
    cache.put(p + 'ttl-default', 'x');
    cache.put(p + 'ttl-zero', 'x', 0);
    cache.put(p + 'ttl-negative', 'x', -5);
    cache.put(p + 'ttl-big', 'x', 99999);
    for (const k of ['ttl-default', 'ttl-zero', 'ttl-negative', 'ttl-big']) {
        console.assert(cache.get(p + k) === 'x');
    }
    console.assert(
        throws(() => cache.put(p + 'ttl-string', 'x', 'abc')) === "Cannot convert 'abc' to int.");
    console.assert(cache.get(p + 'ttl-string') === null);

    // Keys are limited to 250 characters, and values to 100 KB:
    console.assert(
        throws(() => cache.put('k'.repeat(251), 'x')) === 'Argument too large: key');
    console.assert(
        throws(() => cache.put(p + 'big', 'v'.repeat(100 * 1024 + 1)))
            === 'Argument too large: value');

    // getAll leaves out the keys that have no entry:
    const all = cache.getAll([p + 'a', p + 'missing', p + 'n']);
    console.assert(JSON.stringify(Object.keys(all).sort()) === JSON.stringify([p + 'a', p + 'n']));
    console.assert(all[p + 'a'] === '1' && all[p + 'n'] === '42');
    cache.putAll({[p + 'x']: '1', [p + 'y']: '2'});
    const put = cache.getAll([p + 'x', p + 'y']);
    console.assert(put[p + 'x'] === '1' && put[p + 'y'] === '2');
    cache.removeAll([p + 'x', p + 'y']);
    console.assert(JSON.stringify(cache.getAll([p + 'x', p + 'y'])) === '{}');
    cache.remove(p + 'a');
    console.assert(cache.get(p + 'a') === null);
    cache.remove(p + 'missing');

    // The script cache is separate from the user cache:
    CacheService.getScriptCache().put(p + 's', 'script');
    console.assert(cache.get(p + 's') === null);
    console.assert(CacheService.getScriptCache().get(p + 's') === 'script');
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
