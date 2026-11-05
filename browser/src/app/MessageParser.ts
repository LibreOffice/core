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

/// A .uno: command with its arguments, from a uno or commandvalues message.
class ParsedUnoCommand {
	/// The command name without its query, such as '.uno:CellCursor'.
	public readonly name: string;
	/// What follows the question mark in the command, or an empty string.
	private readonly query: string;
	/// The JSON argument of a uno message as text, or undefined.
	private readonly text: string | undefined;

	constructor(command: string, json?: string) {
		const at = command.indexOf('?');
		this.name = at < 0 ? command : command.substring(0, at);
		this.query = at < 0 ? '' : command.substring(at + 1);
		this.text = json;
	}

	/// The query parameters by name, decoded; 'Mode:string=x' is keyed as 'Mode'.
	public params(): Map<string, string> {
		const params = new Map<string, string>();
		if (!this.query) return params;

		for (const pair of this.query.split('&')) {
			const at = pair.indexOf('=');
			const key = at < 0 ? pair : pair.substring(0, at);
			const value = at < 0 ? '' : pair.substring(at + 1);
			const name = key.split(':')[0];
			params.set(name, ParsedUnoCommand.decode(value));
		}
		return params;
	}

	/// The JSON argument of a uno message, parsed, or undefined where there is none or it
	/// does not parse.
	public json(): any {
		if (this.text === undefined) return undefined;
		try {
			return JSON.parse(this.text);
		} catch {
			return undefined;
		}
	}

	/// The value with its percent escapes decoded, or as it is where they are malformed.
	private static decode(value: string): string {
		try {
			return decodeURIComponent(value);
		} catch {
			return value;
		}
	}
}

class MessageParser {
	private readonly message: string;
	private readonly tokens: string[];
	/// What follows the last token: a line break or nothing.
	private readonly tail: string;

	constructor(message: string) {
		const body = message.trimEnd();
		this.message = message;
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

	/// The uno command of a uno or commandvalues message, or undefined for another message.
	public get uno(): ParsedUnoCommand | undefined {
		if (this.command === 'commandvalues') {
			const command = this.value('command');
			return command === undefined ? undefined : new ParsedUnoCommand(command);
		}
		if (this.command !== 'uno' || this.tokens.length < 2) return undefined;

		// The JSON argument holds spaces, so it is taken from the message, not the tokens.
		const at = this.message.indexOf('{');
		const json = at < 0 ? undefined : this.message.substring(at);
		return new ParsedUnoCommand(this.tokens[1], json);
	}

	/// True for a mouse move with no button down.
	public get isHover(): boolean {
		return (
			this.command === 'mouse' &&
			this.value('type') === 'move' &&
			this.value('buttons') === '0'
		);
	}

	/// The message with the named parameter set to the value, added at the end where there
	/// was none.
	public with(name: string, value: string | number): string {
		const token = name + '=' + value;
		const at = this.indexOf(name);
		const tokens = this.tokens.slice();
		if (at < 0) tokens.push(token);
		else tokens[at] = token;
		return this.joined(tokens);
	}

	/// The message without the named parameter, or as it is where there is none.
	public without(name: string): string {
		const at = this.indexOf(name);
		if (at < 0) return this.joined(this.tokens);
		const tokens = this.tokens.slice();
		tokens.splice(at, 1);
		return this.joined(tokens);
	}

	/// The message made of those tokens, with the tail the parsed message had after them.
	private joined(tokens: string[]): string {
		return tokens.join(' ') + this.tail;
	}

	private indexOf(name: string): number {
		return this.tokens.findIndex((one) => one.startsWith(name + '='));
	}
}
