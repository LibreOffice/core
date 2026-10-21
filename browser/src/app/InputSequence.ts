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
	Sequence numbers of the input messages, so the client can tell which of its inputs an answer
	from the engine reflects.

	Every key, text, caret and object change message, and every mouse message except a move with
	no button down, carries the next number as its sequence token, the last token of the message.
	The kit reports the newest number it applied, and as the numbers only grow, such an answer
	reflects every input up to that number.

	A change shown before the engine answers holds a ticket, which takes the number of the message
	that carries the change. The ticket is for the next message stamped, or is named when its
	message is stamped.

	The message stamped last can give its number back, to be stamped again after other messages
	went out ahead of it. The numbers then rise in the order the messages go out.
*/
namespace cool {
	/// The sequence number of the message that carries a change.
	export class InputTicket {
		private sequence: number | undefined;

		/// The number the message went out with, or undefined before it is stamped.
		public sentAs(): number | undefined {
			return this.sequence;
		}

		/// Whether an answer naming that number as the newest applied input reflects this one.
		public appliedBy(inputSequence: number): boolean {
			return this.sequence !== undefined && this.sequence <= inputSequence;
		}

		/// Sets the number, again when the message is stamped again.
		public goesOutAs(sequence: number): void {
			this.sequence = sequence;
		}

		/// Drops the number, as the message gave it back.
		public takenBack(): void {
			this.sequence = undefined;
		}
	}

	/// A message with its number taken back, and the tickets that went with the number.
	export interface Unstamped {
		/// The message without its sequence token.
		message: string;
		tickets: InputTicket[];
	}

	export class InputSequence {
		/// The tickets waiting for the next message stamped.
		private static forTheNextMessage: InputTicket[] = [];

		/// A ticket for the next message stamped.
		public static ticketForTheNextMessage(): InputTicket {
			const ticket = new InputTicket();
			InputSequence.forTheNextMessage.push(ticket);
			return ticket;
		}

		/// The tickets of the message stamped last.
		private static lastTickets: InputTicket[] = [];

		/*
			Takes the next number and gives it to the named tickets, and to those waiting for the
			next message unless the message carries no change of its own.
		*/
		public static next(
			tickets: InputTicket[] = [],
			withTheWaiting: boolean = true,
		): number {
			app.file.inputSequence += 1;
			const sequence = app.file.inputSequence;

			InputSequence.lastTickets = withTheWaiting
				? [...tickets, ...InputSequence.forTheNextMessage]
				: [...tickets];
			if (withTheWaiting) InputSequence.forTheNextMessage = [];
			for (const ticket of InputSequence.lastTickets)
				ticket.goesOutAs(sequence);

			return sequence;
		}

		/// Appends the next number to the message as its sequence token, ahead of the line break
		/// that a key message ends with.
		public static stamp(message: string, tickets: InputTicket[] = []): string {
			return new MessageParser(message).with(
				'sequence',
				InputSequence.next(tickets),
			);
		}

		/*
			Takes the number back from the message stamped last, so that the next message stamped
			gets it, and returns the message without the token and the tickets that held the
			number. Null for any other message, which keeps its number.
		*/
		public static unstamp(message: string): Unstamped | null {
			const parser = new MessageParser(message);
			const sequence = parser.sequence;
			if (sequence === undefined || sequence !== app.file.inputSequence)
				return null;

			app.file.inputSequence -= 1;
			const tickets = InputSequence.lastTickets;
			InputSequence.lastTickets = [];
			for (const ticket of tickets) ticket.takenBack();

			return { message: parser.without('sequence'), tickets };
		}

		/// Drops the tickets waiting for the next message and those of the message stamped last.
		public static forget(): void {
			InputSequence.forTheNextMessage = [];
			InputSequence.lastTickets = [];
		}

		/// The number in the "inputSequence=<number>" of a header line from the kit, if any.
		public static readFrom(header: string): number | undefined {
			const sequence = parseInt(header.split('inputSequence=')[1] ?? '');
			return isNaN(sequence) || sequence <= 0 ? undefined : sequence;
		}
	}
}
