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

class SimpleDissolveTransition extends ClippingTransition {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected getMaskFunction(): string {
		const numSquares = 16;
		const edgeSize = 1.0 / numSquares;
		return `
				// generate a pseudo-random value based on coordinates
				float random(vec2 co) {
					return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
				}

				float getMaskValue(vec2 uv, float time) {
					float progress = time;

					vec2 squareSize = vec2(${edgeSize}, ${edgeSize});

					vec2 checkerCoord = floor(uv / squareSize);

					float randValue = random(checkerCoord);

					bool showEntering = progress > randValue;

					return float(showEntering);
				}
		`;
	}
}
SlideShow.SimpleDissolveTransition = SimpleDissolveTransition;
