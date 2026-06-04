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
 * window.L.IFrameDialog
 */

/* global _ app cool */

window.L.IFrameDialog = window.L.Class.extend({

	options: {
		prefix: 'iframe-none',
		method: 'get'
	},

	initialize: function (url, params, element, options) {
		var content, form;

		this._loading = false;
		window.L.setOptions(this, options);

		const containerCss = this.options.dialogCssClass;
		this._container = window.L.DomUtil.create('div', this.options.prefix + '-wrap ' + containerCss);
		if (this.options.titlebar) {
			const titlebar = window.L.DomUtil.create('div', 'ui-dialog-titlebar ui-corner-all ui-widget-header ui-helper-clearfix', this._container);
			const title = window.L.DomUtil.create('h2', 'ui-dialog-title', titlebar);
			title.innerText = _('Options');
			const closeButton = window.L.DomUtil.create('button', 'ui-button ui-corner-all ui-widget ui-button-icon-only ui-dialog-titlebar-close', titlebar);
			window.L.DomUtil.create('span', 'ui-button-icon ui-icon ui-icon-closethick', closeButton);
			window.L.DomEvent.on(closeButton, 'click', () => this.remove(this._container));
			this._draggable = new window.L.Draggable(this._container, titlebar);
			this._draggable.enable();
		}
		content = window.L.DomUtil.create('div', this.options.prefix + '-content', this._container);

		this._container.style.display = 'none';
		// this should be set for making it focusable
		this._container.tabIndex = -1;

		form = window.L.DomUtil.create('form', '', content);

		this.fillParams(url, params, form);

		this._iframe = window.L.DomUtil.create('iframe', this.options.prefix + '-modal', content);
		this._iframe.name = form.target;

		if (this.options.id) {
			this._iframe.id = this.options.id;
		}

		const modalButtons = this.options.modalButtons;
		if (modalButtons) {
			const buttonBox = window.L.DomUtil.create(
				'div',
				'jsdialog ui-button-box end',
				content
			);
			const buttonBoxLeft = window.L.DomUtil.create(
				'div',
				'jsdialog ui-button-box-left',
				buttonBox
			);
			const buttonBoxRight = window.L.DomUtil.create(
				'div',
				'jsdialog ui-button-box-right',
				buttonBox
			);

			for (const i in modalButtons) {
				const wrapper = window.L.DomUtil.create('div','ui-pushbutton-wrapper', modalButtons[i].align === 'left' ? buttonBoxLeft : buttonBoxRight);
				const button = window.L.DomUtil.create('button', 'ui-pushbutton', wrapper);
				button.id = modalButtons[i].id;
				button.innerText = modalButtons[i].text;
			}
		}

		if (element) {
			document.body.insertBefore(this._container, element);
		} else {
			document.body.appendChild(this._container);
		}

		form.submit();
		this._iframe.addEventListener('load', window.L.bind(this.onLoad, this));
	},

	fillParams: function (url, params, form) {
		var input, keys;
		form.action = url;
		form.target = this.options.prefix + '-form';
		form.method = this.options.method;

		for (var item in params) {
			keys = Object.keys(params[item]);
			if (keys.length > 0) {
				input = window.L.DomUtil.create('input', '', form);
				input.type = 'hidden';
				input.name = String(keys[0]);
				input.value = String(params[item][keys[0]]);
			}
		}
	},

	onLoad: function () {
		var msg = this.options.prefix + '-load';
		var that = this;
		this._loading = true;
		this._errorTimer = setTimeout(function () {
			if (!that.isVisible()) {
				window.postMessage('{"MessageId":"' + msg + '"}');
			}
		}, 1000);

		if (this.options.stylesheets) {
			this.addStyleSheets(this.options.stylesheets);
		}

		// Listen for Escape inside the iframe for when the iframe has focus.
		var self = this;
		this._iframe.contentDocument.addEventListener('keydown', function(e) {
			if (e.key === 'Escape') {
				self.remove();
			}
		});
	},

	addStyleSheet: function (href) {
		if (!this._iframe || !this._iframe.contentDocument) {
			console.error('Cannot access iframe element');
			return false;
		}

		const head = this._iframe.contentDocument.head;
		const link = this._iframe.contentDocument.createElement('link');
		link.rel = 'stylesheet';
		link.type = 'text/css';
		link.href = href;
		head.appendChild(link);

		return true;
	},

	addStyleSheets: function (stylesheets) {
		for (const i in stylesheets) {
			this.addStyleSheet(stylesheets[i]);
		}
	},

	clearTimeout: function ()
	{
		clearTimeout(this._errorTimer);
	},

	focus: function() {
		if (this._iframe && this._iframe.contentWindow) {
			this._iframe.contentWindow.focus();
		} else if (this._container) {
			this._container.focus();
		}
	},

	remove: function () {
		if (this._draggable) {
			this._draggable.disable();
			this._draggable = null;
		}
		window.L.DomEvent.off(this._iframe, 'load', this.onLoad, this);
		window.L.DomUtil.remove(this._container);
		this._container = this._iframe = null;
		app.map._iframeDialog = null;
		app.map.focus();
	},

	hasLoaded: function () {
		return this.queryContainer() && this._loading;
	},

	queryContainer: function () {
		return document.body.querySelector('.' + this.options.prefix + '-wrap');
	},

	postMessage: function (msg) {
		// On the desktop apps the parent and the iframe both load over
		// file://, but WebView2 gives the iframe an opaque "null" origin
		// while the parent is "file://"; a targetOrigin of window.origin
		// then never matches and the message is silently dropped. The
		// iframe is our own trusted local content there, so target "*".
		const targetOrigin = window.mode.isCODesktop() ? '*' : window.origin;
		this._iframe.contentWindow.postMessage(JSON.stringify(msg), targetOrigin);
	},

	isVisible: function () {
		var elem = this.queryContainer();
		return elem && elem.style.display !== 'none';
	},

	show: function () {
		this._container.style.display = '';
		if (this.options.titlebar && !this._container._leaflet_pos) {
			var rect = this._container.getBoundingClientRect();
			this._container.style.left = '0px';
			this._container.style.top = '0px';
			var pos = new cool.Point(rect.left, rect.top);
			window.L.DomUtil.setPosition(this._container, pos);
		}
		app.map._iframeDialog = this;
		this.focus();
	}
});

// Close when pressing Escape
window.addEventListener('keyup', function iframeKeyupListener (e) {
	if (e.keyCode === 27 || e.key === 'Escape') {
		window.postMessage('{"MessageId":"welcome-close"}', window.origin);
		window.postMessage('{"MessageId":"settings-cancel"}', window.origin);
	}
});

window.L.iframeDialog = function (url, params, element, options) {
	return new window.L.IFrameDialog(url, params, element, options);
};
