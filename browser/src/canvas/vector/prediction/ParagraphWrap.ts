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
 * Re-wraps a paragraph after an edit and rebuilds its runs.
 */

namespace cool {
	// The wrap area in local space, taken from the top line before an edit. A line starts at
	// areaLeft plus its indent. The first line has its own indent and width, for a first-line
	// indent or a bullet, and the later lines share another pair.
	export interface ParagraphBox {
		areaLeft: number;
		firstIndent: number;
		firstWidth: number;
		restIndent: number;
		restWidth: number;
		firstBaseline: number;
		lineHeight: number;
		align: 'left' | 'right' | 'center' | 'justify';
		template: TextSimplePortionPrimitive;
	}

	// A code point with its source run, its paragraph offset in UTF-16 units and its width in
	// the server's measure.
	interface ParagraphCharacter {
		character: string;
		source: TextSimplePortionPrimitive;
		offset: number;
		width: number;
	}

	// The characters of a line from one source run, with their start and per-unit advances.
	interface FontSpan {
		source: TextSimplePortionPrimitive;
		text: string;
		start: number;
		advances: number[];
	}

	/// A paragraph that can be re-wrapped: its runs in reading order and its wrap area.
	export class Paragraph {
		private constructor(
			readonly runs: TextRun[],
			readonly box: ParagraphBox,
			private readonly measurer: TextMeasurer,
		) {}

		/// The caret run's paragraph, or null when it cannot be re-wrapped.
		static at(position: CaretPosition): Paragraph | null {
			const runs = Paragraph.collect(position);
			if (!runs) return null;
			return new Paragraph(
				runs,
				Paragraph.captureBox(runs, position.measurer),
				position.measurer,
			);
		}

		// The paragraph's runs in reading order, or null. They need one transform, one
		// buffer, a wrap width and a line height.
		private static collect(position: CaretPosition): TextRun[] | null {
			const caretRun = position.run;
			const paragraph = caretRun.paragraph;
			if (
				paragraph == null ||
				!caretRun.primitive.textAreaWidth ||
				!caretRun.primitive.lineHeight
			)
				return null;

			const runs: TextRun[] = [];
			for (const run of position.objectRuns) {
				if (run.paragraph !== paragraph) continue;
				if (run.rotated || run.transform !== caretRun.transform) return null;
				runs.push(run);
			}
			if (!runs.length) return null;
			// A field has its own buffer, so its paragraph has no single text.
			if (sharedParagraphText(runs) === null) return null;
			runs.sort((a, b) => a.baseline - b.baseline || a.x - b.x);
			return runs;
		}

		// A run without line geometry wraps at the whole text area. A one-line paragraph
		// gives its indent and width to the later lines.
		private static captureBox(
			runs: TextRun[],
			measurer: TextMeasurer,
		): ParagraphBox {
			const template = runs[0].primitive;
			const areaWidth = template.textAreaWidth ?? 0;
			const lineHeight = template.lineHeight ?? 0;
			const align = template.align ?? 'left';

			const firstBaseline = Math.min(...runs.map((run) => run.localBaseline));
			const topLine = runs.filter((run) => run.localBaseline === firstBaseline);
			const topLeft = Math.min(...topLine.map((run) => run.localX));
			const topRight = Math.max(
				...topLine.map((run) => run.localX + run.width(measurer)),
			);

			const firstIndent = template.lineIndent ?? 0;
			const firstWidth = template.lineWidth ?? areaWidth;
			const later = runs.find((run) => run.localBaseline !== firstBaseline);
			const restIndent = later
				? (later.primitive.lineIndent ?? 0)
				: firstIndent;
			const restWidth = later
				? (later.primitive.lineWidth ?? areaWidth)
				: firstWidth;

			// The area's left edge follows from where the aligned top line sits.
			let areaLeft = topLeft - firstIndent;
			if (align === 'right') areaLeft = topRight - firstIndent - firstWidth;
			else if (align === 'center')
				areaLeft = (topLeft + topRight) / 2 - firstIndent - firstWidth / 2;

			return {
				areaLeft,
				firstIndent,
				firstWidth,
				restIndent,
				restWidth,
				firstBaseline,
				lineHeight,
				align,
				template,
			};
		}

		// The start x and the width of the line at the given index.
		private lineBox(index: number): { left: number; width: number } {
			const box = this.box;
			const indent = index === 0 ? box.firstIndent : box.restIndent;
			const width = index === 0 ? box.firstWidth : box.restWidth;
			return { left: box.areaLeft + indent, width };
		}

		get lineCount(): number {
			return new Set(this.runs.map((run) => run.baseline)).size;
		}

		// The paragraph offsets the runs show.
		get firstStart(): number {
			return this.runs[0].start;
		}

		get lastEnd(): number {
			return this.runs[this.runs.length - 1].end;
		}

		/// True when text of the given width no longer fits on the caret's line, at the
		/// line's own width. A justified line is compared by its natural width.
		lineOverflows(position: CaretPosition, advance: number): boolean {
			const wrapWidth = position.run.primitive.lineWidth ?? this.box.firstWidth;
			if (this.box.align === 'justify')
				return position.naturalLineWidth() + advance > wrapWidth;
			return position.lineExtent() + advance > wrapWidth;
		}

		/// Re-wraps the paragraph and rebuilds it as one run per font span on each line.
		reflow(): void {
			const characters = this.characters();
			// The rebuilt runs window the same buffer.
			const paragraphText = sharedParagraphText(this.runs) ?? '';
			const lines = this.wrap(characters);
			const newRuns: Primitive[] = [];
			lines.forEach((lineCharacters, index) => {
				const baseline = this.box.firstBaseline + index * this.box.lineHeight;
				this.appendLineRuns(
					newRuns,
					paragraphText,
					lineCharacters,
					index,
					baseline,
					index === lines.length - 1,
				);
			});
			this.replaceRuns(newRuns);
		}

		// The paragraph's characters in reading order. Gaps between windows, such as a wrap
		// space, come from the buffer. Text before the first window stays out.
		//
		// A character keeps its server advance, so untouched text stays in place. Others
		// are measured and scaled. Justified spaces are measured, as their advances hold
		// the old stretch.
		private characters(): ParagraphCharacter[] {
			const characters: ParagraphCharacter[] = [];
			const measuredSpaces = this.box.align === 'justify';
			let offset = this.firstStart;
			let previous: TextRun | null = null;
			let previousScale = 1;
			for (const run of this.runs) {
				const scale = this.measurer.serverScale(run);
				if (previous) {
					for (let k = run.start - previous.end; k > 0; k--) {
						const character = run.text.charAt(run.start - k);
						characters.push({
							character,
							source: previous.primitive,
							offset: offset++,
							width:
								this.measurer.width(previous.primitive, character) *
								previousScale,
						});
					}
				}
				const steps = run.coveringAdvances();
				let unit = 0;
				for (const character of run.visibleText) {
					let width = 0;
					if (steps && !(measuredSpaces && character === ' ')) {
						for (let k = 0; k < character.length; k++) width += steps[unit + k];
					} else {
						width = this.measurer.width(run.primitive, character) * scale;
					}
					unit += character.length;
					characters.push({ character, source: run.primitive, offset, width });
					offset += character.length;
				}
				previous = run;
				previousScale = scale;
			}
			return characters;
		}

		private static lineWidth(lineCharacters: ParagraphCharacter[]): number {
			let width = 0;
			for (const entry of lineCharacters) width += entry.width;
			return width;
		}

		// Breaks lines at the last space that fits each line's width. A longer word gets a
		// line of its own. The spaces at a break are dropped, as the server hangs them.
		private wrap(characters: ParagraphCharacter[]): ParagraphCharacter[][] {
			const lines: ParagraphCharacter[][] = [];
			let current: ParagraphCharacter[] = [];
			let currentWidth = 0;
			let lastSpace = -1;
			for (const entry of characters) {
				current.push(entry);
				currentWidth += entry.width;
				if (entry.character === ' ') lastSpace = current.length - 1;
				const width = this.lineBox(lines.length).width;
				if (currentWidth <= width || lastSpace <= 0) continue;
				let breakStart = lastSpace;
				while (breakStart > 0 && current[breakStart - 1].character === ' ')
					breakStart--;
				if (breakStart === 0) continue;
				lines.push(current.slice(0, breakStart));
				current = current.slice(lastSpace + 1);
				currentWidth = Paragraph.lineWidth(current);
				lastSpace = -1;
				for (let k = 0; k < current.length; k++)
					if (current[k].character === ' ') lastSpace = k;
			}
			if (current.length) lines.push(current);
			if (!lines.length) lines.push([]);
			return lines;
		}

		// Widens the spaces of a justified line so it reaches the width.
		private static stretch(span: FontSpan, width: number): void {
			let gaps = 0;
			for (const character of span.text) if (character === ' ') gaps++;
			if (gaps === 0) return;

			let natural = 0;
			for (const advance of span.advances) natural += advance;
			const slack = width - natural;
			if (slack <= 0) return;

			const extra = slack / gaps;
			for (let k = 0; k < span.text.length; k++)
				if (span.text[k] === ' ') span.advances[k] += extra;
		}

		// A run for one font span that windows the paragraph text, as the server's runs do.
		private static makeSpanRun(
			source: TextSimplePortionPrimitive,
			paragraphText: string,
			start: number,
			length: number,
			x: number,
			baseline: number,
			advances: number[] | undefined,
		): TextSimplePortionPrimitive {
			const sourceMatrix =
				Matrix2D.fromArray(source.matrix) ?? Matrix2D.IDENTITY;
			// The font and decorations come from the source run.
			return {
				...source,
				text: paragraphText,
				textPosition: start,
				textLength: length,
				matrix: [sourceMatrix.a, 0, 0, sourceMatrix.d, x, baseline],
				advances,
				children: undefined,
			};
		}

		// Splits a line into one span per source run, so each keeps its font.
		private static lineSpans(lineCharacters: ParagraphCharacter[]): FontSpan[] {
			const spans: FontSpan[] = [];
			for (const entry of lineCharacters) {
				let span = spans[spans.length - 1];
				if (!span || span.source !== entry.source) {
					span = {
						source: entry.source,
						text: '',
						start: entry.offset,
						advances: [],
					};
					spans.push(span);
				}
				span.text += entry.character;
				// A surrogate pair carries its width on its first unit.
				span.advances.push(entry.width);
				for (let k = 1; k < entry.character.length; k++) span.advances.push(0);
			}
			return spans;
		}

		// Appends the runs of one line, placed by its indent and the alignment. Only a
		// single-span justified line is stretched. Every run carries the line's indent and
		// width.
		private appendLineRuns(
			out: Primitive[],
			paragraphText: string,
			lineCharacters: ParagraphCharacter[],
			lineIndex: number,
			baseline: number,
			isLastLine: boolean,
		): void {
			const box = this.box;
			const line = this.lineBox(lineIndex);
			const stamp = (
				run: TextSimplePortionPrimitive,
			): TextSimplePortionPrimitive => {
				run.lineIndent = line.left - box.areaLeft;
				run.lineWidth = line.width;
				return run;
			};
			const spans = Paragraph.lineSpans(lineCharacters);
			if (!spans.length) {
				out.push(
					stamp(
						Paragraph.makeSpanRun(
							box.template,
							paragraphText,
							0,
							0,
							line.left,
							baseline,
							undefined,
						),
					),
				);
				return;
			}

			const width = Paragraph.lineWidth(lineCharacters);
			let x = line.left;
			if (box.align === 'right') x = line.left + line.width - width;
			else if (box.align === 'center') x = line.left + (line.width - width) / 2;
			const stretched =
				box.align === 'justify' && !isLastLine && spans.length === 1;

			for (const span of spans) {
				if (stretched) Paragraph.stretch(span, line.width);
				out.push(
					stamp(
						Paragraph.makeSpanRun(
							span.source,
							paragraphText,
							span.start,
							span.text.length,
							x,
							baseline,
							span.advances,
						),
					),
				);
				for (const advance of span.advances) x += advance;
			}
		}

		// Swaps the old run nodes for the new ones, where the first old one was.
		private replaceRuns(newRuns: Primitive[]): void {
			const container = this.runs[0].container;
			let insertIndex = container.indexOf(this.runs[0].primitive);
			for (const run of this.runs) {
				const index = run.container.indexOf(run.primitive);
				if (index !== -1) run.container.splice(index, 1);
			}
			if (insertIndex < 0 || insertIndex > container.length)
				insertIndex = container.length;
			container.splice(insertIndex, 0, ...newRuns);
		}
	}
}
