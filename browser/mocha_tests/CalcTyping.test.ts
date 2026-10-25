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

describe('CalcTyping', function () {
	// In tile twips, the run starts 50 right of the cell corner with its baseline 200 down.
	const viewId = 3;
	const part = 1;
	const origin: cool.CalcCellOrigin = (column: number, row: number) =>
		column === 2 && row === 4 ? new cool.Point(1000, 2000) : undefined;

	function cellEdit(
		text: string,
		column = 2,
		editor = viewId,
	): cool.CalcTextEdit {
		return {
			kind: 'texteditoverlay',
			viewId: editor,
			column,
			row: 4,
			endColumn: column,
			endRow: 4,
			background: '#ffffff',
			primitives: [
				{
					type: 'textSimplePortion',
					text,
					textPosition: 0,
					textLength: text.length,
					advances: Array.from(text, () => 100),
					matrix: [1, 0, 0, 1, 50, 200],
					fontSize: 200,
					lineHeight: 300,
					lineAscent: 240,
					familyname: 'Arial',
					paragraph: 0,
				} as cool.Primitive,
			],
		};
	}

	function textOf(edits: cool.CalcTextEdit[]): string {
		const run = edits[0].primitives[0] as cool.TextSimplePortionPrimitive;
		return cool.visibleText(run);
	}

	// The caret before the given index of the text, with selected text from the anchor.
	function caretAt(index: number, anchor?: cool.LogicalCaret): cool.CalcCaret {
		return {
			rectangle: new cool.SimpleRectangle(1050 + index * 100, 1960, 0, 300),
			logical: { paragraph: 0, index },
			anchor: anchor ?? null,
			selected: anchor !== undefined,
		};
	}

	function newTyping(): cool.CalcTyping {
		return new cool.CalcTyping(new cool.TextMeasurer(predictionTest.measure));
	}

	// The ticket of an input message that went out as the number.
	function sentAs(sequence: number): cool.InputTicket {
		const ticket = new cool.InputTicket();
		ticket.goesOutAs(sequence);
		return ticket;
	}

	function type(
		typing: cool.CalcTyping,
		edits: cool.CalcTextEdit[],
		caret: cool.CalcCaret,
		text: string,
		sequence: number,
	): cool.CalcTypedCaret | null {
		return typing.type(
			part,
			edits,
			viewId,
			origin,
			caret,
			sentAs(sequence),
			(selected) => cool.typedChange(text, selected),
		);
	}

	it('shows typed text in the cell that the view edits', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		const typed = type(typing, edits, caretAt(3), 'd', 5);

		nodeassert.strictEqual(textOf(typing.editsOf(part, edits)), 'abcd');
		nodeassert.strictEqual(typed?.geometry.x, 1450);
		nodeassert.strictEqual(typed?.geometry.top, 1960);
		nodeassert.deepStrictEqual(typed?.logical, { paragraph: 0, index: 4 });
	});

	it('leaves the text edits of the engine as they came', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		nodeassert.strictEqual(textOf(edits), 'abc');
	});

	it('shows the typing in the sheet typed in only', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		const otherSheet = [cellEdit('xyz')];
		nodeassert.strictEqual(typing.editsOf(part + 1, otherSheet), otherSheet);
	});

	it('shows nothing for a view that edits no cell', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc', 2, viewId + 1)];

		nodeassert.strictEqual(type(typing, edits, caretAt(3), 'd', 5), null);
		nodeassert.strictEqual(typing.isTyping(), false);
	});

	it('shows a deletion before the caret', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		const typed = typing.type(
			part,
			edits,
			viewId,
			origin,
			caretAt(3),
			sentAs(5),
			(selected) => cool.deletedChange(1, 0, selected),
		);

		nodeassert.strictEqual(textOf(typing.editsOf(part, edits)), 'ab');
		nodeassert.deepStrictEqual(typed?.logical, { paragraph: 0, index: 2 });
	});

	it('replaces the selected text with the typed text', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3, { paragraph: 0, index: 1 }), 'x', 5);

		nodeassert.strictEqual(textOf(typing.editsOf(part, edits)), 'ax');
	});

	it('leaves a selection that starts in another paragraph to the engine', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];

		nodeassert.strictEqual(
			type(typing, edits, caretAt(3, { paragraph: 1, index: 0 }), 'x', 5),
			null,
		);
	});

	it('draws the edits of the engine once they include all the typing', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		const answer = [cellEdit('abcd')];
		nodeassert.strictEqual(
			typing.answered(part, answer, 5, viewId, origin),
			null,
		);
		nodeassert.strictEqual(typing.editsOf(part, answer), answer);
		nodeassert.strictEqual(typing.isTyping(), false);
	});

	it('types the rest again into edits that do not include all the typing', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);
		type(typing, edits, caretAt(4), 'e', 6);

		const answer = [cellEdit('abcd')];
		const typed = typing.answered(part, answer, 5, viewId, origin);
		nodeassert.strictEqual(textOf(typing.editsOf(part, answer)), 'abcde');
		nodeassert.strictEqual(textOf(answer), 'abcd');
		nodeassert.deepStrictEqual(typed?.logical, { paragraph: 0, index: 5 });
	});

	it('drops the typing when the view edits another cell', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		const answer = [cellEdit('abc', 3)];
		nodeassert.strictEqual(
			typing.answered(part, answer, 4, viewId, origin),
			null,
		);
		nodeassert.strictEqual(typing.editsOf(part, answer), answer);
	});

	it('keeps the caret here while the engine reports an older one', function () {
		const typing = newTyping();
		type(typing, [cellEdit('abc')], caretAt(3), 'd', 5);

		nodeassert.strictEqual(
			typing.keepsCaret({ inputSequence: 4, paragraph: '0', index: '3' }),
			true,
		);
	});

	it('takes the caret of the engine once it includes the typing', function () {
		const typing = newTyping();
		type(typing, [cellEdit('abc')], caretAt(3), 'd', 5);

		nodeassert.strictEqual(
			typing.keepsCaret({ inputSequence: 5, paragraph: '0', index: '4' }),
			false,
		);
		nodeassert.strictEqual(typing.isTyping(part), true);
	});

	it('drops the typing when the engine puts the caret somewhere else', function () {
		const typing = newTyping();
		type(typing, [cellEdit('abc')], caretAt(3), 'd', 5);

		nodeassert.strictEqual(
			typing.keepsCaret({ inputSequence: 5, paragraph: '0', index: '2' }),
			false,
		);
		nodeassert.strictEqual(typing.isTyping(), false);
	});

	it('shows no typing after an input that moves the edit until the engine reports the caret', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		typing.holdUntilCaret();
		nodeassert.strictEqual(typing.isTyping(), false);
		nodeassert.strictEqual(typing.editsOf(part, edits), edits);
		nodeassert.strictEqual(type(typing, edits, caretAt(4), 'e', 7), null);
		nodeassert.strictEqual(typing.isHeld(), true);

		typing.keepsCaret({ inputSequence: 6, paragraph: '0', index: '0' });
		nodeassert.strictEqual(typing.isHeld(), false);
		const typed = type(typing, edits, caretAt(0), 'e', 7);
		nodeassert.strictEqual(textOf(typing.editsOf(part, edits)), 'eabc');
		nodeassert.deepStrictEqual(typed?.logical, { paragraph: 0, index: 1 });
	});

	it('places the caret of the typing again where the cell stands now', function () {
		const typing = newTyping();
		const edits = [cellEdit('abc')];
		type(typing, edits, caretAt(3), 'd', 5);

		// The cell moved 300 twips right, as after a zoom or a wider column before it.
		const moved: cool.CalcCellOrigin = (column: number, row: number) =>
			column === 2 && row === 4 ? new cool.Point(1300, 2000) : undefined;
		const typed = typing.caretAfterTyping(viewId, moved);
		nodeassert.strictEqual(typed?.geometry.x, 1750);
		nodeassert.strictEqual(typed?.geometry.top, 1960);
		nodeassert.deepStrictEqual(typed?.logical, { paragraph: 0, index: 4 });

		typing.forget();
		nodeassert.strictEqual(typing.caretAfterTyping(viewId, moved), null);
	});

	it('names the inputs after which the edit stands somewhere else', function () {
		const moves = [
			'key type=input char=13 key=1280 sequence=6',
			'key type=input char=0 key=5378 sequence=6',
			'key type=input char=27 key=1281 sequence=6',
			'windowkey id=4 type=input char=0 key=1026 sequence=6',
			'mouse type=buttondown x=100 y=200 count=1 buttons=1 modifier=0 sequence=6',
		];
		for (const message of moves)
			nodeassert.strictEqual(cool.calcInputMovesEdit(message), true, message);

		const stays = [
			'textinput id=0 text=a sequence=6',
			'removetextcontext id=0 before=1 after=0 sequence=6',
			'key type=up char=13 key=1280 sequence=6',
			'key type=input char=97 key=512 sequence=6',
			'mouse type=buttonup x=100 y=200 count=1 buttons=1 modifier=0 sequence=6',
			'mouse type=move x=100 y=200 count=1 buttons=0 modifier=0',
		];
		for (const message of stays)
			nodeassert.strictEqual(cool.calcInputMovesEdit(message), false, message);
	});
});
