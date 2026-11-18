/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('LocalCellCursor', function () {
	let sent: string[];
	let savedSend: any;
	let savedTimerRegistry: any;
	let savedSequence: number;

	beforeEach(function () {
		installFakeClock();
		savedTimerRegistry = app.timerRegistry;
		app.timerRegistry = new TimerRegistry();

		// Replace sendMessage with a dummy one that stores messages to 'sent'
		sent = [];
		savedSend = app.socket.sendMessage;
		app.socket.sendMessage = (message: string) => sent.push(message);

		savedSequence = app.file.inputSequence;
		app.file.inputSequence = 12;
	});

	afterEach(function () {
		LocalCellCursor.forget();
		app.socket.sendMessage = savedSend;
		app.file.inputSequence = savedSequence;
		app.timerRegistry = savedTimerRegistry;
		restoreClock();
	});

	it('drops only the reports older than the move reported to the engine', function () {
		// Keep every report while no move waits for a reply.
		nodeassert.strictEqual(LocalCellCursor.isStale(undefined), false);
		nodeassert.strictEqual(LocalCellCursor.isStale(3), false);

		// Pending local move on the client should cause any cellcursor reports
		// from the engine to be dropped.
		LocalCellCursor.moved({ part: 0, cell: [2, 4], anchor: null });
		nodeassert.strictEqual(LocalCellCursor.isStale(undefined), true);
		nodeassert.strictEqual(LocalCellCursor.isStale(1000), true);

		// Report a move with sequence 5 to the engine; older reports
		// should be dropped.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: null }, 5);
		nodeassert.strictEqual(LocalCellCursor.isStale(undefined), true);
		nodeassert.strictEqual(LocalCellCursor.isStale(4), true);
		nodeassert.strictEqual(LocalCellCursor.isStale(5), false);
		nodeassert.strictEqual(LocalCellCursor.isStale(6), false);

		// Forget the move once replied to; no report should be
		// dropped after it.
		LocalCellCursor.forget();
		nodeassert.strictEqual(LocalCellCursor.isStale(4), false);
	});

	it('corrects the client only when the engine put the cursor elsewhere', function () {
		// Move to C5 with no selection; the same state back should be
		// no correction.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: null }, 5);
		let state: LocalCellCursor.State = {
			part: 0,
			cursor: { cell: [2, 4], visible: true },
			selection: [],
		};
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 5), false);

		// Move to C5 again but get the cursor back on A1; it should
		// be a correction.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: null }, 6);
		state = { part: 0, cursor: { cell: [0, 0], visible: true }, selection: [] };
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 6), true);

		// Select from A2 to C5; the cursor should stay on the
		// anchor, A2.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: [0, 1] }, 7);
		state = {
			part: 0,
			cursor: { cell: [0, 1], visible: true },
			selection: [[0, 1, 2, 4]],
		};
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 7), false);

		// Select from C5 up to A2; a selection widened to D5 should
		// be a correction.
		LocalCellCursor.reported({ part: 0, cell: [0, 1], anchor: [2, 4] }, 8);
		state = {
			part: 0,
			cursor: { cell: [2, 4], visible: true },
			selection: [[0, 1, 3, 4]],
		};
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 8), true);

		// Select from A2 to C5 but get no selection back; it should
		// be a correction.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: [0, 1] }, 9);
		state = { part: 0, cursor: { cell: [0, 1], visible: true }, selection: [] };
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 9), true);
	});

	it('finds the moving end of a selection from its cursor', function () {
		// The end should be the corner opposite the cursor, in each corner.
		const range: LocalCellCursor.Range = [1, 2, 4, 6];
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd(range, [1, 2, 1, 2]),
			[4, 6],
		);
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd(range, [4, 2, 4, 2]),
			[1, 6],
		);
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd(range, [1, 6, 1, 6]),
			[4, 2],
		);
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd(range, [4, 6, 4, 6]),
			[1, 2],
		);

		// A range of one column or row should keep that column or row.
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd([3, 2, 3, 6], [3, 2, 3, 2]),
			[3, 6],
		);
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd([3, 2, 3, 2], [3, 2, 3, 2]),
			[3, 2],
		);

		// A merged cell as the anchor should count as the corner it covers.
		nodeassert.deepStrictEqual(
			LocalCellCursor.selectionEnd(range, [3, 5, 4, 6]),
			[1, 2],
		);

		// A cursor inside or outside the range should give no end.
		nodeassert.strictEqual(
			LocalCellCursor.selectionEnd(range, [2, 3, 2, 3]),
			null,
		);
		nodeassert.strictEqual(
			LocalCellCursor.selectionEnd(range, [1, 3, 1, 3]),
			null,
		);
		nodeassert.strictEqual(
			LocalCellCursor.selectionEnd(range, [5, 2, 5, 2]),
			null,
		);

		// The range between the cursor and the end should order its corners.
		nodeassert.deepStrictEqual(
			LocalCellCursor.rangeBetween([4, 6], [1, 2]),
			range,
		);
	});

	it('notifies the engine whether the client moves the cell cursor', function () {
		LocalCellCursor.reportMode(true);
		LocalCellCursor.reportMode(false);
		nodeassert.deepStrictEqual(sent, [
			'setlocalcellcursor enabled=true',
			'setlocalcellcursor enabled=false',
		]);
	});

	it('reports the move once the cursor rests', function () {
		// Move twice in a row; nothing should go out before the
		// cursor rests.
		LocalCellCursor.moved({ part: 0, cell: [2, 4], anchor: null });
		advanceClock(200);
		LocalCellCursor.moved({ part: 1, cell: [3, 4], anchor: [0, 1] });
		advanceClock(200);
		nodeassert.deepStrictEqual(sent, []);

		// Let the cursor rest; the last move should go out once with the
		// next input sequence number.
		const sequence = app.file.inputSequence + 1;
		advanceClock(100);
		advanceClock(1000);
		nodeassert.deepStrictEqual(sent, [
			'setcellcursor part=1 col=3 row=4 anchorcol=0 anchorrow=1 sequence=' +
				sequence,
		]);
		nodeassert.strictEqual(LocalCellCursor.isStale(sequence - 1), true);
		nodeassert.strictEqual(LocalCellCursor.isStale(sequence), false);
	});

	it('reports the move ahead of a message that may read the cursor', function () {
		// Send a tile request, a hover and a sheet geometry request; each
		// should go out alone.
		LocalCellCursor.moved({ part: 0, cell: [2, 4], anchor: null });
		const tile = 'tilecombine nviewid=0 part=0 width=256';
		nodeassert.strictEqual(LocalCellCursor.reportBefore(tile), tile);
		const hover = 'mouse type=move x=100 y=200 count=1 buttons=0 modifier=0';
		nodeassert.strictEqual(LocalCellCursor.reportBefore(hover), hover);
		const geometry = 'commandvalues command=.uno:SheetGeometryData?rows=1';
		nodeassert.strictEqual(LocalCellCursor.reportBefore(geometry), geometry);
		nodeassert.deepStrictEqual(sent, []); // The local move should not have been sent.

		// Send a key numbered just now; the move should take its number,
		// and the key the next one.
		const keySequence = cool.InputSequence.next();
		const key = 'key type=input char=97 key=0 sequence=' + keySequence;
		const renumbered = LocalCellCursor.reportBefore(key);
		nodeassert.deepStrictEqual(sent, [
			'setcellcursor part=0 col=2 row=4 sequence=' + keySequence,
		]);
		nodeassert.strictEqual(
			renumbered,
			'key type=input char=97 key=0 sequence=' + (keySequence + 1),
		);

		// Send a command with no number after a new move; the move
		// should go out first with the next number.
		LocalCellCursor.moved({ part: 0, cell: [3, 4], anchor: null });
		const command = 'uno .uno:Bold';
		nodeassert.strictEqual(LocalCellCursor.reportBefore(command), command);
		nodeassert.strictEqual(
			sent[1],
			'setcellcursor part=0 col=3 row=4 sequence=' + (keySequence + 2),
		);

		// Send another message; the move should not go out twice.
		LocalCellCursor.reportBefore(command);
		nodeassert.strictEqual(sent.length, 2);
	});

	it('ignores a reply that is not for the move reported to the engine', function () {
		// Get a reply while no move waits; it should be no
		// correction.
		const state: LocalCellCursor.State = {
			part: 0,
			cursor: { cell: [0, 0], visible: true },
			selection: [],
		};
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 5), false);

		// Get the reply to an older move; it should be no
		// correction, and the move should still wait for its own
		// reply.
		LocalCellCursor.reported({ part: 0, cell: [2, 4], anchor: null }, 6);
		nodeassert.strictEqual(LocalCellCursor.isCorrection(state, 5), false);
		nodeassert.strictEqual(LocalCellCursor.isStale(5), true);
	});

	it('slides the cell cursor between two rectangles', function () {
		const from = new cool.SimpleRectangle(0, 0, 100, 50);
		const to = new cool.SimpleRectangle(200, 100, 300, 50);

		// Start at the old rectangle and end at the new one.
		let drawn = CellCursorSection.interpolate(from, to, 0);
		nodeassert.deepStrictEqual(drawn.toArray(), [0, 0, 100, 50]);
		drawn = CellCursorSection.interpolate(from, to, 1);
		nodeassert.deepStrictEqual(drawn.toArray(), [200, 100, 300, 50]);

		// Slow down on arrival: half the time covers three quarters of the way.
		// Check at 0.5, which keeps the arithmetic exact.
		drawn = CellCursorSection.interpolate(from, to, 0.5);
		nodeassert.deepStrictEqual(drawn.toArray(), [150, 75, 250, 50]);

		// Reject a progress outside the slide.
		nodeassert.throws(() => CellCursorSection.interpolate(from, to, -1));
		nodeassert.throws(() => CellCursorSection.interpolate(from, to, 2));
		nodeassert.throws(() => CellCursorSection.interpolate(from, to, NaN));
	});
});
