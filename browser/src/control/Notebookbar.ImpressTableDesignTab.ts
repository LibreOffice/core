/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * Notebookbar.ImpressTableDesignTab.ts
 */

class ImpressTableDesignTab extends TableDesignTabBase {
	protected getShortcut(): string {
		return 'TD';
	}

	protected getDesignGroup(): any {
		return {
			type: 'overflowgroup',
			id: 'table-design-group',
			name: _('Design'),
			accessibility: { focusBack: true, combination: 'SD' },
			more: {
				command: '.uno:TableDialog',
				accessibility: { focusBack: true, combination: 'MT' },
			},
			children: [
				{
					id: 'table-design-table-dialog',
					type: 'bigtoolitem',
					text: _UNO('.uno:TableDialog', 'presentation', true),
					command: '.uno:TableDialog',
					accessibility: { focusBack: false, combination: 'SD' },
				},
				{
					type: 'container',
					children: [
						{
							type: 'toolbox',
							children: [
								{
									id: 'table-design-xline-color:ColorPickerMenu',
									type: 'menubutton',
									noLabel: true,
									text: _('Borders'),
									command: '.uno:XLineColor',
									accessibility: {
										focusBack: true,
										combination: 'BL',
									},
								},
							],
						},
						{
							type: 'toolbox',
							children: [
								{
									id: 'table-design-fill-color:ColorPickerMenu',
									type: 'menubutton',
									noLabel: true,
									text: _('Cell Background'),
									command: '.uno:FillColor',
									accessibility: {
										focusBack: true,
										combination: 'BC',
									},
								},
							],
						},
					],
					vertical: true,
				},
			],
		};
	}
}

JSDialog.ImpressTableDesignTab = new ImpressTableDesignTab();
