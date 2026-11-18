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
 * window.L.Control.DocumentRepair.
 */
/* global _ */
window.L.Control.DocumentRepair = window.L.Control.extend({
	builder: null,
	actions: null,
	selected: null,

	addTo: function (map) {
		this.remove();

		var data = {
			id: 'DocumentRepairDialog',
			dialogid: 'DocumentRepairDialog',
			type: 'dialog',
			text: _('Repair Document'),
			title: _('Repair Document'),
			jsontype: 'dialog',
			responses: [
				{
					id: 'ok',
					response: 1
				},
				{
					id: 'cancel',
					response: 0
				},
			],
			'init_focus_id': 'ok',
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
											id: 'cancel',
											type: 'pushbutton',
											text: _('Cancel'),
											enabled: true
										},
										{
											id: 'ok',
											type: 'pushbutton',
											text: _('OK'),
											enabled: true,
											'has_default': true,
										}
									],
									vertical: false,
									layoutstyle: 'end'
								},
							],
						},
					]
				},
			],
		};

		this._map = map;
		this.actions = [];

		var dialogBuildEvent = {
			data: data,
			callback: this._onAction.bind(this),
		};


		this._map.fire(window.mode.isSmallScreenDevice() ? 'mobilewizard' : 'jsdialog', dialogBuildEvent);

		return this;
	},

	createAction: function (type, index, comment, viewId, dateTime, action) {
		this.actions.push({ 'text': comment, 'columns': [
			type,
			comment,
			viewId,
			dateTime
		].map(
			function (item) {
				return { text: item };
			}
		), 'row': index, 'action': action});
	},

	fillAction: function (actions, type, action) {
		for (var iterator = 0; iterator < actions.length; ++iterator) {
			// No user name if the user in question is already disconnected.
			var userName = actions[iterator].userName ? actions[iterator].userName : '';
			if (parseInt(actions[iterator].viewId) === this._map._docLayer._viewId) {
				userName = _('You');
			}
			this.createAction(type, actions[iterator].index, actions[iterator].comment, userName, this.transformTimestamp(actions[iterator].dateTime), action);
		}
	},

	show: function () {
		if (this.actions.length !== 0) {
			var dialogUpdateEvent = {
				data: {
					jsontype: 'dialog',
					action: 'update',
					id: 'DocumentRepairDialog',
					control: {
						id: 'versions',
						type: 'treelistbox',
						headers: [_('Type'), _('What?'), _('Who?'), _('When?')].map(
							function(item) { return { text: item }; }
						),
						text: '',
						enabled: true,
						entries: this.actions,
						sortLocally: true,
					},
				},
				callback: this._onAction.bind(this)
			};
		} else {
			var dialogUpdateEvent = {
				data: {
					jsontype: 'dialog',
					action: 'update',
					id: 'DocumentRepairDialog',
					control: {
						id: 'versions',
						type: 'fixedtext',
						text: _('You have not done anything to rollback yet...'),
					},
				},
			};
		}
		if (window.mode.isSmallScreenDevice()) window.mobileDialogId = dialogUpdateEvent.data.id;
		this._map.fire('jsdialogupdate', dialogUpdateEvent);
	},

	fillActions: function (data) {
		this.fillAction(data.Redo.actions, _('Redo'), 'Redo');
		this.fillAction(data.Undo.actions, _('Undo'), 'Undo');
	},

	_onAction: function(element, action, data, index) {
		if (element === 'dialog' && action === 'close') return;
		if (element === 'treeview') {
			var entry = data.entries[parseInt(index)];
			this.selected = {
				action: entry.action,
				index: parseInt(entry.row),
			};
			return;
		}
		if (element === 'responsebutton' && data.id == 'ok' && this.selected) {
			this._onOk(this.selected.action, this.selected.index);
		}

		var closeEvent = {
		    data: {
				action: 'close',
				id: 'DocumentRepairDialog',
			}
		};
		this._map.fire(window.mode.isSmallScreenDevice() ? 'closemobilewizard' : 'jsdialog', closeEvent);
		console.log('Closed after' + element + ' ' + action);
	},

	_onOk: function (action, index) {
		var command = {
			Repair: {
				type: 'boolean',
				value: true
			}
		};
		command[action] = {
			type: 'unsigned short',
			value: index + 1
		};
		this._map.sendUnoCommand('.uno:' + action, command, true);
	},

	// Transform timestamp from ISO8601 to human readable format with Local time
	transformTimestamp: function (timestamp) {
		var d = new Date(timestamp.split(',')[0] + 'Z');
		var dateOptions = { year: 'numeric', month: '2-digit', day: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit', hour12: true };
		var formattedDateTime = d.toLocaleString(String.locale, dateOptions);
		return formattedDateTime;
	}
});

window.L.control.documentRepair = function (options) {
	return new window.L.Control.DocumentRepair(options);
};
