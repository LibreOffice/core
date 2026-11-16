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
	// holds no usable provider. Unknown fields are ignored.
	export function validate(value: unknown): Data | null {
		if (!isObject(value)) return null;
		if (value.kind !== fileKind) return null;
		if (!Array.isArray(value.providers)) return null;

		const providers: Provider[] = [];
		value.providers.forEach((entry: unknown) => {
			const provider = validateProvider(entry);
			if (provider) providers.push(provider);
		});
		if (providers.length === 0) return null;
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
		const data = validate(parsed);
		return data ? data : reject('the file is not a valid cloud provider list');
	}
}
