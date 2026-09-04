// SPDX-License-Identifier: MPL-2.0
//
// Helpers for COOL extension iframes.  An extension's HTML page loads this script and uses the
// helpers to talk to COOL via postMessage.

(function () {
	window.cool = window.cool || {};

	// --- Localization -------------------------------------------------------------------------
	//
	// COOL opens extension pages with ?lang=<UI language>&dir=<ltr|rtl>. The extension's
	// translations live in l10n/<lang>.json under its own directory (built from po/<lang>.po,
	// see README.md); cool.js loads the catalog for the UI language and offers gettext-style
	// helpers, so a page can write English and stay translatable:
	//
	//   cool.lang                 "de", "pt-BR", ... (never empty; defaults to en-US)
	//   cool.dir                  "ltr" or "rtl"
	//   cool.l10n.ready           Promise resolved once the catalog is loaded (or found missing)
	//   cool._(msgid)             translation of msgid, or msgid itself
	//   cool._n(one, many, n)     plural form for n, using the catalog's Plural-Forms rule
	//   <span data-l10n="Save">Save</span>     translated on load; also data-l10n-title,
	//                                          data-l10n-placeholder, data-l10n-aria-label
	//   cool.l10n.apply(element)  translate data-l10n attributes under element (added later)
	const pageParams = new URLSearchParams(window.location.search);
	window.cool.lang = pageParams.get('lang') || 'en-US';
	window.cool.dir = pageParams.get('dir') === 'rtl' ? 'rtl' : 'ltr';
	let catalog = null;
	// po2json.py joins msgid and msgid_plural, and the plural forms, with NUL.
	const NUL = "\u0000";

	// The extension's own directory: everything up to and including /extensions/<id>/ in
	// this page's URL, whether built in or shipped as a preset.
	function extensionBase() {
		const m = /^(.*\/extensions\/[^/]+\/)/.exec(window.location.href);
		return m ? m[1] : new URL('./', window.location.href).href;
	}

	function catalogCandidates(lang) {
		const norm = lang.replace(/-/g, '_');
		const parts = norm.split('_');
		if (parts[0] === 'en') return [];
		const out = [norm];
		if (parts.length > 1) out.push(parts[0]);
		return out;
	}

	async function loadCatalog() {
		for (const cand of catalogCandidates(window.cool.lang)) {
			try {
				const resp = await fetch(extensionBase() + 'l10n/' + cand + '.json');
				if (!resp.ok) continue;
				const obj = await resp.json();
				if (obj && typeof obj === 'object') return obj;
			} catch (err) {
				// try the next candidate
			}
		}
		return null;
	}

	window.cool._ = function (msgid) {
		if (catalog && Object.prototype.hasOwnProperty.call(catalog, msgid) && catalog[msgid] !== '') {
			return catalog[msgid];
		}
		return msgid;
	};

	let pluralRule = null;
	function pluralIndex(n) {
		if (pluralRule === null) {
			pluralRule = false;
			const rule = catalog && catalog[NUL + 'plural-forms'];
			const m = rule && /plural\s*=\s*([^;]+)/.exec(rule);
			if (m) {
				try {
					// The catalog is the extension's own data, not user input.
					pluralRule = new Function('n', 'return Number(' + m[1] + ');');
				} catch (err) {
					pluralRule = false;
				}
			}
		}
		if (pluralRule) return pluralRule(n);
		return n === 1 ? 0 : 1;
	}

	window.cool._n = function (one, many, n) {
		const key = one + NUL + many;
		if (catalog && Object.prototype.hasOwnProperty.call(catalog, key)) {
			const forms = catalog[key].split(NUL);
			const form = forms[Math.min(pluralIndex(n), forms.length - 1)];
			if (form) return form;
		}
		return n === 1 ? one : many;
	};

	function applyL10n(root) {
		const scope = root || document;
		const set = (attr, apply) => {
			const nodes = scope.querySelectorAll ? scope.querySelectorAll('[' + attr + ']') : [];
			for (const el of nodes) apply(el, window.cool._(el.getAttribute(attr)));
			if (scope.getAttribute && scope.hasAttribute(attr)) apply(scope, window.cool._(scope.getAttribute(attr)));
		};
		set('data-l10n', (el, text) => { el.textContent = text; });
		set('data-l10n-title', (el, text) => { el.title = text; });
		set('data-l10n-placeholder', (el, text) => { el.placeholder = text; });
		set('data-l10n-aria-label', (el, text) => { el.setAttribute('aria-label', text); });
	}

	window.cool.l10n = {
		apply: applyL10n,
		ready: (async () => {
			document.documentElement.lang = window.cool.lang;
			document.documentElement.dir = window.cool.dir;
			catalog = await loadCatalog();
			if (document.readyState === 'loading') {
				await new Promise((resolve) => document.addEventListener('DOMContentLoaded', resolve, { once: true }));
			}
			if (catalog) applyL10n(document);
			return catalog !== null;
		})(),
	};

	let nextCallId = 0;
	const pending = Object.create(null);

	let nextProxyId = 0;
	// Mapping from proxyId to { on: { method: handler, ... }, detach } objects:
	const listeners = Object.create(null);

	let nextDialogId = 0;
	// Pending cool.dialog.open() calls, keyed by dialogId, each holding the
	// promise's { resolve } so Extension_DialogResult can settle it:
	const pendingDialogs = Object.create(null);

	function attachTrampoline(typeName, proxyId, fnSrc, facade) {
		$internal.suppressLegacyUnoApiStart();
		let ended = false;
		try {
			const parts = typeName.split('.');
			let type = uno.idl;
			for (const p of parts) type = type[p];
			if (!facade) {
				$internal.suppressLegacyUnoApiEnd();
				ended = true;
			}
			const proxy = $internal.createProxy(type, proxyId);
			(0, eval)('(' + fnSrc + ')')(proxy);
		} finally {
			if (!ended) $internal.suppressLegacyUnoApiEnd();
		}
	}
	function detachTrampoline(proxyId, fnSrc, facade) {
		if (facade) $internal.suppressLegacyUnoApiStart();
		try {
			const proxy = $internal.takeProxy(proxyId);
			(0, eval)('(' + fnSrc + ')')(proxy);
		} finally {
			if (facade) $internal.suppressLegacyUnoApiEnd();
		}
	}

	// Best-effort parse of `new Error().stack` to locate the first frame that isn't inside
	// cool.js itself; handles the two common stack shapes (Chrome/Edge: `at foo (URL:L:C)`;
	// Firefox/Safari: `foo@URL:L:C`):
	function detectCallerLocation() {
		let stackText;
		try { throw new Error(); } catch (e) { stackText = e.stack || ''; }
		const frameRe = /(?:at\s+[^(\n]*\(([^)]+?):(\d+):(\d+)\))|(?:(?:^|@)([^\s@]+?):(\d+):(\d+))/;
		for (const line of stackText.split('\n')) {
			const m = frameRe.exec(line);
			if (!m) continue;
			const url = m[1] || m[4];
			if (!url) continue;
			if (/(?:^|\/)cool\.js(?:[:?]|$)/.test(url)) continue;
			return { source: url, line: parseInt(m[2] || m[5], 10) };
		}
		return null;
	}

	// Takes a function `fn` followed by arguments `args`.  Runs `fn.apply(null, args)` inside a
	// JS UNO context in the document's kit process and returns a Promise that resolves to the
	// function application's JSON-decoded return value, or rejects with an Error whose `.name`,
	// `.message` and `.stack` reflect the JS engine's own error.
	//
	// The first argument may also be an `{fn, source, line}` object, giving an explicit source
	// name and 1-based start line reported back in stack traces.
	//
	// The function `fn` is shipped as source text via `fn.toString()` and evaluated on the
	// server side, so it must be self-contained: it cannot close over variables in the iframe's
	// scope, but it can use the JS UNO `uno` global and its own arguments and locals.  The
	// `args` must be JSON-serializable.
	window.cool.callRemote = function (fnOrOpts) {
		const args = Array.prototype.slice.call(arguments, 1);
		const callId = String(nextCallId++);
		const promise = new Promise(function (resolve, reject) {
			pending[callId] = { resolve: resolve, reject: reject };
		});
		let fn, source, line;
		if (typeof fnOrOpts === 'function') {
			fn = fnOrOpts;
			const loc = detectCallerLocation();
			if (loc) {
				source = loc.source;
				line = loc.line;
			} else {
				source = '<input>';
				line = 1;
			}
		} else {
			fn = fnOrOpts.fn;
			source = fnOrOpts.source;
			line = fnOrOpts.line;
		}
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_Call',
			callId: callId,
			fn: fn.toString(),
			source,
			line,
			args: args
		}), '*');
		return promise;
	};

	// Reconstruct an Error from the engine's jsuno::Exception payload:
	function makeStructuredError(payload) {
		const frames = payload.stack || [];
		const err = new Error(payload.message || '');
		err.name = payload.name || 'Error';
		let stackText = err.name + ': ' + err.message;
		for (const f of frames) {
			stackText += '\n    at ' + (f.functionName || '<anonymous>')
				+ ' (' + (f.source || '') + ':' + f.line + ':' + f.column + ')';
		}
		err.stack = stackText;
		err.toString = function () { return this.name + ': ' + this.message; };
		return err;
	}

	// Bundle a UNO listener registration with its iframe-side per-method handlers.
	//
	// `typeName` is the UNO interface name as a dotted string, e.g.
	// "com.sun.star.document.XDocumentEventListener".
	//
	// `spec.attach(proxy)` runs in the kit's JS UNO context with a freshly-created proxy
	// already bound to that interface; the implementation just wires the proxy into the
	// relevant UNO API (e.g. `model.addDocumentEventListener(proxy)`).  `spec.detach(proxy)`
	// gets the same proxy back and must do the matching unwire (e.g.
	// `model.removeDocumentEventListener(proxy)`); skipping it would leak the kit-side
	// listener.  Both run as cool.callRemote scripts.
	//
	// `spec.on` is an object mapping UNO method names to handler functions; handlers receive
	// the method's UNO arguments and may return a value (or a Promise of one) to be sent back
	// as the listener method's return value (only relevant for non-void methods).
	//
	// Returns `{ready, detach}`: `ready` resolves after the kit-side attach script has run;
	// `detach()` runs `spec.detach` on the kit as the matching callRemote.
	function attachListenerImpl(typeName, spec, facade) {
		const proxyId = 'p' + (nextProxyId++);
		listeners[proxyId] = {
			on: spec.on,
			detach: spec.detach,
			facade: facade
		};
		const ready = window.cool.callRemote(
			attachTrampoline, typeName, proxyId, spec.attach.toString(), facade);
		return {
			ready,
			detach: function () {
				const entry = listeners[proxyId];
				if (!entry) return;
				delete listeners[proxyId];
				window.cool.callRemote(
					detachTrampoline, proxyId, entry.detach.toString(), entry.facade);
			}
		};
	}
	window.cool.attachListener = function (typeName, spec) {
		return attachListenerImpl(typeName, spec, false);
	};

	// Register a handler map under a caller-chosen proxyId; pairs with a kit-side
	// `$internal.createProxy(type, proxyId)` inside one callRemote body whose runner takes
	// the proxy back before returning:
	window.cool.registerProxy = function (proxyId, handlers) {
		listeners[proxyId] = { on: handlers, detach: null, facade: false };
	};
	window.cool.unregisterProxy = function (proxyId) {
		delete listeners[proxyId];
	};

	// Open a modal dialog whose content is a page under the extension's own base URL, and
	// return a Promise that resolves when the dialog closes.  The resolved value is
	// { cancelled: true } if the user dismissed the dialog (titlebar close, Esc,
	// cool.dialog.cancel()), otherwise { cancelled: false,
	// value: <whatever cool.dialog.close(value) received> in the dialog iframe }.  Only one
	// dialog is allowed at a time per extension; a second call while one is open resolves
	// immediately as cancelled.
	//
	// The dialog iframe loads its own copy of this cool.js and uses cool.dialog.close/
	// cool.dialog.cancel to dismiss itself.  It can also use cool.callRemote etc.
	//
	// opts: { url, title, width, height }.  url is relative to the extension's base URL.
	// cool.onCommand = function (id) {...}: receives `panel: true` commands
	// from the notebookbar, menus and shortcuts. Commands posted before the
	// handler exists are queued and delivered on assignment.
	let commandHandler = null;
	const pendingCommands = [];
	function deliverCommand(id) {
		try {
			commandHandler(id);
		} catch (err) {
			console.warn('cool.onCommand threw:', err);
		}
	}
	Object.defineProperty(window.cool, 'onCommand', {
		enumerable: true,
		get: function () { return commandHandler; },
		set: function (fn) {
			commandHandler = (typeof fn === 'function') ? fn : null;
			if (!commandHandler) return;
			while (pendingCommands.length) deliverCommand(pendingCommands.shift());
		}
	});
	// Save a file the extension generated (e.g. an exported SVG). COOL saves it
	// the platform way: a download or a save to the WOPI host in Online, a
	// filesystem save in the desktop app. `content` is a string (text files) or
	// a Uint8Array / number[] of bytes; `mimeType` defaults to
	// application/octet-stream. Resolves with how it was saved ("download",
	// "host", "filesystem") or rejects if it could not be saved.
	let nextSaveId = 0;
	const pendingSaves = Object.create(null);
	window.cool.saveFile = function (filename, content, mimeType) {
		const saveId = 's' + (nextSaveId++);
		let bytes;
		if (typeof content === 'string') {
			bytes = Array.from(new TextEncoder().encode(content));
		} else if (content instanceof Uint8Array) {
			bytes = Array.from(content);
		} else if (Array.isArray(content)) {
			bytes = content;
		} else {
			return Promise.reject(new Error('saveFile: content must be a string or bytes'));
		}
		const promise = new Promise(function (resolve, reject) {
			pendingSaves[saveId] = { resolve: resolve, reject: reject };
		});
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_SaveFile',
			saveId: saveId,
			filename: filename,
			mimeType: mimeType || 'application/octet-stream',
			bytes: bytes,
		}), '*');
		return promise;
	};

	window.cool.dialog = {
		open: function (opts) {
			const dialogId = 'd' + (nextDialogId++);
			const promise = new Promise(function (resolve) {
				pendingDialogs[dialogId] = { resolve: resolve };
			});
			window.parent.postMessage(JSON.stringify({
				msgId: 'Extension_ShowDialog',
				dialogId: dialogId,
				url: opts.url,
				title: opts.title,
				width: opts.width,
				height: opts.height,
			}), '*');
			return promise;
		},
		close: function (value) {
			window.parent.postMessage(JSON.stringify({
				msgId: 'Extension_DialogClose',
				value: value === undefined ? null : value,
			}), '*');
		},
		cancel: function () {
			window.parent.postMessage(JSON.stringify({
				msgId: 'Extension_DialogCancel',
			}), '*');
		},
	};

	// Define a property on cool.document that lazily wires a UNO listener of `typeName` to the
	// attach/detach scripts when set to a function, and unwires it when set back to null.  The
	// listener interface's relevant method name `methodName` is delivered to the iframe-side
	// handler stored in `current`, which is whatever the property currently holds.  Inherited
	// XEventListener.disposing has no handler installed; the dispatcher's default null
	// response is fine for a void return type.
	function defineUnoDocEvent(propName, typeName, attachFn, detachFn, methodName) {
		let handle = null;
		let current = null;
		const on = {};
		on[methodName] = function (event) {
			if (current) current(event);
		};
		Object.defineProperty(window.cool.document, propName, {
			enumerable: true,
			get: function () { return current; },
			set: function (fn) {
				current = (typeof fn === 'function') ? fn : null;
				if (current && !handle) {
					handle = attachListenerImpl(typeName, {
						attach: attachFn,
						detach: detachFn,
						on: on
					}, true);
				} else if (!current && handle) {
					handle.detach();
					handle = null;
				}
			}
		});
	}

	// High-level facade for document-level events.  Extension authors write
	// `cool.document.onCommentAdded = function (comment) {...}` etc.; setting the property to a
	// function subscribes, setting it back to null unsubscribes.
	window.cool.document = {
		onCommentAdded: null,
		onCommentChanged: null,
		onCommentRemoved: null
	};
	defineUnoDocEvent(
		'onSelectionChanged',
		'com.sun.star.view.XSelectionChangeListener',
		function (proxy) {
			const desktop
				= uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
			desktop.getCurrentFrame().getController().addSelectionChangeListener(proxy);
		},
		function (proxy) {
			const desktop
				= uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
			desktop.getCurrentFrame().getController()
				.removeSelectionChangeListener(proxy);
		},
		'selectionChanged');
	defineUnoDocEvent(
		'onModified',
		'com.sun.star.util.XModifyListener',
		function (proxy) {
			const desktop
				= uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
			desktop.getCurrentFrame().getController().getModel().addModifyListener(proxy);
		},
		function (proxy) {
			const desktop
				= uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
			desktop.getCurrentFrame().getController().getModel()
				.removeModifyListener(proxy);
		},
		'modified');

	// Detach every listener still registered with cool.attachListener, so closing the iframe
	// doesn't leave orphaned proxies on the kit side:
	function detachAll() {
		for (const proxyId of Object.keys(listeners)) {
			const entry = listeners[proxyId];
			delete listeners[proxyId];
			window.cool.callRemote(
				detachTrampoline, proxyId, entry.detach.toString(), entry.facade);
		}
	}

	function deliverProxyResult(callId, value) {
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_ProxyReturn',
			callId: callId,
			value: value === undefined ? null : value
		}), '*');
	}

	window.addEventListener('message', function (e) {
		if (e.source !== window.parent) return;
		let data;
		try {
			data = JSON.parse(e.data);
		} catch (ex) {
			console.warn('postMessage is not JSON: ' + e.data);
			return;
		}
		if (!data || typeof data !== 'object') {
			console.warn('postMessage is not an object: ' + JSON.stringify(data));
			return;
		}
		if (data.msgId === 'Extension_CallResult') {
			const entry = pending[data.callId];
			if (!entry) {
				console.warn('Extension_CallResult: unknown callId ' + data.callId);
				return;
			}
			delete pending[data.callId];
			if (data.err !== undefined) {
				let err;
				if (typeof data.err === 'object' && data.err !== null) {
					err = makeStructuredError(data.err)
					console.error(err.stack);
				} else {
					err = new Error(data.err);
				}
				entry.reject(err);
			} else {
				entry.resolve(data.ok);
			}
		} else if (data.msgId === 'Extension_ProxyCall') {
			const entry = listeners[data.proxyId];
			const fn = entry && entry.on[data.method];
			const result = fn
				? Promise.resolve(fn.apply(null, data.args || []))
				: Promise.resolve(null);
			if (data.callId !== undefined && data.callId !== null) {
				result.then(
					function (value) {
						deliverProxyResult(data.callId, value);
					},
					function (err) {
						console.warn('proxy handler threw:', err);
						deliverProxyResult(data.callId, null);
					}
				);
			}
		} else if (data.msgId === 'Extension_DocumentEvent') {
			// Map the event name (e.g. "commentAdded") to the corresponding cool.document
			// handler slot (onCommentAdded) and call it if the extension installed one:
			const handlerName
				= 'on' + data.name.charAt(0).toUpperCase() + data.name.slice(1);
			const fn = window.cool.document[handlerName];
			if (typeof fn === 'function') {
				try {
					fn(data.payload);
				} catch (err) {
					console.warn('cool.document.' + handlerName + ' threw:', err);
				}
			}
		} else if (data.msgId === 'Extension_Command') {
			// A `panel: true` command from the notebookbar, a menu or a
			// shortcut: the extension handles it with cool.onCommand(id).
			// The command that opened the panel can arrive before the page's
			// scripts have assigned the handler; keep it until they do.
			if (typeof commandHandler === 'function') {
				deliverCommand(data.commandId);
			} else {
				pendingCommands.push(data.commandId);
			}
		} else if (data.msgId === 'Extension_SaveFileResult') {
			const entry = pendingSaves[data.saveId];
			if (!entry) return;
			delete pendingSaves[data.saveId];
			if (data.err !== undefined) entry.reject(new Error(data.err));
			else entry.resolve(data.how);
		} else if (data.msgId === 'Extension_DialogResult') {
			const entry = pendingDialogs[data.dialogId];
			if (!entry) {
				console.warn('Extension_DialogResult: unknown dialogId ' + data.dialogId);
				return;
			}
			delete pendingDialogs[data.dialogId];
			entry.resolve(data.cancelled
				? { cancelled: true }
				: { cancelled: false, value: data.value });
		} else if (data.msgId === 'Extension_Teardown') {
			detachAll();
			window.parent.postMessage(JSON.stringify({
				msgId: 'Extension_TeardownDone'
			}), '*');
		} else {
			console.warn('postMessage has unexpected msgId: ' + data.msgId);
		}
	});

	// Best-effort cleanup when the iframe goes away without a Teardown handshake:
	window.addEventListener('pagehide', function () {
		detachAll();
	});

	// Tell the parent how tall our content actually is so it can size the
	// hosting iframe element to the document body's offsetHeight (i.e. fit
	// without scrollbars inside the iframe).  Uses ResizeObserver if
	// available; falls back to a single send after load.  This works
	// regardless of origin because it's a postMessage, not direct DOM
	// access on the parent side.  Use body.offsetHeight rather than
	// documentElement.scrollHeight because the latter is clamped to at least the iframe's
	// viewport height and would prevent shrinking when the content is smaller than the current
	// iframe.
	function postHeight() {
		const h = document.body.offsetHeight;
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_Resize',
			height: h
		}), '*');
	}
	window.addEventListener('load', postHeight);
	if (typeof ResizeObserver !== 'undefined') {
		// This script may be loaded from <head>, before <body> exists.
		const observeBody = () => new ResizeObserver(postHeight).observe(document.body);
		if (document.body) observeBody();
		else document.addEventListener('DOMContentLoaded', observeBody, { once: true });
	}
})();
