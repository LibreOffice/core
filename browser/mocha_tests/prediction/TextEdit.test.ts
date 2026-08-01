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

describe('text prediction text edits', function () {
	const {
		measure,
		objectAt,
		makeSlide,
		runOf,
		makeFieldSlide,
		makeTwoRunSlide,
		runsOf,
		makeTwoObjectSlide,
		makeSharedWrapSlide,
	} = predictionTest;

	it('appends a character typed at the trailing edge of a run', function () {
		const data = makeSlide('Hi');
		// "Hi" spans 1000..1200, and the caret is at its end.
		const advance = predictionTest.predictTextInsert(
			data,
			1200,
			1800,
			2050,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		nodeassert.strictEqual(runOf(data).text, 'Hia');
		nodeassert.strictEqual(runOf(data).textLength, 3);
	});

	it('leaves newlines to the server', function () {
		const data = makeSlide('Hi');
		const advance = predictionTest.predictTextInsert(
			data,
			1200,
			1800,
			2050,
			'\n',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('leaves tabs to the server', function () {
		// A tab advances to a tab stop the client does not know.
		const data = makeSlide('Hi');
		const advance = predictionTest.predictTextInsert(
			data,
			1200,
			1800,
			2050,
			'\t',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('inserts in the middle of a run', function () {
		const data = makeSlide('Helo');
		// "Helo" spans 1000..1400. A caret at 1300 is between 'l' and 'o'.
		const advance = predictionTest.predictTextInsert(
			data,
			1300,
			1800,
			2050,
			'l',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		nodeassert.strictEqual(runOf(data).text, 'Hello');
		nodeassert.strictEqual(runOf(data).textLength, 5);
	});

	it('moves the caret back by the server width of the deleted character', function () {
		const data = makeSlide('abc');
		runOf(data).advances = [150, 100, 170];
		// Caret at the end (x = 1420). Backspace removes 'c', which is 170 wide by the
		// advances, not the stub's 100.
		const result = predictionTest.predictTextDelete(
			data,
			1420,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(result.caretShift, -170);
		nodeassert.strictEqual(runOf(data).text, 'ab');
	});

	it('keeps the widths of the remaining characters after a deletion', function () {
		const data = makeSlide('abc');
		runOf(data).advances = [150, 100, 170];
		// Backspace at x = 1250 removes 'b'. Its 100-twip entry drops out and the entry
		// after it stays as it is.
		const result = predictionTest.predictTextDelete(
			data,
			1250,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(runOf(data).text, 'ac');
		nodeassert.deepStrictEqual(runOf(data).advances, [150, 170]);
	});

	it('gives a typed character a width scaled to the server widths of the run', function () {
		const data = makeSlide('ab');
		// The server advances are half the stub's 100 per character, so the typed character
		// enters the advances at 50.
		runOf(data).advances = [50, 50];
		// Caret after 'a' (x = 1050).
		const advance = predictionTest.predictTextInsert(
			data,
			1050,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 50);
		nodeassert.strictEqual(runOf(data).text, 'axb');
		nodeassert.deepStrictEqual(runOf(data).advances, [50, 50, 50]);
	});

	it('deletes a whole surrogate pair for one backspace', function () {
		// 'a' followed by a character of two UTF-16 units, three units in all.
		const data = makeSlide('a\u{1F600}');
		// Caret at the end (x = 1300). One backspace, counted in code points, removes both
		// units of the pair.
		const result = predictionTest.predictTextDelete(
			data,
			1300,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 2);
		nodeassert.strictEqual(runOf(data).text, 'a');
	});

	it('leaves text that measures to no width to the server', function () {
		const data = makeSlide('Hi');
		const zeroMeasure: cool.TextRunMeasure = () => 0;
		const advance = predictionTest.predictTextInsert(
			data,
			1000,
			1800,
			2050,
			'a',
			zeroMeasure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('types at a paragraph index after a field', function () {
		// Index 13, the end of " today", maps through the runs that carry the paragraph
		// text. The field keeps its text and window.
		const data = makeFieldSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			2300,
			1800,
			2050,
			'x',
			measure,
			{
				paragraph: 0,
				index: 13,
			},
		);
		nodeassert.strictEqual(advance, 100);
		const runs = runsOf(data);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'Visit ');
		nodeassert.strictEqual(runs[1].text, 'example.com');
		nodeassert.strictEqual(runs[1].textPosition, 0);
		nodeassert.strictEqual(cool.visibleText(runs[2]), ' todayx');
		nodeassert.strictEqual(runs[0].text, 'Visit \u0001 todayx');
	});

	it('shifts a field run along the line when typing before it', function () {
		// Typing at index 6, the end of "Visit ", grows that run and moves the field and "
		// today" right by the typed width. The field's window stays, and the run after it
		// still names the same characters.
		const data = makeFieldSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			1600,
			1800,
			2050,
			'x',
			measure,
			{
				paragraph: 0,
				index: 6,
			},
		);
		nodeassert.strictEqual(advance, 100);
		const runs = runsOf(data);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'Visit x');
		nodeassert.strictEqual(runs[1].text, 'example.com');
		nodeassert.strictEqual(runs[1].matrix[4], 700);
		nodeassert.strictEqual(runs[2].textPosition, 8);
		nodeassert.strictEqual(runs[2].matrix[4], 1800);
		nodeassert.strictEqual(cool.visibleText(runs[2]), ' today');
	});

	it('keeps a field run untouched when editing next to it', function () {
		// An insert found by the caret pixels, at the end of "Visit ", edits the paragraph
		// text. The field run keeps its text and window.
		const data = makeFieldSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			1600,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		const runs = runsOf(data);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'Visit x');
		nodeassert.strictEqual(runs[1].text, 'example.com');
		nodeassert.strictEqual(runs[1].textPosition, 0);
		// The following text run's window shifts with the paragraph text.
		nodeassert.strictEqual(runs[2].textPosition, 8);
		nodeassert.strictEqual(cool.visibleText(runs[2]), ' today');
	});

	it('shifts the following run right when inserting to its left', function () {
		const data = makeTwoRunSlide('Hi', 'Yo');
		// Caret in the left run (x = 1100). "Yo", at e = 200, moves right by the inserted
		// character's width.
		const advance = predictionTest.predictTextInsert(
			data,
			1100,
			1800,
			2050,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		nodeassert.strictEqual(runsOf(data)[0].text, 'Hai');
		nodeassert.strictEqual(runsOf(data)[1].matrix[4], 300);
	});

	it('types into the last run of a line with several runs', function () {
		const data = makeTwoRunSlide('Hi', 'Yo');
		// Caret inside the right run, with no run after it on the line.
		const advance = predictionTest.predictTextInsert(
			data,
			1300,
			1800,
			2050,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		nodeassert.strictEqual(runsOf(data)[1].text, 'Yao');
	});

	it('removes the character before the caret on backspace', function () {
		const data = makeSlide('Hello');
		// Caret at the end (x = 1500). Backspace moves the caret left by one character's
		// width.
		const result = predictionTest.predictTextDelete(
			data,
			1500,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(result.caretShift, -100);
		nodeassert.strictEqual(runOf(data).text, 'Hell');
		nodeassert.strictEqual(runOf(data).textLength, 4);
	});

	it('removes the character after the caret on forward delete', function () {
		const data = makeSlide('Hello');
		// Caret before the second 'l' (x = 1200). Forward delete leaves the caret in place,
		// since nothing before it was removed.
		const result = predictionTest.predictTextDelete(
			data,
			1200,
			1800,
			2050,
			0,
			1,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(result.caretShift, 0);
		nodeassert.strictEqual(runOf(data).text, 'Helo');
	});

	it('defers a backspace at the start of a run to the server', function () {
		const data = makeSlide('Hello');
		// Caret at the run start (x = 1000): the deletion crosses the boundary.
		const result = predictionTest.predictTextDelete(
			data,
			1000,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hello');
	});

	it('shifts the following run left when deleting', function () {
		const data = makeTwoRunSlide('Hi', 'Yo');
		// Backspace inside the left run (caret x = 1100) removes 'H'. "Yo" moves left by
		// the removed width, from matrix e = 200 to 100.
		const result = predictionTest.predictTextDelete(
			data,
			1100,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(runsOf(data)[0].text, 'i');
		nodeassert.strictEqual(runsOf(data)[1].matrix[4], 100);
	});

	it('does not shift runs that belong to another object', function () {
		const data = makeTwoObjectSlide();
		// The second box shares the line further right and stays in place.
		const advance = predictionTest.predictTextInsert(
			data,
			1100,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 100);
		const objects = [objectAt(data, 0), objectAt(data, 1)];
		nodeassert.strictEqual(objects[0].primitives[0].children[0].text, 'AxB');
		nodeassert.strictEqual(objects[1].primitives[0].children[0].matrix[4], 0);
	});

	it('shifts the next run by the typed width under a scaled transform', function () {
		// 'x' is 100 local twips wide. The doubling transform makes that 200 slide twips,
		// and "Yo" moves from local x 200 to 300.
		const data = predictionTest.makeScaledSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			1390,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 200);
		const runs = predictionTest.runsOf(data);
		nodeassert.strictEqual(runs[0].text, 'Hix');
		nodeassert.strictEqual(runs[1].matrix[4], 300);
	});

	it('moves the caret back by the deleted width under a scaled transform', function () {
		// Backspace at the end of "Hi" removes 'i', 100 local twips and 200 slide twips.
		const data = predictionTest.makeScaledSlide();
		const result = predictionTest.predictTextDelete(
			data,
			1390,
			1800,
			2050,
			1,
			0,
			measure,
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(result.caretShift, -200);
		nodeassert.strictEqual(predictionTest.runsOf(data)[1].matrix[4], 100);
	});

	it('scales typed text by the letters of a run whose spaces are stretched', function () {
		// The server stretched the space of "a b" to 300. Its letters are 50 against the
		// browser's 100, so a typed 'x' is 50 wide.
		const data = makeSlide('a b');
		runOf(data).advances = [50, 300, 50];
		const advance = predictionTest.predictTextInsert(
			data,
			1400,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 50);
	});

	it('removes characters from both runs when a forward delete crosses into the next run', function () {
		// "abc" and "def" are two runs on one line sharing the buffer. Deleting two
		// characters forward from offset 2 removes 'c' and 'd', so the second window
		// shrinks and moves.
		const data = makeSharedWrapSlide(
			'abcdef',
			[
				{ start: 0, length: 3, f: 0 },
				{ start: 3, length: 3, f: 0, weight: 7 },
			],
			2000,
		);
		const result = predictionTest.predictTextDelete(
			data,
			1200,
			1800,
			2050,
			0,
			2,
			measure,
			{
				paragraph: 0,
				index: 2,
			},
		);
		nodeassert.strictEqual(result.removed, 2);
		nodeassert.strictEqual(result.removedBefore, 0);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.map(cool.visibleText).join('|'), 'ab|ef');
		nodeassert.strictEqual(runs[0].text, 'abef');
		nodeassert.strictEqual(runs[1].textPosition, 2);
		nodeassert.strictEqual(runs[1].weight, 7);
		// The second run follows the first directly on the same line.
		nodeassert.strictEqual(runs[1].matrix[4], 200);
	});
});
