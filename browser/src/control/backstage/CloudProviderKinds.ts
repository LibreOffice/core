// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Office contributors.
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
		logo?: string;
		logoDark?: string;
	}

	// Any server that speaks the protocol, whichever product runs it.
	const otherKind = 'other';

	// The services this build ships with. A branding offers its own instead.
	const shippedKinds: CloudKindEntry[] = [
		{
			kind: 'nextcloud',
			label: 'Nextcloud',
			isBrand: false,
			logo: 'images/cloud-providers/nextcloud.svg',
		},
		{
			kind: 'opencloud',
			label: 'OpenCloud',
			isBrand: false,
			logo: 'images/cloud-providers/opencloud.svg',
		},
		{
			kind: 'seafile',
			label: 'Seafile',
			isBrand: false,
			logo: 'images/cloud-providers/seafile.svg',
		},
	];

	export function isValidKind(kind: unknown): boolean {
		return CloudProviderList.isValidKind(kind);
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

	// Whether a branding declares its own list of services.
	export function isBranded(): boolean {
		return brandedKinds() !== undefined;
	}

	// The icon of a service, with className added to its classes. A listed service shows its
	// logo, and a second logo for the dark theme when it has one. The bare backstage-cloud-icon
	// class draws the generic cloud, which a service that is no longer listed gets. A branded
	// service gets the classes that a branding stylesheet draws.
	export function icon(
		kind: CloudProviderKind,
		className: string,
	): HTMLElement {
		const element = document.createElement('span');
		element.setAttribute('aria-hidden', 'true');
		const classes = [className, 'backstage-cloud-icon'];
		const entry = find(kind);
		if (entry && entry.isBrand)
			classes.push(
				'backstage-cloud-icon-' + entry.kind,
				'backstage-cloud-brand-icon',
			);
		if (entry && entry.logo) {
			classes.push('has-logo');
			element.appendChild(logoImage(entry.logo, ''));
			if (entry.logoDark)
				element.appendChild(
					logoImage(entry.logoDark, 'backstage-cloud-logo-dark'),
				);
		}
		element.className = classes.join(' ');
		return element;
	}

	function logoImage(source: string, className: string): HTMLImageElement {
		const image = document.createElement('img');
		image.setAttribute('src', source);
		image.setAttribute('alt', '');
		image.draggable = false;
		if (className) image.className = className;
		return image;
	}

	function find(kind: CloudProviderKind): CloudKindEntry | undefined {
		return list().find((entry) => entry.kind === kind);
	}

	// The services a branding declares, or undefined where no branding declared a list. An
	// empty result means a branding declared a list that named no usable service.
	function brandedKinds(): CloudKindEntry[] | undefined {
		const declared =
			typeof brandCloudProviders !== 'undefined' ? brandCloudProviders : (
				undefined
			);
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
