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

class InteractiveAnimationSequence {
	private nId: number;
	private aSlideShow: SlideShowHandler;
	private bIsRunning = false;
	private aStartEvent: DelayEvent = null;
	private aEndEvent: DelayEvent = null;

	constructor(nNodeId: number, aSlideShow: SlideShowHandler) {
		this.nId = nNodeId;
		this.aSlideShow = aSlideShow;
		if (!this.aSlideShow) {
			window.app.console.log(
				'InteractiveAnimationSequence.constructor: invalid slide show handler',
			);
		}
	}

	getId() {
		return this.nId;
	}

	getStartEvent() {
		if (!this.aStartEvent) {
			this.aStartEvent = makeEvent(this.start.bind(this));
		}
		return this.aStartEvent;
	}

	getEndEvent() {
		if (!this.aEndEvent) {
			this.aEndEvent = makeEvent(this.end.bind(this));
		}
		return this.aEndEvent;
	}

	chargeEvents() {
		if (this.aStartEvent) this.aStartEvent.charge();
		if (this.aEndEvent) this.aEndEvent.charge();
	}

	isRunning() {
		return this.bIsRunning;
	}

	start() {
		this.aSlideShow.notifyInteractiveAnimationSequenceStart(this.getId());
		this.bIsRunning = true;
	}

	end() {
		this.aSlideShow.notifyInteractiveAnimationSequenceEnd(this.getId());
		this.bIsRunning = false;
	}
}
