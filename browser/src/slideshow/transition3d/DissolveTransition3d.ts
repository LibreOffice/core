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

class DissolveTransition3dImp extends PermTextureTransition {
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

                void main() {
                    float sn = snoise(10.0*v_texturePosition);
                    if( sn < time)
                        outColor = texture(enteringSlideTexture, v_texturePosition);
                    else
                        outColor = texture(leavingSlideTexture, v_texturePosition);
                }
                `;
	}
}
// TODO - remove code duplication
/* jscpd:ignore-start */
function DissolveTransition3d(transitionParameters: TransitionParameters) {
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

	return new DissolveTransition3dImp(newTransitionParameters);
}
/* jscpd:ignore-end */

SlideShow.DissolveTransition3d = DissolveTransition3d;
