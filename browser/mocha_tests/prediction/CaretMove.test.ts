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
		return cool.stepInParagraph(position, logical, direction);
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

	it('leaves a step out of the paragraph to the server', function () {
		const data = makeTwoParaSlide();
		nodeassert.strictEqual(step(data, { paragraph: 0, index: 5 }, 1), null);
		nodeassert.strictEqual(step(data, { paragraph: 1, index: 0 }, -1), null);
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
});
