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

describe('AccountPresetSync', function () {
	const uriA = 'https://cloud.example/settings/shared/a.otp';
	const uriB = 'https://cloud.example/settings/shared/b.otp';

	function bytesOf(text: string): ArrayBuffer {
		return new TextEncoder().encode(text).buffer as ArrayBuffer;
	}

	// The number of bytes of a file that one storePart call carries.
	const pieceBytes = 90 * 1024;

	// Fake dependencies that record every call. lists gives the settings list of each stream,
	// and a missing one cannot be fetched. wanted gives what applyList answers for each list.
	// refusedOffsets gives the offsets at which storePart answers false.
	function fakeDeps(
		lists: any,
		wanted: any,
		failingDownloads: string[] = [],
		refusedOffsets: number[] = [],
	) {
		const calls = {
			fetchStream: [] as any[],
			applyList: [] as any[],
			storePart: [] as any[],
			finishFile: [] as any[],
		};
		const deps: AccountPresetSyncDeps = {
			fetchStream: (request: any) => {
				calls.fetchStream.push(request);
				if (request.stream === 'settingsFile')
					return Promise.resolve('file:' + request.uri);
				if (lists[request.stream] === undefined)
					return Promise.reject(new Error('Stream not available'));
				return Promise.resolve('list:' + request.stream);
			},
			download: (url: string) => {
				if (failingDownloads.includes(url))
					return Promise.reject(new Error('HTTP 500'));
				if (url.startsWith('list:'))
					return Promise.resolve(bytesOf(lists[url.substring(5)]));
				return Promise.resolve(bytesOf('bytes of ' + url.substring(5)));
			},
			applyList: (list: string, group: string, entriesJson: string) => {
				calls.applyList.push([list, group, entriesJson]);
				return Promise.resolve(wanted[list] || []);
			},
			storePart: (
				list: string,
				group: string,
				uri: string,
				offset: number,
				base64: string,
			) => {
				calls.storePart.push([list, group, uri, offset, base64]);
				return Promise.resolve(!refusedOffsets.includes(offset));
			},
			finishFile: (list: string, group: string, uri: string, stamp: string) => {
				calls.finishFile.push([list, group, uri, stamp]);
				return Promise.resolve(true);
			},
		};
		return { deps: deps, calls: calls };
	}

	function sharedList(): string {
		return JSON.stringify({
			template: [
				{ uri: uriA, stamp: '1' },
				{ uri: uriB, stamp: '7' },
			],
		});
	}

	it('fetches only the files that applyList names', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriB] },
		);
		await syncAccountPresets(fake.deps);

		const fileRequests = fake.calls.fetchStream.filter(
			(request) => request.stream === 'settingsFile',
		);
		nodeassert.deepStrictEqual(fileRequests, [
			{ stream: 'settingsFile', list: 'shared', group: 'template', uri: uriB },
		]);
		nodeassert.deepStrictEqual(fake.calls.storePart, [
			['shared', 'template', uriB, 0, btoa('bytes of ' + uriB)],
		]);
		nodeassert.deepStrictEqual(fake.calls.finishFile, [
			['shared', 'template', uriB, '7'],
		]);
		nodeassert.deepStrictEqual(fake.calls.applyList[0], [
			'shared',
			'template',
			JSON.stringify([
				{ uri: uriA, stamp: '1' },
				{ uri: uriB, stamp: '7' },
			]),
		]);
	});

	it('a list that cannot be fetched is left alone', async function () {
		const fake = fakeDeps({ userSettings: '{"template":[]}' }, {});
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(fake.calls.applyList, [
			['user', 'template', '[]'],
		]);
	});

	it('a list that is no JSON object is left alone', async function () {
		const fake = fakeDeps(
			{ sharedSettings: 'not json', userSettings: '[]' },
			{},
		);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(fake.calls.applyList, []);
	});

	it('a list with no template group applies an empty group', async function () {
		const fake = fakeDeps(
			{ sharedSettings: '{}', userSettings: '{"xcu":[]}' },
			{},
		);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(fake.calls.applyList, [
			['shared', 'template', '[]'],
			['user', 'template', '[]'],
		]);
	});

	it('a file that fails does not stop the next one', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriA, uriB] },
			['file:' + uriA],
		);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(
			fake.calls.finishFile.map((call) => call[2]),
			[uriB],
		);
	});

	it('a file that is larger than the limit is not stored', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriA, uriB] },
		);
		const download = fake.deps.download;
		fake.deps.download = (url: string, maxBytes: number) =>
			url === 'file:' + uriA ?
				Promise.resolve(new ArrayBuffer(50 * 1024 * 1024 + 1))
			:	download(url, maxBytes);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(
			fake.calls.finishFile.map((call) => call[2]),
			[uriB],
		);
	});

	// A download that gives size bytes for uriA, each byte the low part of its offset.
	function downloadOfSize(fake: any, size: number): Uint8Array {
		const bytes = new Uint8Array(size);
		for (let i = 0; i < size; i++) bytes[i] = i % 251;
		const download = fake.deps.download;
		fake.deps.download = (url: string, maxBytes: number) =>
			url === 'file:' + uriA ?
				Promise.resolve(bytes.buffer as ArrayBuffer)
			:	download(url, maxBytes);
		return bytes;
	}

	function base64Of(bytes: Uint8Array): string {
		let binary = '';
		for (let i = 0; i < bytes.length; i++)
			binary += String.fromCharCode(bytes[i]);
		return btoa(binary);
	}

	it('a file goes in pieces, each at the offset where the last one ends', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriA] },
		);
		const bytes = downloadOfSize(fake, 2 * pieceBytes + 5);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(fake.calls.storePart, [
			['shared', 'template', uriA, 0, base64Of(bytes.subarray(0, pieceBytes))],
			[
				'shared',
				'template',
				uriA,
				pieceBytes,
				base64Of(bytes.subarray(pieceBytes, 2 * pieceBytes)),
			],
			[
				'shared',
				'template',
				uriA,
				2 * pieceBytes,
				base64Of(bytes.subarray(2 * pieceBytes)),
			],
		]);
		nodeassert.deepStrictEqual(fake.calls.finishFile, [
			['shared', 'template', uriA, '1'],
		]);
	});

	it('an empty file goes as one empty piece', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriA] },
		);
		downloadOfSize(fake, 0);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(fake.calls.storePart, [
			['shared', 'template', uriA, 0, ''],
		]);
		nodeassert.deepStrictEqual(fake.calls.finishFile, [
			['shared', 'template', uriA, '1'],
		]);
	});

	it('a file with a refused piece is not finished, and the next file is stored', async function () {
		const fake = fakeDeps(
			{ sharedSettings: sharedList(), userSettings: '{}' },
			{ shared: [uriA, uriB] },
			[],
			[pieceBytes],
		);
		downloadOfSize(fake, 3 * pieceBytes);
		await syncAccountPresets(fake.deps);

		nodeassert.deepStrictEqual(
			fake.calls.storePart.map((call) => [call[2], call[3]]),
			[
				[uriA, 0],
				[uriA, pieceBytes],
				[uriB, 0],
			],
		);
		nodeassert.deepStrictEqual(fake.calls.finishFile, [
			['shared', 'template', uriB, '7'],
		]);
	});
});
