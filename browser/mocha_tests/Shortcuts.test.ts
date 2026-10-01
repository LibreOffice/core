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

describe('ShortcutsUtil', function () {
	// 'search' is a UNO_ALIAS for vnd.sun.star.findbar:FocusToFindbar,
	// resolved via the generated unoShortcutsMap (loaded by sources.ts
	// from browser/src/unoshortcuts.js) — Ctrl+F by default.

	describe('getTooltipLabel()', function () {
		it('appends the shortcut in parentheses', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('Find', 'search'),
				'Find (Ctrl+F)',
			);
		});

		it('strips the mnemonic tilde from the label', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('~Find', 'search'),
				'Find (Ctrl+F)',
			);
		});

		it('returns the text unchanged for unknown commands', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('Find', '.uno:NoSuch'),
				'Find',
			);
		});
	});

	describe('getMenuLabel()', function () {
		it('wraps the shortcut in span.shortcut for right-aligned rendering', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('Find', 'search'),
				'Find <span class="shortcut">Ctrl+F</span>',
			);
		});

		it('strips the mnemonic tilde from the label', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('~Find', 'search'),
				'Find <span class="shortcut">Ctrl+F</span>',
			);
		});

		it('returns the text unchanged for unknown commands', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('Find', '.uno:NoSuch'),
				'Find',
			);
		});
	});
});

describe('KeyboardShortcuts', function () {
	describe('processEvent()', function () {
		it('reads a region-tagged language from its base language table', function () {
			const dispatched: string[] = [];
			(window as any).mode = { isChromebook: () => false };
			app.dispatcher = {
				dispatch: (action: string) => dispatched.push(action),
			};
			(window as any).KeyboardShortcuts.initialize({
				_docLayer: { _docType: 'text' },
				isEditMode: () => true,
				on: () => {},
			});

			// Ctrl+Alt+S opens the search bar in a German UI.
			const event = {
				type: 'keydown',
				ctrlKey: true,
				altKey: true,
				shiftKey: false,
				metaKey: false,
				which: 83,
				key: 's',
				preventDefault: () => {},
			} as unknown as KeyboardEvent;

			nodeassert.strictEqual(
				(window as any).KeyboardShortcuts.processEvent('de-DE', event),
				true,
			);
			nodeassert.deepStrictEqual(dispatched, ['home-search']);
		});
	});
});
