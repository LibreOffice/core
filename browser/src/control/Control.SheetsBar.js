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
 * Control.SheetsBar - toolbar with buttons for scrolling tabs
 */

/* global _ JSDialog app */
class SheetsBar {

	constructor(map, showNavigation = true) {
		this.showNavigation = showNavigation;
		this.onAdd(map);
	}

	onAdd(map) {
		this.map = map;
		this.parentContainer = window.L.DomUtil.get('spreadsheet-toolbar');
		this.builder = new window.L.control.jsDialogBuilder(
			{
				mobileWizard: this,
				map: this.map,
				cssClass: 'jsdialog',
				suffix: 'spreadsheet-toolbar',
				callback: this.onSelectSheet.bind(this),
			});

		this.create();

		map.on('doclayerinit', this.onDocLayerInit, this);
		map.on('updateparts', this.onUpdateParts, this);
		app.events.on('updatepermission', this.onUpdatePermission.bind(this));

		if (this.showNavigation) {
			document.addEventListener('scroll', (e) => {
				if (e.target && e.target.id === 'spreadsheet-tab-scroll')
					this.updateNavigation();
			}, true);
			if (window.ResizeObserver)
				new ResizeObserver(() => this.updateNavigation()).observe(this.parentContainer);
		}
	}

	create() {
		var data = [
			{
				id: 'sheets-buttons-toolbox',
				type: 'toolbox',
				children: [
					{
						id: 'firstrecord',
						type: 'customtoolitem',
						text: _('Scroll to the first sheet'),
						command: 'firstrecord',
						visible: this.showNavigation
					},
					{
						id: 'prevrecord',
						type: 'customtoolitem',
						text: _('Scroll Left'),
						command: 'prevrecord',
						visible: this.showNavigation,
						pressAndHold: true
					},
					{
						id: 'nextrecord',
						type: 'customtoolitem',
						text: _('Scroll Right'),
						command: 'nextrecord',
						visible: this.showNavigation,
						pressAndHold: true
					},
					{
						id: 'lastrecord',
						type: 'customtoolitem',
						text: _('Scroll to the last sheet'),
						command: 'lastrecord',
						visible: this.showNavigation
					},
					{
						id: 'insertsheet',
						type: 'customtoolitem',
						text: _('Insert sheet'),
						command: 'insertsheet'
					},
					{
						id: 'sheetlist:SheetListMenu',
						type: 'menubutton',
						noLabel: true,
						text: _('Show sheet list'),
						command: 'sheetlist',
					},
				]
			}
		];

		this.parentContainer.replaceChildren();
		this.builder.build(this.parentContainer, data);

		const toolbox = this.parentContainer.querySelector('#sheets-buttons-toolbox');
		if (toolbox)
			toolbox.addEventListener('keydown', this.onToolboxKeyDown.bind(this));

		this.updateSheetListMenu();
	}

	onToolboxKeyDown(e) {
		if (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight')
			return;
		if (e.altKey || e.ctrlKey || e.metaKey || e.shiftKey)
			return;

		const buttons = Array.from(e.currentTarget.querySelectorAll('button')).filter(
			(button) => !button.disabled && button.offsetParent !== null);
		const current = buttons.findIndex((button) => button.contains(e.target));
		if (current === -1)
			return;

		const step = e.key === 'ArrowRight' ? 1 : -1;
		const next = (current + step + buttons.length) % buttons.length;
		e.preventDefault();
		e.stopPropagation();
		buttons[next].focus();
	}

	onDocLayerInit() {
		var docType = this.map.getDocType();
		if (docType == 'spreadsheet') {
			if (!window.mode.isSmallScreenDevice()) {
				this.show();
			}
		}
	}

	onUpdateParts(e) {
		this.partNames = e.partNames || [];
		this.selectedPart = e.selectedPart;
		this.parts = e.parts;
		this.updateSheetListMenu();
		this.updateNavigation();
	}

	updateNavigation() {
		if (!this.showNavigation)
			return;

		app.layoutingService.appendLayoutingTask(() => {
			const scroll = document.getElementById('spreadsheet-tab-scroll');
			if (!scroll)
				return;

			const maxScroll = scroll.scrollWidth - scroll.clientWidth;
			const position = Math.abs(scroll.scrollLeft);
			const canGoBack = position > 1;
			const canGoForward = position < maxScroll - 1;
			const states = {
				firstrecord: canGoBack,
				prevrecord: canGoBack,
				nextrecord: canGoForward,
				lastrecord: canGoForward,
			};

			const focused = Object.keys(states).find((id) => {
				const item = this.builder._getItemById(this.parentContainer, id);
				return item && item.contains(document.activeElement);
			});

			for (const id in states) {
				this.builder.executeActionImpl(this.parentContainer, {
					'control_id': id,
					'action_type': states[id] ? 'enable' : 'disable'
				});
			}

			if (focused && !states[focused])
				Promise.resolve().then(() => {
					app.layoutingService.appendLayoutingTask(() => this.moveFocusFromNavigation());
				});
		});
	}

	moveFocusFromNavigation() {
		const candidates = ['prevrecord', 'nextrecord', 'firstrecord', 'lastrecord',
			'insertsheet', 'sheetlist'];
		for (const id of candidates) {
			const item = this.builder._getItemById(this.parentContainer, id);
			if (!item || item.hasAttribute('disabled') || item.classList.contains('hidden'))
				continue;
			const button = item.querySelector('button');
			if (button && button.disabled)
				continue;
			(button || item).focus();
			return;
		}
	}

	onUpdatePermission(e) {
		if (e.detail.perm === 'edit') {
			this.enableInsertion(true);
		} else {
			this.enableInsertion(false);
		}
	}

	updateSheetListMenu() {
		let sheetEntries = [];

		if (!this.partNames || this.partNames.length === 0) {
			return sheetEntries;
		}
		
		for (let i = 0; i < this.partNames.length; i++) {
			if (app.calc.isPartHidden(i)) {
				continue;
			}

			if (app.calc.isPartSheetView(i)) {
				continue;
			}

			let displayText = this.partNames[i];

			sheetEntries.push({
				id: 'selectsheet-' + i,
				text: displayText,
				selected: (i === this.selectedPart || app.calc.isDefaultPartOfSelectedSheetView(i))
			});
		}
		
		JSDialog.MenuDefinitions.set('SheetListMenu', sheetEntries);
		this.enableSheetList(sheetEntries.length > 1);
	}

	onSelectSheet(objectType, eventType, object, data, builder) {
		if (object.id === 'sheetlist') {

			if (eventType === 'select') {
				const partIndex = parseInt(data.replace('selectsheet-', ''));
				if (!isNaN(partIndex) && partIndex !== this.map._docLayer._selectedPart) {
					this.map._docLayer._clearReferences();
					this.map.setPart(partIndex, false, true);
				}
			}
			return;
		}
		return builder._defaultCallbackHandler(objectType, eventType, object, data, builder);
  }

	enableInsertion(enable) {
		this.builder.executeAction(this.parentContainer, {
			'control_id': 'insertsheet',
			'action_type': enable ? 'enable' : 'disable'
		});
	}

	enableSheetList(enable) {
		this.builder.executeAction(this.parentContainer, {
			'control_id': 'sheetlist',
			'action_type': enable ? 'enable' : 'disable'
		});
	}

	show() {
		this.parentContainer.style.display = 'grid';
	}

	hide() {
		this.parentContainer.style.display = 'none';
	}
}

JSDialog.SheetsBar = function (map, showNavigation) {
	return new SheetsBar(map, showNavigation);
};
