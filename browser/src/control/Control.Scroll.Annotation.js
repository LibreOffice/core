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
 * window.L.Control.ScrollAnnotation
 */
/* global _ */
window.L.Control.ScrollAnnotation = window.L.Control.extend({
	options: {
		position: 'topright',
		arrowUp: '0x25b2',
		arrowUpTitle: _('Scroll up annotations'),
		arrowDown: '0x25bc',
		arrowDownTitle: _('Scroll down annotations')
	},

	onAdd: function (map) {
		var scrollName = 'leaflet-control-scroll',
		    container = window.L.DomUtil.create('div', 'cool-bar');

		this._map = map;

		this._buttonUp  = this._createButton(
		        this.options.arrowUp, this.options.arrowUpTitle,
		        scrollName + '-up',  container, this._onScrollUp,  this);
		this._buttonDown = this._createButton(
		        this.options.arrowDown, this.options.arrowDownTitle,
		        scrollName + '-down', container, this._onScrollDown, this);

		return container;
	},

	_onScrollUp: function () {
		this._map.fire('AnnotationScrollUp');
	},

	_onScrollDown: function () {
		this._map.fire('AnnotationScrollDown');
	},

	_createButton: function (text, title, className, container, fn, context) {
		var link = window.L.DomUtil.create('a', className, container);
		link.textContent = String.fromCharCode(text);
		link.href = '#';
		link.title = title;

		var stop = window.L.DomEvent.stopPropagation;

		window.L.DomEvent
		    .on(link, 'click', stop)
		    .on(link, 'mousedown', stop)
		    .on(link, 'dblclick', stop)
		    .on(link, 'click', window.L.DomEvent.preventDefault)
		    .on(link, 'click', fn, context);

		return link;
	}
});

window.L.control.scrollannotation = function (options) {
	return new window.L.Control.ScrollAnnotation(options);
};
