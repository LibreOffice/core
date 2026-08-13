/* -*- js-indent-level: 8; fill-column: 100 -*- */

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
 * This file contains the service which keeps the "Table Design" notebookbar tab's style gallery
 * in sync with the engine's list of named table design styles. Writer and Impress differ only in
 * the command that applies a style and reports the current one.
 */

interface TableStyleGalleryEntry {
	Name: string;
	Image: string; // data:image/png;base64,... rendered by the engine
}

class TableStyleGalleryService {
	private styles: Array<TableStyleGalleryEntry> = [];
	private currentStyleName: string = '';

	constructor() {
		app.map.on('commandstatechanged', this.onCommandState.bind(this));
	}

	// Writer applies a style with .uno:SetTableStyle rather than .uno:TableStyle: it already has
	// an unrelated, older command named "TableStyle" (the table-styles family in the Manage
	// Styles sidebar), and UNO command lookup resolves by name against whichever slot registered
	// first, so reusing that name would silently dispatch to the wrong command. The document
	// type is known only once the document has loaded, so it is looked up on every call.
	private getApplyCommand(): string {
		return app.map.getDocType() === 'text'
			? '.uno:SetTableStyle'
			: '.uno:TableStyle';
	}

	public onCommandState(e: any) {
		if (e.commandName === this.getApplyCommand()) {
			if (typeof e.state !== 'string') return;
			this.currentStyleName = e.state;
			this.updateTableStylesGallery();
			return;
		}

		if (e.commandName !== '.uno:TableStyleList') return;
		if (!e.state) return;

		try {
			const parsed =
				typeof e.state === 'string' ? JSON.parse(e.state) : e.state;
			this.styles = parsed.TableStyles || [];
		} catch (ex) {
			app.console.error('Failed to parse TableStyleList: ' + ex);
			return;
		}

		this.updateTableStylesGallery();
	}

	private updateTableStylesGallery() {
		app.map.fire('jsdialogupdate', {
			data: {
				id: WindowId.Notebookbar + '',
				type: '',
				jsontype: 'notebookbar',
				action: 'update',
				control: this.generateTableStylesJSON(),
			} as JSDialogJSON,
		});
	}

	public generateTableStylesJSON(): IconViewJSON {
		return {
			id: 'table-design-styles',
			type: 'iconview',
			text: _('Table Styles'),
			aria: { label: _('Table Styles') },
			accessibility: { focusBack: true, combination: 'TL' },
			entries: this.styles.map((style, index) => {
				return {
					row: index,
					text: style.Name,
					// The engine falls back to an empty string per-style if
					// rendering that one style's preview failed - don't let
					// that show as a broken image.
					image: style.Image || 'images/lc_table_none.svg',
					width: 56,
					height: 31,
					selected: style.Name === this.currentStyleName,
				};
			}),
			singleclickactivate: true,
			textWithIconEnabled: !this.styles.some((style) => style.Image),
			selectionmode: 'single',
		} as IconViewJSON;
	}

	public applyStyle(stylePos: number) {
		const style = this.styles[stylePos];
		if (!style) return;

		// The engine's slot names the command's string parameter after the command itself.
		const applyCommand = this.getApplyCommand();
		const argument = applyCommand.substring('.uno:'.length);
		app.map.sendUnoCommand(applyCommand, {
			[argument]: { type: 'string', value: style.Name },
		});
	}
}
