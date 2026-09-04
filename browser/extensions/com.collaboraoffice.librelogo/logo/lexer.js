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

// Tokenizer for the LibreLogo language. It mirrors the preprocessing that
// LibreLogo.py does with regular expressions before handing the program to
// Python: comments, line continuation with "~", orthographic and Python
// string quotes, "word strings, numbers with units (10cm, 1", 3h, 90°), and
// the whitespace rule that tells a program block "[ ... ]" apart from a
// list "[a, b]".

export const T = {
	NUM: 'num',
	STR: 'str',
	ID: 'id',
	OP: 'op',
	LBLOCK: '[block',
	RBLOCK: 'block]',
	LBRACK: '[',
	RBRACK: ']',
	LPAREN: '(',
	RPAREN: ')',
	COMMA: ',',
	IN_SUFFIX: 'in-suffix',
	NEWLINE: 'newline',
	EOF: 'eof',
};

export class LogoSyntaxError extends Error {
	constructor(message, line, kind) {
		super(message);
		this.name = 'LogoSyntaxError';
		this.line = line;
		this.kind = kind || 'syntax';
	}
}

// Longest first so "+=" beats "+", "**=" beats "**", and so on.
const OPERATORS = ['**=', '//=', '**', '//', '==', '!=', '<>', '<=', '>=',
	'+=', '-=', '*=', '/=', '%=', '+', '-', '*', '/', '%', '=', '<', '>', '|', '&', '^'];

function isWordChar(ch) {
	return /[\p{L}\p{N}_]/u.test(ch);
}

function isDigit(ch) {
	return ch >= '0' && ch <= '9';
}

function isSpace(ch) {
	return ch === ' ' || ch === '\t';
}

export class Lexer {
	constructor(source, locale) {
		this.src = source.replace(/\r/g, '');
		this.locale = locale;
		this.pos = 0;
		this.line = 1;
		this.tokens = [];
	}

	peek(offset = 0) {
		return this.src[this.pos + offset] || '';
	}

	push(type, value, extra) {
		const tok = { type, value, line: this.line, spaceBefore: this._spaceBefore, pos: this._tokStart };
		if (extra) Object.assign(tok, extra);
		this.tokens.push(tok);
		return tok;
	}

	tokenize() {
		const src = this.src;
		let sawSpace = true; // start of input behaves like whitespace
		while (this.pos < src.length) {
			const ch = src[this.pos];
			this._tokStart = this.pos;
			this._spaceBefore = sawSpace;
			sawSpace = false;

			if (isSpace(ch)) {
				this.pos++;
				sawSpace = true;
				continue;
			}
			if (ch === '\n') {
				this.push(T.NEWLINE, '\n');
				this.pos++;
				this.line++;
				sawSpace = true;
				continue;
			}
			// "~" at the end of a line continues it on the next paragraph.
			if (ch === '~') {
				let p = this.pos + 1;
				while (isSpace(src[p])) p++;
				if (src[p] === '\n' || p >= src.length) {
					this.pos = p + 1;
					this.line++;
					sawSpace = true;
					continue;
				}
			}
			// Comments: ";" anywhere, "#" only at the start of a line.
			if (ch === ';' || (ch === '#' && this._atLineStart())) {
				while (this.pos < src.length && src[this.pos] !== '\n') this.pos++;
				sawSpace = true;
				continue;
			}
			if (isDigit(ch)) {
				this.readNumber();
				sawSpace = this._followedBySpace();
				continue;
			}
			if (ch === "'" || this.locale.leftQuotes.includes(ch)) {
				this.readQuotedString();
				sawSpace = this._followedBySpace();
				continue;
			}
			if (ch === '"') {
				this.readWordString();
				sawSpace = this._followedBySpace();
				continue;
			}
			if (ch === '[') {
				const next = this.peek(1);
				const isBlock = next === ' ' || next === '\t' || next === '\n' || next === '';
				this.push(isBlock ? T.LBLOCK : T.LBRACK, '[');
				this.pos++;
				sawSpace = isBlock;
				continue;
			}
			if (ch === ']') {
				const isBlock = this._spaceBefore || this._atLineStart();
				this.push(isBlock ? T.RBLOCK : T.RBRACK, ']');
				this.pos++;
				sawSpace = this._followedBySpace();
				continue;
			}
			if (ch === '(') {
				this.push(T.LPAREN, '(');
				this.pos++;
				continue;
			}
			if (ch === ')') {
				this.push(T.RPAREN, ')');
				this.pos++;
				sawSpace = this._followedBySpace();
				continue;
			}
			if (ch === ',') {
				this.push(T.COMMA, ',');
				this.pos++;
				continue;
			}
			// n-dash and m-dash count as minus signs
			if (ch === '–' || ch === '—') {
				this.push(T.OP, '-', { spaceAfter: this._followedBySpaceAt(this.pos + 1) });
				this.pos++;
				continue;
			}
			let matched = false;
			for (const op of OPERATORS) {
				if (src.startsWith(op, this.pos)) {
					this.push(T.OP, op === '<>' ? '!=' : op,
						{ spaceAfter: this._followedBySpaceAt(this.pos + op.length) });
					this.pos += op.length;
					matched = true;
					break;
				}
			}
			if (matched) continue;
			if (ch === ':' || isWordChar(ch)) {
				this.readIdentifier();
				sawSpace = this._followedBySpace();
				continue;
			}
			throw new LogoSyntaxError('unexpected character ' + JSON.stringify(ch), this.line);
		}
		this._tokStart = this.pos;
		this._spaceBefore = true;
		this.push(T.EOF, '');
		this.joinInSuffixes();
		return this.tokens;
	}

	_atLineStart() {
		let p = this.pos - 1;
		while (p >= 0 && isSpace(this.src[p])) p--;
		return p < 0 || this.src[p] === '\n';
	}

	_followedBySpace() {
		return this._followedBySpaceAt(this.pos);
	}

	_followedBySpaceAt(p) {
		const c = this.src[p];
		return c === undefined || c === ' ' || c === '\t' || c === '\n';
	}

	readNumber() {
		const src = this.src;
		const start = this.pos;
		let value;
		if (src[this.pos] === '0' && (src[this.pos + 1] === 'x' || src[this.pos + 1] === 'X')) {
			this.pos += 2;
			while (/[0-9a-fA-F]/.test(this.peek())) this.pos++;
			value = parseInt(src.slice(start, this.pos), 16);
			this.push(T.NUM, value, { isFloat: false });
			return;
		}
		while (isDigit(this.peek())) this.pos++;
		let isFloat = false;
		const dec = this.locale.decimal;
		if ((this.peek() === '.' || (dec === ',' && this.peek() === ',')) && isDigit(this.peek(1))) {
			isFloat = true;
			this.pos++;
			while (isDigit(this.peek())) this.pos++;
		}
		value = parseFloat(src.slice(start, this.pos).replace(',', '.'));
		// A unit suffix converts to points (or degrees for hours).
		let unit = null;
		const rest = src.slice(this.pos, this.pos + 8).toLowerCase();
		for (const [name, factor] of this.locale.units) {
			if (!rest.startsWith(name)) continue;
			// Letter units need a word boundary; symbols like " and ° do not.
			const after = src[this.pos + name.length];
			if (isWordChar(name[name.length - 1]) && after !== undefined && isWordChar(after)) continue;
			unit = name;
			value *= factor;
			if (factor !== 1) isFloat = true;
			this.pos += name.length;
			break;
		}
		this.push(T.NUM, value, { isFloat, unit });
	}

	readQuotedString() {
		const src = this.src;
		const closers = "'" + this.locale.rightQuotes;
		this.pos++; // opening quote
		let out = '';
		while (this.pos < src.length) {
			const c = src[this.pos];
			if (c === '\n') throw new LogoSyntaxError('unterminated string', this.line);
			if (c === '\\' && closers.includes(src[this.pos + 1] || '')) {
				out += src[this.pos + 1];
				this.pos += 2;
				continue;
			}
			if (closers.includes(c)) {
				this.pos++;
				this.push(T.STR, out);
				return;
			}
			out += c;
			this.pos++;
		}
		throw new LogoSyntaxError('unterminated string', this.line);
	}

	// "word - the traditional Logo string: a double quote followed by word characters.
	readWordString() {
		this.pos++;
		let out = '';
		if (this.peek() === '~') {
			out += '~';
			this.pos++;
		}
		while (this.pos < this.src.length && isWordChar(this.src[this.pos])) {
			out += this.src[this.pos];
			this.pos++;
		}
		this.push(T.STR, out);
	}

	readIdentifier() {
		const src = this.src;
		let colon = false;
		if (src[this.pos] === ':') {
			// ":size" is a variable; "a[1:3]" and "a[:3]" use ":" as a slice operator.
			if (!isWordChar(this.peek(1)) || isDigit(this.peek(1))) {
				this.push(T.OP, ':', { spaceAfter: this._followedBySpaceAt(this.pos + 1) });
				this.pos++;
				return;
			}
			colon = true;
			this.pos++;
		}
		const start = this.pos;
		while (this.pos < src.length && isWordChar(src[this.pos])) this.pos++;
		const word = src.slice(start, this.pos);
		// The colon is an optional variable marker: ":size" and "size" are the
		// same variable, and a colon-prefixed word is never a keyword.
		const keyword = colon ? undefined : this.locale.keywordOf(word);
		this.push(T.ID, word, { keyword, colon, spaceAfter: this._followedBySpace() });
	}

	// In languages where IN is a suffix ("FOR x lista-ban [ ... ]") the lexer
	// sees OP "-" followed by an identifier; fold those into an IN_SUFFIX token.
	joinInSuffixes() {
		if (this.locale.inSuffixes.length === 0) return;
		const out = [];
		for (let i = 0; i < this.tokens.length; i++) {
			const tok = this.tokens[i];
			const next = this.tokens[i + 1];
			if (tok.type === T.OP && tok.value === '-' && !tok.spaceAfter && next && next.type === T.ID
				&& !next.spaceBefore && this.locale.inSuffixes.includes(next.value.toLowerCase())) {
				out.push({ type: T.IN_SUFFIX, value: '-' + next.value, line: tok.line, spaceBefore: tok.spaceBefore, pos: tok.pos });
				i++;
				continue;
			}
			out.push(tok);
		}
		this.tokens = out;
	}
}

export function tokenize(source, locale) {
	return new Lexer(source, locale).tokenize();
}
