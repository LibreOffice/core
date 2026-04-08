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
 * window.L.Control.NotebookbarCalc - definition of notebookbar content in Calc
 */

/* global _ _UNO app JSDialog */
window.L.Control.NotebookbarCalc = window.L.Control.NotebookbarWriter.extend({

	onCallback: function(objectType, eventType, object, data, builder) {
		const consumed
			= JSDialog.CalcTableTab.onCallback(objectType, eventType, object, data, builder);
		return consumed;
	},

	getTabs: function() {
		return [
			{
				'id': 'File-tab-label',
				'text': _('File'),
				'name': 'File',
				'accessibility': { focusBack: true,	combination: 'F', de: null }
			},
			{
				'id': this.HOME_TAB_ID,
				'text': _('Home'),
				'name': 'Home',
				'context': 'default|Cell|Text|DrawText',
				'accessibility': { focusBack: true,	combination: 'H', de: null }
			},
			{
				'id': 'Insert-tab-label',
				'text': _('Insert'),
				'name': 'Insert',
				'accessibility': { focusBack: true,	combination: 'N', de: null }
			},
			{
				'id': 'Layout-tab-label',
				'text': _('Page Layout'),
				'name': 'Layout',
				'accessibility': { focusBack: true,	combination: 'P', de: null }
			},
			{
				'id': 'Formula-tab-label',
				'text': _('Formulas'),
				'name': 'Formula',
				'accessibility': { focusBack: true,	combination: 'M', de: null }
			},
			{
				'id': 'Data-tab-label',
				'text': _('Data'),
				'name': 'Data',
				'accessibility': { focusBack: true,	combination: 'A', de: null }
			},
			{
				'id': 'Review-tab-label',
				'text': _('Review'),
				'name': 'Review',
				'accessibility': { focusBack: true,	combination: 'R', de: null }
			},
			{
				'id': 'Format-tab-label',
				'text': _('Format'),
				'name': 'Format',
				'accessibility': { focusBack: true,	combination: 'O', de: null }
			},
			{
				'id': 'Shape-tab-label',
				'text': _('Shape'),
				'name': 'Shape',
				'context': 'Draw|DrawLine|3DObject|MultiObject|DrawFontwork',
				'accessibility': { focusBack: true,	combination: 'D', de: null }
			},
			{
				'id': 'Picture-tab-label',
				'text': _('Picture'),
				'name': 'Picture',
				'context': 'Graphic',
				'accessibility': { focusBack: true,	combination: 'G', de: null }
			},
			{
				'id': 'Chart-tab-label',
				'text': _('Chart'),
				'name': 'Chart',
				'context': 'Chart|Series|ErrorBar|Axis|Grid|ChartElements|Trendline|ChartTitle|ChartLegend|ChartLabel',
				'accessibility': { focusBack: true,	combination: 'CH', de: null }
			},
			{
				'id': 'View-tab-label',
				'text': _('View'),
				'name': 'View',
				'accessibility': { focusBack: true,	combination: 'W', de: null }
			},
			{
				'id': 'Sparkline-tab-label',
				'text': _('Sparkline'),
				'name': 'Sparkline',
				'context': 'Sparkline',
				'accessibility': { focusBack: true,	combination: 'K', de: null }
			},
			JSDialog.CalcTableTab.getEntry(),
			{
				'id': 'Help-tab-label',
				'text': _('Help'),
				'name': 'Help',
				'accessibility': { focusBack: true,	combination: 'Y', de: null }
			}
		];
	},

	getTabsJSON: function () {
		return [
			this.getFileTab(),
			this.getHomeTab(),
			this.getInsertTab(),
			this.getLayoutTab(),
			this.getFormulaTab(),
			this.getDataTab(),
			this.getReviewTab(),
			this.getFormatTab(),
			this.getShapeTab(),
			this.getPictureTab(),
			this.getChartTab(),
			this.getViewTab(),
			this.getSparklineTab(),
			this.getCalcTableTab(),
			this.getHelpTab()
		]
	},

	getFullJSON: function (selectedId) {
		return this.getNotebookbar(this.getTabsJSON(), selectedId);
	},

	getFileTab: function() {
		var content = [];
		var hasSave = !this.map['wopi'].HideSaveOption;
		var hasSaveAs = !this.map['wopi'].UserCanNotWriteRelative;
		var hasShare = this.map['wopi'].EnableShare;
		var hasRevisionHistory = window.L.Params.revHistoryEnabled;

		if (hasSave) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'file-save',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Save'),
						'command': '.uno:Save',
						'accessibility': { focusBack: true,	combination: 'SV', de: null }
					}
				]
			});
		}

		if (hasSaveAs) {
			content.push(
				(window.prefs.get('saveAsMode') === 'group') ? {
					'id': 'saveas:SaveAsMenu',
					'command': 'saveas',
					'class': 'unosaveas',
					'type': 'exportmenubutton',
					'text': _('Save As'),
					'accessibility': { focusBack: true,	combination: 'A', de: null }
				}:
				{
					'id': 'file-saveas',
					'type': 'bigtoolitem',
					'text': _UNO('.uno:SaveAs', 'spreadsheet'),
					'command': '.uno:SaveAs',
					'accessibility': { focusBack: true,	combination: 'A', de: null }
				}
			);
		}

		if (hasSaveAs) {
			content.push({
				'id': 'exportas:ExportAsMenu',
				'command': 'exportas',
				'class': 'unoexportas',
				'type': 'exportmenubutton',
				'text': _('Export As'),
				'accessibility': { focusBack: true,	combination: 'E', de: null }
			});
		}

		if (hasShare && hasRevisionHistory) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}, {
					'type': 'container',
					'children': [
						{
							'id': 'ShareAs',
							'class': 'unoShareAs',
							'type': 'customtoolitem',
							'text': _('Share'),
							'command': 'shareas',
							'inlineLabel': true,
							'accessibility': { focusBack: true, combination: 'SH' }
						}, {
							'id': 'Rev-History',
							'class': 'unoRev-History',
							'type': 'customtoolitem',
							'text': _('See history'),
							'command': 'rev-history',
							'inlineLabel': true,
							'accessibility': { focusBack: true, combination: 'RH' }
						}
					],
					'vertical': true
				}, {
					'id': 'file-revhistory-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		} else if (hasShare) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}, {
					'type': 'container',
					'children': [
						{
							'id': 'ShareAs',
							'class': 'unoShareAs',
							'type': 'bigcustomtoolitem',
							'text': _('Share'),
							'command': 'shareas',
							'accessibility': { focusBack: true, combination: 'SH' }
						}
					]
				}, {
					'id': 'file-shareas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		} else if (hasRevisionHistory) {
			content.push({
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				}, {
					'type': 'container',
					'children': [
						{
							'id': 'Rev-History',
							'class': 'unoRev-History',
							'type': 'bigcustomtoolitem',
							'text': _('See history'),
							'command': 'rev-history',
							'accessibility': { focusBack: true, combination: 'RH' }
						},
					]
				}, {
					'id': 'file-revhistory-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		}

		if (!this.map['wopi'].HidePrintOption) {
			content.push({
				'id': 'Data-Print:Print',
				'type': 'menubutton',
				'text': _UNO('.uno:Print', 'spreadsheet'),
				'command': '.uno:Print',
				'applyCallback': 'print',
				'accessibility': { focusBack: true,	combination: 'PT', de: null }
			});
		}

		if (window.enableMacrosExecution) {
			content.push({
				'type': 'toolbox',
				'children': [
					{
						'id': 'runmacro',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:RunMacro', 'text'),
						'command': '.uno:RunMacro',
						'accessibility': { focusBack: true,	combination: 'M', de: null }
					}
				]
			});
		}

		if (!this.map['wopi'].HideExportOption) {
			content.push({
				'id': 'downloadas:DownloadAsMenu',
				'command': 'downloadas',
				'class': 'unodownloadas',
				'type': 'exportmenubutton',
				'text': !window.ThisIsAMobileApp ? _('Download') :
					(window.ThisIsTheWindowsApp ? _('Export as') :
					 _('Save As')),
				'accessibility': { focusBack: true,	combination: 'DA', de: null }
			});
		}

		content.push( { type: 'separator', id: 'file-downloadas-break', orientation: 'vertical' } );

		if (!this.map['wopi'].HideRepairOption) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'repair',
						'class': 'unorepair',
						'type': 'bigcustomtoolitem',
						'text': _('Repair'),
						'accessibility': { focusBack: true,	combination: 'RP', de: null }
					}
				],
				'vertical': 'true'
			});
		}

		content.push(
			{
				'type': 'container',
				'children': [
					{
						'id': 'properties',
						'type': 'bigtoolitem',
						'text': _('Properties'),
						'command': '.uno:SetDocumentProperties',
						'accessibility': { focusBack: true,	combination: 'FP', de: 'I' }
					}
				]
		});
		if (window.documentSigningEnabled) {
			content.push({
				'type': 'container',
				'children': [
					{
						'id': 'signature',
						'type': 'bigtoolitem',
						'text': _('Signature'),
						'command': '.uno:Signature',
						'accessibility': { focusBack: true, combination: 'GN' }
					}
				]
			});
		}
		if (this._map['wopi']._supportsRename() && this._map['wopi'].UserCanRename) {
			content.push(
				{
					'type': 'container',
					'children': [
						{
							'id': 'renamedocument',
							'class': 'unoRenameDocument',
							'type': 'bigcustomtoolitem',
							'text': _('Rename'),
							'accessibility': { focusBack: true, combination: 'RN', de: null }
						}
					]
				}
			);
		}

		if (window.wopiSettingBaseUrl) {
			content.push({
				'type': 'separator',
				'id': 'file-properties-break',
				'orientation': 'vertical'
			});

			content.push({
				'id': 'settings-dialog',
				'type': 'bigtoolitem',
				'text': _('Options'),
				'command': '.uno:Settings',
				'accessibility': { focusBack: false, combination: 'T', de: null }
			});
		}

		return this.getTabPage('File', content);
	},

	getHomeTab: function() {
		var content = [
			{
				'id': 'home-do',
				'type': 'container',
				'children': [
					{
						'id': 'home-undo',
						'type': 'toolitem',
						'text': _UNO('.uno:Undo'),
						'command': '.uno:Undo',
						'accessibility': { focusBack: true,	combination: 'ZZ', de: null }
					},
					{
						'id': 'home-redo',
						'type': 'toolitem',
						'text': _UNO('.uno:Redo'),
						'command': '.uno:Redo',
						'accessibility': { focusBack: true,	combination: 'O', de: null }
					},
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-undoredo-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-clipboard',
				'name': _('Clipboard'),
				'accessibility': { focusBack: true,	combination: 'V', de: null },
				'children' :
				[
					{
						'id': 'home-paste:PasteMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:Paste'),
						'command': '.uno:Paste',
						'accessibility': { focusBack: true,	combination: 'V', de: null }
					},
					{
					'type': 'container',
					'children': [
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-cut',
									'type': 'customtoolitem',
									'text': _UNO('.uno:Cut', true),
									'command': '.uno:Cut',
									'accessibility': { focusBack: true,	combination: 'X', de: null }
								},
								{
									'id': 'home-format-paint-brush',
									'type': 'toolitem',
									'text': _UNO('.uno:FormatPaintbrush'),
									'tooltip': _('Clone Formatting (double click to keep active)'),
									'activeTooltip': _('Clone Formatting is active (click again or press Esc to exit)'),
									'command': '.uno:FormatPaintbrush',
									'doubleClickCommand': '.uno:FormatPaintbrush',
									'doubleClickCommandArgs': { PersistentCopy: { type: 'boolean', value: true } },
									'accessibility': { focusBack: true,	combination: 'FP', de: null }
								}
							]
						},
						{
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-copy',
									'type': 'customtoolitem',
									'text': _UNO('.uno:Copy', true),
									'command': '.uno:Copy',
									'accessibility': { focusBack: true,	combination: 'C', de: null }
								},
								{
									'id': 'home-reset-attributes',
									'type': 'toolitem',
									'text': _UNO('.uno:ResetAttributes'),
									'command': '.uno:ResetAttributes',
									'accessibility': { focusBack: true,	combination: 'E', de: null }
								}
							]
						}
					],
					'vertical': 'true'
					}
				]
			},
			{ type: 'separator', id: 'home-resertattributes-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-font',
				'name': _('Font'),
				'accessibility': { focusBack: true,	combination: 'FF', de: null },
				'more': {
					'command':'.uno:CellTextDlg',
					'accessibility': { focusBack: true,	combination: 'MF', de: null },
				},
				'children': [
				{
					'id': 'Home-Section-Format',
					'type': 'container',
					'children': [
						{
							'id': 'box76',
							'type': 'container',
							'children': [
								{
									'id': 'fontnamecombobox',
									'type': 'combobox',
									'text': 'Carlito',
									'entries': [
										'Carlito'
									],
									'selectedCount': '1',
									'selectedEntries': [
										'71'
									],
									'command': '.uno:CharFontName',
									'accessibility': { focusBack: true,	combination: 'FF', de: null }
								},
								{
									'id': 'fontsizecombobox',
									'type': 'combobox',
									'text': '10 pt',
									'entries': [
										'10 pt'
									],
									'selectedCount': '1',
									'selectedEntries': [
										'4'
									],
									'command': '.uno:FontHeight',
									'accessibility': { focusBack: true,	combination: 'FS', de: null }
								},
								{
									'id': 'home-grow',
									'type': 'toolitem',
									'text': _UNO('.uno:Grow'),
									'command': '.uno:Grow',
									'accessibility': { focusBack: true,	combination: 'FG', de: null }
								},
								{
									'id': 'home-shrink',
									'type': 'toolitem',
									'text': _UNO('.uno:Shrink'),
									'command': '.uno:Shrink',
									'accessibility': { focusBack: true,	combination: 'FK', de: null }
								}
							],
							'vertical': 'false'
						},
						{
							'id': 'GroupB11',
							'type': 'container',
							'children': [
								{
									'id': 'home-bold',
									'type': 'toolitem',
									'text': _UNO('.uno:Bold'),
									'command': '.uno:Bold',
									'accessibility': { focusBack: true,	combination: '1', de: null }
								},
								{
									'id': 'home-italic',
									'type': 'toolitem',
									'text': _UNO('.uno:Italic'),
									'command': '.uno:Italic',
									'accessibility': { focusBack: true,	combination: '2', de: null }
								},
								{
									'id': 'home-underline',
									'type': 'toolitem',
									'text': _UNO('.uno:Underline'),
									'command': '.uno:Underline',
									'accessibility': { focusBack: true,	combination: '3', de: null }
								},
								{
									'id': 'home-strikeout',
									'type': 'toolitem',
									'text': _UNO('.uno:Strikeout'),
									'command': '.uno:Strikeout',
									'accessibility': { focusBack: true,	combination: '4', de: null }
								},
								{
									'id': 'home-subscript',
									'type': 'toolitem',
									'text': _UNO('.uno:SubScript'),
									'command': '.uno:SubScript',
									'accessibility': { focusBack: true,	combination: '5', de: null }
								},
								{
									'id': 'home-superscript',
									'type': 'toolitem',
									'text': _UNO('.uno:SuperScript'),
									'command': '.uno:SuperScript',
									'accessibility': { focusBack: true,	combination: '6', de: null }
								},
								{
									'id': 'home-set-border-style:BorderStyleMenu',
									'type': 'menubutton',
									'noLabel': true,
									'text': _UNO('.uno:SetBorderStyle'),
									'command': '.uno:SetBorderStyle',
									'applyCallback': 'defaultborderstyle',
									'accessibility': { focusBack: true,	combination: 'B', de: null }
								},
								{
									'id': 'home-background-color:ColorPickerMenu',
									'noLabel': true,
									'class': 'unospan-BackgroundColor',
									'type': 'toolitem',
									'text': _UNO('.uno:BackgroundColor'),
									'command': '.uno:BackgroundColor',
									'accessibility': { focusBack: true,	combination: 'H', de: null }
								},
								{
									'id': 'home-color:ColorPickerMenu',
									'noLabel': true,
									'class': 'unospan-FontColor',
									'type': 'toolitem',
									'text': _UNO('.uno:Color'),
									'command': '.uno:Color',
									'accessibility': { focusBack: true,	combination: 'FC', de: null }
								}
							],
							'vertical': 'false'
						}
					],
					'vertical': 'true'
				}
			]
			},
			{ type: 'separator', id: 'home-fontcombobox-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-alignment',
				'name': _('Alignment'),
				'accessibility': { focusBack: true,	combination: 'AT', de: null },
				'more': {
					'command':'.uno:Hyphenate',
					'accessibility': { focusBack: true,	combination: 'MA', de: null },
				},
				'children' : [
				{
					'id': 'Home-Section-Align',
					'type': 'container',
					'children': [
						{
							'id': 'GroupB15',
							'type': 'container',
							'children': [
								{
									'id': 'first6',
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-align-top',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignTop', 'spreadsheet'),
											'command': '.uno:AlignTop',
											'accessibility': { focusBack: true,	combination: 'AT', de: null }
										},
										{
											'id': 'home-align-vertical-center',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignVCenter', 'spreadsheet'),
											'command': '.uno:AlignVCenter',
											'accessibility': { focusBack: true,	combination: 'AM', de: null }
										},
										{
											'id': 'home-align-bottom',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignBottom', 'spreadsheet'),
											'command': '.uno:AlignBottom',
											'accessibility': { focusBack: true,	combination: 'AB', de: null }
										},
										{
											'id': 'home-increment-indent',
											'type': 'toolitem',
											'text': _UNO('.uno:IncrementIndent'),
											'command': '.uno:IncrementIndent',
											'accessibility': { focusBack: true,	combination: '7', de: null }
										},
										{
											'id': 'home-decrement-indent',
											'type': 'toolitem',
											'text': _UNO('.uno:DecrementIndent'),
											'command': '.uno:DecrementIndent',
											'accessibility': { focusBack: true,	combination: '8', de: null }
										},
										{
											'id': 'home-para-left-to-right',
											'type': 'toolitem',
											'text': _UNO('.uno:ParaLeftToRight'),
											'command': '.uno:ParaLeftToRight',
											'accessibility': { focusBack: true,	combination: 'RL', de: null }
										}
									]
								}
							],
							'vertical': 'false'
						},
						{
							'id': 'GroupB16',
							'type': 'container',
							'children': [
								{
									'id': 'second6',
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-align-left',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignLeft', 'spreadsheet'),
											'command': '.uno:AlignLeft',
											'accessibility': { focusBack: true,	combination: 'AL', de: null }
										},
										{
											'id': 'home-align-horizontal-center',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignHorizontalCenter', 'spreadsheet'),
											'command': '.uno:AlignHorizontalCenter',
											'accessibility': { focusBack: true,	combination: 'AC', de: null }
										},
										{
											'id': 'home-align-right',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignRight', 'spreadsheet'),
											'command': '.uno:AlignRight',
											'accessibility': { focusBack: true,	combination: 'AR', de: null }
										},
										{
											'id': 'home-align-block',
											'type': 'toolitem',
											'text': _UNO('.uno:AlignBlock', 'spreadsheet'),
											'command': '.uno:AlignBlock',
											'accessibility': { focusBack: true,	combination: 'AO', de: null }
										},
										{
											'id': 'home-wrap-text',
											'type': 'toolitem',
											'text': _UNO('.uno:WrapText', 'spreadsheet'),
											'command': '.uno:WrapText',
											'accessibility': { focusBack: true,	combination: 'W', de: null }
										},
										{
											'id': 'home-para-right-to-left',
											'type': 'toolitem',
											'text': _UNO('.uno:ParaRightToLeft'),
											'command': '.uno:ParaRightToLeft',
											'accessibility': { focusBack: true,	combination: 'RR', de: null }
										}
									]
								}
							],
							'vertical': 'false'
						}
					],
					'vertical': 'true'
				},
				{
					'id': 'home-merge-cells',
					'type': 'bigtoolitem',
					'text': _('Merge & Center'),
					'command': '.uno:ToggleMergeCells',
					'accessibility': { focusBack: true, combination: 'MC', de: null }
				}
			]
		},
			{ type: 'separator', id: 'home-pararighttoleft-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-number-format',
				'name': _('Number'),
				'accessibility': { focusBack: true,	combination: 'N', de: null },
				'more': {
					'command':'.uno:FormatCellDialog',
					'accessibility': { focusBack: true, combination: 'MN', de: null }
				},
				'children' : [
				{
					'id': 'Home-Section-Number',
					'type': 'container',
					'children': [
						{
							'id': 'numbertype',
							'type': 'listbox',
							'entries': [
								_('General'),
								_('Number'),
								_('Percent'),
								_('Currency'),
								_('Date'),
								_('Time'),
								_('Scientific'),
								_('Fraction'),
								_('Boolean Value'),
								_('Text')
							],
							'selectedCount': '1',
							'selectedEntries': [
								'0'
							],
							'aria': { 'label': _('Number Format') },
							'accessibility': { focusBack: true,	combination: 'N', de: null }
						},
						{
							'id': 'GroupB22',
							'type': 'container',
							'children': [
								{
									'id': 'WeldedToolbar', // has to match core .ui file toolbox id!
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-number-format-currency',
											'type': 'toolitem',
											'dropdown': true,
											'text': _UNO('.uno:NumberFormatCurrency', 'spreadsheet'),
											'command': '.uno:NumberFormatCurrency',
											'accessibility': { focusBack: true,	combination: 'P', de: null }
										}
									]
								},
								{
									'id': 'home-number-format-percent-decimal',
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-number-format-percent',
											'type': 'toolitem',
											'text': _UNO('.uno:NumberFormatPercent', 'spreadsheet'),
											'command': '.uno:NumberFormatPercent',
											'accessibility': { focusBack: true,	combination: 'AN', de: null }
										},
										{
											'id': 'home-number-format-decimal',
											'type': 'toolitem',
											'text': _UNO('.uno:NumberFormatDecimal', 'spreadsheet'),
											'command': '.uno:NumberFormatDecimal',
											'accessibility': { focusBack: true,	combination: 'K', de: null }
										}
									]
								},
								{
									'id': 'second2',
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-number-format-increment-decimals',
											'type': 'toolitem',
											'text': _UNO('.uno:NumberFormatIncDecimals', 'spreadsheet'),
											'command': '.uno:NumberFormatIncDecimals',
											'accessibility': { focusBack: true,	combination: '0', de: null }
										},
										{
											'id': 'home-number-format-decrement-decimals',
											'type': 'toolitem',
											'text': _UNO('.uno:NumberFormatDecDecimals', 'spreadsheet'),
											'command': '.uno:NumberFormatDecDecimals',
											'accessibility': { focusBack: true,	combination: '9', de: null }
										}
									]
								}
							],
							'vertical': 'false'
						}
					],
					'vertical': 'true'
				}
			]
		},
			{ type: 'separator', id: 'home-numberformatdecrementdecimals-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-insert-table',
				'name': _('Cells'),
				'accessibility': { focusBack: true,	combination: 'RB', de: null },
				'children' : [
				{
					'id': 'Home-Section-Cell1',
					'type': 'container',
					'children': [
						{
							'id': 'LineA10',
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-insert-rows-before',
									'type': 'toolitem',
									'text': _UNO('.uno:InsertRowsBefore', 'spreadsheet', true),
									'command': '.uno:InsertRowsBefore',
									'accessibility': { focusBack: true,	combination: 'RB', de: null }
								},
								{
									'id': 'home-insert-rows-after',
									'type': 'toolitem',
									'text': _UNO('.uno:InsertRowsAfter', 'spreadsheet', true),
									'command': '.uno:InsertRowsAfter',
									'accessibility': { focusBack: true,	combination: 'RA', de: null }
								},
								{
									'id': 'home-delete-rows',
									'type': 'toolitem',
									'text': _UNO('.uno:DeleteRows', 'spreadsheet'),
									'command': '.uno:DeleteRows',
									'accessibility': { focusBack: true,	combination: 'RD', de: null }
								},
								{
									'id': 'home-row-operations',
									'type': 'toolitem',
									'text': _UNO('.uno:RowOperations', 'spreadsheet'),
									'command': '.uno:RowOperations'
								}
							]
					},
					{
						'id': 'LineB11',
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-insert-columns-before',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertColumnsBefore', 'spreadsheet', true),
								'command': '.uno:InsertColumnsBefore',
								'accessibility': { focusBack: true,	combination: 'UB', de: null }
							},
							{
								'id': 'home-insert-columns-after',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertColumnsAfter', 'spreadsheet', true),
								'command': '.uno:InsertColumnsAfter',
								'accessibility': { focusBack: true,	combination: 'UA', de: null }
							},
							{
								'id': 'home-delete-columns',
								'type': 'toolitem',
								'text': _UNO('.uno:DeleteColumns', 'spreadsheet'),
								'command': '.uno:DeleteColumns',
								'accessibility': { focusBack: true,	combination: 'UD', de: null }
							},
							{
								'id': 'home-column-operations',
								'type': 'toolitem',
								'text': _UNO('.uno:ColumnOperations', 'spreadsheet'),
								'command': '.uno:ColumnOperations'
							}
						]
					}
				],
				'vertical': 'true'
			},]
		},
			{ type: 'separator', id: 'home-columnoperations-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-style',
				'name': _('Style'),
				'accessibility': { focusBack: true,	combination: 'L', de: null },
				'children' : [
				{
					'id': 'home-conditional-format-menu:ConditionalFormatMenu',
					'type': 'menubutton',
					'text': _UNO('.uno:ConditionalFormatMenu', 'spreadsheet'),
					'command': '.uno:ConditionalFormatMenu',
					'accessibility': { focusBack: true, combination: 'L', de: null }
				},
				{
					'id': 'Home-Section-Style2',
					'type': 'container',
					'children': [
						{
							'id': 'SectionBottom102',
							'type': 'toolbox',
							'children': [
								{
									'id': 'StyleApplyDefault',
									'type': 'toolitem',
									'text': _('Default'),
									'command': '.uno:StyleApply?Style:string=Default&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'AD', de: null }
								},
								{
									'id': 'StyleApplyHeading1',
									'type': 'toolitem',
									'text': _('Heading 1'),
									'command': '.uno:StyleApply?Style:string=Heading 1&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'D1', de: null }
								},
								{
									'id': 'StyleApplyHeading2',
									'type': 'toolitem',
									'text': _('Heading 2'),
									'command': '.uno:StyleApply?Style:string=Heading 2&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'D2', de: null }
								}
							]
						},
						{
							'id': 'SectionBottom7',
							'type': 'toolbox',
							'children': [
								{
									'id': 'StyleApplyGood',
									'type': 'toolitem',
									'text': _('Good'),
									'command': '.uno:StyleApply?Style:string=Good&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'TG', de: null }
								},
								{
									'id': 'StyleApplyNeutral',
									'type': 'toolitem',
									'text': _('Neutral'),
									'command': '.uno:StyleApply?Style:string=Neutral&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'TN', de: null }
								},
								{
									'id': 'StyleApplyBad',
									'type': 'toolitem',
									'text': _('Bad'),
									'command': '.uno:StyleApply?Style:string=Bad&FamilyName:string=CellStyles',
									'accessibility': { focusBack: true,	combination: 'TB', de: null }
								}
							]
						}
					],
					'vertical': 'true'
				},
			]
		},
			{ type: 'separator', id: 'home-styleapplybad-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-find-n-filter',
				'name': _('Editing'),
				'accessibility': { focusBack: true,	combination: 'SS', de: null },
				'children' : [
				{
					'type': 'container',
					'children': [
						{
							'type': 'toolbox',
							'children': [
									{
										'id': 'home-search',
										'class': 'unoSearch',
										'type': 'customtoolitem',
										'text': _('Search'),
										'accessibility': { focusBack: false,	combination: 'SS',	de: 'SS' }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-search-dialog',
										'type': 'toolitem',
										'text': _('Replace'),
										'command': '.uno:SearchDialog?InitialFocusReplace:bool=true',
										'accessibility': { focusBack: false, 	combination: 'FD',	de: null }
									}
								]
							}
						],
					'vertical': 'true'
				},
				{
					'id': 'Home-Section-Find',
					'type': 'container',
					'children': [
						{
							'id': 'LineA17',
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-data-sort',
									'type': 'toolitem',
									'text': _UNO('.uno:DataSort', 'spreadsheet'),
									'command': '.uno:DataSort',
									'accessibility': { focusBack: true,	combination: 'SO', de: null }
								}
							]
						},
						{
							'id': 'LineB19',
							'type': 'toolbox',
							'children': [
								{
									'id': 'home-data-filter-auto-filter',
									'type': 'toolitem',
									'text': _UNO('.uno:DataFilterAutoFilter', 'spreadsheet'),
									'command': '.uno:DataFilterAutoFilter',
									'accessibility': { focusBack: true,	combination: 'FI', de: null }
								}
							]
						}
					],
					'vertical': 'true'
				}
			]
		}
		];

		return this.getTabPage('Home', content);
	},

	getLayoutTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'layout-page',
				'name':_('Page Setup'),
				'accessibility': { focusBack: true,	combination: 'PS', de: 'K' },
				'more': {
					'command':'.uno:PageFormatDialog',
					'accessibility': { focusBack: true,	combination: 'PS', de: 'K' },
				},
				'children' : [
					{
						'id': 'Layout-MarginMenu:MenuMargins',
						'type': 'menubutton',
						'text': _('Margin'),
						'enabled': 'true',
						'accessibility': { focusBack: true,	combination: 'M', de: '8' }
					},
					{
						'id': 'Layout-SizeMenu:MenuPageSizesCalc',
						'type': 'menubutton',
						'text': _('Size'),
						'enabled': 'true',
						'accessibility': { focusBack: true,	combination: 'SZ', de: 'R' }
					},
					{
						'id': 'Layout-OrientationMenu:MenuOrientation',
						'type': 'menubutton',
						'text': _UNO('.uno:Orientation', 'text'),
						'enabled': 'true',
						'accessibility': { focusBack: true,	combination: 'O', de: '4' }
					},
					{
						'id': 'Layout-PrintRangesMenu:MenuPrintRanges',
						'type': 'menubutton',
						'text': _UNO('.uno:PrintRangesMenu', 'spreadsheet'),
						'enabled': 'true',
						'accessibility': { focusBack: true,	combination: 'PR', de: 'H' }
					},
				]
			},
			{ type: 'separator', id: 'layout-pageformatdialog-break', orientation: 'vertical' },
			{
				'id': 'layout-sheet-right-to-left',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SheetRightToLeft', 'spreadsheet'),
				'command': '.uno:SheetRightToLeft',
				'accessibility': { focusBack: true,	combination: 'RL', de: null }
			},
			{ type: 'separator', id: 'layout-sheetrighttoleft-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-grid',
				'name':_('Grid'),
				'accessibility': { focusBack: true,	combination: 'SG', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'id': 'view-togglegrid',
								'type': 'bigtoolitem',
								'text': _UNO('.uno:ToggleSheetGrid','spreadsheet'),
								'command': '.uno:ToggleSheetGrid',
								'accessibility': { focusBack: true,	combination: 'SG', de: null }
							},
							{
								'id': 'view-toggleprintgrid',
								'type': 'bigtoolitem',
								'text': _UNO('.uno:TogglePrintGrid', 'spreadsheet'),
								'command': '.uno:TogglePrintGrid',
								'accessibility': { focusBack: true,	combination: 'PG', de: null }
							}
						]
					},
				]
			},
			{ type: 'separator', id: 'layout-printrangesmenu-break', orientation: 'vertical' },
			{
				'id': 'Data-RowMenuHeight:MenuRowHeight',
				'class': 'unoData-RowMenuHeight',
				'type': 'menubutton',
				'text': _('Row Height'),
				'enabled': 'true',
				'accessibility': { focusBack: true,	combination: 'RH', de: null }
			},
			{
				'id': 'Data-ColumnMenuWidth:MenuColumnWidth',
				'class': 'unoData-ColumnMenuWidth',
				'type': 'menubutton',
				'text': _('Column Width'),
				'enabled': 'true',
				'accessibility': { focusBack: true,	combination: 'CW', de: null }
			},
			{ type: 'separator', id: 'layout-columnmenuwidth-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-insert-table',
				'name':_('Insert'),
				'accessibility': { focusBack: true,	combination: 'RB', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-insert-rows-before',
										'type': 'toolitem',
										'text': _('Insert Rows Above'),
										'command': '.uno:InsertRowsBefore',
										'accessibility': { focusBack: true,	combination: 'RB', de: null }
									},
									{
										'id': 'layout-insert-columns-before',
										'type': 'toolitem',
										'text': _('Insert Columns Before'),
										'command': '.uno:InsertColumnsBefore',
										'accessibility': { focusBack: true,	combination: 'CB', de: null }
									},
									{
										'id': 'layout-delete-rows',
										'type': 'toolitem',
										'text': _('Delete Rows'),
										'command': '.uno:DeleteRows',
										'accessibility': { focusBack: true,	combination: 'RD', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-insert-rows-after',
										'type': 'toolitem',
										'text': _('Insert Rows Below'),
										'command': '.uno:InsertRowsAfter',
										'accessibility': { focusBack: true,	combination: 'RA', de: null }
									},
									{
										'id': 'layout-insert-columns-after',
										'type': 'toolitem',
										'text': _('Insert Columns After'),
										'command': '.uno:InsertColumnsAfter',
										'accessibility': { focusBack: true,	combination: 'CA', de: null }
									},
									{
										'id': 'layout-delete-columns',
										'type': 'toolitem',
										'text': _('Delete Columns'),
										'command': '.uno:DeleteColumns',
										'accessibility': { focusBack: true,	combination: 'CD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'layout-deletecolumns-break', orientation: 'vertical' },
			{
				'id': 'layout-select-all',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SelectAll'),
				'command': '.uno:SelectAll',
				'accessibility': { focusBack: true,	combination: 'SA', de: null }
			},
			{ type: 'separator', id: 'layout-selectall-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-align',
				'name':_('Align'),
				'accessibility': { focusBack: true,	combination: 'AL', de: null },
				'children' : [
					{
						'id': 'Layout-Section-Align',
						'type': 'container',
						'children': [
							{
								'id': 'Layout-ObjectAlignLeft-ObjectAlignRight',
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft', 'text'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true,	combination: 'AL', de: null }
									},
									{
										'id': 'layout-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter', 'text'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true,	combination: 'AC', de: null }
									},
									{
										'id': 'layout-align-right',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignRight', 'text'),
										'command': '.uno:ObjectAlignRight',
										'accessibility': { focusBack: true,	combination: 'AR', de: null }
									}
								]
							},
							{
								'id': 'Layout-AlignUp-AlignDown',
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp', 'text'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true,	combination: 'AU', de: null }
									},
									{
										'id': 'layout-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle', 'text'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true,	combination: 'AM', de: null }
									},
									{
										'id': 'layout-align-down',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignDown', 'text'),
										'command': '.uno:AlignDown',
										'accessibility': { focusBack: true,	combination: 'AD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'layout-aligndown-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true,	combination: 'FO', de: null },
				'children' : [
					{
						'id': 'Layout-Section-ForwardBackward',
						'type': 'container',
						'children': [
							{
								'id': 'Layout-ObjectForwardOne-BringToFront',
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-forward-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectForwardOne', 'text'),
										'command': '.uno:ObjectForwardOne',
										'accessibility': { focusBack: true,	combination: 'FO', de: null }
									},
									{
										'id': 'layout-bring-to-front',
										'type': 'toolitem',
										'text': _UNO('.uno:BringToFront', 'text'),
										'command': '.uno:BringToFront',
										'accessibility': { focusBack: true,	combination: 'AF', de: null }
									}
								]
							},
							{
								'id': 'Layout-ObjectBackOne-SendToBack',
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne', 'text'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true,	combination: 'BO', de: null }
									},
									{
										'id': 'layout-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack', 'text'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true,	combination: 'AE', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},
		];

		return this.getTabPage('Layout', content);
	},

	getViewTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'view-sheetview',
				'name':_('Sheet View'),
				'accessibility': { focusBack: true,	combination: 'SV', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'container',
								'children': [
									{
										'id': 'sheetview_combobox',
										'type': 'combobox',
										'text': 'Default',
										'entries': [
											_('Default View')
										],
										'selectedCount': '1',
										'selectedEntries': [
											'1'
										],
										'command': '.uno:SelectSheetView',
										'accessibility': { focusBack: true,	combination: 'VS', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'selectsheetview',
										'type': 'toolitem',
										'text': _('Open Views List'),
										'command': '.uno:SelectSheetView',
										'accessibility': { focusBack: true,	combination: 'SS', de: null }
									},
									{
										'id': 'newsheetview',
										'type': 'toolitem',
										'text': _('New Sheet View'),
										'command': '.uno:NewSheetView',
										'accessibility': { focusBack: true,	combination: 'NS', de: null }
									},
									{
										'id': 'removesheetview',
										'type': 'toolitem',
										'text': _('Remove Sheet View'),
										'command': '.uno:RemoveSheetView',
										'accessibility': { focusBack: true,	combination: 'RS', de: null }
									},
									{
										'id': 'exitsheetview',
										'type': 'toolitem',
										'text': _('Exit Sheet View'),
										'command': '.uno:ExitSheetView',
										'accessibility': { focusBack: true,	combination: 'ES', de: null }
									},
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'layout-sheetview-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'view-freeze',
				'name':_('Freeze'),
				'accessibility': { focusBack: true,	combination: 'FP', de: null },
				'children' : [
					{
						'id': 'layout-freeze-panes',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FreezePanes', 'spreadsheet', true),
						'command': '.uno:FreezePanes',
						'accessibility': { focusBack: true,	combination: 'FP', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-freeze-panes-column',
										'type': 'toolitem',
										'text':_UNO('.uno:FreezePanesColumn', 'spreadsheet', true),
										'command': '.uno:FreezePanesColumn',
										'accessibility': { focusBack: true,	combination: 'FC', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-freeze-panes-row',
										'type': 'toolitem',
										'text': _UNO('.uno:FreezePanesRow', 'spreadsheet', true),
										'command': '.uno:FreezePanesRow',
										'accessibility': { focusBack: true,	combination: 'FR', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'layout-freezepanesrow-break', orientation: 'vertical' },
			{
				'id': 'colrowhighlight',
				'type': 'bigcustomtoolitem',
				'text': _('Focus Cell'),
				'command': 'columnrowhighlight',
				'accessibility': { focusBack: true,	combination: 'HL', de: null }
			},
			{ type: 'separator', id: 'layout-colrowhighlight-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'view-zoom',
				'name':_('Zoom'),
				'accessibility': { focusBack: true,	combination: 'FS', de: null },
				'children' : [
					(window.mode.isTablet()) ?
						{
							'id': 'closemobile',
							'type': 'bigcustomtoolitem',
							'text': _('Read mode'),
							'command': 'closetablet',
						} : {},
					{
						'id': 'fullscreen',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FullScreen'),
						'command': '.uno:FullScreen',
						'accessibility': { focusBack: true,	combination: 'FS', de: null }
					},
					{
						'id': 'zoomreset',
						'class': 'unozoomreset',
						'type': 'bigcustomtoolitem',
						'text': _('Reset zoom'),
						'accessibility': { focusBack: true,	combination: 'J', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'zoomout',
										'class': 'unozoomout',
										'type': 'customtoolitem',
										'text': _UNO('.uno:ZoomMinus'),
										'accessibility': { focusBack: true,	combination: 'ZO', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'zoomin',
										'class': 'unozoomin',
										'type': 'customtoolitem',
										'text': _UNO('.uno:ZoomPlus'),
										'accessibility': { focusBack: true,	combination: 'ZI', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'layout-zoomin-break', orientation: 'vertical' },
			{
				'id': 'toggleuimode',
				'class': 'unotoggleuimode',
				'type': 'bigcustomtoolitem',
				'text': _('Compact view'),
				'accessibility': { focusBack: true,	combination: 'UI', de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'collapsenotebookbar',
								'class': 'unocollapsenotebookbar',
								'type': 'customtoolitem',
								'text': _('Collapse Tabs'),
								'accessibility': { focusBack: true,	combination: 'CT', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'showstatusbar',
								'class': 'unoshowstatusbar',
								'type': 'checkbox',
								'command': 'showstatusbar',
								'text': _('Status Bar'),
								'accessibility': { focusBack: true,	combination: 'SB', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'layout-showstatusbar-break', orientation: 'vertical' },
			{
				'id':'toggledarktheme',
				'class': 'unotoggledarktheme',
				'type': 'bigcustomtoolitem',
				'text': _('Dark Mode'),
				'accessibility': { focusBack: true,	combination: 'DT', de: null }
			},
			{
				'id':'invertbackground',
				'class': 'unoinvertbackground',
				'type': 'bigcustomtoolitem',
				'text': _('Invert Background'),
				'accessibility': { focusBack: true, combination: 'BG', de: null }
			},
			{ type: 'separator', id: 'view-invertbackground-break', orientation: 'vertical' },
			{
				'id': 'view-sidebardeck',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Sidebar'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true,	combination: 'SD', de: null }
			},
			{
				'id': 'view-navigator',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Navigator'),
				'command': '.uno:Navigator',
				'accessibility': { focusBack: true,	combination: 'NV', de: null }
			},
			!this.map['wopi'].DisableAISettings ? {
				'id': 'view-ai-sidebar',
				'type': 'bigcustomtoolitem',
				'text': _('AI Assistant'),
				'tooltip': _('AI Assistant'),
				'icon': 'lc_ai_sidebar.svg',
				'command': 'aichat',
				'accessibility': { focusBack: true, combination: 'AI', de: null }
			} : {},

		];

		return this.getTabPage('View', content);
	},

	getCalcTableTab: function() {
		const tab = JSDialog.CalcTableTab;
		return this.getTabPage(tab.getName(), tab.getContent());
	},

	getSparklineTab: function() {
		var content = [
			{
				'id': 'Sparkline-EditSparkline:EditSparklineMenu',
				'class': 'unoData-EditSparkline',
				'type': 'menubutton',
				'text': _UNO('.uno:EditSparkline', 'spreadsheet'),
				'accessibility': { focusBack: true,	combination: 'ES',	de: null }
			},
			{ type: 'separator', id: 'sparkline-edit-break', orientation: 'vertical' },
			{
				'id': 'groupsparklines',
				'type': 'bigtoolitem',
				'text': _('Group'),
				'command': '.uno:GroupSparklines',
				'accessibility': { focusBack: true, combination: 'GS', de: null }
			},
			{
				'id': 'ungroupsparklines',
				'type': 'bigtoolitem',
				'text': _('Ungroup'),
				'command': '.uno:UngroupSparklines',
				'accessibility': { focusBack: true, combination: 'US', de: null },
			},
			{ type: 'separator', id: 'sparkline-group-break', orientation: 'vertical' },
			{
				'id': 'Sparkline-DeleteSparkline:DeleteSparklineMenu',
				'class': 'unoData-DeleteSparkline',
				'type': 'menubutton',
				'text': _UNO('.uno:DeleteSparkline', 'spreadsheet'),
				'accessibility': { focusBack: true,	combination: 'DS',	de: null }
			},
		];

		return this.getTabPage('Sparkline', content);
	},

	getInsertTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'insert-pivot-table',
				'name':_('Pivot Table'),
				'accessibility': { focusBack: true,	combination: 'V', de: null },
				'children' : [
					{
						'id': 'insert-data-pilot-run',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DataDataPilotRun', 'spreadsheet'),
						'command': '.uno:DataDataPilotRun',
						'accessibility': { focusBack: true,	combination: 'V', de: null }
					},
					{
						'id': 'insert-calculated-field-run',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:CalculatedFieldRun', 'spreadsheet'),
						'command': '.uno:CalculatedFieldRun',
						'accessibility': { focusBack: true,	combination: 'CF', de: null }
					},
					{
						'id': 'Insert-Section-PivotTable-Ext',
						'type': 'container',
						'children': [
							{
								'id': 'LineA152',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-recalc-pivot-table',
										'type': 'toolitem',
										'text': _UNO('.uno:RecalcPivotTable', 'spreadsheet'),
										'command': '.uno:RecalcPivotTable',
										'accessibility': { focusBack: true,	combination: 'R', de: null }
									}
								]
							},
							{
								'id': 'LineB162',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-delete-pivot-table',
										'type': 'toolitem',
										'text': _UNO('.uno:DeletePivotTable', 'spreadsheet'),
										'command': '.uno:DeletePivotTable',
										'accessibility': { focusBack: true,	combination: 'DV', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-deletepivottable-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-calc-table',
				'name':_('Table'),
				'accessibility': { focusBack: true,	combination: 'IT', de: null },
				'children' : [
					{
						'id': 'insert-insert-calc-table',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:InsertCalcTable', 'spreadsheet'),
						'command': '.uno:InsertCalcTable',
						'tooltip': app.LOUtil.isFileODF(this.map)
							? _('Table styles are only available in .xlsx files')
							: _('Insert a styled table'),
						'disabledTooltip': app.LOUtil.isFileODF(this.map)
							? _('Table styles are only available in .xlsx files')
							: _('Select a cell range first to insert a styled table'),
						'accessibility': { focusBack: true,	combination: 'IT', de: null }
					}
				]
			},
			{ type: 'separator', id: 'insert-deletecalctable-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-illustrations',
				'name':_('Illustrations'),
				'accessibility': { focusBack: true,	combination: 'IC', de: null },
				'children' : [
					{
						'id': 'insert-insert-object-chart',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:InsertObjectChart'),
						'command': '.uno:InsertObjectChart',
						'accessibility': { focusBack: true,	combination: 'IC', de: null }
					},
					{
						'id': 'insert-insert-sparkline',
						'type': 'bigtoolitem',
						'text': _('Sparkline'),
						'command': '.uno:InsertSparkline',
						'accessibility': { focusBack: true,	combination: 'IS', de: null }
					},
					{
						'id': 'Insert-Section-PivotTable-Ext',
						'type': 'container',
						'children': [
							{
								'id': 'LineA152',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-insert-graphic:InsertImageMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:InsertGraphic'),
										'command': '.uno:InsertGraphic',
										'accessibility': { focusBack: true,	combination: 'IG', de: null }
									}
								]
							},
							{
								'id': 'LineB162',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-function-dialog',
										'type': 'toolitem',
										'text': _UNO('.uno:FunctionDialog', 'spreadsheet'),
										'command': '.uno:FunctionDialog',
										'accessibility': { focusBack: true,	combination: 'FD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
					{
						'id': 'Insert-BasicShapes-Shapes',
						'type': 'container',
						'children': [
							{
								'id': 'LineA153',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-basic-shapes:InsertShapesMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _('Shapes'),
										'command': '.uno:BasicShapes',
										'accessibility': { focusBack: true,	combination: 'IP', de: null }
									}
								]
							},
							{
								'id': 'LineB163',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-line',
										'type': 'toolitem',
										'text': _UNO('.uno:Line', 'spreadsheet'),
										'command': '.uno:Line',
										'accessibility': { focusBack: true,	combination: 'IL', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-functiondialog-break', orientation: 'vertical' },
			{
				'id': 'HyperlinkDialog',
				'class': 'unoHyperlinkDialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:HyperlinkDialog'),
				'command': '.uno:HyperlinkDialog',
				'accessibility': { focusBack: true,	combination: 'I2', de: null }
			},
			(this.map['wopi'].EnableRemoteLinkPicker) ? {
				'id': 'insert-smart-picker',
				'class': 'unoremotelink',
				'type': 'bigcustomtoolitem',
				'text': _('Smart Picker'),
				'command': 'remotelink',
				'accessibility': { focusBack: true,	combination: 'LR', de: null }
			} : {},
			(this.map['wopi'].EnableRemoteAIContent) ? {
				'id': 'insert-insert-remote-ai-content',
				'class': 'unoremoteaicontent',
				'type': 'bigcustomtoolitem',
				'text': _('Assistant'),
				'command': 'remoteaicontent',
				'accessibility': { focusBack: true, combination: 'RL', de: null }
			} : {},
			{ type: 'separator', id: 'insert-remoteaicontent-break', orientation: 'vertical' },
            {
                'type': 'container',
                'children': [
                    {
                        'type': 'toolbox',
                        'children': [
                            {
								'id': 'insert-insert-current-date',
                                'type': 'toolitem',
                                'text': _UNO('.uno:InsertCurrentDate', 'spreadsheet'),
                                'command': '.uno:InsertCurrentDate',
								'accessibility': { focusBack: true,	combination: 'ID', de: null }
                            }
                        ]
                    },
                    {
                        'type': 'toolbox',
                        'children': [
                            {
								'id': 'insert-insert-current-time',
                                'type': 'toolitem',
                                'text': _UNO('.uno:InsertCurrentTime', 'spreadsheet'),
                                'command': '.uno:InsertCurrentTime',
								'accessibility': { focusBack: true,	combination: 'CT', de: null }
                            }
                        ]
                    }
                ],
                'vertical': 'true'
            },
			{ type: 'separator', id: 'insert-insertcurrenttime-break', orientation: 'vertical' },
			{
				'id': 'Insert-Section-NameRangesTable-Ext',
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-add-name',
								'type': 'toolitem',
								'text': _UNO('.uno:AddName', 'spreadsheet'),
								'command': '.uno:AddName',
								'accessibility': { focusBack: true,	combination: 'IN', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-define-name',
								'type': 'toolitem',
								'text': _UNO('.uno:DefineName', 'spreadsheet'),
								'command': '.uno:DefineName',
								'accessibility': { focusBack: true,	combination: 'DN', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'insert-definename-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-text',
				'name':_('Text'),
				'accessibility': { focusBack: true,	combination: 'DT', de: null },
				'children' : [
					{
						'id': 'insert-draw-text',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DrawText'),
						'command': '.uno:DrawText',
						'accessibility': { focusBack: true,	combination: 'DT', de: null }
					},
					{
						'id': 'Insert-Text-Fontwork',
						'type': 'container',
						'children': [
							{
								'id': 'LineA153',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-font-work-gallery-floater',
										'type': 'toolitem',
										'text': _UNO('.uno:FontworkGalleryFloater'),
										'command': '.uno:FontworkGalleryFloater',
										// Fontwork export/import not supported in other formats.
										'visible': (app.LOUtil.isFileODF(this.map)) ? 'true' : 'false',
										'accessibility': { focusBack: true,	combination: 'IF', de: null }
									}
								]
							},
							{
								'id': 'LineB163',
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-vertical-text',
										'type': app.LOUtil.isFileODF(this.map) ? 'toolitem' : 'bigtoolitem',
										'text': _UNO('.uno:VerticalText', 'spreadsheet'),
										'command': '.uno:VerticalText',
										'accessibility': { focusBack: true,	combination: 'IV', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-verticaltext-break', orientation: 'vertical' },
			{
				'id': 'insert-edit-header-and-footer',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:EditHeaderAndFooter', 'spreadsheet'),
				'command': '.uno:EditHeaderAndFooter',
				'accessibility': { focusBack: true,	combination: 'IH', de: null }
			},
			{ type: 'separator', id: 'insert-editheaderandfooter-break', orientation: 'vertical' },
			{
				'id': 'Insert-Charmap-Annotation',
				'type': 'container',
				'children': [
					{
						'id': 'LineA153',
						'type': 'toolbox',
						'children': [
							{
								'id': 'CharmapControl',
								'class': 'unoCharmapControl',
								'type': 'customtoolitem',
								'text': _UNO('.uno:CharmapControl'),
								'command': 'charmapcontrol',
								'accessibility': { focusBack: true,	combination: 'ZS', de: null }
							}
						]
					},
					{
						'id': 'LineB163',
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-threaded-comment',
								'type': 'customtoolitem',
								'text': _UNO('.uno:InsertThreadedComment', 'spreadsheet'),
								'command': 'insertthreadedcomment',
								'icon': 'lc_insertthreadedcomment.svg',
								'lockUno': '.uno:InsertAnnotation',
								'accessibility': { focusBack: true,	combination: 'TC', de: null }
							},
							{
								'id': 'insert-insert-annotation',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertAnnotation', 'spreadsheet'),
								'command': '.uno:InsertAnnotation',
								'accessibility': { focusBack: true,	combination: 'IA', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			}
		];

		return this.getTabPage('Insert', content);
	},

	getFormulaTab: function() {
		var content = [
			{
				'id': 'Formula-InsertFunction',
				'type': 'bigtoolitem',
				'text': _('Insert Function'),
				'command': '.uno:FunctionDialog',
				'accessibility': { focusBack: true,	combination: 'ZF', de: null }
			},
			{ type: 'separator', id: 'formula-insertformula-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'formula-function-library',
				'name':_('Function Library'),
				'accessibility': { focusBack: true,	combination: 'ZA', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-AutoSumMenu:AutoSumMenu',
										type: 'menubutton',
										class: 'AutoSumMenu',
										noLabel: true,
										text: _('AutoSum'),
										command: '.uno:AutoSumMenu',
										'accessibility': { focusBack: true,	combination: 'ZA', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-FinancialFunctions:FinancialFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Financial'),
										'accessibility': { focusBack: true,	combination: 'ZR', de: null }
									},
								]
							},
						],
						'vertical': 'true'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-LogicalFunctions:LogicalFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Logical'),
										'accessibility': { focusBack: true,	combination: 'ZL', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-TextFunctions:TextFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Text'),
										'accessibility': { focusBack: true,	combination: 'ZT', de: null }
									},
								]
							},
						],
						'vertical': 'true'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-DateAndTimeFunctions:DateAndTimeFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Date & Time'),
										'accessibility': { focusBack: true,	combination: 'ZD', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-LookupAndRefFunctions:LookupAndRefFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Lookup & Reference'),
										'accessibility': { focusBack: true,	combination: 'ZK', de: null }
									},
								]
							},
						],
						'vertical': 'true'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-MathAndTrigFunctions:MathAndTrigFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('Math & Trig'),
										'accessibility': { focusBack: true,	combination: 'ZM', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										id: 'Formula-MoreFunctions:MoreFunctionsMenu',
										type: 'menubutton',
										noLabel: true,
										text: _('More Functions'),
										'accessibility': { focusBack: true,	combination: 'ZX', de: null }
									},
								]
							},
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'formula-section-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'formula-add-name',
								'type': 'toolitem',
								'text': _UNO('.uno:AddName', 'spreadsheet'),
								'command': '.uno:AddName',
								'accessibility': { focusBack: true,	combination: 'AN', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'data-define-name',
								'type': 'toolitem',
								'text': _UNO('.uno:DefineName', 'spreadsheet'),
								'command': '.uno:DefineName',
								'accessibility': { focusBack: true,	combination: 'ZN', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'formula-nameoperations-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'formula-range',
				'name':_('Range'),
				'accessibility': { focusBack: true,	combination: 'DR', de: null },
				'children' : [
					{
						'id': 'formula-define-range',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DefineDBName', 'spreadsheet'),
						'command': '.uno:DefineDBName',
						'accessibility': { focusBack: true,	combination: 'DR', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'formula-select-range',
										'type': 'toolitem',
										'text': _UNO('.uno:SelectDB', 'spreadsheet'),
										'command': '.uno:SelectDB',
										'accessibility': { focusBack: true, combination: 'SR', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'formula-refresh-range',
										'type': 'toolitem',
										'text': _UNO('.uno:DataAreaRefresh', 'spreadsheet'),
										'command': '.uno:DataAreaRefresh',
										'accessibility': { focusBack: true, combination: 'DE', de: null }
									},
								]
							},
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'formula-rangeoperations-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'formula-convert-formula-to-value',
								'type': 'toolitem',
								'text': _UNO('.uno:ConvertFormulaToValue', 'spreadsheet'),
								'command': '.uno:ConvertFormulaToValue',
								'accessibility': { focusBack: true,	combination: 'CV', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'formula-calculate',
								'type': 'toolitem',
								'text': _UNO('.uno:Calculate', 'spreadsheet'),
								'command': '.uno:Calculate',
								'accessibility': { focusBack: true,	combination: 'RC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
		];

		return this.getTabPage('Formula', content);
	},

	getDataTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'data-sort',
				'name':_('Sort'),
				'accessibility': { focusBack: true,	combination: 'SS', de: null },
				'children' : [
					{
						'id': 'data-data-sort',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DataSort', 'spreadsheet'),
						'command': '.uno:DataSort',
						'accessibility': { focusBack: true,	combination: 'SS', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-sort-ascending',
										'type': 'toolitem',
										'text': _UNO('.uno:SortAscending', 'spreadsheet'),
										'command': '.uno:SortAscending',
										'accessibility': { focusBack: true,	combination: 'SA', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-sort-descending',
										'type': 'toolitem',
										'text': _UNO('.uno:SortDescending', 'spreadsheet'),
										'command': '.uno:SortDescending',
										'accessibility': { focusBack: true,	combination: 'SD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'data-sortdescending-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'data-filter',
				'name':_('Filter'),
				'accessibility': { focusBack: true,	combination: 'T', de: null },
				'children' : [
					{
						'id': 'data-data-filter-auto-filter',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DataFilterAutoFilter', 'spreadsheet'),
						'command': '.uno:DataFilterAutoFilter',
						'accessibility': { focusBack: true,	combination: 'T', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-data-filter-standart-filter',
										'type': 'toolitem',
										'text': _UNO('.uno:DataFilterStandardFilter', 'spreadsheet'),
										'command': '.uno:DataFilterStandardFilter',
										'accessibility': { focusBack: true,	combination: 'SF', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-data-filter-special-filter',
										'type': 'toolitem',
										'text': _UNO('.uno:DataFilterSpecialFilter', 'spreadsheet'),
										'command': '.uno:DataFilterSpecialFilter',
										'accessibility': { focusBack: true,	combination: 'Q', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-data-filter-hide-auto-filter',
										'type': 'toolitem',
										'text': _UNO('.uno:DataFilterHideAutoFilter', 'spreadsheet'),
										'command': '.uno:DataFilterHideAutoFilter',
										'accessibility': { focusBack: true,	combination: 'HF', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-data-filter-remove-filter',
										'type': 'toolitem',
										'text': _UNO('.uno:DataFilterRemoveFilter', 'spreadsheet'),
										'command': '.uno:DataFilterRemoveFilter',
										'accessibility': { focusBack: true,	combination: 'FR', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'data-datafilterremovefilter-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'data-group',
				'name':_('Group'),
				'accessibility': { focusBack: true,	combination: 'GA', de: null },
				'children' : [
					{
						'id': 'data-data-group',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Group'),
						'command': '.uno:Group',
						'accessibility': { focusBack: true,	combination: 'GA', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-ungroup',
										'type': 'toolitem',
										'text': _UNO('.uno:Ungroup'),
										'command': '.uno:Ungroup',
										'accessibility': { focusBack: true,	combination: 'GR', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'data-clear-outline',
										'type': 'toolitem',
										'text': _UNO('.uno:ClearOutline', 'spreadsheet'),
										'command': '.uno:ClearOutline',
										'accessibility': { focusBack: true,	combination: 'CO', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'data-clearoutline-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'data-show-detail',
								'type': 'toolitem',
								'text': _UNO('.uno:ShowDetail'),
								'command': '.uno:ShowDetail',
								'accessibility': { focusBack: true,	combination: 'DS', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'data-hide-detail',
								'type': 'toolitem',
								'text': _UNO('.uno:HideDetail'),
								'command': '.uno:HideDetail',
								'accessibility': { focusBack: true,	combination: 'DH', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'data-hidedetail-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'data-goal-seek-dialog',
								'type': 'toolitem',
								'text': _UNO('.uno:GoalSeekDialog', 'spreadsheet'),
								'command': '.uno:GoalSeekDialog',
								'accessibility': { focusBack: true,	combination: 'SG', de: null }
							},
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'data-validation',
								'type': 'toolitem',
								'text': _UNO('.uno:Validation', 'spreadsheet'),
								'command': '.uno:Validation',
								'accessibility': { focusBack: true,	combination: 'DV', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'data-validation-break', orientation: 'vertical' },
			{
				'id': 'Data-StatisticsMenu:Menu Statistic',
				'class': 'unoData-StatisticsMenu',
				'type': 'menubutton',
				'text': _UNO('.uno:StatisticsMenu', 'spreadsheet'),
				'enabled': 'true',
				'accessibility': { focusBack: true,	combination: 'ST', de: null }
			},
		];

		return this.getTabPage('Data', content);
	},

	getReviewTab: function() {
		// Note: when adding track changes elements, consider this._map['wopi'].HideChangeTrackingControls
		var content = [
			{
				'id': 'review-spell-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SpellDialog'),
				'command': '.uno:SpellDialog',
				'accessibility': { focusBack: true,	combination: 'S', de: null }
			},
			{
				'id': 'LanguageMenu',
				'type': 'bigcustomtoolitem',
				'text': _UNO('.uno:LanguageMenu'),
				'command': 'languagemenu',
				'accessibility': { focusBack: true,	combination: 'L', de: null }
			},
			{
				'id': 'Review-Section-Language1',
				'type': 'container',
				'children': [
					{
						'id': 'LineA19',
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-spell-online',
								'type': 'toolitem',
								'text': _('Auto Spell Check'),
								'command': '.uno:SpellOnline',
								'accessibility': { focusBack: true,	combination: 'O', de: null },
								'stateIcons': {
									on: 'autospellcheck-on',
									off: 'autospellcheck-off',
								}
							}
						]
					},
					{
						'id': 'LineB20',
						'type': 'toolbox',
						'children': [
							{
								'id': 'review-hyphenate',
								'type': 'toolitem',
								'text': _UNO('.uno:Hyphenate', 'spreadsheet'),
								'command': '.uno:Hyphenate',
								'accessibility': { focusBack: true,	combination: 'H', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'review-hyphenate-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'review-comments',
				'name':_('Comments'),
				'accessibility': { focusBack: true,	combination: 'IA', de: null },
				'children' : [
					{
						'id': 'review-insert-threaded-comment:CalcAnnotationMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:InsertThreadedComment', 'spreadsheet'),
						'applyCallback': 'insertthreadedcomment',
						'command': '.uno:InsertThreadedComment',
						'accessibility': { focusBack: true,	combination: 'TC', de: null }
					},
					{
						'id': 'Review-Section-Annotation2',
						'type': 'container',
						'children': [
							{
								'id': 'LeftParaMargin17',
								'type': 'toolbox',
								'children': [
									{
										'id': 'review-delete-all-notes',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteAllNotes'),
										'command': '.uno:DeleteAllNotes',
										'accessibility': { focusBack: true,	combination: 'DA', de: null }
									}
								]
							},
							{
								'id': 'belowspacing15',
								'type': 'toolbox',
								'children': [
									{
										'id': 'review-delete-note',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteNote', 'spreadsheet'),
										'command': '.uno:DeleteNote',
										'accessibility': { focusBack: true,	combination: 'DO', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'review-deletenote-break', orientation: 'vertical' },
			{
				'type': 'toolbox',
				'children': [
					{
						'id': 'review-protect-sheet',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Protect', 'spreadsheet'),
						'command': '.uno:Protect',
						'accessibility': { focusBack: true,	combination: 'DP', de: null }
					}
				]
			},
		];

		return this.getTabPage('Review', content);
	},

	getFormatTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'format-style',
				'name':_('Style'),
				'accessibility': { focusBack: true,	combination: 'FD', de: null },
				'children' : [
					{
						'id': 'format-font-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FontDialog'),
						'command': '.uno:FontDialog',
						'accessibility': { focusBack: true,	combination: 'FD', de: null }
					},
					{
						'id': 'FormatMenu:FormatMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:FormatMenu', 'spreadsheet'),
						'command': '.uno:FormatMenu',
						'accessibility': { focusBack: true,	combination: 'FM', de: null }
					},
					{
						'id': 'format-paragraph-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:ParagraphDialog'),
						'command': '.uno:ParagraphDialog',
						'accessibility': { focusBack: true,	combination: 'PD', de: null }
					},
					{
						'id': 'format-style-dialog',
						'type': 'bigtoolitem',
						'text': _('Style list'),
						'command': '.uno:SidebarDeck.StyleListDeck',
						'accessibility': { focusBack: false, combination: 'SD', de: null }
					},
				]
			},
			{ type: 'separator', id: 'format-styledialog-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'format-cells',
				'name':_('Format Cells'),
				'accessibility': { focusBack: true,	combination: 'PF', de: null },
				'children' : [
					{
						'id': 'format-page-format-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:PageFormatDialog', 'spreadsheet', true),
						'command': '.uno:PageFormatDialog',
						'accessibility': { focusBack: true,	combination: 'PF', de: null }
					},
					{
						'id': 'format-format-cell-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FormatCellDialog', 'spreadsheet', true),
						'command': '.uno:FormatCellDialog',
						'accessibility': { focusBack: true,	combination: 'FC', de: null }
					},
					{
						'id': 'format-conditional-format-menu:ConditionalFormatMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:ConditionalFormatMenu', 'spreadsheet'),
						'command': '.uno:ConditionalFormatMenu',
						'accessibility': { focusBack: true,	combination: 'CF', de: null }
					},
				]
			},
			{ type: 'separator', id: 'format-conditionalformatmenu-break', orientation: 'vertical' },
			{
				'id': 'format-format-line',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatLine'),
				'command': '.uno:FormatLine',
				'accessibility': { focusBack: true,	combination: 'FL', de: null }
			},
			{
				'id': 'format-format-area',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatArea'),
				'command': '.uno:FormatArea',
				'accessibility': { focusBack: true,	combination: 'FA', de: null }
			},
			{
				'id': 'format-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: true,	combination: 'TD', de: null }
			},
			{
				'id': 'Format-Sparkline',
				'type': 'bigtoolitem',
				'text': _('Sparkline'),
				'command': '.uno:InsertSparkline',
				'enabled': 'true',
				'accessibility': { focusBack: true,	combination: 'SM', de: null }
			},
			{ type: 'separator', id: 'format-sparkline-break', orientation: 'vertical' },
			{
				'id': 'themes-group',
				'type': 'overflowgroup',
				'name': _('Themes'),
				'nofold': true,
				'icon': 'lc_themesthames.svg',
				'children': [
					{
						'id': 'iconview_theme_colors-iconview-list',
						'type': 'iconviewlist',
						'accessibility': { focusBack: false, combination: 'TC', de: null },
						'children': [
							{
								'id': 'iconview_theme_colors', // has to match core id
								'type': 'iconview'
							}
						]
					}
				]
			},
			{
				'id': 'add-theme-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:AddTheme'),
				'command': '.uno:AddTheme',
				'accessibility': { focusBack: false, combination: 'AT', de: null }
			},
		];

		return this.getTabPage('Format', content);
	},

	getShapeTab: function() {
		var content = [
			{
				'id': 'shape-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog', 'text'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: true,	combination: 'TD', de: null }
			},
			{ type: 'separator', id: 'shape-transformdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-flip-vertical',
								'type': 'toolitem',
								'text': _UNO('.uno:FlipVertical'),
								'command': '.uno:FlipVertical',
								'accessibility': { focusBack: true,	combination: 'FV', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-flip-horizontal',
								'type': 'toolitem',
								'text': _UNO('.uno:FlipHorizontal'),
								'command': '.uno:FlipHorizontal',
								'accessibility': { focusBack: true,	combination: 'FH', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-fliphorizontal-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-xline-color:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor',
								'accessibility': { focusBack: true,	combination: 'DX', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'shape-fill-color:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor',
								'accessibility': { focusBack: true,	combination: 'FC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-fillcolor-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'shape-wrap',
				'name':_('Wrap'),
				'accessibility': { focusBack: true,	combination: 'WO', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-wrap-off',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapOff', 'text'),
										'command': '.uno:WrapOff',
										'accessibility': { focusBack: true,	combination: 'WO', de: null }
									},
									{
										'id': 'shape-wrap-on',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapOn', 'text'),
										'command': '.uno:WrapOn',
										'accessibility': { focusBack: true,	combination: 'WN', de: null }
									},
									{
										'id': 'shape-wrap-ideal',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapIdeal', 'text'),
										'command': '.uno:WrapIdeal',
										'accessibility': { focusBack: true,	combination: 'WI', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-wrap-left',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapLeft', 'text'),
										'command': '.uno:WrapLeft',
										'accessibility': { focusBack: true,	combination: 'WL', de: null }
									},
									{
										'id': 'shape-wrap-through',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapThrough', 'text'),
										'command': '.uno:WrapThrough',
										'accessibility': { focusBack: true,	combination: 'WT', de: null }
									},
									{
										'id': 'shape-wrap-right',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapRight', 'text'),
										'command': '.uno:WrapRight',
										'accessibility': { focusBack: true,	combination: 'WR', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'shape-wrapright-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'shape-align',
				'name':_('Align'),
				'accessibility': { focusBack: true,	combination: 'AL', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true,	combination: 'AL', de: null }
									},
									{
										'id': 'shape-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true,	combination: 'AC', de: null }
									},
									{
										'id': 'shape-object-align-right',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignRight'),
										'command': '.uno:ObjectAlignRight',
										'accessibility': { focusBack: true,	combination: 'AR', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true,	combination: 'AU', de: null }
									},
									{
										'id': 'shape-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true,	combination: 'AM', de: null }
									},
									{
										'id': 'shape-align-down',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignDown'),
										'command': '.uno:AlignDown',
										'accessibility': { focusBack: true,	combination: 'AD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'shape-aligndown-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'shape-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true,	combination: 'BF', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-bring-to-front',
										'type': 'toolitem',
										'text': _UNO('.uno:BringToFront'),
										'command': '.uno:BringToFront',
										'accessibility': { focusBack: true,	combination: 'BF', de: null }
									},
									{
										'id': 'shape-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true,	combination: 'SB', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-object-forward-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectForwardOne'),
										'command': '.uno:ObjectForwardOne',
										'accessibility': { focusBack: true,	combination: 'FO', de: null }
									},
									{
										'id': 'shape-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true,	combination: 'BO', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'shape-objectbackone-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'shape-group',
				'name':_('Group'),
				'accessibility': { focusBack: true,	combination: 'FG', de: null },
				'children' : [
					{
						'id': 'shape-format-group',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FormatGroup'),
						'command': '.uno:FormatGroup',
						'accessibility': { focusBack: true,	combination: 'FG', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-enter-group',
										'type': 'toolitem',
										'text': _UNO('.uno:EnterGroup'),
										'command': '.uno:EnterGroup',
										'accessibility': { focusBack: true,	combination: 'EG', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'shape-leave-group',
										'type': 'toolitem',
										'text': _UNO('.uno:LeaveGroup'),
										'command': '.uno:LeaveGroup',
										'accessibility': { focusBack: true,	combination: 'LG', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
		];

		return this.getTabPage('Shape', content);
	},

	getPictureTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'picture-adjustments',
				'name':_('Image Controls'),
				'accessibility': { focusBack: true,	combination: 'BN', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'id': 'picture-brightness:PictureBrightness',
								'type': 'menubutton',
								'command': '.uno:GrafLuminance',
								'text': _UNO('.uno:GrafLuminance'),
								'icon': 'lc_setbrightness.svg',
								'accessibility': { focusBack: true, combination: 'BN', de: null }
							},
							{
								'id': 'picture-contrast:PictureContrast',
								'type': 'menubutton',
								'command': '.uno:GrafContrast',
								'text': _UNO('.uno:GrafContrast'),
								'icon': 'lc_setcontrast.svg',
								'accessibility': { focusBack: true, combination: 'CN', de: null }
							},
							{
								'id': 'picture-colormode:PictureColorMode',
								'type': 'menubutton',
								'command': '.uno:GrafMode',
								'text': _UNO('.uno:GrafMode'),
								'icon': 'lc_setgraphtransparency.svg',
								'accessibility': { focusBack: true, combination: 'CO', de: null }
							},
							{
								'id': 'picture-transparency:PictureTransparency',
								'type': 'menubutton',
								'command': '.uno:GrafTransparence',
								'text': _UNO('.uno:GrafTransparence'),
								'icon': 'lc_setgraphtransparency.svg',
								'accessibility': { focusBack: true, combination: 'TP', de: null }
							},
						]
					},
				]
			},
			{ type: 'separator', id: 'picture-transparency-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-xline-color:ColorPickerMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor',
								'accessibility': { focusBack: true,	combination: 'DX', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-fill-color:ColorPickerMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor',
								'accessibility': { focusBack: true,	combination: 'FC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-fillcolor-break', orientation: 'vertical' },
			{
				'id': 'picture-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog', 'text'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: true,	combination: 'TD', de: null }
			},
			{ type: 'separator', id: 'picture-transformdialog-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-flip-vertical',
								'type': 'toolitem',
								'text': _UNO('.uno:FlipVertical'),
								'command': '.uno:FlipVertical',
								'accessibility': { focusBack: true,	combination: 'FV', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'picture-flip-horizontal',
								'type': 'toolitem',
								'text': _UNO('.uno:FlipHorizontal'),
								'command': '.uno:FlipHorizontal',
								'accessibility': { focusBack: true,	combination: 'FH', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-fliphorizontal-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'picture-wrap',
				'name':_('Wrap'),
				'accessibility': { focusBack: true,	combination: 'WO', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-wrap-off',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapOff', 'text'),
										'command': '.uno:WrapOff',
										'accessibility': { focusBack: true,	combination: 'WO', de: null }
									},
									{
										'id': 'picture-wrap-on',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapOn', 'text'),
										'command': '.uno:WrapOn',
										'accessibility': { focusBack: true,	combination: 'WN', de: null }
									},
									{
										'id': 'picture-wrap-ideal',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapIdeal', 'text'),
										'command': '.uno:WrapIdeal',
										'accessibility': { focusBack: true,	combination: 'WI', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-wrap-left',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapLeft', 'text'),
										'command': '.uno:WrapLeft',
										'accessibility': { focusBack: true,	combination: 'WL', de: null }
									},
									{
										'id': 'picture-wrap-through',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapThrough', 'text'),
										'command': '.uno:WrapThrough',
										'accessibility': { focusBack: true,	combination: 'WT', de: null }
									},
									{
										'id': 'picture-wrap-right',
										'type': 'toolitem',
										'text': _UNO('.uno:WrapRight', 'text'),
										'command': '.uno:WrapRight',
										'accessibility': { focusBack: true,	combination: 'WR', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'picture-wrapright-break', orientation: 'vertical' },
				{
				'type': 'overflowgroup',
				'id': 'picture-align',
				'name':_('Align'),
				'accessibility': { focusBack: true,	combination: 'AL', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true,	combination: 'AL', de: null }
									},
									{
										'id': 'picture-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true,	combination: 'AC', de: null }
									},
									{
										'id': 'picture-object-align-right',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignRight'),
										'command': '.uno:ObjectAlignRight',
										'accessibility': { focusBack: true,	combination: 'AR', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true,	combination: 'AU', de: null }
									},
									{
										'id': 'picture-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true,	combination: 'AM', de: null }
									},
									{
										'id': 'picture-align-down',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignDown'),
										'command': '.uno:AlignDown',
										'accessibility': { focusBack: true,	combination: 'AD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'picture-aligndown-break', orientation: 'vertical' },
				{
				'type': 'overflowgroup',
				'id': 'picture-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true,	combination: 'BF', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-bring-to-front',
										'type': 'toolitem',
										'text': _UNO('.uno:BringToFront'),
										'command': '.uno:BringToFront',
										'accessibility': { focusBack: true,	combination: 'BF', de: null }
									},
									{
										'id': 'picture-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true,	combination: 'SB', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-object-forward-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectForwardOne'),
										'command': '.uno:ObjectForwardOne',
										'accessibility': { focusBack: true,	combination: 'FO', de: null }
									},
									{
										'id': 'picture-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true,	combination: 'BO', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'picture-objectbackone-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'picture-group',
				'name':_('Group'),
				'accessibility': { focusBack: true,	combination: 'FG', de: null },
				'children' : [
					{
						'id': 'picture-format-group',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FormatGroup'),
						'command': '.uno:FormatGroup',
						'accessibility': { focusBack: true,	combination: 'FG', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-enter-group',
										'type': 'toolitem',
										'text': _UNO('.uno:EnterGroup'),
										'command': '.uno:EnterGroup',
										'accessibility': { focusBack: true,	combination: 'EG', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'picture-leave-group',
										'type': 'toolitem',
										'text': _UNO('.uno:LeaveGroup'),
										'command': '.uno:LeaveGroup',
										'accessibility': { focusBack: true,	combination: 'LG', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'picture-leavegroup-break', orientation: 'vertical' },
			{
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Crop'),
				'command': '.uno:Crop',
				'context': 'Graphic'
			},
		];

		return this.getTabPage('Picture', content);
	}
});

window.L.control.notebookbarCalc = function (options) {
	return new window.L.Control.NotebookbarCalc(options);
};
