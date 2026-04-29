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

	// 'search' is registered as an EXPLICIT_ALIAS in Util.Shortcuts.ts, so
	// it always maps to Ctrl+F regardless of whether the generated
	// unoShortcutsMap is loaded.

	describe('getTooltipLabel()', function () {

		it('appends the shortcut in parentheses', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('Find', 'search'),
				'Find (Ctrl+F)');
		});

		it('strips the mnemonic tilde from the label', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('~Find', 'search'),
				'Find (Ctrl+F)');
		});

		it('returns the text unchanged for unknown commands', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getTooltipLabel('Find', '.uno:NoSuch'),
				'Find');
		});
	});

	describe('getMenuLabel()', function () {

		it('wraps the shortcut in span.shortcut for right-aligned rendering', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('Find', 'search'),
				'Find <span class="shortcut">Ctrl+F</span>');
		});

		it('strips the mnemonic tilde from the label', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('~Find', 'search'),
				'Find <span class="shortcut">Ctrl+F</span>');
		});

		it('returns the text unchanged for unknown commands', function () {
			nodeassert.strictEqual(
				JSDialog.ShortcutsUtil.getMenuLabel('Find', '.uno:NoSuch'),
				'Find');
		});
	});
});
