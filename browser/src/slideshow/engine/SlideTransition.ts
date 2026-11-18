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

class SlideTransition {
	slideInfo: SlideInfo;
	aDuration: Duration;
	nMinFrameCount: number;
	bIsValid: boolean;

	constructor(slideInfo: SlideInfo) {
		this.slideInfo = slideInfo;
		this.bIsValid =
			this.getType() !== undefined && this.getType() !== TransitionType.INVALID;
		if (this.slideInfo.transitionSubtype === undefined)
			this.slideInfo.transitionSubtype = 'Default';
		this.aDuration = new Duration(this.slideInfo.transitionDuration + 'ms');
		if (!this.aDuration.isSet()) {
			this.aDuration = new Duration(null); // duration == 0.0
		}

		// set up min frame count value;
		this.nMinFrameCount = this.getDuration().isValue()
			? this.getDuration().getValue() *
				SlideShowHandler.MINIMUM_FRAMES_PER_SECONDS
			: SlideShowHandler.MINIMUM_FRAMES_PER_SECONDS;
		if (this.nMinFrameCount < 1.0) this.nMinFrameCount = 1;
		else if (this.nMinFrameCount > SlideShowHandler.MINIMUM_FRAMES_PER_SECONDS)
			this.nMinFrameCount = SlideShowHandler.MINIMUM_FRAMES_PER_SECONDS;
	}

	isValid() {
		return this.bIsValid;
	}

	createSlideTransition(
		transitionParameters: TransitionParameters,
	): ISlideChangeBase {
		if (!this.isValid()) return null;
		return createTransition(transitionParameters, /*isSlideTransition*/ true);
	}

	public getType() {
		return stringToTransitionTypeMap[this.slideInfo.transitionType];
	}

	public getSubType() {
		return stringToTransitionSubTypeMap[this.slideInfo.transitionSubtype];
	}

	public getFadeColor(): string {
		return this.slideInfo.transitionFadeColor || '#000000';
	}

	public isDirectionForward() {
		return this.slideInfo.transitionDirection;
	}

	public getDuration() {
		return this.aDuration;
	}

	public getMinFrameCount(): number {
		return this.nMinFrameCount;
	}
}
