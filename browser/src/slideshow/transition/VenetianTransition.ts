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

enum VenetianSubType {
	HORIZONTAL,
	VERTICAL,
}

class VenetianTransition extends ClippingTransition {
	private direction: number;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	protected initMaskFunctionParams() {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (transitionSubType == TransitionSubType.HORIZONTAL) {
			this.direction = VenetianSubType.HORIZONTAL;
		} else {
			this.direction = VenetianSubType.VERTICAL;
		}
	}

	protected getMaskFunction(): string {
		const numBlinds = 6.0;
		const blindWidth = this.gl.canvas.width / numBlinds;
		const blindHeight = this.gl.canvas.height / numBlinds;
		const blindSize =
			this.direction == VenetianSubType.VERTICAL ? blindWidth : blindHeight;

		const comp: string = this.direction == VenetianSubType.VERTICAL ? 'x' : 'y';

		return `
                #define CANVAS_WIDTH ${this.gl.canvas.width}
                #define CANVAS_HEIGHT ${this.gl.canvas.height}

                float getMaskValue(vec2 uv, float time) {
                    float progress = time;

                    vec2 resolution = vec2(CANVAS_WIDTH, CANVAS_HEIGHT);
                    float blindSize = float(${blindSize});

                    float position = mod(uv.${comp} * resolution.${comp}, blindSize);

                    float mask = step(position / blindSize, progress);

                    return mask;
                }
		`;
	}
}

SlideShow.VenetianTransition = VenetianTransition;
