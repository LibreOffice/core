// @ts-strict-ignore
/* -*- tab-width: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class ElapsedTime {
	private aTimeBase: ElapsedTime = null;
	private nLastQueriedTime: number;
	private nStartTime: number;
	private nFrozenTime: number;
	private bInPauseMode: boolean;
	private bInHoldMode: boolean;

	constructor(aTimeBase?: ElapsedTime) {
		this.aTimeBase = aTimeBase;
		this.reset();
	}

	getTimeBase() {
		return this.aTimeBase;
	}

	reset() {
		this.nLastQueriedTime = 0.0;
		this.nStartTime = this.getCurrentTime();
		this.nFrozenTime = 0.0;
		this.bInPauseMode = false;
		this.bInHoldMode = false;
	}

	getElapsedTime() {
		this.nLastQueriedTime = this.getElapsedTimeImpl();
		return this.nLastQueriedTime;
	}

	pauseTimer() {
		this.nFrozenTime = this.getElapsedTimeImpl();
		this.bInPauseMode = true;
	}

	continueTimer() {
		this.bInPauseMode = false;

		// stop pausing, time runs again. Note that
		// getElapsedTimeImpl() honors hold mode, i.e. a
		// continueTimer() in hold mode will preserve the latter
		const nPauseDuration = this.getElapsedTimeImpl() - this.nFrozenTime;

		// adjust start time, such that subsequent getElapsedTime() calls
		// will virtually start from m_fFrozenTime.
		this.nStartTime += nPauseDuration;
	}

	adjustTimer(nOffset: number) {
		// to make getElapsedTime() become _larger_, have to reduce nStartTime.
		this.nStartTime -= nOffset;

		// also adjust frozen time, this method must _always_ affect the
		// value returned by getElapsedTime()!
		if (this.bInHoldMode || this.bInPauseMode) this.nFrozenTime += nOffset;
	}

	holdTimer() {
		// when called during hold mode (e.g. more than once per time
		// object), the original hold time will be maintained.
		this.nFrozenTime = this.getElapsedTimeImpl();
		this.bInHoldMode = true;
	}

	releaseTimer() {
		this.bInHoldMode = false;
	}

	getSystemTime(): number {
		return getCurrentSystemTime() / 1000.0;
	}

	getCurrentTime(): number {
		let nCurrentTime;
		if (!this.aTimeBase) {
			nCurrentTime = this.getSystemTime();
		} else {
			nCurrentTime = this.aTimeBase.getElapsedTimeImpl();
		}

		assert(
			typeof nCurrentTime === typeof 0 && isFinite(nCurrentTime),
			'ElapsedTime.getCurrentTime: assertion failed: nCurrentTime == ' +
				nCurrentTime,
		);

		return nCurrentTime;
	}

	getElapsedTimeImpl(): number {
		if (this.bInHoldMode || this.bInPauseMode) {
			return this.nFrozenTime;
		}

		const nCurTime = this.getCurrentTime();
		return nCurTime - this.nStartTime;
	}
}
