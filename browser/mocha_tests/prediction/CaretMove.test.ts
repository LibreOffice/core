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

describe('text prediction caret steps', function () {
	const {
		measure,
		findCaretPosition,
		makeTwoParaSlide,
		makeFieldSlide,
		makeSharedWrapSlide,
	} = predictionTest;

	// The caret one step from the logical caret in the first object, or null.
	function step(
		data: cool.VectorPrimitivesData,
		logical: cool.LogicalCaret,
		direction: -1 | 1,
	): cool.LogicalCaret | null {
		const position = findCaretPosition(data, 0, 0, 0, measure, logical, 1);
		nodeassert.ok(position, 'the caret is in a run');
		return cool.stepByCharacter(position, logical, direction);
	}

	// One box with a run per entry: its text, its baseline below the box top and its paragraph.
	function paragraphsAt(
		lines: Array<[string, number, number]>,
	): cool.VectorPrimitivesData {
		return predictionTest.boxPart(
			lines.map(([text, f, paragraph]) => ({
				type: 'textSimplePortion',
				text,
				textPosition: 0,
				textLength: text.length,
				matrix: [1, 0, 0, 1, 0, f],
				fontSize: 360,
				familyname: 'Arial',
				paragraph,
			})),
		);
	}

	// A single line paragraph in one run.
	function oneLine(text: string): cool.VectorPrimitivesData {
		return makeSharedWrapSlide(text, [{ start: 0, length: text.length, f: 0 }]);
	}

	it('moves the caret one character right and left inside a paragraph', function () {
		const data = makeTwoParaSlide();
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 2 }, 1), {
			paragraph: 0,
			index: 3,
		});
		nodeassert.deepStrictEqual(step(data, { paragraph: 1, index: 2 }, -1), {
			paragraph: 1,
			index: 1,
		});
	});

	it('steps from the edge of a paragraph into the next or the previous one', function () {
		const data = makeTwoParaSlide();
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 5 }, 1), {
			paragraph: 1,
			index: 0,
		});
		nodeassert.deepStrictEqual(step(data, { paragraph: 1, index: 0 }, -1), {
			paragraph: 0,
			index: 5,
		});
	});

	it('leaves a step out of the text to the server', function () {
		const data = makeTwoParaSlide();
		nodeassert.strictEqual(step(data, { paragraph: 0, index: 0 }, -1), null);
		nodeassert.strictEqual(step(data, { paragraph: 1, index: 5 }, 1), null);
	});

	it('leaves a step into a paragraph without text to the server', function () {
		const data = paragraphsAt([
			['Hello', 0, 0],
			['World', 800, 2],
		]);
		nodeassert.strictEqual(step(data, { paragraph: 0, index: 5 }, 1), null);
	});

	it('steps from the end of a wrapped line to the start of the next one', function () {
		// "Hello " on the first line, "world" on the second.
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 6, f: 0 },
			{ start: 6, length: 5, f: 400 },
		]);
		const next = step(data, { paragraph: 0, index: 5 }, 1);
		nodeassert.deepStrictEqual(next, { paragraph: 0, index: 6 });

		const position = findCaretPosition(data, 0, 0, 0, measure, next!, 1);
		const geometry = position!.geometry();
		const firstLine = findCaretPosition(
			data,
			0,
			0,
			0,
			measure,
			{ paragraph: 0, index: 0 },
			1,
		)!.geometry();
		nodeassert.strictEqual(geometry.x, firstLine.x);
		nodeassert.strictEqual(geometry.top, firstLine.top + 400);
	});

	it('steps over a field as one character', function () {
		// "Visit " ends at 6, the field is offset 6, and " today" starts at 7.
		const data = makeFieldSlide();
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 6 }, 1), {
			paragraph: 0,
			index: 7,
		});
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 7 }, -1), {
			paragraph: 0,
			index: 6,
		});
	});

	it('steps over a letter and its combining accent together', function () {
		const data = oneLine('e\u0301x');
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 0 }, 1), {
			paragraph: 0,
			index: 2,
		});
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 2 }, -1), {
			paragraph: 0,
			index: 0,
		});
	});

	it('steps over a character outside the basic plane as one character', function () {
		const data = oneLine('\ud83d\ude00a');
		nodeassert.deepStrictEqual(step(data, { paragraph: 0, index: 0 }, 1), {
			paragraph: 0,
			index: 2,
		});
	});

	it('leaves text written right to left to the server', function () {
		const data = oneLine('ab \u05d0\u05d1');
		nodeassert.strictEqual(step(data, { paragraph: 0, index: 1 }, 1), null);
	});

	// The measured text of the first object.
	function textOf(data: cool.VectorPrimitivesData): cool.ObjectText {
		const text = cool.ObjectText.of(predictionTest.treeOf(data), 1);
		nodeassert.ok(text, 'the object has text');
		return text;
	}

	// The caret where a click at (x, y) lands in the first object.
	function click(
		data: cool.VectorPrimitivesData,
		x: number,
		y: number,
	): cool.LogicalCaret | null {
		return textOf(data).caretAtPoint(x, y);
	}

	it('puts the caret at the character boundary closest to a click', function () {
		// "Hello" spans 1000..1500 on the first line, "World" the same on the second.
		const data = makeTwoParaSlide();
		nodeassert.deepStrictEqual(click(data, 1220, 1950), {
			paragraph: 0,
			index: 2,
		});
		nodeassert.deepStrictEqual(click(data, 1480, 2400), {
			paragraph: 1,
			index: 5,
		});
	});

	it('keeps the caret on a wrapped line when the click is past its end', function () {
		// "Hello " ends the first line at 1600, and "world" starts the second.
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 6, f: 0 },
			{ start: 6, length: 5, f: 400 },
		]);
		nodeassert.deepStrictEqual(click(data, 1700, 1950), {
			paragraph: 0,
			index: 5,
		});
	});

	it('puts the caret at the end of a run that a raised run follows on the same line', function () {
		// "x" ends at 1100, and the raised "2" follows it on the same line.
		const data = makeSharedWrapSlide('x2 y', [
			{ start: 0, length: 1, f: 0 },
			{ start: 1, length: 1, e: 100, f: -120 },
			{ start: 2, length: 2, e: 200, f: 0 },
		]);
		nodeassert.deepStrictEqual(click(data, 1090, 1950), {
			paragraph: 0,
			index: 1,
		});
	});

	it('leaves a click away from the text to the server', function () {
		const data = makeTwoParaSlide();
		nodeassert.strictEqual(click(data, 5000, 1950), null);
		nodeassert.strictEqual(click(data, 1200, 6000), null);
	});

	// The caret one line from the logical caret in the first object, closest to x, or null.
	function lineStep(
		data: cool.VectorPrimitivesData,
		logical: cool.LogicalCaret,
		x: number,
		direction: -1 | 1,
	): cool.LogicalCaret | null {
		const position = findCaretPosition(data, 0, 0, 0, measure, logical, 1);
		nodeassert.ok(position, 'the caret is in a run');
		return (
			cool.ObjectText.of(predictionTest.treeOf(data), 1)?.caretOnNextLine(
				position.run,
				x,
				direction,
			) ?? null
		);
	}

	it('moves the caret to the closest place on the line below and above', function () {
		const data = makeTwoParaSlide();
		nodeassert.deepStrictEqual(
			lineStep(data, { paragraph: 0, index: 2 }, 1200, 1),
			{
				paragraph: 1,
				index: 2,
			},
		);
		nodeassert.deepStrictEqual(
			lineStep(data, { paragraph: 1, index: 4 }, 1400, -1),
			{
				paragraph: 0,
				index: 4,
			},
		);
	});

	it('moves the caret between the lines of a wrapped paragraph', function () {
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 6, f: 0 },
			{ start: 6, length: 5, f: 400 },
		]);
		nodeassert.deepStrictEqual(
			lineStep(data, { paragraph: 0, index: 2 }, 1200, 1),
			{
				paragraph: 0,
				index: 8,
			},
		);
		// From the end of "world", the closest place on the first line is before the break
		// space.
		nodeassert.deepStrictEqual(
			lineStep(data, { paragraph: 0, index: 11 }, 1900, -1),
			{ paragraph: 0, index: 5 },
		);
	});

	it('moves the caret to the line below when that line has a raised run', function () {
		// "wor" spans 1000..1300 on the second line, and the raised "ld" follows it.
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 6, f: 0 },
			{ start: 6, length: 3, f: 400 },
			{ start: 9, length: 2, e: 300, f: 280 },
		]);
		nodeassert.deepStrictEqual(
			lineStep(data, { paragraph: 0, index: 2 }, 1200, 1),
			{ paragraph: 0, index: 8 },
		);
	});

	it('leaves a move off the first or the last line to the server', function () {
		const data = makeTwoParaSlide();
		nodeassert.strictEqual(
			lineStep(data, { paragraph: 0, index: 2 }, 1200, -1),
			null,
		);
		nodeassert.strictEqual(
			lineStep(data, { paragraph: 1, index: 2 }, 1200, 1),
			null,
		);
	});

	it('leaves a move over a paragraph without text to the server', function () {
		const data = paragraphsAt([
			['Hello', 0, 0],
			['World', 800, 2],
		]);
		nodeassert.strictEqual(
			lineStep(data, { paragraph: 0, index: 2 }, 1200, 1),
			null,
		);
	});

	it('covers the selected text of each line with its own rectangle', function () {
		// "Hello " on the first line, "world" on the second, 400 twips apart.
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 6, f: 0 },
			{ start: 6, length: 5, f: 400 },
		]);
		const rectangles = textOf(data).selectionRectangles(
			{ paragraph: 0, index: 8 },
			{ paragraph: 0, index: 2 },
		);
		nodeassert.deepStrictEqual(
			rectangles.map((rectangle) => [rectangle.x, rectangle.width]),
			[
				[1200, 400],
				[1000, 200],
			],
		);
		nodeassert.strictEqual(rectangles[1].y - rectangles[0].y, 400);
	});

	it('takes a drag below the text to its last line', function () {
		const data = makeTwoParaSlide();
		nodeassert.deepStrictEqual(textOf(data).caretNearPoint(1220, 6000), {
			paragraph: 1,
			index: 2,
		});
	});

	it('takes a drag above the text to its first line when that line has a raised run', function () {
		// "Hel" spans 1000..1300 on the first line, and the raised "lo " follows it.
		const data = makeSharedWrapSlide('Hello world', [
			{ start: 0, length: 3, f: 0 },
			{ start: 3, length: 3, e: 300, f: -120 },
			{ start: 6, length: 5, f: 400 },
		]);
		nodeassert.deepStrictEqual(textOf(data).caretNearPoint(1220, -5000), {
			paragraph: 0,
			index: 2,
		});
	});
});
