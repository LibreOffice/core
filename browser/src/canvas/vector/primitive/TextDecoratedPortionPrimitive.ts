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
	/// A run of text with extra decorations: underline, overline,
	/// strikeout, emphasis marks, relief and shadow. Carries the same
	/// fields as textSimplePortion plus the decoration ones.
	///
	/// underline, overline, strikeout, emphasisMark and relief are
	/// numeric line-style codes. Any non-zero value means the
	/// decoration is present.
	export interface TextDecoratedPortionPrimitive extends Omit<
		TextSimplePortionPrimitive,
		'type'
	> {
		type: typeof TextDecoratedPortionPrimitive.type;
		underline?: number;
		underlineColor?: string;
		underlineAbove?: boolean;
		overline?: number;
		overlineColor?: string;
		strikeout?: number;
		emphasisMark?: number;
		emphasisMarkAbove?: boolean;
		emphasisMarkBelow?: boolean;
		relief?: number;
		shadow?: boolean;
		wordLineMode?: boolean;
	}

	export namespace TextDecoratedPortionPrimitive {
		export const type = 'textDecoratedPortion';
	}
}
