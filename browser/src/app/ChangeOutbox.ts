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

/*
	The changes a view that draws from the objects sends to the engine, held to the engine's pace:
	a few of them in flight at a time. A held arrow key lays a step on the objects here some thirty
	times a second, and Ctrl+Z held down asks for as many undos; the engine applies each, records
	it for undo, compares the slide and sends a delta, and cannot keep up, so the queue towards it
	grows and the answers come back long after the steps they belong to.

	Every change of an object names the whole state it is to reach - the object's mapping, the place
	a handle goes to, the places of the points of a path, a glue point, a crop window - and the
	engine works it out from its own state. So of a run of changes to the same thing only the last
	counts. A change made by a key is held here until the keys are let go, and then only the last
	state of each thing goes out: a single press a moment after it was made, a key held down as one
	change and so as one undo step. The steps are laid on here at once all the same, so the view
	does not wait for any of it. If the window loses the keyboard while a key is down, what is held
	goes out then.

	A change goes out only while fewer than a few are unanswered. What comes meanwhile waits and is
	merged with what is already waiting where it can be: changes of objects keep the newest state of
	each thing; undos merge into one undo of that many steps, and redos likewise. What one key press
	made stays a change of its own, so each press stays one step of the undo. Nothing overtakes
	what was asked for before it, so an undo never goes out ahead of a change that came first, and
	an undo asked for while a key is still down sends what that key did first.

	The answer to the oldest change in flight is the engine's next delta. One that brings none
	stops counting as in flight after a short wait, so the outbox never stalls; an undo or a redo
	does so too, so a run of them keeps going out at that pace however long the connection takes.
	Apart from that, the undos and redos that went out are counted until the engine reports the
	result of each, and the delta the last of them brings has arrived: until then the objects stand
	where only the engine knows, and the deltas that arrive before the results belong to changes
	sent earlier.
*/
/*
	One change waiting to go out. A change of objects or of another thing is sealed where it is
	what one key press made, which is one step of the undo: nothing after it is merged into it.
*/
type OutboxEntry =
	| {
			kind: 'transform';
			what: string;
			mappings: Map<string, string>;
			sealed?: boolean;
	  }
	| { kind: 'undo' | 'redo'; count: number }
	| { kind: 'other'; channel: string; message: string; sealed?: boolean };

class ChangeOutbox {
	/// How long a change may go unanswered before the next one goes out anyway, in milliseconds.
	private static readonly longestWait: number = 400;

	/// How long after the last undo or redo went out its results are taken as all in, where
	/// some never arrive, in milliseconds. The engine reports a result for each of them.
	private static readonly longestUndoWait: number = 10000;

	/// How long the delta of an undo or a redo is waited for once its result is in, in
	/// milliseconds. The engine sends the two close together, in either order.
	private static readonly deltaAfterUndo: number = 250;

	/// The commands besides the mapping of objects that change one thing to a state they name,
	/// each with where the part that names the thing ends and the state begins.
	private static readonly namedStates: { [command: string]: string } = {
		setobjecthandle: ' x=',
		setobjectcontrolpoint: ' x=',
		setobjectgluepoint: ' x=',
		cropobject: ' x1=',
		setobjectpoints: ' changes=',
	};

	private static waiting: OutboxEntry[] = [];
	private static held: Map<string, OutboxEntry> = new Map();
	private static keysDown: Set<string> = new Set();
	/// The most changes in flight at a time. More than one keeps a run of key presses going out
	/// at the pace they are made over a slow connection, few enough for the engine to keep up.
	private static readonly mostInFlight: number = 4;

	/// One wait for each change in flight, the oldest first.
	private static inFlight: ReturnType<typeof setTimeout>[] = [];

	/// How many undos and redos went out whose result the engine has not reported yet.
	private static undosUnanswered: number = 0;

	/// True from the result of the last undo or redo that went out until the delta it brings.
	private static undoSettling: boolean = false;

	/// Which of the two went out last.
	private static undoKind: 'undo' | 'redo' = 'undo';

	/// The wait for the delta after the results, or for results that never arrive.
	private static undoTimeout: ReturnType<typeof setTimeout> | null = null;
	private static listening: boolean = false;

	/// True while the outbox itself sends, so the socket lets the message through.
	public static sending: boolean = false;

	/// True while a key is being handled, so the changes it makes are held until it is let go.
	private static madeByAKey: boolean = false;

	/// The code of the key whose press repeats while it is held down, with Ctrl or Cmd: Z or Y.
	/// Empty while no such press is being handled.
	private static repeatingKey: string = '';

	/*
		Runs what a key press does with the changes it makes held until the keys are let go. The
		key is remembered as down until its release is seen.
	*/
	public static byAKey<T>(event: KeyboardEvent, does: () => T): T {
		ChangeOutbox.listen();
		ChangeOutbox.keysDown.add(event.code || event.key);
		ChangeOutbox.madeByAKey = true;
		try {
			return does();
		} finally {
			ChangeOutbox.madeByAKey = false;
			// A key that made nothing to hold is not waited for.
			if (!ChangeOutbox.held.size) ChangeOutbox.keysDown.clear();
		}
	}

	/*
		Takes a message on its way to the engine, if it is one the outbox holds to the engine's
		pace, and answers whether it took it. It holds the changes of objects and the undos and
		redos, in a view that draws from the objects only.
	*/
	public static take(message: string): boolean {
		if (ChangeOutbox.sending || !RenderManager.isVectorRendering())
			return false;

		const entry = ChangeOutbox.entryOf(message);
		if (!entry) return false;

		ChangeOutbox.listen();

		if (entry.kind === 'undo' || entry.kind === 'redo') {
			if (!ChangeOutbox.undoOnItsWay())
				ChangeOutbox.undoAskedAt = performance.now();

			// The repeats of a held Ctrl+Z or Ctrl+Y are held until the key is let go, and go out
			// as one undo or redo of that many steps.
			if (ChangeOutbox.repeatingKey) {
				ChangeOutbox.keysDown.add(ChangeOutbox.repeatingKey);
				ChangeOutbox.hold(entry);
				return true;
			}

			ChangeOutbox.letGoOfWhatIsHeld();
			ChangeOutbox.merge(entry);
		} else if (ChangeOutbox.madeByAKey) {
			ChangeOutbox.hold(entry);
			return true;
		} else {
			ChangeOutbox.merge(entry);
		}

		ChangeOutbox.sendTheNext();
		return true;
	}

	/// Whether an undo or a redo waits to go out or has not been answered yet.
	public static undoOnItsWay(): boolean {
		return ChangeOutbox.waitsFor() !== null;
	}

	/// When the undos and redos on their way began to be asked for, in the time of
	/// performance.now(). Only meaningful while one is on its way.
	private static undoAskedAt: number = 0;

	/// How long the undos and redos on their way have been waited for, in milliseconds; zero
	/// while none is on its way.
	public static undoWaitedFor(): number {
		return ChangeOutbox.undoOnItsWay()
			? performance.now() - ChangeOutbox.undoAskedAt
			: 0;
	}

	/// The undo or the redo that goes out first among those waiting or in flight, or null where
	/// there is none.
	public static waitsFor(): 'undo' | 'redo' | null {
		if (ChangeOutbox.undosUnanswered > 0 || ChangeOutbox.undoSettling)
			return ChangeOutbox.undoKind;

		for (const entry of ChangeOutbox.waiting)
			if (entry.kind === 'undo' || entry.kind === 'redo') return entry.kind;
		for (const entry of ChangeOutbox.held.values())
			if (entry.kind === 'undo' || entry.kind === 'redo') return entry.kind;
		return null;
	}

	/// The entry a message stands for, or nothing for a message the outbox leaves alone.
	private static entryOf(message: string): OutboxEntry | null {
		if (message === 'uno .uno:Undo') return { kind: 'undo', count: 1 };
		if (message === 'uno .uno:Redo') return { kind: 'redo', count: 1 };

		const transform = /^setobjecttransform to=(\S*) as=(\S+)$/.exec(message);
		if (transform) {
			const mappings = new Map<string, string>();
			for (const one of transform[1].split(';')) {
				if (!one) continue;
				mappings.set(one.split(',')[0], one);
			}
			return { kind: 'transform', what: transform[2], mappings };
		}

		const command = message.split(' ')[0];
		const stateBegins = ChangeOutbox.namedStates[command];
		if (stateBegins === undefined) return null;

		const at = message.indexOf(stateBegins);
		return {
			kind: 'other',
			channel: at < 0 ? message : message.substring(0, at),
			message,
		};
	}

	/// Keeps the entry until the keys are let go, in place of what was held for the same thing:
	/// the newest mapping of each object, the newest state of anything else, the undos or the
	/// redos added up.
	private static hold(entry: OutboxEntry): void {
		if (entry.kind === 'undo' || entry.kind === 'redo') {
			const before = ChangeOutbox.held.get(entry.kind);
			if (before && before.kind === entry.kind) before.count += entry.count;
			else ChangeOutbox.held.set(entry.kind, entry);
			return;
		}

		if (entry.kind === 'transform') {
			const before = ChangeOutbox.held.get('transform');
			if (before && before.kind === 'transform') {
				for (const [objectId, mapping] of entry.mappings)
					before.mappings.set(objectId, mapping);
				before.what = entry.what;
			} else ChangeOutbox.held.set('transform', entry);
			return;
		}

		if (entry.kind === 'other') ChangeOutbox.held.set(entry.channel, entry);
	}

	/// Sends what keys have made, once they are let go or the window loses the keyboard.
	private static letGoOfWhatIsHeld(): void {
		ChangeOutbox.keysDown.clear();
		if (!ChangeOutbox.held.size) return;

		// What the keys made is one step of the undo of its own, so it waits sealed.
		for (const entry of ChangeOutbox.held.values()) {
			ChangeOutbox.merge(entry);
			const last = ChangeOutbox.waiting[ChangeOutbox.waiting.length - 1];
			if (last && (last.kind === 'transform' || last.kind === 'other'))
				last.sealed = true;
		}
		ChangeOutbox.held.clear();
		ChangeOutbox.sendTheNext();
	}

	/// Adds the entry to what is waiting, merged into the last waiting entry where they go
	/// together and that one is not sealed: changes of objects keep the newest state of each
	/// thing, and undos or redos add up. An entry of another kind waits behind.
	private static merge(entry: OutboxEntry): void {
		const last = ChangeOutbox.waiting[ChangeOutbox.waiting.length - 1];

		if (
			last &&
			last.kind === 'transform' &&
			!last.sealed &&
			entry.kind === 'transform' &&
			last.what === entry.what
		) {
			for (const [objectId, mapping] of entry.mappings)
				last.mappings.set(objectId, mapping);
			return;
		}

		if (
			last &&
			last.kind === 'other' &&
			!last.sealed &&
			entry.kind === 'other' &&
			last.channel === entry.channel
		) {
			last.message = entry.message;
			return;
		}

		if (
			last &&
			(last.kind === 'undo' || last.kind === 'redo') &&
			last.kind === entry.kind
		) {
			last.count += (entry as { count: number }).count;
			return;
		}

		ChangeOutbox.waiting.push(entry);
	}

	/// Sends the waiting entries in their order while fewer than mostInFlight are in flight.
	private static sendTheNext(): void {
		while (
			ChangeOutbox.inFlight.length < ChangeOutbox.mostInFlight &&
			ChangeOutbox.waiting.length
		)
			ChangeOutbox.send(ChangeOutbox.waiting.shift() as OutboxEntry);
	}

	/// Sends the entry, which is in flight from now on.
	private static send(entry: OutboxEntry): void {
		const wait: ReturnType<typeof setTimeout> = setTimeout(
			() => ChangeOutbox.answered(wait),
			ChangeOutbox.longestWait,
		);
		ChangeOutbox.inFlight.push(wait);

		if (entry.kind === 'undo' || entry.kind === 'redo') {
			ChangeOutbox.undosUnanswered++;
			ChangeOutbox.undoSettling = false;
			ChangeOutbox.undoKind = entry.kind;
			ChangeOutbox.awaitTheUndos(ChangeOutbox.longestUndoWait);
		}

		ChangeOutbox.sending = true;
		try {
			app.socket.sendMessage(ChangeOutbox.messageOf(entry));
		} finally {
			ChangeOutbox.sending = false;
		}
	}

	/// The message an entry goes out as. An undo or a redo of more than one step names the
	/// number, which the engine's undo takes as an argument.
	private static messageOf(entry: OutboxEntry): string {
		if (entry.kind === 'other') return entry.message;

		if (entry.kind === 'transform')
			return (
				'setobjecttransform to=' +
				Array.from(entry.mappings.values()).join(';') +
				' as=' +
				entry.what
			);

		const command = entry.kind === 'undo' ? 'Undo' : 'Redo';
		if (entry.count === 1) return 'uno .uno:' + command;
		return (
			'uno .uno:' +
			command +
			' ' +
			JSON.stringify({
				[command]: { type: 'unsigned short', value: String(entry.count) },
			})
		);
	}

	/// A change in flight was answered, the oldest one, or the one of that wait was waited for
	/// long enough: the next one may go.
	private static answered(wait?: ReturnType<typeof setTimeout>): void {
		const at = wait === undefined ? 0 : ChangeOutbox.inFlight.indexOf(wait);
		if (at < 0 || at >= ChangeOutbox.inFlight.length) return;

		clearTimeout(ChangeOutbox.inFlight[at]);
		ChangeOutbox.inFlight.splice(at, 1);
		ChangeOutbox.sendTheNext();
	}

	/// Takes the undos and redos that went out as all in after that many milliseconds.
	private static awaitTheUndos(milliseconds: number): void {
		if (ChangeOutbox.undoTimeout) clearTimeout(ChangeOutbox.undoTimeout);
		ChangeOutbox.undoTimeout = setTimeout(
			() => ChangeOutbox.undosAreIn(),
			milliseconds,
		);
	}

	/*
		The undos and redos that went out are all in, or were waited for long enough. What waits
		for them runs, unless more of them wait to go out still.
	*/
	private static undosAreIn(): void {
		if (ChangeOutbox.undoTimeout) clearTimeout(ChangeOutbox.undoTimeout);
		ChangeOutbox.undoTimeout = null;
		ChangeOutbox.undosUnanswered = 0;
		ChangeOutbox.undoSettling = false;

		if (ChangeOutbox.undoOnItsWay() || !ChangeOutbox.afterTheUndos.length)
			return;
		const waiting = ChangeOutbox.afterTheUndos;
		ChangeOutbox.afterTheUndos = [];
		for (const callback of waiting) callback();
	}

	/// What is to run once no undo or redo is on its way any more, in the order it was given.
	private static afterTheUndos: (() => void)[] = [];

	/// Runs the callback once no undo or redo is on its way any more; at once where none is.
	public static whenTheUndosAreIn(callback: () => void): void {
		if (!ChangeOutbox.undoOnItsWay()) {
			callback();
			return;
		}
		ChangeOutbox.afterTheUndos.push(callback);
	}

	/// The key with that code was let go: once no key is down, what keys made goes out.
	public static keyIsUp(code: string): void {
		if (code === ChangeOutbox.repeatingKey) ChangeOutbox.repeatingKey = '';
		ChangeOutbox.keysDown.delete(code);
		if (!ChangeOutbox.keysDown.size) ChangeOutbox.letGoOfWhatIsHeld();
	}

	/// The engine reported the result of an undo or a redo that went out. After the last of them
	/// the delta it brings is waited for a short while more.
	private static undoReported(): void {
		if (ChangeOutbox.undosUnanswered <= 0) return;

		if (--ChangeOutbox.undosUnanswered > 0) return;

		ChangeOutbox.undoSettling = true;
		ChangeOutbox.awaitTheUndos(ChangeOutbox.deltaAfterUndo);
	}

	/*
		Listens for the engine's deltas, which answer what is in flight and, after the results of
		the undos and redos that went out, the last of them - a change laid on here is announced
		with an empty list and answers nothing - for the results of undos and redos, and for the
		keys being let go and the window losing the keyboard, which send what keys have made.
	*/
	private static listen(): void {
		if (ChangeOutbox.listening) return;
		ChangeOutbox.listening = true;

		// A press is seen here before it is handled, so a repeat of Ctrl+Z or Ctrl+Y is known as
		// one when its undo or redo arrives.
		document.addEventListener(
			'keydown',
			(event: KeyboardEvent) => {
				const undoKey =
					(event.ctrlKey || event.metaKey) &&
					(event.code === 'KeyZ' || event.code === 'KeyY');
				ChangeOutbox.repeatingKey = undoKey && event.repeat ? event.code : '';
			},
			true,
		);

		RenderManager.onVectorChanged((answered?: number[]) => {
			if (answered !== undefined && !answered.length) return;
			if (ChangeOutbox.undoSettling) ChangeOutbox.undosAreIn();
			if (ChangeOutbox.inFlight.length) ChangeOutbox.answered();
		});

		app.map.on('commandresult', (event: any) => {
			if (
				event.commandName === '.uno:Undo' ||
				event.commandName === '.uno:Redo'
			)
				ChangeOutbox.undoReported();
		});

		document.addEventListener(
			'keyup',
			(event: KeyboardEvent) => ChangeOutbox.keyIsUp(event.code || event.key),
			true,
		);
		window.addEventListener('blur', () => ChangeOutbox.letGoOfWhatIsHeld());
	}
}
