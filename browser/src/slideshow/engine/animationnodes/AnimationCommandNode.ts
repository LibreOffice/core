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

/*
 * Starts, pauses, resumes or stops the video of its target on activation. A command takes no
 * time and leaves nothing to restore.
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
	private sTargetElement: string;
	private eCommand: EffectCommand;
	private nMediaTime: number;

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
		this.sTargetElement = aNodeInfo.targetElement;
		const sCommand = aNodeInfo.command as keyof typeof EffectCommand;
		this.eCommand = EffectCommand[sCommand];
		this.nMediaTime = aNodeInfo.mediaTime ?? 0;

		if (!this.sTargetElement || this.eCommand === undefined) {
			this.eCurrentState = NodeState.Invalid;
			window.app.console.log(
				'AnimationCommandNode.parseNodeInfo: invalid target or command',
			);
		}
	}

	// The renderer of the target's video on the slide, or null.
	private getVideoRenderer(): AnimatedContentRenderer {
		const slideInfo = this.aNodeContext.metaSlide.info;
		const videoInfo = slideInfo.videos?.find(
			(info: VideoInfo) => String(info.id) === this.sTargetElement,
		);
		if (!videoInfo) return null;
		return this._context.aSlideShowHandler.getVideoRenderer(
			slideInfo.hash,
			videoInfo,
		);
	}

	// Play starts at the media time. Toggle pauses or resumes, and replays an ended video.
	// Stop pauses and rewinds.
	protected activate_st() {
		const video = this.getVideoRenderer();
		if (video) {
			switch (this.eCommand) {
				case EffectCommand.Play:
					video.playFrom(this.nMediaTime);
					break;
				case EffectCommand.TogglePause:
					video.handleClick();
					break;
				case EffectCommand.Stop:
					video.stop();
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
		// A command leaves no state to restore.
	}

	public saveStateOfAnimatedElement() {
		// A command has no animated element.
	}
}
