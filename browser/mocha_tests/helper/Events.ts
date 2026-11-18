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

class HandlerData {
	public numCalls: number;
	public event?: EventBaseType;

	constructor() {
		this.numCalls = 0;
		this.event = null;
	}
}

class DerivedEvented extends Evented {
	public first: HandlerData;
	public second: HandlerData;

	constructor() {
		super();
		this.reset();
	}

	public reset(): void {
		this.first = new HandlerData();
		this.second = new HandlerData();
	}

	public handler1(e: EventBaseType): void {
		this.first.numCalls++;
		this.first.event = e;
	}

	public handler2(e: EventBaseType): void {
		this.second.numCalls++;
		this.second.event = e;
	}
}

class ForeignContext {
	public first: HandlerData;
	public second: HandlerData;

	constructor() {
		this.reset();
	}

	public reset(): void {
		this.first = new HandlerData();
		this.second = new HandlerData();
	}

	public handler1(e: EventBaseType): void {
		this.first.numCalls++;
		this.first.event = e;
	}

	public handler2(e: EventBaseType): void {
		this.second.numCalls++;
		this.second.event = e;
	}
}

interface EventRegistrationData {
	eventList: string;
	handler: CEventListener;
}
