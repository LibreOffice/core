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
 * window.L.Control.Selection enables by mouse drag selection in viewing mode
 */

/* global app */

window.L.Control.Selection = window.L.Control.extend({
	options: {
		position: 'topleft'
	},

	onAdd: function () {
		var partName = 'leaflet-control-editviewswitch',
		    container = window.L.DomUtil.create('label', partName + ' leaflet-bar');

		this._checkBox = window.L.DomUtil.create('input', 'editview-cb', container);
		this._checkBox.type = 'checkbox';
		window.L.DomEvent.on(this._checkBox, 'change', this._onChange, this);
		app.events.on('updatepermission', this._onUpdatePermission.bind(this));
		container.appendChild(document.createTextNode('Enable Selection'));
		return container;
	},

	_onChange: function () {
		this._map.focus();
	},

	_onUpdatePermission: function (e) {
		if (e.detail.perm === 'edit') {
			this._checkBox.checked = false;
			this._checkBox.disabled = true;
		}
		else if (e.detail.perm === 'view') {
			this._checkBox.checked = false;
			this._checkBox.disabled = false;
		}
		else if (e.detail.perm === 'readonly') {
			this._checkBox.checked = false;
			this._checkBox.disabled = false;
		}
	}
});

window.L.control.selection = function (options) {
	return new window.L.Control.Selection(options);
};
