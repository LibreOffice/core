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

// Fixtures shared by the text layout prediction tests.
namespace predictionTest {
	// The stub measures every character as 100 twips wide, in any font.
	export const measure: cool.TextRunMeasure = (_run, text) => text.length * 100;

	// The tree of a part, measured with the given stub.
	export function treeOf(
		data: cool.VectorPrimitivesData,
		measureWith: cool.TextRunMeasure = measure,
	): cool.PredictionTree {
		return new cool.PredictionTree(data, new cool.TextMeasurer(measureWith));
	}

	// The caret position in a part, by rectangle or by object id.
	export function findCaretPosition(
		data: cool.VectorPrimitivesData,
		caretX: number,
		caretYTop: number,
		caretYBottom: number,
		measureWith: cool.TextRunMeasure,
		logical?: cool.LogicalCaret,
		objectId?: number,
	): cool.CaretPosition | null {
		const tree = treeOf(data, measureWith);
		if (objectId !== undefined)
			return logical ? tree.caretIn(objectId, logical) : null;
		return tree.caretAt(caretX, caretYTop, caretYBottom, logical);
	}

	// Inserts text at the caret and returns the inserted width, 0 when nothing changed.
	export function predictTextInsert(
		data: cool.VectorPrimitivesData,
		caretX: number,
		caretYTop: number,
		caretYBottom: number,
		text: string,
		measureWith: cool.TextRunMeasure,
		logical?: cool.LogicalCaret,
		objectId?: number,
	): number {
		const position = findCaretPosition(
			data,
			caretX,
			caretYTop,
			caretYBottom,
			measureWith,
			logical,
			objectId,
		);
		return position ? position.insert(text) : 0;
	}

	// Deletes around the caret and returns what was removed.
	export function predictTextDelete(
		data: cool.VectorPrimitivesData,
		caretX: number,
		caretYTop: number,
		caretYBottom: number,
		before: number,
		after: number,
		measureWith: cool.TextRunMeasure,
		logical?: cool.LogicalCaret,
		objectId?: number,
	): cool.PredictedDeletion {
		const position = findCaretPosition(
			data,
			caretX,
			caretYTop,
			caretYBottom,
			measureWith,
			logical,
			objectId,
		);
		return position
			? position.delete(before, after)
			: { removed: 0, removedBefore: 0, caretShift: 0 };
	}

	// The caret geometry for a logical position, or null when no editable run is found.
	export function caretGeometry(
		data: cool.VectorPrimitivesData,
		caretX: number,
		caretYTop: number,
		caretYBottom: number,
		logical: cool.LogicalCaret | undefined,
		measureWith: cool.TextRunMeasure,
		objectId?: number,
	): cool.CaretGeometry | null {
		const position = findCaretPosition(
			data,
			caretX,
			caretYTop,
			caretYBottom,
			measureWith,
			logical,
			objectId,
		);
		return position ? position.geometry() : null;
	}

	// A part from a fixture listing its objects in drawing order. Objects without an id are
	// numbered from one.
	export function partOf(fixture: any): cool.VectorPrimitivesData {
		const objects = fixture.objects as cool.SlideObject[];
		const ids = objects.map((object, index) => object.id ?? index + 1);
		return {
			...fixture,
			objects: new Map(
				objects.map((object, index): [number, cool.SlideObject] => [
					ids[index],
					object,
				]),
			),
			order: ids,
		};
	}

	// A part with one text box at (1000, 2000) that holds the runs.
	export function boxPart(runs: any[]): cool.VectorPrimitivesData {
		return partOf({
			slideWidth: 10000,
			slideHeight: 7500,
			masterPage: [],
			objects: [
				{
					primitives: [
						{
							type: 'transform',
							matrix: [1, 0, 0, 1, 1000, 2000],
							children: runs,
						},
					],
				},
			],
		});
	}

	// The object drawn at that place in the part's order.
	export function objectAt(data: cool.VectorPrimitivesData, at: number): any {
		return data.objects.get(data.order[at]);
	}

	// A text box at (1000, 2000) under a transform that doubles its size. "Hi" starts at local
	// x 0 and "Yo" at local x 200, so in slide twips they span 1000..1400 and 1400..1800. The
	// font size of 180 gives a line 216 local twips high, with its baseline at slide y 2000.
	export function makeScaledSlide(): cool.VectorPrimitivesData {
		const run = (text: string, e: number): any => ({
			type: 'textSimplePortion',
			text,
			textPosition: 0,
			textLength: text.length,
			matrix: [1, 0, 0, 1, e, 0],
			fontSize: 180,
			familyname: 'Arial',
		});
		return partOf({
			slideWidth: 10000,
			slideHeight: 7500,
			masterPage: [],
			objects: [
				{
					primitives: [
						{
							type: 'transform',
							matrix: [2, 0, 0, 2, 1000, 2000],
							children: [run('Hi', 0), run('Yo', 200)],
						},
					],
				},
			],
		});
	}

	// One text box at (1000, 2000) holding a single run. With 100-twip characters the run ends
	// at 1000 + 100 * length.
	export function makeSlide(text: string): cool.VectorPrimitivesData {
		return boxPart([
			{
				type: 'textSimplePortion',
				text: text,
				textPosition: 0,
				textLength: text.length,
				matrix: [1, 0, 0, 1, 0, 0],
				fontSize: 360,
				familyname: 'Arial',
			},
		]);
	}

	export function runOf(data: cool.VectorPrimitivesData): any {
		return (objectAt(data, 0).primitives[0] as any).children[0];
	}

	// A paragraph with a field. The text runs window the paragraph text, which holds U+0001 for
	// the field. The field run carries its display text.
	export function makeFieldSlide(): cool.VectorPrimitivesData {
		const paragraphText = 'Visit \u0001 today';
		const makeRun = (
			text: string,
			textPosition: number,
			textLength: number,
			e: number,
		): any => ({
			type: 'textSimplePortion',
			text,
			textPosition,
			textLength,
			matrix: [1, 0, 0, 1, e, 0],
			fontSize: 360,
			familyname: 'Arial',
			paragraph: 0,
		});
		return boxPart([
			makeRun(paragraphText, 0, 6, 0), // "Visit "
			makeRun('example.com', 0, 11, 600),
			makeRun(paragraphText, 7, 6, 1700), // " today"
		]);
	}

	// Two runs sharing a line: "Hi" at 1000..1200, then "Yo" at 1200..1400.
	export function makeTwoRunSlide(
		a: string,
		b: string,
	): cool.VectorPrimitivesData {
		return boxPart([
			{
				type: 'textSimplePortion',
				text: a,
				textPosition: 0,
				textLength: a.length,
				matrix: [1, 0, 0, 1, 0, 0],
				fontSize: 360,
				familyname: 'Arial',
			},
			{
				type: 'textSimplePortion',
				text: b,
				textPosition: 0,
				textLength: b.length,
				matrix: [1, 0, 0, 1, a.length * 100, 0],
				fontSize: 360,
				familyname: 'Arial',
			},
		]);
	}

	export function runsOf(data: cool.VectorPrimitivesData): any[] {
		return (objectAt(data, 0).primitives[0] as any).children;
	}

	// Two separate text boxes sharing a line: "AB" at 1000, "CD" at 5000.
	export function makeTwoObjectSlide(): cool.VectorPrimitivesData {
		const box = (x: number, text: string): any => ({
			primitives: [
				{
					type: 'transform',
					matrix: [1, 0, 0, 1, x, 2000],
					children: [
						{
							type: 'textSimplePortion',
							text: text,
							textPosition: 0,
							textLength: text.length,
							matrix: [1, 0, 0, 1, 0, 0],
							fontSize: 360,
							familyname: 'Arial',
						},
					],
				},
			],
		});
		return partOf({
			slideWidth: 10000,
			slideHeight: 7500,
			masterPage: [],
			objects: [box(1000, 'AB'), box(5000, 'CD')],
		});
	}

	// A run of a paragraph in a box at (1000, 2000), with the wrap width, line height and
	// alignment that let a prediction break the lines again. e and f are the run's local
	// origin.
	export function wrapRun(
		text: string,
		e: number,
		f: number,
		options: { width: number; lineHeight: number; align: string },
	): any {
		return {
			type: 'textSimplePortion',
			text,
			textPosition: 0,
			textLength: text.length,
			matrix: [1, 0, 0, 1, e, f],
			fontSize: 360,
			familyname: 'Arial',
			paragraph: 0,
			textAreaWidth: options.width,
			lineHeight: options.lineHeight,
			align: options.align,
		};
	}

	export function makeWrapSlide(runs: any[]): cool.VectorPrimitivesData {
		return boxPart(runs);
	}

	// Two stacked paragraphs in one box: "Hello" (paragraph 0) on the first line, "World"
	// (paragraph 1) on the line below.
	export function makeTwoParaSlide(): cool.VectorPrimitivesData {
		const run = (text: string, f: number, paragraph: number): any => ({
			type: 'textSimplePortion',
			text,
			textPosition: 0,
			textLength: text.length,
			matrix: [1, 0, 0, 1, 0, f],
			fontSize: 360,
			familyname: 'Arial',
			paragraph,
		});
		return boxPart([run('Hello', 0, 0), run('World', 400, 1)]);
	}

	// Two text boxes that only their object ids tell apart.
	export function makeTwoIdSlide(): cool.VectorPrimitivesData {
		const box = (id: number, y: number): any => ({
			id,
			primitives: [
				{
					type: 'transform',
					matrix: [1, 0, 0, 1, 1000, y],
					children: [
						{
							type: 'textSimplePortion',
							text: 'Hi',
							textPosition: 0,
							textLength: 2,
							matrix: [1, 0, 0, 1, 0, 0],
							fontSize: 360,
							familyname: 'Arial',
							paragraph: 0,
						},
					],
				},
			],
		});
		return partOf({
			slideWidth: 10000,
			slideHeight: 7500,
			masterPage: [],
			objects: [box(7, 2000), box(9, 5000)],
		});
	}

	// A paragraph whose runs share one buffer, as the engine's portions do, in a box at (1000,
	// 2000). A wrapped paragraph keeps the break space at the end of the first line.
	export function makeSharedWrapSlide(
		full: string,
		windows: Array<{
			start: number;
			length: number;
			e?: number;
			f: number;
			weight?: number;
		}>,
		width?: number,
	): cool.VectorPrimitivesData {
		const runs = windows.map((slice) => ({
			type: 'textSimplePortion',
			text: full,
			textPosition: slice.start,
			textLength: slice.length,
			matrix: [1, 0, 0, 1, slice.e ?? 0, slice.f],
			fontSize: 360,
			familyname: 'Arial',
			weight: slice.weight,
			paragraph: 0,
			textAreaWidth: width,
			lineHeight: 400,
		}));
		return boxPart(runs);
	}

	// One line of an indented paragraph: the window of the buffer it shows, its local origin
	// e and f, and the indent and width the engine reports for it.
	export interface IndentedLine {
		start: number;
		length: number;
		e: number;
		f: number;
		indent: number;
		width: number;
	}

	// A paragraph whose runs carry the engine's line indent and width, in a text area of the
	// given width.
	export function makeIndentedSlide(
		full: string,
		lines: IndentedLine[],
		areaWidth: number,
	): cool.VectorPrimitivesData {
		const runs = lines.map((line) => ({
			type: 'textSimplePortion',
			text: full,
			textPosition: line.start,
			textLength: line.length,
			matrix: [1, 0, 0, 1, line.e, line.f],
			fontSize: 360,
			familyname: 'Arial',
			paragraph: 0,
			textAreaWidth: areaWidth,
			lineHeight: 400,
			lineIndent: line.indent,
			lineWidth: line.width,
		}));
		return boxPart(runs);
	}
}
