// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global _ */

/*
 * One cloud service a branding offers. A branding declares its list in
 * branding.js, the same file that declares brandProductName, so the
 * services on offer come from the branding rather than from this tree.
 * The kind is a short identifier in lower-case letters, digits and
 * dashes. The label is the service name shown to the user, and is left
 * out when the kind reads well enough on its own.
 */
interface BrandCloudProvider {
	kind?: string;
	label?: string;
}

declare const brandCloudProviders: BrandCloudProvider[] | undefined;

/*
 * The cloud services the backstage Add-account dialog offers, and the
 * name and icon each one is drawn with.
 */
namespace BackstageCloudKinds {
	export interface CloudKindEntry {
		kind: CloudProviderKind;
		label: string;
		isBrand: boolean;
	}

	// A kind ends up inside a CSS class name and inside a stored account id, so it holds
	// lower-case letters, digits and dashes, and starts with a letter or a digit.
	const kindPattern = /^[a-z0-9][a-z0-9-]*$/;

	// Any server that speaks the protocol, whichever product runs it.
	const otherKind = 'other';

	// The services this build ships with. A branding offers its own instead.
	const shippedKinds: CloudKindEntry[] = [
		{ kind: 'nextcloud', label: 'Nextcloud', isBrand: false },
		{ kind: 'opencloud', label: 'OpenCloud', isBrand: false },
		{ kind: 'seafile', label: 'Seafile', isBrand: false },
	];

	export function isValidKind(kind: unknown): boolean {
		return typeof kind === 'string' && kindPattern.test(kind);
	}

	// The services on offer, in the order they are listed to the user. A branding that declares
	// a list is taken at its word, even when nothing in that list is usable: the shipped
	// services appear only where no branding declared a list at all. Other comes last, and
	// exactly once: a branding that names it as well gets the translated entry, not two rows.
	export function list(): CloudKindEntry[] {
		const branded = brandedKinds();
		const offered = branded !== undefined ? branded : shippedKinds;
		return offered.concat([
			{ kind: otherKind, label: _('Other'), isBrand: false },
		]);
	}

	// The service an account is drawn as. An account stored while another branding was in
	// place names a service that is no longer on offer, and is drawn as Other.
	export function offeredKind(kind: CloudProviderKind): CloudProviderKind {
		return find(kind) ? kind : otherKind;
	}

	// The kind the Add-account dialog starts on.
	export function defaultKind(): CloudProviderKind {
		return list()[0].kind;
	}

	// The name of a service. An account stored while another branding was in place names a
	// service that is no longer on offer, and reads as Other.
	export function label(kind: CloudProviderKind): string {
		const entry = find(kind);
		return entry ? entry.label : _('Other');
	}

	// The classes that draw a service's icon. The bare class alone draws the generic cloud, which
	// is what a service that is no longer on offer gets. A per-service class always carries the
	// backstage-cloud-icon- prefix, so it stays apart from the branded marker whatever the
	// service is called.
	export function iconClasses(kind: CloudProviderKind): string {
		const entry = find(kind);
		const classes = ['backstage-cloud-icon'];
		if (entry) {
			classes.push('backstage-cloud-icon-' + entry.kind);
			if (entry.isBrand) classes.push('backstage-cloud-brand-icon');
		}
		return classes.join(' ');
	}

	function find(kind: CloudProviderKind): CloudKindEntry | undefined {
		return list().find((entry) => entry.kind === kind);
	}

	// The services a branding declares, or undefined where no branding declared a list. An
	// empty result means a branding declared a list that named no usable service.
	function brandedKinds(): CloudKindEntry[] | undefined {
		const declared =
			typeof brandCloudProviders !== 'undefined'
				? brandCloudProviders
				: undefined;
		if (!Array.isArray(declared)) return undefined;
		const entries: CloudKindEntry[] = [];
		declared.forEach((provider) => {
			if (!provider || !isValidKind(provider.kind)) return;
			if (provider.kind === otherKind) return;
			const named =
				typeof provider.label === 'string' && provider.label.length > 0;
			entries.push({
				kind: provider.kind,
				label: named ? provider.label : provider.kind,
				isBrand: true,
			});
		});
		return entries;
	}
}
