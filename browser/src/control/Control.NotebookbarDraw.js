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
 * window.L.Control.NotebookbarDraw - definition of notebookbar content in Draw
 */

/* global _ _UNO app */
window.L.Control.NotebookbarDraw = window.L.Control.NotebookbarImpress.extend({

	getShortcutsBarData: function() {
		return [
			!this.map['wopi'].HideSaveOption ?
				{
					'id': 'shortcutstoolbox',
					'type': 'toolbox',
					'children': [
						{
							'id': 'save',
							'type': 'toolitem',
							'text': _('Save'),
							'command': '.uno:Save',
							'accessKey': '1',
							'isCustomTooltip': true
						}
					]
				} : {}
		];
	},

	getOptionsSectionData: function() {
		return this.buildOptionsSectionData([
			{
				'type': 'toolitem',
				'text': _UNO('.uno:Sidebar'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true, combination: 'ZB', de: null }
			},
			{
				'type': 'toolitem',
			}
		]);
	},

	getTabs: function() {
		return this._filterExtensionsTab([
			{
				'id': 'File-tab-label',
				'text': _('File'),
				'name': 'File',
				'accessibility': { focusBack: true, combination: 'F', de: null }
			},
			{
				'id': this.HOME_TAB_ID,
				'text': _('Home'),
				'name': 'Home',
				'context': 'default|DrawText',
				'accessibility': { focusBack: true, combination: 'H', de: null }
			},
			{
				'id': 'Insert-tab-label',
				'text': _('Insert'),
				'name': 'Insert',
				'accessibility': { focusBack: true, combination: 'N', de: null }
			},
			{
				'id': 'Layout-tab-label',
				'text': _('Layout'),
				'name': 'Layout',
				'accessibility': { focusBack: true, combination: 'P', de: null }
			},
			{
				'id': 'Review-tab-label',
				'text': _('Review'),
				'name': 'Review',
				'accessibility': { focusBack: true, combination: 'R', de: null }
			},
			{
				'id': 'Format-tab-label',
				'text': _('Format'),
				'name': 'Format',
				'accessibility': { focusBack: true, combination: 'M', de: null }
			},
			{
				'id': 'Table-tab-label',
				'text': _('Table'),
				'name': 'Table',
				'context': 'Table',
				'accessibility': { focusBack: true, combination: 'T', de: null }
			},
			{
				'id': 'Shape-tab-label',
				'text': _('Shape'),
				'name': 'Shape',
				'context': 'Draw|DrawLine|3DObject|MultiObject|DrawFontwork',
				'accessibility': { focusBack: true, combination: 'D', de: null }
			},
			{
				'id': 'Picture-tab-label',
				'text': _('Picture'),
				'name': 'Picture',
				'context': 'Graphic',
				'accessibility': { focusBack: true, combination: 'G', de: null }
			},
			{
				'id': 'Chart-tab-label',
				'text': _('Chart'),
				'name': 'Chart',
				'context': 'Chart|Series|ErrorBar|Axis|Grid|ChartElements|Trendline|ChartTitle|ChartLegend|ChartLabel',
				'accessibility': { focusBack: true, combination: 'CH', de: null }
			},
			{
				'id': 'View-tab-label',
				'text': _('View'),
				'name': 'View',
				'accessibility': { focusBack: true, combination: 'V', de: null }
			},
			{
				'id': 'Extensions-tab-label',
				'text': _('Extensions'),
				'name': 'Extensions',
				'accessibility': { focusBack: true, combination: 'X' }
			},
			{
				'id': 'Help-tab-label',
				'text': _('Help'),
				'name': 'Help',
				'accessibility': { focusBack: true, combination: 'Y', de: null }
			}
		]);
	},

	getTabsJSON: function () {
		return this._filterExtensionsTab([
			this.getFileTab(),
			this.getHomeTab(),
			this.getInsertTab(),
			this.getLayoutTab(),
			this.getReviewTab(),
			this.getFormatTab(),
			this.getTableTab(),
			this.getShapeTab(),
			this.getPictureTab(),
			this.getChartTab(),
			this.getViewTab(),
			this.getExtensionsTab(),
			this.getHelpTab()
		]);
	},

	getFullJSON: function(selectedId) {
		return this.getNotebookbar(this.getTabsJSON(), selectedId);
	},

	getFileTab: function() {
		var content = [];
		var hasSave = !this.map['wopi'].HideSaveOption;
		var hasSaveAs = !this.map['wopi'].UserCanNotWriteRelative;
		var hasShare = this.map['wopi'].EnableShare;
		var hasRevisionHistory = window.L.Params.revHistoryEnabled;

		if (hasSave) {
			content.push(
				{
					'type': 'container',
					'children': [
						{
							'id': 'file-save',
							'type': 'bigtoolitem',
							'text': _('Save'),
							'command': '.uno:Save',
							'accessibility': { focusBack: true, combination: 'SV', de: null }
						}
					]
				});
			}

		if (hasSaveAs) {
			content.push(
				{
					'id': 'file-saveas',
					'type': 'bigtoolitem',
					'text': _UNO('.uno:SaveAs', 'presentation'),
					'command': '.uno:SaveAs',
					'accessibility': { focusBack: true, combination: 'SA', de: null }
				}
			);
		}

		if (hasSaveAs) {
			content.push(
				{
					'id': 'exportas:ExportAsMenu',
					'command': 'exportas',
					'class': 'unoexportas',
					'type': 'exportmenubutton',
					'text': _('Export As'),
					'accessibility': { focusBack: true, combination: 'EA', de: null }
				}
			);
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
			content.push(
				{
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
					}
				]
				}, {
					'id': 'file-revhistory-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		}

		if (!this.map['wopi'].HidePrintOption) {
			content.push(
				{
					'id': 'print',
					'type': 'bigtoolitem',
					'text': _UNO('.uno:Print', 'presentation'),
					'command': '.uno:Print',
					'accessibility': { focusBack: true, combination: 'P', de: null }
				}
			);
		}

		if (!this.map['wopi'].HideExportOption) {
			content.push({
				'id': 'downloadas:DownloadAsMenu',
				'command': 'downloadas',
				'class': 'unodownloadas',
				'type': 'exportmenubutton',
				'text': _('Download'),
				'accessibility': { focusBack: true, combination: 'DA', de: null }
			});
		}

		content.push(
			{
				'id': 'file-exportpdf',
				'type': 'container',
				'children': [
					{
						'id': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
						'class': 'unoexportas',
						'type': 'customtoolitem',
						'text': _('PDF Document (.pdf)'),
						'command': !window.ThisIsAMobileApp ? 'exportdirectpdf' : 'downloadas-pdf',
						'inlineLabel': true,
						'accessibility': { focusBack: true, combination: 'EP', de: null }
					},
					{
						'id': 'exportpdf' ,
						'class': 'unoexportas',
						'type': 'customtoolitem',
						'text': _('PDF Document (.pdf) - Expert'),
						'command': 'exportpdf' ,
						'inlineLabel': true,
						'accessibility': { focusBack: true, combination: 'ES', de: null }
					},
				],
				'vertical': 'true'
			}
		);

		content.push( { type: 'separator', id: 'file-exportpdf-break', orientation: 'vertical' } );

		if (!this.map['wopi'].HideRepairOption) {
			content.push(
				{
				'type': 'container',
				'children': [
					{
						'id': 'repair',
						'class': 'unorepair',
						'type': 'bigcustomtoolitem',
						'text': _('Repair'),
						'accessibility': { focusBack: true, combination: 'RF', de: null }
					}
				]
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
						'accessibility': { focusBack: true, combination: 'FP', de: 'I' }
					}
				]
			});
		if (window.documentSigningEnabled) {
			content.push(
				{
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
				}
			);
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

	getViewTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'view-zoom',
				'name':_('Zoom'),
				'accessibility': { focusBack: true, combination: 'FR', de: null },
				'children' : [
					(window.mode.isTablet()) ?
						{
							'id': 'closemobile',
							'type': 'bigcustomtoolitem',
							'text': _('Read mode'),
							'command': 'closetablet',
						} : {},
					{
						'id': 'view-fullscreen',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FullScreen'),
						'command': '.uno:FullScreen',
						'accessibility': { focusBack: true, combination: 'FR', de: null }
					},
					{
						'id': 'zoomreset',
						'class': 'unozoomreset',
						'type': 'bigcustomtoolitem',
						'text': _('Reset zoom'),
						'accessibility': { focusBack: true, combination: 'RZ', de: null }
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
										'accessibility': { focusBack: true, combination: 'ZO', de: null }
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
										'accessibility': { focusBack: true, combination: 'ZI', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'view-zoomin-break', orientation: 'vertical' },
			{
				'id': 'toggleuimode',
				'class': 'unotoggleuimode',
				'type': 'bigcustomtoolitem',
				'text': _('Compact view'),
				'accessibility': { focusBack: true, combination: 'UI', de: null }
			},
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'showruler',
								'class': 'unoshowruler',
								'type': 'checkbox',
								'command': 'showruler',
								'text': _('Ruler'),
								'accessibility': { focusBack: true, combination: 'R', de: 'L' }
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
								'accessibility': { focusBack: true, combination: 'SB', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{
				'id': 'collapsenotebookbar',
				'class': 'unocollapsenotebookbar',
				'type': 'bigcustomtoolitem',
				'text': _('Collapse Tabs'),
				'accessibility': { focusBack: true, combination: 'CT', de: null }
			},
			{ type: 'separator', id: 'view-collapsenotebookbar-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'view-grid',
				'name':_('Grid'),
				'accessibility': { focusBack: true, combination: 'GV', de: null },
				'children' : [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-grid-visible',
								'type': 'toolitem',
								'text': _UNO('.uno:GridVisible'),
								'command': '.uno:GridVisible',
								'accessibility': { focusBack: true, combination: 'GV', de: null }
							},
							{
								'id': 'home-grid-use',
								'type': 'toolitem',
								'text': _UNO('.uno:GridUse'),
								'command': '.uno:GridUse',
								'accessibility': { focusBack: true, combination: 'GU', de: null }
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'view-griduse-break', orientation: 'vertical' },
			{
				'id':'toggledarktheme',
				'type': 'bigcustomtoolitem',
				'text': _('Dark Mode'),
				'accessibility': { focusBack: true, combination: 'DT', de: null }
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
				'id': 'view-sidebar',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Sidebar'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true, combination: 'SB', de: null }
			},
			{
				'id': 'view-navigator',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Navigator'),
				'command': '.uno:Navigator',
				'accessibility': { focusBack: true, combination: 'N', de: null }
			}
		];

		return this.getTabPage('View', content);
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
						'accessibility': { focusBack: true, combination: 'ZZ', de: null }
					},
					{
						'id': 'home-redo',
						'type': 'toolitem',
						'text': _UNO('.uno:Redo'),
						'command': '.uno:Redo',
						'accessibility': { focusBack: true, combination: 'O', de: null }
					},
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-undoredo-break', orientation: 'vertical' },
						{
				'type': 'overflowgroup',
				'id': 'home-clipboard',
				'name':_('Clipboard'),
				'accessibility': { focusBack: true, combination: 'V', de: null },
				'children' : [
					{
						'id': 'home-paste:PasteMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:Paste'),
						'command': '.uno:Paste',
						'accessibility': { focusBack: true, combination: 'V', de: null }
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
										'text': _UNO('.uno:Cut'),
										'command': '.uno:Cut',
										'accessibility': { focusBack: true, combination: 'X', de: null }
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
										'accessibility': { focusBack: true, combination: 'FP', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-copy',
										'type': 'customtoolitem',
										'text': _UNO('.uno:Copy'),
										'command': '.uno:Copy',
										'accessibility': { focusBack: true, combination: 'C', de: null }
									},
									{
										'id': 'home-set-default',
										'type': 'toolitem',
										'text': _UNO('.uno:SetDefault'),
										'command': '.uno:SetDefault',
										'accessibility': { focusBack: true, combination: 'SF', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-setdefault-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-font',
				'name':_('Font'),
				'accessibility': { focusBack: true, combination: 'FN', de: null },
				'more': {
					'command':'.uno:FontDialog',
					'accessibility': { focusBack: true, combination: 'MF', de: null }
				},
				'children' : [
					{
						'type': 'container',
						'children': [
							{
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
											'0'
										],
										'command': '.uno:CharFontName',
										'accessibility': { focusBack: true, combination: 'FN', de: null }
									},
									{
										'id': 'fontsizecombobox',
										'type': 'combobox',
										'text': '12 pt',
										'entries': [
											'12 pt'
										],
										'selectedCount': '1',
										'selectedEntries': [
											'0'
										],
										'command': '.uno:FontHeight',
										'accessibility': { focusBack: true, combination: 'FS', de: null }
									},
									{
										'id': 'home-grow',
										'type': 'toolitem',
										'text': _UNO('.uno:Grow'),
										'command': '.uno:Grow',
										'accessibility': { focusBack: true, combination: 'FG', de: null }
									},
									{
										'id': 'home-shrink',
										'type': 'toolitem',
										'text': _UNO('.uno:Shrink'),
										'command': '.uno:Shrink',
										'accessibility': { focusBack: true, combination: 'FK', de: null }
									}
								],
								'vertical': 'false'
							},
							{
								'type': 'container',
								'children': [
									{
										'id': 'ExtTop4',
										'type': 'toolbox',
										'children': [
											{
												'id': 'home-bold',
												'type': 'toolitem',
												'text': _UNO('.uno:Bold'),
												'command': '.uno:Bold',
												'accessibility': { focusBack: true, combination: '1', de: null }
											},
											{
												'id': 'home-italic',
												'type': 'toolitem',
												'text': _UNO('.uno:Italic'),
												'command': '.uno:Italic',
												'accessibility': { focusBack: true, combination: '2', de: null }
											},
											{
												'id': 'home-underline',
												'type': 'toolitem',
												'text': _UNO('.uno:Underline'),
												'command': '.uno:Underline',
												'accessibility': { focusBack: true, combination: '3', de: null }
											},
											{
												'id': 'home-strikeout',
												'type': 'toolitem',
												'text': _UNO('.uno:Strikeout'),
												'command': '.uno:Strikeout',
												'accessibility': { focusBack: true, combination: '4', de: null }
											},
											{
												'id': 'home-shadowed',
												'type': 'toolitem',
												'text': _UNO('.uno:Shadowed'),
												'command': '.uno:Shadowed',
												'accessibility': { focusBack: true, combination: 'SH', de: null }
											},
											{
												'id': 'home-fontwork-gallery',
												'type': 'toolitem',
												'text': _UNO('.uno:FontworkGalleryFloater'),
												'command': '.uno:FontworkGalleryFloater',
												// Fontwork export/import not supported in other formats.
												'visible': app.LOUtil.isFileODF(this.map) ? 'true' : 'false',
												'accessibility': { focusBack: true, combination: 'FW', de: null }
											},
											{
												'id': 'home-char-back-color:ColorPickerMenu',
												'noLabel': true,
												'class': 'unospan-CharBackColor',
												'type': 'toolitem',
												'text': _UNO('.uno:CharBackColor'),
												'command': '.uno:CharBackColor',
												'accessibility': { focusBack: true, combination: 'BC', de: null }
											},
											{
												'id': 'home-color:ColorPickerMenu',
												'noLabel': true,
												'class': 'unospan-FontColor',
												'type': 'toolitem',
												'text': _UNO('.uno:Color'),
												'command': '.uno:Color',
												'accessibility': { focusBack: true, combination: 'FC', de: null }
											}
										]
									}
								],
								'vertical': 'false'
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-font-combobox', orientation: 'vertical'},
			{
				'type': 'overflowgroup',
				'id': 'home-alignment',
				'name':_('Alignment'),
				'accessibility': { focusBack: true, combination: 'AT', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'container',
								'children': [
									{
										'type': 'toolbox',
										'children': [
											{
												'id': 'home-cell-vertical-top',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertTop'),
												'command': '.uno:CellVertTop',
												'accessibility': { focusBack: true, combination: 'AT', de: null }
											},
											{
												'id': 'home-cell-vertical-center',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertCenter'),
												'command': '.uno:CellVertCenter',
												'accessibility': { focusBack: true, combination: 'AC', de: null }
											},
											{
												'id': 'home-cell-vertical-bottom',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertBottom'),
												'command': '.uno:CellVertBottom',
												'accessibility': { focusBack: true, combination: 'AB', de: null }
											}
										]
									},
								],
								'vertical': 'false'
							},
							{
								'type': 'container',
								'children': [
									{
										'id': 'SectionBottom13',
										'type': 'toolbox',
										'children': [
											{
												'id': 'home-left-para',
												'type': 'toolitem',
												'text': _UNO('.uno:LeftPara'),
												'command': '.uno:LeftPara',
												'accessibility': { focusBack: true, combination: 'PL', de: null }
											},
											{
												'id': 'home-center-para',
												'type': 'toolitem',
												'text': _UNO('.uno:CenterPara'),
												'command': '.uno:CenterPara',
												'accessibility': { focusBack: true, combination: 'PC', de: null }
											},
											{
												'id': 'home-right-para',
												'type': 'toolitem',
												'text': _UNO('.uno:RightPara'),
												'command': '.uno:RightPara',
												'accessibility': { focusBack: true, combination: 'PR', de: null }
											},
											{
												'id': 'home-justify-para',
												'type': 'toolitem',
												'text': _UNO('.uno:JustifyPara'),
												'command': '.uno:JustifyPara',
												'accessibility': { focusBack: true, combination: 'PJ', de: null }
											}
										]
									},
								],
								'vertical': 'false'
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-justifypara-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-paragraph',
				'name':_('Paragraph'),
				'accessibility': { focusBack: true, combination: 'BD', de: null },
				'more': {
					'command':'.uno:ParagraphDialog',
					'accessibility': { focusBack: true, combination: 'MP', de: null }
				},
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'container',
								'children': [
									{
										'type': 'toolbox',
										'children': [
											{
												'id': 'home-default-bullet',
												'type': 'toolitem',
												'text': _UNO('.uno:DefaultBullet'),
												'command': '.uno:DefaultBullet',
												'accessibility': { focusBack: true, combination: 'BD', de: null }
											},
											{
												'id': 'home-default-numbering',
												'type': 'toolitem',
												'text': _UNO('.uno:DefaultNumbering'),
												'command': '.uno:DefaultNumbering',
												'accessibility': { focusBack: true, combination: 'ND', de: null }
											},
											{
												'id': 'home-increment-indent',
												'type': 'toolitem',
												'text': _UNO('.uno:IncrementIndent'),
												'command': '.uno:IncrementIndent',
												'accessibility': { focusBack: true, combination: 'II', de: null }
											},
											{
												'id': 'home-decrement-indent',
												'type': 'toolitem',
												'text': _UNO('.uno:DecrementIndent'),
												'command': '.uno:DecrementIndent',
												'accessibility': { focusBack: true, combination: 'ID', de: null }
											},
											{
												'id': 'home-para-left-to-right',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaLeftToRight'),
												'command': '.uno:ParaLeftToRight',
												'accessibility': { focusBack: true, combination: 'LT', de: null }
											}
										]
									},
								],
								'vertical': 'false'
							},
							{
								'type': 'container',
								'children': [
									{
										'id': 'SectionBottom13',
										'type': 'toolbox',
										'children': [
											{
												'id': 'home-para-space-increase',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaspaceIncrease'),
												'command': '.uno:ParaspaceIncrease',
												'accessibility': { focusBack: true, combination: 'SI', de: null }
											},
											{
												'id': 'home-para-space-decrease',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaspaceDecrease'),
												'command': '.uno:ParaspaceDecrease',
												'accessibility': { focusBack: true, combination: 'SD', de: null }
											},
											{
												'id': 'home-line-spacing:LineSpacingMenu',
												'type': 'menubutton',
												'noLabel': true,
												'text': _UNO('.uno:LineSpacing'),
												'command': '.uno:LineSpacing',
												'accessibility': { focusBack: false, combination: 'LS', de: null }
											},
											{
												'id': 'home-para-right-to-left',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaRightToLeft'),
												'command': '.uno:ParaRightToLeft',
												'accessibility': { focusBack: true, combination: 'RT', de: null }
											}
										]
									},
								],
								'vertical': 'false'
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-pararighttoleft-break', orientation: 'vertical' },
			{
				'id': 'home-text',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Text'),
				'command': '.uno:Text',
				'accessibility': { focusBack: true, combination: 'DT', de: null }
			},
			{ type: 'separator', id: 'home-inserttext-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-illustrations',
				'name':_('Illustrations'),
				'accessibility': { focusBack: true, combination: 'IS', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'id': 'LineA6',
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-basic-shapes:InsertShapesMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _('Shapes'),
										'command': '.uno:BasicShapes',
										'accessibility': { focusBack: true, combination: 'IS', de: null }
									}
								]
							},
							{
								'id': 'LineB7',
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-connector-toolbox:InsertConnectorsMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:ConnectorToolbox', 'presentation'),
										'command': '.uno:ConnectorToolbox',
										'accessibility': { focusBack: true, combination: 'TC', de: null }
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
										'id': 'home-insert-graphic:InsertImageMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:InsertGraphic'),
										'command': '.uno:InsertGraphic',
										'accessibility': { focusBack: true, combination: 'IG', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-insert-object-chart',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertObjectChart'),
										'command': '.uno:InsertObjectChart',
										'accessibility': { focusBack: true, combination: 'IC', de: null }
									},
									{
										'id': 'home-insert-table:InsertTableMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:InsertTable', 'presentation'),
										'command': '.uno:InsertTable',
										'accessibility': { focusBack: true, combination: 'IT', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},

				]
			},

			{ type: 'separator', id: 'home-insertconnectorsmenu-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-xline-color:ColorPickerMenu',
								'noLabel': true,
								'class': 'unospan-XLineColor',
								'type': 'toolitem',
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor',
								'accessibility': { focusBack: true, combination: 'NC', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'home-fill-color:ColorPickerMenu',
								'noLabel': true,
								'class': 'unospan-BackgroundColor',
								'type': 'toolitem',
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor',
								'accessibility': { focusBack: true, combination: 'LC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'home-fillcolorpickermenu-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-page-insert',
				'name':_('Insert'),
				'accessibility': { focusBack: true, combination: 'IP', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
										{
											'id': 'home-insert-page',
											'type': 'toolitem',
											'text': _UNO('.uno:InsertPage', 'presentation'),
											'command': '.uno:InsertPage',
											'accessibility': { focusBack: true, combination: 'IP', de: null }
										},
									]
								},
								{
									'type': 'toolbox',
									'children': [
										{
											'id': 'home-duplicate-page',
											'type': 'toolitem',
											'text': _UNO('.uno:DuplicatePage', 'presentation'),
											'command': '.uno:DuplicatePage',
											'accessibility': { focusBack: true, combination: 'DP', de: null }
										},
									]
								}
							],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-inserttablemenu-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-find-n-filter',
				'name':_('Search'),
				'accessibility': { focusBack: true, combination: 'SS', de: null },
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
				]
			},
		];

		return this.getTabPage('Home', content);
	},

	getLayoutTab: function() {
		var content = [
			{
				'id': 'layout-header-and-footer',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:HeaderAndFooter', 'presentation'),
				'command': '.uno:HeaderAndFooter',
				'accessibility': { focusBack: true, combination: 'IH', de: null }
			},
			{ type: 'separator', id: 'layout-headerandfooter-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-page',
				'name':_('Page'),
				'accessibility': { focusBack: true, combination: 'IP', de: null },
				'children' : [
					{
						'id': 'layout-insert-page',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:InsertPage', 'presentation'),
						'command': '.uno:InsertPage',
						'accessibility': { focusBack: true, combination: 'IP', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-duplicate-page',
										'type': 'toolitem',
										'text': _UNO('.uno:DuplicatePage', 'presentation'),
										'command': '.uno:DuplicatePage',
										'accessibility': { focusBack: true, combination: 'DP', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-page-setup',
										'type': 'toolitem',
										'text': _UNO('.uno:PageSetup', 'presentation'),
										'command': '.uno:PageSetup',
										'accessibility': { focusBack: true, combination: 'PS', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'selectbackground',
										'class': 'unoselectbackground',
										'type': 'customtoolitem',
										'text': _UNO('.uno:SelectBackground', 'presentation'),
										'accessibility': { focusBack: true, combination: 'SB', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},
			{ type: 'separator', id: 'layout-selectbackground-break', orientation: 'vertical' },
			{
				'id': 'layout-sidebar-deck',
				'type': 'bigtoolitem',
				'text': _('Page Layout'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true, combination: 'PL', de: null }
			},
			{ type: 'separator', id: 'layout-sidebardeck-break', orientation: 'vertical' },
			{
				'id': 'layout-select-all',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SelectAll'),
				'command': '.uno:SelectAll',
				'accessibility': { focusBack: true, combination: 'SA', de: null }
			},
			{ type: 'separator', id: 'layout-selectall-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'layout-align',
				'name':_('Align'),
				'accessibility': { focusBack: true, combination: 'AL', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true, combination: 'AL', de: null }
									},
									{
										'id': 'layout-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true, combination: 'AC', de: null }
									},
									{
										'id': 'layout-object-align-right',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignRight'),
										'command': '.uno:ObjectAlignRight',
										'accessibility': { focusBack: true, combination: 'AR', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true, combination: 'AU', de: null }
									},
									{
										'id': 'layout-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true, combination: 'AM', de: null }
									},
									{
										'id': 'layout-align-down',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignDown'),
										'command': '.uno:AlignDown',
										'accessibility': { focusBack: true, combination: 'AD', de: null }
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
				'accessibility': { focusBack: true, combination: 'FO', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-forward-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectForwardOne'),
										'command': '.uno:ObjectForwardOne',
										'accessibility': { focusBack: true, combination: 'FO', de: null }
									},
									{
										'id': 'layout-bring-to-front',
										'type': 'toolitem',
										'text': _UNO('.uno:BringToFront'),
										'command': '.uno:BringToFront',
										'accessibility': { focusBack: true, combination: 'BF', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'layout-object-backwards-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true, combination: 'BO', de: null }
									},
									{
										'id': 'layout-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true, combination: 'SB', de: null }
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

	getInsertTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'insert-page',
				'name':_('Page'),
				'accessibility': { focusBack: true, combination: 'IP', de: null },
				'children' : [
					{
						'id': 'insert-insert-page',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:InsertPage', 'presentation'),
						'command': '.uno:InsertPage',
						'accessibility': { focusBack: true, combination: 'IP', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-duplicate-page',
										'type': 'toolitem',
										'text': _UNO('.uno:DuplicatePage', 'presentation'),
										'command': '.uno:DuplicatePage',
										'accessibility': { focusBack: true, combination: 'DP', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-delete-page',
										'type': 'toolitem',
										'text': _UNO('.uno:DeletePage', 'presentation'),
										'command': '.uno:DeletePage',
										'accessibility': { focusBack: true, combination: 'RP', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-deletepage-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-illustrations',
				'name':_('Illustrations'),
				'accessibility': { focusBack: true, combination: 'IG', de: null },
				'children' : [
					{
						'id': 'insert-insert-graphic:InsertImageMenu',
						'type': 'menubutton',
						'text': _UNO('.uno:InsertGraphic'),
						'command': '.uno:InsertGraphic',
						'accessibility': { focusBack: true, combination: 'IG', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-insert-table:InsertTableMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _('Table'),
										'command': '.uno:InsertTable',
										'accessibility': { focusBack: true, combination: 'IT', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-insert-object-chart',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertObjectChart', 'presentation'),
										'command': '.uno:InsertObjectChart',
										'accessibility': { focusBack: true, combination: 'IC', de: null }
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
										'id': 'insert-basic-shapes:InsertShapesMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _('Shapes'),
										'command': '.uno:BasicShapes',
										'accessibility': { focusBack: true, combination: 'BS', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-line',
										'type': 'toolitem',
										'text': _UNO('.uno:Line', 'presentation'),
										'command': '.uno:Line',
										'accessibility': { focusBack: true, combination: 'IL', de: null }
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
										'id': 'bezier_unfilled',
										'type': 'toolitem',
										'text': _UNO('.uno:Bezier_Unfilled', 'presentation'),
										'command': '.uno:Bezier_Unfilled',
										'accessibility': { focusBack: true, combination: 'CR', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},
			{ type: 'separator', id: 'insert-insertobjectchart-break', orientation: 'vertical' },
			{
				'id': 'HyperlinkDialog',
				'class': 'unoHyperlinkDialog',
				'type': 'bigcustomtoolitem',
				'text': _UNO('.uno:HyperlinkDialog'),
				'command': 'hyperlinkdialog',
				'accessibility': { focusBack: true, combination: 'HD', de: null }
			},
			{ type: 'separator', id: 'insert-hyperlinkdialog-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-field',
				'name':_('Field'),
				'accessibility': { focusBack: true, combination: 'RL', de: null },
				'children' : [
					(this.map['wopi'].EnableRemoteLinkPicker) ? {
						'id': 'insert-remote-link',
						'class': 'unoremotelink',
						'type': 'bigcustomtoolitem',
						'text': _('Smart Picker'),
						'command': 'remotelink',
						'accessibility': { focusBack: true, combination: 'RL', de: null }
					} : {},
					(this.map['wopi'].EnableRemoteAIContent) ? {
						'id': 'insert-insert-remote-ai-content',
						'class': 'unoremoteaicontent',
						'type': 'bigcustomtoolitem',
						'text': _('Assistant'),
						'command': 'remoteaicontent',
						'accessibility': { focusBack: true, combination: 'RL', de: null }
					} : {},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-innsert-date-field-fix',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertDateFieldFix', 'presentation'),
										'command': '.uno:InsertDateFieldFix',
										'accessibility': { focusBack: true, combination: 'ID', de: null }
									},
									{
										'id': 'insert-insert-date-field-var',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertDateFieldVar', 'presentation'),
										'command': '.uno:InsertDateFieldVar',
										'accessibility': { focusBack: true, combination: 'IV', de: null }
									},
									{
										'id': 'insert-insert-page-field',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertPageField', 'presentation'),
										'command': '.uno:InsertPageField',
										'accessibility': { focusBack: true, combination: 'IF', de: null }
									},
									{
										'id': 'insert-insert-pages-field',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertPagesField', 'presentation'),
										'command': '.uno:InsertPagesField',
										'accessibility': { focusBack: true, combination: 'IS', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-time-field-fix',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertTimeFieldFix', 'presentation'),
										'command': '.uno:InsertTimeFieldFix',
										'accessibility': { focusBack: true, combination: 'TF', de: null }
									},
									{
										'id': 'insert-time-field-var',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertTimeFieldVar', 'presentation'),
										'command': '.uno:InsertTimeFieldVar',
										'accessibility': { focusBack: true, combination: 'TV', de: null }
									},
									{
										'id': 'insert-page-title-field',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertPageTitleField', 'presentation'),
										'command': '.uno:InsertPageTitleField',
										'accessibility': { focusBack: true, combination: 'PT', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-pagetitlefield-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-text',
				'name':_('Text'),
				'accessibility': { focusBack: true, combination: 'TI', de: null },
				'children' : [
					{
						'id': 'insert-insert-text',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Text'),
						'command': '.uno:Text',
						'accessibility': { focusBack: true, combination: 'TI', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-font-gallery-floater',
										'type': 'toolitem',
										'text': _UNO('.uno:FontworkGalleryFloater'),
										'command': '.uno:FontworkGalleryFloater',
										// Fontwork export/import not supported in other formats.
										'visible': app.LOUtil.isFileODF(this.map) ? 'true' : 'false',
										'accessibility': { focusBack: true, combination: 'FG', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-vertical-text',
										'type': 'toolitem',
										'text': _UNO('.uno:VerticalText', 'presentation'),
										'command': '.uno:VerticalText',
										'accessibility': { focusBack: true, combination: 'VT', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'insert-insertannotation-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'CharmapControl',
								'class': 'unoCharmapControl',
								'type': 'toolitem',
								'text': _UNO('.uno:CharmapControl'),
								'command': '.uno:InsertSymbol',
								'accessibility': { focusBack: true, combination: 'CM', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-annnotation',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertAnnotation', 'text'),
								'command': '.uno:InsertAnnotation',
								'accessibility': { focusBack: true, combination: 'IA', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
		];

		return this.getTabPage('Insert', content);
	},

	getFormatTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'format-style',
				'name':_('Style'),
				'accessibility': { focusBack: true, combination: 'FD', de: null },
				'children' : [
					{
						'id': 'format-font-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FontDialog'),
						'command': '.uno:FontDialog',
						'accessibility': { focusBack: true, combination: 'FD', de: null }
					},
					{
						'id': 'format-paragraph-dialog',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:ParagraphDialog'),
						'command': '.uno:ParagraphDialog',
						'accessibility': { focusBack: true, combination: 'PD', de: null }
					},
				]
			},
			{ type: 'separator', id: 'font-paragraphdialog-break', orientation: 'vertical' },
			{
				'id': 'format-outline-bullet',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:OutlineBullet'),
				'command': '.uno:OutlineBullet',
				'accessibility': { focusBack: true, combination: 'OB', de: null }
			},
			{ type: 'separator', id: 'font-outlinebullet-break', orientation: 'vertical' },
			{
				'id': 'format-page-setup',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:PageSetup', 'presentation'),
				'command': '.uno:PageSetup',
				'accessibility': { focusBack: true, combination: 'PS', de: null }
			},
			{ type: 'separator', id: 'font-pagesetup-break', orientation: 'vertical' },
			{
				'id': 'format-line',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatLine'),
				'command': '.uno:FormatLine',
				'accessibility': { focusBack: true, combination: 'FL', de: null }
			},
			{
				'id': 'format-area',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatArea'),
				'command': '.uno:FormatArea',
				'accessibility': { focusBack: true, combination: 'FA', de: null }
			},
			{ type: 'separator', id: 'font-formatarea-break', orientation: 'vertical' },
			{
				'id': 'format-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: true, combination: 'TD', de: null }
			},
			{ type: 'separator', id: 'font-transformdialog-break', orientation: 'vertical' },
			{
				'id': 'format-name-description',
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'format-name-group',
								'type': 'toolitem',
								'text': _UNO('.uno:NameGroup', 'text'),
								'command': '.uno:NameGroup',
								'accessibility': { focusBack: false, combination: 'NG', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'format-object-title-description',
								'type': 'toolitem',
								'text': _UNO('.uno:ObjectTitleDescription', 'text'),
								'command': '.uno:ObjectTitleDescription',
								'accessibility': { focusBack: false, combination: 'DS', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'font-objecttitledescription-break', orientation: 'vertical' },
			{
				'id': 'format-theme-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ThemeDialog'),
				'command': '.uno:ThemeDialog',
				'accessibility': { focusBack: false, combination: 'J', de: null }
			}
		];

		return this.getTabPage('Format', content);
	},

});

window.L.control.notebookbarDraw = function (options) {
	return new window.L.Control.NotebookbarDraw(options);
};
