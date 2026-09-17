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
	///
	/// fontId, when present, identifies the exact font face the engine
	/// rendered this run with. The face's file can be fetched by that id
	/// and, once loaded, used in place of the family name for an exact
	/// match.
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
		/// How far each glyph advances from the one before it, in twips.
		/// Adding them up from zero gives each glyph's distance from the
		/// start of the run.
		advances?: number[];
		fontId?: string;
		/// Set when the face is not itself bold or italic, so the reader adds it.
		syntheticBold?: boolean;
		syntheticItalic?: boolean;
		/// 0-based index of the paragraph this portion belongs to.
		paragraph?: number;
		/// Alignment of the paragraph this portion belongs to.
		align?: 'left' | 'right' | 'center' | 'justify';
		/// Height and ascent of the line this portion sits on, in twips.
		lineHeight?: number;
		lineAscent?: number;
		/// Size of the area the text wraps within, in twips.
		textAreaWidth?: number;
		textAreaHeight?: number;
		/// Where this portion's line starts in the text area before alignment,
		/// and the width it wraps at, in twips. The indent is the paragraph
		/// indent, plus the first-line indent or the bullet on a first line.
		lineIndent?: number;
		lineWidth?: number;
	}

	export namespace TextSimplePortionPrimitive {
		export const type = 'textSimplePortion';
	}
}
