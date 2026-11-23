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

describe('A new access token on the /cool/collab connection of a page', function () {
	let page: DocumentPage;

	before(async function () {
		this.timeout(60000);
		page = await openDocumentPage('text');
	});

	after(function () {
		page?.close();
	});

	it('goes to the server, encoded as in the first message', function () {
		const collab = page.window.collab;
		const sent: string[] = [];
		collab.ws = {
			readyState: page.window.WebSocket.OPEN,
			send: (data: string) => sent.push(data),
		};
		collab.renewAccessToken('a b');
		collab.ws = null;
		nodeassert.deepEqual(sent, ['{"type":"access_token","token":"a%20b"}']);
	});
});
