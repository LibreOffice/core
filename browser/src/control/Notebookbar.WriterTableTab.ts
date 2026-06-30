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
 * WriterTableTab - contains JSON for the table tab
 */

class WriterTableTab implements NotebookbarTab {
	public getName(): string {
		return 'Table';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: this.getName() + '-tab-label',
			text: _('Table'),
			name: this.getName(),
			context: 'Table',
			accessibility: {
				focusBack: true,
				combination: 'T',
			},
		};
	}

	public getContent(): NotebookbarTabContent {
		var content: NotebookbarTabContent = [
			{
				type: 'overflowgroup',
				id: 'table-select-group',
				name: _('Select'),
				accessibility: { focusBack: false, combination: 'SE', de: null },
				children: [
					{
						id: 'table-select',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-entire-column',
										text: _UNO('.uno:EntireColumn', 'text'),
										command: '.uno:EntireColumn',
										accessibility: {
											focusBack: true,
											combination: 'CE',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-select-table',
										text: _UNO('.uno:SelectTable', 'text', true),
										command: '.uno:SelectTable',
										accessibility: {
											focusBack: true,
											combination: 'ST',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-entire-row',
										text: _UNO('.uno:EntireRow', 'text'),
										command: '.uno:EntireRow',
										accessibility: {
											focusBack: true,
											combination: 'ER',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-delete-table',
										text: _UNO('.uno:DeleteTable', 'text', true),
										command: '.uno:DeleteTable',
										accessibility: {
											focusBack: true,
											combination: 'DT',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-deletetable-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-insert',
				name: _('Insert'),
				accessibility: { focusBack: false, combination: 'TI', de: null },
				children: [
					{
						id: 'table-insert-container',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-insert-columns-before',
										text: _UNO('.uno:InsertColumnsBefore', 'text', true),
										command: '.uno:InsertColumnsBefore',
										accessibility: {
											focusBack: true,
											combination: 'CB',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-insert-columns-after',
										text: _UNO('.uno:InsertColumnsAfter', 'text', true),
										command: '.uno:InsertColumnsAfter',
										accessibility: {
											focusBack: true,
											combination: 'CA',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-insert-delete-columns',
										text: _UNO('.uno:DeleteColumns', 'text', true),
										command: '.uno:DeleteColumns',
										accessibility: {
											focusBack: true,
											combination: 'CD',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-insert-rows-before',
										text: _UNO('.uno:InsertRowsBefore', 'text', true),
										command: '.uno:InsertRowsBefore',
										accessibility: {
											focusBack: true,
											combination: 'RB',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-insert-rows-after',
										text: _UNO('.uno:InsertRowsAfter', 'text', true),
										command: '.uno:InsertRowsAfter',
										accessibility: {
											focusBack: true,
											combination: 'RA',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-insert-delete-rows',
										text: _UNO('.uno:DeleteRows', 'text', true),
										command: '.uno:DeleteRows',
										accessibility: {
											focusBack: true,
											combination: 'RD',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-deleterows-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-split',
				name: _('Merge & Split'),
				children: [
					{
						type: 'bigtoolitem',
						id: 'table-insert-merge-cells',
						text: _UNO('.uno:MergeCells', 'text'),
						command: '.uno:MergeCells',
						accessibility: { focusBack: true, combination: 'MC', de: null },
					} as ToolItemWidgetJSON,
					{
						id: 'table-split-container',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-insert-split-cell',
										text: _UNO('.uno:SplitCell', 'text'),
										command: '.uno:SplitCell',
										accessibility: {
											focusBack: true,
											combination: 'SC',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-insert-split-table',
										text: _UNO('.uno:SplitTable', 'text'),
										command: '.uno:SplitTable',
										accessibility: {
											focusBack: true,
											combination: 'TS',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-splittable-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-alignment',
				name: _('Alignment'),
				accessibility: { focusBack: true, combination: 'CT', de: null },
				children: [
					{
						id: 'table-alignment-container',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-cell-vert-top',
										text: _UNO('.uno:CellVertTop'),
										command: '.uno:CellVertTop',
										accessibility: {
											focusBack: true,
											combination: 'CT',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-cell-vert-center',
										text: _UNO('.uno:CellVertCenter'),
										command: '.uno:CellVertCenter',
										accessibility: {
											focusBack: true,
											combination: 'CC',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-cell-vert-bottom',
										text: _UNO('.uno:CellVertBottom'),
										command: '.uno:CellVertBottom',
										accessibility: {
											focusBack: true,
											combination: 'CM',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-left-para',
										text: _UNO('.uno:LeftPara'),
										command: '.uno:LeftPara',
										accessibility: {
											focusBack: true,
											combination: 'PL',
											de: null,
										},
									},
									{
										type: 'toolitem',
										id: 'table-center-para',
										text: _UNO('.uno:CenterPara'),
										command: '.uno:CenterPara',
										accessibility: {
											focusBack: true,
											combination: 'PC',
											de: null,
										},
									},
									{
										type: 'toolitem',
										id: 'table-right-para',
										text: _UNO('.uno:RightPara'),
										command: '.uno:RightPara',
										accessibility: {
											focusBack: true,
											combination: 'PR',
											de: null,
										},
									},
									{
										type: 'toolitem',
										id: 'table-justify-para',
										text: _UNO('.uno:JustifyPara'),
										command: '.uno:JustifyPara',
										accessibility: {
											focusBack: true,
											combination: 'PJ',
											de: null,
										},
									},
								],
							},
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-justifypara-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
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
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-bigtoolitem-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-number-format',
				name: _('Format'),
				accessibility: { focusBack: true, combination: 'FO', de: null },
				more: {
					command: '.uno:TableNumberFormatDialog',
					accessibility: { focusBack: true, combination: 'MN', de: null },
				},
				children: [
					{
						id: 'table-number-format-container',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-number-format-currency',
										text: _UNO('.uno:NumberFormatCurrency', 'text'),
										command: '.uno:NumberFormatCurrency',
										accessibility: {
											focusBack: true,
											combination: 'FC',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										type: 'toolitem',
										id: 'table-number-format-percent',
										text: _UNO('.uno:NumberFormatPercent', 'text', true),
										command: '.uno:NumberFormatPercent',
										accessibility: {
											focusBack: true,
											combination: 'NP',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolitem',
								id: 'table-number-format-date',
								text: _UNO('.uno:NumberFormatDate', 'text', false),
								command: '.uno:NumberFormatDate',
								accessibility: { focusBack: true, combination: 'DA', de: null },
							} as ToolItemWidgetJSON,
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-unsetcellsreadonly-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-sort-group',
				name: _('Sort'),
				accessibility: { focusBack: true, combination: 'SO', de: null },
				children: [
					{
						type: 'bigtoolitem',
						id: 'table-table-sort',
						text: _UNO('.uno:TableSort', 'text'),
						command: '.uno:TableSort',
						accessibility: { focusBack: true, combination: 'SO', de: null },
					} as ToolItemWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-sort-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'overflowgroup',
				id: 'table-protect-group',
				name: _('Protect'),
				accessibility: { focusBack: true, combination: 'PR', de: null },
				children: [
					{
						id: 'table-protection-controls',
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-protect-cell',
										text: _UNO('.uno:Protect', 'text'),
										command: '.uno:Protect',
										accessibility: {
											focusBack: true,
											combination: 'PP',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
							{
								type: 'toolbox',
								children: [
									{
										type: 'toolitem',
										id: 'table-unset-cells-read-only',
										text: _UNO('.uno:UnsetCellsReadOnly', 'text'),
										command: '.uno:UnsetCellsReadOnly',
										accessibility: {
											focusBack: true,
											combination: 'UP',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							} as ToolboxWidgetJSON,
						],
						vertical: true,
					} as ContainerWidgetJSON,
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'table-numberformatpercent-break',
				orientation: 'vertical',
			} as SeparatorWidgetJSON,
			{
				type: 'bigtoolitem',
				id: 'table-insert-caption-dialog',
				text: _UNO('.uno:InsertCaptionDialog', 'text'),
				command: '.uno:InsertCaptionDialog',
				accessibility: { focusBack: true, combination: 'IC', de: null },
			} as ToolItemWidgetJSON,
		];
		return content;
	}
}

JSDialog.WriterTableTab = new WriterTableTab();
