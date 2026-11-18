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

enum WheelSubType {
	ONEWHEEL = 1,
	TWOWHEEL = 2,
	THREEWHEEL = 3,
	FOURWHEEL = 4,
	EIGHTWHEEL = 8,
}

class WheelTransition extends ClippingTransition {
	private stocks: number;
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (transitionSubType == TransitionSubType.TWOBLADEVERTICAL) {
			this.stocks = WheelSubType.TWOWHEEL;
		} else if (transitionSubType == TransitionSubType.THREEBLADE) {
			this.stocks = WheelSubType.THREEWHEEL;
		} else if (transitionSubType == TransitionSubType.FOURBLADE) {
			this.stocks = WheelSubType.FOURWHEEL;
		} else if (transitionSubType == TransitionSubType.EIGHTBLADE) {
			this.stocks = WheelSubType.EIGHTWHEEL;
		} else {
			this.stocks = WheelSubType.ONEWHEEL;
		}
	}

	protected getMaskFunction(): string {
		const slice = (2.0 * Math.PI) / this.stocks;
		return `
                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    float angle = atan(uv.y - 0.5, uv.x - 0.5) + ${Math.PI / 2};
                    float mask = step(mod(angle, ${slice}), ${slice} * progress);

                    return mask;
                }
		`;
	}
}

SlideShow.WheelTransition = WheelTransition;
