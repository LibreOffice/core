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

describe('text prediction caret lookup', function () {
	const {
		measure,
		partOf,
		objectAt,
		makeSlide,
		runOf,
		runsOf,
		wrapRun,
		makeWrapSlide,
		makeTwoParaSlide,
		makeTwoIdSlide,
		makeSharedWrapSlide,
	} = predictionTest;

	it('does nothing when the caret is far from any run edge', function () {
		const data = makeSlide('Hi');
		const advance = predictionTest.predictTextInsert(
			data,
			5000,
			1800,
			2050,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('ignores a caret where the run would be without its box offset', function () {
		// Without the box translation the run would end at 200. A caret there finds no run.
		const data = makeSlide('Hi');
		const advance = predictionTest.predictTextInsert(
			data,
			200,
			1800,
			2050,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('ignores a caret on a different line', function () {
		const data = makeSlide('Hi');
		// The x matches the run's end, but the caret is far below the line.
		const advance = predictionTest.predictTextInsert(
			data,
			1200,
			5000,
			5250,
			'a',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('maps the caret to a run and a character offset', function () {
		const data = makeSlide('Hello');
		const position = predictionTest.findCaretPosition(
			data,
			1300,
			1800,
			2050,
			measure,
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.text, 'Hello');
		nodeassert.strictEqual(position.offset, 3);
	});

	it('maps the caret by the character widths from the server', function () {
		// Uneven advances end the characters at 150, 250 and 420, so x = 1250 maps to
		// offset 2 only through them.
		const data = makeSlide('abc');
		runOf(data).advances = [150, 100, 170];
		const position = predictionTest.findCaretPosition(
			data,
			1250,
			1800,
			2050,
			measure,
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.offset, 2);
		nodeassert.strictEqual(position.positions[3], 420);
	});

	it('leaves the edit to the server when the logical paragraph has no runs', function () {
		const data = makeSlide('Hi');
		runOf(data).paragraph = 0;
		// The caret pixels match the run, and the logical caret names a paragraph the
		// object does not contain.
		const advance = predictionTest.predictTextInsert(
			data,
			1200,
			1800,
			2050,
			'a',
			measure,
			{
				paragraph: 3,
				index: 0,
			},
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(runOf(data).text, 'Hi');
	});

	it('puts the caret in the logical paragraph when the pixels point at another', function () {
		const data = makeTwoParaSlide();
		// The caret pixels fall on the first paragraph's line, but the server says the
		// caret is in the second paragraph at offset 2.
		const position = predictionTest.findCaretPosition(
			data,
			1100,
			1800,
			2050,
			measure,
			{
				paragraph: 1,
				index: 2,
			},
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.text, 'World');
		nodeassert.strictEqual(position.offset, 2);
	});

	it('types into the logical paragraph when the pixels point at another', function () {
		const data = makeTwoParaSlide();
		// Even though the pixels point at "Hello", the edit lands in "World".
		predictionTest.predictTextInsert(data, 1100, 1800, 2050, 'X', measure, {
			paragraph: 1,
			index: 2,
		});
		nodeassert.strictEqual(runsOf(data)[0].text, 'Hello');
		nodeassert.strictEqual(runsOf(data)[1].text, 'WoXrld');
	});

	it('maps a paragraph offset across a wrap to the right run', function () {
		// One paragraph on two lines. Neither run shows the space at index 7, the wrap
		// point.
		const data = makeSharedWrapSlide('aaa bbb ccc', [
			{ start: 0, length: 7, f: 0 },
			{ start: 8, length: 3, f: 400 },
		]);
		// Index 8, after the dropped wrap space at index 7, is the start of "ccc" on the
		// second line.
		const position = predictionTest.findCaretPosition(
			data,
			1100,
			1800,
			2050,
			measure,
			{
				paragraph: 0,
				index: 8,
			},
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.start, 8);
		nodeassert.strictEqual(position.offset, 0);
	});

	it('places the caret on the new line after a wrap', function () {
		// Typing wraps "aaa bbb" into "aaa" and "bbbx". The caret at the end now belongs on
		// the second line, not at the end of the first.
		const data = makeWrapSlide([
			wrapRun('aaa bbb', 0, 0, { width: 700, lineHeight: 400, align: 'left' }),
		]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const geometry = predictionTest.caretGeometry(
			data,
			1700,
			1800,
			2050,
			{ paragraph: 0, index: 8 },
			measure,
		);
		nodeassert.ok(geometry);
		// End of "bbbx" on the second line: origin 1000 plus its four 100-wide characters,
		// and a top below the first line's baseline at 2000.
		nodeassert.strictEqual(geometry.x, 1400);
		nodeassert.strictEqual(geometry.height, 400);
		nodeassert.ok(geometry.top > 2000);
	});

	it('measures the caret in slide twips under a scaled transform', function () {
		// The end of "Hi" is at local x 200, which the doubling transform puts at slide x
		// 1400. The line is 216 local twips high with an ascent of 144, so 432 and 288 in
		// slide twips.
		const data = predictionTest.makeScaledSlide();
		const position = predictionTest.findCaretPosition(
			data,
			1390,
			1800,
			2050,
			measure,
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.text, 'Hi');
		nodeassert.strictEqual(position.offset, 2);
		nodeassert.deepStrictEqual(position.geometry(), {
			x: 1400,
			top: 1712,
			height: 432,
		});
	});

	it('leaves an index before the first run to the server', function () {
		// A leading tab has no text run, so the only run windows the text from offset 1.
		// Index 0 is before the tab, where no run shows the text.
		const data = predictionTest.makeSharedWrapSlide(
			'\taaa bbb',
			[{ start: 1, length: 7, f: 0 }],
			700,
		);
		const position = predictionTest.findCaretPosition(
			data,
			1000,
			1800,
			2050,
			measure,
			{
				paragraph: 0,
				index: 0,
			},
		);
		nodeassert.strictEqual(position, null);
	});

	it('does not type into a field when the caret has no logical position', function () {
		// The caret pixels fall inside the field run "example.com" at 1600..2700. The field
		// is not the paragraph's text, and the runs around it are too far away.
		const data = predictionTest.makeFieldSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			2100,
			1800,
			2050,
			'x',
			measure,
		);
		nodeassert.strictEqual(advance, 0);
		nodeassert.strictEqual(predictionTest.runsOf(data)[1].text, 'example.com');
	});

	it('maps a caret at a line break to the line it sits on', function () {
		// "line1" and "line2" are split by a line break with no character, so index 5 ends
		// line 1 and starts line 2. The caret's line decides.
		const data = makeSharedWrapSlide('line1line2', [
			{ start: 0, length: 5, f: 0 },
			{ start: 5, length: 5, f: 400 },
		]);
		// Caret on the first line maps to the end of "line1".
		const onLine1 = predictionTest.findCaretPosition(
			data,
			1500,
			1800,
			2050,
			measure,
			{
				paragraph: 0,
				index: 5,
			},
		);
		nodeassert.ok(onLine1);
		nodeassert.strictEqual(onLine1.run.start, 0);
		nodeassert.strictEqual(onLine1.offset, 5);
		// The same index on the second line maps to the start of "line2".
		const onLine2 = predictionTest.findCaretPosition(
			data,
			1000,
			2200,
			2450,
			measure,
			{
				paragraph: 0,
				index: 5,
			},
		);
		nodeassert.ok(onLine2);
		nodeassert.strictEqual(onLine2.run.start, 5);
		nodeassert.strictEqual(onLine2.offset, 0);
	});

	it('inserts into the object named by id without a caret rectangle', function () {
		const data = makeTwoIdSlide();
		// The zeros point at neither box. The id picks the second one and the logical
		// position places the edit.
		const advance = predictionTest.predictTextInsert(
			data,
			0,
			0,
			0,
			'x',
			measure,
			{ paragraph: 0, index: 2 },
			9,
		);
		nodeassert.strictEqual(advance, 100);
		const texts = [objectAt(data, 0), objectAt(data, 1)].map(
			(object: any) => object.primitives[0].children[0].text,
		);
		nodeassert.deepStrictEqual(texts, ['Hi', 'Hix']);
	});

	it('reports the id of the object the caret sits in', function () {
		const data = makeTwoIdSlide();
		const position = predictionTest.findCaretPosition(
			data,
			1200,
			4800,
			5050,
			measure,
			{
				paragraph: 0,
				index: 2,
			},
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.objectId, 9);
	});

	it('does nothing for an object id the tree does not carry', function () {
		const data = makeTwoIdSlide();
		const advance = predictionTest.predictTextInsert(
			data,
			0,
			0,
			0,
			'x',
			measure,
			{ paragraph: 0, index: 2 },
			42,
		);
		nodeassert.strictEqual(advance, 0);
	});

	it('places an edit found by id at a line break on the line that starts there', function () {
		// Index 5 ends "line1" and starts "line2". Without a caret rectangle the edit goes
		// to the start of the next line, where the caret sits after a wrap.
		const data = makeSharedWrapSlide('line1line2', [
			{ start: 0, length: 5, f: 0 },
			{ start: 5, length: 5, f: 400 },
		]);
		const position = predictionTest.findCaretPosition(
			data,
			0,
			0,
			0,
			measure,
			{ paragraph: 0, index: 5 },
			1,
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.start, 5);
		nodeassert.strictEqual(position.offset, 0);
	});
});
