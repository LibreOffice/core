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

class WedgeTransition extends ClippingTransition {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected getMaskFunction(): string {
		return `
		            #define M_PI ${Math.PI}

                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    vec2 center = vec2(0.5, 0.5);

                    vec2 dist = uv - center;
                    float angle = atan(dist.x, dist.y);

                    if (angle < 0.0) {
                        angle += 2.0 * M_PI;
                    }

                    float wedgeAngle = M_PI * progress;

                    float mask = step(angle, wedgeAngle) + step(2.0 * M_PI - wedgeAngle, angle);
										mask = min(mask, 1.0);

                    return mask;
                }
		`;
	}
}

SlideShow.WedgeTransition = WedgeTransition;
