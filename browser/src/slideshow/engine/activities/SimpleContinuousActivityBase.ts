/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

abstract class SimpleContinuousActivityBase extends ActivityBase {
	private aTimer: ElapsedTime;
	protected nMinSimpleDuration: number;
	private nMinNumberOfFrames: number;
	protected nCurrPerformCalls: number;
	constructor(aCommonParamSet: ActivityParamSet) {
		super(aCommonParamSet);

		// Time elapsed since activity started
		this.aTimer = new ElapsedTime(aCommonParamSet.aActivityQueue.getTimer());
		// Simple duration of activity
		this.nMinSimpleDuration = aCommonParamSet.nMinDuration;
		// Minimal number of frames to show
		this.nMinNumberOfFrames = aCommonParamSet.nMinNumberOfFrames;
		// Actual number of frames shown until now.
		this.nCurrPerformCalls = 0;
	}

	public startAnimation() {
		// init timer. We measure animation time only when we're
		// actually started.
		this.aTimer.reset();
	}

	public calcTimeLag() {
		super.calcTimeLag();

		if (!this.isActive()) return 0.0;

		// retrieve locally elapsed time
		const nCurrElapsedTime = this.aTimer.getElapsedTime();

		// go to great length to ensure a proper animation
		// run. Since we don't know how often we will be called
		// here, try to spread the animator calls uniquely over
		// the [0,1] parameter range. Be aware of the fact that
		// perform will be called at least mnMinNumberOfTurns
		// times.

		// fraction of time elapsed
		const nFractionElapsedTime = nCurrElapsedTime / this.nMinSimpleDuration;

		// fraction of minimum calls performed
		const nFractionRequiredCalls =
			this.nCurrPerformCalls / this.nMinNumberOfFrames;

		// okay, so now, the decision is easy:
		//
		// If the fraction of time elapsed is smaller than the
		// number of calls required to be performed, then we calc
		// the position on the animation range according to
		// elapsed time. That is, we're so to say ahead of time.
		//
		// In contrary, if the fraction of time elapsed is larger,
		// then we're lagging, and we thus calc the position on
		// the animation time line according to the fraction of
		// calls performed. Thus, the animation is forced to slow
		// down, and take the required minimal number of steps,
		// sufficiently equally distributed across the animation
		// time line.

		if (nFractionElapsedTime < nFractionRequiredCalls) {
			return 0.0;
		} else {
			// lag global time, so all other animations lag, too:
			return (
				(nFractionElapsedTime - nFractionRequiredCalls) *
				this.nMinSimpleDuration
			);
		}
	}

	public perform() {
		// call base class, for start() calls and end handling
		if (!super.perform()) return false; // done, we're ended

		// get relative animation position
		const nCurrElapsedTime = this.aTimer.getElapsedTime();
		let nT = nCurrElapsedTime / this.nMinSimpleDuration;

		// one of the stop criteria reached?

		// will be set to true below, if one of the termination criteria matched.
		let bActivityEnding = false;

		if (this.isRepeatCountValid()) {
			// Finite duration case

			// When we've autoreverse on, the repeat count doubles
			const nRepeatCount = this.getRepeatCount();
			const nEffectiveRepeat = this.isAutoReverse()
				? 2.0 * nRepeatCount
				: nRepeatCount;

			// time (or frame count) elapsed?
			if (nEffectiveRepeat <= nT) {
				// Ok done for now. Will not exit right here,
				// to give animation the chance to render the last
				// frame below
				bActivityEnding = true;

				// clamp animation to max permissible value
				nT = nEffectiveRepeat;
			}
		}

		// need to do auto-reverse?

		let nRepeats;
		let nRelativeSimpleTime;
		// TODO(Q3): Refactor this mess
		if (this.isAutoReverse()) {
			// divert active duration into repeat and
			// fractional part.
			nRepeats = Math.floor(nT);
			const nFractionalActiveDuration = nT - nRepeats;

			// for auto-reverse, map ranges [1,2), [3,4), ...
			// to ranges [0,1), [1,2), etc.
			if (nRepeats % 2) {
				// we're in an odd range, reverse sweep
				nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;
			} else {
				// we're in an even range, pass on as is
				nRelativeSimpleTime = nFractionalActiveDuration;
			}

			// effective repeat count for autoreverse is half of
			// the input time's value (each run of an autoreverse
			// cycle is half of a repeat)
			nRepeats /= 2;
		} else {
			// determine repeat

			// calc simple time and number of repeats from nT
			// Now, that's easy, since the fractional part of
			// nT gives the relative simple time, and the
			// integer part the number of full repeats:
			nRepeats = Math.floor(nT);
			nRelativeSimpleTime = nT - nRepeats;

			// clamp repeats to max permissible value (maRepeats.getValue() - 1.0)
			if (this.isRepeatCountValid() && nRepeats >= this.getRepeatCount()) {
				// Note that this code here only gets
				// triggered if this.nRepeats is an
				// _integer_. Otherwise, nRepeats will never
				// reach nor exceed
				// maRepeats.getValue(). Thus, the code below
				// does not need to handle cases of fractional
				// repeats, and can always assume that a full
				// animation run has ended (with
				// nRelativeSimpleTime = 1.0 for
				// non-autoreversed activities).

				// with modf, nRelativeSimpleTime will never
				// become 1.0, since nRepeats is incremented and
				// nRelativeSimpleTime set to 0.0 then.
				//
				// For the animation to reach its final value,
				// nRepeats must although become this.nRepeats - 1.0,
				// and nRelativeSimpleTime = 1.0.
				nRelativeSimpleTime = 1.0;
				nRepeats -= 1.0;
			}
		}

		// actually perform something

		this.simplePerform(nRelativeSimpleTime, nRepeats);

		// delayed endActivity() call from end condition check
		// below. Issued after the simplePerform() call above, to
		// give animations the chance to correctly reach the
		// animation end value, without spurious bail-outs because
		// of isActive() returning false.
		if (bActivityEnding) this.endActivity();

		// one more frame successfully performed
		++this.nCurrPerformCalls;

		return this.isActive();
	}

	protected simplePerform(nSimpleTime: number, nRepeatCount: number): void {
		// empty body
	}
}
