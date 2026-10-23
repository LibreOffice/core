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
 * window.L.Control.VersionHistory
 *
 * Dialog for the desktop apps' local version store: versions come from
 * common/VersionStore.cpp over the 'versionhistory' protocol. The server
 * product delegates to the WOPI host instead (see openRevisionHistory).
 */
/* global _ app */
window.L.Control.VersionHistory = window.L.Control.extend({
	entries: null,
	selected: null,
	_visible: false,

	addTo: function (map) {
		var data = {
			id: 'VersionHistoryDialog',
			dialogid: 'VersionHistoryDialog',
			type: 'dialog',
			text: _('Version History'),
			title: _('Version History'),
			jsontype: 'dialog',
			responses: [
				{
					id: 'version-open-copy',
					response: 1,
				},
				{
					id: 'version-delete-history',
					response: 2,
				},
				{
					id: 'cancel',
					response: 0,
				},
			],
			init_focus_id: 'version-open-copy',
			enabled: true,
			children: [
				{
					id: 'dialog-vbox1',
					type: 'container',
					text: '',
					enabled: true,
					vertical: true,
					children: [
						{
							type: 'treelistbox',
							id: 'versions',
							enabled: false,
						},
						{
							id: 'version-store-note',
							type: 'fixedtext',
							text: '',
						},
						{
							id: 'dialog-action_area1',
							type: 'container',
							text: '',
							enabled: true,
							vertical: true,
							children: [
								{
									id: '',
									type: 'buttonbox',
									text: '',
									enabled: true,
									children: [
										{
											id: 'version-delete-history',
											type: 'pushbutton',
											text: _('Delete history'),
											enabled: true,
										},
										{
											id: 'cancel',
											type: 'pushbutton',
											text: _('Close'),
											enabled: true,
										},
										{
											id: 'version-open-copy',
											type: 'pushbutton',
											text: _('Open a copy'),
											enabled: true,
											has_default: true,
										},
									],
									vertical: false,
									layoutstyle: 'end',
								},
							],
						},
					],
				},
			],
		};

		this._map = map;
		this.entries = [];
		this.selected = null;

		var dialogBuildEvent = {
			data: data,
			callback: this._onAction.bind(this),
		};

		this._map.fire(
			window.mode.isSmallScreenDevice() ? 'mobilewizard' : 'jsdialog',
			dialogBuildEvent,
		);
		this._visible = true;

		return this;
	},

	isVisible: function () {
		return this._visible;
	},

	fill: function (data) {
		this.entries = data.entries || [];
		this.selected = null;
	},

	show: function () {
		var rows = this.entries.map(function (entry, index) {
			var when = new Date(entry.timestamp).toLocaleString(String.locale, {
				year: 'numeric',
				month: '2-digit',
				day: '2-digit',
				hour: '2-digit',
				minute: '2-digit',
			});
			return {
				text: when,
				columns: [
					when,
					window.L.Control.VersionHistory.humanizeSize(entry.size),
				].map(function (item) {
					return { text: item };
				}),
				row: index,
			};
		});

		var control;
		if (rows.length) {
			control = {
				id: 'versions',
				type: 'treelistbox',
				headers: [_('When'), _('Size')].map(function (item) {
					return { text: item };
				}),
				text: '',
				enabled: true,
				entries: rows,
			};
		} else {
			control = {
				id: 'versions',
				type: 'fixedtext',
				text: _('No versions of this document have been kept yet.'),
			};
		}

		var dialogUpdateEvent = {
			data: {
				jsontype: 'dialog',
				action: 'update',
				id: 'VersionHistoryDialog',
				control: control,
			},
			callback: this._onAction.bind(this),
		};
		if (window.mode.isSmallScreenDevice())
			window.mobileDialogId = dialogUpdateEvent.data.id;
		this._map.fire('jsdialogupdate', dialogUpdateEvent);

		var note = this.storeEnabled
			? _('Versions are kept on this computer.')
			: _('Versions are not being kept.');
		if (this.entries.length)
			note +=
				' ' +
				window.L.Control.VersionHistory.humanizeSize(this.storeSize) +
				' ' +
				_('in use for all documents.');
		this._map.fire('jsdialogupdate', {
			data: {
				jsontype: 'dialog',
				action: 'update',
				id: 'VersionHistoryDialog',
				control: {
					id: 'version-store-note',
					type: 'fixedtext',
					text: note,
				},
			},
			callback: this._onAction.bind(this),
		});
	},

	_close: function () {
		var closeEvent = {
			data: {
				action: 'close',
				id: 'VersionHistoryDialog',
			},
		};
		this._map.fire(
			window.mode.isSmallScreenDevice() ? 'closemobilewizard' : 'jsdialog',
			closeEvent,
		);
		this._visible = false;
	},

	_onAction: function (element, action, data, index) {
		if (element === 'dialog' && action === 'close') {
			this._visible = false;
			return;
		}
		if (element === 'treeview') {
			// A click on a column header comes here too, with no row.
			if (action === 'select' || action === 'activate')
				this.selected = parseInt(index);
			return;
		}

		var id = data && data.id;
		if (element === 'responsebutton' || element === 'pushbutton') {
			if (id === 'version-open-copy' && this.entries.length) {
				var entry = this.entries[this.selected !== null ? this.selected : 0];
				app.socket.sendMessage('versionhistory opencopy name=' + entry.name);
				this._close();
				return;
			}
			if (id === 'version-delete-history' && this.entries.length) {
				this._map.uiManager.showYesNoButton(
					'version-delete-history-confirm',
					_('Delete history'),
					_('Delete all kept versions of this document from this computer?'),
					_('Delete'),
					_('Cancel'),
					function () {
						app.socket.sendMessage('versionhistory clear');
					},
					function () {},
					true,
				);
				return;
			}
		}

		this._close();
	},
});

window.L.Control.VersionHistory.humanizeSize = function (bytes) {
	var units = ['B', 'KB', 'MB', 'GB'];
	var size = bytes || 0;
	var unit = 0;
	while (size >= 1024 && unit < units.length - 1) {
		size /= 1024;
		unit++;
	}
	return (unit ? size.toFixed(1) : size) + ' ' + units[unit];
};

/// Entry point for 'versionhistory:' messages from coolwsd.
window.L.Control.VersionHistory.handleMessage = function (map, json) {
	if (json.exportedUri !== undefined) {
		// The reply to 'versionhistory opencopy'.
		window.postMobileMessage(
			'opendoc file=' + encodeURIComponent(json.exportedUri) + ' draft=true',
		);
		return;
	}

	if (!map._versionHistory) {
		map._versionHistory = window.L.control.versionHistory();
	}

	var dialog = map._versionHistory;
	if (!dialog.isVisible()) {
		dialog.addTo(map);
	}
	dialog.storeSize = json.storeSize || 0;
	dialog.storeEnabled = json.enabled !== false;
	dialog.fill(json);
	dialog.show();
};

window.L.control.versionHistory = function (options) {
	return new window.L.Control.VersionHistory(options);
};
