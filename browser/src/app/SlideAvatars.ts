/* -*- js-indent-level: 8 -*- */
/* global app */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface AvatarSlots {
	faces: number;
	hidden: number;
}

class SlideAvatars {
	// Circles drawn over one preview, counter included.
	public static readonly slotCount: number = 3;

	// The counter carries one digit, so the chip stays a circle.
	public static readonly counterCap: number = 9;

	public static slots(
		people: number,
		slotCount: number = SlideAvatars.slotCount,
	): AvatarSlots {
		if (!(people > 0) || !(slotCount > 0)) return { faces: 0, hidden: 0 };
		if (people <= slotCount) return { faces: people, hidden: 0 };
		if (slotCount === 1) return { faces: 0, hidden: people };
		return { faces: slotCount - 1, hidden: people - (slotCount - 1) };
	}

	// Nine stands for nine or more.
	public static counterText(hidden: number): string {
		return '+' + Math.min(hidden, SlideAvatars.counterCap);
	}
}

app.SlideAvatars = SlideAvatars;
