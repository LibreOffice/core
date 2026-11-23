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

describe('The /cool/collab user list of a page', function () {
	let page: DocumentPage;

	before(async function () {
		this.timeout(60000);
		page = await openDocumentPage('text');
	});

	after(function () {
		page?.close();
	});

	it('keeps the other connection of a user when one of them leaves', function () {
		const collab = page.window.collab;
		collab.notificationHandler({
			data: JSON.stringify({
				type: 'user_list',
				users: [
					{ id: 'alice', name: 'Alice', connection: 'collab_1' },
					{ id: 'alice', name: 'Alice', connection: 'collab_2' },
				],
				editingActive: false,
			}),
		});
		collab.notificationHandler({
			data: JSON.stringify({
				type: 'user_left',
				user: { id: 'alice', name: 'Alice', connection: 'collab_1' },
			}),
		});
		nodeassert.deepEqual(
			Array.from(collab.users, (user: any) => user.connection),
			['collab_2'],
		);
	});
});
