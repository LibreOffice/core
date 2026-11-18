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

class ActivityParamSet {
	aEndEvent: DelayEvent;
	aWakeupEvent: WakeupEvent;
	aTimerEventQueue: TimerEventQueue;
	aActivityQueue: ActivityQueue;
	nMinDuration: number;
	nMinNumberOfFrames: number = SlideShowHandler.MINIMUM_FRAMES_PER_SECONDS;
	bAutoReverse: boolean = false;
	nRepeatCount: number = 1.0;
	nAccelerationFraction: number = 0;
	nDecelerationFraction: number = 0;
	nSlideWidth: number;
	nSlideHeight: number;
	aFormula: (x: any) => any;
	aDiscreteTimes: Array<number> = [];
}

abstract class AnimationActivity {
	private static CURR_UNIQUE_ID = 0;
	private readonly nId: number;

	protected constructor() {
		this.nId = AnimationActivity.getUniqueId();
	}
	private static getUniqueId() {
		++AnimationActivity.CURR_UNIQUE_ID;
		return AnimationActivity.CURR_UNIQUE_ID;
	}

	getId() {
		return this.nId;
	}

	abstract activate(aEvent: EventBase): void;
	abstract dispose(): void;
	abstract calcTimeLag(): number;
	abstract perform(): boolean;
	abstract isActive(): void;
	abstract dequeued(): void;
	abstract end(): void;
	abstract setTargets(aTargetElement: any): void;
}
