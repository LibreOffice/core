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

describe('Notebookbar contributes.notebookbar handling', function () {

	function newNotebookbar(): any {
		return new (window as any).L.Control.Notebookbar();
	}

	// A manifest fixture with two tabs: the first reuses command "cmdX" as both a
	// direct button and a menu entry, and declares two groups sharing the same
	// (deliberately duplicated) group id; the second sets both insertBefore and
	// insertAfter, and names an insertBefore anchor that does not exist.
	function twoTabExtension() {
		return {
			options: {
				baseUrl: '/extensions/ext-a/',
				manifest: {
					contributes: {
						commands: [
							{ id: 'cmdX', title: 'Command X' },
							{ id: 'cmdY', title: 'Command Y' },
						],
						notebookbar: [
							{
								tab: 'Ext A Tab 1',
								insertAfter: 'Home',
								groups: [
									{
										id: 'dup',
										label: 'Group 1',
										items: [
											{ type: 'button', command: 'cmdX' },
											{
												type: 'menu',
												title: 'Menu',
												items: [{ command: 'cmdX' }],
											},
										],
									},
									{
										id: 'dup',
										label: 'Group 2',
										items: [{ type: 'button', command: 'cmdY' }],
									},
								],
							},
							{
								tab: 'Ext A Tab 2',
								insertBefore: 'NoSuchTab',
								insertAfter: 'Home',
								groups: [
									{ id: 'g', label: 'Group', items: [{ type: 'button', command: 'cmdX' }] },
								],
							},
						],
					},
				},
			},
		};
	}

	function collectWarnings(fn: () => void): string[] {
		const warnings: string[] = [];
		const original = console.warn;
		console.warn = function (msg: any) {
			warnings.push(String(msg));
		};
		try {
			fn();
		} finally {
			console.warn = original;
		}
		return warnings;
	}

	beforeEach(function () {
		(window as any).enableExperimentalFeatures = true;
		app.map = { _extensions: {} } as any;
	});

	it('getTabNames() never touches _nextContributedId (no build-and-discard)', function () {
		const notebookbar = newNotebookbar();
		app.map._extensions = { extA: twoTabExtension() };

		const names = notebookbar._getContributedNotebookbarTabNames();

		nodeassert.deepStrictEqual(
			names.map(function (t: any) { return t.name; }),
			['Ext A Tab 1', 'Ext A Tab 2']);
		nodeassert.strictEqual(notebookbar._nextContributedId, 0);
	});

	it('assigns distinct ids to the same command reused as a button and a menu entry, across tabs', function () {
		const notebookbar = newNotebookbar();
		app.map._extensions = { extA: twoTabExtension() };

		const warnings = collectWarnings(function () {
			var tabs = notebookbar._getContributedNotebookbarTabs();
			(notebookbar as any).__tabs = tabs;
		});
		const tabs = (notebookbar as any).__tabs;

		// tab1.items: [group(dup, dup warning above), groupsep, group(dup)]
		const group1 = tabs[0].items[0];
		const button = group1.children[0];
		const menu = group1.children[1];
		const group2 = tabs[0].items[2];
		const tab2Group = tabs[1].items[0];
		const tab2Button = tab2Group.children[0];

		const ids = [
			button.id,
			menu.id,
			menu.menu[0].id,
			group1.id,
			group2.id,
			tab2Button.id,
		];
		nodeassert.strictEqual(
			new Set(ids).size, ids.length,
			'expected all generated ids to be distinct: ' + JSON.stringify(ids));

		// The dispatch-relevant fields stay extId+command regardless of the id churn:
		nodeassert.strictEqual(button.command, 'ext:extA:cmdX');
		nodeassert.strictEqual(menu.menu[0].action, 'ext:extA:cmdX');

		nodeassert.ok(warnings.some(function (w) { return w.indexOf('more than one group with id "dup"') >= 0; }));
	});

	it('warns on a duplicate group id but still builds both groups', function () {
		const notebookbar = newNotebookbar();
		app.map._extensions = { extA: twoTabExtension() };

		let tabs: any;
		const warnings = collectWarnings(function () {
			tabs = notebookbar._getContributedNotebookbarTabs();
		});

		nodeassert.strictEqual(warnings.filter(function (w) {
			return w.indexOf('more than one group with id "dup"') >= 0;
		}).length, 1);
		// group + groupsep + group = 3 entries in tab1's content:
		nodeassert.strictEqual(tabs[0].items.length, 3);
		nodeassert.strictEqual(tabs[0].items[0].type, 'overflowgroup');
		nodeassert.strictEqual(tabs[0].items[1].type, 'separator');
		nodeassert.strictEqual(tabs[0].items[2].type, 'overflowgroup');
	});

	it('drops an unknown command reference and an unknown item type, each with a warning', function () {
		const notebookbar = newNotebookbar();
		app.map._extensions = {
			extB: {
				options: {
					baseUrl: '/extensions/ext-b/',
					manifest: {
						contributes: {
							commands: [{ id: 'known', title: 'Known' }],
							notebookbar: [
								{
									tab: 'Ext B Tab',
									groups: [
										{
											id: 'g',
											label: 'Group',
											items: [
												{ type: 'button', command: 'known' },
												{ type: 'button', command: 'missing' },
												{ type: 'bogus' },
											],
										},
									],
								},
							],
						},
					},
				},
			},
		};

		let tabs: any;
		const warnings = collectWarnings(function () {
			tabs = notebookbar._getContributedNotebookbarTabs();
		});

		nodeassert.ok(warnings.some(function (w) { return w.indexOf('unknown command "missing"') >= 0; }));
		nodeassert.ok(warnings.some(function (w) { return w.indexOf('unknown type "bogus"') >= 0; }));
		// Only the one valid button should have survived into the group's children:
		const group = tabs[0].items[0];
		nodeassert.strictEqual(group.children.length, 1);
		nodeassert.strictEqual(group.children[0].command, 'ext:extB:known');
	});

	describe('_insertContributedNotebookbarTabs', function () {
		function builtInArr() {
			return [{ name: 'File' }, { name: 'Home' }, { name: 'Insert' }];
		}
		function identityBuildEntry(tab: any) {
			return { name: tab.name };
		}

		it('positions insertBefore/insertAfter relative to an existing entry, defaulting to the end', function () {
			const notebookbar = newNotebookbar();
			const arr = builtInArr();
			notebookbar._insertContributedNotebookbarTabs(arr, [
				{ extId: 'e', name: 'A', insertAfter: 'File' },
				{ extId: 'e', name: 'B', insertBefore: 'Insert' },
				{ extId: 'e', name: 'C' },
			], identityBuildEntry);

			nodeassert.deepStrictEqual(
				arr.map(function (t: any) { return t.name; }),
				['File', 'A', 'Home', 'B', 'Insert', 'C']);
		});

		it('warns (only when validate is true) on a name collision, both anchors set, and an unknown anchor', function () {
			const notebookbar = newNotebookbar();
			const tabs = [
				{ extId: 'extA', name: 'Home' },
				{ extId: 'extA', name: 'NewTab', insertBefore: 'NoSuchTab', insertAfter: 'Home' },
			];

			const silentWarnings = collectWarnings(function () {
				notebookbar._insertContributedNotebookbarTabs(builtInArr(), tabs, identityBuildEntry, false);
			});
			nodeassert.deepStrictEqual(silentWarnings, []);

			const warnings = collectWarnings(function () {
				notebookbar._insertContributedNotebookbarTabs(builtInArr(), tabs, identityBuildEntry, true);
			});
			nodeassert.ok(warnings.some(function (w) { return w.indexOf('collides with an existing tab name') >= 0; }));
			nodeassert.ok(warnings.some(function (w) { return w.indexOf('sets both insertBefore and insertAfter') >= 0; }));
			nodeassert.ok(warnings.some(function (w) { return w.indexOf('insertBefore names unknown tab "NoSuchTab"') >= 0; }));
		});
	});
});
