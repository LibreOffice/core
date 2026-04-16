/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// In Node (mocha) there is no browser `window`. Many source files assign
// to `window.*` at top level (e.g. `window.app.a11yValidator = ...`).
// Aliasing `window` to `globalThis` makes those assignments land on the
// same object as bare `app`, `L`, etc., so the whole source tree can be
// loaded without a browser DOM.
(globalThis as any).window = globalThis;

// docstatefunctions.js calls `window.addEventListener('load', ...)` at
// module load time, before any test wires up jsdom. Provide a no-op so
// the call does not throw; tests that need real DOM events overwrite
// `global.window` later via initializeJSDOM.
(globalThis as any).addEventListener = () => {};

// gettext translation shim. In the browser `_` is provided by l10n.js
// (and `_` resolves to the identity on untranslated strings).
(globalThis as any)._ = (s: string) => s;

// Leaflet's `L` namespace is a script-loaded global in the browser. Source
// files run mixin calls (L.Map.include, L.Handler.extend, ...) at module
// load time, so we need enough of a stub for those to be no-ops. extend()
// returns an empty class so `L.Foo.Settings = L.Handler.extend({...})`
// evaluates to a usable constructor.
const noopClass: any = function () {};
noopClass.extend = () => noopClass;
noopClass.include = () => {};
noopClass.mergeOptions = () => {};
noopClass.addInitHook = () => {};

(globalThis as any).L = {
    LOUtil: {},
    Map: noopClass,
    Handler: noopClass,
    Control: noopClass,
    Class: noopClass,
    Layer: noopClass,
    Evented: noopClass,
    control: {},
};

// DOMPurify is loaded via <script> tag in the browser; source files call
// DOMPurify.addHook(...) at module load time. Stub it so source
// concatenation doesn't blow up under Node.
(globalThis as any).DOMPurify = {
    addHook() {},
    sanitize(s: string) { return s; },
};

(globalThis as any).app = {
    CSections: { Scroll: { name : 'scroll' } },
    roundedDpiScale : 1,
    canvasSize: null,
    definitions: {},
    dpiScale: 1,
    twipsToPixels: 1 / 15,
    pixelsToTwips: 15,
    sectionContainer: {},
    socket: {},
    // Loader-time stubs: source files touch these at module-load time.
    // In the browser, js/global.js + setLogging(true) populate app.console
    // with real console wrappers (assert, error, warn, ...). app.events is
    // a DocEvents instance with on/off/fire/emit. Mirror just enough of
    // that so source concatenation doesn't blow up under Node.
    console: console,
    events: { on() {}, off() {}, fire() {}, emit() {} },
    // docstatefunctions.js attaches functions to app.calc / app.impress at
    // module-load time (e.g. `app.calc.isRTL = function () {...}`), so the
    // sub-objects must already exist. In the browser these are created in
    // docstate.ts; we mirror the bare structure here.
    //
    // docstate.ts reassigns `window.app` to a brand-new object, but that
    // only updates the jsdom window's property — `globalThis.app` keeps
    // pointing at this stub. The `load` event listener registered in
    // docstatefunctions.js runs in Node scope where bare `app` resolves to
    // `globalThis.app`, so the stub also needs `file`, `tile`, `map`, and
    // `following` populated well enough for the load callback and the
    // RAF helpers (app.enterRAF / app.exitRAF) not to throw.
    calc: {},
    impress: {},
    file: {
        textCursor: {
            visible: false,
            rectangle: null,
        },
    },
    tile: {
        size: null,
    },
    // `app.enterRAF`/`app.exitRAF` in docstatefunctions.js dereference
    // `app.map._debug` without a null check on `map`, and SheetGeometry
    // calls `app.map.fire(...)` after guarding only on `app.map`. Give
    // `map` no-op event methods so neither site throws when RAF fires
    // during tests. Tests that exercise map behavior (ViewLayout) still
    // overwrite `app.map` with a richer fixture.
    map: { fire() {}, on() {}, off() {}, emit() {}, _debug: null },
    following: {
        mode: 'none',
        viewId: -1,
    },
};

globalThis.window = (function() {
	const jsdom = require('jsdom');
	const dom = new jsdom.JSDOM('<html><body><div id="document-container"></div></body></html>', { pretendToBeVisual: true });
	return dom.window;
})();

globalThis.document = globalThis.window.document;

(globalThis.window as any).prefs = {
	canPersist: false,
};

(globalThis.window as any).app = (globalThis as any).app;

(globalThis as any).L = {
	Browser: {
		any3d: true,
		cypressTest: false,
		mac: false,
		win: false,
		lang: 'en',
	},
	LOUtil: {},
	Map: {
		include(input: any) {},
		mergeOptions(input: any) {},
		addInitHook(i1: any, i2: any, i3: any) {},
	},
	Handler: {
		extend(input: any) {},
	},

	Control: class _Control {
	},

	control: {},
};

(globalThis.window as any).L = (globalThis as any).L;

globalThis._ = (input: string) => input;
(globalThis.ResizeObserver as any) = class _ResizeObserver {
	constructor(firer: () => void) {
	}
	observe(container: HTMLElement) {
	}
};
 globalThis.JSDialog = {};

(globalThis as any).DOMPurify = require('dompurify');
(globalThis as any).glMatrix = require('gl-matrix');

String.locale = 'en';
globalThis._UNO = function(i1: string, i2: string) {
	return i2;
};
globalThis.SlideShow = {};
(globalThis.window as any).getBorderStyleUNOCommand = () => {};

var nodeassert = require('assert').strict;
var jsdom = require('jsdom');
