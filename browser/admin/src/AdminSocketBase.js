/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Abstract class for Admin Sockets
 */

/* global _ DlgYesNo Base */

var AdminSocketBase = Base.extend({
	socket: null,

	constructor: function (host) {
		// because i am abstract
		if (this.constructor === AdminSocketBase) {
			throw new Error('Cannot instantiate abstract class');
		}

		// We do not allow such child class to instantiate websocket that do not implement
		// onSocketMessage and onSocketOpen.
		if (typeof this.onSocketMessage === 'function' && typeof this.onSocketOpen === 'function') {
			this.socket = new WebSocket(host);
			this.socket.onopen = this.onSocketOpen.bind(this);
			this.socket.onclose = this.onSocketClose.bind(this);
			this.socket.onmessage = this.onSocketMessage.bind(this);
			this.socket.onerror = this.onSocketError.bind(this);
			this.socket.binaryType = 'arraybuffer';
		}

		this.pageWillBeRefreshed = false;
		var onBeforeFunction = function() {
			this.pageWillBeRefreshed = true;
		};
		window.onbeforeunload = onBeforeFunction.bind(this);
	},

	onSocketOpen: function () {
		// Authenticate
		this.socket.send('auth jwt=' + window.jwtToken);
	},

	onSocketMessage: function () {
		/* Implemented by child */
	},

	onSocketClose: function () {
		this.socket.onerror = function () { };
		this.socket.onclose = function () { };
		this.socket.onmessage = function () { };
		this.socket.close();

		if (this.pageWillBeRefreshed === false) {
			var dialog = (new DlgYesNo())
				.title(_('Refresh'))
				.text(_('Server has been shut down; please reload the page.'))
				.yesButtonText(_('OK'))
				.type('warning')
				.yesFunction(function() { window.location.reload(); });
			dialog.open();
		}
	},

	onSocketError: function () {
		var dialog = (new DlgYesNo())
			.title(_('Connection error'))
			.text(_('Connection error'))
			.yesButtonText(_('OK'))
			.type('warning')
			.yesFunction(function() { window.location.reload(); });
		dialog.open();
	}
});
