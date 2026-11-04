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

describe('Accessibility statement notice', function () {
	const realSanitizeUrl = window.sanitizeUrl;
	const realStatementUrl = window.accessibilityStatementUrl;

	beforeEach(function () {
		installFakeClock();
		window.mode = { isSmallScreenDevice: () => true } as any;
		window.sanitizeUrl = (url: string) => url;
	});

	afterEach(function () {
		AccessibilityNotice.hide();
		restoreClock();
		window.sanitizeUrl = realSanitizeUrl;
		window.accessibilityStatementUrl = realStatementUrl;
	});

	function showNoticeWithStatement(url: string): void {
		window.accessibilityStatementUrl = url;
		new UIManager().showAccessibilityStatementNotice();
	}

	function find(selector: string): HTMLElement | null {
		return document.querySelector(selector);
	}

	it('says nothing when the deployment has no accessibility statement', function () {
		showNoticeWithStatement('');
		nodeassert.strictEqual(find('.accessibility-notice'), null);
	});

	it('warns about the mobile view and offers the statement', function () {
		showNoticeWithStatement('https://example.com/accessibility');

		nodeassert.strictEqual(
			find('.accessibility-notice-message').textContent,
			'This mobile view is not fully accessible.',
		);
		const link = find('.accessibility-notice-link');
		nodeassert.strictEqual(link.textContent, 'Accessibility statement');
		nodeassert.strictEqual(
			link.getAttribute('href'),
			'https://example.com/accessibility',
		);
		nodeassert.strictEqual(link.getAttribute('target'), '_blank');
	});

	it('speaks the warning a moment later without moving the focus', function () {
		showNoticeWithStatement('https://example.com/accessibility');

		const announcement = find('.accessibility-notice-announcement');
		nodeassert.strictEqual(announcement.getAttribute('role'), 'alert');
		nodeassert.strictEqual(announcement.textContent, '');

		advanceClock(3000);
		nodeassert.strictEqual(
			announcement.textContent,
			'This mobile view is not fully accessible.',
		);
		nodeassert.ok(
			!find('.accessibility-notice').contains(document.activeElement),
		);
	});

	it('dismissing the notice takes it off the screen', function () {
		showNoticeWithStatement('https://example.com/accessibility');

		find('.accessibility-notice-dismiss').click();
		nodeassert.strictEqual(find('.accessibility-notice'), null);
	});
});
