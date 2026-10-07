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
	one of them in flight at a time. A held arrow key lays a step on the objects here some thirty
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

	A change goes out only once the one before it was answered. What comes meanwhile waits and is
	merged with what is already waiting where it can be: changes of objects keep the newest state of
	each thing; undos merge into one undo of that many steps, and redos likewise. Nothing overtakes
	what was asked for before it, so an undo never goes out ahead of a change that came first, and
	an undo asked for while a key is still down sends what that key did first.

	The answer is the engine's next delta. A change that brings none - an undo of something that is
	not drawn - frees the slot after a short wait, so the outbox never stalls.
*/
type OutboxEntry =
	| { kind: 'transform'; what: string; mappings: Map<string, string> }
	| { kind: 'undo' | 'redo'; count: number }
	| { kind: 'other'; channel: string; message: string };

class ChangeOutbox {
	/// How long a change may go unanswered before the next one goes out anyway, in milliseconds.
	private static readonly longestWait: number = 400;

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
	private static inFlight: boolean = false;
	private static timeout: ReturnType<typeof setTimeout> | null = null;
	private static listening: boolean = false;

	/// True while the outbox itself sends, so the socket lets the message through.
	public static sending: boolean = false;

	/// True while a key is being handled, so the changes it makes are held until it is let go.
	private static madeByAKey: boolean = false;

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
	/// the newest mapping of each object, the newest state of anything else.
	private static hold(entry: OutboxEntry): void {
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

		for (const entry of ChangeOutbox.held.values()) ChangeOutbox.merge(entry);
		ChangeOutbox.held.clear();
		ChangeOutbox.sendTheNext();
	}

	/// Adds the entry to what is waiting, merged into the last waiting entry where they go
	/// together: changes of objects keep the newest state of each thing, and undos or redos add
	/// up. An entry of another kind waits behind.
	private static merge(entry: OutboxEntry): void {
		const last = ChangeOutbox.waiting[ChangeOutbox.waiting.length - 1];

		if (
			last &&
			last.kind === 'transform' &&
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

	/// Sends the first waiting entry, if nothing is in flight.
	private static sendTheNext(): void {
		if (ChangeOutbox.inFlight) return;

		const entry = ChangeOutbox.waiting.shift();
		if (!entry) return;

		ChangeOutbox.inFlight = true;
		ChangeOutbox.timeout = setTimeout(
			() => ChangeOutbox.answered(),
			ChangeOutbox.longestWait,
		);

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

	/// The change in flight was answered, or waited for long enough: the next one may go.
	private static answered(): void {
		if (ChangeOutbox.timeout) clearTimeout(ChangeOutbox.timeout);
		ChangeOutbox.timeout = null;
		ChangeOutbox.inFlight = false;
		ChangeOutbox.sendTheNext();
	}

	/*
		Listens for the engine's deltas, which answer what is in flight - a change laid on here
		is announced with an empty list and answers nothing - and for the keys being let go and
		the window losing the keyboard, which send what keys have made.
	*/
	private static listen(): void {
		if (ChangeOutbox.listening) return;
		ChangeOutbox.listening = true;

		RenderManager.onVectorChanged((answered?: number[]) => {
			if (!ChangeOutbox.inFlight) return;
			if (answered !== undefined && !answered.length) return;
			ChangeOutbox.answered();
		});

		document.addEventListener(
			'keyup',
			(event: KeyboardEvent) => {
				ChangeOutbox.keysDown.delete(event.code || event.key);
				if (!ChangeOutbox.keysDown.size) ChangeOutbox.letGoOfWhatIsHeld();
			},
			true,
		);
		window.addEventListener('blur', () => ChangeOutbox.letGoOfWhatIsHeld());
	}
}
