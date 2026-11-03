/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

declare var SlideShow: any;

enum FadeSubType {
	FADEOVERBLACK,
	FADEOVERWHITE,
	SMOOTHLY,
}

class FadeTransition extends SlideShow.Transition2d {
	private effectTransition: number = 0;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	public start(): void {
		const transitionSubType = this.transitionFilterInfo.transitionSubtype;

		this.effectTransition = FadeSubType.FADEOVERBLACK; // default

		// A morph plays as a smooth fade from the old slide to the new one.
		if (
			transitionSubType == TransitionSubType.CROSSFADE ||
			transitionSubType == TransitionSubType.MORPH ||
			transitionSubType == TransitionSubType.MORPHBYWORD ||
			transitionSubType == TransitionSubType.MORPHBYCHARACTER
		) {
			this.effectTransition = FadeSubType.SMOOTHLY;
		} else if (
			transitionSubType == TransitionSubType.FADEOVERCOLOR &&
			this.transitionFilterInfo.fadeColor &&
			this.transitionFilterInfo.fadeColor.toUpperCase() === '#FFFFFF'
		) {
			this.effectTransition = FadeSubType.FADEOVERWHITE;
		}

		this.startTransition();
	}

	public renderUniformValue(): void {
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'effectType'),
			this.effectTransition,
		);
	}

	public getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D leavingSlideTexture;
				uniform sampler2D enteringSlideTexture;
				uniform float time;
				uniform int effectType; // 0: Fade through black, 1: Fade through white, 2: Smooth fade

				in vec2 v_texCoord;
				out vec4 outColor;

				void main() {
					vec4 color0 = texture(leavingSlideTexture, v_texCoord);
					vec4 color1 = texture(enteringSlideTexture, v_texCoord);
					vec4 transitionColor;

					if (effectType == 0) {
						// Fade through black
						transitionColor = vec4(0.0, 0.0, 0.0, 1.0);
					} else if (effectType == 1) {
						// Fade through white
						transitionColor = vec4(1.0, 1.0, 1.0, 1.0);
					}
					if (effectType == 2) {
						// Smooth fade
						float smoothTime = smoothstep(0.0, 1.0, time);
						outColor = mix(color0, color1, smoothTime);
					} else {
						if (time < 0.5) {
							outColor = mix(color0, transitionColor, time * 2.0);
						} else {
							outColor = mix(transitionColor, color1, (time - 0.5) * 2.0);
						}
					}
				}
				`;
	}
}

SlideShow.FadeTransition = FadeTransition;
