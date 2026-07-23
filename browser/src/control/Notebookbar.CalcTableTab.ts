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
 * Notebookbar.CalcTableTab.ts
 */

class CalcTableTab implements NotebookbarTab {
	public getName(): string {
		return 'Table';
	}

	public onCallback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		if (object.id === 'tablestyles_design') {
			if (eventType === 'activate') {
				const stylePos = data as number;
				app.tableStyles.applyStyle(stylePos);
			}

			return true;
		}

		return false;
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: 'Table-tab-label',
			text: _('Table Design'),
			name: this.getName(),
			context: 'Table',
			accessibility: {
				focusBack: true,
				combination: 'T',
				de: null,
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	/* ids have to match transition pane ids from the .ui in the core */
	public getContent(): NotebookbarTabContent {
		const content = [
			{
				type: 'overflowgroup',
				id: 'table-properties',
				name: _('Properties'),
				accessibility: { focusBack: true, combination: 'PR' },
				children: [
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'rename-calc-table',
										type: 'toolitem',
										text: _UNO('.uno:RenameCalcTable', 'spreadsheet'),
										command: '.uno:RenameCalcTable',
										accessibility: { focusBack: true, combination: 'RN' },
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'resize-calc-table',
										type: 'toolitem',
										text: _UNO('.uno:ResizeCalcTable', 'spreadsheet'),
										command: '.uno:ResizeCalcTable',
										accessibility: { focusBack: true, combination: 'RS' },
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
				id: 'resize-calc-table-break',
				orientation: 'vertical',
			},
			{
				type: 'overflowgroup',
				id: 'table-tools',
				name: _('Tools'),
				accessibility: { focusBack: true, combination: 'TL' },
				children: [
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'summarize-with-pivottable',
										type: 'toolitem',
										text: _UNO('.uno:DataDataPilotRun', 'spreadsheet'),
										command: '.uno:DataDataPilotRun',
										accessibility: {
											focusBack: true,
											combination: 'PV',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'handle-duplicates',
										type: 'toolitem',
										text: _UNO('.uno:HandleDuplicateRecords', 'spreadsheet'),
										command: '.uno:HandleDuplicateRecords',
										accessibility: {
											focusBack: true,
											combination: 'HD',
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
										id: 'convert-calc-table-to-range',
										type: 'toolitem',
										text: _UNO('.uno:ConvertCalcTableToRange', 'spreadsheet'),
										command: '.uno:ConvertCalcTableToRange',
										accessibility: {
											focusBack: true,
											combination: 'CR',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'insert-remove-calc-table',
										type: 'toolitem',
										text: _UNO('.uno:RemoveCalcTable', 'spreadsheet'),
										command: '.uno:RemoveCalcTable',
										accessibility: {
											focusBack: true,
											combination: 'DT',
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
				id: 'table-tools-break',
				orientation: 'vertical',
			},
			{
				type: 'overflowgroup',
				id: 'table-options',
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
										id: 'chk_header_row2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'ContainsHeader',
										text: _('Header Row'),
										accessibility: {
											focusBack: true,
											combination: 'SH',
										},
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'chk_total_row2',
										type: 'checkbox',
										command: '.uno:TableTotalRow',
										text: _('Total Row'),
										accessibility: {
											focusBack: true,
											combination: 'ST',
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
										id: 'chk_banded_rows2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'UseRowStripes',
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
										id: 'chk_banded_cols2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'UseColStripes',
										text: _('Banded Columns'),
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
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'chk_first_column2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'UseFirstColumnFormatting',
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
										id: 'chk_last_column2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'UseLastColumnFormatting',
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
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'chk_filter_buttons2',
										type: 'checkbox',
										command: '.uno:DatabaseSettings',
										commandField: 'AutoFilter',
										text: _('Filter Buttons'),
										accessibility: { focusBack: true, combination: 'SF' },
									},
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'chk_empty_row',
										type: 'checkbox',
										text: ' ',
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
				id: 'table-options-break',
				orientation: 'vertical',
			},
			app.tableStyles?.generateTableStylesJSON(),
		];

		return content as NotebookbarTabContent;
	}
}

JSDialog.CalcTableTab = new CalcTableTab();
