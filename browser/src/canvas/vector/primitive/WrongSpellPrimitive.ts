/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	/// The mark under a misspelled word: the line from start to end that its wave
	/// follows, and the font height of the word, in twips.
	export interface WrongSpellPrimitive extends Primitive {
		type: typeof WrongSpellPrimitive.type;
		color?: string;
		fontHeight?: number;
		startX?: number;
		startY?: number;
		endX?: number;
		endY?: number;
	}

	export namespace WrongSpellPrimitive {
		export const type = 'wrongSpell';
	}
}
