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

class Duration {
	private bIndefinite: boolean;
	private bMedia: boolean;
	private nValue: number;
	private bDefined: boolean;

	constructor(sDuration: string) {
		this.bIndefinite = false;
		this.bMedia = false;
		this.nValue = undefined;
		this.bDefined = false;

		if (!sDuration) return;

		if (sDuration === 'indefinite') this.bIndefinite = true;
		else if (sDuration === 'media') this.bMedia = true;
		else {
			this.nValue = Timing.parseClockValue(sDuration);
			if (this.nValue <= 0.0) this.nValue = 0.001; // duration must be always greater than 0
		}
		this.bDefined = true;
	}

	isSet() {
		return this.bDefined;
	}

	isIndefinite() {
		return this.bIndefinite;
	}

	isMedia() {
		return this.bMedia;
	}

	isValue() {
		return this.nValue != undefined;
	}

	getValue() {
		return this.nValue;
	}

	info() {
		var sInfo;

		if (this.isIndefinite()) sInfo = 'indefinite';
		else if (this.isMedia()) sInfo = 'media';
		else if (this.getValue()) sInfo = this.getValue();
		return sInfo;
	}
}
