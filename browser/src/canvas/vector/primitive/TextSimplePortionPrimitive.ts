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
	/// A run of text positioned by an affine transform matrix
	/// [a, b, c, d, e, f]. The substring to paint is
	/// text.substring(textPosition, textPosition + textLength).
	///
	/// fontSize is the Y-scale of the matrix in user units. fontcolor
	/// is the text fill. fillcolor, when present, is a background
	/// highlight behind the run.
	///
	/// Font shape: familyname, optional stylename, weight as a number
	/// from 0 to 10 where 0 is unknown and 5 is normal, and italic,
	/// outline, rtl and monospaced as optional booleans.
	export interface TextSimplePortionPrimitive extends Primitive {
		type: typeof TextSimplePortionPrimitive.type;
		text?: string;
		textPosition?: number;
		textLength?: number;
		matrix?: number[];
		fontSize?: number;
		fontcolor?: string;
		fillcolor?: string;
		letterSpacing?: number;
		familyname?: string;
		stylename?: string;
		weight?: number;
		italic?: boolean;
		outline?: boolean;
		rtl?: boolean;
		monospaced?: boolean;
		dxarray?: number[];
	}

	export namespace TextSimplePortionPrimitive {
		export const type = 'textSimplePortion';
	}
}
