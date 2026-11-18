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

class StaticNoiseTransitionImp extends PermTextureTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D leavingSlideTexture;
				uniform sampler2D enteringSlideTexture;
				uniform sampler2D permTexture;
				uniform float time;

				in vec2 v_texturePosition;
				in vec3 v_normal;
				out vec4 outColor;

				float snoise(vec2 P) {
					return texture(permTexture, P).r;
				}

				#define PART 0.5
				#define START 0.4
				#define END 0.9

				void main() {
					float sn = snoise(10.0 * v_texturePosition + time * 0.07);
					
					if (time < PART) {
						float sn1 = snoise(vec2(time * 15.0, 20.0 * v_texturePosition.y));
						float sn2 = snoise(v_texturePosition);
						
						if (sn1 > 1.0 - time * time && sn2 < 2.0 * time + 0.1)
							outColor = vec4(sn, sn, sn, 1.0);
						else if (time > START)
							outColor = mix(texture(leavingSlideTexture, v_texturePosition), vec4(sn, sn, sn, 1.0), (time - START) / (PART - START));
						else
							outColor = texture(leavingSlideTexture, v_texturePosition);
					} else if (time > END) {
						outColor = mix(vec4(sn, sn, sn, 1.0), texture(enteringSlideTexture, v_texturePosition), (time - END) / (1.0 - END));
					} else {
						outColor = vec4(sn, sn, sn, 1.0);
					}
				}
				`;
	}
}

function StaticNoiseTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();
	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [];
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	slide.operations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, -1),
			90,
			false,
			false,
			0.0,
			1.0,
		),
	);

	const aEnteringPrimitives: Primitive[] = [];
	aEnteringPrimitives.push(slide);

	const aOperations: Operation[] = [];
	aOperations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, -1),
			-90,
			false,
			true,
			0.0,
			1.0,
		),
	);

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: aOperations,
	};

	return new StaticNoiseTransitionImp(newTransitionParameters);
}

SlideShow.StaticNoiseTransition = StaticNoiseTransition;
