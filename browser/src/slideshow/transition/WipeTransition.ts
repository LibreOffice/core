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

declare var SlideShow: any;

enum WipeSubType {
	LEFTTORIGHT,
	RIGHTTOLEFT,
	TOPTOBOTTOM,
	BOTTOMTOTOP,
}

class WipeTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;
		if (
			transitionSubType == TransitionSubType.TOPTOBOTTOM &&
			this.transitionFilterInfo.isDirectionForward
		) {
			this.direction = WipeSubType.TOPTOBOTTOM;
		} else if (
			transitionSubType == TransitionSubType.TOPTOBOTTOM &&
			!this.transitionFilterInfo.isDirectionForward
		) {
			this.direction = WipeSubType.BOTTOMTOTOP;
		} else if (
			transitionSubType == TransitionSubType.LEFTTORIGHT &&
			this.transitionFilterInfo.isDirectionForward
		) {
			this.direction = WipeSubType.LEFTTORIGHT;
		} else {
			this.direction = WipeSubType.RIGHTTOLEFT;
		}
	}

	// jscpd:ignore-start
	protected getMaskFunction(): string {
		const isHorizontalDir =
			this.direction == WipeSubType.LEFTTORIGHT ||
			this.direction == WipeSubType.RIGHTTOLEFT;
		const isBackwardDir =
			this.direction == WipeSubType.RIGHTTOLEFT ||
			this.direction == WipeSubType.BOTTOMTOTOP;

		return `
        float getMaskValue(vec2 uv, float time) {
            float progress = time;

            float coord = ${isHorizontalDir ? 'uv.x' : 'uv.y'};
            ${isBackwardDir ? 'coord = 1.0 - coord;' : ''}
            float mask = step(coord, progress);
            return mask;
        }
		`;
	}
	// jscpd:ignore-end
}

SlideShow.WipeTransition = WipeTransition;
