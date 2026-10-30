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

class MetaSlide {
	private _info: SlideInfo;
	private readonly _metaPres: MetaPresentation;
	private readonly _slideShowHandler: SlideShowHandler;
	private _transitionHandler: SlideTransition;
	private _animationsHandler: SlideAnimations;

	constructor(slideInfo: SlideInfo, metaPres: MetaPresentation) {
		this._info = slideInfo;
		this._metaPres = metaPres;
		this._slideShowHandler = metaPres.slideShowHandler;

		if (this.hasTransition())
			this._transitionHandler = new SlideTransition(slideInfo);

		if (slideInfo.animations) {
			this._animationsHandler = new SlideAnimations(
				this._slideShowHandler.getContext(),
				this,
			);
			this._animationsHandler.importAnimations(slideInfo.animations.root);
			this._animationsHandler.parseInfo();
			this.setInitialVisibilities(slideInfo.animations.root);
			app.console.debug(
				`\x1B[1mSlide ${this._info.index} Animation Tree\x1B[m\n` +
					this._animationsHandler.info(true),
			);
		}
	}

	// A target starts the slide in the visibility of its first effect, where
	// the engine resolved one. An effect is a node with a preset id, and its
	// target the first named under it.
	private setInitialVisibilities(aRoot: AnimationNodeInfo) {
		const aSeenTargets = new Set<string>();
		this.walkEffects(aRoot, (aEffect: AnimationNodeInfo) => {
			const sTarget = MetaSlide.firstTargetUnder(aEffect);
			if (!sTarget || aSeenTargets.has(sTarget)) return;
			aSeenTargets.add(sTarget);

			if (aEffect.initVisible === undefined) return;
			const aElement = this._animationsHandler
				.getAnimatedElementMap()
				.get(sTarget);
			if (aElement) aElement.setInitialVisibility(aEffect.initVisible);
		});
	}

	private walkEffects(
		aNode: AnimationNodeInfo,
		onEffect: (aEffect: AnimationNodeInfo) => void,
	) {
		if (!aNode) return;
		if (aNode.presetId) {
			onEffect(aNode);
			return;
		}
		const aChildren = (aNode as ContainerNodeInfo).children;
		if (aChildren)
			for (const aChild of aChildren) this.walkEffects(aChild, onEffect);
	}

	private static firstTargetUnder(
		aNode: AnimationNodeInfo,
	): string | undefined {
		const sTarget = (aNode as AnimateNodeInfo).targetElement;
		if (sTarget) return sTarget;
		const aChildren = (aNode as ContainerNodeInfo).children;
		if (aChildren)
			for (const aChild of aChildren) {
				const sFound = MetaSlide.firstTargetUnder(aChild);
				if (sFound) return sFound;
			}
		return undefined;
	}

	public hasTransition(): boolean {
		return stringToTransitionTypeMap[this._info.transitionType] !== undefined;
	}

	public get info(): SlideInfo {
		return this._info;
	}

	public get hidden(): boolean {
		return this._info.hidden;
	}

	public get next(): MetaSlide {
		return this.info.next ? this._metaPres.getMetaSlide(this.info.next) : null;
	}

	public get prev(): MetaSlide {
		return this.info.prev ? this._metaPres.getMetaSlide(this.info.prev) : null;
	}

	public get transitionHandler(): SlideTransition {
		return this._transitionHandler;
	}

	public get animationsHandler(): SlideAnimations {
		return this._animationsHandler;
	}

	public getTriggerInfo(hash: string) {
		const triggers = this._info.triggers;
		if (triggers) {
			const index = triggers.findIndex((value) => value.hash === hash);
			if (index !== -1) {
				return {
					bounds: triggers[index].bounds,
					index: index,
				};
			}
		}
		return null;
	}

	public show() {
		// TODO implement it ?
	}

	public hide() {
		// TODO implement it ?
	}
}
