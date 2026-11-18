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

enum DirectionType {
	Backward,
	Forward,
}

class SimpleActivity extends ContinuousActivityBase {
	private aAnimation: AnimationBase;
	private nDirection: number;

	constructor(
		aCommonParamSet: ActivityParamSet,
		aNumberAnimation: AnimationBase,
		eDirection: DirectionType,
	) {
		super(aCommonParamSet);

		assert(
			eDirection == DirectionType.Backward ||
				eDirection == DirectionType.Forward,
			'SimpleActivity constructor: animation direction is not valid',
		);

		assert(
			aNumberAnimation,
			'SimpleActivity constructor: animation object is not valid',
		);

		this.aAnimation = aNumberAnimation;
		this.nDirection = eDirection == DirectionType.Forward ? 1.0 : 0.0;
	}

	public startAnimation() {
		if (this.isDisposed() || !this.aAnimation) return;

		ANIMDBG.print('SimpleActivity.startAnimation invoked');
		super.startAnimation();

		// start animation
		this.aAnimation.start(this.getTargetElement());
	}

	public endAnimation() {
		if (this.aAnimation) this.aAnimation.end();
	}

	protected performContinuousHook(
		nModifiedTime: number,
		nRepeatCount?: number,
	) {
		// nRepeatCount is not used

		if (this.isDisposed() || !this.aAnimation) return;

		const nT =
			1.0 - this.nDirection + nModifiedTime * (2.0 * this.nDirection - 1.0);
		this.aAnimation.perform(nT, nT === this.nDirection);
	}

	public performEnd() {
		if (this.aAnimation) {
			app.console.debug('SimpleActivity.performEnd');
			this.aAnimation.perform(this.nDirection, /*last:*/ true);
		}
	}
}
