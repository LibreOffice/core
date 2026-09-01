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

        function textFacade(sel, initial) {
            let cur = initial;
            const commit = function() { if (sel) sel.replace(cur); };
            const t = {
                getText: function() { return cur; },
                setText: function(v) { cur = String(v); commit(); return t; },
                appendText: function(v) { cur += String(v); commit(); return t; },
                insertText: function(idx, v) {
                    cur = cur.substring(0, idx) + String(v) + cur.substring(idx);
                    commit();
                    return t;
                },
                deleteText: function(a, b) {
                    cur = cur.substring(0, a) + cur.substring(b + 1);
                    commit();
                    return t;
                },
                clear: function() { cur = ''; commit(); return t; },
                asText: function() { return t; },
                editAsText: function() { return t; },
                removeFromParent: function() { cur = ''; commit(); return t; },
                getParent: function() {
                    return {
                        asText: function() { return t; },
                        getPreviousSibling: function() { return null; },
                        getNextSibling: function() { return null; }
                    };
                },
                getPreviousSibling: function() { return null; },
                getNextSibling: function() { return null; },
                getType: function() { return uno.idl.scriptinterop.ElementType.TEXT; }
            };
            return t;
        }

        function selectionFacade() {
            const sel = activeDoc().getSelection();
            if (!sel) return null;
            const ranges = sel.getRangeElements();
            if (!ranges.length) return null;
            const wrapped = [];
            for (let i = 0; i < ranges.length; ++i) {
                const r = ranges[i];
                const para = r.getElement();
                const el = para ? para : textFacade(sel, sel.getText());
                wrapped.push({
                    isPartial: function() { return r.isPartial(); },
                    getElement: function() { return el; },
                    getStartOffset: function() { return r.getStartOffset(); },
                    getEndOffsetInclusive: function() { return r.getEndOffsetInclusive(); }
                });
            }
            return {
                getSelectedElements: function() { return wrapped; },
                getRangeElements: function() { return wrapped; }
            };
        }

        function cursorFacade() {
            const xc = activeDoc().getCursor();
            return {
                getElement: function() {
                    const p = xc.getElement();
                    return p ? p : { getType: function() {
                        return uno.idl.scriptinterop.ElementType.TEXT;
                    } };
                },
                getOffset: function() { return xc.getOffset(); },
                getSurroundingText: function() {
                    const t = xc.getSurroundingText();
                    return { getText: function() { return t; } };
                },
                getSurroundingTextOffset: function() { return xc.getOffset(); },
                insertText: function(t) { xc.insertText(String(t)); }
            };
        }

        const uiStub = {
            createAddonMenu: function() {
                const m = { addItem: function() { return m; }, addToUi: function() {} };
                return m;
            },
            createMenu: function() {
                const m = { addItem: function() { return m; }, addToUi: function() {} };
                return m;
            },
            showSidebar: function() {},
            showDialog: function() {
                throw new Error(
                    'DocumentApp.getUi().showDialog is not yet supported in the COOL Apps'
                        + ' Script wrapper');
            },
            showModalDialog: function() {
                throw new Error(
                    'DocumentApp.getUi().showModalDialog is not yet supported in the COOL'
                        + ' Apps Script wrapper');
            },
            showModelessDialog: function() {
                throw new Error(
                    'DocumentApp.getUi().showModelessDialog is not yet supported in the COOL'
                        + ' Apps Script wrapper');
            },
            alert: function() {
                throw new Error(
                    'DocumentApp.getUi().alert is not yet supported in the COOL Apps Script'
                        + ' wrapper');
            },
            prompt: function() {
                throw new Error(
                    'DocumentApp.getUi().prompt is not yet supported in the COOL Apps Script'
                        + ' wrapper');
            }
        };

        function footnoteContentsFacade(footnote) {
            const paras = footnote.getParagraphs();
            const section = {
                getType: function() { return 'FOOTNOTE_SECTION'; },
                getNumChildren: function() { return paras.length; },
                getChild: function(n) { return paras[n]; },
                getParent: function() { return null; },
                getAttributes: function() { return {}; },
                findText: function() { return null; }
            };
            return section;
        }
        function footnoteFacade(footnote) {
            let contents = null;
            return {
                getType: function() { return 'FOOTNOTE'; },
                getFootnoteContents: function() {
                    if (!contents) contents = footnoteContentsFacade(footnote);
                    return contents;
                },
                getParent: function() { return null; },
                getAttributes: function() { return {}; }
            };
        }

        function bodyFacade() {
            const xbody = activeDoc().getBody();
            const body = {
                getType: function() { return uno.idl.scriptinterop.ElementType.BODY_SECTION; },
                getText: function() { return xbody.getText(); },
                getNumChildren: function() { return xbody.getNumChildren(); },
                getChild: function(n) { return xbody.getChild(n); },
                editAsText: function() { return textFacade(null, xbody.getText()); },
                asText: function() { return textFacade(null, xbody.getText()); },
                copy: function() { return body; },
                appendParagraph: function(text) { return xbody.appendParagraph(text || ''); },
                appendListItem: function(text) { return xbody.appendListItem(text || ''); },
                getParent: function() { return null; },
                getAttributes: function() { return {}; },
                findText: function() { return null; }
            };
            return body;
        }

        function footnotesFacade() {
            const doc = activeDoc();
            const list = doc.getFootnotes();
            const facades = [];
            for (let i = 0; i < list.length; ++i) {
                facades.push(footnoteFacade(list[i]));
            }
            return facades;
        }

        globalThis.DocumentApp = {
            getActiveDocument: function() {
                return {
                    getSelection: selectionFacade,
                    getCursor: cursorFacade,
                    getBody: bodyFacade,
                    getFootnotes: footnotesFacade,
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
                FOOTNOTE: 'FOOTNOTE',
                FOOTNOTE_SECTION: 'FOOTNOTE_SECTION',
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
            HorizontalAlignment: {
                LEFT: 'LEFT', CENTER: 'CENTER', RIGHT: 'RIGHT', JUSTIFY: 'JUSTIFY'
            },
            VerticalAlignment: { TOP: 'TOP', MIDDLE: 'MIDDLE', BOTTOM: 'BOTTOM' },
            ParagraphHeading: {
                NORMAL: 'NORMAL', TITLE: 'TITLE', SUBTITLE: 'SUBTITLE', HEADING1: 'HEADING1',
                HEADING2: 'HEADING2', HEADING3: 'HEADING3', HEADING4: 'HEADING4',
                HEADING5: 'HEADING5', HEADING6: 'HEADING6'
            },
            GlyphType: {
                BULLET: 'BULLET', HOLLOW_BULLET: 'HOLLOW_BULLET', SQUARE_BULLET: 'SQUARE_BULLET',
                NUMBER: 'NUMBER', LATIN_UPPER: 'LATIN_UPPER', LATIN_LOWER: 'LATIN_LOWER',
                ROMAN_UPPER: 'ROMAN_UPPER', ROMAN_LOWER: 'ROMAN_LOWER'
            }
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
            }
        };

        // Round-tripped over the XClientRuntime proxy so the store lives in the iframe's
        // localStorage:
        function userPropsFacade() {
            const f = {
                getProperty: function(k) {
                    $internal.suppressLegacyUnoApiStart();
                    try {
                        const opt = clientRuntime.userPropGetProperty(String(k));
                        return opt.IsPresent ? opt.Value : null;
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
                            const opt = clientRuntime.userPropGetProperty(keys[i]);
                            if (opt.IsPresent) out[keys[i]] = opt.Value;
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
        const fn = globalThis[fnName];
        if (typeof fn !== 'function') {
            throw new Error('Apps Script function not defined: ' + fnName);
        }
        return fn.apply(null, callArgs || []);
    } finally {
        $internal.takeProxy(proxyId);
    }
};
