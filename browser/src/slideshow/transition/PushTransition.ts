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

enum PushSubType {
	FROMBOTTOM,
	FROMLEFT,
	FROMRIGHT,
	FROMTOP,
}

class PushTransition extends Transition2d {
	private direction: number = 0;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	public start(): void {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (transitionSubType == TransitionSubType.FROMTOP) {
			this.direction = PushSubType.FROMTOP;
		} else if (transitionSubType == TransitionSubType.FROMRIGHT) {
			this.direction = PushSubType.FROMRIGHT;
		} else if (transitionSubType == TransitionSubType.FROMLEFT) {
			this.direction = PushSubType.FROMLEFT;
		} else {
			this.direction = PushSubType.FROMBOTTOM;
		}
		this.startTransition();
	}

	// jscpd:ignore-start
	public renderUniformValue(): void {
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'direction'),
			this.direction,
		);
	}

	public getFragmentShader(): string {
		return `#version 300 es
                precision mediump float;

                uniform sampler2D leavingSlideTexture;
                uniform sampler2D enteringSlideTexture;
                uniform float time;
                uniform int direction;

                in vec2 v_texCoord;
                out vec4 outColor;

                void main() {
                    vec2 uv = v_texCoord;
                    float progress = time;

                    vec2 leavingUV = uv;
                    vec2 enteringUV = uv;

                    if (direction == 0) {
                        // bottom to top
                        leavingUV = uv + vec2(0.0, progress);
                        enteringUV = uv + vec2(0.0, -1.0 + progress);
                    } else if (direction == 1) {
                        // left to right
                        leavingUV = uv + vec2(-progress, 0.0);
                        enteringUV = uv + vec2(1.0 - progress, 0.0);
                    } else if (direction == 2) {
                        // right to left
                        leavingUV = uv + vec2(progress, 0.0);
                        enteringUV = uv + vec2(-1.0 + progress, 0.0);
                    } else if (direction == 3) {
                        // top to bottom
                        leavingUV = uv + vec2(0.0, -progress);
                        enteringUV = uv + vec2(0.0, 1.0 - progress);
                    }

                    if ((direction == 0 && uv.y > 1.0 - progress) ||
                        (direction == 1 && uv.x < progress) ||
                        (direction == 2 && uv.x > 1.0 - progress) ||
                        (direction == 3 && uv.y < progress)) {
                        outColor = texture(enteringSlideTexture, enteringUV);
                    } else {
                        outColor = texture(leavingSlideTexture, leavingUV);
                    }
                }
                `;
	}
	// jscpd:ignore-end
}

SlideShow.PushTransition = PushTransition;
