// @ts-strict-ignore
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

abstract class DiscreteActivityBase extends ActivityBase {
	private aOriginalWakeupEvent: WakeupEvent;
	private aWakeupEvent: WakeupEvent;
	private aDiscreteTimes: Array<number>;
	protected nMinSimpleDuration: number;
	protected nCurrPerformCalls: number;

	protected constructor(aCommonParamSet: ActivityParamSet) {
		super(aCommonParamSet);

		this.aOriginalWakeupEvent = aCommonParamSet.aWakeupEvent;
		this.aOriginalWakeupEvent.setActivity(this);
		this.aWakeupEvent = this.aOriginalWakeupEvent;
		this.aWakeupEvent = aCommonParamSet.aWakeupEvent;
		this.aDiscreteTimes = aCommonParamSet.aDiscreteTimes;
		// Simple duration of activity
		this.nMinSimpleDuration = aCommonParamSet.nMinDuration;
		// Actual number of frames shown until now.
		this.nCurrPerformCalls = 0;
	}

	public activate(aEndElement: any) {
		super.activate(aEndElement);

		this.aWakeupEvent = this.aOriginalWakeupEvent;
		this.aWakeupEvent.setNextTimeout(0);
		this.nCurrPerformCalls = 0;
	}

	public startAnimation() {
		this.aWakeupEvent.start();
	}

	public calcFrameIndex(nCurrCalls: number, nVectorSize: number) {
		if (this.isAutoReverse()) {
			// every full repeat run consists of one
			// forward and one backward traversal.
			let nFrameIndex = nCurrCalls % (2 * nVectorSize);

			// nFrameIndex values >= nVectorSize belong to
			// the backward traversal
			if (nFrameIndex >= nVectorSize)
				nFrameIndex = 2 * nVectorSize - nFrameIndex; // invert sweep

			return nFrameIndex;
		} else {
			return nCurrCalls % nVectorSize;
		}
	}

	public calcRepeatCount(nCurrCalls: number, nVectorSize: number) {
		if (this.isAutoReverse()) {
			return Math.floor(nCurrCalls / (2 * nVectorSize)); // we've got 2 cycles per repeat
		} else {
			return Math.floor(nCurrCalls / nVectorSize);
		}
	}

	protected abstract performDiscreteHook(
		nFrame: number,
		nRepeatCount: number,
	): void;
	// protected performDiscreteHook(nFrame: number, nRepeatCount: number): void {
	// 	// TODO throw abstract
	// }

	public perform() {
		// call base class, for start() calls and end handling
		if (!super.perform()) return false; // done, we're ended

		const nVectorSize = this.aDiscreteTimes.length;

		const nFrameIndex = this.calcFrameIndex(
			this.nCurrPerformCalls,
			nVectorSize,
		);
		const nRepeatCount = this.calcRepeatCount(
			this.nCurrPerformCalls,
			nVectorSize,
		);
		this.performDiscreteHook(nFrameIndex, nRepeatCount);

		// one more frame successfully performed
		++this.nCurrPerformCalls;

		// calc currently reached repeat count
		let nCurrRepeat = this.nCurrPerformCalls / nVectorSize;

		// if auto-reverse is specified, halve the
		// effective repeat count, since we pass every
		// repeat run twice: once forward, once backward.
		if (this.isAutoReverse()) nCurrRepeat /= 2;

		// schedule next frame, if either repeat is indefinite
		// (repeat forever), or we've not yet reached the requested
		// repeat count
		if (!this.isRepeatCountValid() || nCurrRepeat < this.getRepeatCount()) {
			// add wake-up event to queue (modulo vector size, to cope with repeats).

			// repeat is handled locally, only apply acceleration/deceleration.
			// Scale time vector with simple duration, offset with full repeat
			// times.

			// Note that calcAcceleratedTime() is only applied to the current repeat's value,
			// not to the total resulting time. This is in accordance with the SMIL spec.

			const nFrameIndex = this.calcFrameIndex(
				this.nCurrPerformCalls,
				nVectorSize,
			);
			const nCurrentRepeatTime = this.aDiscreteTimes[nFrameIndex];
			const nRepeatCount = this.calcRepeatCount(
				this.nCurrPerformCalls,
				nVectorSize,
			);
			const nNextTimeout =
				this.nMinSimpleDuration *
				(nRepeatCount + this.calcAcceleratedTime(nCurrentRepeatTime));
			this.aWakeupEvent.setNextTimeout(nNextTimeout);

			this.getEventQueue().addEvent(this.aWakeupEvent);
		} else {
			// release event reference (relation to wake up event is circular!)
			this.aWakeupEvent = null;

			// done with this activity
			this.endActivity();
		}

		return false; // remove from queue, will be added back by the wakeup event.
	}

	public dispose() {
		// dispose event
		if (this.aWakeupEvent) this.aWakeupEvent.dispose();

		// release references
		this.aWakeupEvent = null;

		super.dispose();
	}
}
