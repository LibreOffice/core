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
	Reads a message to the engine as tokens split on single spaces: the command first, then its
	parameters, each a name and a value joined by an equals sign. A key message ends with a line
	break, which is kept apart from the tokens.
*/
class MessageParser {
	private readonly tokens: string[];
	/// What follows the last token: a line break or nothing.
	private readonly tail: string;

	constructor(message: string) {
		const body = message.trimEnd();
		this.tokens = body.split(' ');
		this.tail = message.substring(body.length);
	}

	/// The first token.
	public get command(): string {
		return this.tokens[0];
	}

	/// How many tokens there are, the command included.
	public get length(): number {
		return this.tokens.length;
	}

	/// The value of the named parameter, or undefined where there is none.
	public value(name: string): string | undefined {
		const at = this.indexOf(name);
		return at < 0 ? undefined : this.tokens[at].substring(name.length + 1);
	}

	/// The value of the named parameter as a non-negative integer, or undefined where it is
	/// not one.
	public count(name: string): number | undefined {
		const text = this.value(name);
		const number = Number(text);
		return text && Number.isInteger(number) && number >= 0 ? number : undefined;
	}

	/// The sequence number of an input message, or undefined where there is none.
	public get sequence(): number | undefined {
		return this.count('sequence');
	}

	/// The message with the named parameter set to the value, added at the end where there
	/// was none.
	public with(name: string, value: string | number): string {
		const token = name + '=' + value;
		const at = this.indexOf(name);
		const tokens = this.tokens.slice();
		if (at < 0) tokens.push(token);
		else tokens[at] = token;
		return tokens.join(' ') + this.tail;
	}

	private indexOf(name: string): number {
		return this.tokens.findIndex((one) => one.startsWith(name + '='));
	}
}
