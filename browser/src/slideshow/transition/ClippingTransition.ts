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

class ClippingTransition extends Transition2d {
	private forwardParameterSweep: boolean;
	private invertMask: boolean;
	private scaleIsotropically: boolean;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}
	protected initProgramTemplateParams() {
		this.forwardParameterSweep = true;
		this.invertMask = false;
		this.scaleIsotropically = false;

		const transitionType = this.transitionFilterInfo.transitionType;
		const transitionSubtype = this.transitionFilterInfo.transitionSubtype;
		const isDirectionForward = this.transitionFilterInfo.isDirectionForward;
		const isModeIn = this.transitionFilterInfo.isModeIn;

		const transitionInfo =
			aTransitionInfoTable[transitionType][transitionSubtype];
		if (!transitionInfo) {
			window.app.console.log(
				`ClippingTransition: no transition info found for type: ` +
					`${transitionType} and subtype: ${transitionSubtype}`,
			);
			return;
		}
		this.scaleIsotropically = transitionInfo.scaleIsotropically;

		if (!isDirectionForward) {
			// At present Rotate180, FlipX, FlipY reverse methods are handled
			// case by case in each mask function transition
			switch (transitionInfo.reverseMethod) {
				default:
					window.app.console.log(
						`ClippingTransition: unexpected reverse method for type: ` +
							`${transitionType} and subtype: ${transitionSubtype}`,
					);
					return;
				case TransitionReverseMethod.Ignore:
					break;
				case TransitionReverseMethod.SubtractAndInvert:
					this.forwardParameterSweep = !this.forwardParameterSweep;
					this.invertMask = !this.invertMask;
					break;
				case TransitionReverseMethod.Rotate180:
					// to be handled
					break;
				case TransitionReverseMethod.FlipX:
					// to be handled
					break;
				case TransitionReverseMethod.FlipY:
					// to be handled
					break;
			}
		}

		if (!isModeIn) {
			if (transitionInfo.outInvertsSweep)
				this.forwardParameterSweep = !this.forwardParameterSweep;
			else this.invertMask = !this.invertMask;
		}

		console.debug(`ClippingTransition: 
			type: ${transitionType} 
			subtype: ${transitionSubtype} 
			direction forward: ${isDirectionForward}
			mode in: ${isModeIn}
			reverse method: ${TransitionReverseMethod[transitionInfo.reverseMethod]}
			outInvertsSweep: ${transitionInfo.outInvertsSweep}
			forwardParameterSweep: ${this.forwardParameterSweep}
			invertMask: ${this.invertMask}
			`);
		this.initMaskFunctionParams();
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected initMaskFunctionParams() {}

	protected getMaskFunction(): string {
		return '';
	}

	public getFragmentShader(): string {
		const isSlideTransition = !!this.leavingSlide;

		let scaleFactor = 1.0;
		let isLandscape: boolean;
		if (this.scaleIsotropically) {
			const ctx = this.transitionParameters.context;
			const width = ctx.canvas.width;
			const height = ctx.canvas.height;
			isLandscape = width > height;
			scaleFactor = isLandscape ? height / width : width / height;
		}
		const needScaling = scaleFactor != 1.0;
		const comp = isLandscape ? 'y' : 'x';

		// prettier-ignore
		return `#version 300 es
                precision mediump float;

                ${isSlideTransition
                      ? 'uniform sampler2D leavingSlideTexture;'
                      : ''}
                uniform sampler2D enteringSlideTexture;
                uniform float time;
                ${!isSlideTransition
                      ? `
                          uniform float alpha;
                          uniform vec4 fromFillColor;
                          uniform vec4 toFillColor;
                          uniform vec4 fromLineColor;
                          uniform vec4 toLineColor;
                          `
                      : ''}

                in vec2 v_texCoord;
                out vec4 outColor;

                ${this.getMaskFunction()}

		            float scaleCompWrtCenter(float c, float s) {
		                return (c - 0.5) * s + 0.5;
		            }

                ${!isSlideTransition
                      ? `
                          ${GlHelpers.nearestPointOnSegment}
                          ${GlHelpers.computeColor}
                          `
                      : ''}

                void main() {
                    // reverse direction / mode out ?
                    float progress = ${
                        this.forwardParameterSweep
                            ? 'time'
														: '1.0 - time'
                    };


                    vec2 uv = v_texCoord;
                    // isotropic scale case ?
                    ${needScaling ? `uv.${comp} = scaleCompWrtCenter(uv.${comp}, ${scaleFactor})` : ''};

                    float mask = getMaskValue(uv, progress);
                    // reverse direction / mode out ?
                    ${this.invertMask ? 'mask = 1.0 - mask;' : ''}

                    vec4 color1 = ${
                        isSlideTransition
                          ? 'texture(leavingSlideTexture, v_texCoord)'
                          : 'vec4(0, 0, 0, 0)'};
                    vec4 color2 = texture(enteringSlideTexture, v_texCoord);
                    ${!isSlideTransition
                          ? `
                              color2 = computeColor(color2);
                              color2 *= alpha;
                              `
                          : ''}

                    outColor = mix(color1, color2, mask);
                }
                `;
	}
}

SlideShow.ClippingTransition = ClippingTransition;
