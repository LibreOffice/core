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
 * JSDialog.AddressInputField - implementation of address input field in calc
 */

declare var JSDialog: any;

class AddressInputField {
	map: any;
	parentContainer: HTMLElement;
	builder: any;
	data: any;

	public constructor(map: any) {
		this.map = map;
		this.parentContainer = window.L.DomUtil.get('addressInput');

		this.map.on('jsdialogupdate', this.onJSUpdate, this);
		this.map.on('jsdialogaction', this.onJSAction, this);
		this.map.on('doclayerinit', this.onDocLayerInit, this);
		this.map.on('celladdress', this.onCellAddress, this);

		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			cssClass: 'addressInput jsdialog',
			windowId: WindowId.AddressInput,
			suffix: 'address',
		});

		this.createAddressInputField();
	}

	public onRemove() {
		this.map.off('jsdialogupdate', this.onJSUpdate, this);
		this.map.off('jsdialogaction', this.onJSAction, this);
		this.map.off('doclayerinit', this.onDocLayerInit, this);
		this.map.off('celladdress', this.onCellAddress, this);
	}

	private onCellAddress(event: any) {
		const addressInput = document.querySelector<HTMLInputElement>(
			'#addressInput input',
		);
		if (addressInput && document.activeElement !== addressInput) {
			// if the user is not editing the address field
			addressInput.value = event.address;
			addressInput.setAttribute('aria-label', event.address);
		}
		this.map.formulabarSetDirty();
	}

	private createAddressInputField() {
		this.data = [
			{
				id: 'pos_window',
				type: 'combobox',
				text: _('cell address'),
				enabled: true,
				changeOnEnterOnly: true,
				focusMapOnEnter: true,
				accessibility: { focusBack: true, combination: 'CA' },
				children: [
					{
						id: 'expand',
						type: 'pushbutton',
						text: '',
						symbol: 'SPIN_DOWN',
					},
					{
						id: '',
						type: 'edit',
						text: '',
						enabled: true,
					},
					{
						id: '',
						type: 'borderwindow',
						text: '',
						enabled: true,
						children: [
							{
								type: 'edit',
								id: '',
								text: '',
								enabled: true,
							},
						],
					},
				],
				selectedEntries: [],
				selectedCount: 0,
			},
		];
		this.parentContainer.replaceChildren();
		this.builder.build(this.parentContainer, this.data);
	}

	public getAddressItems(): any[] {
		return this.data;
	}

	private onJSUpdate(e: any) {
		const data = e.data;
		if (data.jsontype !== 'addressinputfield') return;

		// we don't want to send change event on every keypress,
		// otherwise core will keep on adding new named range on every keypress.
		// we want to create a named range only when user presses the 'Enter' key.
		data.control.changeOnEnterOnly = true;
		data.control.focusMapOnEnter = true;
		this.builder.updateWidget(this.parentContainer, data.control);
	}

	private onDocLayerInit() {
		var docType = this.map.getDocType();
		if (docType == 'spreadsheet' && this.parentContainer) {
			this.parentContainer.style.setProperty('display', 'table-cell');
		}
	}

	private onJSAction(e: any) {
		const data = e.data;
		if (data.jsontype !== 'addressinputfield') return;

		const innerData = data.data;
		this.builder.executeAction(this.parentContainer, innerData);
	}
}

JSDialog.AddressInputField = function (map: any) {
	return new AddressInputField(map);
};
