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

/*
 * The morph from one slide to the next: the objects the two slides share move into their new
 * places while the rest fade. Where the first frame cannot be drawn object by object, the whole
 * transition plays as a smooth fade.
 */
class MorphTransition extends SlideShow.FadeTransition {
	// Whether the frames are drawn object by object, null until the first frame.
	private isMorphing: boolean | null = null;

	constructor(transitionParameters: TransitionParameters) {
		super(transitionParameters);
	}

	// The fade draws the slides from their textures. A morph that got none builds no program.
	protected createProgram(): void {
		if (this.enteringSlide) super.createProgram();
	}

	protected prepareTransition(): void {
		if (this.enteringSlide) super.prepareTransition();
	}

	public endTransition(): void {
		if (this.program) super.endTransition();
	}

	public render(nT: number, properties?: AnimatedElementRenderProperties) {
		if (this.context.isDisposed()) return;
		const drawMorph = this.transitionParameters.drawMorph;
		if (this.isMorphing === null) {
			this.isMorphing = !!drawMorph && drawMorph(MorphTransition._eased(nT));
			// A morph draws the slides as they are now, so the fade's textures need no update.
			if (this.isMorphing) this.transitionParameters.refreshSlides = null;
		} else if (this.isMorphing) {
			drawMorph(MorphTransition._eased(nT));
		}
		if (!this.isMorphing) {
			if (this.enteringSlide) {
				super.render(nT, properties);
				return;
			}
			// Without the slide textures there is no fade, so the entering slide shows at once.
			if (!this.transitionParameters.drawEnteringSlide?.()) return;
		}
		app.map.fire('newslideshowframe', { frame: this.gl.canvas });
	}

	// The objects start and stop gently.
	private static _eased(nT: number): number {
		return nT * nT * (3 - 2 * nT);
	}
}

SlideShow.MorphTransition = MorphTransition;
