/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Moves the caret by one character inside its paragraph, from the cached runs alone.
 *
 * A step goes over one grapheme cluster, as the engine's caret does, so a base letter and its
 * combining marks, or an emoji sequence, count as one character. A field counts as one character
 * too. Text written right to left, and rotated text, are left to the engine.
 */

namespace cool {
	// Characters of scripts written right to left, and the controls that change the direction.
	const rightToLeft =
		/[\u0590-\u08ff\ufb1d-\ufdff\ufe70-\ufefc\u200e\u200f\u202a-\u202e\u2066-\u2069]/;

	// The part of Intl.Segmenter that splits text into grapheme clusters. It is typed here, as
	// the type libraries of some builds are older than it.
	interface GraphemeSegmenter {
		segment(text: string): {
			containing(index: number): { index: number; segment: string } | undefined;
		};
	}

	let graphemes: GraphemeSegmenter | null | undefined;

	function graphemeSegmenter(): GraphemeSegmenter | null {
		if (graphemes === undefined) {
			const Segmenter = (Intl as any).Segmenter;
			graphemes =
				typeof Segmenter === 'function'
					? (new Segmenter(undefined, {
							granularity: 'grapheme',
						}) as GraphemeSegmenter)
					: null;
		}
		return graphemes;
	}

	// The boundary one grapheme cluster from index, or null at the edge of the text. Without
	// grapheme support, a step goes over one code point.
	function graphemeBoundary(
		text: string,
		index: number,
		step: -1 | 1,
	): number | null {
		if (step > 0 ? index >= text.length : index <= 0) return null;

		const segmenter = graphemeSegmenter();
		if (segmenter) {
			const segment = segmenter
				.segment(text)
				.containing(step > 0 ? index : index - 1);
			if (!segment) return null;
			return step > 0 ? segment.index + segment.segment.length : segment.index;
		}

		const next = index + step;
		const splitsPair =
			next > 0 &&
			next < text.length &&
			isHighSurrogate(text.charCodeAt(next - 1)) &&
			isLowSurrogate(text.charCodeAt(next));
		return splitsPair ? next + step : next;
	}

	/// The caret one character before (step -1) or after (step 1) logical, or null when the
	/// step leaves the paragraph, no run shows the new offset, or the paragraph is rotated or
	/// right to left.
	export function stepInParagraph(
		position: CaretPosition,
		logical: LogicalCaret,
		step: -1 | 1,
	): LogicalCaret | null {
		const paragraph = logical.paragraph;
		const runs = position.objectRuns;
		if (runs.some((run) => run.paragraph === paragraph && run.rotated))
			return null;

		const textRuns = paragraphTextRuns(runs, paragraph);
		if (!textRuns.length) return null;

		const text = textRuns[0].text;
		if (rightToLeft.test(text)) return null;

		const next = graphemeBoundary(text, logical.index, step);
		if (next === null) return null;

		// An offset that no run covers, such as one before a leading tab, has no place for
		// the caret in the runs.
		const shown = textRuns.some((run) => next >= run.start && next <= run.end);
		return shown ? { paragraph, index: next } : null;
	}
}
