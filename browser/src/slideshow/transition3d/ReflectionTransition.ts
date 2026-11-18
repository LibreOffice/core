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

class ReflectionTransition extends SimpleTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}
	// TODO - remove code duplication
	/* jscpd:ignore-start */
	public displaySlides_(t: number): void {
		this.applyAllOperation(t);

		if (t < 0.5) {
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				0,
				this.leavingPrimitives,
				'slideTexture',
			);
		} else {
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				1,
				this.enteringPrimitives,
				'slideTexture',
			);
		}
	}
	/* jscpd:ignore-end */
}

SlideShow.ReflectionTransition = ReflectionTransition;
