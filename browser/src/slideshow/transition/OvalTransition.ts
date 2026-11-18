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

enum OvalSubType {
	HORIZONTAL,
	VERTICAL,
}

class OvalTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}
	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;
		if (transitionSubType == TransitionSubType.HORIZONTAL) {
			this.direction = OvalSubType.HORIZONTAL;
		} else if (transitionSubType == TransitionSubType.VERTICAL) {
			this.direction = OvalSubType.VERTICAL;
		}
	}

	// jscpd:ignore-start
	protected getMaskFunction(): string {
		const isHorizontalDir = this.direction == OvalSubType.HORIZONTAL;

		return `
                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    vec2 center = vec2(0.5, 0.5);

                    vec2 dist = abs(uv - center);

                    ${isHorizontalDir ? 'dist.y' : 'dist.x'} *= 2.0;

                    float size = progress * 1.2;

                    float mask = step(length(dist), size);

                    mask = min(mask, 1.0);

                    return mask;
                }
		`;
	}
	// jscpd:ignore-end
}

SlideShow.OvalTransition = OvalTransition;
