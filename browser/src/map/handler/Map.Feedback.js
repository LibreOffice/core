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
 * window.L.Map.Feedback.
 */

/* global _ */

window.L.Map.mergeOptions({
	feedback: !window.ThisIsAMobileApp,
	feedbackTimeout: 30000
});

window.L.Map.Feedback = window.L.Handler.extend({

	addHooks: function () {
		this.initialized = false;

		if (this._map.wopi)
			this._map.on('updateviewslist', this.onUpdateList, this);
		else
			this._map.on('docloaded', this.onDocLoaded, this);

		window.L.DomEvent.on(window, 'message', this.onMessage, this);
	},

	removeHooks: function () {
		window.L.DomEvent.off(window, 'message', this.onMessage, this);
	},

	removeIframe: function () {
		if (this._iframeDialog)
			this._iframeDialog.remove()
	},

	onUpdateList: function () {
		var docLayer = this._map._docLayer || {};

		if (docLayer && docLayer._viewId == 0)
			this.onDocLoaded();
	},

	onDocLoaded: function () {
		if (this.initialized)
			return;

		this.initialized = true;

		if (window.prefs.getBoolean('WSDFeedbackEnabled', true)) {
			var laterDate = new Date();
			var currentDate = new Date();
			var timeValue = window.prefs.getNumber('WSDFeedbackLaterDate');
			var docCount = window.prefs.getNumber('WSDFeedbackCount', 0);

			docCount++;

			window.prefs.set('WSDFeedbackCount', docCount);

			if (isNaN(timeValue)) {
				/* - 5 seconds */
				laterDate.setTime(currentDate.getTime() - 5000);
			} else {
				/* + 5 days (432,000,000 Milliseconds) */
				laterDate.setTime(timeValue + 432000000);
			}

			if (docCount > 15 && currentDate > laterDate && window.autoShowFeedback)
				setTimeout(window.L.bind(this.onFeedback, this), this._map.options.feedbackTimeout);
		}
	},

	onFeedback: function () {
		if (this._map.welcome && this._map.welcome.isVisible && this._map.welcome.isVisible()) {
			setTimeout(window.L.bind(this.onFeedback, this), this._map.options.feedbackTimeout);
			return;
		}

		if (this._map.welcome && this._map.welcome.isVisible && this._map.welcome.isVisible())
			setTimeout(window.L.bind(this.onFeedback, this), 3000);
		else {
			this.askForFeedbackDialog();
		}
	},

	askForFeedbackDialog: function () {
		this._map.uiManager.showSnackbar(
			_('Please send us your feedback'),
			_('OK'),
			this.showFeedbackDialog.bind(this));
	},

	showFeedbackDialog: function () {
		if (this._iframeDialog && this._iframeDialog.hasLoaded())
			this.removeIframe();

		var proxyPrefixEnabled = window.socketProxy ? "True" : "False";

		var cssVar = getComputedStyle(document.documentElement).getPropertyValue('--co-primary-element');
		var params = [{ mobile : window.mode.isSmallScreenDevice() },
			      { cssvar : cssVar},
			      { wsdhash : window.app.socket.WSDServer.Hash },
			      { 'version_number' : window.app.socket.WSDServer.Version },
			      { 'wopi_host_id' : window.wopiHostId },
			      { 'proxy_prefix_enabled' : proxyPrefixEnabled },
			      { 'doc_type': this._map.getDocType()}];

		var options = {
			prefix: 'iframe-dialog',
			id: 'iframe-feedback',
		};

		this._iframeDialog = window.L.iframeDialog(window.feedbackUrl, params, null, options);
	},

	onError: function () {
		window.prefs.remove('WSDFeedbackEnabled');
		this.removeIframe();
	},

	onMessage: function (e) {
		if (typeof e.data !== 'string')
			return; // Some extensions may inject scripts resulting in load events that are not strings

		if (e.data.startsWith('updatecheck-show'))
			return;

		var data = e.data;
		data = JSON.parse(data).MessageId;

		if (data == 'feedback-show') {
			this._iframeDialog.show();
		}
		else if (data == 'feedback-never') {
			window.prefs.set('WSDFeedbackEnabled', false);
			window.prefs.remove('WSDFeedbackCount');
			this.removeIframe();
		} else if (data == 'feedback-later') {
			var currentDate = new Date();
			this.removeIframe();
			window.prefs.set('WSDFeedbackLaterDate', currentDate.getTime());
			window.prefs.remove('WSDFeedbackCount');
		} else if (data == 'feedback-submit') {
			window.prefs.set('WSDFeedbackEnabled', false);
			window.prefs.remove('WSDFeedbackCount');
		} else if (data == 'iframe-feedback-load' && !this._iframeDialog.isVisible()) {
			this.removeIframe();
			setTimeout(window.L.bind(this.onFeedback, this), this._map.options.feedbackTimeout);
		} else if (data.endsWith('close')) {
			this.removeIframe();
		}
	}
});
if (window.feedbackUrl && window.prefs.canPersist) {
	window.L.Map.addInitHook('addHandler', 'feedback', window.L.Map.Feedback);
}
