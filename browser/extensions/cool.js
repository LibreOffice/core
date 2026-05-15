// SPDX-License-Identifier: MPL-2.0
//
// Helpers for COOL extension iframes.  An extension's HTML page loads this script and uses the
// helpers to talk to COOL via postMessage.

(function () {
	window.cool = window.cool || {};

	let nextCallId = 0;
	const pending = Object.create(null);

	let nextProxyId = 0;
	// Mapping from proxyId to { on: { method: handler, ... }, detach } objects:
	const listeners = Object.create(null);

	function attachTrampoline(typeName, proxyId, fnSrc) {
		const parts = typeName.split('.');
		let type = uno.idl;
		for (const p of parts) type = type[p];
		const proxy = $internal.createProxy(type, proxyId);
		(0, eval)('(' + fnSrc + ')')(proxy);
	}
	function detachTrampoline(proxyId, fnSrc) {
		const proxy = $internal.takeProxy(proxyId);
		(0, eval)('(' + fnSrc + ')')(proxy);
	}

	// Takes a function `fn` followed by arguments `args`.  Runs `fn.apply(null, args)` inside a
	// JS UNO context in the document's kit process and returns a Promise that resolves to the
	// function application's JSON-decoded return value, or rejects with an Error wrapping the
	// exception message.
	//
	// The function `fn` is shipped as source text via `fn.toString()` and evaluated on the
	// server side, so it must be self-contained: it cannot close over variables in the iframe's
	// scope, but it can use the JS UNO `uno` global and its own arguments and locals.  The
	// `args` must be JSON-serializable.
	window.cool.callRemote = function (fn) {
		const args = Array.prototype.slice.call(arguments, 1);
		const callId = String(nextCallId++);
		const promise = new Promise(function (resolve, reject) {
			pending[callId] = { resolve: resolve, reject: reject };
		});
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_Call',
			callId: callId,
			fn: fn.toString(),
			args: args
		}), '*');
		return promise;
	};

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
	// Returns a handle with a `.detach()` method.
	window.cool.attachListener = function (typeName, spec) {
		const proxyId = 'p' + (nextProxyId++);
		listeners[proxyId] = {
			on: spec.on,
			detach: spec.detach
		};
		window.cool.callRemote(attachTrampoline, typeName, proxyId, spec.attach.toString());
		return {
			detach: function () {
				const entry = listeners[proxyId];
				if (!entry) return;
				delete listeners[proxyId];
				window.cool.callRemote(
					detachTrampoline, proxyId, entry.detach.toString());
			}
		};
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
					handle = window.cool.attachListener(typeName, {
						attach: attachFn,
						detach: detachFn,
						on: on
					});
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
				detachTrampoline, proxyId, entry.detach.toString());
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
				//TODO: reconstruct specific (JS and UNO) exception types?
				entry.reject(new Error(data.err));
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
	// hosting iframe element to the document body's scrollHeight (i.e. fit
	// without scrollbars inside the iframe).  Uses ResizeObserver if
	// available; falls back to a single send after load.  This works
	// regardless of origin because it's a postMessage, not direct DOM
	// access on the parent side.
	function postHeight() {
		const h = document.documentElement.scrollHeight;
		window.parent.postMessage(JSON.stringify({
			msgId: 'Extension_Resize',
			height: h
		}), '*');
	}
	window.addEventListener('load', postHeight);
	if (typeof ResizeObserver !== 'undefined') {
		new ResizeObserver(postHeight).observe(document.documentElement);
	}
})();
