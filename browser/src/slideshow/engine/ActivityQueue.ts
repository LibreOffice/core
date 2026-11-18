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

class ActivityQueue {
	private aTimer: ElapsedTime;
	private aCurrentActivityWaitingSet: AnimationActivity[];
	private aCurrentActivityReinsertSet: AnimationActivity[];
	private aDequeuedActivitySet: AnimationActivity[];

	constructor(aTimer: ElapsedTime) {
		this.aTimer = aTimer;
		this.aCurrentActivityWaitingSet = [];
		this.aCurrentActivityReinsertSet = [];
		this.aDequeuedActivitySet = [];
	}

	addActivity(aActivity: AnimationActivity) {
		if (!aActivity) {
			window.app.console.log(
				'ActivityQueue.addActivity: activity is not valid',
			);
			return false;
		}

		this.aCurrentActivityWaitingSet.push(aActivity);
		aActivityQueueDebugPrinter.print(
			'ActivityQueue.addActivity: activity appended',
		);
		return true;
	}

	process() {
		const nSize = this.aCurrentActivityWaitingSet.length;
		let nLag = 0.0;
		for (let i = 0; i < nSize; ++i) {
			nLag = Math.max(nLag, this.aCurrentActivityWaitingSet[i].calcTimeLag());
		}

		if (nLag > 0.0) this.aTimer.adjustTimer(-nLag);

		while (this.aCurrentActivityWaitingSet.length != 0) {
			const aActivity = this.aCurrentActivityWaitingSet.shift();
			let bReinsert = false;

			bReinsert = aActivity.perform();

			if (bReinsert) {
				this.aCurrentActivityReinsertSet.push(aActivity);
			} else {
				this.aDequeuedActivitySet.push(aActivity);
			}
		}

		if (this.aCurrentActivityReinsertSet.length != 0) {
			// TODO: optimization, try to swap reference here
			this.aCurrentActivityWaitingSet = this.aCurrentActivityReinsertSet;
			this.aCurrentActivityReinsertSet = [];
		}
	}

	processDequeued() {
		// notify all dequeued activities from last round
		const nSize = this.aDequeuedActivitySet.length;
		for (let i = 0; i < nSize; ++i) this.aDequeuedActivitySet[i].dequeued();

		this.aDequeuedActivitySet = [];
	}

	isEmpty() {
		return (
			this.aCurrentActivityWaitingSet.length == 0 &&
			this.aCurrentActivityReinsertSet.length == 0
		);
	}

	clear() {
		aActivityQueueDebugPrinter.print('ActivityQueue.clear invoked');
		let nSize = this.aCurrentActivityWaitingSet.length;
		for (let i = 0; i < nSize; ++i)
			this.aCurrentActivityWaitingSet[i].dequeued();
		this.aCurrentActivityWaitingSet = [];

		nSize = this.aCurrentActivityReinsertSet.length;
		for (let i = 0; i < nSize; ++i)
			this.aCurrentActivityReinsertSet[i].dequeued();
		this.aCurrentActivityReinsertSet = [];
	}

	endAll() {
		aActivityQueueDebugPrinter.print('ActivityQueue.endAll invoked');
		let nSize = this.aCurrentActivityWaitingSet.length;
		for (let i = 0; i < nSize; ++i) this.aCurrentActivityWaitingSet[i].end();
		this.aCurrentActivityWaitingSet = [];

		nSize = this.aCurrentActivityReinsertSet.length;
		for (let i = 0; i < nSize; ++i) this.aCurrentActivityReinsertSet[i].end();
		this.aCurrentActivityReinsertSet = [];
	}

	getTimer() {
		return this.aTimer;
	}

	size() {
		return (
			this.aCurrentActivityWaitingSet.length +
			this.aCurrentActivityReinsertSet.length +
			this.aDequeuedActivitySet.length
		);
	}
}
