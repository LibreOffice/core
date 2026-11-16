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

	const svgLogo = 'data:image/svg+xml;base64,PHN2Zy8+';
	const darkLogo = 'data:image/png;base64,iVBORw0KGgo=';
	let storage: MemoryStorage;
	let originalStorage: PropertyDescriptor;

	function cacheList(providers: object[]): void {
		storage.setItem(
			'coda.cloudProviderList',
			JSON.stringify({
				kind: 'cloudproviders',
				providers: providers,
			}),
		);
	}

	function provider(kind: string, extra?: object): object {
		return Object.assign(
			{ kind: kind, label: kind + ' label', logo: svgLogo },
			extra,
		);
	}

	before(function () {
		originalStorage = Object.getOwnPropertyDescriptor(globals, 'localStorage');
	});

	beforeEach(function () {
		delete globals.brandCloudProviders;
		storage = new MemoryStorage();
		Object.defineProperty(globals, 'localStorage', {
			value: storage,
			configurable: true,
			writable: true,
		});
	});

	after(function () {
		delete globals.brandCloudProviders;
		if (originalStorage)
			Object.defineProperty(globals, 'localStorage', originalStorage);
		else delete globals.localStorage;
	});

	it('offers only Other when no list was downloaded', function () {
		nodeassert.deepStrictEqual(kinds(), ['other']);
		nodeassert.strictEqual(BackstageCloudKinds.defaultKind(), 'other');
		nodeassert.strictEqual(BackstageCloudKinds.isBranded(), false);
	});

	it('lists the downloaded providers, then Other', function () {
		cacheList([provider('nextcloud'), provider('seafile')]);
		nodeassert.deepStrictEqual(kinds(), ['nextcloud', 'seafile', 'other']);
		nodeassert.strictEqual(
			BackstageCloudKinds.label('seafile'),
			'seafile label',
		);

		const icon = BackstageCloudKinds.icon('seafile', 'x');
		nodeassert.strictEqual(icon.className, 'x backstage-cloud-icon has-logo');
		const logos = images(icon);
		nodeassert.strictEqual(logos.length, 1);
		nodeassert.strictEqual(logos[0].getAttribute('src'), svgLogo);
		nodeassert.strictEqual(logos[0].getAttribute('alt'), '');
	});

	it('draws a second logo for the dark theme', function () {
		cacheList([provider('owncloud', { logoDark: darkLogo })]);
		const logos = images(BackstageCloudKinds.icon('owncloud', 'x'));
		nodeassert.strictEqual(logos.length, 2);
		nodeassert.strictEqual(logos[1].getAttribute('src'), darkLogo);
		nodeassert.ok(logos[1].classList.contains('backstage-cloud-logo-dark'));
	});

	it('does not download a list when a branding declares one', async function () {
		globals.brandCloudProviders = [{ kind: 'acme' }];
		const originalFetch = globals.fetch;
		let calls = 0;
		globals.fetch = () => {
			calls++;
			return Promise.reject(new Error('unexpected'));
		};
		try {
			nodeassert.strictEqual(await BackstageCloudKinds.refreshList(), false);
		} finally {
			globals.fetch = originalFetch;
		}
		nodeassert.strictEqual(calls, 0);
	});

	it('draws a provider that is no longer listed with the generic icon', function () {
		const icon = BackstageCloudKinds.icon('gone', 'x');
		nodeassert.strictEqual(icon.className, 'x backstage-cloud-icon');
		nodeassert.strictEqual(images(icon).length, 0);
	});

	it('offers the branding list instead of the downloaded one', function () {
		globals.brandCloudProviders = [{ kind: 'acme' }];
		nodeassert.deepStrictEqual(kinds(), ['acme', 'other']);
		nodeassert.strictEqual(BackstageCloudKinds.isBranded(), true);

		const icon = BackstageCloudKinds.icon('acme', 'x');
		nodeassert.ok(icon.classList.contains('backstage-cloud-icon-acme'));
		nodeassert.ok(icon.classList.contains('backstage-cloud-brand-icon'));
		nodeassert.strictEqual(images(icon).length, 0);
	});
});
