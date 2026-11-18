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

interface MouseClickHandler {
	handleClick: (aMouseEvent?: any) => boolean;
}

class EventMultiplexer {
	private static CURR_UNIQUE_ID = 0;
	private nId: number;
	private aTimerEventQueue: TimerEventQueue;
	private aEventMap = new Map<EventTrigger, Map<string, EventBase[]>>();
	private aAnimationsEndHandler: Handler0 = null;
	private aSkipEffectEndHandlerSet: Handler0[] = [];
	private aMouseClickHandlerSet: PriorityQueue = null;
	private aSkipEffectEvent: DelayEvent = null;
	private aRewindCurrentEffectEvent: DelayEvent = null;
	private aRewindLastEffectEvent: DelayEvent = null;
	private aSkipInteractiveEffectEventSet = new Map<number, DelayEvent>();
	private aRewindRunningInteractiveEffectEventSet = new Map<
		number,
		DelayEvent
	>();
	private aRewindEndedInteractiveEffectEventSet = new Map<number, DelayEvent>();
	private aRewindedEffectHandlerSet = new Map<string, Handler0>();
	private aElementChangedHandlerSet = new Map<string, Handler1>();

	constructor(aTimerEventQueue: TimerEventQueue) {
		this.nId = EventMultiplexer.getUniqueId();
		this.aTimerEventQueue = aTimerEventQueue;
		this.aMouseClickHandlerSet = new PriorityQueue(PriorityEntry.compare);
	}

	private static getUniqueId() {
		++EventMultiplexer.CURR_UNIQUE_ID;
		return EventMultiplexer.CURR_UNIQUE_ID;
	}

	clear() {
		this.aEventMap.clear();
		this.aEventMap = null;
		this.aSkipEffectEndHandlerSet = null;
		this.aMouseClickHandlerSet.clear();
		this.aMouseClickHandlerSet = null;
		this.aSkipInteractiveEffectEventSet.clear();
		this.aSkipInteractiveEffectEventSet = null;
		this.aRewindRunningInteractiveEffectEventSet.clear();
		this.aRewindRunningInteractiveEffectEventSet = null;
		this.aRewindEndedInteractiveEffectEventSet.clear();
		this.aRewindEndedInteractiveEffectEventSet = null;
		this.aRewindedEffectHandlerSet.clear();
		this.aRewindedEffectHandlerSet = null;
		this.aElementChangedHandlerSet.clear();
		this.aElementChangedHandlerSet = null;
	}

	getId() {
		return this.nId;
	}

	hasRegisteredMouseClickHandlers() {
		return !this.aMouseClickHandlerSet.isEmpty();
	}

	registerMouseClickHandler(aHandler: MouseClickHandler, nPriority: number) {
		const aHandlerEntry = new PriorityEntry(aHandler, nPriority);
		this.aMouseClickHandlerSet.push(aHandlerEntry);
	}

	notifyMouseClick(aMouseEvent: any) {
		const aMouseClickHandlerSet = this.aMouseClickHandlerSet.clone();
		while (!aMouseClickHandlerSet.isEmpty()) {
			const aHandlerEntry = aMouseClickHandlerSet.top();
			aMouseClickHandlerSet.pop();
			if (aHandlerEntry.aValue && aHandlerEntry.aValue.handleClick) {
				if (aHandlerEntry.aValue.handleClick(aMouseEvent)) break;
			}
		}
	}

	notifyMouseMove(aMouseEvent: any) {
		const aMouseClickHandlerSet = this.aMouseClickHandlerSet.clone();
		while (!aMouseClickHandlerSet.isEmpty()) {
			const aHandlerEntry = aMouseClickHandlerSet.top();
			aMouseClickHandlerSet.pop();
			if (aHandlerEntry.aValue && aHandlerEntry.aValue.onMouseMove) {
				if (aHandlerEntry.aValue.onMouseMove(aMouseEvent.x, aMouseEvent.y))
					break;
			}
		}
	}

	registerEvent(
		eEventType: EventTrigger,
		aNotifierId: number | string,
		aEvent: EventBase,
	) {
		const sNotifierId = '' + aNotifierId;
		this.DBG('registerEvent', eEventType, sNotifierId);
		if (!this.aEventMap.has(eEventType)) {
			this.aEventMap.set(eEventType, new Map<string, EventBase[]>());
		}
		if (!this.aEventMap.get(eEventType).has(sNotifierId)) {
			this.aEventMap.get(eEventType).set(sNotifierId, []);
		}
		this.aEventMap.get(eEventType).get(sNotifierId).push(aEvent);
	}

	notifyEvent(eEventType: EventTrigger, aNotifierId: number | string) {
		const sNotifierId = '' + aNotifierId;
		this.DBG('notifyEvent', eEventType, sNotifierId);
		if (this.aEventMap.has(eEventType)) {
			if (this.aEventMap.get(eEventType).has(sNotifierId)) {
				const aEventArray = this.aEventMap.get(eEventType).get(sNotifierId);
				const nSize = aEventArray.length;
				for (let i = 0; i < nSize; ++i) {
					this.aTimerEventQueue.addEvent(aEventArray[i]);
				}
			}
		}
	}

	registerAnimationsEndHandler(aHandler: Handler0) {
		this.aAnimationsEndHandler = aHandler;
	}

	notifyAnimationsEndEvent() {
		if (this.aAnimationsEndHandler) this.aAnimationsEndHandler();
	}

	registerNextEffectEndHandler(aHandler: Handler0) {
		this.aSkipEffectEndHandlerSet.push(aHandler);
	}

	notifyNextEffectEndEvent() {
		const nSize = this.aSkipEffectEndHandlerSet.length;
		for (let i = 0; i < nSize; ++i) {
			this.aSkipEffectEndHandlerSet[i]();
		}
		this.aSkipEffectEndHandlerSet = [];
	}

	registerSkipEffectEvent(aEvent: DelayEvent) {
		this.aSkipEffectEvent = aEvent;
	}

	notifySkipEffectEvent() {
		if (this.aSkipEffectEvent) {
			this.aTimerEventQueue.addEvent(this.aSkipEffectEvent);
			this.aSkipEffectEvent = null;
		}
	}

	registerRewindCurrentEffectEvent(aEvent: DelayEvent) {
		this.aRewindCurrentEffectEvent = aEvent;
	}

	notifyRewindCurrentEffectEvent() {
		if (this.aRewindCurrentEffectEvent) {
			this.aTimerEventQueue.addEvent(this.aRewindCurrentEffectEvent);
			this.aRewindCurrentEffectEvent = null;
		}
	}

	registerRewindLastEffectEvent(aEvent: DelayEvent) {
		this.aRewindLastEffectEvent = aEvent;
	}

	notifyRewindLastEffectEvent() {
		if (this.aRewindLastEffectEvent) {
			this.aTimerEventQueue.addEvent(this.aRewindLastEffectEvent);
			this.aRewindLastEffectEvent = null;
		}
	}

	registerSkipInteractiveEffectEvent(nNotifierId: number, aEvent: DelayEvent) {
		this.aSkipInteractiveEffectEventSet.set(nNotifierId, aEvent);
	}

	notifySkipInteractiveEffectEvent(nNotifierId: number) {
		if (this.aSkipInteractiveEffectEventSet.has(nNotifierId)) {
			this.aTimerEventQueue.addEvent(
				this.aSkipInteractiveEffectEventSet.get(nNotifierId),
			);
		}
	}

	registerRewindRunningInteractiveEffectEvent(
		nNotifierId: number,
		aEvent: DelayEvent,
	) {
		this.aRewindRunningInteractiveEffectEventSet.set(nNotifierId, aEvent);
	}

	notifyRewindRunningInteractiveEffectEvent(nNotifierId: number) {
		if (this.aRewindRunningInteractiveEffectEventSet.has(nNotifierId)) {
			this.aTimerEventQueue.addEvent(
				this.aRewindRunningInteractiveEffectEventSet.get(nNotifierId),
			);
		}
	}

	registerRewindEndedInteractiveEffectEvent(
		nNotifierId: number,
		aEvent: DelayEvent,
	) {
		this.aRewindEndedInteractiveEffectEventSet.set(nNotifierId, aEvent);
	}

	notifyRewindEndedInteractiveEffectEvent(nNotifierId: number) {
		if (this.aRewindEndedInteractiveEffectEventSet.has(nNotifierId)) {
			this.aTimerEventQueue.addEvent(
				this.aRewindEndedInteractiveEffectEventSet.get(nNotifierId),
			);
		}
	}

	registerRewindedEffectHandler(
		aNotifierId: number | string,
		aHandler: Handler0,
	) {
		const sNotifierId = '' + aNotifierId;
		this.aRewindedEffectHandlerSet.set(sNotifierId, aHandler);
	}

	notifyRewindedEffectEvent(aNotifierId: number | string) {
		const sNotifierId = '' + aNotifierId;
		if (this.aRewindedEffectHandlerSet.has(sNotifierId)) {
			this.aRewindedEffectHandlerSet.get(sNotifierId)();
		}
	}

	registerElementChangedHandler(aNotifierId: string, aHandler: Handler1) {
		this.aElementChangedHandlerSet.set(aNotifierId, aHandler);
	}

	notifyElementChangedEvent(aNotifierId: string, aElement: any) {
		if (this.aElementChangedHandlerSet.has(aNotifierId)) {
			this.aElementChangedHandlerSet.get(aNotifierId)(aElement);
		}
	}

	DBG(
		sMethodName: string,
		eEventType: EventTrigger,
		nNotifierId: number | string,
		nTime?: number,
	) {
		if (aEventMultiplexerDebugPrinter.isEnabled()) {
			let sInfo = 'EventMultiplexer.' + sMethodName;
			sInfo += '( type: ' + EventTrigger[eEventType];
			sInfo += ', notifier: ' + nNotifierId + ' )';
			aEventMultiplexerDebugPrinter.print(sInfo, nTime);
		}
	}
}
