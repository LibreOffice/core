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

// A stand-in for setTimeout, so a delay can be asked what it does rather than
// waited out.

interface PendingTimer {
	id: number;
	at: number;
	fn: () => void;
}

let clockNow = 0;
let pendingTimers: PendingTimer[] = [];
let nextTimerId = 1;
let savedSetTimeout: any;
let savedClearTimeout: any;

function installFakeClock(): void {
	clockNow = 0;
	pendingTimers = [];
	nextTimerId = 1;
	savedSetTimeout = globalThis.setTimeout;
	savedClearTimeout = globalThis.clearTimeout;

	(globalThis as any).setTimeout = function (fn: () => void, delay: number) {
		const id = nextTimerId++;

		pendingTimers.push({ id: id, at: clockNow + (delay || 0), fn: fn });
		return id;
	};

	// Mocha's own timers were taken out with the real setTimeout and carry an
	// object for an id, so anything this clock does not know goes back to Node.
	(globalThis as any).clearTimeout = function (id: any) {
		if (pendingTimers.some((timer) => timer.id === id))
			pendingTimers = pendingTimers.filter((timer) => timer.id !== id);
		else savedClearTimeout(id);
	};
}

function restoreClock(): void {
	(globalThis as any).setTimeout = savedSetTimeout;
	(globalThis as any).clearTimeout = savedClearTimeout;
}

// Earliest first, so a callback that sets another timer inside the window still
// runs.
function advanceClock(ms: number): void {
	const until = clockNow + ms;

	for (;;) {
		const due = pendingTimers
			.filter((timer) => timer.at <= until)
			.sort((one, other) => one.at - other.at)[0];

		if (!due) break;

		pendingTimers = pendingTimers.filter((timer) => timer !== due);
		clockNow = due.at;
		due.fn();
	}

	clockNow = until;
}
