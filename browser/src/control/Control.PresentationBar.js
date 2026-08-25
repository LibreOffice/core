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
 * JSDialog.PresentationBar - buttons for adding/removing slides
 */

/* global JSDialog _ _UNO app */

class PresentationBar {

	constructor(map) {
		this.map = map;
		this.parentContainer = window.L.DomUtil.get('presentation-toolbar');
		this.builder = new window.L.control.jsDialogBuilder(
			{
				mobileWizard: this,
				map: this.map,
				cssClass: 'jsdialog',
				suffix: 'presentation-toolbar',
			});

		this.create();

		map.on('wopiprops', this.onWopiProps, this);
		map.on('doclayerinit', this.onDocLayerInit, this);
		app.events.on('updatepermission', this.onUpdatePermission.bind(this));
		map.on('commandstatechanged', this.onCommandStateChanged, this);

		if (this.map.getDocType() === 'presentation') {
			this.map.on('updateparts', this.onSlideHideToggle, this);
			this.map.on('toggleslidehide', this.onSlideHideToggle, this);
		}
	}

	create() {
		if (this.parentContainer.firstChild)
			return;

		var data = [
			{
				id: 'presentation-buttons-toolbar',
				type: 'toolbox',
				children: [
					{
						id: 'presentation',
						type: 'customtoolitem',
						text: this._getItemUnoName('presentation'),
						command: 'presentation'
					},
					{
						id: 'insertpage',
						type: 'customtoolitem',
						text: this._getItemUnoName('insertpage'),
						command: 'insertpage',
					},
					{
						id: 'duplicatepage',
						type: 'customtoolitem',
						text: this._getItemUnoName('duplicatepage'),
						command: 'duplicatepage',
					},
					{
						id: 'deletepage',
						type: 'customtoolitem',
						text: this._getItemUnoName('deletepage'),
						command: 'deletepage'
					},
					{
						id: 'showslide',
						type: 'customtoolitem',
						text: _UNO('.uno:ShowSlide', 'presentation'),
						command: 'showslide',
						visible: this.map.getDocType() === 'presentation'
					},
					{
						id: 'hideslide',
						type: 'customtoolitem',
						text: _UNO('.uno:HideSlide', 'presentation'),
						command: 'hideslide',
						visible: this.map.getDocType() === 'presentation'
					}
				]
			},
			{
				id: 'presentation-import-toolbar',
				type: 'toolbox',
				children: [
					{
						id: 'importslides',
						type: 'customtoolitem',
						text: _('Import your slides'),
						command: 'importslides',
						image: false,
						inlineLabel: true,
						visible: this.map.getDocType() === 'presentation'
					}
				]
			}
		];

		this.parentContainer.replaceChildren();
		this.builder.build(this.parentContainer, data);

		if (this.map.getDocType() === 'drawing') {
			this.showItem('presentation', false);
		}
	}

	enableItem(command, enable) {
		this.builder.executeAction(this.parentContainer, {
			'control_id': command,
			'action_type': enable ? 'enable' : 'disable'
		});
	}

	showItem(command, show) {
		this.builder.executeAction(this.parentContainer, {
			'control_id': command,
			'action_type': show ? 'show' : 'hide'
		});
	}

	uncheck() {
		// TODO
	}

	show() {
		this.parentContainer.style.display = 'grid';
	}

	_getItemUnoName(id) {
		var docType = this.map.getDocType();
		switch (id) {
		case 'presentation':
			return docType === 'presentation' ? _('Fullscreen presentation') : '';
		case 'insertpage':
			return docType === 'presentation' ? _UNO('.uno:TaskPaneInsertPage', 'presentation') : _UNO('.uno:InsertPage', 'presentation');
		case 'duplicatepage':
			return docType === 'presentation' ? _UNO('.uno:DuplicateSlide', 'presentation') : _UNO('.uno:DuplicatePage', 'presentation');
		case 'deletepage':
			return docType === 'presentation' ? _UNO('.uno:DeleteSlide', 'presentation') : _UNO('.uno:DeletePage', 'presentation');
		}
		return '';
	}

	onWopiProps(e) {
		if (e.HideExportOption) {
			this.showItem('presentation', false);
		}
	}

	onDocLayerInit() {
		if (!this.map['wopi'].HideExportOption && this.map.getDocType() !== 'drawing') {
			this.showItem('presentation', true);
		}

		if (!window.mode.isSmallScreenDevice())
			this.show();
	}

	onUpdatePermission(e) {
		var presentationButtons = ['insertpage', 'duplicatepage', 'deletepage'];

		if (this.map.getDocType() === 'presentation') {
			this.showItem('importslides', e.detail.perm === 'edit');
		}

		if (e.detail.perm === 'edit') {
			presentationButtons.forEach((id) => { this.enableItem(id, true); });

			presentationButtons.forEach((id) => {
				if (id === 'deletepage') {
					var itemState = this.map['stateChangeHandler'].getItemValue('.uno:DeletePage');
				} else if (id === 'insertpage') {
					itemState = this.map['stateChangeHandler'].getItemValue('.uno:InsertPage');
				} else if (id === 'duplicatepage') {
					itemState = this.map['stateChangeHandler'].getItemValue('.uno:DuplicatePage');
				} else {
					itemState = 'enabled';
				}

				if (itemState === 'enabled') {
					this.enableItem(id, true);
				} else {
					this.enableItem(id, false);
				}
			});
		} else {
			presentationButtons.forEach((id) => { this.enableItem(id, false); });
		}
	}

	onCommandStateChanged(e) {
		var commandName = e.commandName;
		var state = e.state;

		if (this.map.isEditMode() && (state === 'enabled' || state === 'disabled')) {
			var id = window.unoCmdToToolbarId(commandName);

			if (id === 'deletepage' || id === 'insertpage' || id === 'duplicatepage') {
				if (state === 'enabled') {
					this.enableItem(id, true);
				} else {
					this.uncheck(id);
					this.enableItem(id, false);
				}
			}
		}
	}

	onSlideHideToggle() {
		if (this.map.getDocType() !== 'presentation')
			return;

		if (!app.impress.isSlideHidden(this.map.getCurrentPartNumber())) {
			this.showItem('showslide', false);
			this.showItem('hideslide', false);
		}
		else {
			this.showItem('showslide', true);
			this.showItem('hideslide', true);
		}
	}
}

JSDialog.PresentationBar = function (map) {
	return new PresentationBar(map);
};
