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

abstract class ActivityBase extends AnimationActivity {
	private aTargetElement: null;
	private aEndEvent: DelayEvent;
	private aTimerEventQueue: TimerEventQueue;
	private nRepeats: number;
	private nAccelerationFraction: number;
	private nDecelerationFraction: number;
	private bAutoReverse: boolean;
	private bFirstPerformCall: boolean;
	private bIsActive: boolean;

	protected constructor(aCommonParamSet: ActivityParamSet) {
		super();

		this.aTargetElement = null;
		this.aEndEvent = aCommonParamSet.aEndEvent;
		this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;
		this.nRepeats = aCommonParamSet.nRepeatCount;
		this.nAccelerationFraction = aCommonParamSet.nAccelerationFraction;
		this.nDecelerationFraction = aCommonParamSet.nDecelerationFraction;
		this.bAutoReverse = aCommonParamSet.bAutoReverse;

		this.bFirstPerformCall = true;
		this.bIsActive = true;
	}

	public activate(aEndEvent: DelayEvent) {
		this.aEndEvent = aEndEvent;
		this.bFirstPerformCall = true;
		this.bIsActive = true;
	}

	public dispose() {
		// deactivate
		this.bIsActive = false;

		// dispose event
		if (this.aEndEvent) this.aEndEvent.dispose();

		this.aEndEvent = null;
	}

	public perform() {
		// still active?
		if (!this.isActive()) return false; // no, early exit.

		assert(
			!this.bFirstPerformCall,
			'ActivityBase.perform: assertion (!this.FirstPerformCall) failed',
		);

		return true;
	}

	public calcTimeLag() {
		// TODO: implement different init process!
		if (this.isActive() && this.bFirstPerformCall) {
			this.bFirstPerformCall = false;

			// notify derived classes that we're
			// starting now
			this.startAnimation();
		}
		return 0.0;
	}

	public isActive() {
		return this.bIsActive;
	}

	public isDisposed() {
		return !this.bIsActive && !this.aEndEvent;
	}

	public dequeued() {
		if (!this.isActive()) this.endAnimation();
	}

	public setTargets(aTargetElement: any) {
		assert(
			aTargetElement,
			'ActivityBase.setTargets: target element is not valid',
		);

		this.aTargetElement = aTargetElement;
	}

	// public abstract startAnimation(): void;
	public startAnimation(): void {
		throw 'ActivityBase.startAnimation: abstract method invoked';
	}

	public abstract endAnimation(): void;
	// public endAnimation(): void {
	// 	// TODO throw abstract
	// }

	public endActivity() {
		// this is a regular activity end
		this.bIsActive = false;

		// Activity is ending, queue event, then
		if (this.aEndEvent) this.aTimerEventQueue.addEvent(this.aEndEvent);

		this.aEndEvent = null;
	}

	public calcAcceleratedTime(nT: number) {
		// Handle acceleration/deceleration

		// clamp nT to permissible [0,1] range
		nT = clampN(nT, 0.0, 1.0);

		// take acceleration/deceleration into account. if the sum
		// of nAccelerationFraction and nDecelerationFraction
		// exceeds 1.0, ignore both (that's according to SMIL spec)
		if (
			(this.nAccelerationFraction > 0.0 || this.nDecelerationFraction > 0.0) &&
			this.nAccelerationFraction + this.nDecelerationFraction <= 1.0
		) {
			const nC =
				1.0 -
				0.5 * this.nAccelerationFraction -
				0.5 * this.nDecelerationFraction;

			// this variable accumulates the new time value
			let nTPrime = 0.0;

			if (nT < this.nAccelerationFraction) {
				nTPrime += (0.5 * nT * nT) / this.nAccelerationFraction; // partial first interval
			} else {
				nTPrime += 0.5 * this.nAccelerationFraction; // full first interval

				if (nT <= 1.0 - this.nDecelerationFraction) {
					nTPrime += nT - this.nAccelerationFraction; // partial second interval
				} else {
					nTPrime +=
						1.0 - this.nAccelerationFraction - this.nDecelerationFraction; // full second interval

					const nTRelative = nT - 1.0 + this.nDecelerationFraction;

					nTPrime +=
						nTRelative -
						(0.5 * nTRelative * nTRelative) / this.nDecelerationFraction;
				}
			}
			// normalize, and assign to work variable
			nT = nTPrime / nC;
		}
		return nT;
	}

	public getEventQueue() {
		return this.aTimerEventQueue;
	}

	public getTargetElement() {
		return this.aTargetElement;
	}

	public isRepeatCountValid() {
		return !!this.nRepeats; // first ! convert to bool
	}

	public getRepeatCount() {
		return this.nRepeats;
	}

	public isAutoReverse() {
		return this.bAutoReverse;
	}

	public end() {
		if (!this.isActive() || this.isDisposed()) return;

		// assure animation is started:
		if (this.bFirstPerformCall) {
			this.bFirstPerformCall = false;
			// notify derived classes that we're starting now
			this.startAnimation();
		}

		this.performEnd();
		this.endAnimation();
		this.endActivity();
	}

	public abstract performEnd(): void;
	// public performEnd(): void {
	// 	// TODO throw abstract
	// }
}
