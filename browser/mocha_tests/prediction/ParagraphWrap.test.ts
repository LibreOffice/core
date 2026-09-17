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

describe('text prediction paragraph wrap', function () {
	const {
		measure,
		runsOf,
		wrapRun,
		makeWrapSlide,
		makeSharedWrapSlide,
		makeIndentedSlide,
	} = predictionTest;

	it('re-wraps a paragraph to a new line when an insert overflows', function () {
		// "aaa bbb" fills the 700-wide area exactly. Typing 'x' at the end pushes the
		// second word onto its own line.
		const data = makeWrapSlide([
			wrapRun('aaa bbb', 0, 0, { width: 700, lineHeight: 400, align: 'left' }),
		]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aaa');
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bbbx');
		// The second line steps down by one line height and starts at the left.
		nodeassert.strictEqual(runs[1].matrix[5], 400);
		nodeassert.strictEqual(runs[0].matrix[4], 0);
		nodeassert.strictEqual(runs[1].matrix[4], 0);
	});

	it('centers each line of a re-wrapped paragraph', function () {
		const data = makeWrapSlide([
			wrapRun('aaa bbb', 0, 0, {
				width: 700,
				lineHeight: 400,
				align: 'center',
			}),
		]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		// "aaa" (300 wide) and "bbbx" (400 wide) are each centered in the 700 area: offsets
		// (700-300)/2 and (700-400)/2.
		nodeassert.strictEqual(runs[0].matrix[4], 200);
		nodeassert.strictEqual(runs[1].matrix[4], 150);
	});

	it('right-aligns each line of a re-wrapped paragraph', function () {
		const data = makeWrapSlide([
			wrapRun('aaa bbb', 0, 0, { width: 700, lineHeight: 400, align: 'right' }),
		]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		// Each line ends at the area's right edge, so it starts at the area width minus the
		// line width.
		nodeassert.strictEqual(runs[0].matrix[4], 400);
		nodeassert.strictEqual(runs[1].matrix[4], 300);
	});

	it('stretches a justified line to fill the wrap width', function () {
		// "aa bb cc" fills the 800-wide area. Typing 'x' wraps "ccx" down, leaving "aa bb"
		// to be justified across the full width.
		const data = makeWrapSlide([
			wrapRun('aa bb cc', 0, 0, {
				width: 800,
				lineHeight: 400,
				align: 'justify',
			}),
		]);
		predictionTest.predictTextInsert(data, 1800, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		// The first line's one space widens by 300 so the line reaches 800. The last line
		// keeps its natural advances.
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aa bb');
		nodeassert.deepStrictEqual(runs[0].advances, [100, 100, 400, 100, 100]);
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'ccx');
		nodeassert.deepStrictEqual(runs[1].advances, [100, 100, 100]);
	});

	it('pulls a word up when deleting from a wrapped paragraph', function () {
		// "aaaa bbbb" on two lines of an 800-wide area. Both runs window the shared
		// paragraph text, and the first keeps the break space at its end. Deleting one 'a'
		// lets "aaa bbbb" fit on one line.
		const paragraphText = 'aaaa bbbb';
		const options = { width: 800, lineHeight: 400, align: 'left' };
		const line1 = wrapRun(paragraphText, 0, 0, options);
		line1.textLength = 5; // "aaaa "
		const line2 = wrapRun(paragraphText, 0, 400, options);
		line2.textPosition = 5;
		line2.textLength = 4; // "bbbb"
		const data = makeWrapSlide([line1, line2]);
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
		const runs = runsOf(data);
		// The line and the pulled-up word come from different source runs, so they stay
		// separate runs, now both on the first line.
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aaa ');
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bbbb');
		nodeassert.strictEqual(runs[0].matrix[5], 0);
		nodeassert.strictEqual(runs[1].matrix[5], 0);
	});

	it('removes the last characters with repeated backspace at the end of a wrapped paragraph', function () {
		// The two line windows touch, because the first line keeps its break space. Two
		// backspaces at the end remove the last two characters, the second one after the
		// first has re-wrapped the paragraph.
		const paragraphText = 'aaaa bbbb cc';
		const options = { width: 900, lineHeight: 400, align: 'left' };
		const line1 = wrapRun(paragraphText, 0, 0, options);
		line1.textLength = 10; // "aaaa bbbb "
		const line2 = wrapRun(paragraphText, 0, 400, options);
		line2.textPosition = 10;
		line2.textLength = 2; // "cc"
		const data = makeWrapSlide([line1, line2]);
		const logical = { paragraph: 0, index: paragraphText.length };

		// The caret sits at the end of the second line (x = 1200).
		const first = predictionTest.predictTextDelete(
			data,
			1200,
			2200,
			2450,
			1,
			0,
			measure,
			logical,
		);
		nodeassert.strictEqual(first.removed, 1);
		logical.index -= first.removedBefore;

		const second = predictionTest.predictTextDelete(
			data,
			1100,
			2200,
			2450,
			1,
			0,
			measure,
			logical,
		);
		nodeassert.strictEqual(second.removed, 1);
		logical.index -= second.removedBefore;

		// Both 'c's are gone and every 'a' and 'b' survived.
		const joined = runsOf(data)
			.map(cool.visibleText)
			.join('')
			.replace(/ /g, '');
		nodeassert.strictEqual(joined, 'aaaabbbb');
	});

	it('gives each re-wrapped line the font of its own text', function () {
		// A bold first line and a normal second line in a 400-wide area. After an edit
		// re-wraps the paragraph, each line keeps its own weight.
		const data = makeSharedWrapSlide(
			'aaaa bbbb',
			[
				{ start: 0, length: 4, f: 0, weight: 7 },
				{ start: 5, length: 4, f: 400, weight: 5 },
			],
			400,
		);
		// Type at the end of the second (normal) line.
		predictionTest.predictTextInsert(data, 1400, 2300, 2450, 'X', measure, {
			paragraph: 0,
			index: 9,
		});
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(runs[0].weight, 7);
		nodeassert.strictEqual(runs[1].weight, 5);
	});

	it('keeps both fonts on a re-wrapped line', function () {
		// Bold "aa " then normal "bb cc" on one line of an 800-wide area. Typing at the end
		// wraps it, and the first line keeps both fonts as two runs.
		const data = makeSharedWrapSlide(
			'aa bb cc',
			[
				{ start: 0, length: 3, e: 0, f: 0, weight: 7 },
				{ start: 3, length: 5, e: 300, f: 0, weight: 5 },
			],
			800,
		);
		// "aa bb cc" is 800 wide. One more character overflows and wraps "cc" down, leaving
		// "aa bb" (bold + normal) on the first line.
		predictionTest.predictTextInsert(data, 1800, 1800, 2050, 'X', measure, {
			paragraph: 0,
			index: 8,
		});
		const runs = runsOf(data);
		// First line: bold "aa " then normal "bb". Second line: normal "ccX".
		nodeassert.strictEqual(runs.length, 3);
		nodeassert.strictEqual(runs[0].weight, 7);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aa ');
		nodeassert.strictEqual(runs[1].weight, 5);
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bb');
		nodeassert.strictEqual(cool.visibleText(runs[2]), 'ccX');
	});

	it("keeps the server's advances of the text a re-wrap moves", function () {
		// The server's letters are 50 wide, half the browser's. Typing 'x' at the end of
		// "aaa bbb" in a 360-wide area wraps "bbbx" down. Every rebuilt run steps by the
		// server's advances, and the typed 'x' is scaled to match.
		const run = wrapRun('aaa bbb', 0, 0, {
			width: 360,
			lineHeight: 400,
			align: 'left',
		});
		run.advances = [50, 50, 50, 50, 50, 50, 50];
		const data = makeWrapSlide([run]);
		predictionTest.predictTextInsert(data, 1350, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aaa');
		nodeassert.deepStrictEqual(runs[0].advances, [50, 50, 50]);
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bbbx');
		nodeassert.deepStrictEqual(runs[1].advances, [50, 50, 50, 50]);
	});

	it("breaks lines by the server's widths when the browser measures wider", function () {
		// "ab cd ef gh" is 550 by the server and 1100 by the browser. Typing 'x' overflows
		// the 580-wide area. By the server's widths "ab cd ef" still fits on the first
		// line.
		const run = wrapRun('ab cd ef gh', 0, 0, {
			width: 580,
			lineHeight: 400,
			align: 'left',
		});
		run.advances = new Array(11).fill(50);
		const data = makeWrapSlide([run]);
		predictionTest.predictTextInsert(data, 1550, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'ab cd ef');
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'ghx');
	});

	it("right-aligns the rebuilt lines by the server's widths", function () {
		// The same text right-aligned in the 580-wide area, so it starts at local x 30.
		// "ab cd ef" is 400 wide and ends at 580, and so does "ghx" at 150 wide.
		const run = wrapRun('ab cd ef gh', 30, 0, {
			width: 580,
			lineHeight: 400,
			align: 'right',
		});
		run.advances = new Array(11).fill(50);
		const data = makeWrapSlide([run]);
		predictionTest.predictTextInsert(data, 1580, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(runs[0].matrix[4], 180);
		nodeassert.strictEqual(runs[1].matrix[4], 430);
	});

	it('keeps a justified line unwrapped while its text fits without the stretch', function () {
		// The server stretched the space of "aa bb" so the line fills 400. Without the
		// stretch the line is 250 by the server's letters, so a typed 'c' still fits.
		const run = wrapRun('aa bb', 0, 0, {
			width: 400,
			lineHeight: 400,
			align: 'justify',
		});
		run.advances = [50, 50, 200, 50, 50];
		const data = makeWrapSlide([run]);
		predictionTest.predictTextInsert(data, 1400, 1800, 2050, 'c', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 1);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aa bbc');
	});

	it('drops every space at a line break', function () {
		// "aaa  bbb" has two spaces. Typing 'x' wraps "bbbx" down, and neither space starts
		// the second line.
		const data = makeWrapSlide([
			wrapRun('aaa  bbb', 0, 0, { width: 850, lineHeight: 400, align: 'left' }),
		]);
		predictionTest.predictTextInsert(data, 1800, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aaa');
		nodeassert.strictEqual(runs[1].textPosition, 5);
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bbbx');
	});

	it('keeps the decoration of an underlined run through a re-wrap', function () {
		// An underlined "aaa bbb" wraps when 'x' is typed at its end. Both rebuilt lines
		// stay underlined runs.
		const run = wrapRun('aaa bbb', 0, 0, {
			width: 700,
			lineHeight: 400,
			align: 'left',
		});
		run.type = 'textDecoratedPortion';
		run.underline = 1;
		const data = makeWrapSlide([run]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		for (const rebuilt of runs) {
			nodeassert.strictEqual(rebuilt.type, 'textDecoratedPortion');
			nodeassert.strictEqual(rebuilt.underline, 1);
			nodeassert.strictEqual(rebuilt.advances.length, rebuilt.textLength);
		}
	});

	it('places a caret at the start of a rebuilt line by its paragraph index', function () {
		// Typing wraps "aaa bbb" into "aaa" and "bbbx". The second line starts at paragraph
		// offset 4, "aaa" plus the wrap space, so a caret at index 4 lands at its start.
		const data = makeWrapSlide([
			wrapRun('aaa bbb', 0, 0, { width: 700, lineHeight: 400, align: 'left' }),
		]);
		predictionTest.predictTextInsert(data, 1700, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs[1].textPosition, 4);
		const position = predictionTest.findCaretPosition(
			data,
			1100,
			2200,
			2450,
			measure,
			{
				paragraph: 0,
				index: 4,
			},
		);
		nodeassert.ok(position);
		nodeassert.strictEqual(position.run.visibleText, 'bbbx');
		nodeassert.strictEqual(position.offset, 0);
	});

	it('keeps the run intact when an insert still fits', function () {
		// Inserting within the wrap width does not rebuild the paragraph.
		const data = makeWrapSlide([
			wrapRun('aa', 0, 0, { width: 700, lineHeight: 400, align: 'left' }),
		]);
		predictionTest.predictTextInsert(data, 1200, 1800, 2050, 'x', measure);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 1);
		nodeassert.strictEqual(runs[0].text, 'aax');
	});

	it('keeps a line unwrapped when it fits by the server widths but not by the browser widths', function () {
		const data = makeWrapSlide([
			wrapRun('abcde', 0, 0, { width: 490, lineHeight: 400, align: 'left' }),
		]);
		// The server's advances total 400 in a 490-wide area. The typed 'x' is 100 in the
		// browser and scales to 80 in the server's measure (400/500), so the line fits at
		// 480 and does not wrap.
		runsOf(data)[0].advances = [80, 80, 80, 80, 80];
		predictionTest.predictTextInsert(data, 1400, 1800, 2050, 'x', measure, {
			paragraph: 0,
			index: 5,
		});
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 1);
		nodeassert.strictEqual(runs[0].text, 'abcdex');
	});

	it('keeps paragraph offsets through a re-wrap when text precedes the first run', function () {
		// A leading tab has no text run, so the only run windows the text from offset 1.
		// Typing at the end overflows the 700-wide area, and the rebuilt runs keep
		// windowing the same buffer from the same offsets.
		const data = makeSharedWrapSlide(
			'\taaa bbb',
			[{ start: 1, length: 7, f: 0 }],
			700,
		);
		const advance = predictionTest.predictTextInsert(
			data,
			1700,
			1800,
			2050,
			'x',
			measure,
			{
				paragraph: 0,
				index: 8,
			},
		);
		nodeassert.strictEqual(advance, 100);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.length, 2);
		nodeassert.strictEqual(runs[0].text, '\taaa bbbx');
		nodeassert.strictEqual(runs[0].textPosition, 1);
		nodeassert.strictEqual(cool.visibleText(runs[0]), 'aaa');
		nodeassert.strictEqual(runs[1].textPosition, 5);
		nodeassert.strictEqual(cool.visibleText(runs[1]), 'bbbx');
	});

	it('removes the break space on a backspace at the start of a wrapped line', function () {
		// "aaaa " and "bbbb" on two lines of an 800-wide area, with the caret at the start
		// of the second line, offset 5. Backspace removes the space and the text fits on
		// one line.
		const data = makeSharedWrapSlide(
			'aaaa bbbb',
			[
				{ start: 0, length: 5, f: 0 },
				{ start: 5, length: 4, f: 400 },
			],
			800,
		);
		const result = predictionTest.predictTextDelete(
			data,
			1000,
			2200,
			2450,
			1,
			0,
			measure,
			{
				paragraph: 0,
				index: 5,
			},
		);
		nodeassert.strictEqual(result.removed, 1);
		nodeassert.strictEqual(result.removedBefore, 1);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.map(cool.visibleText).join('|'), 'aaaa|bbbb');
		nodeassert.strictEqual(runs[0].matrix[5], 0);
		nodeassert.strictEqual(runs[1].matrix[5], 0);
		nodeassert.strictEqual(runs[1].matrix[4], 400);
		nodeassert.strictEqual(runs[0].text, 'aaaabbbb');
	});

	it('joins a mixed-font paragraph onto one line when it fits', function () {
		// Bold characters are 200 wide, normal ones 100. Deleting one 'b' leaves 1000 + 300
		// = 1300, which fits the 1500-wide area on one line. In bold throughout, the line
		// would measure 1600.
		const byWeight: cool.TextRunMeasure = (run, text) =>
			text.length * (run.weight === 7 ? 200 : 100);
		const data = makeSharedWrapSlide(
			'aaaa bbbb',
			[
				{ start: 0, length: 5, f: 0, weight: 7 },
				{ start: 5, length: 4, f: 400 },
			],
			1500,
		);
		const result = predictionTest.predictTextDelete(
			data,
			1400,
			2200,
			2450,
			1,
			0,
			byWeight,
			{ paragraph: 0, index: 9 },
		);
		nodeassert.strictEqual(result.removed, 1);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.map(cool.visibleText).join('|'), 'aaaa |bbb');
		nodeassert.strictEqual(runs[0].matrix[5], 0);
		nodeassert.strictEqual(runs[1].matrix[5], 0);
		nodeassert.strictEqual(runs[1].matrix[4], 1000);
	});

	it('wraps an indented paragraph at the width its line reports', function () {
		// The text area is 700 wide, but the line is indented by 200 and has 500, which
		// "aaa b" fills. Typing 'x' overflows the line though it would fit the area, so the
		// paragraph wraps and both lines start at the indent.
		const data = makeIndentedSlide(
			'aaa b',
			[{ start: 0, length: 5, e: 200, f: 0, indent: 200, width: 500 }],
			700,
		);
		const advance = predictionTest.predictTextInsert(
			data,
			1700,
			1800,
			2050,
			'x',
			measure,
			{
				paragraph: 0,
				index: 5,
			},
		);
		nodeassert.strictEqual(advance, 100);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.map(cool.visibleText).join('|'), 'aaa|bx');
		nodeassert.strictEqual(runs[0].matrix[4], 200);
		nodeassert.strictEqual(runs[1].matrix[4], 200);
		nodeassert.strictEqual(runs[1].matrix[5], 400);
		nodeassert.strictEqual(runs[1].lineIndent, 200);
		nodeassert.strictEqual(runs[1].lineWidth, 500);
	});

	it('keeps a hanging indent on the lines after the first through a re-wrap', function () {
		// The first line starts at the area's left with all 700. The second hangs 200 in
		// with 500. Deleting one 'b' leaves "aaaa bbb" too wide for one line, so the
		// paragraph rebuilds with the second line at its own indent.
		const data = makeIndentedSlide(
			'aaaa bbbb',
			[
				{ start: 0, length: 5, e: 0, f: 0, indent: 0, width: 700 },
				{ start: 5, length: 4, e: 200, f: 400, indent: 200, width: 500 },
			],
			700,
		);
		const result = predictionTest.predictTextDelete(
			data,
			1600,
			2200,
			2450,
			1,
			0,
			measure,
			{
				paragraph: 0,
				index: 9,
			},
		);
		nodeassert.strictEqual(result.removed, 1);
		const runs = runsOf(data);
		nodeassert.strictEqual(runs.map(cool.visibleText).join('|'), 'aaaa|bbb');
		nodeassert.strictEqual(runs[0].matrix[4], 0);
		nodeassert.strictEqual(runs[1].matrix[4], 200);
		nodeassert.strictEqual(runs[1].lineIndent, 200);
	});
});
