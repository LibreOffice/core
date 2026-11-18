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

class RippleTransitionImp extends PermTextureTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public displayPermSlide_(): void {
		const centerUniform = this.gl.getUniformLocation(this.program, 'center');
		this.gl.uniform2fv(centerUniform, [0.5, 0.5]);

		const slideRation = this.gl.getUniformLocation(this.program, 'slideRatio');
		this.gl.uniform1f(slideRation, 1.0);
	}

	public getFragmentShader(): string {
		return `#version 300 es
                precision mediump float;

                #define M_PI 3.1415926535897932384626433832795


                uniform sampler2D leavingSlideTexture;
                uniform sampler2D enteringSlideTexture;
                uniform float time;
                uniform vec2 center;
                uniform float slideRatio;

                in vec2 v_texturePosition;
                in vec3 v_normal;
                out vec4 outColor;

                float betterDistance(vec2 p1, vec2 p2)
                {
                    p1.x *= slideRatio;
                    p2.x *= slideRatio;
                    return distance(p1, p2);
                }

                void main() {
                    const float w = 0.7;
                    const float v = 0.1;

                    float dist = betterDistance(center, v_texturePosition);

                    float t = time * (sqrt(2.0) * (slideRatio < 1.0 ? 1.0 / slideRatio : slideRatio));

                    float mixed = smoothstep(t*w-v, t*w+v, dist);

                    vec2 offset = (v_texturePosition - center) * (sin(dist * 64.0 - time * 16.0) + 0.5) / 32.0;
                    vec2 wavyTexCoord = mix(v_texturePosition + offset, v_texturePosition, time);

                    vec2 pos = mix(wavyTexCoord, v_texturePosition, mixed);

                    vec4 leaving = texture(leavingSlideTexture, pos);
                    vec4 entering = texture(enteringSlideTexture, pos);
                    outColor = mix(entering, leaving, mixed);
                    
                }
`;
	}
}

function RippleTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();

	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [];
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	const aEnteringPrimitives: Primitive[] = [Primitive.cloneDeep(slide)];

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new RippleTransitionImp(newTransitionParameters);
}

SlideShow.RippleTransition = RippleTransition;
