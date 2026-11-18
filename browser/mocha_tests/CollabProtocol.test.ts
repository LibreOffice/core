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

describe('The /co/collab protocol number of a page', function () {
	let page: DocumentPage;

	before(async function () {
		this.timeout(60000);
		page = await openDocumentPage('text');
	});

	after(function () {
		page?.close();
	});

	it('signs in with a server that speaks its protocol', async function () {
		const pageWebSocket = page.window.WebSocket;
		let socket: any;
		page.window.WebSocket = function (this: any, url: string) {
			this.url = url;
			this.send = () => {};
			this.close = () => {};
			socket = this;
		};
		const signIn = page.window.collab.connect(
			'https://host/wopi/files/1',
			'token',
			'https://host',
		);
		page.window.WebSocket = pageWebSocket;
		nodeassert.match(socket.url, /&protocol=1$/);
		socket.onmessage({
			data: JSON.stringify({
				type: 'authenticated',
				protocol: 1,
				fileInfo: { UserCanWrite: true },
			}),
		});
		nodeassert.equal((await signIn).UserCanWrite, true);
	});

	it('gives up on a server that speaks an older protocol', async function () {
		const pageWebSocket = page.window.WebSocket;
		let socket: any;
		page.window.WebSocket = function (this: any, url: string) {
			this.url = url;
			this.send = () => {};
			this.close = () => {};
			socket = this;
		};
		const signIn = page.window.collab.connect(
			'https://host/wopi/files/1',
			'token',
			'https://host',
		);
		page.window.WebSocket = pageWebSocket;
		socket.onmessage({
			data: JSON.stringify({
				type: 'authenticated',
				protocol: 0,
				fileInfo: {},
			}),
		});
		await nodeassert.rejects(signIn, /the server speaks protocol/);
	});

	it('gives up on a server without protocol numbers', async function () {
		const pageWebSocket = page.window.WebSocket;
		let socket: any;
		page.window.WebSocket = function (this: any, url: string) {
			this.url = url;
			this.send = () => {};
			this.close = () => {};
			socket = this;
		};
		const signIn = page.window.collab.connect(
			'https://host/wopi/files/1',
			'token',
			'https://host',
		);
		page.window.WebSocket = pageWebSocket;
		socket.onmessage({
			data: JSON.stringify({ type: 'authenticated', fileInfo: {} }),
		});
		await nodeassert.rejects(signIn, /the server speaks protocol/);
	});
});
