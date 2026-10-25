// @ts-strict-ignore
/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

enum EffectCommand {
	Custom,
	Verb,
	Play,
	TogglePause,
	Stop,
	StopAudio,
}

class AnimationCommandNode extends BaseNode {
	private sTargetHash: string;
	private eCommand: EffectCommand;

	constructor(
		aNodeInfo: AnimationNodeInfo,
		aParentNode: BaseContainerNode,
		aNodeContext: NodeContext,
	) {
		super(aNodeInfo, aParentNode, aNodeContext);
		this.sClassName = 'AnimationCommandNode';
	}

	public parseNodeInfo() {
		super.parseNodeInfo();

		const aNodeInfo: CommandNodeInfo = this.aNodeInfo;
		this.sTargetHash = aNodeInfo.targetElement;
		const sCommand = aNodeInfo.command as keyof typeof EffectCommand;
		this.eCommand = EffectCommand[sCommand];

		if (!this.sTargetHash || this.eCommand === undefined) {
			this.eCurrentState = NodeState.Invalid;
			window.app.console.log(
				'AnimationCommandNode.parseNodeInfo: invalid target or command',
			);
		}
	}

	private getVideoRenderer(): AnimatedContentRenderer {
		const slideInfo = this.aNodeContext.metaSlide.info;
		const videoInfo = slideInfo.videos?.find(
			(info) => info.hash === this.sTargetHash,
		);
		if (!videoInfo) return null;
		return this._context.aSlideShowHandler.getVideoRenderer(
			slideInfo.hash,
			videoInfo,
		);
	}

	protected activate_st() {
		const video = this.getVideoRenderer();
		if (video) {
			switch (this.eCommand) {
				case EffectCommand.Play:
					video.play();
					break;
				case EffectCommand.TogglePause:
					video.handleClick();
					break;
				case EffectCommand.Stop:
					video.pause();
					break;
			}
		}
		// A command takes no time, so the next effect in the sequence starts at once.
		this.scheduleDeactivationEvent(this.makeDeactivationEvent(0.0));
	}

	public hasPendingAnimation() {
		return true;
	}

	public removeEffect() {
		// empty body
	}

	public saveStateOfAnimatedElement() {
		// empty body
	}
}
