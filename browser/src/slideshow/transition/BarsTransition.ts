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

enum BarsSubType {
	VERTICAL,
	HORIZONTAL,
}

class BarsTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;
		if (transitionSubType == TransitionSubType.VERTICAL) {
			this.direction = BarsSubType.VERTICAL;
		} else if (transitionSubType == TransitionSubType.HORIZONTAL) {
			this.direction = BarsSubType.HORIZONTAL;
		}
	}

	// jscpd:ignore-start
	protected getMaskFunction(): string {
		const isVerticalDir = this.direction == BarsSubType.VERTICAL;

		return `
                float random(vec2 co) {
                    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
                }

                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    float numBars = 128.0;
                    float coord = ${isVerticalDir ? 'uv.x' : 'uv.y'};
                    float randValue = random(vec2(floor(coord * numBars), 0.0));

                    bool showEntering = (randValue < progress);
                    return float(showEntering);
                }
		`;
	}
	// jscpd:ignore-end
}

SlideShow.BarsTransition = BarsTransition;
