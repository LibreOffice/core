/* -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*- */

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
 * The list of cloud services that the backstage Add-account dialog offers, as data. A list file is
 * a JSON object with kind "cloudproviders" and the providers.
 */
namespace CloudProviderList {
	// One cloud service.
	export interface Provider {
		kind: string; // A short identifier.
		label: string; // The brand name shown to the user.
		logo: string; // An image URL for the light theme.
		logoDark?: string; // An image URL for the dark theme.
		hasSyncClient: boolean; // True when the service has a desktop client that syncs files.
	}

	export interface Data {
		providers: Provider[];
	}

	const fileKind = 'cloudproviders';
	const maxRemoteLength = 524288;

	// A kind is part of a stored account id, which has at most 64 characters, and of a CSS class
	// name. So it holds lower-case letters, digits and dashes, starts with a letter or a digit,
	// and has at most 32 characters.
	const kindPattern = /^[a-z0-9][a-z0-9-]*$/;
	const maxKindLength = 32;

	// Any server that speaks the protocol. It is always the last entry, and no list names it.
	const otherKind = 'other';

	// A logo is inline data, because the page loads images only from its own files and a few
	// fixed hosts.
	const logoPrefixes = ['data:image/svg+xml', 'data:image/png'];

	export function isValidKind(kind: unknown): boolean {
		return (
			typeof kind === 'string' &&
			kind.length <= maxKindLength &&
			kindPattern.test(kind)
		);
	}

	function isValidLogo(logo: unknown): boolean {
		if (typeof logo !== 'string') return false;
		return logoPrefixes.some((prefix) => logo.indexOf(prefix) === 0);
	}

	function isObject(value: unknown): value is Record<string, unknown> {
		return typeof value === 'object' && value !== null && !Array.isArray(value);
	}

	// A copy of the provider with only the known fields, or null when a required field is missing
	// or has the wrong type. A bad dark logo is left out, and the provider stays.
	function validateProvider(value: unknown): Provider | null {
		if (!isObject(value)) return null;
		if (!isValidKind(value.kind) || value.kind === otherKind) return null;
		if (typeof value.label !== 'string' || value.label.length === 0)
			return null;
		if (!isValidLogo(value.logo)) return null;

		const provider: Provider = {
			kind: value.kind as string,
			label: value.label,
			logo: value.logo as string,
			hasSyncClient: value.hasSyncClient === true,
		};
		if (isValidLogo(value.logoDark))
			provider.logoDark = value.logoDark as string;
		return provider;
	}

	// The list a parsed file holds, or null when the file is not a list this code can read or
	// holds no usable provider. Unknown fields are ignored. reject receives the reason for a null
	// result.
	export function validate(
		value: unknown,
		reject: (reason: string) => null = () => null,
	): Data | null {
		if (!isObject(value) || value.kind !== fileKind)
			return reject('the file is not a cloud provider list');
		if (!Array.isArray(value.providers))
			return reject('the file is not a cloud provider list');

		const providers: Provider[] = [];
		value.providers.forEach((entry: unknown) => {
			const provider = validateProvider(entry);
			if (provider) providers.push(provider);
		});
		if (providers.length === 0)
			return reject('the file holds no usable provider');
		return { providers: providers };
	}

	// The list a downloaded file holds, or null. onReject receives the reason for a null result.
	export function parseRemote(
		text: string,
		onReject?: (reason: string) => void,
	): Data | null {
		const reject = (reason: string): null => {
			if (onReject) onReject(reason);
			return null;
		};
		if (text.length > maxRemoteLength)
			return reject('the file is over ' + maxRemoteLength + ' bytes');

		let parsed: unknown;
		try {
			parsed = JSON.parse(text);
		} catch (error) {
			return reject('the file is not JSON');
		}
		return validate(parsed, reject);
	}

	// The reply of a fetch call, as far as refresh reads it.
	export interface FetchReply {
		ok: boolean;
		status: number;
		text(): Promise<string>;
	}

	export type FetchFunction = (
		url: string,
		init?: RequestInit,
	) => Promise<FetchReply>;

	// The published list. The address stays the same, and the file behind it changes.
	export const remoteUrl = 'https://col.la/codacloudproviders';

	// The last downloaded list after validation, stored in the format of the file.
	const cacheKey = 'coda.cloudProviderList';

	function readItem(storage: Storage, key: string): string | null {
		try {
			return storage.getItem(key);
		} catch (error) {
			return null;
		}
	}

	// The last downloaded list, or null when there is none or it cannot be read.
	export function cached(storage: Storage): Data | null {
		const text = readItem(storage, cacheKey);
		if (text === null) return null;
		try {
			return validate(JSON.parse(text));
		} catch (error) {
			return null;
		}
	}

	// Downloads the published list and stores it. Resolves with the new list when it differs
	// from the stored one, and with null in every other case. A failure keeps the stored list.
	export async function refresh(
		fetchFunction: FetchFunction,
		storage: Storage,
	): Promise<Data | null> {
		const fail = (reason: string): null => {
			window.app.console.warn(
				'Cloud provider list: ' + reason + ', so the stored list stays',
			);
			return null;
		};

		let text: string;
		try {
			// no-cache makes the web view ask the server whether its copy is current.
			const reply = await fetchFunction(remoteUrl, { cache: 'no-cache' });
			if (!reply.ok)
				return fail(remoteUrl + ' answered with HTTP status ' + reply.status);
			text = await reply.text();
		} catch (error) {
			return fail('a network error stopped the download of ' + remoteUrl);
		}

		let reason = '';
		const data = parseRemote(text, (rejected) => {
			reason = rejected;
		});
		if (!data) return fail(reason);

		const stored = JSON.stringify({
			kind: fileKind,
			providers: data.providers,
		});
		if (readItem(storage, cacheKey) === stored) return null;
		try {
			storage.setItem(cacheKey, stored);
		} catch (error) {
			return fail('the browser storage refused the list');
		}
		return data;
	}
}
