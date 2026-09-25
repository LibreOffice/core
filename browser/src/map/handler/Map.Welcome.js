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
/*
 * window.L.Map.Welcome.
 */

/* global app _ */
window.L.Map.mergeOptions({
	welcome: true
});

window.L.Map.Welcome = window.L.Handler.extend({

	_getLocalWelcomeUrl: function () {
		var welcomeLocation = app.LOUtil.getURL('/welcome/welcome.html');
		if (window.socketProxy)
			welcomeLocation = window.makeWsUrl(welcomeLocation);
		return welcomeLocation;
	},

	initialize: function (map) {
		window.L.Handler.prototype.initialize.call(this, map);
		this._map.on('updateviewslist', this.onUpdateList, this);

		// temporarily use only local welcome dialog
		this._url = /*window.welcomeUrl ? window.welcomeUrl:*/ this._getLocalWelcomeUrl();
		this._retries = 2;
		this._fallback = false;
	},

	addHooks: function () {
		if (window.mode.isCODesktop())
			return;
		window.L.DomEvent.on(window, 'message', this.onMessage, this);
		this.remove();
	},

	isGuest: function () {
		var docLayer = this._map._docLayer || {};
		var viewInfo = this._map._viewInfo[docLayer._viewId];
		return viewInfo && viewInfo.userextrainfo && viewInfo.userextrainfo.is_guest;
	},

	onUpdateList: function () {
		if (!this.isGuest() && window.autoShowWelcome && this.shouldWelcome()) {
			this.showWelcomeDialog();
		} else if (this.shouldWelcomeWithSlideshow()) {
			this.showWelcomeSlideshow();
		}
	},

	shouldWelcomeWithSlideshow: function() {
		if (!window.mode.isCODesktop())
			return false;

		let storedVersion = window.prefs.get('WSDWelcomeVersion');
		let currentVersion = app.socket.WSDServer.Version;

		if(!storedVersion || storedVersion !== currentVersion) {
			return true;
		}

		return false;
	},

	shouldWelcome: function () {
		let storedVersion = window.prefs.get('WSDWelcomeVersion');
		let currentVersion = app.socket.WSDServer.Version;
		let welcomeDisabledCookie = window.prefs.getBoolean('WSDWelcomeDisabled');
		let welcomeDisabledDate = window.prefs.get('WSDWelcomeDisabledDate');
		if (welcomeDisabledDate)
			welcomeDisabledDate = welcomeDisabledDate.replaceAll('-', ' ');
		let isWelcomeDisabled = false;

		if (welcomeDisabledCookie && welcomeDisabledDate) {
			// Check if we are still in the same day
			var currentDate = new Date();
			if (welcomeDisabledDate === currentDate.toDateString())
				isWelcomeDisabled = true;
			else {
				//Values expired. Clear the local values
				window.prefs.remove('WSDWelcomeDisabled');
				window.prefs.remove('WSDWelcomeDisabledDate');
			}
		}

		if ((!storedVersion || storedVersion !== currentVersion) && !isWelcomeDisabled) {
			return true;
		}

		return false;
	},

	showWelcomeSlideshow: function() {
		if (window.coolParams.get('welcome') === 'true')
			return;

		app.socket.sendMessage('WELCOME');
		window.prefs.set('WSDWelcomeVersion', app.socket.WSDServer.Version);
	},

	// Whether a welcome dialog is open: loading, shown, or waiting to be loaded again after a
	// failed load.
	isOpen: function () {
		if (this._reloadPending)
			return true;
		return !!this._iframeWelcome && !!this._iframeWelcome.queryContainer();
	},

	// Loads the welcome dialog again after a failed load.
	_reloadWelcomeDialog: function () {
		this._reloadPending = true;
		app.timerRegistry.setTimeout('welcome', window.L.bind(function () {
			this._reloadPending = false;
			this.showWelcomeDialog();
		}, this), 200);
	},

	showWelcomeDialog: function () {
		if (this._iframeWelcome && this._iframeWelcome.queryContainer())
			this.remove();

		var uiTheme = window.prefs.getBoolean('darkTheme') ? 'dark' : 'light';
		var params = [{ 'ui_theme': uiTheme }];

		this._iframeWelcome = window.L.iframeDialog(this._url, params, null, {
			prefix: 'iframe-welcome',
			onUserClose: window.L.bind(this.close, this),
		});
		this._iframeWelcome._iframe.title = _('Welcome Dialog');
	},

	removeHooks: function () {
		window.L.DomEvent.off(window, 'message', this.onMessage, this);
		this.close();
	},

	remove: function () {
		if (this._iframeWelcome) {
			this._iframeWelcome.remove();
			delete this._iframeWelcome;
		}
	},

	// Removes the welcome dialog for good, with no load retry to follow.
	close: function () {
		var wasOpen = this.isOpen();
		this._reloadPending = false;
		this.remove();
		if (wasOpen)
			this._map.fire('welcomeclosed');
	},

	onMessage: function (e) {
		// Only the welcome dialog and this window itself send these:
		if (e.source !== window
		    && !(this._iframeWelcome && this._iframeWelcome.isFrameWindow(e.source)))
			return;
		if (typeof e.data !== 'string')
			return; // Some extensions may inject scripts resulting in load events that are not strings

		if (e.data.startsWith('updatecheck-show'))
			return;

		var data = JSON.parse(e.data);

		if (data.MessageId === 'welcome-show') {
			this._iframeWelcome.show();
		} else if (data.MessageId == 'welcome-translate') {
			this._iframeWelcome.clearTimeout();
			var keys = Object.keys(data.strings);
			for (var it in keys) {
				data.strings[keys[it]] = _(keys[it]).replace('{coolversion}', window.coolwsdVersion);
			}
			this._iframeWelcome.postMessage(data);
		} else if (data.MessageId === 'welcome-close') {
			window.prefs.set('WSDWelcomeVersion', app.socket.WSDServer.Version);
			this.close();
		} else if (data.MessageId == 'iframe-welcome-load' && !this._iframeWelcome.isVisible()) {
			if (this._retries-- > 0) {
				this.remove();
				this._reloadWelcomeDialog();
			} else if (this._fallback) {
				var currentDate = new Date();
				window.prefs.set('WSDWelcomeDisabled', true);
				window.prefs.set('WSDWelcomeDisabledDate', currentDate.toDateString().replaceAll(' ', '-'));
				this.close();
			} else {
				// fallback
				this._url = this._getLocalWelcomeUrl();
				this._fallback = true;
				this._reloadWelcomeDialog();
			}
		}
	}
});

if (!window.L.Browser.cypressTest && (window.enableWelcomeMessage || window.mode.isCODesktop()) && window.prefs.canPersist) {
	window.L.Map.addInitHook('addHandler', 'welcome', window.L.Map.Welcome);
}
