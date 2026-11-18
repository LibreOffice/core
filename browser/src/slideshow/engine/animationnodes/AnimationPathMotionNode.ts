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

class AnimationPathMotionNode extends AnimationBaseNode {
	private path: string;

	constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		super(aNodeInfo, aParentNode, aNodeContext);
		this.sClassName = 'AnimationPathMotionNode';
	}

	public parseNodeInfo() {
		super.parseNodeInfo();

		const aNodeInfo = this.aNodeInfo as AnimateMotionNodeInfo;
		if (!aNodeInfo.path) {
			this.eCurrentState = NodeState.Invalid;
			window.app.console.log(
				'AnimationPathMotionNode.parseElement: path is not valid',
			);
		}
		this.path = aNodeInfo.path;
	}

	public createActivity(): AnimationActivity {
		const aActivityParamSet = this.fillActivityParams();

		const aAnimation = new PathAnimation(
			this.path,
			this.getAdditiveMode(),
			aActivityParamSet.nSlideWidth,
			aActivityParamSet.nSlideHeight,
		);

		return new SimpleActivity(
			aActivityParamSet,
			aAnimation,
			DirectionType.Forward,
		);
	}

	public getPath() {
		return this.path;
	}

	public info(bVerbose: boolean = false) {
		let sInfo = super.info.call(this, bVerbose);

		if (bVerbose) {
			// svg path
			sInfo += ';  path: ' + this.getPath();
		}
		return sInfo;
	}
}
