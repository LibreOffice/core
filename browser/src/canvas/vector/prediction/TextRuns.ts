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
 * Text runs as the prediction sees them, and the measurer that turns text into widths.
 */

namespace cool {
	// The width in twips of text in the run's font.
	export type TextRunMeasure = (
		run: TextSimplePortionPrimitive,
		text: string,
	) => number;

	export function isHighSurrogate(code: number): boolean {
		return code >= 0xd800 && code <= 0xdbff;
	}

	export function isLowSurrogate(code: number): boolean {
		return code >= 0xdc00 && code <= 0xdfff;
	}

	// The start and the length in UTF-16 units of the part of its buffer a run shows.
	export function textWindow(run: TextSimplePortionPrimitive): {
		start: number;
		length: number;
	} {
		const start = run.textPosition ?? 0;
		return { start, length: run.textLength ?? (run.text ?? '').length - start };
	}

	// The part of its buffer a run shows.
	export function visibleText(run: TextSimplePortionPrimitive): string {
		const { start, length } = textWindow(run);
		return (run.text ?? '').substring(start, start + length);
	}

	/// Measures text in a run's font and scales it to the server's measure. Widths are sums of
	/// single code points, so the parts of a text add up to the whole. Kerning is lost, and the
	/// server's layout restores it.
	export class TextMeasurer {
		constructor(private readonly measure: TextRunMeasure) {}

		width(run: TextSimplePortionPrimitive, text: string): number {
			let width = 0;
			for (const character of text) width += this.measure(run, character);
			return width;
		}

		// The step to each UTF-16 unit from the one before. A surrogate pair puts its width
		// on its first unit.
		unitAdvances(run: TextSimplePortionPrimitive, text: string): number[] {
			const advances: number[] = [];
			for (const character of text) {
				advances.push(this.measure(run, character));
				for (let k = 1; k < character.length; k++) advances.push(0);
			}
			return advances;
		}

		// The ratio of the run's server advances to its browser width, from its letters
		// only, since justification stretches spaces. A run of spaces uses all its text,
		// and a run without advances has the factor 1.
		serverScale(run: TextRun): number {
			const steps = run.coveringAdvances();
			if (!steps) return 1;
			let letterServer = 0;
			let letterBrowser = 0;
			let allServer = 0;
			let allBrowser = 0;
			let unit = 0;
			for (const character of run.visibleText) {
				let server = 0;
				for (let k = 0; k < character.length; k++) server += steps[unit + k];
				unit += character.length;
				const browser = this.measure(run.primitive, character);
				allServer += server;
				allBrowser += browser;
				if (character !== ' ') {
					letterServer += server;
					letterBrowser += browser;
				}
			}
			if (letterServer > 0 && letterBrowser > 0)
				return letterServer / letterBrowser;
			if (allServer > 0 && allBrowser > 0) return allServer / allBrowser;
			return 1;
		}
	}

	/// A text run primitive with its place on the slide. The primitive is the cached tree's,
	/// so edits through the run change the tree.
	export class TextRun {
		// True when the run's own matrix rotates or skews it.
		readonly rotated: boolean;

		constructor(
			readonly primitive: TextSimplePortionPrimitive,
			// Run origin in slide twips.
			readonly x: number,
			readonly baseline: number,
			// The accumulated transform above the run.
			readonly transform: Matrix2D,
			// The children array that holds the run.
			readonly container: Primitive[],
		) {
			const matrix = Matrix2D.fromArray(primitive.matrix) ?? Matrix2D.IDENTITY;
			this.rotated = matrix.b !== 0 || matrix.c !== 0;
		}

		get fontSize(): number {
			return this.primitive.fontSize ?? 12;
		}

		// The scale from the run's local space to the slide.
		get scaleX(): number {
			return this.transform.a || 1;
		}

		get scaleY(): number {
			return Math.abs(this.transform.d) || 1;
		}

		// The line's ascent and height in local space, estimated from the font size when
		// the run has none.
		get lineAscent(): number {
			return this.primitive.lineAscent || this.fontSize * 0.8;
		}

		get lineHeight(): number {
			return this.primitive.lineHeight || this.fontSize * 1.2;
		}

		// The slide x of a local distance from the run's origin.
		slideX(local: number): number {
			return this.x + local * this.scaleX;
		}

		get paragraph(): number | undefined {
			return this.primitive.paragraph;
		}

		// The buffer, which holds the paragraph text around the run's window.
		get text(): string {
			return this.primitive.text ?? '';
		}

		set text(text: string) {
			this.primitive.text = text;
		}

		// The window of the buffer that the run shows, in UTF-16 units.
		get start(): number {
			return this.primitive.textPosition ?? 0;
		}

		set start(start: number) {
			this.primitive.textPosition = start;
		}

		get length(): number {
			return textWindow(this.primitive).length;
		}

		set length(length: number) {
			this.primitive.textLength = length;
		}

		get end(): number {
			return this.start + this.length;
		}

		get visibleText(): string {
			return visibleText(this.primitive);
		}

		// The run's origin in local space, where advances and wrapping work.
		get localX(): number {
			return this.primitive.matrix?.[4] ?? 0;
		}

		get localBaseline(): number {
			return this.primitive.matrix?.[5] ?? 0;
		}

		// The advances when they cover the visible text, otherwise null.
		coveringAdvances(): number[] | null {
			const advances = this.primitive.advances;
			const length = this.length;
			return advances && advances.length >= length && length > 0
				? advances
				: null;
		}

		// Replaces the advances of units [cutFrom, cutTo). A run with too few advances
		// drops them and is measured.
		spliceAdvances(cutFrom: number, cutTo: number, inserted: number[]): void {
			const steps = this.coveringAdvances();
			if (steps)
				this.primitive.advances = [
					...steps.slice(0, cutFrom),
					...inserted,
					...steps.slice(cutTo, this.length),
				];
			else if (this.primitive.advances) this.primitive.advances = undefined;
		}

		// The local x before each character, from the advances or measured. The last entry
		// is the run width.
		characterPositions(measurer: TextMeasurer): number[] {
			const text = this.visibleText;
			const length = text.length;
			const positions: number[] = new Array(length + 1);
			positions[0] = 0;
			const advances =
				this.coveringAdvances() ?? measurer.unitAdvances(this.primitive, text);
			for (let k = 1; k <= length; k++)
				positions[k] = positions[k - 1] + advances[k - 1];
			return positions;
		}

		// The width of the visible text in local space.
		width(measurer: TextMeasurer): number {
			const positions = this.characterPositions(measurer);
			return positions[positions.length - 1];
		}

		// Moves the run's origin by a distance in slide twips.
		shift(delta: number): void {
			const matrix = this.primitive.matrix;
			if (matrix && matrix.length === 6)
				matrix[4] += delta / (this.transform.a || 1);
		}

		// The text runs of a subtree, with their origins in slide twips.
		static collect(
			primitives: Primitive[] | undefined,
			matrix: Matrix2D = Matrix2D.IDENTITY,
			out: TextRun[] = [],
		): TextRun[] {
			if (!primitives) return out;
			for (const primitive of primitives) {
				let childMatrix = matrix;
				if (primitive.type === TransformPrimitive.type) {
					const transformMatrix = Matrix2D.fromArray(
						(primitive as TransformPrimitive).matrix,
					);
					if (transformMatrix) childMatrix = transformMatrix.then(matrix);
				} else if (TextRun.isTextRun(primitive)) {
					const run = primitive as TextSimplePortionPrimitive;
					const runMatrix = Matrix2D.fromArray(run.matrix) ?? Matrix2D.IDENTITY;
					const origin = matrix.apply(runMatrix.e, runMatrix.f);
					out.push(new TextRun(run, origin.x, origin.y, matrix, primitives));
				}
				TextRun.collect(primitive.children, childMatrix, out);
			}
			return out;
		}

		private static isTextRun(primitive: Primitive): boolean {
			return (
				primitive.type === TextSimplePortionPrimitive.type ||
				primitive.type === TextDecoratedPortionPrimitive.type
			);
		}
	}
}
