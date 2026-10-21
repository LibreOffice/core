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
 * Moves the caret from the cached runs alone: by one character, by one word, by one line, or to
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

	// The part of Intl.Segmenter that the caret moves use, typed here for older type libraries.
	interface Segment {
		index: number;
		segment: string;
		/// With word granularity, true for a word and false for whitespace or punctuation.
		isWordLike?: boolean;
	}

	interface Segmenter {
		segment(
			text: string,
		): Iterable<Segment> & { containing(index: number): Segment | undefined };
	}

	// One segmenter for each granularity, made on first use, or null without Intl.Segmenter.
	const segmenters = new Map<string, Segmenter | null>();

	function segmenter(granularity: 'grapheme' | 'word'): Segmenter | null {
		let found = segmenters.get(granularity);
		if (found === undefined) {
			const Segmenter = (Intl as any).Segmenter;
			found =
				typeof Segmenter === 'function'
					? (new Segmenter(undefined, { granularity }) as Segmenter)
					: null;
			segmenters.set(granularity, found);
		}
		return found;
	}

	// The boundary one grapheme cluster from index, or null at the edge of the text. Without
	// grapheme support, a step goes over one code point.
	function graphemeBoundary(
		text: string,
		index: number,
		step: -1 | 1,
	): number | null {
		if (step > 0 ? index >= text.length : index <= 0) return null;

		const graphemes = segmenter('grapheme');
		if (graphemes) {
			const segment = graphemes
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

	// The runs of a paragraph that a caret can move in, or null when the paragraph has no runs,
	// is rotated or is written right to left.
	function movableRuns(runs: TextRun[], paragraph: number): TextRun[] | null {
		if (runs.some((run) => run.paragraph === paragraph && run.rotated))
			return null;
		const textRuns = paragraphTextRuns(runs, paragraph);
		if (!textRuns.length || rightToLeft.test(textRuns[0].text)) return null;
		return textRuns;
	}

	// True when a run shows the paragraph offset. An offset before a leading tab is in none.
	function isShown(textRuns: TextRun[], index: number): boolean {
		return textRuns.some((run) => index >= run.start && index <= run.end);
	}

	// The offsets where words start in the text, or null without word support. Punctuation
	// counts as a word and whitespace does not, as in the engine.
	function wordStarts(text: string): number[] | null {
		const words = segmenter('word');
		if (!words) return null;
		const starts: number[] = [];
		for (const segment of words.segment(text))
			if (segment.segment.trim() !== '') starts.push(segment.index);
		return starts;
	}

	// Characters the engine splits into words by its own rules or a dictionary: punctuation,
	// symbols and scripts written without spaces.
	const engineSplitCharacters =
		/[\u0021-\u002f\u003a-\u0040\u005b-\u0060\u007b-\u00bf\u00d7\u00f7\u0e00-\u0eff\u1000-\u109f\u1100-\u11ff\u1780-\u17ff\u2000-\u206f\u3000-\u30ff\u3400-\u4dbf\u4e00-\u9fff\uac00-\ud7af\ud800-\udfff]/;

	/// The anchor and the caret of a selection.
	export interface CaretSpan {
		anchor: LogicalCaret;
		caret: LogicalCaret;
	}

	/// The word a double click at logical selects in the engine: the word that covers or starts
	/// at the caret, or the whitespace there, anchored at its start. Null at the end of the
	/// paragraph, and for a word with punctuation or a symbol, in a script that needs a
	/// dictionary, or in a rotated or right to left paragraph.
	export function wordAt(
		position: CaretPosition,
		logical: LogicalCaret,
	): CaretSpan | null {
		const textRuns = movableRuns(position.objectRuns, logical.paragraph);
		if (!textRuns) return null;
		const words = segmenter('word');
		if (!words) return null;
		const segment = words.segment(textRuns[0].text).containing(logical.index);
		if (!segment) return null;
		const whitespace = segment.segment.trim() === '';
		if (!whitespace && !segment.isWordLike) return null;
		if (!whitespace && engineSplitCharacters.test(segment.segment)) return null;

		const start = segment.index;
		const end = start + segment.segment.length;
		if (!isShown(textRuns, start) || !isShown(textRuns, end)) return null;
		const paragraph = logical.paragraph;
		return {
			anchor: { paragraph, index: start },
			caret: { paragraph, index: end },
		};
	}

	/// The paragraph a triple click at logical selects, anchored at its start. Null when no run
	/// shows one of its ends, or it is rotated or right to left.
	export function paragraphAt(
		position: CaretPosition,
		logical: LogicalCaret,
	): CaretSpan | null {
		const textRuns = movableRuns(position.objectRuns, logical.paragraph);
		if (!textRuns) return null;
		const end = textRuns[0].text.length;
		if (!isShown(textRuns, 0) || !isShown(textRuns, end)) return null;
		const paragraph = logical.paragraph;
		return {
			anchor: { paragraph, index: 0 },
			caret: { paragraph, index: end },
		};
	}

	/// The caret one word before (step -1) or after (step 1) logical: back to the start of the
	/// word at or before the caret, forward to the start of the next word, and into the next or
	/// previous paragraph at an edge, as in the engine. Null when no run shows the new place,
	/// or that paragraph is empty, rotated or right to left.
	export function stepByWord(
		position: CaretPosition,
		logical: LogicalCaret,
		step: -1 | 1,
	): LogicalCaret | null {
		const runs = position.objectRuns;
		const textRuns = movableRuns(runs, logical.paragraph);
		if (!textRuns) return null;
		const text = textRuns[0].text;
		const starts = wordStarts(text);
		if (!starts) return null;

		let index: number;
		if (step > 0) {
			index = starts.find((start) => start > logical.index) ?? text.length;
		} else {
			if (logical.index === 0) {
				const previousRuns = movableRuns(runs, logical.paragraph - 1);
				if (!previousRuns) return null;
				const end = previousRuns[0].text.length;
				return isShown(previousRuns, end)
					? { paragraph: logical.paragraph - 1, index: end }
					: null;
			}
			const before = starts.filter((start) => start < logical.index);
			index = before.length ? before[before.length - 1] : 0;
		}

		if (step > 0 && index >= text.length) {
			const nextRuns = movableRuns(runs, logical.paragraph + 1);
			// Only the engine knows whether a next paragraph without runs exists.
			if (!nextRuns) return null;
			return isShown(nextRuns, 0)
				? { paragraph: logical.paragraph + 1, index: 0 }
				: null;
		}
		return isShown(textRuns, index)
			? { paragraph: logical.paragraph, index }
			: null;
	}

	/// The caret one character before (step -1) or after (step 1) logical, into the next or
	/// previous paragraph at an edge. Null when no run shows the new place, or that paragraph
	/// is empty, rotated or right to left.
	export function stepByCharacter(
		position: CaretPosition,
		logical: LogicalCaret,
		step: -1 | 1,
	): LogicalCaret | null {
		const runs = position.objectRuns;
		const textRuns = movableRuns(runs, logical.paragraph);
		if (!textRuns) return null;

		const next = graphemeBoundary(textRuns[0].text, logical.index, step);
		if (next !== null)
			return isShown(textRuns, next)
				? { paragraph: logical.paragraph, index: next }
				: null;

		const paragraph = logical.paragraph + step;
		const nextRuns = paragraph >= 0 ? movableRuns(runs, paragraph) : null;
		if (!nextRuns) return null;
		const index = step > 0 ? 0 : nextRuns[0].text.length;
		return isShown(nextRuns, index) ? { paragraph, index } : null;
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

	// The caret in lineRuns closest to the slide x, or null. With nearOnly, runs more than a
	// font size from x are skipped. At the end of a wrapped line the caret goes before the last
	// character, as in the engine.
	function caretOnLine(
		lineRuns: TextRun[],
		objectRuns: TextRun[],
		positionsOf: (run: TextRun) => number[],
		x: number,
		nearOnly: boolean,
	): LogicalCaret | null {
		let target: { run: TextRun; positions: number[] } | null = null;
		let bestDelta = Infinity;
		for (const run of lineRuns) {
			const positions = positionsOf(run);
			const left = run.x;
			const right = run.slideX(positions[positions.length - 1]);
			const delta = x < left ? left - x : x > right ? x - right : 0;
			// The font size scaled to slide twips, like delta.
			const near = delta < run.fontSize * run.scaleX;
			if ((!nearOnly || near) && delta < bestDelta) {
				bestDelta = delta;
				target = { run, positions };
			}
		}
		if (!target) return null;

		const run = target.run;
		const paragraph = run.paragraph;
		if (paragraph === undefined || rightToLeft.test(run.text)) return null;

		let index = nearestGraphemeBoundary(run, target.positions, x);
		if (index === run.end && endsWrappedLine(run, objectRuns))
			index = Math.max(
				run.start,
				graphemeBoundary(run.text, index, -1) ?? index,
			);
		return { paragraph, index };
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
		static collectFrom(
			tree: PredictionTree,
			objectId: number,
		): ObjectText | null {
			const object = tree.data.objects.get(objectId);
			if (!object) return null;
			const runs = TextRun.collect(object.primitives);
			if (runs.some((run) => run.rotated)) return null;
			return new ObjectText(object, runs, tree.measurer);
		}

		/// True while the tree still holds the object this text was collected from.
		isCollectedFrom(tree: PredictionTree, objectId: number): boolean {
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

		/// The caret where a click at slide (x, y) lands, or null off the text.
		caretAtPoint(x: number, y: number): LogicalCaret | null {
			// A click further than a font size from the text is in the empty part of
			// the box.
			const lineRuns = this.shown.filter((run) => onLine(run, y, y));
			return caretOnLine(
				lineRuns,
				this.runs,
				(run) => this.positionsOf(run),
				x,
				true,
			);
		}

		/// The caret a drag at slide (x, y) reaches, or null. A point off the lines goes to
		/// the closest one.
		caretNearPoint(x: number, y: number): LogicalCaret | null {
			let closest: TextRun | null = null;
			let lineDistance = Infinity;
			for (const run of this.shown) {
				const top = run.baseline - run.lineAscent * run.scaleY;
				const bottom = top + run.lineHeight * run.scaleY;
				const distance = y < top ? top - y : y > bottom ? y - bottom : 0;
				if (distance < lineDistance) {
					lineDistance = distance;
					closest = run;
				}
			}
			if (closest === null) return null;
			const line = closest;
			return caretOnLine(
				this.shown.filter((run) => onSameLine(run, line)),
				this.runs,
				(run) => this.positionsOf(run),
				x,
				false,
			);
		}

		/// The rectangles that cover the text between two carets, one for each run that
		/// shows some of it, in reading order.
		selectionRectangles(
			a: LogicalCaret,
			b: LogicalCaret,
		): SelectionRectangle[] {
			const [from, to] = compareCarets(a, b) <= 0 ? [a, b] : [b, a];

			const rectangles: SelectionRectangle[] = [];
			const runs = [...this.shown].sort(
				(first, second) =>
					first.baseline - second.baseline || first.x - second.x,
			);
			for (const run of runs) {
				const paragraph = run.paragraph;
				if (paragraph === undefined) continue;
				if (paragraph < from.paragraph || paragraph > to.paragraph) continue;
				const start = Math.max(
					run.start,
					paragraph === from.paragraph ? from.index : run.start,
				);
				const end = Math.min(
					run.end,
					paragraph === to.paragraph ? to.index : run.end,
				);
				if (end <= start) continue;

				const positions = this.positionsOf(run);
				const left = run.slideX(positions[start - run.start]);
				const right = run.slideX(positions[end - run.start]);
				rectangles.push({
					x: Math.min(left, right),
					y: run.baseline - run.lineAscent * run.scaleY,
					width: Math.abs(right - left),
					height: run.lineHeight * run.scaleY,
				});
			}
			return rectangles;
		}

		/// The caret on the line above (step -1) or below (step 1) the run's line, closest
		/// to the slide x. Null when there is no such line, or it is past a paragraph
		/// without text.
		caretOnNextLine(
			from: TextRun,
			x: number,
			step: -1 | 1,
		): LogicalCaret | null {
			const paragraph = from.paragraph;
			if (paragraph === undefined) return null;

			// The nearest run off the caret's line in the direction of the step.
			let next: TextRun | null = null;
			for (const run of this.shown) {
				if (onSameLine(run, from)) continue;
				const ahead =
					step > 0
						? run.baseline > from.baseline
						: run.baseline < from.baseline;
				const closer =
					next === null ||
					(step > 0
						? run.baseline < next.baseline
						: run.baseline > next.baseline);
				if (ahead && closer) next = run;
			}
			if (next === null) return null;

			const line = next;
			const caret = caretOnLine(
				this.shown.filter(
					(run) => onSameLine(run, line) && !onSameLine(run, from),
				),
				this.runs,
				(run) => this.positionsOf(run),
				x,
				false,
			);
			if (!caret || Math.abs(caret.paragraph - paragraph) > 1) return null;
			return caret;
		}
	}

	/// Negative when caret a is before b, positive when after, 0 when they are equal.
	export function compareCarets(a: LogicalCaret, b: LogicalCaret): number {
		return a.paragraph - b.paragraph || a.index - b.index;
	}

	/// A rectangle of selected text, in slide twips.
	export interface SelectionRectangle {
		x: number;
		y: number;
		width: number;
		height: number;
	}
}
