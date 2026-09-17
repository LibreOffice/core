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
 * Edits the cached tree as the user types, so text shows before the server re-renders.
 *
 * Existing glyphs keep the server's advances, and typed text is measured. Runs after the edit
 * on its line shift by the change in width. A line that crosses its wrap width re-wraps the
 * paragraph. Rotated runs are left to the server.
 */

namespace cool {
	/// True for text without line breaks and tabs, which need the server's layout.
	export function canPredictText(text: string): boolean {
		return (
			!!text &&
			text.indexOf('\n') === -1 &&
			text.indexOf('\r') === -1 &&
			text.indexOf('\t') === -1
		);
	}

	/// The UTF-16 units removed, those before the caret, and the caret move in slide twips.
	export interface PredictedDeletion {
		removed: number;
		removedBefore: number;
		caretShift: number;
	}

	/// The drawn caret in slide twips.
	export interface CaretGeometry {
		x: number;
		top: number;
		height: number;
	}

	// Steps count code points from index towards limit and returns where it stops.
	function stepCodePoints(
		text: string,
		index: number,
		count: number,
		limit: number,
	): number {
		const direction = limit < index ? -1 : 1;
		for (; count > 0 && index !== limit; count--) {
			index += direction;
			if (
				index !== limit &&
				isLowSurrogate(text.charCodeAt(index)) &&
				isHighSurrogate(text.charCodeAt(index - 1))
			)
				index += direction;
		}
		return index;
	}

	/// An offset in a run where edits apply. positions[k] is the local x before character k.
	export class CaretPosition {
		constructor(
			readonly run: TextRun,
			readonly offset: number,
			readonly rightward: TextRun[],
			readonly positions: number[],
			// Every run of the run's object.
			readonly objectRuns: TextRun[],
			readonly measurer: TextMeasurer,
			// The id of the run's object, when the tree has ids.
			readonly objectId?: number,
		) {}

		/// True when the position ends its run, which at a line break also starts the next.
		get atEnd(): boolean {
			return this.offset > 0 && this.offset === this.positions.length - 1;
		}

		/// The caret at this position, in slide twips.
		geometry(): CaretGeometry {
			const run = this.run;
			const x = run.slideX(this.positions[this.offset] ?? 0);
			const ascent = run.lineAscent * run.scaleY;
			return {
				x,
				top: run.baseline - ascent,
				height: run.lineHeight * run.scaleY,
			};
		}

		/// Inserts text and returns its width in slide twips, or 0 when nothing changed.
		insert(text: string): number {
			if (!canPredictText(text)) return 0;
			const paragraph = Paragraph.at(this);

			// Typed text is measured and scaled to the server's measure.
			const scale = this.measurer.serverScale(this.run);
			const advance = this.measurer.width(this.run.primitive, text) * scale;
			// Text without width is left to the server.
			if (!(advance > 0)) return 0;

			const overflows =
				paragraph !== null && paragraph.lineOverflows(this, advance);

			this.insertIntoRun(text, scale);

			const slideAdvance = advance * this.run.scaleX;
			if (paragraph && overflows) paragraph.reflow();
			else this.shiftRightward(slideAdvance);
			return slideAdvance;
		}

		/// Deletes code points before and after the caret. A wrapped paragraph, or one the
		/// deletion leaves the run in, is re-wrapped so words can move up.
		delete(before: number, after: number): PredictedDeletion {
			const nothing: PredictedDeletion = {
				removed: 0,
				removedBefore: 0,
				caretShift: 0,
			};
			const paragraph = Paragraph.at(this);

			// In a paragraph that can re-wrap, a deletion may leave the run, so a
			// backspace at a line start removes the break space.
			const run = this.run;
			const whole = run.text;
			const lowest = paragraph ? paragraph.firstStart : run.start;
			const highest = paragraph ? paragraph.lastEnd : run.end;

			const caret = run.start + this.offset;
			const from = stepCodePoints(whole, caret, before, lowest);
			const to = stepCodePoints(whole, caret, after, highest);
			if (to <= from) return nothing;

			const scale = this.measurer.serverScale(run);
			const removedWidth = this.widthOf(from, to, scale) * run.scaleX;
			const beforeWidth = this.widthOf(from, caret, scale) * run.scaleX;
			const crossed = from < run.start || to > run.end;

			this.deleteRange(from, to);

			if (paragraph && (paragraph.lineCount > 1 || crossed)) paragraph.reflow();
			else this.shiftRightward(-removedWidth);
			return {
				removed: to - from,
				removedBefore: caret - from,
				caretShift: beforeWidth ? -beforeWidth : 0,
			};
		}

		// The runs of the caret's paragraph on the caret's line.
		lineRuns(): TextRun[] {
			return this.objectRuns.filter(
				(run) =>
					run.paragraph === this.run.paragraph &&
					run.baseline === this.run.baseline,
			);
		}

		// The extent of the caret's line in local space.
		lineExtent(): number {
			let left = Infinity;
			let right = -Infinity;
			for (const run of this.lineRuns()) {
				left = Math.min(left, run.localX);
				right = Math.max(right, run.localX + run.width(this.measurer));
			}
			return right > left ? right - left : 0;
		}

		// The width of the caret's line without justification, in the server's measure.
		naturalLineWidth(): number {
			let width = 0;
			for (const run of this.lineRuns())
				width +=
					this.measurer.width(run.primitive, run.visibleText) *
					this.measurer.serverScale(run);
			return width;
		}

		// The local width of the paragraph offsets [from, to). Text outside the run is
		// measured and scaled.
		private widthOf(from: number, to: number, scale: number): number {
			if (to <= from) return 0;
			const run = this.run;
			const insideFrom = Math.max(from, run.start) - run.start;
			const insideTo = Math.min(to, run.end) - run.start;
			let width = 0;
			if (insideTo > insideFrom)
				width += this.positions[insideTo] - this.positions[insideFrom];
			let outside = '';
			if (from < run.start)
				outside += run.text.substring(from, Math.min(to, run.start));
			if (to > run.end)
				outside += run.text.substring(Math.max(from, run.end), to);
			if (outside) width += this.measurer.width(run.primitive, outside) * scale;
			return width;
		}

		// The runs of the paragraph that share the caret run's buffer.
		private bufferRuns(): TextRun[] {
			const run = this.run;
			if (run.paragraph == null) return [run];
			return this.objectRuns.filter(
				(other) => other.paragraph === run.paragraph && other.text === run.text,
			);
		}

		// Inserts text into every run of the buffer, with scaled advances for the typed
		// characters. Windows after the caret shift right.
		private insertIntoRun(text: string, scale: number): void {
			const run = this.run;
			const length = run.length;
			const clamped = Math.max(0, Math.min(this.offset, length));
			const at = run.start + clamped;

			const inserted = this.measurer
				.unitAdvances(run.primitive, text)
				.map((advance) => advance * scale);
			run.spliceAdvances(clamped, clamped, inserted);

			for (const other of this.bufferRuns()) {
				const whole = other.text;
				other.text = whole.substring(0, at) + text + whole.substring(at);
				if (other === run) other.length = length + text.length;
				else if (other.start >= at) other.start += text.length;
			}
		}

		// Removes [from, to) from every run of the buffer. Later windows move left and
		// overlapping ones shrink.
		private deleteRange(from: number, to: number): void {
			const removed = to - from;
			for (const other of this.bufferRuns()) {
				const whole = other.text;
				const start = other.start;
				const length = other.length;
				const cutFrom = Math.max(from, start) - start;
				const cutTo = Math.min(to, start + length) - start;
				if (cutTo > cutFrom) {
					other.spliceAdvances(cutFrom, cutTo, []);
					other.length = length - (cutTo - cutFrom);
				}
				if (start >= to) other.start = start - removed;
				else if (start > from) other.start = from;
				other.text = whole.substring(0, from) + whole.substring(to);
			}
		}

		// Moves the runs after the caret by a distance in slide twips.
		private shiftRightward(delta: number): void {
			for (const run of this.rightward) run.shift(delta);
		}
	}
}
