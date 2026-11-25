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

describe('Backstage kind dropdown', function () {
	const globals = globalThis as any;
	const svgLogo = 'data:image/svg+xml;base64,PHN2Zy8+';
	let storage: MemoryStorage;
	let originalStorage: PropertyDescriptor;
	let originalRefresh: () => Promise<boolean>;
	let finishRefresh: (changed: boolean) => void;
	let dropdown: { element: HTMLElement; getValue: () => CloudProviderKind };

	function cacheList(kinds: string[]): void {
		storage.setItem(
			'coda.cloudProviderList',
			JSON.stringify({
				kind: 'cloudproviders',
				providers: kinds.map((kind) => ({
					kind: kind,
					label: kind + ' label',
					logo: svgLogo,
				})),
			}),
		);
	}

	function options(): HTMLElement[] {
		return Array.from(
			dropdown.element.querySelectorAll('.backstage-modal-dropdown-option'),
		);
	}

	function optionKinds(): string[] {
		return options().map((option) => option.getAttribute('data-value'));
	}

	function toggleLabel(): string {
		return dropdown.element.querySelector('.backstage-modal-dropdown-label')
			.textContent;
	}

	// Lets the download finish, then waits until the dropdown has handled the result.
	async function downloadFinishes(changed: boolean): Promise<void> {
		finishRefresh(changed);
		await new Promise((resolve) => setTimeout(resolve, 0));
	}

	before(function () {
		originalStorage = Object.getOwnPropertyDescriptor(globals, 'localStorage');
		originalRefresh = BackstageCloudKinds.refreshList;
	});

	beforeEach(function () {
		delete globals.brandCloudProviders;
		(CloudProviderList as any).builtInList = null;
		storage = new MemoryStorage();
		Object.defineProperty(globals, 'localStorage', {
			value: storage,
			configurable: true,
			writable: true,
		});
		(BackstageCloudKinds as any).refreshList = () =>
			new Promise<boolean>((resolve) => {
				finishRefresh = resolve;
			});
	});

	afterEach(function () {
		if (dropdown) dropdown.element.remove();
		dropdown = null;
	});

	after(function () {
		(BackstageCloudKinds as any).refreshList = originalRefresh;
		if (originalStorage)
			Object.defineProperty(globals, 'localStorage', originalStorage);
		else delete globals.localStorage;
	});

	it('shows the providers that arrive while the dialog is open', async function () {
		dropdown = BackstageTemplates.kindDropdown('other');
		document.body.appendChild(dropdown.element);
		nodeassert.deepStrictEqual(optionKinds(), ['other']);

		cacheList(['nextcloud', 'seafile']);
		await downloadFinishes(true);
		nodeassert.deepStrictEqual(optionKinds(), [
			'nextcloud',
			'seafile',
			'other',
		]);
		nodeassert.strictEqual(dropdown.getValue(), 'other');
		nodeassert.deepStrictEqual(
			options().map((option) => option.getAttribute('aria-selected')),
			['false', 'false', 'true'],
		);
	});

	it('leaves a dialog that was closed before the list arrived unchanged', async function () {
		dropdown = BackstageTemplates.kindDropdown('other');
		cacheList(['nextcloud']);
		await downloadFinishes(true);
		nodeassert.deepStrictEqual(optionKinds(), ['other']);
	});

	it('keeps the options when the list did not change', async function () {
		cacheList(['nextcloud']);
		dropdown = BackstageTemplates.kindDropdown('other');
		document.body.appendChild(dropdown.element);
		const before = options();
		await downloadFinishes(false);
		nodeassert.deepStrictEqual(options(), before);
	});

	it('keeps the kind that the user picked when the list arrives', async function () {
		cacheList(['nextcloud']);
		dropdown = BackstageTemplates.kindDropdown('other');
		document.body.appendChild(dropdown.element);
		options()[0].click();
		nodeassert.strictEqual(dropdown.getValue(), 'nextcloud');

		cacheList(['seafile', 'nextcloud']);
		await downloadFinishes(true);
		nodeassert.strictEqual(dropdown.getValue(), 'nextcloud');
		nodeassert.strictEqual(toggleLabel(), 'nextcloud label');

		options()[0].click();
		nodeassert.strictEqual(dropdown.getValue(), 'seafile');
	});

	it('starts a new account on the first provider once the list arrives', async function () {
		dropdown = BackstageTemplates.kindDropdown(null);
		document.body.appendChild(dropdown.element);
		nodeassert.strictEqual(dropdown.getValue(), 'other');

		cacheList(['nextcloud', 'seafile']);
		await downloadFinishes(true);
		nodeassert.strictEqual(dropdown.getValue(), 'nextcloud');
		nodeassert.strictEqual(toggleLabel(), 'nextcloud label');
	});

	it('shows an edited account with its own kind once that kind arrives', async function () {
		dropdown = BackstageTemplates.kindDropdown('nextcloud');
		document.body.appendChild(dropdown.element);
		nodeassert.strictEqual(toggleLabel(), 'Other');

		cacheList(['nextcloud']);
		await downloadFinishes(true);
		nodeassert.strictEqual(dropdown.getValue(), 'nextcloud');
		nodeassert.strictEqual(toggleLabel(), 'nextcloud label');
	});

	it('keeps the kind of an edited account while no list is known', function () {
		dropdown = BackstageTemplates.kindDropdown('nextcloud');
		document.body.appendChild(dropdown.element);
		nodeassert.strictEqual(toggleLabel(), 'Other');
		nodeassert.strictEqual(dropdown.getValue(), 'nextcloud');
	});

	it('changes an edited account to Other when the user picks it', function () {
		dropdown = BackstageTemplates.kindDropdown('nextcloud');
		document.body.appendChild(dropdown.element);
		options()[0].click();
		nodeassert.strictEqual(dropdown.getValue(), 'other');
	});

	it('changes an edited account to Other when the list does not offer its kind', function () {
		cacheList(['seafile']);
		dropdown = BackstageTemplates.kindDropdown('nextcloud');
		document.body.appendChild(dropdown.element);
		nodeassert.strictEqual(toggleLabel(), 'Other');
		nodeassert.strictEqual(dropdown.getValue(), 'other');
	});
});
