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

// Five seconds after the last test, lists what still keeps node running. A run that exits in time
// prints nothing, because the timer does not hold node open itself. Node leaves timers out of
// process._getActiveHandles(), so each timer is recorded from its creation until it is destroyed.

const liveTimers = new Map<number, any>();

require('async_hooks')
	.createHook({
		init(id: number, type: string, trigger: number, resource: any) {
			if (type === 'Timeout') liveTimers.set(id, resource);
		},
		destroy(id: number) {
			liveTimers.delete(id);
		},
	})
	.enable();

function liveTimerDetail(timer: any): string {
	return (
		(timer._repeat ? 'interval of ' : 'timeout of ') +
		timer._idleTimeout +
		' ms: ' +
		String(timer._onTimeout).slice(0, 400)
	);
}

after(function () {
	const dump = setTimeout(function () {
		const node: any = process;
		const handles = node
			._getActiveHandles()
			.filter(
				(handle: any) => handle !== process.stdout && handle !== process.stderr,
			)
			.map((handle: any) =>
				handle.constructor ? handle.constructor.name : typeof handle,
			);
		const timers = [...liveTimers.values()]
			.filter((timer) => !timer._destroyed && timer.hasRef())
			.map(liveTimerDetail);
		console.error(
			'Still running after the tests: ' +
				handles.length +
				' handles, ' +
				timers.length +
				' timers, ' +
				node._getActiveRequests().length +
				' requests',
		);
		for (const line of handles.concat(timers)) console.error('  ' + line);
	}, 5000);
	dump.unref();
});
