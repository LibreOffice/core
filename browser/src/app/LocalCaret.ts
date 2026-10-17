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

/*
	The caret of a text edit, moved here ahead of the engine. A caret key that the cached text
	answers moves the caret here and sends nothing. The engine is told in a textcursor message,
	numbered like a keystroke, right before the next message that may read or move the caret, or
	once the caret rests. Until the engine answers it, every caret it reports is older than the
	caret here.
*/

/// A caret moved here.
interface MovedCaret {
	partId: cool.VectorPartGuid;
	/// The id of the cached entry that holds the caret.
	entryId: number;
	/// The engine's unique id of the edited object.
	objectId: number;
	logical: cool.LogicalCaret;
}

class LocalCaret {
	/// The prefixes of the messages that neither read nor move the caret.
	private static readonly unrelated: string[] = [
		'tile ',
		'tilecombine ',
		'canceltiles',
		'clientvisiblearea ',
		'clientzoom ',
		'commandvalues command=.uno:Vector',
		'useractive',
		'userinactive',
	];

	/// A mouse move with no button down, which neither reads nor moves the caret.
	private static readonly hover: RegExp = /^mouse type=move .* buttons=0 /;

	/// The newest caret moved here, while the engine has not answered for it, or null.
	private static caret: MovedCaret | null = null;

	/// The sequence number of the textcursor message that told the engine of the caret, or 0
	/// while the engine has not been told.
	private static sequence: number = 0;

	/// How long the caret rests before the engine is told, in milliseconds. A held key moves
	/// the caret many times a second, and the engine shows each caret to the toolbar and the
	/// other views.
	private static readonly restDelay: number = 300;

	/// The timer that tells the engine once the caret rests, or null while none runs.
	private static restTimer: ReturnType<typeof setTimeout> | null = null;

	public static moved(caret: MovedCaret): void {
		LocalCaret.caret = caret;
		LocalCaret.sequence = 0;
		LocalCaret.stopRestTimer();
		LocalCaret.restTimer = app.timerRegistry.setTimeout(
			'localcaretrest',
			() => {
				LocalCaret.restTimer = null;
				LocalCaret.tell();
			},
			LocalCaret.restDelay,
		);
	}

	private static stopRestTimer(): void {
		if (LocalCaret.restTimer === null) return;
		app.timerRegistry.clearTimeout(LocalCaret.restTimer);
		LocalCaret.restTimer = null;
	}

	/// The caret moved here while the engine is behind it, with the sequence number of the
	/// message that told the engine. Infinity while the engine has not been told, so the caret
	/// is newer than any input sent.
	public static ahead(): { caret: MovedCaret; sequence: number } | null {
		if (!LocalCaret.caret) return null;
		return {
			caret: LocalCaret.caret,
			sequence: LocalCaret.sequence || Infinity,
		};
	}

	/// Tells the engine where the caret is, once for each move.
	public static tell(): void {
		LocalCaret.stopRestTimer();
		const caret = LocalCaret.caret;
		if (!caret || LocalCaret.sequence) return;
		// The number is taken before the message goes out, so it is older than the message
		// itself.
		LocalCaret.sequence = app.nextInputSequence();
		app.socket.sendMessage(
			'textcursor object=' +
				caret.objectId +
				' paragraph=' +
				caret.logical.paragraph +
				' index=' +
				caret.logical.index +
				' sequence=' +
				LocalCaret.sequence,
		);
	}

	/// Tells the engine where the caret is when the message may read or move it.
	public static tellBefore(message: string): void {
		if (!LocalCaret.caret || LocalCaret.sequence) return;
		if (LocalCaret.isUnrelated(message)) return;
		LocalCaret.tell();
	}

	/// True for a message that neither reads nor moves the caret.
	public static isUnrelated(message: string): boolean {
		return (
			LocalCaret.unrelated.some((prefix) => message.startsWith(prefix)) ||
			LocalCaret.hover.test(message)
		);
	}

	/// True when the caret update from the engine is older than the caret here, which then
	/// stays. The update that answers the textcursor message, or a later input, carries the
	/// engine's caret.
	public static isAhead(update: cool.CaretUpdate): boolean {
		if (!LocalCaret.caret) return false;
		if (!LocalCaret.sequence) return true;
		const inputSequence = Number(update.inputSequence);
		if (update.inputSequence === undefined || isNaN(inputSequence)) return true;
		if (inputSequence < LocalCaret.sequence) return true;
		LocalCaret.forget();
		return false;
	}

	public static forget(): void {
		LocalCaret.stopRestTimer();
		LocalCaret.caret = null;
		LocalCaret.sequence = 0;
	}
}
