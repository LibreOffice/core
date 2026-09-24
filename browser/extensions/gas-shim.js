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
                    const proxyId = window.cool.idPrefix + 'gasrt' + (nextClientRuntimeId++);
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
                        callArgs,
                        extensionIdMatch ? extensionIdMatch[1] : '').then(function(result) {
                        done();
                        const value = unwrapEnvelope(result);
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
            },
            ownKeys: function() {
                return (window.__gasFunctionNames || []).concat(chainMethods);
            },
            getOwnPropertyDescriptor: function() {
                return { enumerable: true, configurable: true, value: undefined };
            }
        });
    }
    // A result marked __coolGas holds the add-on function's own return value in value, every
    // message it passed to getUi().alert() in alerts, (if any) the HtmlOutput file its
    // ui.showSidebar was called with in sidebarFile, and (if any) the dialog one of its
    // ui.show*Dialog calls asked for in dialog.  Show the messages, ask the host to open the
    // sidebar and the dialog, return the value:
    function unwrapEnvelope(result) {
        if (!result || result.__coolGas !== true) return result;
        const alerts = Array.isArray(result.alerts) ? result.alerts : [];
        for (const a of alerts) {
            showAlert(a.title, a.message);
        }
        if (typeof result.sidebarFile === 'string' && result.sidebarFile) {
            window.parent.postMessage(JSON.stringify({
                msgId: 'Extension_OpenSidebar',
                sidebarFile: result.sidebarFile
            }), '*');
        }
        if (result.dialog) {
            window.parent.postMessage(JSON.stringify({
                msgId: 'Extension_ShowGasDialog',
                dialog: result.dialog
            }), '*');
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

    // A page's own messages get the same presentation through this:
    window.__gasShowAlert = showAlert;

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

    let nextClientRuntimeId = 0;

    // Extension id keyed into localStorage so one add-on's user properties don't see another's:
    const baseParam = new URLSearchParams(location.search).get('base') || '';
    const extensionIdMatch = baseParam.match(/\/extensions\/([^/]+)\/?$/);
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

    // A cache entry lives in localStorage under the add-on and the scope, as JSON with the value
    // and the time at which it expires:
    function cacheKey(scope, key) {
        return 'gas-cache:' + (extensionIdMatch ? extensionIdMatch[1] : 'unknown')
            + ':' + scope + ':' + key;
    }
    // The cache entries of all add-ons share a budget of a million characters, well below the
    // localStorage quota that COOL's own settings need too, and storing one drops the expired
    // entries and, as far as needed, those that expire soonest:
    function cacheStore(storageKey, envelope) {
        const now = Date.now();
        const entries = [];
        const expired = [];
        let used = 0;
        for (let i = 0; i < localStorage.length; ++i) {
            const k = localStorage.key(i);
            if (k === null || !k.startsWith('gas-cache:') || k === storageKey) continue;
            const raw = localStorage.getItem(k) || '';
            let exp = 0;
            try { exp = JSON.parse(raw).exp; } catch (_) { /* dropped as expired */ }
            if (typeof exp !== 'number' || exp <= now) {
                expired.push(k);
            } else {
                entries.push({ key: k, exp: exp, size: k.length + raw.length });
                used += k.length + raw.length;
            }
        }
        for (const k of expired) localStorage.removeItem(k);
        entries.sort(function(a, b) { return a.exp - b.exp; });
        const size = storageKey.length + envelope.length;
        let next = 0;
        while (used + size > 1000000 && next < entries.length) {
            const entry = entries[next++];
            localStorage.removeItem(entry.key);
            used -= entry.size;
        }
        // A value that does not fit is not cached, which GAS allows as well:
        for (;;) {
            if (used + size > 1000000) {
                localStorage.removeItem(storageKey);
                return;
            }
            try {
                localStorage.setItem(storageKey, envelope);
                return;
            } catch (_) {
                if (next === entries.length) {
                    localStorage.removeItem(storageKey);
                    return;
                }
                const entry = entries[next++];
                localStorage.removeItem(entry.key);
                used -= entry.size;
            }
        }
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
        },
        cacheGet: function(scope, key) {
            const raw = localStorage.getItem(cacheKey(String(scope), String(key)));
            if (raw === null) return { IsPresent: false, Value: '' };
            try {
                const parsed = JSON.parse(raw);
                if (parsed && typeof parsed.exp === 'number' && Date.now() < parsed.exp) {
                    return { IsPresent: true, Value: String(parsed.v) };
                }
            } catch (_) { /* fall through and drop */ }
            localStorage.removeItem(cacheKey(String(scope), String(key)));
            return { IsPresent: false, Value: '' };
        },
        cachePut: function(scope, key, value, ttlSeconds) {
            const envelope = JSON.stringify({
                v: String(value),
                exp: Date.now() + Math.max(0, ttlSeconds) * 1000
            });
            cacheStore(cacheKey(String(scope), String(key)), envelope);
        },
        cacheRemove: function(scope, key) {
            localStorage.removeItem(cacheKey(String(scope), String(key)));
        },
        urlFetch: async function(url, method, contentType, payload, payloadIsBase64,
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
    };

})();
