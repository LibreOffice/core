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

describe('BackstageCloudKinds', function () {
	const globals = globalThis as any;

	function kinds(): string[] {
		return BackstageCloudKinds.list().map((entry) => entry.kind);
	}

	function images(element: HTMLElement): HTMLImageElement[] {
		return Array.from(element.querySelectorAll('img'));
	}

	beforeEach(function () {
		delete globals.brandCloudProviders;
	});

	after(function () {
		delete globals.brandCloudProviders;
	});

	it('lists the shipped providers, then Other', function () {
		nodeassert.deepStrictEqual(kinds(), [
			'nextcloud',
			'opencloud',
			'seafile',
			'other',
		]);
		nodeassert.strictEqual(BackstageCloudKinds.isBranded(), false);
	});

	it('draws a provider logo as an image without alternative text', function () {
		const icon = BackstageCloudKinds.icon('seafile', 'x');
		nodeassert.strictEqual(icon.className, 'x backstage-cloud-icon has-logo');
		const logos = images(icon);
		nodeassert.strictEqual(logos.length, 1);
		nodeassert.strictEqual(
			logos[0].getAttribute('src'),
			'images/cloud-providers/seafile.svg',
		);
		nodeassert.strictEqual(logos[0].getAttribute('alt'), '');
	});

	it('draws a provider that is no longer listed with the generic icon', function () {
		const icon = BackstageCloudKinds.icon('gone', 'x');
		nodeassert.strictEqual(icon.className, 'x backstage-cloud-icon');
		nodeassert.strictEqual(images(icon).length, 0);
	});

	it('offers the branding list instead of the shipped one', function () {
		globals.brandCloudProviders = [{ kind: 'acme' }];
		nodeassert.deepStrictEqual(kinds(), ['acme', 'other']);
		nodeassert.strictEqual(BackstageCloudKinds.isBranded(), true);

		const icon = BackstageCloudKinds.icon('acme', 'x');
		nodeassert.ok(icon.classList.contains('backstage-cloud-icon-acme'));
		nodeassert.ok(icon.classList.contains('backstage-cloud-brand-icon'));
		nodeassert.strictEqual(images(icon).length, 0);
	});
});
