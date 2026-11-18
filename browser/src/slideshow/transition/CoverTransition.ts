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

enum CoverSubType {
	FROMBOTTOM,
	FROMLEFT,
	FROMRIGHT,
	FROMTOP,
}

class CoverTransition extends Transition2d {
	private direction: number = 0;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	public start(): void {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		if (transitionSubType == TransitionSubType.FROMTOP) {
			this.direction = CoverSubType.FROMTOP;
		} else if (transitionSubType == TransitionSubType.FROMLEFT) {
			this.direction = CoverSubType.FROMLEFT;
		} else if (transitionSubType == TransitionSubType.FROMRIGHT) {
			this.direction = CoverSubType.FROMRIGHT;
		} else {
			this.direction = CoverSubType.FROMBOTTOM;
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
                        enteringUV = uv + vec2(0.0, -1.0 + progress);
                    } else if (direction == 1) {
                        enteringUV = uv + vec2(1.0 - progress, 0.0);
                    } else if (direction == 2) {
                        enteringUV = uv + vec2(-1.0 + progress, 0.0);
                    } else if (direction == 3) {
                        enteringUV = uv + vec2(0.0, 1.0 - progress);
                    } else if (direction == 4) {
                        // bottom left to top right
                        enteringUV = uv + vec2(1.0 - progress, -1.0 + progress);
                    } else if (direction == 5) {
                        // top right to bottom left
                        enteringUV = uv + vec2(-1.0 + progress, 1.0 - progress);
                    } else if (direction == 6) {
                        // top left to bottom right
                        enteringUV = uv + vec2(1.0 - progress, 1.0 - progress);
                    } else if (direction == 7) {
                        // bottom right to top left
                        enteringUV = uv + vec2(-1.0 + progress, -1.0 + progress);
                    }

                    bool showEntering = false;
                    if (direction == 0) {
                        showEntering = uv.y > 1.0 - progress;
                    } else if (direction == 1) {
                        showEntering = uv.x < progress;
                    } else if (direction == 2) {
                        showEntering = uv.x > 1.0 - progress;
                    } else if (direction == 3) {
                        showEntering = uv.y < progress;
                    } else if (direction == 4) {
                        showEntering = uv.x < progress && uv.y > 1.0 - progress;
                    } else if (direction == 5) {
                        showEntering = uv.x > 1.0 - progress && uv.y < progress;
                    } else if (direction == 6) {
                        showEntering = uv.x < progress && uv.y < progress;
                    } else if (direction == 7) {
                        showEntering = uv.x > 1.0 - progress && uv.y > 1.0 - progress;
                    }

                    if (showEntering) {
                        outColor = texture(enteringSlideTexture, enteringUV);
                    } else {
                        outColor = texture(leavingSlideTexture, leavingUV);
                    }
                }
                `;
	}
	// jscpd:ignore-end
}

SlideShow.CoverTransition = CoverTransition;
