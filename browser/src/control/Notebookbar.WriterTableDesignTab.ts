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
 * Notebookbar.WriterTableDesignTab.ts
 */

class WriterTableDesignTab extends TableDesignTabBase {
	// Impress reaches this tab with TD. Writer's Table tab is reached with T alone, and the keys
	// of a tab shortcut are matched one at a time, so TD would open the Table tab instead.
	protected getShortcut(): string {
		return 'DS';
	}

	protected getDesignGroup(): any {
		return {
			type: 'overflowgroup',
			id: 'table-design',
			name: _('Design'),
			accessibility: { focusBack: true, combination: 'SD', de: null },
			more: {
				command: '.uno:TableDialog',
				accessibility: { focusBack: true, combination: 'MT', de: null },
			},
			children: [
				{
					id: 'table-table-dialog',
					type: 'bigtoolitem',
					text: _UNO('.uno:TableDialog', 'text', true),
					command: '.uno:TableDialog',
					accessibility: { focusBack: false, combination: 'SD', de: null },
				},
				{
					id: 'table-line-style-container',
					type: 'container',
					children: [
						{
							type: 'toolbox',
							children: [
								{
									type: 'menubutton',
									id: 'set-border-style:BorderStyleMenuWriter',
									noLabel: true,
									text: _('Borders'),
									command: '.uno:SetBorderStyle',
									accessibility: {
										focusBack: true,
										combination: 'BL',
										de: null,
									},
								} as MenuButtonWidgetJSON,
							],
						} as ToolboxWidgetJSON,
						{
							type: 'toolbox',
							children: [
								{
									type: 'menubutton',
									id: 'table-xline-color:ColorPickerMenu',
									noLabel: true,
									text: _('Cell Background'),
									command: '.uno:TableCellBackgroundColor',
									accessibility: {
										focusBack: true,
										combination: 'BC',
										de: null,
									},
								} as MenuButtonWidgetJSON,
							],
						} as ToolboxWidgetJSON,
					],
					vertical: true,
				} as ContainerWidgetJSON,
			],
		} as OverflowGroupWidgetJSON;
	}
}

JSDialog.WriterTableDesignTab = new WriterTableDesignTab();
