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

// The next line's number is recorded as a hardcoded 13 in browser/extensions/gas-kit-runner.js:
window.__gasKitRunner = function(proxyId, gsSources, gsNames, fnName, callArgs) {
    // Body must be self-contained; gas-shim.js ships it as source text via fn.toString():
    const clientRuntime = $internal.createProxy(uno.idl.scriptinterop.XClientRuntime, proxyId);
    try {
        function activeDoc() { return cool.getActiveDocument(); }

        // What getUi() collects over one call: the messages an add-on passed to alert(), and the
        // items it put in its menu.  Both travel back with the call's result:
        const pendingAlerts = [];
        const menuItems = [];

        // ButtonSet and Button members are objects rather than strings so alert()'s overloads
        // stay distinguishable: alert(title, prompt) and alert(prompt, buttons) both take two
        // arguments, and only the second one's type tells them apart:
        const buttonValues = {
            OK: { name: 'OK' }, CANCEL: { name: 'CANCEL' }, YES: { name: 'YES' },
            NO: { name: 'NO' }, CLOSE: { name: 'CLOSE' }
        };
        const buttonSetValues = {
            OK: { name: 'OK' }, OK_CANCEL: { name: 'OK_CANCEL' }, YES_NO: { name: 'YES_NO' },
            YES_NO_CANCEL: { name: 'YES_NO_CANCEL' }
        };

        // An add-on's onOpen() builds its menu against this.  Submenu items land in the same flat
        // list as top-level ones, so a menu is a sequence of captioned items and separators:
        function menuBuilder() {
            const m = {
                addItem: function(caption, functionName) {
                    menuItems.push(
                        { caption: String(caption), functionName: String(functionName) });
                    return m;
                },
                addSeparator: function() {
                    menuItems.push({ separator: true });
                    return m;
                },
                addSubMenu: function() { return m; },
                addToUi: function() {}
            };
            return m;
        }

        const uiStub = {
            createAddonMenu: menuBuilder,
            createMenu: menuBuilder,
            showSidebar: function() {},
            showDialog: function() {
                throw new Error(
                    'getUi().showDialog is not yet supported in the COOL Apps Script wrapper');
            },
            showModalDialog: function() {
                throw new Error(
                    'getUi().showModalDialog is not yet supported in the COOL Apps Script'
                        + ' wrapper');
            },
            showModelessDialog: function() {
                throw new Error(
                    'getUi().showModelessDialog is not yet supported in the COOL Apps Script'
                        + ' wrapper');
            },
            // GAS blocks the script on a modal here.  This one only records the message, which
            // travels back with the call's result, so an add-on that alerts and then keeps
            // editing has its message appear after the edit rather than before it:
            alert: function() {
                const texts = [];
                for (let i = 0; i !== arguments.length; ++i) {
                    const a = arguments[i];
                    if (typeof a === 'string' || typeof a === 'number') {
                        texts.push(String(a));
                    }
                }
                pendingAlerts.push({
                    title: texts.length > 1 ? texts[0] : '',
                    message: texts.length > 1 ? texts.slice(1).join('\n') : (texts[0] || '')
                });
                return buttonValues.OK;
            },
            prompt: function() {
                throw new Error(
                    'getUi().prompt is not yet supported in the COOL Apps Script wrapper');
            },
            ButtonSet: buttonSetValues,
            Button: buttonValues
        };

        globalThis.DocumentApp = {
            getActiveDocument: function() {
                return {
                    getSelection: function() { return activeDoc().getSelection(); },
                    getCursor: function() { return activeDoc().getCursor(); },
                    getBody: function() { return activeDoc().getBody(); },
                    getFootnotes: function() { return activeDoc().getFootnotes(); },
                    newRange: function() { return activeDoc().newRange(); },
                    setSelection: function(sel) { activeDoc().setSelection(sel); },
                    getName: function() { return 'Untitled'; },
                    getUrl: function() { return ''; },
                    getId: function() { return ''; },
                    getUi: function() { return uiStub; }
                };
            },
            getUi: function() { return uiStub; },
            // Members scriptinterop's ElementType has round-trip as the same enum object, so a
            // strict === comparison with what getType() returns matches; members GAS defines that
            // scriptinterop does not know stay as strings that never match anything on our side:
            ElementType: {
                BODY_SECTION: uno.idl.scriptinterop.ElementType.BODY_SECTION,
                COMMENT_SECTION: 'COMMENT_SECTION',
                DATE: 'DATE',
                DOCUMENT: 'DOCUMENT',
                EQUATION: 'EQUATION',
                EQUATION_FUNCTION: 'EQUATION_FUNCTION',
                EQUATION_FUNCTION_ARGUMENT_SEPARATOR: 'EQUATION_FUNCTION_ARGUMENT_SEPARATOR',
                EQUATION_SYMBOL: 'EQUATION_SYMBOL',
                FOOTER_SECTION: 'FOOTER_SECTION',
                FOOTNOTE: uno.idl.scriptinterop.ElementType.FOOTNOTE,
                FOOTNOTE_SECTION: uno.idl.scriptinterop.ElementType.FOOTNOTE_SECTION,
                HEADER_SECTION: 'HEADER_SECTION',
                HORIZONTAL_RULE: uno.idl.scriptinterop.ElementType.HORIZONTAL_RULE,
                INLINE_DRAWING: 'INLINE_DRAWING',
                INLINE_IMAGE: uno.idl.scriptinterop.ElementType.INLINE_IMAGE,
                LIST_ITEM: uno.idl.scriptinterop.ElementType.LIST_ITEM,
                PAGE_BREAK: uno.idl.scriptinterop.ElementType.PAGE_BREAK,
                PARAGRAPH: uno.idl.scriptinterop.ElementType.PARAGRAPH,
                PERSON: 'PERSON',
                RICH_LINK: 'RICH_LINK',
                TABLE: uno.idl.scriptinterop.ElementType.TABLE,
                TABLE_CELL: uno.idl.scriptinterop.ElementType.TABLE_CELL,
                TABLE_OF_CONTENTS: 'TABLE_OF_CONTENTS',
                TABLE_ROW: uno.idl.scriptinterop.ElementType.TABLE_ROW,
                TEXT: uno.idl.scriptinterop.ElementType.TEXT,
                UNSUPPORTED: uno.idl.scriptinterop.ElementType.UNSUPPORTED
            },
            TextAlignment: uno.idl.scriptinterop.TextAlignment,
            HorizontalAlignment: uno.idl.scriptinterop.HorizontalAlignment,
            VerticalAlignment: { TOP: 'TOP', MIDDLE: 'MIDDLE', BOTTOM: 'BOTTOM' },
            ParagraphHeading: uno.idl.scriptinterop.ParagraphHeading,
            GlyphType: uno.idl.scriptinterop.GlyphType
        };

        function makeHtmlOutput() {
            const o = {
                setTitle: function() { return o; },
                setWidth: function() { return o; },
                setHeight: function() { return o; },
                setContent: function() { return o; },
                setSandboxMode: function() { return o; },
                getContent: function() { return ''; },
                append: function() { return o; }
            };
            return o;
        }
        globalThis.HtmlService = {
            createHtmlOutputFromFile: makeHtmlOutput,
            createHtmlOutput: makeHtmlOutput,
            createTemplateFromFile: function() { return { evaluate: makeHtmlOutput }; },
            SandboxMode: { IFRAME: 'IFRAME', NATIVE: 'NATIVE' }
        };

        globalThis.Utilities = globalThis.Utilities || {
            base64Encode: function(v) {
                return typeof btoa === 'function' ? btoa(String(v)) : String(v);
            },
            base64Decode: function(v) {
                return typeof atob === 'function' ? atob(String(v)) : String(v);
            },
            getUuid: function() {
                return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
                    const r = Math.floor(Math.random() * 16);
                    return (c === 'x' ? r : (r & 0x3) | 0x8).toString(16);
                });
            },
            newBlob: cool.newBlob.bind(cool)
        };

        // Round-tripped over the XClientRuntime proxy so the store lives in the iframe's
        // localStorage:
        function userPropsFacade() {
            const f = {
                getProperty: function(k) {
                    $internal.suppressLegacyUnoApiStart();
                    try {
                        return clientRuntime.userPropGetProperty(String(k));
                    } finally { $internal.suppressLegacyUnoApiEnd(); }
                },
                setProperty: function(k, v) {
                    clientRuntime.userPropSetProperty(String(k), String(v));
                    return f;
                },
                deleteProperty: function(k) {
                    clientRuntime.userPropDeleteProperty(String(k));
                    return f;
                },
                getProperties: function() {
                    const out = {};
                    const keys = clientRuntime.userPropGetKeys();
                    $internal.suppressLegacyUnoApiStart();
                    try {
                        for (let i = 0; i < keys.length; ++i) {
                            const value = clientRuntime.userPropGetProperty(keys[i]);
                            if (value !== null) out[keys[i]] = value;
                        }
                    } finally { $internal.suppressLegacyUnoApiEnd(); }
                    return out;
                },
                setProperties: function(o) {
                    for (const k of Object.keys(o)) {
                        clientRuntime.userPropSetProperty(String(k), String(o[k]));
                    }
                    return f;
                },
                deleteAllProperties: function() {
                    clientRuntime.userPropDeleteAll();
                    return f;
                },
                getKeys: function() { return clientRuntime.userPropGetKeys(); }
            };
            return f;
        }
        function notYetImplementedScope(name) {
            return function() {
                throw new Error(
                    'PropertiesService.' + name + ' is not yet supported in the COOL Apps'
                        + ' Script wrapper; only getUserProperties is wired up so far.');
            };
        }
        globalThis.PropertiesService = {
            getUserProperties: userPropsFacade,
            getScriptProperties: notYetImplementedScope('getScriptProperties'),
            getDocumentProperties: notYetImplementedScope('getDocumentProperties')
        };

        // Hops back to the iframe over the XClientRuntime proxy since the kit has no outbound
        // network:
        globalThis.LanguageApp = {
            translate: function(text, origin, dest) {
                return clientRuntime.translate(
                    String(text), String(origin || ''), String(dest || ''));
            }
        };

        globalThis.Logger = globalThis.Logger || {
            log: function() { console.log.apply(console, arguments); }
        };

        globalThis.Session = globalThis.Session || {
            getActiveUser: function() {
                return { getEmail: function() { return ''; } };
            }
        };

        // Eval each .gs under its own filename so exception messages name the .gs, not the runner
        // blob:
        for (let i = 0; i < gsSources.length; ++i) {
            const name = (gsNames && gsNames[i]) || ('gs-source-' + i);
            $internal.evalWithSource(gsSources[i], name, 1);
        }
        // The name __coolGasMenu is reserved and stands for none of the add-on's own functions:
        // it runs onOpen() and returns the menu that building it produced.  That is how a
        // menu-driven add-on, which ships no HTML at all, still describes a user interface:
        let value;
        if (fnName === '__coolGasMenu') {
            if (typeof globalThis.onOpen === 'function') {
                globalThis.onOpen({});
            }
            value = menuItems;
        } else {
            const fn = globalThis[fnName];
            if (typeof fn !== 'function') {
                throw new Error('Apps Script function not defined: ' + fnName);
            }
            value = fn.apply(null, callArgs || []);
        }
        // A result marked __coolGas holds the add-on function's own return value in value, and
        // every message it passed to getUi().alert() in alerts:
        return { __coolGas: true, value: value, alerts: pendingAlerts };
    } finally {
        $internal.takeProxy(proxyId);
    }
};
