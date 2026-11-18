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

class PlusTransition extends ClippingTransition {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected getMaskFunction(): string {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;
		if (transitionSubType === TransitionSubType.CORNERSOUT)
			return `
                  float getMaskValue(vec2 uv, float time) {
                      vec2 center = vec2(0.5, 0.5);

                      vec2 dist = abs(uv - center);

                      float innerBound = 0.25 - time / 4.0;
                      float outerBound = 0.25 + time / 4.0;

                      // dist >= innerBound && dist <= outerBound
                      float mask =
                          step(innerBound, dist.x) * step(-outerBound, -dist.x) *
                          step(innerBound, dist.y) * step(-outerBound, -dist.y);

                      return mask;
                  }
          `;
		else if (transitionSubType === TransitionSubType.CORNERSIN)
			return `
                  float getMaskValue(vec2 uv, float time) {
                      vec2 center = vec2(0.5, 0.5);

                      vec2 dist = abs(uv - center);

                      float size = 1.01 * (1.0 - time) / 2.0;

                      float mask = step(size, dist.x) * step(size, dist.y);

                      return mask;
                  }
          `;
	}
}

SlideShow.PlusTransition = PlusTransition;
