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

namespace cool {
	/// A run of text positioned by an affine transform matrix
	/// [a, b, c, d, e, f]. The substring to paint is
	/// text.substring(textPosition, textPosition + textLength).
	///
	/// fontSize is the Y-scale of the matrix in user units. fontcolor
	/// is the text fill. fillcolor, when present, is a background
	/// highlight behind the run, and highlight the area it covers.
	/// backdrop, when present, is the fill right under the text, such
	/// as the fill of its shape or of its table cell, and
	/// backdropTransparency the share of what lies behind that fill
	/// that shows through, from 0 to 1, absent for a fill that covers
	/// it fully. Without a backdrop the run sits on
	/// whatever the page shows there.
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
		/// The number of the text among the distinct texts of the entry, when the entry
		/// carries each of them once.
		textId?: number;
		/// As received, the textId of the portion of the same entry that carries the text
		/// of this one. This portion has the same text as that portion.
		textRef?: number;
		textPosition?: number;
		textLength?: number;
		matrix?: number[];
		fontSize?: number;
		fontcolor?: string;
		fillcolor?: string;
		highlight?: TextHighlight;
		backdrop?: string;
		backdropTransparency?: number;
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
		/// In vertical text, the characters that stand upright, as [from, to)
		/// ranges of UTF-16 units of the visible text. The others lie on their
		/// side along the turned baseline.
		upright?: number[][];
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
		/// True when the text is a string of blanks of its own in place of a
		/// range of the paragraph text. Such blanks are there for what is
		/// drawn along them: an underline, an overline, a strikeout or a
		/// background. textPosition counts in that string.
		fillerBlanks?: boolean;
	}

	export namespace TextSimplePortionPrimitive {
		export const type = 'textSimplePortion';
	}
}
