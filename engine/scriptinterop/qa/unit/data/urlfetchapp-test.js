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

// The requests go to https://httpbin.org (or, in CppunitTest_scriptinterop_gas, to the httpbin
// hook in qa/unit/gas.cxx, which answers the same way), whose /anything echoes the request back
// as JSON.

// Header names come in the case the server sent them in GAS, but in lower case here:
function header(response, name) {
    const headers = response.getHeaders();
    for (const k of Object.keys(headers)) {
        if (k.toLowerCase() === name) {
            return headers[k];
        }
    }
    return undefined;
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
    const url = 'https://httpbin.org/anything';

    const get = UrlFetchApp.fetch(url, {headers: {'X-Test': 'a'}});
    console.assert(get.getResponseCode() === 200);
    console.assert(header(get, 'content-type').startsWith('application/json'));
    console.assert(get.getContent().length === get.getContentText().length);
    console.assert(get.getBlob().getContentType() === 'application/json');
    const getEcho = JSON.parse(get.getContentText());
    console.assert(getEcho.method === 'GET');

    // The bytes of a body come back signed, here those that every PNG starts with:
    const png = UrlFetchApp.fetch('https://httpbin.org/image/png');
    console.assert(JSON.stringify(png.getContent().slice(0, 8)) === '[-119,80,78,71,13,10,26,10]');
    console.assert(getEcho.headers['X-Test'] === 'a');

    // An object payload is form-encoded:
    const form = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: {a: '1', b: 'x y'}}).getContentText());
    console.assert(form.method === 'POST');
    console.assert(form.headers['Content-Type'] === 'application/x-www-form-urlencoded');
    console.assert(form.form.a === '1');
    console.assert(form.form.b === 'x y');

    // A string payload is sent as it is, by default as application/x-www-form-urlencoded:
    const string = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: 'hello', contentType: 'text/plain'}).getContentText());
    console.assert(string.data === 'hello');
    console.assert(string.headers['Content-Type'] === 'text/plain');
    const defaultType = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: 'hello'}).getContentText());
    console.assert(defaultType.headers['Content-Type'] === 'application/x-www-form-urlencoded');

    // A byte payload is sent as those bytes, by default as application/x-www-form-urlencoded too:
    const bytes = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: [0xff, 0, 0x80, -1], contentType: 'application/octet-stream'})
        .getContentText());
    console.assert(bytes.data === 'data:application/octet-stream;base64,/wCA/w==');
    const defaultBytes = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: [0xff, 0, 0x80, -1]}).getContentText());
    console.assert(defaultBytes.headers['Content-Type'] === 'application/x-www-form-urlencoded');

    // A Blob payload is sent as its bytes, with its own content type:
    const blob = JSON.parse(UrlFetchApp.fetch(
        url, {method: 'post', payload: Utilities.newBlob([0x68, 0x69], 'text/plain')})
        .getContentText());
    console.assert(blob.data === 'hi');
    console.assert(blob.headers['Content-Type'] === 'text/plain');

    const request = UrlFetchApp.getRequest(url, {method: 'post', payload: {a: '1', b: 'x y'}});
    console.assert(request.method === 'post');
    console.assert(request.contentType === 'application/x-www-form-urlencoded');
    console.assert(request.payload === 'a=1&b=x+y');

    // An error status throws, unless muteHttpExceptions is set:
    console.assert(throws(() => UrlFetchApp.fetch('https://httpbin.org/status/404'))
        === 'Request failed for https://httpbin.org returned code 404');
    const muted = UrlFetchApp.fetch('https://httpbin.org/status/404', {muteHttpExceptions: true});
    console.assert(muted.getResponseCode() === 404);
    console.assert(muted.getBlob().getContentType() === 'text/html');

    // A host that cannot be reached throws, with a message that differs between GAS and here:
    console.assert(throws(() => UrlFetchApp.fetch('https://nonexistent.invalid/')) !== null);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
