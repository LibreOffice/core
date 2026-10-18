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

describe('ChangeOutbox typed text', function () {
	let sent: string[];
	let savedSend: any;
	let savedInstance: any;
	let manager: VectorManager;

	// Empties the outbox and has it listen to a fresh manager.
	beforeEach(function () {
		sent = [];
		savedSend = (app as any).socket.sendMessage;
		(app as any).socket.sendMessage = (message: string) => sent.push(message);
		savedInstance = (RenderManager as any)._instance;
		manager = new VectorManager();
		(RenderManager as any)._instance = manager;
		emptyTheOutbox();
		(ChangeOutbox as any).held.clear();
		(ChangeOutbox as any).keysDown.clear();
		(ChangeOutbox as any).listening = false;
	});
	afterEach(function () {
		(app as any).socket.sendMessage = savedSend;
		(RenderManager as any)._instance = savedInstance;
		emptyTheOutbox();
	});

	function emptyTheOutbox(): void {
		const outbox = ChangeOutbox as any;
		for (const wait of outbox.inFlight) clearTimeout(wait);
		outbox.inFlight = [];
		outbox.waiting = [];
	}

	// The engine's delta that answers the oldest message in flight.
	function answer(): void {
		(manager as any)._fireChanged([5]);
	}

	// Sends messages until one slot in flight is left, so the next message is the last to go
	// out before an answer. Returns what went out.
	function fillTheOutbox(
		send: (message: string) => void = (message) => ChangeOutbox.take(message),
	): string[] {
		const filled: string[] = [];
		const free = (ChangeOutbox as any).mostInFlight - 1;
		for (let i = 0; i < free; i++) {
			const message = 'textinput id=0 text=w sequence=' + (i + 1);
			send(message);
			filled.push(message);
		}
		nodeassert.deepStrictEqual(sent, filled);
		return filled;
	}

	it('sends the first typed character at once', function () {
		nodeassert.ok(ChangeOutbox.take('textinput id=0 text=a sequence=11'));
		nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
	});

	it('merges the characters typed while the engine is behind into one message', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('textinput id=0 text=b sequence=12');
		ChangeOutbox.take('textinput id=0 text=c sequence=13');
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
		]);
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=bc sequence=13',
		]);
	});

	it('adds up the characters deleted while the engine is behind', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('removetextcontext id=0 before=1 after=0 sequence=12');
		ChangeOutbox.take('removetextcontext id=0 before=1 after=0 sequence=13');
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'removetextcontext id=0 before=2 after=0 sequence=13',
		]);
	});

	it('keeps a backspace after a forward delete apart from it', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('removetextcontext id=0 before=0 after=1 sequence=12');
		ChangeOutbox.take('removetextcontext id=0 before=1 after=0 sequence=13');
		ChangeOutbox.take('removetextcontext id=0 before=0 after=1 sequence=14');
		answer();
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'removetextcontext id=0 before=0 after=1 sequence=12',
			'removetextcontext id=0 before=1 after=1 sequence=14',
		]);
	});

	it('keeps typed text and deletions in the order they were made', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('textinput id=0 text=b sequence=12');
		ChangeOutbox.take('removetextcontext id=0 before=1 after=0 sequence=13');
		ChangeOutbox.take('textinput id=0 text=c sequence=14');
		answer();
		answer();
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=b sequence=12',
			'removetextcontext id=0 before=1 after=0 sequence=13',
			'textinput id=0 text=c sequence=14',
		]);
	});

	it('sends the waiting text ahead of a key that follows it', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('textinput id=0 text=b sequence=12');
		ChangeOutbox.sendAhead('key type=input char=13 key=1280 sequence=13');
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=b sequence=12',
		]);
		// The text sent ahead is in flight too, so the next character waits for a slot.
		ChangeOutbox.take('textinput id=0 text=c sequence=14');
		nodeassert.strictEqual(sent.length, filled.length + 2);
		answer();
		answer();
		nodeassert.strictEqual(
			sent[filled.length + 2],
			'textinput id=0 text=c sequence=14',
		);
	});

	it('lets a tile request go out on its own while text waits', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take('textinput id=0 text=a sequence=11');
		ChangeOutbox.take('textinput id=0 text=b sequence=12');
		ChangeOutbox.sendAhead('tilecombine nviewid=0 part=0');
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
		]);
	});

	describe('with a caret moved here', function () {
		let savedNextInputSequence: any;
		let savedTimerRegistry: any;
		let savedSequence: number;
		beforeEach(function () {
			// Messages are routed as the socket routes them.
			(app as any).socket.sendMessage = (message: string) => {
				LocalCaret.tellBefore(message);
				if (ChangeOutbox.take(message)) return;
				ChangeOutbox.sendAhead(message);
				sent.push(message);
			};
			savedSequence = app.file.inputSequence;
			app.file.inputSequence = 12;
			savedNextInputSequence = (app as any).nextInputSequence;
			(app as any).nextInputSequence = () => ++app.file.inputSequence;
			savedTimerRegistry = (app as any).timerRegistry;
			(app as any).timerRegistry = new TimerRegistry();
		});
		afterEach(function () {
			LocalCaret.forget();
			app.file.inputSequence = savedSequence;
			(app as any).nextInputSequence = savedNextInputSequence;
			(app as any).timerRegistry = savedTimerRegistry;
		});

		function moveCaret(): void {
			LocalCaret.moved({
				partId: 'S0',
				entryId: 5,
				objectId: 5,
				logical: { paragraph: 0, index: 0 },
				anchor: null,
			});
		}

		it('sends the text typed before the caret moved ahead of the caret', function () {
			const filled = fillTheOutbox(app.socket.sendMessage);
			app.socket.sendMessage('textinput id=0 text=a sequence=11');
			app.socket.sendMessage('textinput id=0 text=b sequence=12');
			moveCaret();
			answer();
			nodeassert.deepStrictEqual(sent, [
				...filled,
				'textinput id=0 text=a sequence=11',
				'textinput id=0 text=b sequence=12',
			]);
			LocalCaret.tell();
			nodeassert.strictEqual(
				sent[filled.length + 2],
				'textcursor object=5 paragraph=0 index=0 sequence=13',
			);
		});

		it('sends the caret between the text typed before it moved and the text typed after', function () {
			const filled = fillTheOutbox(app.socket.sendMessage);
			app.socket.sendMessage('textinput id=0 text=a sequence=11');
			app.socket.sendMessage('textinput id=0 text=b sequence=12');
			moveCaret();
			app.socket.sendMessage('textinput id=0 text=c sequence=13');
			answer();
			answer();
			nodeassert.deepStrictEqual(sent, [
				...filled,
				'textinput id=0 text=a sequence=11',
				'textinput id=0 text=b sequence=12',
				'textcursor object=5 paragraph=0 index=0 sequence=13',
				'textinput id=0 text=c sequence=13',
			]);
		});
	});

	it('leaves text typed into a dialog alone', function () {
		nodeassert.strictEqual(
			ChangeOutbox.take('textinput id=7 text=a sequence=11'),
			false,
		);
		nodeassert.deepStrictEqual(sent, []);
	});
});
