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

describe('UIManager', function () {
	describe('the theme handed to the engine', function () {
		const realPrefs = window.prefs;
		const realMode = window.mode;
		const realSocket = app.socket;
		const realSectionContainer = app.sectionContainer;
		let sent: string[];
		// Applying a theme marks it on the page.
		const themeAttributes = ['data-theme', 'data-bg-theme'];
		let savedAttributes: string[];

		beforeEach(function () {
			savedAttributes = themeAttributes.map((name) =>
				document.documentElement.getAttribute(name),
			);
			sent = [];
			window.mode = { isCODesktop: () => false } as any;
			app.socket = {
				sendMessage: (message: string) => sent.push(message),
			} as any;
			app.sectionContainer = null;
		});

		afterEach(function () {
			window.prefs = realPrefs;
			window.mode = realMode;
			app.socket = realSocket;
			app.sectionContainer = realSectionContainer;
			themeAttributes.forEach((name, index) => {
				if (savedAttributes[index] === null)
					document.documentElement.removeAttribute(name);
				else
					document.documentElement.setAttribute(name, savedAttributes[index]);
			});
		});

		// A manager on a loaded document whose settings resolve to the given theme.
		function managerSettledOn(dark: boolean): UIManager {
			window.prefs = {
				seedDarkModeDefault: () => dark,
				getBoolean: (name: string, fallback?: boolean) =>
					name === 'darkTheme' ? dark : !!fallback,
				_getUIDefault: (): string => undefined,
				_userBrowserSetting: {},
				_localStorageCache: {},
			} as any;
			const map = {
				_docLoaded: true,
				fire() {},
				on() {},
				off() {},
				getDocType: () => 'spreadsheet',
			};
			const manager: any = new UIManager();
			manager.map = map;
			manager._map = map;
			return manager;
		}

		function themeCommands(): string[] {
			return sent.filter((message) => message.includes('.uno:ChangeTheme'));
		}

		it('a theme the engine does not hold is handed to it, once', function () {
			const manager = managerSettledOn(true);
			manager.rememberThemeSentWithLoad(false, false);

			manager.initDarkModeFromSettings();
			manager.initDarkModeFromSettings();

			nodeassert.strictEqual(themeCommands().length, 1);
			nodeassert.ok(themeCommands()[0].includes('"value":"Dark"'));
		});

		it('a light theme is handed to an engine that holds the dark one, once', function () {
			const manager = managerSettledOn(false);
			manager.rememberThemeSentWithLoad(true, false);

			manager.initDarkModeFromSettings();
			manager.initDarkModeFromSettings();

			nodeassert.strictEqual(themeCommands().length, 1);
			nodeassert.ok(themeCommands()[0].includes('"value":"Light"'));
		});

		it('a theme the engine already holds is not handed again', function () {
			const manager = managerSettledOn(false);
			manager.rememberThemeSentWithLoad(false, false);

			manager.initDarkModeFromSettings();

			nodeassert.deepStrictEqual(themeCommands(), []);
		});
	});

	describe('a redline tooltip', function () {
		const realActiveDocument = app.activeDocument;
		const realDomUtil = window.L.DomUtil;
		const realSectionContainer = app.sectionContainer;
		const sections: any = app.CSections;
		const realJQuery = (globalThis as any).$;
		// The tooltip is a jQuery UI widget, and jQuery UI is not loaded here. The calls the
		// widget would get are kept instead.
		let tooltipCalls: any[][];

		beforeEach(function () {
			tooltipCalls = [];
			(globalThis as any).$ = () => ({
				tooltip: (...args: any[]) => tooltipCalls.push(args),
			});
			window.L.DomUtil = {
				create: (tag: string) => document.createElement(tag),
			} as any;
			app.activeDocument = { activeLayout: null } as any;
			// A tooltip that closes looks for the anchor sections of the side by side view.
			sections.TooltipAnchorLeft = { name: 'tooltip anchor left' };
			sections.TooltipAnchorRight = { name: 'tooltip anchor right' };
			app.sectionContainer = { doesSectionExist: () => false } as any;
		});

		afterEach(function () {
			// A pointer move closes an open tooltip, which takes its listeners off the document.
			moveThePointer();
			(globalThis as any).$ = realJQuery;
			delete sections.TooltipAnchorLeft;
			delete sections.TooltipAnchorRight;
			app.sectionContainer = realSectionContainer;
			window.L.DomUtil = realDomUtil;
			app.activeDocument = realActiveDocument;
		});

		function hoverRedlineWhileTrackedChangesShow(shown: string): void {
			const manager: any = new UIManager();
			manager.map = {
				stateChangeHandler: {
					getItemValue: (command: string) =>
						command === '.uno:ShowTrackedChanges' ? shown : undefined,
				},
			};
			manager.showDocumentTooltip({
				type: 'generaltooltip',
				text: 'Inserted: LocalUser#0 - 02/11/2026 11:44:56',
				rectangle: '5785, 2293, 1240, 275',
				redlineType: 'Insert',
			});
		}

		function opened(): boolean {
			return tooltipCalls.some((args) => args[0] === 'open');
		}

		function moveThePointer(): void {
			document.dispatchEvent(new window.MouseEvent('mousemove'));
		}

		it('is shown when tracked changes are visible', function () {
			hoverRedlineWhileTrackedChangesShow('true');
			nodeassert.ok(opened());
		});

		it('closes once when the pointer moves', function () {
			hoverRedlineWhileTrackedChangesShow('true');
			moveThePointer();
			moveThePointer();
			const closes = tooltipCalls.filter((args) => args[0] === 'close');
			nodeassert.strictEqual(closes.length, 1);
		});

		it('is suppressed when tracked changes are hidden', function () {
			hoverRedlineWhileTrackedChangesShow('false');
			nodeassert.deepStrictEqual(tooltipCalls, []);
		});
	});
});
