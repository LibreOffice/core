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

/**
 * A dialog with the table style gallery, for the classic UI where the notebookbar is not
 * there. A click picks a style and OK applies it, a double click applies it at once. In Calc
 * the dialog also creates a new style or clears the style, and a right click on a style
 * offers to make it the default.
 */
class TableStylesDialog {
	private readonly id = 'TableStylesDialog';
	// The gallery keeps the id it has in the notebookbar, so it takes the same layout.
	private readonly iconViewId: string;
	private readonly isCalc: boolean;
	private selectedRow: number | null = null;

	constructor() {
		this.isCalc = app.map.getDocType() === 'spreadsheet';
		this.iconViewId =
			this.isCalc ? 'tablestyles_design' : 'table-design-styles';
	}

	private getIconViewJSON(): IconViewJSON {
		const gallery =
			this.isCalc ?
				app.tableStyles.generateIconViewJSON()
			:	app.tableStyleGallery.generateTableStylesJSON();
		return {
			...gallery,
			singleclickactivate: false,
			accessibility: undefined,
		} as IconViewJSON;
	}

	private getButton(
		id: string,
		text: string,
		command: string,
	): PushButtonWidget {
		return {
			id: id,
			type: 'pushbutton',
			text: text,
			enabled:
				app.map['stateChangeHandler'].getItemValue(command) !== 'disabled',
		} as PushButtonWidget;
	}

	private getJSON(): JSDialogJSON {
		const children: Array<WidgetJSON> = [this.getIconViewJSON()];
		if (this.isCalc)
			children.push({
				id: this.id + '-actions',
				type: 'container',
				vertical: false,
				children: [
					this.getButton('new', _('New Table Style...'), '.uno:NewTableStyle'),
					this.getButton('clear', _('Clear Style'), '.uno:ClearTableStyle'),
				],
			} as ContainerWidgetJSON);

		children.push({
			id: this.id + '-buttonbox',
			type: 'buttonbox',
			layoutstyle: 'end',
			children: [
				{
					id: 'cancel',
					type: 'pushbutton',
					text: _('Cancel'),
				} as PushButtonWidget,
				{
					id: 'ok',
					type: 'pushbutton',
					text: _('OK'),
					has_default: true,
				} as PushButtonWidget,
			],
		} as ButtonBoxWidget);

		return {
			id: this.id,
			dialogid: this.id,
			type: 'dialog',
			text: _('Table Styles'),
			title: _('Table Styles'),
			jsontype: 'dialog',
			responses: [
				{ id: 'ok', response: 1 },
				{ id: 'cancel', response: 0 },
			],
			children: [
				{
					id: this.id + '-mainbox',
					type: 'container',
					vertical: true,
					children: children,
				} as ContainerWidgetJSON,
			],
		} as JSDialogJSON;
	}

	private applyStyle(row: number): void {
		if (this.isCalc) app.tableStyles.applyStyle(row);
		else app.tableStyleGallery.applyStyle(row);
	}

	private close(): void {
		app.map.fire('jsdialog', { data: { action: 'close', id: this.id } });
	}

	private callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
	): void {
		if (object.id === this.iconViewId) {
			if (eventType === 'select') {
				this.selectedRow = Number(data);
			} else if (eventType === 'activate') {
				this.close();
				this.applyStyle(Number(data));
			} else if (eventType === 'keypress' && data === UNOKey.RETURN) {
				this.close();
				if (this.selectedRow !== null) this.applyStyle(this.selectedRow);
			} else if (eventType === 'contextmenu' && this.isCalc) {
				app.tableStyles.showSetDefaultMenu(data as IconViewContextMenuData);
			}
			return;
		}

		if (object.id === 'new') {
			this.close();
			app.map.sendUnoCommand('.uno:NewTableStyle');
		} else if (object.id === 'clear') {
			this.close();
			app.map.sendUnoCommand('.uno:ClearTableStyle');
		} else if (object.id === 'ok') {
			this.close();
			if (this.selectedRow !== null) this.applyStyle(this.selectedRow);
		} else if (object.id === 'cancel') {
			this.close();
		}
	}

	public open(): void {
		app.map.fire('jsdialog', {
			data: this.getJSON(),
			callback: this.callback.bind(this) as JSDialogCallback,
		});
	}
}
