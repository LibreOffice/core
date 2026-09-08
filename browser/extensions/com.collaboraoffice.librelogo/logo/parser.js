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

// Parser for LibreLogo programs. Produces a plain-object AST that interp.js
// walks. The grammar follows LibreLogo.py's behaviour: commands and
// functions take a fixed number of arguments written after them without
// parentheses ("FORWARD 10", "PRINT SQRT 100"), or an explicit Python-style
// argument list ("star(100, 'red')"); operators inside an argument bind
// tighter than the call, so "PRINT 5 + 10" prints 15.

import { T, LogoSyntaxError } from './lexer.js';

// Commands usable as statements, with their argument count.
export const COMMANDS = {
	FORWARD: 1, BACKWARD: 1, TURNLEFT: 1, TURNRIGHT: 1, PENUP: 0, PENDOWN: 0,
	HOME: 0, POINT: 0, CIRCLE: 1, ELLIPSE: 1, SQUARE: 1, RECTANGLE: 1,
	LABEL: 1, TEXT: 1, PENCOLOR: 1, PENWIDTH: 1, PENSTYLE: 1, PENJOINT: 1,
	PENCAP: 1, CLOSE: 0, FILL: 0, FILLCOLOR: 1, FILLTRANSPARENCY: 1,
	PENTRANSPARENCY: 1, FILLSTYLE: 1, FONTCOLOR: 1, FONTTRANSPARENCY: 1,
	FONTHEIGHT: 1, FONTWEIGHT: 1, FONTSTYLE: 1, FONTFAMILY: 1,
	CLEARSCREEN: 0, HIDETURTLE: 0, SHOWTURTLE: 0, POSITION: 1, HEADING: 1,
	PRINT: 1, SLEEP: 1,
};

// Functions usable inside expressions, with their argument count
// (RANGE takes one to three).
export const FUNCTIONS = {
	RANDOM: 1, INT: 1, FLOAT: 1, STR: 1, SQRT: 1, LOG10: 1, SIN: 1, COS: 1,
	ROUND: 1, ABS: 1, COUNT: 1, SET: 1, RANGE: 3, LIST: 1, TUPLE: 1,
	SORTED: 1, RESUB: 3, RESEARCH: 2, REFINDALL: 2, MIN: 1, MAX: 1, INPUT: 1,
};

// Setter commands that read the current value when used inside an
// expression without an argument: "PRINT FILLCOLOR", "p = POSITION".
export const GETTERS = new Set([
	'PENCOLOR', 'PENWIDTH', 'PENSTYLE', 'PENJOINT', 'PENCAP', 'FILLCOLOR',
	'FILLTRANSPARENCY', 'PENTRANSPARENCY', 'FONTCOLOR', 'FONTTRANSPARENCY',
	'FONTHEIGHT', 'FONTWEIGHT', 'FONTSTYLE', 'FONTFAMILY', 'POSITION',
	'HEADING', 'PAGESIZE',
]);

const CONSTANTS = new Set(['TRUE', 'FALSE', 'ANY', 'PI', 'REPCOUNT']);

// Keywords that can only start a statement; an expression ends when it meets one.
const STATEMENT_KEYWORDS = new Set([
	'TO', 'END', 'REPEAT', 'WHILE', 'FOR', 'IF', 'BREAK', 'CONTINUE', 'STOP',
	'OUTPUT', 'GLOBAL', 'GROUP',
]);

const COMPARISONS = new Set(['<', '>', '<=', '>=', '=', '==', '!=']);

// Assignment operators. Returns null for a plain "=" and the base operator
// ("+", "//", ...) for an augmented assignment, or undefined for anything else.
const AUGMENTED = { '+=': '+', '-=': '-', '*=': '*', '/=': '/', '//=': '//', '%=': '%', '**=': '**' };
function assignOp(tok) {
	if (!tok || tok.type !== T.OP) return undefined;
	if (tok.value === '=') return null;
	if (Object.prototype.hasOwnProperty.call(AUGMENTED, tok.value)) return AUGMENTED[tok.value];
	return undefined;
}

// Builds the AST that interp.js walks from the lexer's tokens.
export class Parser {
	constructor(tokens, locale) {
		this.toks = tokens;
		this.locale = locale;
		this.i = 0;
		this.procs = new Map(); // name -> { params, body, isFunction, line }
		// > 0 while parsing the arguments of a prefix call ("SET [1, 2] | SET [3]"):
		// there the set operators bind looser than the call, as in LibreLogo.py,
		// where they end the argument. Reset inside parentheses and lists.
		this.argDepth = 0;
	}

	// Parse a full expression regardless of the surrounding argument context.
	parseNested() {
		const saved = this.argDepth;
		this.argDepth = 0;
		try {
			return this.parseExpr();
		} finally {
			this.argDepth = saved;
		}
	}

	// --- token helpers -------------------------------------------------

	peek(o = 0) {
		return this.toks[Math.min(this.i + o, this.toks.length - 1)];
	}

	next() {
		const t = this.toks[this.i];
		if (this.i < this.toks.length - 1) this.i++;
		return t;
	}

	skipNewlines() {
		while (this.peek().type === T.NEWLINE) this.next();
	}

	isKeyword(tok, key) {
		return tok.type === T.ID && tok.keyword === key;
	}

	error(message, tok, kind) {
		throw new LogoSyntaxError(message, (tok || this.peek()).line, kind);
	}

	blockError(tok) {
		this.error(this.locale.message('ERR_BLOCK'), tok, 'block');
	}

	expect(type, value) {
		const t = this.peek();
		if (t.type !== type || (value !== undefined && t.value !== value)) {
			if (type === T.LBLOCK || type === T.RBLOCK) this.blockError(t);
			this.error('expected ' + (value || type) + ' but found ' + JSON.stringify(t.value), t);
		}
		return this.next();
	}

	// --- program ---------------------------------------------------------

	parseProgram() {
		this.prescanProcedures();
		const body = [];
		this.skipNewlines();
		while (this.peek().type !== T.EOF) {
			const stmt = this.parseStatement();
			if (stmt) body.push(stmt);
			this.skipNewlines();
		}
		return { type: 'program', body, procs: this.procs };
	}

	// Procedures can be called before they are defined, and a procedure's
	// argument count decides how its calls are parsed, so collect the
	// headers first. A procedure that uses OUTPUT is a function.
	prescanProcedures() {
		for (let i = 0; i < this.toks.length; i++) {
			const t = this.toks[i];
			if (!this.isKeyword(t, 'TO')) continue;
			const prev = this.toks[i - 1];
			if (prev && prev.type !== T.NEWLINE && prev.type !== T.RBLOCK && prev.type !== T.LBLOCK) continue;
			const nameTok = this.toks[i + 1];
			if (!nameTok || nameTok.type !== T.ID || nameTok.keyword) continue;
			const params = [];
			let j = i + 2;
			while (this.toks[j] && this.toks[j].type === T.ID) {
				params.push(this.toks[j].value);
				j++;
			}
			let isFunction = false;
			for (let k = j; k < this.toks.length; k++) {
				const u = this.toks[k];
				if (this.isKeyword(u, 'END') || this.isKeyword(u, 'TO')) break;
				if (this.isKeyword(u, 'OUTPUT')) isFunction = true;
			}
			this.procs.set(nameTok.value, { name: nameTok.value, params, body: null, isFunction, line: t.line });
		}
	}

	// --- statements ------------------------------------------------------

	parseBlock() {
		const open = this.peek();
		if (open.type !== T.LBLOCK) this.blockError(open);
		this.next();
		const body = [];
		this.skipNewlines();
		while (this.peek().type !== T.RBLOCK) {
			if (this.peek().type === T.EOF) this.blockError(open);
			const stmt = this.parseStatement();
			if (stmt) body.push(stmt);
			this.skipNewlines();
		}
		this.next();
		return body;
	}

	parseStatement() {
		const tok = this.peek();
		const line = tok.line;
		if (tok.type === T.RBLOCK || tok.type === T.RBRACK || tok.type === T.RPAREN) this.blockError(tok);

		if (tok.type === T.ID) {
			const key = tok.keyword;
			switch (key) {
			case 'TO': return this.parseProcedure();
			case 'END': this.error(this.locale.message('ERR_BLOCK'), tok, 'block'); break;
			case 'REPEAT': {
				this.next();
				let count = null;
				if (this.peek().type !== T.LBLOCK) count = this.parseExpr();
				const body = this.parseBlock();
				return { type: 'repeat', count, body, line };
			}
			case 'WHILE': {
				this.next();
				const cond = this.parseExpr();
				const body = this.parseBlock();
				return { type: 'while', cond, body, line };
			}
			case 'FOR': {
				this.next();
				const v = this.expect(T.ID);
				let iter;
				if (this.isKeyword(this.peek(), 'IN')) {
					this.next();
					iter = this.parseExpr();
				} else {
					iter = this.parseExpr();
					if (this.peek().type !== T.IN_SUFFIX) this.error('expected IN', this.peek());
					this.next();
				}
				const body = this.parseBlock();
				return { type: 'for', name: v.value, iter, body, line };
			}
			case 'IF': {
				this.next();
				const cond = this.parseExpr();
				const then = this.parseBlock();
				let otherwise = null;
				const save = this.i;
				this.skipNewlines();
				if (this.peek().type === T.LBLOCK) otherwise = this.parseBlock();
				else this.i = save;
				return { type: 'if', cond, then, otherwise, line };
			}
			case 'BREAK': this.next(); return { type: 'break', line };
			case 'CONTINUE': this.next(); return { type: 'continue', line };
			case 'STOP': this.next(); return { type: 'stop', line };
			case 'OUTPUT': {
				this.next();
				return { type: 'output', value: this.parseExpr(), line };
			}
			case 'GLOBAL': {
				this.next();
				const names = [];
				while (this.peek().type === T.ID && !this.peek().keyword) names.push(this.next().value);
				return { type: 'global', names, line };
			}
			case 'GROUP': {
				this.next();
				if (this.peek().type === T.LBLOCK) return { type: 'picture', name: null, body: this.parseBlock(), line };
				if (this.startsExpression(this.peek())) {
					const name = this.parseExpr();
					if (this.peek().type === T.LBLOCK) return { type: 'picture', name, body: this.parseBlock(), line };
				}
				return { type: 'picture', name: null, body: null, line };
			}
			default:
				break;
			}
			if (key && COMMANDS[key] !== undefined) {
				this.next();
				const args = this.parsePrefixArgs(tok, COMMANDS[key], true);
				return { type: 'command', key, args, line };
			}
			if (!key && !this.procs.has(tok.value)) {
				// x = value or x <op>= value, and the same for x[i]
				const op1 = assignOp(this.peek(1));
				if (op1 !== undefined) {
					this.next();
					this.next();
					return { type: 'assign', target: { type: 'var', name: tok.value }, op: op1, value: this.parseExpr(), line };
				}
				if (this.peek(1).type === T.LBRACK && !this.peek(1).spaceBefore) {
					const save = this.i;
					const target = this.parsePostfix(this.parsePrimary());
					const op = assignOp(this.peek());
					if (op !== undefined && target.type === 'index') {
						this.next();
						return { type: 'assign', target, op, value: this.parseExpr(), line };
					}
					this.i = save;
				}
			}
		}
		// Anything else is an expression evaluated for its effect: a
		// procedure call, a function call, a bare value.
		const expr = this.parseExpr();
		return { type: 'expr', expr, line };
	}

	parseProcedure() {
		const toTok = this.next();
		const nameTok = this.peek();
		if (nameTok.type !== T.ID || nameTok.keyword) this.error('expected a procedure name after TO', nameTok);
		this.next();
		const proc = this.procs.get(nameTok.value);
		while (this.peek().type === T.ID) this.next(); // parameters, collected by prescan
		if (this.peek().type !== T.NEWLINE && this.peek().type !== T.EOF) this.error('expected a new line after the procedure header', this.peek());
		const body = [];
		this.skipNewlines();
		while (!this.isKeyword(this.peek(), 'END')) {
			if (this.peek().type === T.EOF) this.error(this.locale.message('ERR_BLOCK'), toTok, 'block');
			const stmt = this.parseStatement();
			if (stmt) body.push(stmt);
			this.skipNewlines();
		}
		this.next(); // END
		proc.body = body;
		return null;
	}

	// n argument expressions written after a command or function name.
	// `top` is set for statement commands (PRINT, FORWARD, ...), whose argument
	// is the whole rest of the expression, set operators included.
	parsePrefixArgs(nameTok, arity, top) {
		const args = [];
		const saved = this.argDepth;
		this.argDepth = top ? 0 : this.argDepth + 1;
		try {
			for (let k = 0; k < arity; k++) {
				if (!this.startsExpression(this.peek())) {
					this.error(this.locale.message('ERR_ARGUMENTS', this.displayName(nameTok), arity, k), nameTok, 'arguments');
				}
				args.push(this.parseExpr());
			}
		} finally {
			this.argDepth = saved;
		}
		return args;
	}

	displayName(tok) {
		return tok.keyword ? this.locale.localName(tok.keyword).toUpperCase() : tok.value;
	}

	startsExpression(tok) {
		switch (tok.type) {
		case T.NUM: case T.STR: case T.LBRACK: case T.LPAREN: return true;
		case T.OP: return tok.value === '-' || tok.value === '+';
		case T.ID:
			if (!tok.keyword) return true;
			if (STATEMENT_KEYWORDS.has(tok.keyword)) return false;
			if (COMMANDS[tok.keyword] !== undefined && !GETTERS.has(tok.keyword)) return false;
			return tok.keyword !== 'IN' && tok.keyword !== 'AND' && tok.keyword !== 'OR';
		default: return false;
		}
	}

	// --- expressions -----------------------------------------------------

	parseExpr() {
		return this.parseOr();
	}

	parseOr() {
		let left = this.parseAnd();
		while (this.isKeyword(this.peek(), 'OR')) {
			this.next();
			left = { type: 'or', left, right: this.parseAnd() };
		}
		return left;
	}

	parseAnd() {
		let left = this.parseNot();
		while (this.isKeyword(this.peek(), 'AND')) {
			this.next();
			left = { type: 'and', left, right: this.parseNot() };
		}
		return left;
	}

	parseNot() {
		if (this.isKeyword(this.peek(), 'NOT')) {
			this.next();
			return { type: 'not', operand: this.parseNot() };
		}
		return this.parseComparison();
	}

	parseComparison() {
		const first = this.parseBitOr();
		const ops = [];
		const operands = [first];
		for (;;) {
			const t = this.peek();
			if (t.type === T.OP && COMPARISONS.has(t.value)) {
				this.next();
				ops.push(t.value === '=' ? '==' : t.value);
				operands.push(this.parseBitOr());
			} else if (this.isKeyword(t, 'IN')) {
				this.next();
				ops.push('in');
				operands.push(this.parseBitOr());
			} else if (this.isKeyword(t, 'NOT') && this.isKeyword(this.peek(1), 'IN')) {
				this.next();
				this.next();
				ops.push('not in');
				operands.push(this.parseBitOr());
			} else {
				break;
			}
		}
		if (ops.length === 0) return first;
		return { type: 'compare', ops, operands };
	}

	parseBitOr() {
		let left = this.parseBitXor();
		while (this.argDepth === 0 && this.peek().type === T.OP && this.peek().value === '|') {
			this.next();
			left = { type: 'binop', op: '|', left, right: this.parseBitXor() };
		}
		return left;
	}

	parseBitXor() {
		let left = this.parseBitAnd();
		while (this.argDepth === 0 && this.peek().type === T.OP && this.peek().value === '^') {
			this.next();
			left = { type: 'binop', op: '^', left, right: this.parseBitAnd() };
		}
		return left;
	}

	parseBitAnd() {
		let left = this.parseArith();
		while (this.argDepth === 0 && this.peek().type === T.OP && this.peek().value === '&') {
			this.next();
			left = { type: 'binop', op: '&', left, right: this.parseArith() };
		}
		return left;
	}

	// "10 - 5" and "10-5" subtract; "ABS -10" is a sign: a minus with space
	// before but none after starts a new operand, as in LibreLogo.py.
	isBinarySign(t) {
		return t.type === T.OP && (t.value === '-' || t.value === '+') && !(t.spaceBefore && !t.spaceAfter);
	}

	parseArith() {
		let left = this.parseTerm();
		while (this.isBinarySign(this.peek())) {
			const op = this.next().value;
			left = { type: 'binop', op, left, right: this.parseTerm() };
		}
		return left;
	}

	parseTerm() {
		let left = this.parseUnary();
		for (;;) {
			const t = this.peek();
			if (t.type === T.OP && (t.value === '*' || t.value === '/' || t.value === '//' || t.value === '%')) {
				this.next();
				left = { type: 'binop', op: t.value, left, right: this.parseUnary() };
			} else {
				break;
			}
		}
		return left;
	}

	parseUnary() {
		const t = this.peek();
		if (t.type === T.OP && (t.value === '-' || t.value === '+')) {
			this.next();
			return { type: 'unop', op: t.value, operand: this.parseUnary() };
		}
		return this.parsePower();
	}

	parsePower() {
		const base = this.parsePostfix(this.parsePrimary());
		if (this.peek().type === T.OP && this.peek().value === '**') {
			this.next();
			return { type: 'binop', op: '**', left: base, right: this.parseUnary() };
		}
		return base;
	}

	parsePostfix(expr) {
		for (;;) {
			const t = this.peek();
			if (t.type === T.LBRACK && !t.spaceBefore) {
				this.next();
				let lo = null, hi = null, isSlice = false;
				if (this.peek().type === T.OP && this.peek().value === ':') isSlice = true;
				else lo = this.parseExpr();
				if (this.peek().type === T.OP && this.peek().value === ':') {
					isSlice = true;
					this.next();
					if (this.peek().type !== T.RBRACK) hi = this.parseExpr();
				} else if (this.peek().type === T.ID && this.peek().colon) {
					// ":x" lexed as a colon variable inside a slice, e.g. a[1:x]
					isSlice = true;
					hi = { type: 'var', name: this.next().value };
				}
				if (this.peek().type !== T.RBRACK) this.error(this.locale.message('ERR_BLOCK'), t, 'block');
				this.next();
				expr = isSlice ? { type: 'slice', obj: expr, lo, hi } : { type: 'index', obj: expr, index: lo };
			} else if (t.type === T.LPAREN && !t.spaceBefore && (expr.type === 'var' || expr.type === 'proc')) {
				// value(args): calling the result of a variable is not supported,
				// but "f(x)" where f is a procedure is handled in parsePrimary.
				break;
			} else {
				break;
			}
		}
		return expr;
	}

	parseArgList() {
		this.expect(T.LPAREN);
		const args = [];
		this.skipNewlines();
		if (this.peek().type !== T.RPAREN) {
			args.push(this.parseNested());
			while (this.peek().type === T.COMMA) {
				this.next();
				this.skipNewlines();
				args.push(this.parseNested());
			}
		}
		this.skipNewlines();
		this.expect(T.RPAREN);
		return args;
	}

	parsePrimary() {
		const t = this.peek();
		switch (t.type) {
		case T.NUM:
			this.next();
			return { type: 'num', value: t.value, isFloat: t.isFloat };
		case T.STR:
			this.next();
			return { type: 'str', value: t.value };
		case T.LBRACK: {
			this.next();
			const items = [];
			this.skipNewlines();
			if (this.peek().type !== T.RBRACK) {
				items.push(this.parseNested());
				while (this.peek().type === T.COMMA) {
					this.next();
					this.skipNewlines();
					if (this.peek().type === T.RBRACK) break;
					items.push(this.parseNested());
				}
			}
			this.skipNewlines();
			if (this.peek().type !== T.RBRACK) this.blockError(t);
			this.next();
			return { type: 'list', items };
		}
		case T.LPAREN: {
			this.next();
			this.skipNewlines();
			const e = this.parseNested();
			this.skipNewlines();
			// A comma turns "(a, b, ...)" into a tuple, matching Python: MAX (2, 3).
			if (this.peek().type === T.COMMA) {
				const items = [e];
				while (this.peek().type === T.COMMA) {
					this.next();
					this.skipNewlines();
					if (this.peek().type === T.RPAREN) break;
					items.push(this.parseNested());
					this.skipNewlines();
				}
				this.expect(T.RPAREN);
				return { type: 'tuple', items };
			}
			this.expect(T.RPAREN);
			return e;
		}
		case T.ID:
			return this.parseName();
		case T.LBLOCK:
			this.blockError(t);
			break;
		default:
			break;
		}
		this.error('unexpected ' + JSON.stringify(t.value), t);
		return null;
	}

	parseName() {
		const t = this.next();
		const key = t.keyword;
		const explicit = this.peek().type === T.LPAREN && !this.peek().spaceBefore;
		if (key) {
			if (CONSTANTS.has(key)) return { type: 'const', key, line: t.line };
			if (FUNCTIONS[key] !== undefined) {
				const allowSpacedParen = key === 'RESUB' || key === 'RESEARCH' || key === 'REFINDALL';
				if (explicit || (allowSpacedParen && this.peek().type === T.LPAREN)) {
					return { type: 'call', key, args: this.parseArgList(), explicit: true, line: t.line };
				}
				if (key === 'RANGE') {
					const args = [];
					this.argDepth++;
					try {
					while (args.length < 3 && this.startsExpression(this.peek()) && !(this.peek().type === T.LBRACK)) {
						args.push(this.parseExpr());
					}
					} finally {
						this.argDepth--;
					}
					if (args.length === 0) this.error(this.locale.message('ERR_ARGUMENTS', this.displayName(t), 1, 0), t, 'arguments');
					return { type: 'call', key, args, explicit: false, line: t.line };
				}
				return { type: 'call', key, args: this.parsePrefixArgs(t, FUNCTIONS[key]), explicit: false, line: t.line };
			}
			if (GETTERS.has(key)) {
				// "POSITION [10, 10]" inside an expression is still a setter
				// followed by a list index in LibreLogo.py: position()[...] is
				// handled by parsePostfix on the getter node.
				return { type: 'getter', key, line: t.line };
			}
			if (COMMANDS[key] !== undefined) {
				this.error('the command ' + this.displayName(t) + ' has no value', t);
			}
			this.error('unexpected ' + this.displayName(t), t);
		}
		const proc = this.procs.get(t.value);
		if (proc) {
			if (explicit) return { type: 'proc', name: t.value, args: this.parseArgList(), explicit: true, line: t.line };
			return { type: 'proc', name: t.value, args: this.parsePrefixArgs(t, proc.params.length), explicit: false, line: t.line };
		}
		if (explicit) this.error(this.locale.message('ERR_NAME', t.value), t, 'name');
		return { type: 'var', name: t.value, line: t.line };
	}
}

export function parse(tokens, locale) {
	return new Parser(tokens, locale).parseProgram();
}
