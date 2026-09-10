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
	Name: string; // the style's name in the document model, the key for applying it
	DisplayName?: string; // the name in the UI language, if it differs from Name
	Image: string; // data:image/png;base64,... rendered by the engine
}

// A section title between two runs of styles in the list the engine sends.
interface TableStyleGallerySeparator {
	Separator: true;
	Title: string;
}

type TableStyleGalleryListItem =
	| TableStyleGalleryEntry
	| TableStyleGallerySeparator;

class TableStyleGalleryService {
	// The styles alone, in list order; the row index of a gallery entry indexes this array.
	private styles: Array<TableStyleGalleryEntry> = [];
	// The list as the engine sent it: the styles with the section titles between them.
	private listItems: Array<TableStyleGalleryListItem> = [];
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
			this.listItems = parsed.TableStyles || [];
			this.styles = this.listItems.filter(
				(item): item is TableStyleGalleryEntry => !('Separator' in item),
			);
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
			entries: this.generateEntries(),
			singleclickactivate: true,
			textWithIconEnabled: !this.styles.some((style) => style.Image),
			selectionmode: 'single',
		} as IconViewJSON;
	}

	// A section title becomes a separator entry without a row. A style's row is its index in
	// the styles array, which is what the selection callback reports back.
	private generateEntries(): Array<any> {
		let row = 0;
		// A used style is listed twice, first among the styles the document uses and again in
		// its catalog section, and only the first entry shows as selected.
		let selectedFound = false;
		return this.listItems.map((item) => {
			if ('Separator' in item) {
				return { separator: true, text: item.Title };
			}
			const isSelected = !selectedFound && item.Name === this.currentStyleName;
			if (isSelected) selectedFound = true;
			return {
				row: row++,
				text: item.DisplayName || item.Name,
				// The engine falls back to an empty string per-style if
				// rendering that one style's preview failed - don't let
				// that show as a broken image.
				image: item.Image || 'images/lc_table_none.svg',
				width: 56,
				height: 31,
				selected: isSelected,
			};
		});
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
