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
// leaf call ships gas-kit-runner.js (the kit half) via cool.callRemote:
(function() {
    if (!window.cool || typeof window.cool.callRemote !== 'function') {
        console.warn('gas-shim.js: cool.callRemote is not available');
        return;
    }

    // Names JS uses for thenable/serialization traps are returned undefined so an accidental
    // console.log or await on the chain does not fire a bogus remote call:
    const chainReserved = Object.freeze({
        then: 1, catch: 1, finally: 1,
        toString: 1, toJSON: 1, valueOf: 1, constructor: 1, inspect: 1
    });
    function makeChain(state) {
        const target = function() {};
        return new Proxy(target, {
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
                    const proxyId = 'gasrt' + (nextClientRuntimeId++);
                    window.cool.registerProxy(proxyId, clientRuntimeHandlers);
                    const done = function() { window.cool.unregisterProxy(proxyId); };
                    // Explicit source/line so kit-side stack frames map to gas-kit-runner.js on
                    // disk:
                    window.cool.callRemote(
                        { fn: window.__gasKitRunner, source: 'gas-kit-runner.js', line: 13 },
                        proxyId,
                        window.__gasScriptSources || [],
                        window.__gasScriptNames || [],
                        prop,
                        callArgs).then(function(value) {
                        done();
                        if (typeof state.success === 'function') {
                            try { state.success(value, state.userObject); }
                            catch (ex) { console.warn('gas-shim success handler threw:', ex); }
                        }
                    }, function(err) {
                        done();
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
            }
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
            window.parent.postMessage(JSON.stringify({ msgId: 'Extension_Close' }), '*');
        },
        setHeight: function() {},
        setWidth: function() {},
    };

    let nextClientRuntimeId = 0;

    // Extension id keyed into localStorage so one add-on's user properties don't see another's:
    const extensionIdMatch = location.pathname.match(/\/extensions\/([^/]+)\//);
    const propStoragePrefix = 'gas-user-props:'
        + (extensionIdMatch ? extensionIdMatch[1] : 'unknown') + ':';
    function propKeys() {
        const out = [];
        for (let i = 0; i < localStorage.length; ++i) {
            const k = localStorage.key(i);
            if (k !== null && k.indexOf(propStoragePrefix) === 0) {
                out.push(k.substring(propStoragePrefix.length));
            }
        }
        return out;
    }

    const clientRuntimeHandlers = {
        translate: function() {
            throw new Error(
                'LanguageApp.translate is not supported in the COOL Apps Script wrapper');
        },
        userPropGetProperty: function(key) {
            const raw = localStorage.getItem(propStoragePrefix + String(key));
            return { IsPresent: raw !== null, Value: raw === null ? '' : raw };
        },
        userPropSetProperty: function(key, value) {
            localStorage.setItem(propStoragePrefix + String(key), String(value));
        },
        userPropDeleteProperty: function(key) {
            localStorage.removeItem(propStoragePrefix + String(key));
        },
        userPropGetKeys: function() { return propKeys(); },
        userPropDeleteAll: function() {
            for (const k of propKeys()) {
                localStorage.removeItem(propStoragePrefix + k);
            }
        }
    };

})();
