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
 * Notebookbar.ImpressTableDesignTab.ts
 */

class ImpressTableDesignTab implements NotebookbarTab {
	public getName(): string {
		return 'TableDesign';
	}

	public onCallback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		if (object.id === 'table-design-styles') {
			if (eventType === 'activate') {
				const stylePos = Number(data);
				app.impressTableStyles.applyStyle(stylePos);
			}

			return true;
		}

		return false;
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: 'TableDesign-tab-label',
			text: _('Table Design'),
			name: this.getName(),
			context: 'Table',
			accessibility: {
				focusBack: true,
				combination: 'TD',
				de: null,
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	public getContent(): NotebookbarTabContent {
		const content = [
			{
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
						vertical: 'true',
					},
				],
			},
			{
				type: 'separator',
				id: 'table-design-group-break',
				orientation: 'vertical',
			},
			{
				type: 'overflowgroup',
				id: 'table-design-options',
				name: _('Table Options'),
				accessibility: { focusBack: true, combination: 'TO' },
				children: [
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-header-row',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseFirstRowStyle',
										commandFieldFlat: true,
										text: _('Header Row'),
										accessibility: {
											focusBack: true,
											combination: 'HR',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-total-row',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseLastRowStyle',
										commandFieldFlat: true,
										text: _('Total Row'),
										accessibility: {
											focusBack: true,
											combination: 'TR',
										},
									},
								],
							},
						],
						vertical: 'true',
					},
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-banded-rows',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseBandingRowStyle',
										commandFieldFlat: true,
										text: _('Banded Rows'),
										accessibility: {
											focusBack: true,
											combination: 'BR',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-banded-columns',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseBandingColumnStyle',
										commandFieldFlat: true,
										text: _('Banded Columns'),
										accessibility: {
											focusBack: true,
											combination: 'BD',
										},
									},
								],
							},
						],
						vertical: 'true',
					},
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-first-column',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseFirstColumnStyle',
										commandFieldFlat: true,
										text: _('First Column'),
										accessibility: {
											focusBack: true,
											combination: 'FC',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'table-design-last-column',
										type: 'checkbox',
										command: '.uno:TableStyleSettings',
										commandField: 'UseLastColumnStyle',
										commandFieldFlat: true,
										text: _('Last Column'),
										accessibility: {
											focusBack: true,
											combination: 'LC',
										},
									},
								],
							},
						],
						vertical: 'true',
					},
				],
			},
			{
				type: 'separator',
				id: 'table-design-options-break',
				orientation: 'vertical',
			},
			{
				id: 'table-design-styles-group',
				type: 'overflowgroup',
				name: _('Table Styles'),
				nofold: true,
				children: [
					{
						id: 'table-design-styles-iconview-list',
						type: 'iconviewlist',
						accessibility: { focusBack: false, combination: 'TS' },
						children: [
							app.impressTableStyles?.generateTableStylesJSON() ?? {
								id: 'table-design-styles',
								type: 'iconview',
								text: _('Table Styles'),
								aria: { label: _('Table Styles') },
								entries: [],
								singleclickactivate: true,
								textWithIconEnabled: true,
								selectionmode: 'single',
							},
						],
					},
				],
			},
		];

		return content as NotebookbarTabContent;
	}
}

JSDialog.ImpressTableDesignTab = new ImpressTableDesignTab();
