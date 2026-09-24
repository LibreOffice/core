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
 * window.L.Control.MobileSlide is used to add new slide button on the Impress document.
 */

window.L.Control.MobileSlide = window.L.Control.extend({
	options: {
		position: 'bottomright'
	},

	onAdd: function (map) {
		this._map = map;

		if (!this._container) {
			this._initLayout();
		}

		return this._container;
	},

	onRemove: function () {
		this._map = undefined;
	},

	_onAddSlide: function () {
		this._map.insertPage();
	},

	_initLayout: function () {
		this._container = window.L.DomUtil.create('div', 'leaflet-control-zoom leaflet-bar');
		this._createButton('+', '', 'leaflet-control-zoom-in',  this._container, this._onAddSlide,  this);
		return this._container;
	},

	_createButton: function (text, title, className, container, fnOnClick, context) {
		var button = window.L.DomUtil.create('a', className, container);
		button.textContent = text;
		button.href = '#';
		button.title = title;

		window.L.DomEvent
		    .on(button, 'click', window.L.DomEvent.stopPropagation)
		    .on(button, 'mousedown', window.L.DomEvent.stopPropagation)
		    .on(button, 'click', window.L.DomEvent.preventDefault)
		    .on(button, 'click', this._map.focus, this._map)
		    .on(button, 'click', fnOnClick, context);

		return button;
	},
});

window.L.control.mobileSlide = function (options) {
	return new window.L.Control.MobileSlide(options);
};

