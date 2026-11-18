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

function ValueListActivityTemplate<T extends AGConstructor<ActivityBase>>(
	BaseType: T,
) {
	abstract class ValueListActivity extends BaseType {
		private aValueList: any[];
		private aAnimation: AnimationBase;
		private aInterpolator: PropertyInterpolatorType;
		private add: (a: any, b: any) => any;
		private scale: (k: any, v: any) => any;
		private bCumulative: boolean;
		private aLastValue: any;
		private aFormula: (x: any) => any;

		constructor(...args: any[]) {
			assert(
				args.length === 6,
				'ValueListActivity, constructor args length is wrong',
			);

			const aValueList: any[] = args[0];
			const aActivityParamSet: ActivityParamSet = args[1];
			const aAnimation: AnimationBase = args[2];
			const aInterpolator: PropertyInterpolatorType = args[3];
			const aOperatorSet: PropertyOperatorSet = args[4];
			const bAccumulate: boolean = args[5];

			assert(
				aAnimation,
				'ValueListActivity constructor: invalid animation object',
			);
			assert(aValueList.length != 0, 'ValueListActivity: value list is empty');

			super(
				aValueList,
				aActivityParamSet,
				aAnimation,
				aInterpolator,
				aOperatorSet,
				bAccumulate,
			);

			this.aValueList = aValueList;
			this.aAnimation = aAnimation;
			this.aInterpolator = aInterpolator;
			this.add = aOperatorSet.add;
			this.scale = aOperatorSet.scale;
			this.bCumulative = bAccumulate;
			this.aLastValue = this.aValueList[this.aValueList.length - 1];
			this.aFormula = aActivityParamSet.aFormula;
		}

		public activate(aEndEvent: DelayEvent) {
			super.activate(aEndEvent);
			for (let i = 0; i < this.aValueList.length; ++i) {
				ANIMDBG.print(
					'ValueListActivity.activate: value[' +
						i +
						'] = ' +
						this.aValueList[i],
				);
			}
		}

		public initAnimatedElement() {
			if (this.aAnimation) {
				let aValue = this.aValueList[0];
				aValue = this.aFormula ? this.aFormula(aValue) : aValue;
				this.aAnimation.perform(aValue);
			}
		}

		public startAnimation() {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'ValueListActivity.startAnimation: activity disposed or not valid animation',
				);
				return;
			}

			super.startAnimation();

			this.aAnimation.start(this.getTargetElement());
		}

		public endAnimation() {
			if (this.aAnimation) this.aAnimation.end();
		}

		// perform hook override for ContinuousKeyTimeActivityBase base
		public performContinuousHook(
			nIndex: number,
			nFractionalIndex: number,
			nRepeatCount: number,
		) {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'ValueListActivity.performContinuousHook: activity disposed or not valid animation',
				);
				return;
			}

			assert(
				nIndex + 1 < this.aValueList.length,
				'ValueListActivity.performContinuousHook: assertion (nIndex + 1 < this.aValueList.length) failed',
			);

			// interpolate between nIndex and nIndex+1 values

			let aValue = this.aInterpolator(
				this.aValueList[nIndex],
				this.aValueList[nIndex + 1],
				nFractionalIndex,
			);

			if (this.bCumulative) {
				//aValue = aValue + nRepeatCount * this.aLastValue;
				aValue = this.add(aValue, this.scale(nRepeatCount, this.aLastValue));
			}

			aValue = this.aFormula ? this.aFormula(aValue) : aValue;
			this.aAnimation.perform(aValue);
		}

		// perform hook override for DiscreteActivityBase base
		protected performDiscreteHook(nFrame: number, nRepeatCount: number) {
			if (this.isDisposed() || !this.aAnimation) {
				window.app.console.log(
					'ValueListActivity.performDiscreteHook: activity disposed or not valid animation',
				);
				return;
			}

			assert(
				nFrame < this.aValueList.length,
				'ValueListActivity.performDiscreteHook: assertion ( nFrame < this.aValueList.length) failed',
			);

			// this is discrete, thus no lerp here.
			let aValue = this.aValueList[nFrame];

			if (this.bCumulative) {
				aValue = this.add(aValue, this.scale(nRepeatCount, this.aLastValue));
				// for numbers:   aValue = aValue + nRepeatCount * this.aLastValue;
				// for enums, bools or strings:   aValue = aValue;
			}

			aValue = this.aFormula ? this.aFormula(aValue) : aValue;
			this.aAnimation.perform(aValue);
		}

		public performEnd() {
			if (this.aAnimation) {
				const aValue = this.aFormula
					? this.aFormula(this.aLastValue)
					: this.aLastValue;
				this.aAnimation.perform(aValue);
			}
		}

		public dispose() {
			super.dispose();
		}
	}

	return ValueListActivity;
}

abstract class ContinuousKeyTimeActivityBaseValueListCtor extends ContinuousKeyTimeActivityBase {
	constructor(
		aValueList: any[],
		aActivityParamSet: ActivityParamSet,
		aAnimation: AnimationBase,
		aInterpolator: PropertyInterpolatorType,
		aOperatorSet: PropertyOperatorSet,
		bAccumulate: boolean,
	) {
		super(aActivityParamSet);
	}
}

const AbstractLinearValueListActivity = ValueListActivityTemplate(
	ContinuousKeyTimeActivityBaseValueListCtor,
);

class LinearValueListActivity extends AbstractLinearValueListActivity {}

abstract class DiscreteActivityBaserValueListCtor extends DiscreteActivityBase {
	constructor(
		aValueList: any[],
		aActivityParamSet: ActivityParamSet,
		aAnimation: AnimationBase,
		aInterpolator: PropertyInterpolatorType,
		aOperatorSet: PropertyOperatorSet,
		bAccumulate: boolean,
	) {
		super(aActivityParamSet);
	}
}

const AbstractDiscreteValueListActivity = ValueListActivityTemplate(
	DiscreteActivityBaserValueListCtor,
);

class DiscreteValueListActivity extends AbstractDiscreteValueListActivity {}

type ValueListActivityCtorType = new (
	aValueList: any[],
	aActivityParamSet: ActivityParamSet,
	aAnimation: AnimationBase,
	aInterpolator: PropertyInterpolatorType,
	aOperatorSet: PropertyOperatorSet,
	bAccumulate: boolean,
) => ActivityBase;
