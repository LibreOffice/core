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

// The next line's number is recorded as a hardcoded 13 in browser/extensions/gas-shim.js:
globalThis.__gasKitRunner = function(
    proxyId, gsSources, gsNames, fnName, callArgs, extensionId, libraries)
{
    // Body must be self-contained; gas-shim.js ships it as source text via fn.toString():
    const clientRuntime = $internal.createProxy(uno.idl.scriptinterop.XClientRuntime, proxyId);
    try {
        // One document object for the whole call, as it holds the position that setCursor set:
        let activeDocument = null;
        function activeDoc() {
            if (activeDocument === null) {
                activeDocument = cool.getActiveDocument();
            }
            return activeDocument;
        }

        // What getUi() collects over one call: the messages an add-on passed to alert(), the
        // items it put in its menu, and (if any) the sidebar file and the dialog it asked us to
        // show.  All travel back with the call's result:
        const pendingAlerts = [];
        const menuItems = [];
        let showSidebarFile = null;
        let showDialogSpec = null;

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

        function recordDialog(callName, html, title) {
            if (!html || !html.__gasSourceFile) {
                throw new Error(
                    'getUi().' + callName + ' supports only HtmlOutput made from an add-on file'
                        + ' in the COOL Apps Script wrapper');
            }
            showDialogSpec = {
                file: html.__gasSourceFile,
                title: title === undefined ? html.__gasTitle : String(title),
                width: html.__gasWidth,
                height: html.__gasHeight,
                templateValues: html.__gasTemplateValues
            };
        }

        const uiStub = {
            createAddonMenu: menuBuilder,
            createMenu: menuBuilder,
            showSidebar: function(html) {
                showSidebarFile = html && html.__gasSourceFile ? html.__gasSourceFile : null;
            },
            showDialog: function(html) { recordDialog('showDialog', html, undefined); },
            showModalDialog: function(html, title) {
                recordDialog('showModalDialog', html, title);
            },
            // The host has modal dialogs only, so a modeless one blocks the document too:
            showModelessDialog: function(html, title) {
                recordDialog('showModelessDialog', html, title);
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
                    newPosition: function(element, offset) {
                        return activeDoc().newPosition(element, offset);
                    },
                    newRange: function() { return activeDoc().newRange(); },
                    setCursor: function(position) { activeDoc().setCursor(position); },
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

        // GAS's Spreadsheet/Sheet/Range on top of scriptinterop's XSpreadsheet/XSheet/XRange.
        // Only what add-ons have actually asked for so far is wired up; anything missing is
        // simply absent, so an add-on that needs more fails naming the call it wanted:
        function activeSpreadsheet() { return cool.getActiveSpreadsheet(); }

        // An empty cell reads as a void Any, which arrives here as null, while GAS hands out ''
        // for it - and add-ons test a cell for emptiness with === '':
        function cellValue(v) { return v === null || v === undefined ? '' : v; }

        function rangeFacade(xr, sheet) {
            if (!xr) return null;
            const r = {
                getRow: function() { return xr.getRow(); },
                getRowIndex: function() { return xr.getRow(); },
                getColumn: function() { return xr.getColumn(); },
                getColumnIndex: function() { return xr.getColumn(); },
                getNumRows: function() { return xr.getNumRows(); },
                getHeight: function() { return xr.getNumRows(); },
                getNumColumns: function() { return xr.getNumColumns(); },
                getWidth: function() { return xr.getNumColumns(); },
                getLastRow: function() { return xr.getRow() + xr.getNumRows() - 1; },
                getLastColumn: function() { return xr.getColumn() + xr.getNumColumns() - 1; },
                getSheet: function() { return sheet; },
                getValue: function() { return cellValue(xr.getValue()); },
                getValues: function() {
                    return xr.getValues().map(function(row) { return row.map(cellValue); });
                },
                // Formatted cell text, which scriptinterop has no call of its own for yet.
                // Stringifying the raw values gets the emptiness tests add-ons use this for
                // right, but a formatted number comes out unformatted:
                getDisplayValues: function() {
                    return xr.getValues().map(function(row) {
                        return row.map(function(v) { return String(cellValue(v)); });
                    });
                },
                setValue: function(v) { xr.setValue(v); return r; },
                setValues: function(v) { xr.setValues(v); return r; },
                // GAS counts an omitted numRows/numColumns as "keep this dimension":
                offset: function(rowOffset, columnOffset, numRows, numColumns) {
                    return rangeFacade(
                        xr.offset(
                            rowOffset, columnOffset,
                            numRows === undefined ? xr.getNumRows() : numRows,
                            numColumns === undefined ? xr.getNumColumns() : numColumns),
                        sheet);
                },
                setBackground: function(c) { xr.setBackgroundColor(String(c)); return r; },
                setFontWeight: function(w) { xr.setFontWeight(String(w)); return r; },
                setFontStyle: function(w) { xr.setFontStyle(String(w)); return r; },
                setFontColor: function(c) { xr.setFontColor(String(c)); return r; },
                setNumberFormat: function(f) { xr.setNumberFormat(String(f)); return r; }
            };
            return r;
        }

        function sheetFacade(xs) {
            if (!xs) return null;
            const s = {
                getName: function() { return xs.getName(); },
                getSheetName: function() { return xs.getName(); },
                // One JS name for scriptinterop's four getRange overloads, dispatched the way
                // GAS dispatches its own: A1 notation, a cell, a column of cells, a block:
                getRange: function(a, b, numRows, numColumns) {
                    if (typeof a === 'string') return rangeFacade(xs.getRange(a), s);
                    if (numRows === undefined) return rangeFacade(xs.getRange(a, b), s);
                    if (numColumns === undefined) {
                        return rangeFacade(xs.getRange(a, b, numRows), s);
                    }
                    return rangeFacade(xs.getRange(a, b, numRows, numColumns), s);
                },
                getActiveRange: function() { return rangeFacade(xs.getActiveRange(), s); },
                getActiveCell: function() { return rangeFacade(xs.getActiveCell(), s); },
                getDataRange: function() { return rangeFacade(xs.getDataRange(), s); },
                getMaxRows: function() { return xs.getMaxRows(); },
                getMaxColumns: function() { return xs.getMaxColumns(); },
                getLastRow: function() { return xs.getLastRow(); },
                getLastColumn: function() { return xs.getLastColumn(); },
                getFrozenRows: function() { return xs.getFrozenRows(); },
                getFrozenColumns: function() { return xs.getFrozenColumns(); },
                deleteRow: function(row) { xs.deleteRow(row); return s; },
                deleteRows: function(row, numRows) { xs.deleteRows(row, numRows); return s; },
                deleteColumn: function(column) { xs.deleteColumn(column); return s; },
                deleteColumns: function(column, numColumns) {
                    xs.deleteColumns(column, numColumns);
                    return s;
                },
                setColumnWidth: function(column, pixels) {
                    xs.setColumnWidth(column, pixels);
                    return s;
                },
                autoResizeColumn: function(column) { xs.autoResizeColumns(column, 1); return s; },
                autoResizeColumns: function(column, numColumns) {
                    xs.autoResizeColumns(column, numColumns);
                    return s;
                },
                autoResizeRows: function(row, numRows) {
                    xs.autoResizeRows(row, numRows);
                    return s;
                },
                clear: function() { xs.clear(); return s; },
                getParent: function() { return spreadsheetFacade(activeSpreadsheet()); }
            };
            return s;
        }

        function spreadsheetFacade(xss) {
            const ss = {
                getName: function() { return xss.getName(); },
                getId: function() { return ''; },
                getUrl: function() { return ''; },
                getActiveSheet: function() { return sheetFacade(xss.getActiveSheet()); },
                getSheetByName: function(name) {
                    return sheetFacade(xss.getSheetByName(String(name)));
                },
                getSheets: function() {
                    return xss.getSheets().map(function(x) { return sheetFacade(x); });
                },
                insertSheet: function(name) {
                    return sheetFacade(name === undefined ? xss.insertSheet()
                                                          : xss.insertSheet(String(name)));
                },
                getRangeByName: function(name) {
                    return rangeFacade(xss.getRangeByName(String(name)), null);
                },
                getActiveRange: function() {
                    return sheetFacade(xss.getActiveSheet()).getActiveRange();
                },
                getActiveCell: function() {
                    return sheetFacade(xss.getActiveSheet()).getActiveCell();
                },
                flush: function() { xss.flush(); },
                // Not a modal, but the same one-way message, so it takes the alert path:
                toast: function(message, title) {
                    pendingAlerts.push(
                        { title: title === undefined ? '' : String(title),
                          message: String(message) });
                },
                getUi: function() { return uiStub; }
            };
            return ss;
        }

        globalThis.SpreadsheetApp = {
            getActive: function() { return spreadsheetFacade(activeSpreadsheet()); },
            getActiveSpreadsheet: function() { return spreadsheetFacade(activeSpreadsheet()); },
            getActiveSheet: function() {
                return sheetFacade(activeSpreadsheet().getActiveSheet());
            },
            getActiveRange: function() {
                return sheetFacade(activeSpreadsheet().getActiveSheet()).getActiveRange();
            },
            getActiveCell: function() {
                return sheetFacade(activeSpreadsheet().getActiveSheet()).getActiveCell();
            },
            flush: function() { activeSpreadsheet().flush(); },
            getUi: function() { return uiStub; }
        };

        function activePresentation() { return cool.getActivePresentation(); }

        // The enums round-trip as the same enum objects scriptinterop hands back, so a strict ===
        // comparison with a getter's result matches:
        globalThis.SlidesApp = {
            getActivePresentation: activePresentation,
            newAffineTransformBuilder: function() { return cool.newAffineTransformBuilder(); },
            getUi: function() { return uiStub; },
            AlignmentPosition: uno.idl.scriptinterop.AlignmentPosition,
            ArrowStyle: uno.idl.scriptinterop.ArrowStyle,
            AutofitType: uno.idl.scriptinterop.AutofitType,
            AutoTextType: uno.idl.scriptinterop.AutoTextType,
            CellMergeState: uno.idl.scriptinterop.CellMergeState,
            ColorType: uno.idl.scriptinterop.ColorType,
            ContentAlignment: uno.idl.scriptinterop.ContentAlignment,
            DashStyle: uno.idl.scriptinterop.DashStyle,
            FillType: uno.idl.scriptinterop.FillType,
            LineCategory: uno.idl.scriptinterop.LineCategory,
            LineFillType: uno.idl.scriptinterop.LineFillType,
            LineType: uno.idl.scriptinterop.LineType,
            LinkType: uno.idl.scriptinterop.LinkType,
            ListPreset: uno.idl.scriptinterop.ListPreset,
            PageBackgroundType: uno.idl.scriptinterop.PageBackgroundType,
            PageElementType: uno.idl.scriptinterop.PageElementType,
            PageType: uno.idl.scriptinterop.PageType,
            ParagraphAlignment: uno.idl.scriptinterop.ParagraphAlignment,
            PlaceholderType: uno.idl.scriptinterop.PlaceholderType,
            PredefinedLayout: uno.idl.scriptinterop.PredefinedLayout,
            SelectionType: uno.idl.scriptinterop.SelectionType,
            ShapeType: uno.idl.scriptinterop.ShapeType,
            SlideLinkingMode: uno.idl.scriptinterop.SlideLinkingMode,
            SlidePosition: uno.idl.scriptinterop.SlidePosition,
            SpacingMode: uno.idl.scriptinterop.SpacingMode,
            TextBaselineOffset: uno.idl.scriptinterop.TextBaselineOffset,
            TextDirection: uno.idl.scriptinterop.TextDirection,
            ThemeColorType: uno.idl.scriptinterop.ThemeColorType,
            VideoSourceType: uno.idl.scriptinterop.VideoSourceType
        };

        function makeHtmlOutput(fileName, templateValues) {
            const o = {
                __gasSourceFile: fileName,
                __gasTemplateValues: templateValues,
                __gasTitle: undefined,
                __gasWidth: undefined,
                __gasHeight: undefined,
                setTitle: function(t) { o.__gasTitle = String(t); return o; },
                getTitle: function() { return o.__gasTitle === undefined ? '' : o.__gasTitle; },
                setWidth: function(w) { o.__gasWidth = Number(w); return o; },
                getWidth: function() { return o.__gasWidth; },
                setHeight: function(h) { o.__gasHeight = Number(h); return o; },
                getHeight: function() { return o.__gasHeight; },
                setContent: function() { return o; },
                setSandboxMode: function() { return o; },
                getContent: function() { return ''; },
                append: function() { return o; }
            };
            return o;
        }
        globalThis.HtmlService = {
            createHtmlOutputFromFile: makeHtmlOutput,
            createHtmlOutput: function() { return makeHtmlOutput(); },
            // The page's scriptlets run in the iframe that shows it, with the properties the
            // add-on set on the template as their variables, so evaluate() takes a JSON copy of
            // those properties:
            createTemplateFromFile: function(name) {
                const t = {
                    evaluate: function() {
                        const values = {};
                        for (const k of Object.keys(t)) {
                            if (typeof t[k] !== 'function') values[k] = t[k];
                        }
                        return makeHtmlOutput(name, JSON.parse(JSON.stringify(values)));
                    }
                };
                return t;
            },
            SandboxMode: { IFRAME: 'IFRAME', NATIVE: 'NATIVE' }
        };

        const charsetValues = { UTF_8: 'UTF-8', US_ASCII: 'US-ASCII' };
        // As in GAS, a string is encoded as US-ASCII unless UTF_8 is asked for, with "?" for each
        // code point that US-ASCII does not have:
        function stringBytes(text, charset) {
            const bytes = [];
            for (const character of String(text)) {
                const c = character.codePointAt(0);
                if (c < 0x80) {
                    bytes.push(c);
                } else if (charset !== charsetValues.UTF_8) {
                    bytes.push(0x3f);
                } else if (c < 0x800) {
                    bytes.push(0xc0 | c >> 6, 0x80 | c & 0x3f);
                } else if (c < 0x10000) {
                    bytes.push(0xe0 | c >> 12, 0x80 | c >> 6 & 0x3f, 0x80 | c & 0x3f);
                } else {
                    bytes.push(
                        0xf0 | c >> 18, 0x80 | c >> 12 & 0x3f, 0x80 | c >> 6 & 0x3f,
                        0x80 | c & 0x3f);
                }
            }
            return bytes;
        }
        // GAS rejects a charset argument that is passed but null or undefined:
        function checkCharset(args) {
            if (args.length > 1 && args[1] == null) {
                throw new Error('Argument cannot be null: charset');
            }
        }
        globalThis.Utilities = {
            base64Encode: function(data, charset) {
                checkCharset(arguments);
                const bytes = typeof data === 'string' ? stringBytes(data, charset) : data;
                return Uint8Array.from(bytes).toBase64();
            },
            // Like GAS, this returns signed bytes and ignores white space and any missing or extra
            // padding:
            base64Decode: function(encoded) {
                checkCharset(arguments);
                const text = String(encoded).replace(/[\s=]/g, '');
                if (!/^[A-Za-z0-9+/]*$/.test(text) || text.length % 4 === 1) {
                    throw new Error('Could not decode string.');
                }
                return Array.from(Uint8Array.fromBase64(text), b => b > 127 ? b - 256 : b);
            },
            getUuid: function() {
                return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
                    const r = Math.floor(Math.random() * 16);
                    return (c === 'x' ? r : (r & 0x3) | 0x8).toString(16);
                });
            },
            newBlob: cool.newBlob.bind(cool),
            Charset: charsetValues
        };

        // Round-tripped over the XClientRuntime proxy so the store lives in the iframe's
        // localStorage:
        function userPropsFacade() {
            const f = {
                getProperty: function(k) {
                    return clientRuntime.userPropGetProperty(String(k));
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
                    for (let i = 0; i < keys.length; ++i) {
                        const value = clientRuntime.userPropGetProperty(keys[i]);
                        if (value !== null) out[keys[i]] = value;
                    }
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

        globalThis.Logger = {
            log: function() { console.log.apply(console, arguments); }
        };

        globalThis.Session = {
            getActiveUser: function() {
                return { getEmail: function() { return ''; } };
            },
            getActiveUserLocale: cool.getActiveUserLocale.bind(cool)
        };

        // A GAS enum value is an object of its own that prints as its name and has the name(),
        // ordinal() and compareTo() of a Java enum:
        function gasEnum(names) {
            const values = {};
            names.forEach(function(name, ordinal) {
                values[name] = Object.freeze({
                    toString: function() { return name; },
                    toJSON: function() { return name; },
                    name: function() { return name; },
                    ordinal: function() { return ordinal; },
                    compareTo: function(other) { return ordinal - other.ordinal(); }
                });
            });
            return Object.freeze(values);
        }
        function scriptAppNotSupported(name) {
            return function() {
                throw new Error(
                    'ScriptApp.' + name + ' is not supported in the COOL Apps Script wrapper.');
            };
        }
        globalThis.ScriptApp = {
            AuthMode: gasEnum(['NONE', 'CUSTOM_FUNCTION', 'LIMITED', 'FULL']),
            getScriptId: function() { return extensionId; },
            getOAuthToken: scriptAppNotSupported('getOAuthToken'),
            getIdentityToken: scriptAppNotSupported('getIdentityToken'),
            getService: scriptAppNotSupported('getService'),
            getInstallationSource: scriptAppNotSupported('getInstallationSource'),
            newTrigger: scriptAppNotSupported('newTrigger'),
            getProjectTriggers: scriptAppNotSupported('getProjectTriggers'),
            getUserTriggers: scriptAppNotSupported('getUserTriggers'),
            deleteTrigger: scriptAppNotSupported('deleteTrigger'),
            requireAllScopes: scriptAppNotSupported('requireAllScopes'),
            getAuthorizationInfo: scriptAppNotSupported('getAuthorizationInfo'),
            invalidateAuth: scriptAppNotSupported('invalidateAuth')
        };
        // A cache for one scope, whose entries live in the iframe's localStorage and which checks
        // its arguments the way GAS does:
        function cacheFacade(scope) {
            function normalizedTtl(ttl) {
                const n = Math.trunc(Number(ttl));
                if (!Number.isFinite(n)) {
                    throw new Error("Cannot convert '" + ttl + "' to int.");
                }
                return n <= 0 ? 600 : Math.min(n, 21600);
            }
            function checkedKey(k) {
                const key = String(k);
                if (key.length > 250) {
                    throw new Error('Argument too large: key');
                }
                return key;
            }
            const c = {
                get: function(k) {
                    return clientRuntime.cacheGet(scope, String(k));
                },
                // As in GAS, a null value leaves no entry:
                put: function(k, v, ttl) {
                    const key = checkedKey(k);
                    const expiration = ttl === undefined ? 600 : normalizedTtl(ttl);
                    if (v == null) {
                        c.remove(key);
                        return;
                    }
                    const value = String(v);
                    if (value.length > 100 * 1024) {
                        throw new Error('Argument too large: value');
                    }
                    clientRuntime.cachePut(scope, key, value, expiration);
                },
                remove: function(k) {
                    clientRuntime.cacheRemove(scope, String(k));
                },
                getAll: function(keys) {
                    const out = {};
                    for (const k of keys) {
                        const v = c.get(k);
                        if (v !== null) out[k] = v;
                    }
                    return out;
                },
                putAll: function(values, ttl) {
                    for (const k of Object.keys(values)) {
                        c.put(k, values[k], ttl);
                    }
                },
                removeAll: function(keys) {
                    for (const k of keys) {
                        c.remove(k);
                    }
                }
            };
            return c;
        }
        globalThis.CacheService = {
            getUserCache: function() { return cacheFacade('user'); },
            getScriptCache: function() { return cacheFacade('script'); },
            getDocumentCache: function() {
                throw new Error(
                    'CacheService.getDocumentCache is not supported in the COOL Apps Script'
                        + ' wrapper; a per-document cache would need a document-side key that is'
                        + ' not plumbed through the iframe yet.');
            }
        };
        // Real UrlFetchApp, routed to the iframe (kit has no outbound network).  The response
        // object matches Apps Script's HTTPResponse shape: getContentText returns UTF-8 text,
        // getContent returns a byte array, and getHeaders and getAllHeaders return the headers
        // under the lower-case names the iframe's fetch reports.  When the underlying fetch or the
        // callback itself fails, urlFetch fills `error` and this shim throws with that message so
        // an add-on's try/catch runs.  A non-2xx status without muteHttpExceptions:true also
        // throws, matching GAS.
        // A byte payload (an array of GAS's signed bytes, a Uint8Array, or a Blob) travels to the
        // iframe as base64.  As in GAS, a payload without a contentType (and, for a Blob, without
        // a content type of its own) is sent as application/x-www-form-urlencoded:
        function urlFetchNormalize(payload, givenContentType) {
            const contentType = givenContentType || 'application/x-www-form-urlencoded';
            if (payload == null) {
                return { body: '', isBase64: false, contentType: givenContentType };
            }
            if (typeof payload === 'string') {
                return { body: payload, isBase64: false, contentType: contentType };
            }
            if (payload instanceof Uint8Array || Array.isArray(payload)) {
                return {
                    body: Uint8Array.from(payload).toBase64(), isBase64: true,
                    contentType: contentType
                };
            }
            if (typeof payload.getBytes === 'function') {
                const blobType = payload.getContentType();
                return {
                    body: Uint8Array.from(payload.getBytes()).toBase64(), isBase64: true,
                    contentType: givenContentType || blobType || contentType
                };
            }
            if (typeof payload === 'object') {
                // GAS form-encodes an object payload, with a space as "+":
                const encode = (v) => encodeURIComponent(v).replace(/%20/g, '+');
                const parts = [];
                for (const k of Object.keys(payload)) {
                    parts.push(encode(k) + '=' + encode(String(payload[k])));
                }
                return { body: parts.join('&'), isBase64: false, contentType: contentType };
            }
            return { body: String(payload), isBase64: false, contentType: contentType };
        }
        // Decode UTF-8 the way the Encoding Standard's TextDecoder does, without a leading byte
        // order mark and with one U+FFFD for each maximal subpart of an ill-formed sequence:
        function decodeUtf8(bytes) {
            const codePoints = [];
            let i = bytes[0] === 0xef && bytes[1] === 0xbb && bytes[2] === 0xbf ? 3 : 0;
            while (i < bytes.length) {
                const lead = bytes[i];
                let codePoint = 0xfffd;
                let length = 1;
                let need = 0;
                let lower = 0x80;
                let upper = 0xbf;
                if (lead < 0x80) {
                    codePoint = lead;
                } else if (lead >= 0xc2 && lead <= 0xdf) {
                    need = 1;
                    codePoint = lead & 0x1f;
                } else if (lead >= 0xe0 && lead <= 0xef) {
                    need = 2;
                    codePoint = lead & 0x0f;
                    lower = lead === 0xe0 ? 0xa0 : 0x80;
                    upper = lead === 0xed ? 0x9f : 0xbf;
                } else if (lead >= 0xf0 && lead <= 0xf4) {
                    need = 3;
                    codePoint = lead & 0x07;
                    lower = lead === 0xf0 ? 0x90 : 0x80;
                    upper = lead === 0xf4 ? 0x8f : 0xbf;
                }
                for (let j = 1; j <= need; ++j) {
                    const next = bytes[i + j];
                    if (next === undefined || next < lower || next > upper) {
                        codePoint = 0xfffd;
                        length = j;
                        break;
                    }
                    codePoint = (codePoint << 6) | (next & 0x3f);
                    lower = 0x80;
                    upper = 0xbf;
                    length = j + 1;
                }
                codePoints.push(codePoint);
                i += length;
            }
            let text = '';
            for (let k = 0; k < codePoints.length; k += 0x8000) {
                text += String.fromCodePoint.apply(null, codePoints.slice(k, k + 0x8000));
            }
            return text;
        }
        // The body arrives as base64, and is decoded only when the add-on asks for it:
        function urlFetchResponse(r) {
            let bytes = null;
            let text = null;
            const content = function() {
                if (bytes === null) {
                    bytes = Uint8Array.fromBase64(r.body);
                }
                return bytes;
            };
            const headerMap = {};
            for (let i = 0; i < r.headerNames.length; ++i) {
                headerMap[r.headerNames[i]] = r.headerValues[i];
            }
            return {
                getResponseCode: function() { return r.code; },
                getContentText: function(charset) {
                    if (charset && charset.toLowerCase() !== 'utf-8'
                        && charset.toLowerCase() !== 'utf8') {
                        throw new Error(
                            'HTTPResponse.getContentText: charset override to ' + charset
                                + ' is not supported; only the response\'s own charset is'
                                + ' available.');
                    }
                    if (text === null) {
                        text = decodeUtf8(content());
                    }
                    return text;
                },
                // As in GAS, the bytes are signed:
                getContent: function() {
                    return Array.from(content(), (b) => b > 127 ? b - 256 : b);
                },
                getHeaders: function() { return headerMap; },
                // GAS gives the values of a repeated header as an array, but the iframe's fetch
                // has already joined them into one string, separated by ", ":
                getAllHeaders: function() { return headerMap; },
                // GAS gives the blob the response's media type without its parameters:
                getBlob: function() {
                    const contentType = headerMap['content-type'];
                    return cool.newBlob(
                        this.getContent(),
                        contentType === undefined ? null : contentType.split(';')[0].trim());
                }
            };
        }
        function urlFetchOne(url, params) {
            params = params || {};
            const method = String(params.method || 'get').toUpperCase();
            const norm = urlFetchNormalize(params.payload, params.contentType || '');
            const headerNames = [];
            const headerValues = [];
            const headers = params.headers || {};
            for (const k of Object.keys(headers)) {
                headerNames.push(String(k));
                headerValues.push(String(headers[k]));
            }
            const followRedirects = params.followRedirects !== false;
            const resp = clientRuntime.urlFetch(
                String(url), method, norm.contentType, norm.body, norm.isBase64,
                headerNames, headerValues, followRedirects);
            if (resp.error) {
                throw new Error(
                    'UrlFetchApp.fetch ' + url + ': ' + resp.error);
            }
            // GAS names only the scheme and host of the URL here:
            if (!params.muteHttpExceptions && resp.code >= 400) {
                const origin = String(url).match(/^[^:/?#]+:\/\/[^/?#]*/);
                throw new Error(
                    'Request failed for ' + (origin === null ? url : origin[0])
                        + ' returned code ' + resp.code);
            }
            return urlFetchResponse(resp);
        }
        globalThis.UrlFetchApp = {
            fetch: urlFetchOne,
            fetchAll: function(requests) {
                return requests.map(function(r) {
                    if (typeof r === 'string') return urlFetchOne(r, {});
                    return urlFetchOne(r.url, r);
                });
            },
            getRequest: function(url, params) {
                params = params || {};
                const norm = urlFetchNormalize(params.payload, params.contentType || '');
                return {
                    url: String(url),
                    method: String(params.method || 'get').toLowerCase(),
                    contentType: norm.contentType,
                    payload: norm.isBase64 ? params.payload : norm.body,
                    headers: params.headers || {}
                };
            }
        };

        // As in GAS, a library runs in a scope of its own, together with the libraries that it
        // declares in turn, and exposes its top-level functions and variables whose names do not
        // end in "_":
        function loadLibrary(library) {
            const names = [];
            const declaration
                = /^(?:function\s*\*?\s*|class\s+|(?:var|let|const)\s+)([A-Za-z_$][\w$]*)/gm;
            for (const source of library.sources) {
                for (const m of source.matchAll(declaration)) {
                    if (!m[1].endsWith('_') && !names.includes(m[1])) {
                        names.push(m[1]);
                    }
                }
            }
            const code = '(function(' + library.libraries.map(l => l.userSymbol).join(', ')
                + ') {\n' + library.sources.join('\n') + '\nreturn Object.freeze({'
                + names.map(n => n + ': ' + n).join(', ') + '});\n})';
            return $internal.evalWithSource(code, library.userSymbol + ' library', 0).apply(
                null, library.libraries.map(loadLibrary));
        }
        for (const library of libraries) {
            globalThis[library.userSymbol] = loadLibrary(library);
        }
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
            // The runner runs everything the add-on asks for, so onOpen gets the FULL authorization
            // mode:
            if (typeof globalThis.onOpen === 'function') {
                globalThis.onOpen({ authMode: globalThis.ScriptApp.AuthMode.FULL });
            }
            value = menuItems;
        } else {
            const fn = globalThis[fnName];
            if (typeof fn !== 'function') {
                throw new Error('Apps Script function not defined: ' + fnName);
            }
            value = fn.apply(null, callArgs || []);
        }
        // A result marked __coolGas holds the add-on function's own return value in value,
        // every message it passed to getUi().alert() in alerts, and (if any) the sidebar file
        // it asked us to show through ui.showSidebar in sidebarFile and the dialog it asked us
        // to show through one of the ui.show*Dialog calls in dialog:
        return {
            __coolGas: true, value: value, alerts: pendingAlerts, sidebarFile: showSidebarFile,
            dialog: showDialogSpec
        };
    } finally {
        $internal.takeProxy(proxyId);
    }
};
