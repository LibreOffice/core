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

class CutTransition extends SlideShow.Transition2d {
	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	public getFragmentShader(): string {
		return `#version 300 es
                precision mediump float;

                uniform sampler2D leavingSlideTexture;
                uniform sampler2D enteringSlideTexture;
                uniform float time;

                in vec2 v_texCoord;
                out vec4 outColor;

                void main() {
                    if (time < 1.0) {
                        outColor =  vec4(0.0f, 0.0f, 0.0f, 1.0f);
                    } else {
                        outColor = texture(enteringSlideTexture, v_texCoord);
                    }
                }
                `;
	}
}

SlideShow.CutTransition = CutTransition;
