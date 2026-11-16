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

describe('CloudProviderList', function () {
	const svgLogo = 'data:image/svg+xml;base64,PHN2Zy8+';

	function provider(kind: string, extra?: object): any {
		return Object.assign(
			{
				kind: kind,
				label: kind + ' label',
				logo: svgLogo,
				hasSyncClient: true,
			},
			extra,
		);
	}

	function remoteFile(providers: any, extra?: object): any {
		return Object.assign(
			{
				kind: 'cloudproviders',
				providers: providers,
			},
			extra,
		);
	}

	function kindsOf(data: CloudProviderList.Data | null): string[] {
		return data ? data.providers.map((entry) => entry.kind) : [];
	}

	it('accepts a valid remote file and keeps its provider unchanged', function () {
		const owncloud = {
			kind: 'owncloud',
			label: 'ownCloud',
			logo: svgLogo,
			hasSyncClient: true,
		};
		const data = CloudProviderList.validate(remoteFile([owncloud]));
		nodeassert.deepStrictEqual(data, { providers: [owncloud] });
	});

	it('refuses a file of another kind', function () {
		const file = remoteFile([provider('owncloud')], { kind: 'configuration' });
		nodeassert.strictEqual(CloudProviderList.validate(file), null);
	});

	it('ignores unknown top-level fields', function () {
		for (const extra of [{ extra: 'x' }, { version: 9 }]) {
			const extended = remoteFile([provider('owncloud')], extra);
			nodeassert.deepStrictEqual(
				kindsOf(CloudProviderList.validate(extended)),
				['owncloud'],
			);
		}
	});

	it('drops bad providers and keeps the good one', function () {
		const file = remoteFile([
			provider('Bad'),
			provider('a'.repeat(33)),
			provider('other'),
			provider('remotelogo', { logo: 'https://example.com/x.svg' }),
			provider('owncloud'),
		]);
		nodeassert.deepStrictEqual(kindsOf(CloudProviderList.validate(file)), [
			'owncloud',
		]);
	});

	it('accepts a kind of 32 characters', function () {
		const kind = 'a'.repeat(32);
		const file = remoteFile([provider(kind)]);
		nodeassert.deepStrictEqual(kindsOf(CloudProviderList.validate(file)), [
			kind,
		]);
	});

	it('refuses a file with no valid provider', function () {
		const file = remoteFile([provider('Bad'), provider('other')]);
		nodeassert.strictEqual(CloudProviderList.validate(file), null);
	});

	it('refuses a malformed file without throwing', function () {
		const refused = [
			remoteFile(null),
			remoteFile('x'),
			remoteFile([{ kind: 'owncloud', logo: svgLogo }]),
			remoteFile([42]),
			null,
			'text',
			[],
		];
		refused.forEach((value) => {
			nodeassert.strictEqual(CloudProviderList.validate(value), null);
		});

		const mixed = remoteFile([
			42,
			{ kind: 'nolabel', logo: svgLogo },
			provider('owncloud'),
		]);
		nodeassert.deepStrictEqual(kindsOf(CloudProviderList.validate(mixed)), [
			'owncloud',
		]);
	});

	it('takes only inline image data as a logo', function () {
		const file = remoteFile([
			provider('imagepath', { logo: 'images/cloud-providers/owncloud.svg' }),
			provider('pngdata', { logo: 'data:image/png;base64,AA==' }),
			provider('outside', { logo: '../x.svg' }),
		]);
		nodeassert.deepStrictEqual(kindsOf(CloudProviderList.validate(file)), [
			'pngdata',
		]);
	});

	it('drops only a bad dark logo and keeps its provider', function () {
		const file = remoteFile([
			provider('owncloud', { logoDark: 'https://example.com/x.svg' }),
		]);
		const data = CloudProviderList.validate(file);
		nodeassert.deepStrictEqual(kindsOf(data), ['owncloud']);
		nodeassert.strictEqual(data.providers[0].logo, svgLogo);
		nodeassert.strictEqual('logoDark' in data.providers[0], false);
	});

	it('reads a provider with no sync client flag as one with no sync client', function () {
		const file = remoteFile([
			provider('missing', { hasSyncClient: undefined }),
			provider('text', { hasSyncClient: 'yes' }),
		]);
		const data = CloudProviderList.validate(file);
		nodeassert.deepStrictEqual(
			data.providers.map((entry) => entry.hasSyncClient),
			[false, false],
		);
	});

	it('keeps a data URL logo with quotes and parentheses as it is', function () {
		const logo = 'data:image/svg+xml,<svg a="1">)</svg>';
		const file = remoteFile([provider('owncloud', { logo: logo })]);
		const data = CloudProviderList.validate(file);
		nodeassert.strictEqual(data.providers[0].logo, logo);
	});

	it('refuses a remote text over 512 KB', function () {
		const text = JSON.stringify(remoteFile([provider('owncloud')]));
		const padded = text + ' '.repeat(524289 - text.length);
		nodeassert.strictEqual(padded.length, 524289);
		nodeassert.strictEqual(CloudProviderList.parseRemote(padded), null);
		nodeassert.deepStrictEqual(kindsOf(CloudProviderList.parseRemote(text)), [
			'owncloud',
		]);
	});

	it('refuses a remote text that is not JSON and says why', function () {
		const reasons: string[] = [];
		nodeassert.strictEqual(
			CloudProviderList.parseRemote('not json', (reason) =>
				reasons.push(reason),
			),
			null,
		);
		nodeassert.strictEqual(reasons.length, 1);
	});
});

// A Storage that keeps its items in memory.
class MemoryStorage {
	items: Map<string, string> = new Map();

	get length(): number {
		return this.items.size;
	}

	key(index: number): string | null {
		const keys = Array.from(this.items.keys());
		return index < keys.length ? keys[index] : null;
	}

	getItem(key: string): string | null {
		return this.items.has(key) ? this.items.get(key) : null;
	}

	setItem(key: string, value: string): void {
		this.items.set(key, String(value));
	}

	removeItem(key: string): void {
		this.items.delete(key);
	}

	clear(): void {
		this.items.clear();
	}
}

describe('CloudProviderList refresh', function () {
	const svgLogo = 'data:image/svg+xml;base64,PHN2Zy8+';
	const cacheKey = 'coda.cloudProviderList';

	function listFile(kinds: string[]): string {
		return JSON.stringify({
			kind: 'cloudproviders',
			providers: kinds.map((kind) => ({
				kind: kind,
				label: kind + ' label',
				logo: svgLogo,
				hasSyncClient: false,
			})),
		});
	}

	// A fetch that answers every request with the same reply, and records the URLs it got.
	function fakeFetch(reply: {
		ok: boolean;
		status: number;
		body?: string;
	}): any {
		const fetchFunction: any = (url: string) => {
			fetchFunction.urls.push(url);
			return Promise.resolve({
				ok: reply.ok,
				status: reply.status,
				text: () => Promise.resolve(reply.body || ''),
			});
		};
		fetchFunction.urls = [];
		return fetchFunction;
	}

	function failingFetch(): any {
		return () => Promise.reject(new TypeError('Failed to fetch'));
	}

	function kindsOf(data: CloudProviderList.Data | null): string[] {
		return data ? data.providers.map((entry) => entry.kind) : [];
	}

	let storage: MemoryStorage;
	let warnings: string[];
	let originalWarn: any;

	beforeEach(function () {
		storage = new MemoryStorage();
		warnings = [];
		originalWarn = app.console.warn;
		app.console.warn = (...args: any[]) => {
			warnings.push(args.join(' '));
		};
	});

	afterEach(function () {
		app.console.warn = originalWarn;
	});

	it('caches a valid downloaded list and returns it', async function () {
		const data = await CloudProviderList.refresh(
			fakeFetch({
				ok: true,
				status: 200,
				body: listFile(['owncloud', 'pydio']),
			}),
			storage as any,
		);
		nodeassert.deepStrictEqual(kindsOf(data), ['owncloud', 'pydio']);
		nodeassert.deepStrictEqual(
			kindsOf(CloudProviderList.cached(storage as any)),
			['owncloud', 'pydio'],
		);
		nodeassert.deepStrictEqual(warnings, []);
	});

	it('keeps the cached list when the download fails', async function () {
		const cachedText = listFile(['owncloud']);
		const failures = [
			failingFetch(),
			fakeFetch({ ok: false, status: 404 }),
			fakeFetch({ ok: true, status: 200, body: '{' }),
		];
		for (const fetchFunction of failures) {
			storage.setItem(cacheKey, cachedText);
			const data = await CloudProviderList.refresh(
				fetchFunction,
				storage as any,
			);
			nodeassert.strictEqual(data, null);
			nodeassert.strictEqual(storage.getItem(cacheKey), cachedText);
		}
	});

	it('returns nothing when the downloaded list did not change', async function () {
		const fetchFunction = fakeFetch({
			ok: true,
			status: 200,
			body: listFile(['owncloud']),
		});
		nodeassert.notStrictEqual(
			await CloudProviderList.refresh(fetchFunction, storage as any),
			null,
		);
		nodeassert.strictEqual(
			await CloudProviderList.refresh(fetchFunction, storage as any),
			null,
		);
	});

	it('downloads the list once from the published address', async function () {
		const fetchFunction = fakeFetch({
			ok: true,
			status: 200,
			body: listFile(['owncloud']),
		});
		await CloudProviderList.refresh(fetchFunction, storage as any);
		nodeassert.deepStrictEqual(fetchFunction.urls, [
			CloudProviderList.remoteUrl,
		]);
	});

	it('ignores a cached list that it cannot read', function () {
		for (const text of ['{', '[]', listFile([])]) {
			storage.setItem(cacheKey, text);
			nodeassert.strictEqual(CloudProviderList.cached(storage as any), null);
		}
		storage.clear();
		nodeassert.strictEqual(CloudProviderList.cached(storage as any), null);
	});

	it('reads no list from a storage that throws', function () {
		const broken: any = {
			getItem: () => {
				throw new Error('denied');
			},
		};
		nodeassert.strictEqual(CloudProviderList.cached(broken), null);
	});

	it('logs one warning that names the cause of a failed download', async function () {
		const cases: [any, string][] = [
			[failingFetch(), 'network'],
			[fakeFetch({ ok: false, status: 404 }), '404'],
			[
				fakeFetch({ ok: true, status: 200, body: listFile([]) }),
				'no usable provider',
			],
		];
		for (const [fetchFunction, cause] of cases) {
			warnings = [];
			await CloudProviderList.refresh(fetchFunction, storage as any);
			nodeassert.strictEqual(warnings.length, 1, cause);
			nodeassert.ok(warnings[0].indexOf(cause) >= 0, warnings[0]);
		}
	});
});
