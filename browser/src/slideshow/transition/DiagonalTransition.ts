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

enum DiagonalSubType {
	BOTTOMLEFT,
	TOPLEFT,
	TOPRIGHT,
	BOTTOMRIGHT,
}

class DiagonalTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (
			transitionSubType == TransitionSubType.HORIZONTALRIGHT &&
			!this.transitionFilterInfo.isDirectionForward
		) {
			this.direction = DiagonalSubType.BOTTOMLEFT;
		} else if (
			transitionSubType == TransitionSubType.HORIZONTALLEFT &&
			!this.transitionFilterInfo.isDirectionForward
		) {
			this.direction = DiagonalSubType.BOTTOMRIGHT;
		} else if (transitionSubType == TransitionSubType.HORIZONTALRIGHT) {
			this.direction = DiagonalSubType.TOPRIGHT;
		} else {
			this.direction = DiagonalSubType.TOPLEFT;
		}
	}

	// jscpd:ignore-start
	protected getMaskFunction(): string {
		const startsFromRight =
			this.direction == DiagonalSubType.TOPRIGHT ||
			this.direction == DiagonalSubType.BOTTOMRIGHT;
		const startsFromBottom =
			this.direction == DiagonalSubType.BOTTOMLEFT ||
			this.direction == DiagonalSubType.BOTTOMRIGHT;

		return `
                float getMaskValue(vec2 uv, float time) {
                    float u_steps = 10.0f;

                    float xCoord = ${startsFromRight ? '1.0f - uv.x' : 'uv.x'};
                    float yCoord = floor(uv.y * u_steps) / u_steps;
                    float yStep = ${startsFromBottom ? '1.0f - yCoord' : 'yCoord'};

                    float stepDelay = 1.0f;
                    float adjustedTime = time * 2.0f - yStep * stepDelay;

                    float stepWidth = 1.0f;
                    float mask = step(xCoord * stepWidth, adjustedTime);

                    return mask;
                }
		`;
	}
	// jscpd:ignore-end
}

SlideShow.DiagonalTransition = DiagonalTransition;
