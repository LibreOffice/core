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

describe('Collaboration notices in a document window', function () {
	let page: DocumentPage;

	before(async function () {
		this.timeout(60000);
		page = await openDocumentPage('text');
		// The user may edit, and has changed the document.
		page.serverSays('perm: edit');
		page.serverSays('statechanged: .uno:ModifiedStatus=true');
		await page.waitFor(() => page.map.isEditMode(), 'edit mode');
	});

	after(function () {
		page.close();
	});

	afterEach(async function () {
		page.map.uiManager.closeAll();
		await page.waitFor(
			() => !page.window.document.getElementById('collab-choice-title'),
			'the notice to close',
		);
	});

	// The notice on screen: its title, and the heading of each choice.
	async function noticeShown() {
		const document = page.window.document;
		await page.waitFor(
			() =>
				document.querySelector('[id$="-card"][data-coda-collab-card-wired]'),
			'the notice with its choices',
		);
		return {
			title: document.getElementById('collab-choice-title').textContent,
			choices: Array.from(
				document.querySelectorAll('[id$="-card-heading"]'),
			).map((heading: any) => heading.textContent),
		};
	}

	it('someone opening the document being edited is named, with a choice of how to go on', async function () {
		page.map._onCollabUserJoined('Robin');

		const notice = await noticeShown();
		nodeassert.equal(notice.title, 'Robin opened this document');
		nodeassert.deepEqual(notice.choices, [
			'Edit locally',
			'Collaborative editing',
		]);
	});

	it('choosing to edit together saves the local changes', async function () {
		page.map._onCollabUserJoined('Robin');
		await noticeShown();
		const sentBefore = page.sent.length;

		page.window.document.getElementById('start-collaborative-card').click();

		const saves = page.sent
			.slice(sentBefore)
			.filter((message) => message.startsWith('save '));
		nodeassert.equal(saves.length, 1);
	});

	it('someone starting to edit while this window edits is named the same way', async function () {
		page.map._onOtherUserEditingStarted('Robin');

		const notice = await noticeShown();
		nodeassert.equal(notice.title, 'Robin started editing');
		nodeassert.deepEqual(notice.choices, [
			'Edit locally',
			'Collaborative editing',
		]);
	});

	it('the page reported no errors', function () {
		nodeassert.deepEqual(page.errors, []);
	});
});
