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

abstract class EventBase {
	private static CURR_UNIQUE_ID = 0;
	private readonly nId: number;

	constructor() {
		this.nId = EventBase.getUniqueId();
	}

	private static getUniqueId() {
		++EventBase.CURR_UNIQUE_ID;
		return EventBase.CURR_UNIQUE_ID;
	}

	getId() {
		return this.nId;
	}

	abstract fire(): boolean;
	abstract isCharged(): boolean;
	abstract getActivationTime(nCurrentTime: number): number;
	abstract dispose(): void;
}

class DelayEvent extends EventBase {
	private aFunctor: any;
	private nTimeout: number;
	private bWasFired = false;

	constructor(aFunctor: any, nTimeout: number) {
		super();

		this.aFunctor = aFunctor;
		this.nTimeout = nTimeout;
	}

	fire() {
		assert(this.isCharged(), 'DelayEvent.fire: assertion isCharged failed');

		this.bWasFired = true;
		this.aFunctor();
		return true;
	}

	isCharged() {
		return !this.bWasFired;
	}

	getActivationTime(nCurrentTime: number) {
		return this.nTimeout + nCurrentTime;
	}

	dispose() {
		// don't clear unconditionally, because it may currently be executed:
		if (this.isCharged()) this.bWasFired = true;
	}

	charge() {
		if (!this.isCharged()) this.bWasFired = false;
	}
}

class WakeupEvent extends EventBase {
	private aTimer: ElapsedTime;
	private nNextTime = 0.0;
	private aActivity: any = null;
	private aActivityQueue: any;

	constructor(aTimer: ElapsedTime, aActivityQueue: any) {
		super();

		this.aTimer = new ElapsedTime(aTimer);
		this.aActivityQueue = aActivityQueue;
	}

	clone() {
		const aWakeupEvent = new WakeupEvent(
			this.aTimer.getTimeBase(),
			this.aActivityQueue,
		);
		aWakeupEvent.nNextTime = this.nNextTime;
		aWakeupEvent.aActivity = this.aActivity;
		return aWakeupEvent;
	}

	dispose() {
		this.aActivity = null;
	}

	fire() {
		if (!this.aActivity) return false;

		return this.aActivityQueue.addActivity(this.aActivity);
	}

	isCharged() {
		// this event won't expire, we fire every time we're
		// re-inserted into the event queue.
		return true;
	}

	getActivationTime(nCurrentTime: number) {
		const nElapsedTime = this.aTimer.getElapsedTime();

		return Math.max(nCurrentTime, nCurrentTime - nElapsedTime + this.nNextTime);
	}

	start() {
		this.aTimer.reset();
	}

	setNextTimeout(nNextTime: number) {
		this.nNextTime = nNextTime;
	}

	setActivity(aActivity: any) {
		this.aActivity = aActivity;
	}
}

function makeEvent(aFunctor: any) {
	return new DelayEvent(aFunctor, 0.0);
}

function makeDelay(aFunctor: any, nTimeout: number) {
	return new DelayEvent(aFunctor, nTimeout);
}

function registerEvent(
	aAnimationNode: BaseNode,
	aSlideShow: SlideShowHandler,
	aTiming: Timing,
	aEvent: EventBase,
	aNodeContext: NodeContext,
) {
	const nNodeId = aAnimationNode.getId();
	const aSlideShowContext = aNodeContext._context;
	const eTimingType = aTiming.getType();

	aRegisterEventDebugPrinter.print(
		'registerEvent( node id: ' + nNodeId + ', timing: ' + aTiming.info() + ' )',
	);

	if (eTimingType == TimingType.Offset) {
		aSlideShowContext.aTimerEventQueue.addEvent(aEvent);
	} else if (aNodeContext.bFirstRun) {
		const aEventMultiplexer = aSlideShowContext.aEventMultiplexer;
		if (!aEventMultiplexer) {
			window.app.console.log(
				'registerEvent: event multiplexer not initialized',
			);
			return;
		}
		const aNextEffectEventArray = aSlideShowContext.aNextEffectEventArray;
		if (!aNextEffectEventArray) {
			window.app.console.log(
				'registerEvent: next effect event array not initialized',
			);
			return;
		}
		const aInteractiveAnimationSequenceMap =
			aSlideShowContext.aInteractiveAnimationSequenceMap;
		if (!aInteractiveAnimationSequenceMap) {
			window.app.console.log(
				'registerEvent: interactive animation sequence map not initialized',
			);
			return;
		}

		switch (eTimingType) {
			case TimingType.Event:
				{
					const eEventType = aTiming.getEventType();
					const sEventBaseElemId = aTiming.getEventBaseElementId();
					if (sEventBaseElemId) {
						const aSourceEventElement = aNodeContext.makeSourceEventElement(
							sEventBaseElemId,
							aSlideShow,
						);

						if (!aInteractiveAnimationSequenceMap.has(nNodeId)) {
							aInteractiveAnimationSequenceMap.set(
								nNodeId,
								new InteractiveAnimationSequence(nNodeId, aSlideShow),
							);
						}

						let bEventRegistered = false;
						switch (eEventType) {
							case EventTrigger.OnClick:
								aEventMultiplexer.registerEvent(
									eEventType,
									aSourceEventElement.getId(),
									aEvent,
								);
								aEventMultiplexer.registerRewindedEffectHandler(
									aSourceEventElement.getId(),
									aSourceEventElement.charge.bind(aSourceEventElement),
								);
								bEventRegistered = true;
								break;
							default:
								window.app.console.log(
									'generateEvent: not handled event type: ' + eEventType,
								);
						}
						if (bEventRegistered) {
							const aStartEvent = aInteractiveAnimationSequenceMap
								.get(nNodeId)
								.getStartEvent();
							const aEndEvent = aInteractiveAnimationSequenceMap
								.get(nNodeId)
								.getEndEvent();
							aEventMultiplexer.registerEvent(
								eEventType,
								aSourceEventElement.getId(),
								aStartEvent,
							);
							aEventMultiplexer.registerEvent(
								EventTrigger.EndEvent,
								nNodeId,
								aEndEvent,
							);
							aEventMultiplexer.registerRewindedEffectHandler(
								nNodeId,
								aInteractiveAnimationSequenceMap
									.get(nNodeId)
									.chargeEvents.bind(
										aInteractiveAnimationSequenceMap.get(nNodeId),
									),
							);
						}
					} // no base event element present
					else {
						switch (eEventType) {
							case EventTrigger.OnNext:
								aNextEffectEventArray.appendEvent(aEvent, aAnimationNode);
								break;
							default:
								window.app.console.log(
									'registerEvent: not handled event type: ' + eEventType,
								);
						}
					}
				}
				break;
			case TimingType.SyncBase:
				{
					const eEventType = aTiming.getEventType();
					const sEventBaseElemId = aTiming.getEventBaseElementId();
					if (sEventBaseElemId) {
						const aAnimationNode =
							aNodeContext.aAnimationNodeMap.get(sEventBaseElemId);
						if (!aAnimationNode) {
							window.app.console.log(
								'registerEvent: TimingType.SyncBase: event base element not found: ' +
									sEventBaseElemId,
							);
							return;
						}
						aEventMultiplexer.registerEvent(
							eEventType,
							aAnimationNode.getId(),
							aEvent,
						);
					} else {
						window.app.console.log(
							'registerEvent: TimingType.SyncBase: event base element not specified',
						);
					}
				}
				break;
			default:
				window.app.console.log(
					'registerEvent: not handled timing type: ' + eTimingType,
				);
		}
	}
}
