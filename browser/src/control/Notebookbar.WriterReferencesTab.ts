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
 * Notebookbar.WriterReferencesTab.ts - it contains dynamically added bibliography
 *                                      management entries which can work with Zotero
 */

class WriterReferencesTab implements NotebookbarTab {
	public getName(): string {
		return 'References';
	}

	public getEntry(): NotebookbarTabEntry {
		return {
			id: this.getName() + '-tab-label',
			text: _('References'),
			name: this.getName(),
			accessibility: {
				focusBack: true,
				combination: 'S',
				de: 'C',
			} as NotebookbarAccessibilityDescriptor,
		} as NotebookbarTabEntry;
	}

	/* ids have to match transition pane ids from the .ui in the core */
	public getContent(): NotebookbarTabContent {
		let content = [
			{
				type: 'overflowgroup',
				id: 'Reference-TOC',
				name: _('Table of content'),
				accessibility: { focusBack: false, combination: 'T', de: 'LA' },
				children: [
					{
						id: 'references-insert-multi-index',
						type: 'bigtoolitem',
						text: _UNO('.uno:IndexesMenu', 'text'),
						command: '.uno:InsertMultiIndex',
						accessibility: { focusBack: false, combination: 'T', de: 'LA' },
					} as ToolItemWidgetJSON,
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-insert-indexes-entry',
										type: 'toolitem',
										text: _UNO('.uno:InsertIndexesEntry', 'text'),
										command: '.uno:InsertIndexesEntry',
										accessibility: { focusBack: false, combination: 'IE' },
									} as ToolItemWidgetJSON,
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-update-current-index',
										type: 'toolitem',
										text: _('Update Index'),
										command: '.uno:UpdateCurIndex',
										accessibility: {
											focusBack: false,
											combination: 'UI',
											de: 'T',
										},
									} as ToolItemWidgetJSON,
								],
							},
						],
						vertical: 'true',
					},
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'references-updatecurindex-break',
				orientation: 'vertical',
			},
			{
				type: 'overflowgroup',
				id: 'reference-footnote',
				name: _('Footnote/Endnote'),
				accessibility: { focusBack: false, combination: 'F', de: 'U' },
				children: [
					{
						id: 'references-insert-foot-note',
						type: 'bigtoolitem',
						text: _UNO('.uno:InsertFootnote', 'text'),
						command: '.uno:InsertFootnote',
						accessibility: { focusBack: true, combination: 'F', de: 'U' },
					} as ToolItemWidgetJSON,
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-insert-end-note',
										type: 'toolitem',
										text: _UNO('.uno:InsertEndnote', 'text'),
										command: '.uno:InsertEndnote',
										accessibility: {
											focusBack: true,
											combination: 'E',
											de: 'E',
										},
									} as ToolItemWidgetJSON,
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-foot-note-dialog',
										type: 'toolitem',
										text: _UNO('.uno:FootnoteDialog', 'text'),
										command: '.uno:FootnoteDialog',
										accessibility: {
											focusBack: false,
											combination: 'H',
											de: 'I',
										},
									} as ToolItemWidgetJSON,
								],
							},
						],
						vertical: 'true',
					},
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'references-footnotedialog-break',
				orientation: 'vertical',
			},
			{
				type: 'container',
				children: [
					{
						type: 'toolbox',
						children: [
							{
								id: 'references-insert-bookmark',
								type: 'toolitem',
								text: _UNO('.uno:InsertBookmark', 'text'),
								command: '.uno:InsertBookmark',
								accessibility: {
									focusBack: false,
									combination: 'IB',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
					{
						type: 'toolbox',
						children: [
							{
								id: 'references-insert-reference-field',
								type: 'toolitem',
								text: _UNO('.uno:InsertReferenceField', 'text'),
								command: '.uno:InsertReferenceField',
								accessibility: {
									focusBack: false,
									combination: 'IR',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
				],
				vertical: 'true',
			},
			{
				type: 'separator',
				id: 'references-insertreferencefield-break',
				orientation: 'vertical',
			},
		] as WidgetJSON[];

		// zotero plugin buttons are visible when it is initialized in ServerConnectionService
		// as initial visibility value we set existence of zotero plugin, later we can show/hide
		// using notebookbar show/hide item functionality
		content = content.concat([
			{
				id: 'zoteroaddeditbibliography',
				class: 'unozoteroaddeditbibliography',
				type: 'bigcustomtoolitem',
				text: _('Add Bibliography'),
				command: 'zoteroaddeditbibliography',
				visible: !!app.map.zotero,
				accessibility: { focusBack: true, combination: 'AB', de: null },
			} as ToolItemWidgetJSON,
			{
				id: 'zoterocontaineradd',
				visible: !!app.map.zotero,
				type: 'container',
				children: [
					{
						type: 'toolbox',
						children: [
							{
								id: 'zoteroAddEditCitation',
								class: 'unozoteroAddEditCitation',
								type: 'customtoolitem',
								text: _('Add Citation'),
								command: 'zoteroaddeditcitation',
								accessibility: {
									focusBack: true,
									combination: 'AC',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
					{
						type: 'toolbox',
						children: [
							{
								id: 'zoteroaddnote',
								class: 'unozoteroaddnote',
								type: 'customtoolitem',
								text: _('Add Citation Note'),
								command: 'zoteroaddnote',
								accessibility: {
									focusBack: true,
									combination: 'CN',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
				],
				vertical: 'true',
			},
			{
				id: 'zoterocontainerrefresh',
				visible: !!app.map.zotero,
				type: 'container',
				children: [
					{
						type: 'toolbox',
						children: [
							{
								id: 'zoterorefresh',
								class: 'unozoterorefresh',
								type: 'customtoolitem',
								text: _('Refresh Citations'),
								command: 'zoterorefresh',
								accessibility: {
									focusBack: true,
									combination: 'RC',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
					{
						type: 'toolbox',
						children: [
							{
								id: 'zoterounlink',
								class: 'unozoterounlink',
								type: 'customtoolitem',
								text: _('Unlink Citations'),
								command: 'zoterounlink',
								accessibility: {
									focusBack: true,
									combination: 'UC',
									de: null,
								},
							} as ToolItemWidgetJSON,
						],
					},
				],
				vertical: 'true',
			},
			{
				id: 'zoteroSetDocPrefs',
				class: 'unozoteroSetDocPrefs',
				type: 'bigcustomtoolitem',
				text: _('Citation Preferences'),
				command: 'zoterosetdocprefs',
				visible: !!app.map.zotero,
				accessibility: { focusBack: true, combination: 'CP', de: null },
			} as ToolItemWidgetJSON,
			{
				type: 'separator',
				id: 'references-zoterosetdocprefs-break',
				orientation: 'vertical',
				visible: !!app.map.zotero,
			},
		] as WidgetJSON[]);

		content = content.concat([
			{
				type: 'overflowgroup',
				id: 'reference-field',
				name: _('Field'),
				accessibility: { focusBack: false, combination: 'IF', de: null },
				children: [
					{
						id: 'references-insert-field-control',
						type: 'bigtoolitem',
						text: _UNO('.uno:InsertFieldCtrl', 'text'),
						command: '.uno:InsertFieldCtrl',
						accessibility: { focusBack: false, combination: 'IF', de: null },
					} as ToolItemWidgetJSON,
					{
						type: 'container',
						children: [
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-inset-page-number-field',
										type: 'toolitem',
										text: _UNO('.uno:InsertPageNumberField'),
										command: '.uno:InsertPageNumberField',
										accessibility: {
											focusBack: true,
											combination: 'PN',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										id: 'references-insert-page-count-field',
										type: 'toolitem',
										text: _UNO('.uno:InsertPageCountField', 'text'),
										command: '.uno:InsertPageCountField',
										accessibility: {
											focusBack: true,
											combination: 'PC',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							},
							{
								type: 'toolbox',
								children: [
									{
										id: 'references-insert-date-field',
										type: 'toolitem',
										text: _UNO('.uno:InsertDateField', 'text'),
										command: '.uno:InsertDateField',
										accessibility: {
											focusBack: true,
											combination: 'ID',
											de: null,
										},
									} as ToolItemWidgetJSON,
									{
										id: 'references-insert-title-field',
										type: 'toolitem',
										text: _UNO('.uno:InsertTitleField', 'text'),
										command: '.uno:InsertTitleField',
										accessibility: {
											focusBack: true,
											combination: 'IT',
											de: null,
										},
									} as ToolItemWidgetJSON,
								],
							},
						],
						vertical: 'true',
					},
				],
			} as OverflowGroupWidgetJSON,
			{
				type: 'separator',
				id: 'references-inserttitlefield-break',
				orientation: 'vertical',
			},
			{
				id: 'references-update-all',
				type: 'bigtoolitem',
				text: _UNO('.uno:UpdateAll', 'text'),
				command: '.uno:UpdateAll',
				accessibility: { focusBack: true, combination: 'UA' },
			} as ToolItemWidgetJSON,
		] as WidgetJSON[]);

		return content as NotebookbarTabContent;
	}
}

JSDialog.WriterReferencesTab = new WriterReferencesTab();
