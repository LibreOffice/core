// SPDX-License-Identifier: MPL-2.0
//
// Helpers for COOL extension iframes.  An extension's HTML page loads this script and uses the
// helpers to talk to COOL via postMessage.

(function () {
	window.cool = window.cool || {};

	let nextCallId = 0;
	const pending = Object.create(null);

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
		if (data.msgId !== 'Extension_CallResult') {
			console.warn('postMessage has unexpected msgId: ' + data.msgId);
			return;
		}
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
	});
})();
