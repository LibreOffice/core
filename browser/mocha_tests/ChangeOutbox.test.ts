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
	let savedOuterSequence: number;
	let manager: VectorManager;

	// Empties the outbox and has it listen to a fresh manager.
	beforeEach(function () {
		sent = [];
		savedSend = (app as any).socket.sendMessage;
		(app as any).socket.sendMessage = (message: string) => sent.push(message);
		savedInstance = (RenderManager as any)._instance;
		manager = new VectorManager();
		(RenderManager as any)._instance = manager;
		savedOuterSequence = app.file.inputSequence;
		app.file.inputSequence = 10;
		cool.InputSequence.forget();
		emptyTheOutbox();
		(ChangeOutbox as any).held.clear();
		(ChangeOutbox as any).keysDown.clear();
		(ChangeOutbox as any).listening = false;
	});
	afterEach(function () {
		(app as any).socket.sendMessage = savedSend;
		(RenderManager as any)._instance = savedInstance;
		app.file.inputSequence = savedOuterSequence;
		cool.InputSequence.forget();
		emptyTheOutbox();
		ChangeOutbox.setMostInFlight(null);
		ChangeOutbox.setTextWait(0);
	});

	function emptyTheOutbox(): void {
		const outbox = ChangeOutbox as any;
		for (const one of outbox.inFlight) clearTimeout(one.wait);
		outbox.inFlight = [];
		outbox.waiting = [];
	}

	// The message of that text typed, numbered as the next input.
	function typed(text: string): string {
		return cool.InputSequence.stamp('textinput id=0 text=' + text);
	}

	// The message of that many characters deleted, numbered as the next input.
	function deleted(before: number, after: number): string {
		return cool.InputSequence.stamp(
			'removetextcontext id=0 before=' + before + ' after=' + after,
		);
	}

	// The engine's delta that answers the oldest message in flight.
	function answer(): void {
		(manager as any)._fireChanged([5]);
	}

	// Sends a change of objects through a socket that routes the message as the real one does,
	// and returns the ticket the outbox took it with.
	function aChange(message: string): cool.InputTicket | null {
		const saved = (app as any).socket.sendMessage;
		(app as any).socket.sendMessage = (one: string) => {
			if (!ChangeOutbox.take(one)) sent.push(one);
		};
		try {
			return ChangeOutbox.sendAChange(message);
		} finally {
			(app as any).socket.sendMessage = saved;
		}
	}

	// Sends messages until one slot in flight is left, so the next message is the last to go
	// out before an answer. They are numbered below 10, so what the test sends starts at 11.
	// Returns what went out.
	function fillTheOutbox(
		send: (message: string) => void = (message) => ChangeOutbox.take(message),
	): string[] {
		const filled: string[] = [];
		const free = (ChangeOutbox as any).mostInFlight - 1;
		app.file.inputSequence = 10 - free;
		for (let i = 0; i < free; i++) {
			const message = typed('w');
			send(message);
			filled.push(message);
		}
		nodeassert.deepStrictEqual(sent, filled);
		nodeassert.strictEqual(app.file.inputSequence, 10);
		return filled;
	}

	it('sends the first typed character at once', function () {
		nodeassert.ok(ChangeOutbox.take(typed('a')));
		nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
	});

	it('merges the characters typed while the engine is behind into one message', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		ChangeOutbox.take(typed('c'));
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
		]);
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=bc sequence=12',
		]);
	});

	it('numbers the merged text when it goes out, so the numbers follow the order sent', function () {
		ChangeOutbox.setMostInFlight(1);
		const ticket = cool.InputSequence.ticketForTheNextMessage();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		// The text waits, so its number went back and its ticket waits with it.
		nodeassert.strictEqual(app.file.inputSequence, 11);
		nodeassert.strictEqual(ticket.sentAs(), 11);

		const later = cool.InputSequence.ticketForTheNextMessage();
		ChangeOutbox.take(typed('c'));
		nodeassert.strictEqual(later.sentAs(), undefined);
		answer();
		nodeassert.deepStrictEqual(sent, [
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=bc sequence=12',
		]);
		nodeassert.strictEqual(later.sentAs(), 12);
	});

	it('adds up the characters deleted while the engine is behind', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(deleted(1, 0));
		ChangeOutbox.take(deleted(1, 0));
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'removetextcontext id=0 before=2 after=0 sequence=12',
		]);
	});

	it('keeps a backspace after a forward delete apart from it', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(deleted(0, 1));
		ChangeOutbox.take(deleted(1, 0));
		ChangeOutbox.take(deleted(0, 1));
		answer();
		answer();
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'removetextcontext id=0 before=0 after=1 sequence=12',
			'removetextcontext id=0 before=1 after=1 sequence=13',
		]);
	});

	it('keeps typed text and deletions in the order they were made', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		ChangeOutbox.take(deleted(1, 0));
		ChangeOutbox.take(typed('c'));
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

	it('lets one delta answer all the text it reflects', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		ChangeOutbox.take(typed('c'));
		// The kit applied every message in flight together, and the delta says so.
		(manager as any)._fireChanged([5], 11);
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=bc sequence=12',
		]);
		// Only "bc" is in flight now, so the next character goes out at once.
		ChangeOutbox.take(typed('d'));
		nodeassert.strictEqual(
			sent[filled.length + 2],
			'textinput id=0 text=d sequence=13',
		);
	});

	it('lets one delta answer a change sent before the text it reflects', function () {
		ChangeOutbox.setMostInFlight(2);
		ChangeOutbox.take('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		nodeassert.deepStrictEqual(sent, [
			'setobjecttransform to=5,1,0,0,1,10,20 as=move sequence=11',
			'textinput id=0 text=a sequence=12',
		]);
		// The kit applied the change and the text in order, and the delta reflects the text.
		(manager as any)._fireChanged([5], 12);
		ChangeOutbox.take(typed('c'));
		nodeassert.deepStrictEqual(sent.slice(2), [
			'textinput id=0 text=b sequence=13',
			'textinput id=0 text=c sequence=14',
		]);
	});

	it('numbers a change that waits behind text above the text', function () {
		ChangeOutbox.setMostInFlight(1);
		aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		const change = aChange('setobjecttransform to=5,1,0,0,1,30,20 as=move');
		const edit = cool.InputSequence.ticketForTheNextMessage();
		ChangeOutbox.take(typed('a'));
		nodeassert.deepStrictEqual(sent, [
			'setobjecttransform to=5,1,0,0,1,10,20 as=move sequence=11',
		]);

		answer();
		answer();
		nodeassert.deepStrictEqual(sent.slice(1), [
			'setobjecttransform to=5,1,0,0,1,30,20 as=move sequence=12',
			'textinput id=0 text=a sequence=13',
		]);
		nodeassert.strictEqual(change?.sentAs(), 12);
		nodeassert.strictEqual(edit.sentAs(), 13);
	});

	it('gives the changes merged into one message the number it goes out with', function () {
		ChangeOutbox.setMostInFlight(1);
		const first = aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		const second = aChange('setobjecttransform to=5,1,0,0,1,30,20 as=move');
		const third = aChange('setobjecttransform to=5,1,0,0,1,50,20 as=move');
		nodeassert.strictEqual(first?.sentAs(), 11);
		nodeassert.strictEqual(second?.sentAs(), undefined);

		answer();
		nodeassert.deepStrictEqual(sent, [
			'setobjecttransform to=5,1,0,0,1,10,20 as=move sequence=11',
			'setobjecttransform to=5,1,0,0,1,50,20 as=move sequence=12',
		]);
		nodeassert.strictEqual(second?.sentAs(), 12);
		nodeassert.strictEqual(third?.sentAs(), 12);
	});

	it('has no ticket for a message it did not take', function () {
		nodeassert.ok(aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move'));
		nodeassert.strictEqual(aChange('uno .uno:Bold'), null);
	});

	it('has no ticket for a change the socket let go unsent', function () {
		nodeassert.ok(aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move'));
		(app as any).socket.sendMessage = () => {};
		nodeassert.strictEqual(
			ChangeOutbox.sendAChange('setobjecttransform to=5,1,0,0,1,30,20 as=move'),
			null,
		);
	});

	it('keeps the ticket of a change while sending what it set going', function () {
		ChangeOutbox.setMostInFlight(1);
		aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		const ticket = aChange('setobjecttransform to=5,1,0,0,1,30,20 as=move');
		// Taking the change answers nothing, so it goes out as the one in flight is answered.
		// The outbox's own send passes through take again, and the ticket gets its number.
		(app as any).socket.sendMessage = (message: string) => {
			ChangeOutbox.take(message);
			sent.push(message);
		};
		answer();
		nodeassert.strictEqual(ticket?.sentAs(), 12);
		nodeassert.strictEqual(sent.length, 2);
	});

	it('lets one delta answer every change of objects it reflects', function () {
		ChangeOutbox.setMostInFlight(2);
		ChangeOutbox.take('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		ChangeOutbox.take('setobjecttransform to=6,1,0,0,1,10,20 as=move');
		ChangeOutbox.take('setobjecttransform to=7,1,0,0,1,10,20 as=move');
		nodeassert.strictEqual(sent.length, 2);
		// The kit applied both changes in flight before its callbacks went out, so the one
		// delta answers both and the third change goes out alone.
		(manager as any)._fireChanged([5, 6], 12);
		nodeassert.deepStrictEqual(sent.slice(2), [
			'setobjecttransform to=7,1,0,0,1,10,20 as=move sequence=13',
		]);
		nodeassert.strictEqual((ChangeOutbox as any).inFlight.length, 1);
	});

	it('answers no change in flight with a delta older than all of them', function () {
		ChangeOutbox.setMostInFlight(1);
		ChangeOutbox.take('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		ChangeOutbox.take('setobjecttransform to=6,1,0,0,1,10,20 as=move');
		// The delta reflects an input sent before the change in flight, so the change still
		// waits for its own answer and the next one stays back.
		(manager as any)._fireChanged([5], 10);
		nodeassert.deepStrictEqual(sent, [
			'setobjecttransform to=5,1,0,0,1,10,20 as=move sequence=11',
		]);
		(manager as any)._fireChanged([5], 11);
		nodeassert.deepStrictEqual(sent.slice(1), [
			'setobjecttransform to=6,1,0,0,1,10,20 as=move sequence=12',
		]);
	});

	it('keeps a change it sends later apart from the edit predicted meanwhile', function () {
		ChangeOutbox.setMostInFlight(1);
		aChange('setobjecttransform to=5,1,0,0,1,10,20 as=move');
		const change = aChange('setobjecttransform to=5,1,0,0,1,30,20 as=move');
		const edit = cool.InputSequence.ticketForTheNextMessage();
		answer();
		nodeassert.strictEqual(change?.sentAs(), 12);
		// The edit goes with its own message, which waits behind the change in flight.
		nodeassert.strictEqual(edit.sentAs(), undefined);
		ChangeOutbox.take(typed('a'));
		answer();
		nodeassert.deepStrictEqual(sent.slice(2), [
			'textinput id=0 text=a sequence=13',
		]);
		nodeassert.strictEqual(edit.sentAs(), 13);
	});

	it('sends the waiting text ahead of a key that follows it', function () {
		const filled = fillTheOutbox();
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		const key = ChangeOutbox.sendAhead(
			cool.InputSequence.stamp('key type=input char=13 key=1280'),
		);
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=b sequence=12',
		]);
		// The key is numbered above the text that went ahead of it.
		nodeassert.strictEqual(key, 'key type=input char=13 key=1280 sequence=13');
		// The text sent ahead is in flight too, so the next character waits for a slot.
		ChangeOutbox.take(typed('c'));
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
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		nodeassert.strictEqual(
			ChangeOutbox.sendAhead('tilecombine nviewid=0 part=0'),
			'tilecombine nviewid=0 part=0',
		);
		nodeassert.deepStrictEqual(sent, [
			...filled,
			'textinput id=0 text=a sequence=11',
		]);
	});

	describe('with a caret moved here', function () {
		let savedTimerRegistry: any;
		beforeEach(function () {
			// Messages are routed as the socket routes them.
			(app as any).socket.sendMessage = (message: string) => {
				message = LocalCaret.tellBefore(message);
				if (ChangeOutbox.take(message)) return;
				sent.push(ChangeOutbox.sendAhead(message));
			};
			savedTimerRegistry = (app as any).timerRegistry;
			(app as any).timerRegistry = new TimerRegistry();
		});
		afterEach(function () {
			LocalCaret.forget();
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

		it('numbers the caret below the key that it goes ahead of', function () {
			moveCaret();
			app.socket.sendMessage(
				cool.InputSequence.stamp('key type=input char=0 key=1286') + '\n',
			);
			nodeassert.deepStrictEqual(sent, [
				'textcursor object=5 paragraph=0 index=0 sequence=11',
				'key type=input char=0 key=1286 sequence=12\n',
			]);
			nodeassert.strictEqual(app.file.inputSequence, 12);
		});

		it('sends the text typed before the caret moved ahead of the caret', function () {
			const filled = fillTheOutbox(app.socket.sendMessage);
			app.socket.sendMessage(typed('a'));
			app.socket.sendMessage(typed('b'));
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
			app.socket.sendMessage(typed('a'));
			app.socket.sendMessage(typed('b'));
			moveCaret();
			app.socket.sendMessage(typed('c'));
			answer();
			answer();
			nodeassert.deepStrictEqual(sent, [
				...filled,
				'textinput id=0 text=a sequence=11',
				'textinput id=0 text=b sequence=12',
				'textcursor object=5 paragraph=0 index=0 sequence=13',
				'textinput id=0 text=c sequence=14',
			]);
		});
	});

	it('sends as many changes at a time as it is set to', function () {
		ChangeOutbox.setMostInFlight(1);
		ChangeOutbox.take(typed('a'));
		ChangeOutbox.take(typed('b'));
		ChangeOutbox.take(typed('c'));
		nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);

		// A higher number lets what waits go out at once.
		ChangeOutbox.setMostInFlight(2);
		nodeassert.deepStrictEqual(sent, [
			'textinput id=0 text=a sequence=11',
			'textinput id=0 text=bc sequence=12',
		]);

		ChangeOutbox.setMostInFlight(null);
		nodeassert.strictEqual(
			(ChangeOutbox as any).mostInFlight,
			ChangeOutbox.usualMostInFlight,
		);
	});

	describe('with a wait for typed text', function () {
		beforeEach(function () {
			installFakeClock();
			ChangeOutbox.setTextWait(100);
		});
		afterEach(function () {
			emptyTheOutbox();
			restoreClock();
		});

		it('sends what is typed during the wait as one message once the wait is over', function () {
			ChangeOutbox.take(typed('a'));
			advanceClock(50);
			ChangeOutbox.take(typed('b'));
			advanceClock(49);
			nodeassert.deepStrictEqual(sent, []);
			advanceClock(1);
			nodeassert.deepStrictEqual(sent, ['textinput id=0 text=ab sequence=11']);
		});

		it('sends a deletion after the text before it, each once its own wait is over', function () {
			ChangeOutbox.take(typed('a'));
			advanceClock(50);
			ChangeOutbox.take(deleted(1, 0));
			advanceClock(50);
			nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
			advanceClock(50);
			nodeassert.deepStrictEqual(sent, [
				'textinput id=0 text=a sequence=11',
				'removetextcontext id=0 before=1 after=0 sequence=12',
			]);
		});

		it('sends the text ahead of a key before the wait is over', function () {
			ChangeOutbox.take(typed('a'));
			const key = ChangeOutbox.sendAhead(
				cool.InputSequence.stamp('key type=input char=13 key=1280'),
			);
			nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
			nodeassert.strictEqual(
				key,
				'key type=input char=13 key=1280 sequence=12',
			);
		});

		it('waits again for text that merges into text kept back by the changes in flight', function () {
			ChangeOutbox.setMostInFlight(1);
			ChangeOutbox.take(typed('a'));
			advanceClock(100);
			ChangeOutbox.take(typed('b'));
			advanceClock(100);
			nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
			ChangeOutbox.take(typed('c'));
			answer();
			advanceClock(99);
			nodeassert.deepStrictEqual(sent, ['textinput id=0 text=a sequence=11']);
			advanceClock(1);
			nodeassert.deepStrictEqual(sent, [
				'textinput id=0 text=a sequence=11',
				'textinput id=0 text=bc sequence=12',
			]);
		});
	});

	it('leaves a text message alone when it cannot read every parameter', function () {
		for (const message of [
			'removetextcontext id=0 before=x after=0 sequence=11',
			'removetextcontext id=0 before=1 after=0',
			'textinput id=0 text=a sequence=11 more=1',
			'textinput id=0 sequence=11',
		])
			nodeassert.strictEqual(ChangeOutbox.take(message), false, message);
		nodeassert.deepStrictEqual(sent, []);
	});

	it('leaves text typed into a dialog alone', function () {
		nodeassert.strictEqual(
			ChangeOutbox.take('textinput id=7 text=a sequence=11'),
			false,
		);
		nodeassert.deepStrictEqual(sent, []);
	});
});
