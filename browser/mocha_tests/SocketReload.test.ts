/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('Socket reload after a fatal error', function () {
	// The intervals the socket set, and not cleared yet, by id.
	let intervals: Map<number, () => void>;
	let nextId: number;
	let savedSetInterval: any;
	let savedClearTimeout: any;
	let savedErrorMessages: any;
	let savedIdleHandler: any;
	let loads: number;
	let map: any;
	let socket: any;

	beforeEach(function () {
		intervals = new Map();
		nextId = 1000;
		savedSetInterval = globalThis.setInterval;
		savedClearTimeout = globalThis.clearTimeout;
		(globalThis as any).setInterval = function (fn: () => void) {
			const id = nextId++;
			intervals.set(id, fn);
			return id;
		};
		// The socket clears its intervals with clearTimeout.
		(globalThis as any).clearTimeout = function (id: any) {
			if (!intervals.delete(id)) savedClearTimeout(id);
		};

		savedErrorMessages = (window as any).errorMessages;
		(window as any).errorMessages = {
			docunloadingretry: 'retry',
			docunloadinggiveup: 'give up',
			unauthorized: 'unauthorized',
			verificationerror: 'verification',
			storageunavailable: 'unavailable',
		};

		savedIdleHandler = app.idleHandler;
		(app as any).idleHandler = { _active: true, _serverRecycling: false };

		loads = 0;
		map = {
			_fatal: false,
			_docLayer: null,
			options: {},
			fire: () => {},
			hideBusy: () => {},
			showBusy: () => {},
			loadDocument: () => {
				++loads;
			},
		};

		socket = new Socket(map);
		socket.connected = () => false;
		socket.close = () => {};
	});

	afterEach(function () {
		(globalThis as any).setInterval = savedSetInterval;
		(globalThis as any).clearTimeout = savedClearTimeout;
		(window as any).errorMessages = savedErrorMessages;
		(app as any).idleHandler = savedIdleHandler;
	});

	function receive(message: string): void {
		socket._onErrorMsg(message, socket.parseServerCmd(message));
	}

	function tick(): void {
		for (const fn of Array.from(intervals.values())) fn();
	}

	it('asks for the document again while it unloads', function () {
		receive('error: cmd=load kind=docunloading');
		tick();
		nodeassert.strictEqual(loads, 1);
	});

	it('stops asking for the document after a fatal error', function () {
		receive('error: cmd=load kind=docunloading');
		receive('error: cmd=internal kind=unauthorized');
		nodeassert.strictEqual(map._fatal, true);

		// Before, the timer went on loading the document again, and with it asking the host
		// for the file info, which only refused again.
		tick();
		nodeassert.strictEqual(loads, 0);
		nodeassert.strictEqual(intervals.size, 0);
	});
});
