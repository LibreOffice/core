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
 * Moves the caret from the cached runs alone: by one character inside its paragraph, or to
 * where a click lands.
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

	// The runs that show paragraph text, without fields, over all paragraphs.
	function shownRuns(runs: TextRun[]): TextRun[] {
		const paragraphs = new Set<number>();
		for (const run of runs)
			if (run.paragraph !== undefined) paragraphs.add(run.paragraph);
		const shown: TextRun[] = [];
		for (const paragraph of paragraphs)
			shown.push(...paragraphTextRuns(runs, paragraph));
		return shown;
	}

	// True when two runs are on the same line. A superscript or subscript moves its baseline by
	// less than half a line height.
	function onSameLine(a: TextRun, b: TextRun): boolean {
		const height = Math.min(a.lineHeight * a.scaleY, b.lineHeight * b.scaleY);
		return Math.abs(a.baseline - b.baseline) < height / 2;
	}

	// True when the run ends at a soft line break: another line of its paragraph starts there.
	function endsWrappedLine(run: TextRun, runs: TextRun[]): boolean {
		return runs.some(
			(other) =>
				other !== run &&
				other.paragraph === run.paragraph &&
				other.start === run.end &&
				!onSameLine(other, run),
		);
	}

	// The grapheme boundary in the run that is closest to the slide x.
	function nearestGraphemeBoundary(
		run: TextRun,
		positions: number[],
		x: number,
	): number {
		const text = run.text;
		let best = run.start;
		let bestDistance = Infinity;
		let index: number | null = run.start;
		while (index !== null && index <= run.end) {
			const distance = Math.abs(x - run.slideX(positions[index - run.start]));
			if (distance < bestDistance) {
				best = index;
				bestDistance = distance;
			}
			if (index === run.end) break;
			index = Math.min(graphemeBoundary(text, index, 1) ?? run.end, run.end);
		}
		return best;
	}

	/// The runs of one object, each measured once, for the caret at a pointer.
	export class ObjectText {
		// The runs that show paragraph text, without fields.
		private readonly shown: TextRun[];
		private readonly positions = new Map<TextRun, number[]>();

		private constructor(
			// The cached object the runs were collected from.
			private readonly object: SlideObject,
			private readonly runs: TextRun[],
			private readonly measurer: TextMeasurer,
		) {
			this.shown = shownRuns(runs);
		}

		/// The text of the object, or null when the tree lacks it or some of its text is
		/// rotated.
		static of(tree: PredictionTree, objectId: number): ObjectText | null {
			const object = tree.data.objects.get(objectId);
			if (!object) return null;
			const runs = TextRun.collect(object.primitives);
			if (runs.some((run) => run.rotated)) return null;
			return new ObjectText(object, runs, tree.measurer);
		}

		/// True while the tree still holds the object this text was collected from.
		isOf(tree: PredictionTree, objectId: number): boolean {
			return tree.data.objects.get(objectId) === this.object;
		}

		// The local x before each character of the run, measured on first use.
		private positionsOf(run: TextRun): number[] {
			let positions = this.positions.get(run);
			if (!positions) {
				positions = run.characterPositions(this.measurer);
				this.positions.set(run, positions);
			}
			return positions;
		}

		/// The caret where a click at slide (x, y) lands, or null off the text. Past the
		/// end of a wrapped line it goes before the last character, as in the engine.
		caretAtPoint(x: number, y: number): LogicalCaret | null {
			let target: { run: TextRun; positions: number[] } | null = null;
			let bestDelta = Infinity;
			for (const run of this.shown) {
				if (!onLine(run, y, y)) continue;
				const positions = this.positionsOf(run);
				const left = run.x;
				const right = run.slideX(positions[positions.length - 1]);
				const delta = x < left ? left - x : x > right ? x - right : 0;
				// A click further than a font size from the text is in the empty
				// part of the box.
				if (delta < run.fontSize && delta < bestDelta) {
					bestDelta = delta;
					target = { run, positions };
				}
			}
			if (!target) return null;

			const run = target.run;
			const paragraph = run.paragraph;
			if (paragraph === undefined || rightToLeft.test(run.text)) return null;

			let index = nearestGraphemeBoundary(run, target.positions, x);
			if (index === run.end && endsWrappedLine(run, this.runs))
				index = Math.max(
					run.start,
					graphemeBoundary(run.text, index, -1) ?? index,
				);
			return { paragraph, index };
		}
	}
}
