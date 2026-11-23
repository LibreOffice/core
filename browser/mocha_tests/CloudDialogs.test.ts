/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('The server URL of a new cloud account', function () {
	let page: DocumentPage;

	before(async function () {
		this.timeout(60000);
		page = await openDocumentPage('text');
	});

	after(function () {
		page?.close();
	});

	it('gets https:// in front of a host and port', function () {
		let submitted: string | undefined;
		const dialog = page.window.BackstageTemplates.addCloudDialog({
			isEdit: false,
			initialKind: page.window.BackstageCloudKinds.defaultKind(),
			initialDomain: '',
			initialName: '',
			onCancel: () => {},
			onSubmit: (_kind: string, _name: string, domain: string) => {
				submitted = domain;
			},
		});
		const inputs = dialog.querySelectorAll('.backstage-modal-input');
		inputs[0].value = 'localhost:8080';
		inputs[1].value = 'Cloud';
		dialog
			.querySelector('form')
			.dispatchEvent(new page.window.Event('submit', { cancelable: true }));
		nodeassert.equal(submitted, 'https://localhost:8080');
	});

	it('gets https:// in front of a host and path', function () {
		let submitted: string | undefined;
		const dialog = page.window.BackstageTemplates.addCloudDialog({
			isEdit: false,
			initialKind: page.window.BackstageCloudKinds.defaultKind(),
			initialDomain: '',
			initialName: '',
			onCancel: () => {},
			onSubmit: (_kind: string, _name: string, domain: string) => {
				submitted = domain;
			},
		});
		const inputs = dialog.querySelectorAll('.backstage-modal-input');
		inputs[0].value = 'cloud.example.com/office/';
		inputs[1].value = 'Cloud';
		dialog
			.querySelector('form')
			.dispatchEvent(new page.window.Event('submit', { cancelable: true }));
		nodeassert.equal(submitted, 'https://cloud.example.com/office/');
	});

	it('keeps an address that has http://', function () {
		let submitted: string | undefined;
		const dialog = page.window.BackstageTemplates.addCloudDialog({
			isEdit: false,
			initialKind: page.window.BackstageCloudKinds.defaultKind(),
			initialDomain: '',
			initialName: '',
			onCancel: () => {},
			onSubmit: (_kind: string, _name: string, domain: string) => {
				submitted = domain;
			},
		});
		const inputs = dialog.querySelectorAll('.backstage-modal-input');
		inputs[0].value = 'http://localhost:8080';
		inputs[1].value = 'Cloud';
		dialog
			.querySelector('form')
			.dispatchEvent(new page.window.Event('submit', { cancelable: true }));
		nodeassert.equal(submitted, 'http://localhost:8080');
	});
});
