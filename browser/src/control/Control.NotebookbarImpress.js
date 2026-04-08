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
 * window.L.Control.NotebookbarImpress - definition of notebookbar content in Impress
 */

/* global _ _UNO app JSDialog */

window.L.Control.NotebookbarImpress = window.L.Control.NotebookbarWriter.extend({

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
		return this.buildOptionsSectionData(this.getDefaultToolItems());
	},

	getDefaultToolItems: function() {
		const optionsToolItems = [
			{
				'id': 'options-modify-page',
				'type': 'toolitem',
				'text': _UNO('.uno:ModifyPage', 'presentation', true),
				'command': '.uno:ModifyPage',
				'accessibility': { focusBack: true, combination: 'ZL', de: null }
			},
			{
				'id': 'options-custom-animation',
				'type': 'toolitem',
				'text': _UNO('.uno:CustomAnimation', 'presentation', true),
				'command': '.uno:CustomAnimation',
				'accessibility': { focusBack: true, combination: 'ZA', de: null }
			},
		];
		if (this._map && this._map['wopi'].EnableShare) {
			optionsToolItems.push({
				'type': 'customtoolitem',
				'text': _('Share'),
				'command': 'shareas',
				'inlineLabel': true,
				'accessibility': { focusBack: false, combination: 'ZS', de: null }
			});
		}
		return optionsToolItems;
	},

	getTabs: function() {
		return [
			{
				'id': 'File-tab-label',
				'text': _('File'),
				'name': 'File',
				'accessibility': { focusBack: false, combination: 'F', de: null }
			},
			{
				'id': this.HOME_TAB_ID,
				'text': _('Home'),
				'name': 'Home',
				'context': 'default|DrawText',
				'accessibility': { focusBack: false, combination: 'H', de: null }
			},
			{
				'id': 'Insert-tab-label',
				'text': _('Insert'),
				'name': 'Insert',
				'accessibility': { focusBack: false, combination: 'N', de: null }
			},
			{
				'id': 'Design-tab-label',
				'text': _('Design'),
				'name': 'Design',
				'accessibility': { focusBack: false, combination: 'P', de: null }
			},
			JSDialog.ImpressTransitionTab.getEntry(),
			// JSDialog.ImpressAnimationTab.getEntry(), requires core change
			{
				'id': 'Slideshow-tab-label',
				'text': _('Slide Show'),
				'name': 'Slideshow',
				'accessibility': { focusBack: false, combination: 'SE', de: null }
			},
			{
				'id': 'Review-tab-label',
				'text': _('Review'),
				'name': 'Review',
				'accessibility': { focusBack: false, combination: 'R', de: null }
			},
			{
				'id': 'Format-tab-label',
				'text': _('Format'),
				'name': 'Format',
				'accessibility': { focusBack: false, combination: 'O', de: null }
			},
			{
				'id': 'Table-tab-label',
				'text': _('Table'),
				'name': 'Table',
				'context': 'Table',
				'accessibility': { focusBack: false, combination: 'T', de: null }
			},
			{
				'id': 'Shape-tab-label',
				'text': _('Shape'),
				'name': 'Shape',
				'context': 'Draw|DrawLine|3DObject|MultiObject|DrawFontwork',
				'accessibility': { focusBack: false, combination: 'D', de: null }
			},
			{
				'id': 'Picture-tab-label',
				'text': _('Picture'),
				'name': 'Picture',
				'context': 'Graphic',
				'accessibility': { focusBack: false, combination: 'I', de: null }
			},
			{
				'id': 'Chart-tab-label',
				'text': _('Chart'),
				'name': 'Chart',
				'context': 'Chart|Series|ErrorBar|Axis|Grid|ChartElements|Trendline|ChartTitle|ChartLegend|ChartLabel',
				'accessibility': { focusBack: false, combination: 'CH', de: null }
			},
			{
				'id': 'MasterPage-tab-label',
				'text': _('Master'),
				'name': 'MasterPage',
				'context': 'MasterPage',
				'accessibility': { focusBack: false, combination: 'M', de: null }
			},
			{
				'id': 'View-tab-label',
				'text': _('View'),
				'name': 'View',
				'accessibility': { focusBack: false, combination: 'W', de: null }
			},
			{
				'id': 'Help-tab-label',
				'text': _('Help'),
				'name': 'Help',
				'accessibility': { focusBack: false, combination: 'Y', de: null }
			}
		];
	},

	getTabsJSON: function () {
		return [
			this.getFileTab(),
			this.getHomeTab(),
			this.getInsertTab(),
			this.getDesignTab(),
			this.getTransitionTab(),
			// this.getAnimationTab(), requires core change
			this.getSlideshowTab(),
			this.getReviewTab(),
			this.getFormatTab(),
			this.getTableTab(),
			this.getShapeTab(),
			this.getPictureTab(),
			this.getChartTab(),
			this.getMasterTab(),
			this.getViewTab(),
			this.getHelpTab()
		];
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
			content.push(
			{
				'type': 'container',
				'children': [
					{
						'id': 'file-save',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Save'),
						'command': '.uno:Save',
						'accessibility': { focusBack: true, combination: 'SV', de: null }
					}
				]
			});
		}

		if (hasSaveAs) {
			content.push(
				(window.prefs.get('saveAsMode') === 'group') ?
				{
					'id': 'saveas:SaveAsMenu',
					'command': 'saveas',
					'class': 'unosaveas',
					'type': 'exportmenubutton',
					'text': _('Save As'),
					'accessibility': { focusBack: true, combination: 'SA', de: null }
				}:
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
			});
		}

		if (hasShare && hasRevisionHistory) {
			content.push(
				{
					'id': 'file-exportas-break',
					'type': 'separator',
					'orientation': 'vertical'
				},
				{
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
				},
				{
					'id': 'file-revhistory-break',
					'type': 'separator',
					'orientation': 'vertical'
				}
			);
		} else if (hasShare) {
			content.push({
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
					'id': 'file-revhistory-break',
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
				'id': 'file-print:PrintOptions',
				'type': 'exportmenubutton',
				'text': _UNO('.uno:Print', 'presentation'),
				'command': '.uno:Print',
				'applyCallback': 'print',
				'accessibility': { focusBack: true, combination: 'PF', de: null }
			});
		}

		if (window.enableMacrosExecution) {
			content.push(
			{
				'type': 'toolbox',
				'children': [
					{
						'id': 'file-runmacro',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:RunMacro', 'text'),
						'command': '.uno:RunMacro',
						'accessibility': { focusBack: true, combination: 'RM', de: null }
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
				'accessibility': { focusBack: true, combination: 'DA', de: null }
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
						'accessibility': { focusBack: true, combination: 'RF', de: null }
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
						'accessibility': { focusBack: true, combination: 'FP', de: 'I' }
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

	getTransitionTab: function() {
		const tab = JSDialog.ImpressTransitionTab;
		return this.getTabPage(tab.getName(), tab.getContent());
	},

	getAnimationTab: function() {
		const tab = JSDialog.ImpressAnimationTab;
		return this.getTabPage(tab.getName(), tab.getContent());
	},

	getSlideshowTab: function() {
		var content = [
			window.mode.isTablet() ?
				{
					'id': 'closemobile',
					'type': 'bigcustomtoolitem',
					'text': _('Read mode'),
					'command': 'closetablet'
				} : {},
			{
				'type': 'overflowgroup',
				'id': 'slide-start',
				'name': _('Start Slide Show'),
				'children' : [
					{
						'id': 'slide-fullscreen-presentation',
						'type': 'bigcustomtoolitem',
						'text': _('From Beginning'),
						'tooltip': _('Fullscreen, starting at slide 1'),
						'command': 'fullscreen-presentation',
						'accessibility': { focusBack: true, combination: 'FB', de: null }
					},
					{
						'id': 'slide-presentation-currentslide',
						'type': 'bigcustomtoolitem',
						'text':  _('From Current Slide'),
						'tooltip': _('Fullscreen, starting at this slide'),
						'command': 'presentation-currentslide',
						'accessibility': { focusBack: true, combination: 'FC', de: null }
					},
				],
			},
			!window.ThisIsAMobileApp || window.mode.isCODesktop() ?
				{
					type: 'separator',
					id: 'slide-show-presentation',
					orientation: 'vertical'
				} : {},
			{
				'type': 'overflowgroup',
				'id': 'slide-present',
				'name': _('Present'),
				'children' : [
				!window.ThisIsAMobileApp || window.mode.isCODesktop() ?
					{
						'id': 'slide-presentation-in-window',
						'type': 'bigcustomtoolitem',
						'text': _('Present in Window'),
						'tooltip': _('Plays inside the document window. Starts at slide 1'),
						'command': 'presentinwindow',
						'accessibility': { focusBack: true, combination: 'PW', de: null }
					} : {},
				(!window.ThisIsAMobileApp || window.mode.isCODesktop()) && window.canvasSlideshowEnabled ?
				{
						'id': 'slide-presentation-in-console',
						'type': 'bigcustomtoolitem',
						'text': _('Presenter View'),
						'tooltip': _('Shows your notes, next slide, and a timer'),
						'command': 'presenterconsole',
						'accessibility': { focusBack: true, combination: 'PC', de: null }
					}: {},
				!window.ThisIsAMobileApp ?
					{
						'id': 'slide-presentation-follow-me',
						'type': 'bigcustomtoolitem',
						'text': _('Present to All'),
						'tooltip': _('Starts a slideshow on every viewer\'s screen'),
						'command': 'followmepresentation',
						'accessibility': { focusBack: true, combination: 'PL', de: null }
					} : {},
				!window.ThisIsAMobileApp ?
					{
						'id': 'slide-presentation-follow',
						'type': 'bigcustomtoolitem',
						'text': _('Follow Presenter'),
						'tooltip': _('View slides as the presenter advances them'),
						'command': 'followpresentation',
						'accessibility': { focusBack: true, combination: 'PF', de: null }
					} : {},
				],
			},
			{ type: 'separator', id: 'slide-zoomin-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'slide-set-up',
				'name':_('Set Up'),
				'children' : [
					{
						'id': 'showslide',
						'type': 'bigcustomtoolitem',
						'text': _UNO('.uno:ShowSlide', 'presentation'),
						'accessibility': { focusBack: true, combination: 'SS', de: null }
					},
					{
						'id': 'hideslide',
						'class': 'unohideslide',
						'type': 'bigcustomtoolitem',
						'text': _UNO('.uno:HideSlide', 'presentation'),
						'accessibility': { focusBack: true, combination: 'HS', de: null }
					},
				],
			},
		];

		return this.getTabPage('Slideshow', content);
	},

	getViewTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'view-zoom',
				'name':_('Zoom'),
				'accessibility': { focusBack: true, combination: 'FS', de: null },
				'children' : [
					{
						'id': 'fullscreen',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FullScreen'),
						'command': '.uno:FullScreen',
						'accessibility': { focusBack: true, combination: 'FS', de: null }
					},
					{
						'id': 'zoomreset',
						'class': 'unozoomreset',
						'type': 'bigcustomtoolitem',
						'text': _('Reset zoom'),
						'accessibility': { focusBack: true, combination: 'ZR', de: null }
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
				'accessibility': { focusBack: true, combination: 'TU', de: null }
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
				'id': 'view-slide-view',
				'name':_('Slide Views'),
				'accessibility': { focusBack: true, combination: 'NV', de: null },
				'children' : [
					{
						'id': 'notesmode',
						'class': 'notesmode',
						'type': 'bigcustomtoolitem',
						'text': _('Notes View'),
						'accessibility': { focusBack: false, combination: 'NV' }
					},
					{
						'id': 'view-master-view',
						'type': 'bigtoolitem',
						'text': _('Master View'),
						'command': '.uno:SlideMasterPage',
						'accessibility': { focusBack: true, combination: 'MP', de: null }
					},
				]
			},
			{ type: 'separator', id: 'view-masterview-break', orientation: 'vertical' },
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
								'type': 'bigtoolitem',
								'text': _('Show Grid'),
								'command': '.uno:GridVisible',
								'accessibility': { focusBack: true, combination: 'GV', de: null }
							},
							{
								'id': 'home-grid-use',
								'type': 'bigtoolitem',
								'text': _('Snap to Grid'),
								'command': '.uno:GridUse',
								'accessibility': { focusBack: true, combination: 'GU', de: null }
							}
						],
					},
				]
			},
			{ type: 'separator', id: 'view-griduse-break', orientation: 'vertical' },
			{
				'id':'toggledarktheme',
				'class': 'unotoggledarktheme',
				'type': 'bigcustomtoolitem',
				'text': _('Dark Mode'),
				'accessibility': { focusBack: true, combination: 'TT', de: null }
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
				'id': 'view-side-bar',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:Sidebar'),
				'command': '.uno:SidebarDeck.PropertyDeck',
				'accessibility': { focusBack: true, combination: 'SD', de: null }
			},
			!this.map['wopi'].DisableAISettings ? {
				'id': 'view-ai-sidebar',
				'type': 'bigcustomtoolitem',
				'text': _('AI Assistant'),
				'tooltip': _('AI Assistant'),
				'icon': 'lc_ai_sidebar.svg',
				'command': 'aichat',
				'accessibility': { focusBack: true, combination: 'AI', de: null }
			} : {}
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
										'class': 'unoResetAttributes',
										'type': 'toolitem',
										'text': _UNO('.uno:SetDefault'),
										'command': '.uno:SetDefault',
										'accessibility': { focusBack: true, combination: 'SD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-slidefunctions-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-slide-layout',
				'name':_('Slide Layout'),
				'accessibility': { focusBack: true, combination: 'NS', de: null },
				'more': {
					'command':'.uno:PageSetup',
					'accessibility': { focusBack: true, combination: 'ML', de: null }
				},
				'children' : [
					{
						'id': 'home-create-slide:NewSlideLayoutMenu',
						'type': 'menubutton',
						'applyCallback': '.uno:InsertPage',
						'text': _('New'),
						'command': '.uno:InsertPage',
						'accessibility': { focusBack: true, combination: 'NS', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'id': 'home-change-layout:ChangeSlideLayoutMenu',
								'type': 'menubutton',
								'noLabel': true,
								'text': _('Change Layout'),
								'icon': 'lc_changelayout.svg',
								'command': '.uno:AssignLayout',
								'accessibility': { focusBack: true, combination: 'GL', de: null }
							},
							{
								'id': 'home-assign-layout',
								'noLabel': true,
								'text': _('Reset Layout'),
								'type': 'toolitem',
								'command': '.uno:AssignLayout',
								'accessibility': { focusBack: true, combination: 'RS', de: null }
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-resertattributes-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-character',
				'name':_('Character'),
				'accessibility': { focusBack: true, combination: 'FN', de: null },
				'more': {
					'command':'.uno:FontDialog',
					'accessibility': { focusBack: true, combination: 'MC', de: null }
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
												'id': 'home-superscript',
												'type': 'toolitem',
												'text': _UNO('.uno:SuperScript'),
												'command': '.uno:SuperScript',
												'accessibility': { focusBack: true, combination: '6', de: '6' }
											},
											{
												'id': 'home-spacing:CharSpacingMenu',
												'type': 'menubutton',
												'noLabel': true,
												'text': _UNO('.uno:Spacing'),
												'command': '.uno:CharSpacing',
												'accessibility': { focusBack: false, combination: 'FT', de: null }
											},
											{
												'id': 'home-charbackcolor:ColorPickerMenu',
												'noLabel': true,
												'class': 'unospan-CharBackColor',
												'type': 'toolitem',
												'text': _UNO('.uno:CharBackColor'),
												'command': '.uno:CharBackColor',
												'accessibility': { focusBack: true, combination: 'HC', de: null }
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
			{ type: 'separator', id: 'home-fontcombobox-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-paragraph',
				'more': {
					'command':'.uno:ParagraphDialog',
					'accessibility': { focusBack: true, combination: 'MP', de: null }
				},
				'name':_('Paragraph'),
				'accessibility': { focusBack: true, combination: 'DB', de: null },
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
												'accessibility': { focusBack: true, combination: 'DB', de: null }
											},
											{
												'id': 'home-default-numbering',
												'type': 'toolitem',
												'text': _UNO('.uno:DefaultNumbering'),
												'command': '.uno:DefaultNumbering',
												'accessibility': { focusBack: true, combination: 'DN', de: null }
											},
											{
												'id': 'home-increment-indent',
												'type': 'toolitem',
												'text': _UNO('.uno:IncrementIndent'),
												'command': '.uno:IncrementIndent',
												'accessibility': { focusBack: true, combination: 'AI', de: null }
											},
											{
												'id': 'home-decrement-indent',
												'type': 'toolitem',
												'text': _UNO('.uno:DecrementIndent'),
												'command': '.uno:DecrementIndent',
												'accessibility': { focusBack: true, combination: 'AO', de: null }
											},
											{
												'id': 'home-para-left-to-right',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaLeftToRight'),
												'command': '.uno:ParaLeftToRight',
												'accessibility': { focusBack: true, combination: 'TR', de: null }
											},
											{
												'id': 'home-para-right-to-left',
												'type': 'toolitem',
												'text': _UNO('.uno:ParaRightToLeft'),
												'command': '.uno:ParaRightToLeft',
												'accessibility': { focusBack: true, combination: 'TL', de: null }
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
												'id': 'home-left-paragraph',
												'type': 'toolitem',
												'text': _UNO('.uno:LeftPara'),
												'command': '.uno:LeftPara',
												'accessibility': { focusBack: true, combination: 'PL', de: null }
											},
											{
												'id': 'home-center-paragraph',
												'type': 'toolitem',
												'text': _UNO('.uno:CenterPara'),
												'command': '.uno:CenterPara',
												'accessibility': { focusBack: true, combination: 'PC', de: null }
											},
											{
												'id': 'home-right-paragraph',
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
											},
											{
												'id': 'home-line-spacing:LineSpacingMenu',
												'type': 'menubutton',
												'noLabel': true,
												'text': _UNO('.uno:LineSpacing'),
												'command': '.uno:LineSpacing',
												'accessibility': { focusBack: false, combination: 'SL', de: null }
											},
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
			{ type: 'separator', id: 'home-linespacingmenu-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'home-spacing-color',
				'name':_('Spacing & Color'),
				'accessibility': { focusBack: true, combination: 'LT', de: null },
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
												'accessibility': { focusBack: true, combination: 'LT', de: null }
											},
											{
												'id': 'home-cell-vertical-center',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertCenter'),
												'command': '.uno:CellVertCenter',
												'accessibility': { focusBack: true, combination: 'LC', de: null }
											},
											{
												'id': 'home-cell-vertical-bottom',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertBottom'),
												'command': '.uno:CellVertBottom',
												'accessibility': { focusBack: true, combination: 'LB', de: null }
											}
										]
									},
								],
								'vertical': 'false'
							},
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
										'accessibility': { focusBack: true, combination: 'LR', de: null }
									},
									{
										'id': 'home-fill-color:ColorPickerMenu',
										'noLabel': true,
										'class': 'unospan-FillColor',
										'type': 'toolitem',
										'text': _UNO('.uno:FillColor'),
										'command': '.uno:FillColor',
										'accessibility': { focusBack: true, combination: 'BC', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-fillcolormenu-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'illustrations',
				'name':_('Illustrations'),
				'accessibility': { focusBack: true, combination: 'IH', de: null },
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
										'accessibility': { focusBack: true, combination: 'IH', de: null }
									}
								]
							},
							{
								'id': 'LineB7',
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-connector-tool-box:InsertConnectorsMenu',
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
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'home-text',
										'type': 'toolitem',
										'text': _UNO('.uno:Text'),
										'command': '.uno:Text',
										'accessibility': { focusBack: true, combination: 'TI', de: null }
									},
									{
										'id': 'home-object-chart',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertObjectChart'),
										'command': '.uno:InsertObjectChart',
										'accessibility': { focusBack: true, combination: 'IC', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'home-insertobjectchart-break', orientation: 'vertical' },
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
			{ type: 'separator', id: 'format-paragraphdialog-break', orientation: 'vertical' },
			{
				'id': 'format-outline-bullet',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:OutlineBullet'),
				'command': '.uno:OutlineBullet',
				'accessibility': { focusBack: true, combination: 'OB', de: null }
			},
			{ type: 'separator', id: 'format-outlinebullet-break', orientation: 'vertical' },
			{
				'id': 'format-slide-setup',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SlideSetup', 'presentation'),
				'command': '.uno:PageSetup',
				'accessibility': { focusBack: true, combination: 'SS', de: null }
			},
			{ type: 'separator', id: 'format-slidesetup-break', orientation: 'vertical' },
			{
				'id': 'format-format-line',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatLine'),
				'command': '.uno:FormatLine',
				'accessibility': { focusBack: true, combination: 'FL', de: null }
			},
			{
				'id': 'format-format-area',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:FormatArea'),
				'command': '.uno:FormatArea',
				'accessibility': { focusBack: true, combination: 'FA', de: null }
			},
			{
				'id': 'format-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: true, combination: 'TD', de: null }
			},
			{ type: 'separator', id: 'format-transformdialog-break', orientation: 'vertical' },
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
		];

		return this.getTabPage('Format', content);
	},

	getInsertTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'insert-slide',
				'name':_('Slides'),
				'accessibility': { focusBack: true, combination: 'IP', de: null },
				'children' : [
					{
						'id': 'home-create-slide:NewSlideLayoutMenu',
						'type': 'menubutton',
						'applyCallback': '.uno:InsertPage',
						'text': _('New'),
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
										'id': 'insert-duplicate-slide',
										'type': 'toolitem',
										'text': _UNO('.uno:DuplicateSlide', 'presentation'),
										'command': '.uno:DuplicatePage',
										'accessibility': { focusBack: true, combination: 'DP', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-delete-slide',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteSlide', 'presentation'),
										'command': '.uno:DeletePage',
										'accessibility': { focusBack: true, combination: 'RP', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},
			{ type: 'separator', id: 'insert-deleteslide-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-illustrations',
				'name':_('Illustrations'),
				'icon': 'lc_insertgraphic.svg',
				'accessibility': { focusBack: true, combination: 'IG', de: null },
				'children' : [
										{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
									'id': 'insert-insert-graphic:InsertImageMenu',
									'type': 'menubutton',
									'noLabel': true,
									'text': _UNO('.uno:InsertGraphic'),
									'command': '.uno:InsertGraphic',
									'accessibility': { focusBack: true, combination: 'IG', de: null }
									},
								]
							},
							{
								'type': 'toolbox',
								'children': [
								!window.ThisIsAMobileApp ?
									{
										'id': 'insert-insert-multimedia:InsertMultimediaMenu',
										'type': 'menubutton',
										'noLabel': true,
										'text': _UNO('.uno:InsertAVMedia'),
										'command': 'insertmultimedia',
										'accessibility': { focusBack: true, combination: 'MM', de: null }, // IM was already taken, so 'MM' for MultiMedia
									} : {},
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
										'accessibility': { focusBack: true, combination: 'IS', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-presentation',
										'type': 'toolitem',
										'text': _UNO('.uno:Line', 'presentation'),
										'command': '.uno:Line',
										'accessibility': { focusBack: true, combination: 'IR', de: null }
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
			(this.map['wopi'].EnableRemoteLinkPicker) ? {
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-hyperlink-dialog',
								'class': 'unoHyperlinkDialog',
								'type': 'customtoolitem',
								'text': _UNO('.uno:HyperlinkDialog'),
								'command': 'hyperlinkdialog',
								'accessibility': { focusBack: true, combination: 'IL', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-insert-remote-link',
								'class': 'unoremotelink',
								'type': 'customtoolitem',
								'text': _('Smart Picker'),
								'command': 'remotelink',
								'accessibility': { focusBack: true, combination: 'RL', de: null }
							}
						]
					}
				],
			'vertical': 'true'
			} : {
				'id': 'HyperlinkDialog',
				'class': 'unoHyperlinkDialog',
				'type': 'bigcustomtoolitem',
				'text': _UNO('.uno:HyperlinkDialog'),
				'command': 'hyperlinkdialog',
				'accessibility': { focusBack: true, combination: 'IL', de: null }
			},
			{ type: 'separator', id: 'insert-hyperlinkdialog-break', orientation: 'vertical' },
			(this.map['wopi'].EnableRemoteAIContent) ? {
				'id': 'insert-insert-remote-ai-content',
				'class': 'unoremoteaicontent',
				'type': 'bigcustomtoolitem',
				'text': _('Assistant'),
				'command': 'remoteaicontent',
				'accessibility': { focusBack: true, combination: 'RL', de: null }
			} : {},
			{
				'id': 'insert-insert-annotation',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:InsertAnnotation', 'text'),
				'command': '.uno:InsertAnnotation',
				'accessibility': { focusBack: false, combination: 'ZC', de: 'ZC' }
			},
			{ type: 'separator', id: 'insert-insertannotation-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-field',
				'name':_('Field'),
				'accessibility': { focusBack: true, combination: 'DF', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-insert-date-field-fix',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertDateFieldFix', 'presentation'),
										'command': '.uno:InsertDateFieldFix',
										'accessibility': { focusBack: true, combination: 'DF', de: null }
									},
									{
										'id': 'insert-insert-date-field-var',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertDateFieldVar', 'presentation'),
										'command': '.uno:InsertDateFieldVar',
										'accessibility': { focusBack: true, combination: 'DV', de: null }
							},
							{
								'id': 'insert-insert-slide-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertSlideField', 'presentation'),
								'command': '.uno:InsertPageField',
								'accessibility': { focusBack: true, combination: 'SF', de: null }
							},
							{
								'id': 'insert-insert-slides-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertSlidesField', 'presentation'),
								'command': '.uno:InsertPagesField',
								'accessibility': { focusBack: true, combination: 'SM', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-insert-time-field-fix',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertTimeFieldFix', 'presentation'),
										'command': '.uno:InsertTimeFieldFix',
										'accessibility': { focusBack: true, combination: 'TF', de: null }
									},
									{
										'id': 'insert-insert-time-field-var',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertTimeFieldVar', 'presentation'),
										'command': '.uno:InsertTimeFieldVar',
										'accessibility': { focusBack: true, combination: 'TV', de: null }
							},
							{
								'id': 'insert-insert-slide-title-field',
								'type': 'toolitem',
								'text': _UNO('.uno:InsertSlideTitleField', 'presentation'),
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
			{ type: 'separator', id: 'insert-insertslidetitlefield-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'insert-text',
				'name':_('Text'),
				'accessibility': { focusBack: true, combination: 'IX', de: null },
				'children' : [
					{
						'id': 'insert-insert-text',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:Text'),
						'command': '.uno:Text',
						'accessibility': { focusBack: true, combination: 'IX', de: null }
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-fontwork-gallery-floater',
										'type': 'toolitem',
										'text': _UNO('.uno:FontworkGalleryFloater'),
										'command': '.uno:FontworkGalleryFloater',
										// Fontwork export/import not supported in other formats.
										'visible': app.LOUtil.isFileODF(this.map) ? 'true' : 'false',
										'accessibility': { focusBack: true, combination: 'FW', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'insert-vertical-text',
										'type': app.LOUtil.isFileODF(this.map) ? 'toolitem' : 'bigtoolitem',
										'text': _UNO('.uno:VerticalText', 'presentation'),
										'command': '.uno:VerticalText',
										'accessibility': { focusBack: true, combination: 'VT', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},
			{ type: 'separator', id: 'insert-verticaltext-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'insert-header-and-footer',
								'type': 'toolitem',
								'text': _UNO('.uno:HeaderAndFooter', 'presentation'),
								'command': '.uno:HeaderAndFooter',
								'accessibility': { focusBack: true, combination: 'HF', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'CharmapControl',
								'class': 'unoCharmapControl',
								'type': 'customtoolitem',
								'text': _UNO('.uno:CharmapControl'),
								'command': 'charmapcontrol',
								'accessibility': { focusBack: true, combination: 'IM', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
		];

		return this.getTabPage('Insert', content);
	},

	getDesignTab: function() {
		var content = [
			{
				'id': 'design-master-page-group',
				'type': 'overflowgroup',
				'name': _('Master Slide Templates'),
				'nofold': true,
				'icon': 'lc_masterslide.svg',
				'children': [
					{
						'id': 'masterpagenb',
						'type': 'iconviewlist',
						'accessibility': { focusBack: false, combination: 'MS', de: null },
						'children': [
							{
								'id': 'masterpagecurrent_label',
								'type': 'fixedtext',
								'text': _('This Presentation'),
							},
							{
								'id': 'masterpagecurrent_icons', // has to match core id
								'type': 'iconview',
							},
							{
								'id': 'masterpageall_label',
								'type': 'fixedtext',
								'text': _('Presentation Templates'),
							},
							{
								'id': 'masterpageall_icons', // has to match core id
								'type': 'iconview',
							}
						]
					}
				]
			},
			{ type: 'separator', id: 'design-masterslides-break', orientation: 'vertical' },
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
			{ type: 'separator', id: 'theme-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'design-slide-layout',
				'name':_('Slide Layout'),
				'accessibility': { focusBack: true, combination: 'CS', de: null },
				'more': {
					'command':'.uno:PageSetup',
					'accessibility': { focusBack: true, combination: 'ML', de: null }
				},
				'children': [
					{
						'id': 'home-change-layout:ChangeSlideLayoutMenu',
						'type': 'menubutton',
						'text': _('Change Layout'),
						'icon': 'lc_changelayout.svg',
						'command': '.uno:AssignLayout',
						'accessibility': { focusBack: true, combination: 'CL', de: null }
					},
					{
						'id': 'home-assign-layout',
						'text': _('Reset Layout'),
						'type': 'bigtoolitem',
						'command': '.uno:AssignLayout',
						'accessibility': { focusBack: true, combination: 'RS', de: null }
					}
				]
			},
			{ type: 'separator', id: 'design-slidelayout-break', orientation: 'vertical' },
			{
				'id': 'design-slide-size:SlideSizeMenu',
				'class': 'unoSlideSize',
				'type': 'menubutton',
				'text': _('Slide Size'),
				'command': 'slidesize',
				'accessibility': { focusBack: true, combination: 'SS', de: null }
			},
			{
				'id': 'design-slide-setup',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SlideSetup', 'presentation'),
				'command': '.uno:PageSetup',
				'accessibility': { focusBack: true, combination: 'SP', de: null }
			},
			{
				'id': 'design-header-and-footer',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:HeaderAndFooter', 'presentation'),
				'command': '.uno:HeaderAndFooter',
				'accessibility': { focusBack: true, combination: 'HF', de: null }
			},
			{ type: 'separator', id: 'design-headerandfooter-break', orientation: 'vertical' },
			{
				'id': 'design-selectbackground',
				'class': 'unoselectbackground',
				'type': 'bigtoolitem',
				'text': _('Background Image'),
				'command': '.uno:SelectBackground',
				'accessibility': { focusBack: true, combination: 'SB', de: null }
			},
		];

		return this.getTabPage('Design', content);
	},

	getMasterTab: function() {
		var content = [
			{
				'id': 'master-slide-setup',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SlideSetup', 'presentation'),
				'command': '.uno:PageSetup',
				'accessibility': { focusBack: false, combination: 'PS', de: null }
			},
			{
				'id': 'master-header-and-footer',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:HeaderAndFooter', 'presentation'),
				'command': '.uno:HeaderAndFooter',
				'accessibility': { focusBack: false, combination: 'HF', de: null }
			},
			{
				'id': 'master-rename-master-page',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:RenameMasterPage', 'presentation'),
				'command': '.uno:RenameMasterPage',
				'accessibility': { focusBack: false, combination: 'RP', de: null }
			},
			{
				'id': 'master-close-master-view',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:CloseMasterView', 'presentation'),
				'command': '.uno:CloseMasterView',
				'accessibility': { focusBack: false, combination: 'CV', de: null }
			}
		];

		return this.getTabPage('MasterPage', content);
	},

	getReviewTab: function() {
		// Note: when adding track changes elements, consider this._map['wopi'].HideChangeTrackingControls
		var content = [
			{
				'id': 'review-spell-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:SpellDialog'),
				'command': '.uno:SpellDialog',
				'accessibility': { focusBack: false, combination: 'SD', de: null }
			},
			{
				'id': 'LanguageMenu',
				'type': 'bigcustomtoolitem',
				'text': _UNO('.uno:LanguageMenu'),
				'command': 'languagemenu',
				'accessibility': { focusBack: false, combination: 'TM', de: null }
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
								'accessibility': { focusBack: true, combination: 'SO', de: null },
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
								'id': 'review-hyphenation',
								'type': 'toolitem',
								'text': _UNO('.uno:Hyphenation', 'presentation'),
								'command': '.uno:Hyphenation',
								'accessibility': { focusBack: true, combination: 'HY', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'review-hyphenation-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'review-comments',
				'name':_('Comments'),
				'accessibility': { focusBack: true, combination: 'L', de: null },
				'children' : [
					{
						'id': 'review-insert-annotation',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:InsertAnnotation'),
						'command': '.uno:InsertAnnotation',
						'accessibility': { focusBack: true, combination: 'L', de: null }
					},
					{
						'id': 'review-delete-all-annotations',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:DeleteAllAnnotation', 'presentation'),
						'command': '.uno:DeleteAllAnnotation',
						'accessibility': { focusBack: true, combination: 'RC', de: null }
					}
				]
			},
		];

		return this.getTabPage('Review', content);
	},

	getTableTab: function() {
		var content = [
			{
				'id': 'table-table-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TableDialog', 'presentation', true),
				'command': '.uno:TableDialog',
				'accessibility': { focusBack: false, combination: 'SD', de: null }
			},
			{ type: 'separator', id: 'table-bigtoolitem-break', orientation: 'vertical' },
						{
				'type': 'overflowgroup',
				'id': 'table-insert',
				'name':_('Insert'),
				'accessibility': { focusBack: true, combination: 'CB', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-insert-columns-before',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertColumnsBefore', 'presentation'),
										'command': '.uno:InsertColumnsBefore',
										'accessibility': { focusBack: true, combination: 'CB', de: null }
									},
									{
										'id': 'table-insert-columns-after',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertColumnsAfter', 'presentation'),
										'command': '.uno:InsertColumnsAfter',
										'accessibility': { focusBack: true, combination: 'CA', de: null }
									},
									{
										'id': 'table-delete-columns',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteColumns', 'presentation'),
										'command': '.uno:DeleteColumns',
										'accessibility': { focusBack: true, combination: 'CD', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-insert-rows-before',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertRowsBefore', 'presentation'),
										'command': '.uno:InsertRowsBefore',
										'accessibility': { focusBack: true, combination: 'RB', de: null }
									},
									{
										'id': 'table-insert-rows-after',
										'type': 'toolitem',
										'text': _UNO('.uno:InsertRowsAfter', 'presentation'),
										'command': '.uno:InsertRowsAfter',
										'accessibility': { focusBack: true, combination: 'RA', de: null }
									},
									{
										'id': 'table-delete-rows',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteRows', 'presentation'),
										'command': '.uno:DeleteRows',
										'accessibility': { focusBack: true, combination: 'RD', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'table-deleterows-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'table-merge-split',
				'name':_('Merge & Split'),
				'accessibility': { focusBack: true, combination: 'MC', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-merge-cells',
										'type': 'toolitem',
										'text': _UNO('.uno:MergeCells', 'presentation'),
										'command': '.uno:MergeCells',
										'accessibility': { focusBack: true, combination: 'MC', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-split-cells',
										'type': 'toolitem',
										'text': _UNO('.uno:SplitCell', 'presentation'),
										'command': '.uno:SplitCell',
										'accessibility': { focusBack: true, combination: 'SC', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'table-merge-split-table-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'table-select',
				'name':_('Select'),
				'accessibility': { focusBack: true, combination: 'ST', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-select-table',
										'type': 'toolitem',
										'text': _UNO('.uno:SelectTable', 'presentation'),
										'command': '.uno:SelectTable',
										'accessibility': { focusBack: true, combination: 'ST', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-delete-table',
										'type': 'toolitem',
										'text': _UNO('.uno:DeleteTable', 'presentation'),
										'command': '.uno:DeleteTable',
										'accessibility': { focusBack: true, combination: 'TD', de: null }
									}
								]
							}
						],
						'vertical': 'true',
					},
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-entire-column',
										'type': 'toolitem',
										'text': _UNO('.uno:EntireColumn', 'presentation'),
										'command': '.uno:EntireColumn',
										'accessibility': { focusBack: true, combination: 'CE', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-entire-row',
										'type': 'toolitem',
										'text': _UNO('.uno:EntireRow', 'presentation'),
										'command': '.uno:EntireRow',
										'accessibility': { focusBack: true, combination: 'RE', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'table-select-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'table-paragraph',
				'name':_('Paragraph'),
				'accessibility': { focusBack: true, combination: 'CT', de: null },
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
												'id': 'table-cell-vertical-top',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertTop'),
												'command': '.uno:CellVertTop',
												'accessibility': { focusBack: true, combination: 'CT', de: null }
											},
											{
												'id': 'table-cell-vertical-center',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertCenter'),
												'command': '.uno:CellVertCenter',
												'accessibility': { focusBack: true, combination: 'CC', de: null }
											},
											{
												'id': 'table-cell-vertical-bottom',
												'type': 'toolitem',
												'text': _UNO('.uno:CellVertBottom'),
												'command': '.uno:CellVertBottom',
												'accessibility': { focusBack: true, combination: 'CM', de: null }
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
												'id': 'table-left-para',
												'type': 'toolitem',
												'text': _UNO('.uno:LeftPara'),
												'command': '.uno:LeftPara',
												'accessibility': { focusBack: true, combination: 'PL', de: null }
											},
											{
												'id': 'table-center-para',
												'type': 'toolitem',
												'text': _UNO('.uno:CenterPara'),
												'command': '.uno:CenterPara',
												'accessibility': { focusBack: true, combination: 'PC', de: null }
											},
											{
												'id': 'table-right-para',
												'type': 'toolitem',
												'text': _UNO('.uno:RightPara'),
												'command': '.uno:RightPara',
												'accessibility': { focusBack: true, combination: 'PR', de: null }
											},
											{
												'id': 'table-justify-para',
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
			{ type: 'separator', id: 'table-justifypara-break', orientation: 'vertical' },
			{
				'type': 'container',
				'children': [
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'table-xline-color:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:XLineColor'),
								'command': '.uno:XLineColor',
								'accessibility': { focusBack: true, combination: 'LC', de: null }
							}
						]
					},
					{
						'type': 'toolbox',
						'children': [
							{
								'id': 'table-fill-color:ColorPickerMenu',
								'noLabel': true,
								'type': 'toolitem',
								'text': _UNO('.uno:FillColor'),
								'command': '.uno:FillColor',
								'accessibility': { focusBack: true, combination: 'FC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'table-color-fill-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'table-align',
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
										'id': 'table-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true, combination: 'AL', de: null }
									},
									{
										'id': 'table-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true, combination: 'AC', de: null }
									},
									{
										'id': 'table-object-align-right',
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
										'id': 'table-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true, combination: 'AU', de: null }
									},
									{
										'id': 'table-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true, combination: 'AM', de: null }
									},
									{
										'id': 'table-align-down',
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
			{ type: 'separator', id: 'table-align-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'table-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true, combination: 'BF', de: null },
				'children' : [
					{
						'type': 'container',
						'children': [
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-bring-to-front',
										'type': 'toolitem',
										'text': _UNO('.uno:BringToFront'),
										'command': '.uno:BringToFront',
										'accessibility': { focusBack: true, combination: 'BF', de: null }
									},
									{
										'id': 'table-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true, combination: 'SB', de: null }
									}
								]
							},
							{
								'type': 'toolbox',
								'children': [
									{
										'id': 'table-object-forward-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectForwardOne'),
										'command': '.uno:ObjectForwardOne',
										'accessibility': { focusBack: true, combination: 'FO', de: null }
									},
									{
										'id': 'table-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true, combination: 'BO', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					}
				]
			},

		];

		return this.getTabPage('Table', content);
	},

	getShapeTab: function() {
		var content = [
			{
				'id': 'shape-transform-dialog',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:TransformDialog', 'text'),
				'command': '.uno:TransformDialog',
				'accessibility': { focusBack: false, combination: 'TD', de: null }
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
								'accessibility': { focusBack: true, combination: 'FV', de: null }
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
								'accessibility': { focusBack: true, combination: 'FH', de: null }
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
								'accessibility': { focusBack: true, combination: 'LC', de: null }
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
								'accessibility': { focusBack: true, combination: 'FC', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'shape-fillcolor-break', orientation: 'vertical' },
			{
				'id': 'shape-convert-curve',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:ChangeBezier', 'presentation'),
				'command': '.uno:ChangeBezier',
				'accessibility': { focusBack: true, combination: 'CB', de: null }
			},
			{ type: 'separator', id: 'shape-convertcurve-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'shape-align',
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
										'id': 'shape-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true, combination: 'AL', de: null }
									},
									{
										'id': 'shape-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true, combination: 'AC', de: null }
									},
									{
										'id': 'shape-object-align-right',
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
										'id': 'shape-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true, combination: 'AU', de: null }
									},
									{
										'id': 'shape-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true, combination: 'AM', de: null }
									},
									{
										'id': 'shape-align-down',
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
			{ type: 'separator', id: 'shape-aligndown-break', orientation: 'vertical' },
						{
				'type': 'overflowgroup',
				'id': 'shape-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true, combination: 'BF', de: null },
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
										'accessibility': { focusBack: true, combination: 'BF', de: null }
									},
									{
										'id': 'shape-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true, combination: 'SB', de: null }
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
										'accessibility': { focusBack: true, combination: 'FO', de: null }
									},
									{
										'id': 'shape-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true, combination: 'BO', de: null }
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
				'accessibility': { focusBack: true, combination: 'FG', de: null },
				'children' : [
					{
						'id': 'shape-format-group',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FormatGroup'),
						'command': '.uno:FormatGroup',
						'accessibility': { focusBack: true, combination: 'FG', de: null }
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
										'accessibility': { focusBack: true, combination: 'EG', de: null }
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
										'accessibility': { focusBack: true, combination: 'LG', de: null }
									}
								]
							}
						],
						'vertical': 'true'
					},
				]
			},
			{ type: 'separator', id: 'shape-leavegroup-break', orientation: 'vertical' },
			{
				'id': 'layout-interaction',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:AnimationEffects', 'presentation'),
				'command': '.uno:AnimationEffects',
				'accessibility': { focusBack: true, combination: 'IA', de: null }
			}
		];

		return this.getTabPage('Shape', content);
	},

	getPictureTab: function() {
		var content = [
			{
				'type': 'overflowgroup',
				'id': 'picture-image-adjustment',
				'name':_('Image Controls'),
				'accessibility': { focusBack: true, combination: 'BN', de: null },
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
								'id': 'picture-effects:PictureEffectsMenu',
								'type': 'menubutton',
								'text': _('Effects'),
								'accessibility': { focusBack: true, combination: 'EF', de: null }
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
								'accessibility': { focusBack: true, combination: 'LC', de: null }
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
								'accessibility': { focusBack: true, combination: 'FC', de: null }
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
				'accessibility': { focusBack: false, combination: 'TD', de: null }
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
								'accessibility': { focusBack: true, combination: 'FV', de: null }
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
								'accessibility': { focusBack: true, combination: 'FH', de: null }
							}
						]
					}
				],
				'vertical': 'true'
			},
			{ type: 'separator', id: 'picture-fliphorizontal-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'picture-align',
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
										'id': 'picture-object-align-left',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectAlignLeft'),
										'command': '.uno:ObjectAlignLeft',
										'accessibility': { focusBack: true, combination: 'AL', de: null }
									},
									{
										'id': 'picture-align-center',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignCenter'),
										'command': '.uno:AlignCenter',
										'accessibility': { focusBack: true, combination: 'AC', de: null }
									},
									{
										'id': 'picture-object-align-right',
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
										'id': 'picture-align-up',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignUp'),
										'command': '.uno:AlignUp',
										'accessibility': { focusBack: true, combination: 'AU', de: null }
									},
									{
										'id': 'picture-align-middle',
										'type': 'toolitem',
										'text': _UNO('.uno:AlignMiddle'),
										'command': '.uno:AlignMiddle',
										'accessibility': { focusBack: true, combination: 'AM', de: null }
									},
									{
										'id': 'picture-align-down',
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
			{ type: 'separator', id: 'picture-aligndown-break', orientation: 'vertical' },
			{
				'type': 'overflowgroup',
				'id': 'picture-arrange',
				'name':_('Arrange'),
				'accessibility': { focusBack: true, combination: 'BF', de: null },
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
										'accessibility': { focusBack: true, combination: 'BF', de: null }
									},
									{
										'id': 'picture-send-to-back',
										'type': 'toolitem',
										'text': _UNO('.uno:SendToBack'),
										'command': '.uno:SendToBack',
										'accessibility': { focusBack: true, combination: 'SB', de: null }
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
										'accessibility': { focusBack: true, combination: 'FO', de: null }
									},
									{
										'id': 'picture-object-back-one',
										'type': 'toolitem',
										'text': _UNO('.uno:ObjectBackOne'),
										'command': '.uno:ObjectBackOne',
										'accessibility': { focusBack: true, combination: 'BO', de: null }
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
				'accessibility': { focusBack: true, combination: 'FG', de: null },
				'children' : [
					{
						'id': 'picture-format-group',
						'type': 'bigtoolitem',
						'text': _UNO('.uno:FormatGroup'),
						'command': '.uno:FormatGroup',
						'accessibility': { focusBack: true, combination: 'FG', de: null }
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
										'accessibility': { focusBack: true, combination: 'EG', de: null }
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
										'accessibility': { focusBack: true, combination: 'LG', de: null }
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
			{ type: 'separator', id: 'picture-crop-break', orientation: 'vertical' },
			{
				'id': 'layout-interaction',
				'type': 'bigtoolitem',
				'text': _UNO('.uno:AnimationEffects', 'presentation'),
				'command': '.uno:AnimationEffects',
				'accessibility': { focusBack: true, combination: 'IA', de: null }
			}
		];

		return this.getTabPage('Picture', content);
	},
});

window.L.control.notebookbarImpress = function (options) {
	return new window.L.Control.NotebookbarImpress(options);
};
