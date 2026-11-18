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

enum CheckersSubType {
	ACROSS,
	DOWN,
}

class CheckersTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (transitionSubType == TransitionSubType.DOWN) {
			this.direction = CheckersSubType.DOWN;
		} else if (transitionSubType == TransitionSubType.ACROSS) {
			this.direction = CheckersSubType.ACROSS;
		}
	}

	// jscpd:ignore-start
	protected getMaskFunction(): string {
		const numSquares = 8;
		const edgeSize = 1.0 / numSquares;
		const isAcrossDir = this.direction == CheckersSubType.ACROSS;

		return `
                float getMaskValue(vec2 uv, float time) {
                    float progress = time * 2.0;

                    vec2 squareSize = vec2(${edgeSize}, ${edgeSize});
                    vec2 checkerCoord = floor(uv / squareSize);

                    bool isWhiteSquare = mod(checkerCoord.x + checkerCoord.y, 2.0) == 0.0;
                    vec2 localUV = fract(uv / squareSize);

                    float adjustedProgress = isWhiteSquare ? progress : progress - 1.0;
                    adjustedProgress = clamp(adjustedProgress, 0.0, 1.0);

                    float localCoord = ${isAcrossDir ? 'localUV.x' : 'localUV.y'};
                    bool showEntering = localCoord < adjustedProgress;

                    return float(showEntering);
                }
		`;
	}
	// jscpd:ignore-end
}

SlideShow.CheckersTransition = CheckersTransition;
