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

function FromToByActivityTemplate<T extends AGConstructor<ActivityBase>>(
	BaseType: T,
) {
	abstract class FromToByActivity extends BaseType {
		private aFrom: any;
		private aTo: any;
		private aBy: any;
		private aStartValue: any;
		private aEndValue: any;
		private aPreviousValue: any;
		private aStartInterpolationValue: any;
		private aAnimation: AnimationBase;
		private aInterpolator: PropertyInterpolatorType;
		private equal: (a: any, b: any) => boolean;
		private add: (a: any, b: any) => any;
		private scale: (k: any, v: any) => any;
		private bDynamicStartValue: boolean;
		private nIteration: number;
		private bCumulative: boolean;
		private aFormula: (x: any) => any;

		constructor(...args: any[]) {
			assert(
				args.length === 8,
				'FromToByActivity, constructor args length is wrong',
			);

			const aFromValue: any = args[0];
			const aToValue: any = args[1];
			const aByValue: any = args[2];
			const aActivityParamSet: ActivityParamSet = args[3];
			const aAnimation: AnimationBase = args[4];
			const aInterpolator: PropertyInterpolatorType = args[5];
			const aOperatorSet: PropertyOperatorSet = args[6];
			const bAccumulate: boolean = args[7];

			super(
				aFromValue,
				aToValue,
				aByValue,
				aActivityParamSet,
				aAnimation,
				aInterpolator,
				aOperatorSet,
				bAccumulate,
			);

			this.aFrom = aFromValue;
			this.aTo = aToValue;
			this.aBy = aByValue;
			this.aStartValue = null;
			this.aEndValue = null;
			this.aPreviousValue = null;
			this.aStartInterpolationValue = null;
			this.aAnimation = aAnimation;
			this.aInterpolator = aInterpolator;
			this.equal = aOperatorSet.equal;
			this.add = aOperatorSet.add;
			this.scale = aOperatorSet.scale;
			this.bDynamicStartValue = false;
			this.nIteration = 0;
			this.bCumulative = bAccumulate;
			this.aFormula = aActivityParamSet.aFormula;
		}

		public initAnimatedElement() {
			if (this.aAnimation && hasValue(this.aFrom)) {
				const aValue = this.aFormula ? this.aFormula(this.aFrom) : this.aFrom;
				this.aAnimation.perform(aValue);
			}
		}

		public startAnimation() {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'FromToByActivity.startAnimation: activity disposed or not valid animation',
				);
				return;
			}

			super.startAnimation();

			this.aAnimation.start(this.getTargetElement());

			const aAnimationStartValue = this.aAnimation.getUnderlyingValue();

			// first of all, determine general type of
			// animation, by inspecting which of the FromToBy values
			// are actually valid.
			// See http://www.w3.org/TR/smil20/animation.html#AnimationNS-FromToBy
			// for a definition
			if (hasValue(this.aFrom)) {
				// From-to or From-by animation. According to
				// SMIL spec, the To value takes precedence
				// over the By value, if both are specified
				if (hasValue(this.aTo)) {
					// From-To animation
					this.aStartValue = this.aFrom;
					this.aEndValue = this.aTo;
				} else if (hasValue(this.aBy)) {
					// From-By animation
					this.aStartValue = this.aFrom;

					this.aEndValue = this.add(this.aStartValue, this.aBy);
				}
				this.aStartInterpolationValue = this.aStartValue;
			} else {
				this.aStartValue = aAnimationStartValue;
				this.aStartInterpolationValue = this.aStartValue;

				// By or To animation. According to SMIL spec,
				// the To value takes precedence over the By
				// value, if both are specified
				if (hasValue(this.aTo)) {
					// To animation

					// According to the SMIL spec
					// (http://www.w3.org/TR/smil20/animation.html#animationNS-ToAnimation),
					// the to animation interpolates between
					// the _running_ underlying value and the to value (as the end value)
					this.bDynamicStartValue = true;
					this.aPreviousValue = this.aStartValue;
					this.aEndValue = this.aTo;
				} else if (hasValue(this.aBy)) {
					// By animation
					this.aStartValue = aAnimationStartValue;

					this.aEndValue = this.add(this.aStartValue, this.aBy);
				}
			}

			ANIMDBG.print(
				'FromToByActivity.startAnimation: aStartValue = ' +
					this.aStartValue +
					', aEndValue = ' +
					this.aEndValue,
			);
		}

		public endAnimation() {
			if (this.aAnimation) this.aAnimation.end();
		}

		// perform hook override for ContinuousActivityBase
		public performContinuousHook(nModifiedTime: number, nRepeatCount: number) {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'FromToByActivity.performContinuousHook: activity disposed or not valid animation',
				);
				return;
			}

			// According to SMIL 3.0 spec 'to' animation if no other (lower priority)
			// animations are active or frozen then a simple interpolation is performed.
			// That is, the start interpolation value is constant while the animation
			// is running, and is equal to the underlying value retrieved when
			// the animation start.
			// However if another animation is manipulating the underlying value,
			// the 'to' animation will initially add to the effect of the lower priority
			// animation, and increasingly dominate it as it nears the end of the
			// simple duration, eventually overriding it completely.
			// That is, each time the underlying value is changed between two
			// computations of the animation function the new underlying value is used
			// as start value for the interpolation.
			// See:
			// http://www.w3.org/TR/SMIL3/smil-animation.html#animationNS-ToAnimation
			// (Figure 6 - Effect of Additive to animation example)
			// Moreover when a 'to' animation is repeated, at each new iteration
			// the start interpolation value is reset to the underlying value
			// of the animated property when the animation started,
			// as it is shown in the example provided by the SMIL 3.0 spec.
			// This is exactly as Firefox performs SVG 'to' animations.
			if (this.bDynamicStartValue) {
				if (this.nIteration != nRepeatCount) {
					this.nIteration = nRepeatCount;
					this.aStartInterpolationValue = this.aStartValue;
				} else {
					const aActualValue = this.aAnimation.getUnderlyingValue();
					if (!this.equal(aActualValue, this.aPreviousValue))
						this.aStartInterpolationValue = aActualValue;
				}
			}

			let aValue = this.aInterpolator(
				this.aStartInterpolationValue,
				this.aEndValue,
				nModifiedTime,
			);

			// According to the SMIL spec:
			// Because 'to' animation is defined in terms of absolute values of
			// the target attribute, cumulative animation is not defined.
			if (this.bCumulative && !this.bDynamicStartValue) {
				// aValue = this.aEndValue * nRepeatCount + aValue;
				aValue = this.add(this.scale(nRepeatCount, this.aEndValue), aValue);
			}

			aValue = this.aFormula ? this.aFormula(aValue) : aValue;
			this.aAnimation.perform(aValue);

			if (this.bDynamicStartValue) {
				this.aPreviousValue = this.aAnimation.getUnderlyingValue();
			}
		}

		// perform hook override for DiscreteActivityBase
		public performDiscreteHook(/*nFrame, nRepeatCount*/) {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'FromToByActivity.performDiscreteHook: activity disposed or not valid animation',
				);
				return;
			}
		}

		public performEnd() {
			if (this.aAnimation) {
				let aValue = this.isAutoReverse() ? this.aStartValue : this.aEndValue;
				aValue = this.aFormula ? this.aFormula(aValue) : aValue;
				this.aAnimation.perform(aValue);
			}
		}

		public dispose() {
			super.dispose();
		}
	}

	return FromToByActivity;
}

abstract class ContinuousActivityBaserFromToByCtor extends ContinuousActivityBase {
	constructor(
		aFromValue: any,
		aToValue: any,
		aByValue: any,
		aActivityParamSet: ActivityParamSet,
		aAnimation: AnimationBase,
		aInterpolator: PropertyInterpolatorType,
		aOperatorSet: PropertyOperatorSet,
		bAccumulate: boolean,
	) {
		super(aActivityParamSet);
	}
}

const AbstractLinearFromToByActivity = FromToByActivityTemplate(
	ContinuousActivityBaserFromToByCtor,
);

class LinearFromToByActivity extends AbstractLinearFromToByActivity {}

abstract class DiscreteActivityBaserFromToByCtor extends DiscreteActivityBase {
	constructor(
		aFromValue: any,
		aToValue: any,
		aByValue: any,
		aActivityParamSet: ActivityParamSet,
		aAnimation: AnimationBase,
		aInterpolator: PropertyInterpolatorType,
		aOperatorSet: PropertyOperatorSet,
		bAccumulate: boolean,
	) {
		super(aActivityParamSet);
	}
}

const AbstractDiscreteFromToByActivity = FromToByActivityTemplate(
	DiscreteActivityBaserFromToByCtor,
);

class DiscreteFromToByActivity extends AbstractDiscreteFromToByActivity {}

type FromToByActivityCtorType = new (
	aFromValue: any,
	aToValue: any,
	aByValue: any,
	aActivityParamSet: ActivityParamSet,
	aAnimation: AnimationBase,
	aInterpolator: PropertyInterpolatorType,
	aOperatorSet: PropertyOperatorSet,
	bAccumulate: boolean,
) => ActivityBase;
