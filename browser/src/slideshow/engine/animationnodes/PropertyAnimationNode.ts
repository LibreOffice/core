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

class PropertyAnimationNode extends AnimationBaseNode3 {
	constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		super(aNodeInfo, aParentNode, aNodeContext);
		this.sClassName = 'PropertyAnimationNode';
	}

	createActivity(): AnimationActivity {
		const aActivityParamSet = this.fillActivityParams();

		const aAnimation = createPropertyAnimation(
			this.getAttributeName(),
			this.getAnimatedElement(),
			this.aNodeContext._context.nSlideWidth,
			this.aNodeContext._context.nSlideHeight,
		);

		if (!aAnimation) {
			window.app.console.log(
				'PropertyAnimationNode.createActivity: failed to create animation.',
			);
			return null;
		}
		// TODO Interpolator = null ?
		const aInterpolator: PropertyInterpolatorType = null; // createActivity will compute it;
		return createActivity(aActivityParamSet, this, aAnimation, aInterpolator);
	}
}
