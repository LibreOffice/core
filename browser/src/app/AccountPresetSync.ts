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

/*
 * Brings the stored template files of a cloud account up to date with the two settings lists of
 * its server, the "shared" list and the "user" list. A settings list is a JSON object that gives,
 * for each group, an array of entries with a uri and a stamp.
 */

interface AccountPresetSyncDeps {
	// Asks the server for one fetch stream and gives the URL to download it from.
	fetchStream(request: object): Promise<string>;
	// Gives the bytes at url. Fails when the server announces more than maxBytes.
	download(url: string, maxBytes: number): Promise<ArrayBuffer>;
	// Removes the stored files that the entries of the group do not name, and gives the uris of
	// the entries whose file is missing or has another stamp. entriesJson is the JSON text of
	// the entries of the group.
	applyList(
		list: string,
		group: string,
		entriesJson: string,
	): Promise<string[]>;
	// Stores the part at offset of one file, and gives false when it was not stored. A part at
	// offset 0 starts the file, and each next part starts where the parts before it end.
	storePart(
		list: string,
		group: string,
		uri: string,
		offset: number,
		base64: string,
	): Promise<boolean>;
	// Stores the parts of one file as that file, with its stamp. Gives false when it was not
	// stored.
	finishFile(
		list: string,
		group: string,
		uri: string,
		stamp: string,
	): Promise<boolean>;
}

interface AccountPresetEntry {
	uri: string;
	stamp: string;
}

// The size of the largest file that is stored.
const accountPresetMaxBytes = 50 * 1024 * 1024;

// The number of bytes of a file that one bridge message carries. Their base64 text and the other
// arguments of the call fit in one part of a bridge message.
const accountPresetPieceBytes = 90 * 1024;

function accountPresetBase64(view: Uint8Array): string {
	const chunk = 0x8000;
	let binary = '';
	for (let i = 0; i < view.length; i += chunk)
		binary += String.fromCharCode.apply(
			null,
			view.subarray(i, i + chunk) as unknown as number[],
		);
	return btoa(binary);
}

// Stores bytes as the file of entry. Each piece goes after the reply to the piece before it, so
// the other bridge messages of the page go between the pieces. Gives false when a piece or the
// finish is refused.
async function storeAccountPresetFile(
	deps: AccountPresetSyncDeps,
	list: string,
	group: string,
	entry: AccountPresetEntry,
	bytes: ArrayBuffer,
): Promise<boolean> {
	if (bytes.byteLength > accountPresetMaxBytes)
		throw new Error('the file is larger than the limit');
	const view = new Uint8Array(bytes);
	// The piece at offset 0 always goes, because it starts the file.
	let offset = 0;
	do {
		const piece = view.subarray(offset, offset + accountPresetPieceBytes);
		// eslint-disable-next-line no-await-in-loop
		const stored = await deps.storePart(
			list,
			group,
			entry.uri,
			offset,
			accountPresetBase64(piece),
		);
		if (!stored) return false;
		offset += piece.length;
	} while (offset < view.length);
	return deps.finishFile(list, group, entry.uri, entry.stamp);
}

// The entries of one group of a settings list. A list with no such group gives no entry.
function accountPresetEntries(
	settings: any,
	group: string,
): AccountPresetEntry[] {
	if (!settings || typeof settings !== 'object' || Array.isArray(settings))
		throw new Error('the settings list is not a JSON object');
	if (!Array.isArray(settings[group])) return [];
	return settings[group]
		.filter((entry: any) => entry && typeof entry.uri === 'string')
		.map((entry: any) => ({ uri: entry.uri, stamp: String(entry.stamp) }));
}

function syncAccountPresetFile(
	deps: AccountPresetSyncDeps,
	list: string,
	group: string,
	entry: AccountPresetEntry,
): Promise<void> {
	return deps
		.fetchStream({
			stream: 'settingsFile',
			list: list,
			group: group,
			uri: entry.uri,
		})
		.then((url) => deps.download(url, accountPresetMaxBytes))
		.then((bytes) => storeAccountPresetFile(deps, list, group, entry, bytes))
		.then((stored) => {
			if (!stored)
				window.app.console.warn(
					'Account presets: a file of the ' + list + ' list was not stored',
				);
		})
		.catch((error) => {
			window.app.console.warn(
				'Account presets: a file of the ' + list + ' list failed: ' + error,
			);
		});
}

// A list that cannot be fetched or read keeps the files it has.
function syncAccountPresetList(
	deps: AccountPresetSyncDeps,
	list: string,
): Promise<void> {
	const group = 'template';
	return deps
		.fetchStream({
			stream: list === 'shared' ? 'sharedSettings' : 'userSettings',
		})
		.then((url) => deps.download(url, accountPresetMaxBytes))
		.then((bytes) => {
			const entries = accountPresetEntries(
				JSON.parse(new TextDecoder().decode(bytes)),
				group,
			);
			return deps
				.applyList(list, group, JSON.stringify(entries))
				.then((wanted) => {
					let files = Promise.resolve();
					for (const entry of entries) {
						if (wanted.includes(entry.uri))
							files = files.then(() =>
								syncAccountPresetFile(deps, list, group, entry),
							);
					}
					return files;
				});
		})
		.catch((error) => {
			window.app.console.warn(
				'Account presets: the ' + list + ' list failed: ' + error,
			);
		});
}

function syncAccountPresets(deps: AccountPresetSyncDeps): Promise<void> {
	return syncAccountPresetList(deps, 'shared').then(() =>
		syncAccountPresetList(deps, 'user'),
	);
}
