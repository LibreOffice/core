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

		// The vector path draws slides as static content and does not
		// drive shape effects yet, so the animation tree is built for
		// the bitmap path only. Without the tree an effect dispatch
		// falls through to the next slide.
		if (slideInfo.animations && !RenderManager.isVectorRendering()) {
			this._animationsHandler = new SlideAnimations(
				this._slideShowHandler.getContext(),
				this,
			);
			this._animationsHandler.importAnimations(slideInfo.animations.root);
			this._animationsHandler.parseInfo();
			app.console.debug(
				`\x1B[1mSlide ${this._info.index} Animation Tree\x1B[m\n` +
					this._animationsHandler.info(true),
			);
		}
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
