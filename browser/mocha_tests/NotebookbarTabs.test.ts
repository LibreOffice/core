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

describe('Notebookbar tabs', function () {
	let saved: any;
	let map: any;

	beforeEach(function () {
		saved = {
			map: app.map,
			windowMap: window.app.map,
			isSlideImportSupported: app.impress.isSlideImportSupported,
			params: window.L.Params,
			showAINotebookbar: window.showAINotebookbar,
			tableStyles: app.tableStyles,
			makeHttpUrl: (window as any).makeHttpUrl,
			isAIAssistantAvailable: app.LOUtil.isAIAssistantAvailable,
		};
		window.L.Params = {};
		(window as any).makeHttpUrl = (path: string) => 'http://localhost' + path;
		app.impress.isSlideImportSupported = () => false;
		map = {
			wopi: { _supportsRename: () => true },
			isAIConfigured: false,
			isReadOnlyMode: () => false,
			on: () => undefined,
			stateChangeHandler: { getItemValue: (): string => undefined },
			getDocType: () => 'text',
			// The debug tools, switched off. The layouting service of the page can
			// still run its frame hooks while a test runs.
			_debug: {
				debugOn: false,
				enterRAF: () => undefined,
				exitRAF: () => undefined,
			},
			zotero: null,
			slideLinks: { hasLinks: () => false },
			uiManager: {
				getCurrentMode: () => 'notebookbar',
				isCommandVisible: () => true,
				isButtonVisible: () => true,
				isTabVisible: () => true,
				shouldSelectAIAssistantTab:
					UIManager.prototype.shouldSelectAIAssistantTab,
			},
		};
		map.uiManager.map = map;
		app.map = map;
		window.app.map = map;
		app.tableStyles = new TableStylesService();
	});

	afterEach(function () {
		app.map = saved.map;
		window.app.map = saved.windowMap;
		app.impress.isSlideImportSupported = saved.isSlideImportSupported;
		window.L.Params = saved.params;
		window.showAINotebookbar = saved.showAINotebookbar;
		app.tableStyles = saved.tableStyles;
		(window as any).makeHttpUrl = saved.makeHttpUrl;
		app.LOUtil.isAIAssistantAvailable = saved.isAIAssistantAvailable;
	});

	// The notebookbar of the given kind, for a document of the given type and file name.
	function notebookbar(kind: string, docType: string, fileName: string): any {
		map.getDocType = () => docType;
		map.wopi.BaseFileName = fileName;
		const nb = new (window.L.Control as any)[kind]();
		nb.map = map;
		nb._map = map;
		return nb;
	}

	function writer(): any {
		return notebookbar('NotebookbarWriter', 'text', 'help_dialog.odt');
	}

	function impress(): any {
		return notebookbar('NotebookbarImpress', 'presentation', 'help_dialog.odp');
	}

	// Every node of a notebookbar JSON tree.
	function allNodes(tree: any): any[] {
		const found: any[] = [];
		(function walk(node: any) {
			if (Array.isArray(node)) {
				node.forEach(walk);
				return;
			}
			if (!node || typeof node !== 'object') return;
			found.push(node);
			if (node.children) walk(node.children);
		})(tree);
		return found;
	}

	it('Impress has a Transition tab that offers Apply to All Slides', function () {
		const nb = impress();

		const tab = nb
			.getTabs()
			.find((entry: any) => entry && entry.id === 'Transition-tab-label');
		nodeassert.ok(tab);
		nodeassert.strictEqual(tab.text, 'Transition');

		const page = allNodes(nb.getFullJSON()).find(
			(node) => node.type === 'tabpage' && node.name === 'Transition',
		);
		nodeassert.ok(page);

		// The tab page is built as the notebookbar builds it.
		const dialogMap = setUpDialogPage();
		const container = document.createElement('div');
		document.body.appendChild(container);
		try {
			const builder = window.L.control.notebookbarBuilder({
				windowId: WindowId.Notebookbar,
				mobileWizard: {},
				map: dialogMap,
				cssClass: 'notebookbar',
				suffix: 'notebookbar',
				callback: () => {},
			});
			builder.build(container, page.children);
			drainLayout();
			nodeassert.ok(container.querySelector('#apply_to_all button'));
		} finally {
			container.remove();
			closeAllDialogs();
		}
	});

	it('the notebookbar opens on the AI Assistant tab only once AI is configured', function () {
		// The tab is offered to an unconfigured user as the way in to the setup dialog, so
		// it is offered either way here and only the choice of the first tab is checked.
		app.LOUtil.isAIAssistantAvailable = () => true;
		window.showAINotebookbar = true;
		const nb = writer();

		map.isAIConfigured = false;
		nodeassert.strictEqual(nb.getInitialTabId(), 'Home-tab-label');

		map.isAIConfigured = true;
		nodeassert.strictEqual(nb.getInitialTabId(), 'AIAssistant-tab-label');

		window.showAINotebookbar = false;
		nodeassert.strictEqual(nb.getInitialTabId(), 'Home-tab-label');
	});
});
