// @ts-strict-ignore
/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

enum TimingType {
	Unknown,
	Offset,
	WallClock,
	Indefinite,
	Event,
	SyncBase,
	Media,
}

enum EventTrigger {
	Unknown,
	OnBegin, // on slide begin
	OnEnd, // on slide end
	BeginEvent,
	EndEvent,
	OnClick,
	OnDblClick,
	OnMouseEnter,
	OnMouseLeave,
	OnNext, // on next effect
	OnPrev, // on previous effect
	Repeat,
}

function getEventTriggerType(sEventTrigger: string): EventTrigger {
	if (sEventTrigger == 'BeginEvent') return EventTrigger.BeginEvent;
	else if (sEventTrigger == 'EndEvent') return EventTrigger.EndEvent;
	else if (sEventTrigger == 'OnNext') return EventTrigger.OnNext;
	else if (sEventTrigger == 'OnPrev') return EventTrigger.OnPrev;
	else if (sEventTrigger == 'OnClick') return EventTrigger.OnClick;
	else return EventTrigger.Unknown;
}

class Timing {
	private eTimingType = TimingType.Unknown;
	private aAnimationNode: BaseNode;
	private sTimingDescription: string;
	private nOffset = 0.0;
	private sEventBaseElementId = '';
	private eEventType: EventTrigger = EventTrigger.Unknown;

	private static CHARCODE_PLUS = '+'.charCodeAt(0);
	private static CHARCODE_MINUS = '-'.charCodeAt(0);
	private static CHARCODE_0 = '0'.charCodeAt(0);
	private static CHARCODE_9 = '9'.charCodeAt(0);

	constructor(aAnimationNode: BaseNode, sTimingAttribute: string) {
		this.aAnimationNode = aAnimationNode; // the node, the timing attribute belongs to
		this.sTimingDescription = removeWhiteSpaces(sTimingAttribute);

		this.nOffset = 0.0; // in seconds
		this.sEventBaseElementId = ''; // the element id for event based timing
	}

	getAnimationNode() {
		return this.aAnimationNode;
	}

	getType() {
		return this.eTimingType;
	}

	getOffset() {
		return this.nOffset;
	}

	getEventBaseElementId() {
		return this.sEventBaseElementId;
	}

	getEventType() {
		return this.eEventType;
	}

	parse() {
		if (!this.sTimingDescription) {
			this.eTimingType = TimingType.Offset;
			return;
		}

		if (this.sTimingDescription == 'indefinite')
			this.eTimingType = TimingType.Indefinite;
		else {
			const nFirstCharCode = this.sTimingDescription.charCodeAt(0);
			let bPositiveOffset = !(nFirstCharCode == Timing.CHARCODE_MINUS);
			if (
				nFirstCharCode == Timing.CHARCODE_PLUS ||
				nFirstCharCode == Timing.CHARCODE_MINUS ||
				(nFirstCharCode >= Timing.CHARCODE_0 &&
					nFirstCharCode <= Timing.CHARCODE_9)
			) {
				const sClockValue =
					nFirstCharCode == Timing.CHARCODE_PLUS ||
					nFirstCharCode == Timing.CHARCODE_MINUS
						? this.sTimingDescription.substring(1)
						: this.sTimingDescription;

				const TimeInSec = Timing.parseClockValue(sClockValue);
				if (TimeInSec != undefined) {
					this.eTimingType = TimingType.Offset;
					this.nOffset = bPositiveOffset ? TimeInSec : -TimeInSec;
				}
			} else {
				let aTimingSplit = [];
				bPositiveOffset = true;
				if (this.sTimingDescription.indexOf('+') != -1) {
					aTimingSplit = this.sTimingDescription.split('+');
				} else if (this.sTimingDescription.indexOf('-') != -1) {
					aTimingSplit = this.sTimingDescription.split('-');
					bPositiveOffset = false;
				} else {
					aTimingSplit[0] = this.sTimingDescription;
					aTimingSplit[1] = '';
				}

				if (aTimingSplit[0].indexOf('.') != -1) {
					const aEventSplit = aTimingSplit[0].split('.');
					this.sEventBaseElementId = aEventSplit[0];
					this.eEventType = getEventTriggerType(aEventSplit[1]);
				} else {
					this.eEventType = getEventTriggerType(aTimingSplit[0]);
				}

				if (this.eEventType == EventTrigger.Unknown) return;

				if (
					this.eEventType == EventTrigger.BeginEvent ||
					this.eEventType == EventTrigger.EndEvent
				) {
					this.eTimingType = TimingType.SyncBase;
				} else {
					this.eTimingType = TimingType.Event;
				}

				if (aTimingSplit[1]) {
					const sClockValue = aTimingSplit[1];
					const TimeInSec = Timing.parseClockValue(sClockValue);
					if (TimeInSec != undefined) {
						this.nOffset = bPositiveOffset ? TimeInSec : -TimeInSec;
					} else {
						this.eTimingType = TimingType.Unknown;
					}
				}
			}
		}
	}

	public static parseClockValue(sClockValue: string) {
		if (!sClockValue) return 0.0;

		let nTimeInSec = undefined;

		const reFullClockValue = /^([0-9]+):([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;
		const rePartialClockValue = /^([0-5][0-9]):([0-5][0-9])(.[0-9]+)?$/;
		const reTimeCountValue = /^([0-9]+)(.[0-9]+)?(h|min|s|ms)?$/;

		if (reFullClockValue.test(sClockValue)) {
			const aClockTimeParts = reFullClockValue.exec(sClockValue);

			const nHours = parseInt(aClockTimeParts[1]);
			const nMinutes = parseInt(aClockTimeParts[2]);
			let nSeconds = parseInt(aClockTimeParts[3]);
			if (aClockTimeParts[4]) nSeconds += parseFloat(aClockTimeParts[4]);

			nTimeInSec = (nHours * 60 + nMinutes) * 60 + nSeconds;
		} else if (rePartialClockValue.test(sClockValue)) {
			const aClockTimeParts = rePartialClockValue.exec(sClockValue);

			const nMinutes = parseInt(aClockTimeParts[1]);
			let nSeconds = parseInt(aClockTimeParts[2]);
			if (aClockTimeParts[3]) nSeconds += parseFloat(aClockTimeParts[3]);

			nTimeInSec = nMinutes * 60 + nSeconds;
		} else if (reTimeCountValue.test(sClockValue)) {
			const aClockTimeParts = reTimeCountValue.exec(sClockValue);

			let nTimeCount = parseInt(aClockTimeParts[1]);
			if (aClockTimeParts[2]) nTimeCount += parseFloat(aClockTimeParts[2]);

			if (aClockTimeParts[3]) {
				if (aClockTimeParts[3] == 'h') {
					nTimeInSec = nTimeCount * 3600;
				} else if (aClockTimeParts[3] == 'min') {
					nTimeInSec = nTimeCount * 60;
				} else if (aClockTimeParts[3] == 's') {
					nTimeInSec = nTimeCount;
				} else if (aClockTimeParts[3] == 'ms') {
					nTimeInSec = nTimeCount / 1000;
				}
			} else {
				nTimeInSec = nTimeCount;
			}
		}

		if (nTimeInSec) nTimeInSec = parseFloat(nTimeInSec.toFixed(3));
		return nTimeInSec;
	}

	info(bVerbose: boolean = false) {
		let sInfo = '';

		if (bVerbose) {
			sInfo = 'description: ' + this.sTimingDescription + ', ';

			sInfo += ', type: ' + TimingType[this.getType()];
			sInfo += ', offset: ' + this.getOffset();
			sInfo += ', event base element id: ' + this.getEventBaseElementId();
			sInfo += ', timing event type: ' + EventTrigger[this.getEventType()];
		} else {
			switch (this.getType()) {
				case TimingType.Indefinite:
					sInfo += 'indefinite';
					break;
				case TimingType.Offset:
					sInfo += this.getOffset();
					break;
				case TimingType.Event:
				case TimingType.SyncBase:
					if (this.getEventBaseElementId())
						sInfo += this.getEventBaseElementId() + '.';
					sInfo += EventTrigger[this.getEventType()];
					if (this.getOffset()) {
						if (this.getOffset() > 0) sInfo += '+';
						sInfo += this.getOffset();
					}
					break;
			}
		}
		return sInfo;
	}
}
