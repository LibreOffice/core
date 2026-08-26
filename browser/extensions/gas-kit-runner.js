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
                getType: function() { return 'TEXT'; }
            };
            return t;
        }

        function selectionFacade() {
            const doc = activeDoc();
            const sel = doc.getSelection();
            if (!sel) return null;
            const txt = sel.getText();
            if (!txt) return null;
            const el = textFacade(sel, txt);
            // Report the range as partial so filters looking for a subrange (Docs-to-Markdown etc.)
            // accept it:
            const range = {
                isPartial: function() { return true; },
                getElement: function() { return el; },
                getStartOffset: function() { return 0; },
                getEndOffsetInclusive: function() { return txt.length - 1; }
            };
            return {
                getSelectedElements: function() { return [range]; },
                getRangeElements: function() { return [range]; }
            };
        }

        function cursorFacade() {
            const doc = activeDoc();
            const sel = doc.getSelection();
            // XTextViewCursor is not exposed by scriptinterop, so approximate from the selection:
            const selText = sel ? sel.getText() : '';
            return {
                getElement: function() { return { getType: function() { return 'TEXT'; } }; },
                getOffset: function() { return 0; },
                getSurroundingText: function() {
                    return { getText: function() { return selText; } };
                },
                getSurroundingTextOffset: function() { return 0; },
                insertText: function(t) { if (sel) sel.replace(String(t)); }
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

        // Placeholder used by appendParagraph/appendListItem when there is no real UNO paragraph
        // yet:
        const emptyPara = {
            getText: function() { return ''; },
            getTextRuns: function() { return []; },
            isLeftToRight: function() { return true; }
        };

        function paragraphElement(paragraph, index) {
            const text = paragraph.getText();
            let runs = null;
            // Cached because callers hit getFontFamily/isBold repeatedly on the same paragraph:
            function runAt(offset) {
                if (runs === null) runs = paragraph.getTextRuns();
                let start = 0;
                for (let i = 0; i < runs.length; ++i) {
                    const r = runs[i];
                    const len = r.getText().length;
                    if (offset < start + len) return r;
                    start += len;
                }
                return runs.length > 0 ? runs[runs.length - 1] : null;
            }
            const textEl = {
                getType: function() { return 'TEXT'; },
                getText: function() { return text; },
                asText: function() { return textEl; },
                editAsText: function() { return textEl; },
                copy: function() { return textEl; },
                getNumChildren: function() { return 0; },
                getAttributes: function() { return {}; },
                // Apps Script guarantees at least one attribute index even for uniform text:
                getTextAttributeIndices: function() {
                    if (runs === null) runs = paragraph.getTextRuns();
                    const offsets = [];
                    let off = 0;
                    for (let i = 0; i < runs.length; ++i) {
                        offsets.push(off);
                        off += runs[i].getText().length;
                    }
                    return offsets.length > 0 ? offsets : [0];
                },
                setAttributes: function() { return textEl; },
                findText: function() { return null; },
                getParent: function() { return null; },
                getFontFamily: function(offset) {
                    const r = runAt(offset);
                    return r ? r.getFontFamily() : '';
                },
                isBold: function(offset) {
                    const r = runAt(offset);
                    return r ? r.isBold() : false;
                },
                isItalic: function(offset) {
                    const r = runAt(offset);
                    return r ? r.isItalic() : false;
                },
                isUnderline: function(offset) {
                    const r = runAt(offset);
                    return r ? r.isUnderline() : false;
                },
                isStrikethrough: function(offset) {
                    const r = runAt(offset);
                    return r ? r.isStrikethrough() : false;
                },
                getTextAlignment: function(offset) {
                    const r = runAt(offset);
                    const esc = r ? r.getEscapement() : 0;
                    if (esc > 0) return 'SUPERSCRIPT';
                    if (esc < 0) return 'SUBSCRIPT';
                    return 'NORMAL';
                },
                getLinkUrl: function(offset) {
                    const r = runAt(offset);
                    const url = r ? r.getLinkUrl() : '';
                    return url === '' ? null : url;
                }
            };
            const paraEl = {
                getType: function() { return 'PARAGRAPH'; },
                getText: function() { return text; },
                asText: function() { return textEl; },
                editAsText: function() { return textEl; },
                copy: function() { return paraEl; },
                getNumChildren: function() { return 1; },
                getChild: function() { return textEl; },
                getChildIndex: function() { return index; },
                getHeading: function() { return 'NORMAL'; },
                getAlignment: function() { return 'NORMAL'; },
                getAttributes: function() { return {}; },
                getTextAttributeIndices: function() { return []; },
                getIndentStart: function() { return 0; },
                getSpacingBefore: function() { return 0; },
                getSpacingAfter: function() { return 0; },
                getParent: function() { return null; },
                getPreviousSibling: function() { return null; },
                getNextSibling: function() { return null; },
                findElement: function() { return null; },
                findText: function() { return null; },
                isLeftToRight: function() { return paragraph.isLeftToRight(); }
            };
            return paraEl;
        }
        function footnoteContentsFacade(footnote) {
            let cached = null;
            function paras() {
                if (!cached) {
                    const list = footnote.getParagraphs();
                    cached = [];
                    for (let i = 0; i < list.length; ++i) {
                        cached.push(paragraphElement(list[i], i));
                    }
                }
                return cached;
            }
            const section = {
                getType: function() { return 'FOOTNOTE_SECTION'; },
                getNumChildren: function() { return paras().length; },
                getChild: function(n) { return paras()[n]; },
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
            const doc = activeDoc();
            let cached = null;
            function paras() {
                if (!cached) {
                    const list = doc.getParagraphs();
                    cached = [];
                    for (let i = 0; i < list.length; ++i) {
                        cached.push(paragraphElement(list[i], i));
                    }
                }
                return cached;
            }
            const body = {
                getType: function() { return 'BODY_SECTION'; },
                getText: function() { return doc.getText(); },
                getNumChildren: function() { return paras().length; },
                getChild: function(n) { return paras()[n]; },
                editAsText: function() { return textFacade(null, doc.getText()); },
                asText: function() { return textFacade(null, doc.getText()); },
                copy: function() { return body; },
                appendParagraph: function() { return paragraphElement(emptyPara, paras().length); },
                appendListItem: function() { return paragraphElement(emptyPara, paras().length); },
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
            ElementType: {
                BODY_SECTION: 'BODY_SECTION',
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
                HORIZONTAL_RULE: 'HORIZONTAL_RULE',
                INLINE_DRAWING: 'INLINE_DRAWING',
                INLINE_IMAGE: 'INLINE_IMAGE',
                LIST_ITEM: 'LIST_ITEM',
                PAGE_BREAK: 'PAGE_BREAK',
                PARAGRAPH: 'PARAGRAPH',
                PERSON: 'PERSON',
                RICH_LINK: 'RICH_LINK',
                TABLE: 'TABLE',
                TABLE_CELL: 'TABLE_CELL',
                TABLE_OF_CONTENTS: 'TABLE_OF_CONTENTS',
                TABLE_ROW: 'TABLE_ROW',
                TEXT: 'TEXT',
                UNSUPPORTED: 'UNSUPPORTED'
            },
            TextAlignment: { NORMAL: 'NORMAL', SUBSCRIPT: 'SUBSCRIPT', SUPERSCRIPT: 'SUPERSCRIPT' },
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
