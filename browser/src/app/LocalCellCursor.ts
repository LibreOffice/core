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

/**
 * The cell cursor move and selection change that the client reports to the
 * engine. The engine in turn reports back the accepted cursor position and
 * selection range with .uno:CellCursorState.
 */

namespace LocalCellCursor {
	/** A cell, as [col, row]. */
	export type Address = [number, number];

	/** A cell range, as [startCol, startRow, endCol, endRow]. */
	export type Range = [number, number, number, number];

	/**
	 * The cell cursor and the selection of a view, as .uno:CellCursorState
	 * sends them.
	 */
	export interface State {
		part: number;
		cursor: {
			cell: Address;
			/** False when the engine hides the cell cursor. */
			visible: boolean;
		};
		selection: Range[];
	}

	/** A move reported to the engine in a setcellcursor message. */
	export interface Move {
		part: number;
		cell: Address;
		/**
		 * The other end of the selection, or null for a cursor with no
		 * selection.
		 */
		anchor: Address | null;
	}
}

class LocalCellCursor {
	/**
	 * The commands that read or move the cell cursor: the input, the uno
	 * commands, the selection and the sheet switch.
	 *
	 * A mouse message counts unless it is a hover, and a commandvalues
	 * only for the cursor.
	 */
	private static readonly cursorCommands: string[] = [
		'key',
		'textinput',
		'removetextcontext',
		'paste',
		'uno',
		'dialogevent',
		'selecttext',
		'gettextselection',
		'resetselection',
		'completefunction',
		'insertfile',
		'setclientpart',
	];

	/**
	 * The latest move made on the client that the engine has not replied
	 * to, or null.  When this is not null, the client has yet to notify the
	 * engine of the move.
	 *
	 * The move is to be reported as a setcellcursor message.
	 */
	private static move: LocalCellCursor.Move | null = null;

	/**
	 * The sequence number of the setcellcursor message that reported the
	 * move, or 0 while the move is not reported yet.
	 */
	private static sequence: number = 0;

	/**
	 * Wait this long for the cursor to rest before reporting the move, in
	 * milliseconds. A held arrow key moves the cursor many times a
	 * second.
	 */
	private static readonly restDelay: number = 300;

	/** The timer that reports the move once the cursor rests, or null. */
	private static restTimer: ReturnType<typeof setTimeout> | null = null;

	/** Test whether this view moves the cell cursor locally. */
	public static isEnabled(): boolean {
		const debug = app.map ? app.map._debug : null;
		return !!debug && debug.localCellCursorOn;
	}

	/**
	 * Notify the engine whether this view moves the cell cursor and selects
	 * cells by itself.
	 */
	public static reportMode(enabled: boolean): void {
		app.socket.sendMessage('setlocalcellcursor enabled=' + String(enabled));
	}

	/**
	 * Record a move made on the client, and report it to the engine at
	 * rest.
	 */
	public static moved(move: LocalCellCursor.Move): void {
		LocalCellCursor.move = move;
		LocalCellCursor.sequence = 0;
		LocalCellCursor.stopRestTimer();
		LocalCellCursor.restTimer = app.timerRegistry.setTimeout(
			'localcellcursorrest',
			LocalCellCursor.reportAtRest,
			LocalCellCursor.restDelay,
		);
	}

	/**
	 * Report the move to the engine in a setcellcursor message, once per
	 * move, numbered with the next sequence number.
	 */
	public static report(): void {
		LocalCellCursor.stopRestTimer();
		const move = LocalCellCursor.move;
		if (!move || LocalCellCursor.sequence) return;

		// The message carries no change of its own, so the tickets waiting
		// for the next message keep waiting.
		const number = cool.InputSequence.next([], false);
		LocalCellCursor.reported(move, number);
		const message = LocalCellCursor.buildSetCellCursorMessage(move, number);
		app.socket.sendMessage(message);
	}

	/**
	 * Report the pending move ahead of a message that may read or move the
	 * cell cursor, and get the message to send after it. The engine acts at
	 * wherever its cursor is, so a local move the engine does not yet know
	 * would cause the typed text, the command or the paste to be sent to
	 * the wrong cell.
	 */
	public static reportBefore(message: string): string {
		if (!LocalCellCursor.move || LocalCellCursor.sequence) return message;
		if (!LocalCellCursor.usesCursor(message)) return message;

		// Steal the sequence number of the original message to report the
		// move, then increment the sequence number of the original message
		// before returning it to the caller.
		const unstamped = cool.InputSequence.unstamp(message);
		LocalCellCursor.report();
		return unstamped
			? cool.InputSequence.stamp(unstamped.message, unstamped.tickets)
			: message;
	}

	/**
	 * Remember a move reported to the engine in a setcellcursor message
	 * with this number.
	 */
	public static reported(move: LocalCellCursor.Move, sequence: number): void {
		LocalCellCursor.move = move;
		LocalCellCursor.sequence = sequence;
	}

	/**
	 * Test whether an input sequence number predates the local move. Call
	 * this on receiving a cellcursor, textselection or similar message
	 * from the engine, with the number stamped on it, and drop the message
	 * when this returns true to prevent the pending local move to be
	 * undone.
	 */
	public static isStale(inputSequence: number | undefined): boolean {
		if (!LocalCellCursor.move)
			// no move waits for the engine
			return false;

		// The engine has not been reported of the move yet.
		if (!LocalCellCursor.sequence) return true;
		if (inputSequence === undefined) return true;

		return inputSequence < LocalCellCursor.sequence;
	}

	/**
	 * Test whether the engine, in its reply to the move, put the cursor
	 * elsewhere.
	 */
	public static isCorrection(
		state: LocalCellCursor.State,
		inputSequence: number | undefined,
	): boolean {
		const move = LocalCellCursor.move; // what is sent to the engine
		if (!move || inputSequence !== LocalCellCursor.sequence) return false;

		return !LocalCellCursor.matches(move, state);
	}

	/** Drop the move made on the client. */
	public static forget(): void {
		LocalCellCursor.stopRestTimer();
		LocalCellCursor.move = null;
		LocalCellCursor.sequence = 0;
	}

	/** Test whether a message may read or move the cell cursor. */
	public static usesCursor(message: string): boolean {
		const parser = new MessageParser(message);
		const command = parser.command;
		if (command === 'mouse') return !parser.isHover;
		if (command === 'commandvalues')
			return parser.uno?.name === '.uno:CellCursor';

		return LocalCellCursor.cursorCommands.includes(command);
	}

	/** Report the move once the rest timer fires. */
	private static reportAtRest(): void {
		LocalCellCursor.restTimer = null;
		LocalCellCursor.report();
	}

	/** Stop the rest timer, if it runs. */
	private static stopRestTimer(): void {
		if (LocalCellCursor.restTimer === null) return;
		app.timerRegistry.clearTimeout(LocalCellCursor.restTimer);
		LocalCellCursor.restTimer = null;
	}

	/** Build the setcellcursor message that reports the move. */
	private static buildSetCellCursorMessage(
		move: LocalCellCursor.Move,
		sequence: number,
	): string {
		let message =
			'setcellcursor part=' +
			move.part +
			' col=' +
			move.cell[0] +
			' row=' +
			move.cell[1];
		if (move.anchor)
			message +=
				' anchorcol=' + move.anchor[0] + ' anchorrow=' + move.anchor[1];
		return message + ' sequence=' + sequence;
	}

	/** Test whether the engine's state is what the move asked for. */
	private static matches(
		move: LocalCellCursor.Move,
		state: LocalCellCursor.State,
	): boolean {
		if (state.part !== move.part) return false;

		// Calc keeps the cell cursor on the anchor of a selection.
		const expectedCursor = move.anchor ?? move.cell;
		if (!LocalCellCursor.equalArrays(state.cursor.cell, expectedCursor))
			return false;

		if (!move.anchor) return state.selection.length === 0;

		if (state.selection.length !== 1) return false;
		const expectedRange = LocalCellCursor.rangeBetween(move.anchor, move.cell);
		return LocalCellCursor.equalArrays(state.selection[0], expectedRange);
	}

	/** Get the range that two corner cells span. */
	public static rangeBetween(
		a: LocalCellCursor.Address,
		b: LocalCellCursor.Address,
	): LocalCellCursor.Range {
		const startCol = Math.min(a[0], b[0]);
		const startRow = Math.min(a[1], b[1]);
		const endCol = Math.max(a[0], b[0]);
		const endRow = Math.max(a[1], b[1]);
		return [startCol, startRow, endCol, endRow];
	}

	/**
	 * Get the moving end of a selected range, the corner opposite its
	 * anchor. Null when the anchor is not at a corner of the range.
	 *
	 * @param anchor The cells that the anchor covers, more than one for a
	 * merged cell.
	 */
	public static selectionEnd(
		range: LocalCellCursor.Range,
		anchor: LocalCellCursor.Range,
	): LocalCellCursor.Address | null {
		const atStartCol = anchor[0] === range[0];
		const atEndCol = anchor[2] === range[2];
		const atStartRow = anchor[1] === range[1];
		const atEndRow = anchor[3] === range[3];
		if (!(atStartCol || atEndCol) || !(atStartRow || atEndRow)) return null;

		const col = atStartCol ? range[2] : range[0];
		const row = atStartRow ? range[3] : range[1];
		return [col, row];
	}

	/** Test whether two arrays of indexes are equal. */
	public static equalArrays(a: number[], b: number[]): boolean {
		if (a.length !== b.length) return false;

		for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;

		return true;
	}
}
