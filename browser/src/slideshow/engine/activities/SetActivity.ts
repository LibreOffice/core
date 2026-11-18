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

class SetActivity extends AnimationActivity {
	private aAnimation: AnimationBase;
	private aTargetElement: null;
	private aEndEvent: DelayEvent;
	private aTimerEventQueue: TimerEventQueue;
	private aToAttr: any;
	private bIsActive: boolean;

	constructor(
		aCommonParamSet: ActivityParamSet,
		aAnimationNode: AnimationSetNode,
		aAnimation: AnimationBase,
	) {
		super();

		this.aAnimation = aAnimation;
		this.aTargetElement = null;
		this.aEndEvent = aCommonParamSet.aEndEvent;
		this.aTimerEventQueue = aCommonParamSet.aTimerEventQueue;
		this.bIsActive = true;

		const aAnimatedElement = aAnimationNode.getAnimatedElement();
		const sAttributeName = aAnimationNode.getAttributeName();
		const sKey = sAttributeName as PropertyGetterSetterMapKeyType;
		const aAttributeProp = aPropertyGetterSetterMap[sKey];

		const eValueType: PropertyValueType = aAttributeProp['type'];

		const aValueSet = [aAnimationNode.getToValue()];

		ANIMDBG.print(
			'SetActivity: value type: ' +
				PropertyValueType[eValueType] +
				', aTo = ' +
				aValueSet[0],
		);

		const aValueList: any[] = [];

		extractAttributeValues(
			eValueType,
			aValueList,
			aValueSet,
			aAnimatedElement.getBaseBBox(),
			aCommonParamSet.nSlideWidth,
			aCommonParamSet.nSlideHeight,
		);

		ANIMDBG.print('SetActivity ctor: aTo = ' + aValueList[0]);

		this.aToAttr = aValueList[0];
	}

	public activate(aEndEvent: DelayEvent) {
		this.aEndEvent = aEndEvent;
		this.bIsActive = true;
	}

	public dispose() {
		this.bIsActive = false;
		if (this.aEndEvent && this.aEndEvent.isCharged()) this.aEndEvent.dispose();
	}

	public calcTimeLag() {
		return 0.0;
	}

	public perform() {
		if (!this.isActive()) return false;

		// we're going inactive immediately:
		this.bIsActive = false;

		if (this.aAnimation && this.aTargetElement) {
			this.aAnimation.start(this.aTargetElement);
			this.aAnimation.perform(this.aToAttr);
			this.aAnimation.end();
		}

		if (this.aEndEvent) this.aTimerEventQueue.addEvent(this.aEndEvent);
	}

	public isActive() {
		return this.bIsActive;
	}

	public dequeued() {
		// empty body
	}

	public end() {
		this.perform();
	}

	public setTargets(aTargetElement: any) {
		assert(
			aTargetElement,
			'SetActivity.setTargets: target element is not valid',
		);
		this.aTargetElement = aTargetElement;
	}
}
