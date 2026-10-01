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

// Client-side google.script.run shim for Google Apps Script Editor Add-ons hosted in COOL; each
// leaf call has its add-on function run in the kit, through gas-kit-runner.js:
(function() {
    if (!window.cool || typeof window.cool._postCall !== 'function') {
        console.warn('gas-shim.js: cool._postCall is not available');
        return;
    }

    // Names JS uses for thenable/serialization traps are returned undefined so an accidental
    // console.log or await on the chain does not fire a bogus remote call:
    const chainReserved = Object.freeze({
        then: 1, catch: 1, finally: 1,
        toString: 1, toJSON: 1, valueOf: 1, constructor: 1, inspect: 1
    });
    // The chain-configuration method names, exposed under ownKeys so an add-on that uses
    // Object.keys on google.script.run to discover callable server functions (as the gas-client
    // library does) sees the same shape the real Google API exposes:
    const chainMethods = ['withSuccessHandler', 'withFailureHandler', 'withUserObject'];
    function makeChain(state) {
        return new Proxy({}, {
            get: function(_target, prop) {
                if (prop === 'withSuccessHandler') {
                    return function(fn) { state.success = fn; return makeChain(state); };
                }
                if (prop === 'withFailureHandler') {
                    return function(fn) { state.failure = fn; return makeChain(state); };
                }
                if (prop === 'withUserObject') {
                    return function(o) { state.userObject = o; return makeChain(state); };
                }
                if (typeof prop !== 'string') return undefined;
                if (Object.prototype.hasOwnProperty.call(chainReserved, prop)) return undefined;
                return function() {
                    const callArgs = Array.prototype.slice.call(arguments);
                    window.cool._postCall({
                        msgId: 'Extension_GasRun',
                        functionName: prop,
                        args: callArgs
                    }).then(function(result) {
                        const value = unwrapEnvelope(result);
                        if (typeof state.success === 'function') {
                            try { state.success(value, state.userObject); }
                            catch (ex) { console.warn('gas-shim success handler threw:', ex); }
                        }
                    }, function(err) {
                        // Deliver the Error object itself; matches the real API, and Error's
                        // toString still works for handlers that only want the message:
                        if (typeof state.failure === 'function') {
                            try { state.failure(err, state.userObject); }
                            catch (ex) { console.warn('gas-shim failure handler threw:', ex); }
                        } else {
                            console.warn('gas-shim call rejected:', err);
                        }
                    });
                };
            },
            ownKeys: function() {
                return (window.__gasFunctionNames || []).concat(chainMethods);
            },
            getOwnPropertyDescriptor: function() {
                return { enumerable: true, configurable: true, value: undefined };
            }
        });
    }
    // A result marked __coolGas holds the add-on function's own return value in value, and every
    // message it passed to getUi().alert() in alerts.  Show the messages, return the value:
    function unwrapEnvelope(result) {
        if (!result || result.__coolGas !== true) return result;
        const alerts = Array.isArray(result.alerts) ? result.alerts : [];
        for (const a of alerts) {
            showAlert(a.title, a.message);
        }
        return result.value;
    }

    // GAS shows ui.alert() as a modal the script waits on.  A message that arrives once the call
    // is over becomes a dismissible banner at the top of the panel instead:
    function showAlert(title, message) {
        const box = document.createElement('div');
        box.style.cssText = 'margin: 8px; padding: 8px 28px 8px 8px; position: relative;'
            + ' border: 1px solid #d0d7de; border-left: 3px solid #0b57d0; border-radius: 4px;'
            + ' background: #f6f8fa; font: 13px -apple-system, BlinkMacSystemFont, "Segoe UI",'
            + ' sans-serif; white-space: pre-wrap; word-break: break-word;';
        if (title) {
            const h = document.createElement('div');
            h.style.cssText = 'font-weight: 600; margin-bottom: 2px;';
            h.textContent = title;
            box.appendChild(h);
        }
        const body = document.createElement('div');
        body.textContent = message || '';
        box.appendChild(body);
        const close = document.createElement('button');
        close.type = 'button';
        close.setAttribute('aria-label', 'Dismiss');
        close.textContent = '\u00d7';
        close.style.cssText = 'position: absolute; top: 2px; right: 2px; border: none;'
            + ' background: none; cursor: pointer; font-size: 16px; line-height: 1;'
            + ' padding: 2px 6px; color: #57606a;';
        close.onclick = function() { box.remove(); };
        box.appendChild(close);
        document.body.insertBefore(box, document.body.firstChild);
    }

    // The main window has this frame make the add-on's UrlFetchApp requests, as the
    // Content-Security-Policy of cool.html does not apply here:
    async function urlFetch(url, method, contentType, payload, payloadIsBase64,
                            headerNames, headerValues, followRedirects) {
        const headers = {};
        for (let i = 0; i < headerNames.length; ++i) {
            headers[headerNames[i]] = headerValues[i];
        }
        if (contentType) headers['Content-Type'] = contentType;
        try {
            const body = method === 'GET' || method === 'HEAD'
                ? undefined
                : payloadIsBase64
                    ? Uint8Array.from(atob(payload), c => c.charCodeAt(0))
                    : payload;
            const resp = await fetch(url, {
                method: method,
                headers: headers,
                body: body,
                redirect: followRedirects ? 'follow' : 'manual'
            });
            const buf = await resp.arrayBuffer();
            const bytes = new Uint8Array(buf);
            let binary = '';
            for (let i = 0; i < bytes.length; i += 0x8000) {
                binary += String.fromCharCode.apply(null, bytes.subarray(i, i + 0x8000));
            }
            const outNames = [];
            const outValues = [];
            resp.headers.forEach(function(v, k) {
                outNames.push(k);
                outValues.push(v);
            });
            return {
                code: resp.status,
                headerNames: outNames,
                headerValues: outValues,
                body: btoa(binary),
                error: ''
            };
        } catch (err) {
            return {
                code: 0, headerNames: [], headerValues: [], body: '',
                error: err && err.message ? err.message : String(err)
            };
        }
    }
    window.cool._handleHostMessage = function(data) {
        if (data.msgId !== 'Extension_UrlFetch') return false;
        const answer = function(value) {
            window.parent.postMessage(JSON.stringify({
                msgId: 'Extension_UrlFetchResult', requestId: data.requestId, value: value
            }), '*');
        };
        urlFetch.apply(null, data.args).then(answer, function(err) {
            answer({
                code: 0, headerNames: [], headerValues: [], body: '',
                error: err && err.message ? err.message : String(err)
            });
        });
        return true;
    };

    // A page's own messages get the same presentation through this:
    window.__gasShowAlert = showAlert;

    // A Storage object with the entries in items, which passes every change to onChange:
    function makeStorage(items, onChange) {
        return {
            get length() { return items.size; },
            key: function(index) {
                const keys = Array.from(items.keys());
                return index >= 0 && index < keys.length ? keys[index] : null;
            },
            getItem: function(key) {
                const value = items.get(String(key));
                return value === undefined ? null : value;
            },
            setItem: function(key, value) {
                items.set(String(key), String(value));
                onChange(String(key), String(value));
            },
            removeItem: function(key) {
                items.delete(String(key));
                onChange(String(key), null);
            },
            clear: function() {
                for (const key of Array.from(items.keys())) {
                    items.delete(key);
                    onChange(key, null);
                }
            }
        };
    }
    // The frame's localStorage holds the add-on's own entries, which the main window keeps and
    // hands over before the add-on's HTML goes in, and to which every change is passed back:
    const storageItems = new Map();
    function postStorageChange(key, value) {
        window.parent.postMessage(JSON.stringify({
            msgId: 'Extension_GasStorageSet', key: key, value: value
        }), '*');
    }
    Object.defineProperty(window, 'localStorage', {
        configurable: true, value: makeStorage(storageItems, postStorageChange)
    });
    // The frame's sessionStorage lasts as long as the frame:
    Object.defineProperty(window, 'sessionStorage', {
        configurable: true, value: makeStorage(new Map(), function() {})
    });
    window.__gasStorageReady = window.cool._postCall({ msgId: 'Extension_GasStorageLoad' }).then(
        function(items) {
            for (const key of Object.keys(items || {})) storageItems.set(key, items[key]);
        });

    // A dialog closes on Escape, as the main window's other dialogs do:
    if (window.cool.idPrefix) {
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') window.cool.dialog.cancel();
        });
    }

    window.google = window.google || {};
    window.google.script = window.google.script || {};
    Object.defineProperty(window.google.script, 'run', {
        configurable: true,
        get: function() {
            return makeChain({ success: null, failure: null, userObject: undefined });
        }
    });
    window.google.script.host = window.google.script.host || {
        close: function() {
            if (window.cool.idPrefix) {
                window.cool.dialog.close();
            } else {
                window.parent.postMessage(JSON.stringify({ msgId: 'Extension_Close' }), '*');
            }
        },
        setHeight: function() {},
        setWidth: function() {},
    };
})();
