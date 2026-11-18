// @ts-strict-ignore
/* -*- tab-width: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

enum ColorSpace {
	rgb,
	hsl,
}

enum ClockDirection {
	clockwise,
	counterClockwise,
}

class AnimationColorNode extends AnimationBaseNode3 {
	private eColorInterpolation: ColorSpace;
	private eColorInterpolationDirection: ClockDirection;

	constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		super(aNodeInfo, aParentNode, aNodeContext);
		this.sClassName = 'AnimationColorNode';
	}

	public parseNodeInfo() {
		super.parseNodeInfo();

		const aNodeInfo = this.aNodeInfo as AnimateColorNodeInfo;

		this.eColorInterpolation = ColorSpace.rgb;
		if (
			aNodeInfo.colorInterpolation &&
			aNodeInfo.colorInterpolation in ColorSpace
		) {
			const sColorInterpolation =
				aNodeInfo.colorInterpolation as keyof typeof ColorSpace;
			this.eColorInterpolation = ColorSpace[sColorInterpolation];
		}

		this.eColorInterpolationDirection = ClockDirection.clockwise;
		if (
			aNodeInfo.colorInterpolationDirection &&
			aNodeInfo.colorInterpolationDirection in ClockDirection
		) {
			const sColorInterpolationDir =
				aNodeInfo.colorInterpolationDirection as keyof typeof ClockDirection;
			this.eColorInterpolationDirection =
				ClockDirection[sColorInterpolationDir];
		}
	}

	public createActivity(): AnimationActivity {
		const aActivityParamSet = this.fillActivityParams();

		// initialize object dim color
		if (
			this.getAnimatedElement() &&
			this.getToValue() &&
			this.getAttributeName() === 'dimcolor'
		) {
			this.getAnimatedElement().setDefaultDimColor(
				colorParser(this.getToValue()) as RGBColor,
			);
		}

		const aAnimation = createPropertyAnimation(
			this.getAttributeName(),
			this.getAnimatedElement(),
			this.aNodeContext._context.nSlideWidth,
			this.aNodeContext._context.nSlideHeight,
		);

		let aColorAnimation: AnimationBase;
		let aInterpolator;
		if (this.getColorInterpolation() === ColorSpace.hsl) {
			ANIMDBG.print('AnimationColorNode.createActivity: color space hsl');
			aColorAnimation = new HSLAnimationWrapper(aAnimation);
			aInterpolator = PropertyInterpolator.getInterpolator(
				PropertyValueType.Color,
				ColorSpace.hsl,
				this.getColorInterpolationDirection(),
			);
		} else {
			ANIMDBG.print('AnimationColorNode.createActivity: color space rgb');
			aColorAnimation = aAnimation;
			aInterpolator = PropertyInterpolator.getInterpolator(
				PropertyValueType.Color,
				ColorSpace.rgb,
			);
		}

		return createActivity(
			aActivityParamSet,
			this,
			aColorAnimation,
			aInterpolator,
		);
	}

	public getColorInterpolation() {
		return this.eColorInterpolation;
	}

	public getColorInterpolationDirection() {
		return this.eColorInterpolationDirection;
	}

	public info(bVerbose: boolean = false) {
		let sInfo = super.info.call(this, bVerbose);

		if (bVerbose) {
			// color interpolation
			sInfo +=
				';  color-interpolation: ' + ColorSpace[this.getColorInterpolation()];

			// color interpolation direction
			sInfo +=
				';  color-interpolation-direction: ' +
				ClockDirection[this.getColorInterpolationDirection()];
		}
		return sInfo;
	}
}
