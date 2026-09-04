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

// Tree-walking evaluator for LibreLogo programs. Execution is a generator:
// the driver (worker.js) pulls steps and services the requests the program
// yields - a PRINT line, an INPUT prompt, a SLEEP pause, or a periodic step
// notification - so a long program never blocks the host and can be stopped.

import { tokenize, LogoSyntaxError } from './lexer.js';
import { parse, FUNCTIONS } from './parser.js';
import {
	LogoError, Tuple, truthy, compare, binop, negate, index, setIndex, slice,
	seqItems, pyRound, formatValue, isSequence,
} from './values.js';

const STEPS_PER_YIELD = 2000;
const MAX_DEPTH = 400;

class BreakSignal {}
class ContinueSignal {}
class ReturnSignal { constructor(value) { this.value = value; } }
export class HaltSignal {}

class Scope {
	constructor(parent, isGlobal) {
		this.vars = new Map();
		this.parent = parent; // the global scope for procedure scopes, null for the global scope
		this.isGlobal = isGlobal;
		this.globals = new Set();
	}
}

// Python-compatible regular expressions for SUB, SEARCH and FINDALL.
function pyRegex(pattern, extraFlags) {
	let flags = 'u' + (extraFlags || '');
	let p = String(pattern);
	const inline = /^\(\?([aiLmsux]+)\)/.exec(p);
	if (inline) {
		p = p.slice(inline[0].length);
		if (inline[1].includes('i') && !flags.includes('i')) flags += 'i';
		if (inline[1].includes('m') && !flags.includes('m')) flags += 'm';
		if (inline[1].includes('s') && !flags.includes('s')) flags += 's';
	}
	p = p.replace(/\(\?P<([^>]+)>/g, '(?<$1>').replace(/\(\?P=([^)]+)\)/g, '\\k<$1>');
	try {
		return new RegExp(p, flags);
	} catch (e) {
		throw new LogoError('ERROR');
	}
}

function pyReplacement(repl) {
	// \1 and \g<1> group references become $1; a literal $ must be doubled.
	return String(repl).replace(/\$/g, '$$$$').replace(/\\g<(\d+)>/g, '$$$1').replace(/\\(\d)/g, '$$$1');
}

export class Interpreter {
	// turtle: a Turtle from turtle.js; io: { print(text), random() }
	constructor(locale, turtle, io) {
		this.locale = locale;
		this.turtle = turtle;
		this.io = io || {};
		this.random = this.io.random || Math.random;
		this.global = new Scope(null, true);
		this.procs = new Map();
		this.steps = 0;
		this.depth = 0;
		this.repcounts = [];
	}

	// Parse `source` and return the program; syntax errors carry a line number.
	compile(source) {
		const tokens = tokenize(source, this.locale);
		const program = parse(tokens, this.locale);
		for (const [name, proc] of program.procs) {
			if (!proc.body) throw new LogoSyntaxError(this.locale.message('ERR_BLOCK'), proc.line, 'block');
			this.procs.set(name, proc);
		}
		return program;
	}

	// Run a program; a generator, see the module comment.
	*run(program) {
		this.steps = 0;
		this.repcounts = [];
		try {
			yield* this.execBlock(program.body, this.global);
		} catch (e) {
			if (e instanceof ReturnSignal) return;
			if (e instanceof BreakSignal || e instanceof ContinueSignal) {
				throw this.withLine(new LogoError('ERROR'), e.line);
			}
			throw e;
		}
	}

	withLine(err, line) {
		if (err && err.line === null || err.line === undefined) err.line = line;
		return err;
	}

	*tick(line) {
		this.steps++;
		if (this.steps % STEPS_PER_YIELD === 0) yield { type: 'step', steps: this.steps, line };
	}

	// --- statements ------------------------------------------------------

	*execBlock(stmts, scope) {
		for (const stmt of stmts) {
			yield* this.tick(stmt.line);
			try {
				yield* this.exec(stmt, scope);
			} catch (e) {
				if (e instanceof LogoError || e instanceof LogoSyntaxError) {
					if (e.line === null || e.line === undefined) e.line = stmt.line;
				} else if (e instanceof RangeError && /call stack/i.test(e.message)) {
					throw this.withLine(new LogoError('ERR_MAXRECURSION', [MAX_DEPTH]), stmt.line);
				} else if (e instanceof BreakSignal || e instanceof ContinueSignal) {
					if (e.line === undefined) e.line = stmt.line;
				}
				throw e;
			}
		}
	}

	*exec(stmt, scope) {
		switch (stmt.type) {
		case 'expr':
			yield* this.eval(stmt.expr, scope);
			return;
		case 'assign': {
			let value = yield* this.eval(stmt.value, scope);
			if (stmt.target.type === 'var') {
				if (stmt.op) {
					const cur = this.lookup(scope, stmt.target.name, stmt.line);
					// "list += seq" extends the existing list in place, as in
					// Python, so a list passed into a procedure grows for the
					// caller too. Every other case computes a new value.
					if (stmt.op === '+' && Array.isArray(cur)) {
						if (!isSequence(value)) throw this.withLine(new LogoError('ERROR'), stmt.line);
						for (const item of seqItems(value)) cur.push(item);
						return;
					}
					value = binop(stmt.op, cur, value);
				}
				this.assign(scope, stmt.target.name, value);
			} else {
				const obj = yield* this.eval(stmt.target.obj, scope);
				const i = yield* this.eval(stmt.target.index, scope);
				if (stmt.op) {
					const cur = index(obj, i);
					if (stmt.op === '+' && Array.isArray(cur)) {
						if (!isSequence(value)) throw this.withLine(new LogoError('ERROR'), stmt.line);
						for (const item of seqItems(value)) cur.push(item);
						return;
					}
					value = binop(stmt.op, cur, value);
				}
				setIndex(obj, i, value);
			}
			return;
		}
		case 'command':
			yield* this.command(stmt, scope);
			return;
		case 'repeat': {
			let count = Infinity;
			if (stmt.count) {
				const c = yield* this.eval(stmt.count, scope);
				if (typeof c !== 'number') throw new LogoError('ERROR');
				count = Math.trunc(c);
			}
			yield* this.loop(count === Infinity ? null : count, stmt.body, scope, null);
			return;
		}
		case 'while':
			yield* this.loop(null, stmt.body, scope, stmt.cond);
			return;
		case 'for': {
			const iterable = yield* this.eval(stmt.iter, scope);
			const items = iterable instanceof Set ? Array.from(iterable) : seqItems(iterable).slice();
			this.repcounts.push(0);
			try {
				for (let i = 0; i < items.length; i++) {
					this.repcounts[this.repcounts.length - 1] = i + 1;
					this.assign(scope, stmt.name, items[i]);
					try {
						yield* this.execBlock(stmt.body, scope);
					} catch (e) {
						if (e instanceof BreakSignal) break;
						if (e instanceof ContinueSignal) continue;
						throw e;
					}
				}
			} finally {
				this.repcounts.pop();
			}
			return;
		}
		case 'if': {
			const c = yield* this.eval(stmt.cond, scope);
			if (truthy(c)) yield* this.execBlock(stmt.then, scope);
			else if (stmt.otherwise) yield* this.execBlock(stmt.otherwise, scope);
			return;
		}
		case 'break': throw new BreakSignal();
		case 'continue': throw new ContinueSignal();
		case 'stop': throw new ReturnSignal(null);
		case 'output': {
			const v = yield* this.eval(stmt.value, scope);
			throw new ReturnSignal(v);
		}
		case 'global':
			for (const n of stmt.names) scope.globals.add(n);
			return;
		case 'picture': {
			if (!stmt.body) {
				this.turtle.newShape();
				return;
			}
			const name = stmt.name ? yield* this.eval(stmt.name, scope) : '';
			this.turtle.pictureStart(name);
			// The block runs twice in LibreLogo.py only to make REPCOUNT work
			// inside PICTURE; here it is a plain block.
			let shape = null;
			try {
				yield* this.execBlock(stmt.body, scope);
			} finally {
				shape = this.turtle.pictureEnd(name);
			}
			// A named ".svg" picture is also exported as an SVG file, as in
			// desktop LibreLogo; the shape stays in the drawing as well.
			if (shape && typeof name === 'string' && /\.svg\s*$/i.test(name)) {
				yield { type: 'export', name: name.trim(), shape };
			}
			return;
		}
		default:
			throw new LogoError('ERROR');
		}
	}

	*loop(count, body, scope, cond) {
		this.repcounts.push(0);
		try {
			for (let i = 0; count === null || i < count; i++) {
				this.repcounts[this.repcounts.length - 1] = i + 1;
				if (cond) {
					const c = yield* this.eval(cond, scope);
					if (!truthy(c)) break;
				}
				try {
					yield* this.execBlock(body, scope);
				} catch (e) {
					if (e instanceof BreakSignal) break;
					if (e instanceof ContinueSignal) continue;
					throw e;
				}
				yield* this.tick();
			}
		} finally {
			this.repcounts.pop();
		}
	}

	assign(scope, name, value) {
		if (!scope.isGlobal && scope.globals.has(name)) this.global.vars.set(name, value);
		else scope.vars.set(name, value);
	}

	lookup(scope, name, line) {
		if (scope.vars.has(name)) return scope.vars.get(name);
		if (!scope.isGlobal && this.global.vars.has(name)) return this.global.vars.get(name);
		throw this.withLine(new LogoError('ERR_NAME', [name]), line);
	}

	// --- commands --------------------------------------------------------

	*command(stmt, scope) {
		const t = this.turtle;
		const args = [];
		for (const a of stmt.args) args.push(yield* this.eval(a, scope));
		const a = args[0];
		switch (stmt.key) {
		case 'FORWARD': t.forward(a); break;
		case 'BACKWARD': t.backward(a); break;
		case 'TURNLEFT': t.left(a); break;
		case 'TURNRIGHT': t.right(a); break;
		case 'PENUP': t.penup(); break;
		case 'PENDOWN': t.pendown(); break;
		case 'HOME': t.home(); break;
		case 'POINT': t.point(); break;
		case 'CIRCLE': case 'ELLIPSE': t.ellipse(a); break;
		case 'SQUARE': case 'RECTANGLE': t.rectangle(a); break;
		case 'LABEL': t.label(this.asText(a)); break;
		case 'TEXT': t.text(this.asText(a)); break;
		case 'PENCOLOR': t.setPencolor(a); break;
		case 'PENWIDTH': t.setPensize(a); break;
		case 'PENSTYLE': t.setPenstyle(a); break;
		case 'PENJOINT': t.setPenjoint(a); break;
		case 'PENCAP': t.setPencap(a); break;
		case 'CLOSE': t.fill(false); break;
		case 'FILL': t.fill(true); break;
		case 'FILLCOLOR': t.setFillcolor(a); break;
		case 'FILLTRANSPARENCY': t.setFilltransparency(a); break;
		case 'PENTRANSPARENCY': t.setPentransparency(a); break;
		case 'FILLSTYLE': t.setFillstyle(a); break;
		case 'FONTCOLOR': t.setFontcolor(a); break;
		case 'FONTTRANSPARENCY': t.setFonttransparency(a); break;
		case 'FONTHEIGHT': t.setFontheight(a); break;
		case 'FONTWEIGHT': t.setFontweight(a); break;
		case 'FONTSTYLE': t.setFontstyle(a); break;
		case 'FONTFAMILY': t.setFontfamily(a); break;
		case 'CLEARSCREEN': t.clearscreen(); break;
		case 'HIDETURTLE': t.hideturtle(); break;
		case 'SHOWTURTLE': t.showturtle(); break;
		case 'POSITION': t.setPosition(a); break;
		case 'HEADING': t.setHeading(a); break;
		case 'PRINT':
			yield { type: 'print', text: formatValue(a, this.locale) };
			break;
		case 'SLEEP': {
			if (typeof a !== 'number') throw new LogoError('ERROR');
			t.sleep(a);
			if (a > 0) yield { type: 'sleep', ms: a };
			break;
		}
		default:
			throw new LogoError('ERROR');
		}
	}

	// LABEL/TEXT arguments: a string, or [dx, dy, string]; numbers print as text.
	asText(v) {
		if (Array.isArray(v) && v.length === 3) return [v[0], v[1], formatValue(v[2], this.locale)];
		if (typeof v === 'string') return v;
		return formatValue(v, this.locale);
	}

	// --- expressions -----------------------------------------------------

	*eval(node, scope) {
		switch (node.type) {
		case 'num': return node.value;
		case 'str': return node.value;
		case 'list': {
			const out = [];
			for (const item of node.items) out.push(yield* this.eval(item, scope));
			return out;
		}
		case 'tuple': {
			const out = [];
			for (const item of node.items) out.push(yield* this.eval(item, scope));
			return new Tuple(out);
		}
		case 'var': return this.lookup(scope, node.name, node.line);
		case 'const':
			switch (node.key) {
			case 'TRUE': return true;
			case 'FALSE': return false;
			case 'ANY': return 'any';
			case 'PI': return Math.PI;
			case 'REPCOUNT':
				if (this.repcounts.length === 0) throw this.withLine(new LogoError('ERR_NAME', [this.locale.localName('REPCOUNT')]), node.line);
				return this.repcounts[this.repcounts.length - 1];
			default: throw new LogoError('ERROR');
			}
		case 'getter': return this.getter(node.key);
		case 'call': {
			const args = [];
			for (const a of node.args) args.push(yield* this.eval(a, scope));
			return yield* this.callFunction(node, args);
		}
		case 'proc': {
			const args = [];
			for (const a of node.args) args.push(yield* this.eval(a, scope));
			return yield* this.callProcedure(node.name, args, node.line);
		}
		case 'binop': {
			const l = yield* this.eval(node.left, scope);
			const r = yield* this.eval(node.right, scope);
			return binop(node.op, l, r);
		}
		case 'unop': {
			const v = yield* this.eval(node.operand, scope);
			return node.op === '-' ? negate(v) : v;
		}
		case 'not': return !truthy(yield* this.eval(node.operand, scope));
		case 'and': {
			const l = yield* this.eval(node.left, scope);
			if (!truthy(l)) return l;
			return yield* this.eval(node.right, scope);
		}
		case 'or': {
			const l = yield* this.eval(node.left, scope);
			if (truthy(l)) return l;
			return yield* this.eval(node.right, scope);
		}
		case 'compare': {
			let left = yield* this.eval(node.operands[0], scope);
			for (let i = 0; i < node.ops.length; i++) {
				const right = yield* this.eval(node.operands[i + 1], scope);
				if (!compare(node.ops[i], left, right)) return false;
				left = right;
			}
			return true;
		}
		case 'index': {
			const obj = yield* this.eval(node.obj, scope);
			const i = yield* this.eval(node.index, scope);
			return index(obj, i);
		}
		case 'slice': {
			const obj = yield* this.eval(node.obj, scope);
			const lo = node.lo ? yield* this.eval(node.lo, scope) : null;
			const hi = node.hi ? yield* this.eval(node.hi, scope) : null;
			return slice(obj, lo, hi);
		}
		default:
			throw new LogoError('ERROR');
		}
	}

	getter(key) {
		const t = this.turtle;
		switch (key) {
		case 'PENCOLOR': return t.getPencolor();
		case 'PENWIDTH': return t.getPensize();
		case 'PENSTYLE': return t.getPenstyle();
		case 'PENJOINT': return t.getPenjoint();
		case 'PENCAP': return t.getPencap();
		case 'FILLCOLOR': return t.getFillcolor();
		case 'FILLTRANSPARENCY': return t.getFilltransparency();
		case 'PENTRANSPARENCY': return t.getPentransparency();
		case 'FONTCOLOR': return t.getFontcolor();
		case 'FONTTRANSPARENCY': return t.getFonttransparency();
		case 'FONTHEIGHT': return t.getFontheight();
		case 'FONTWEIGHT': return t.getFontweight();
		case 'FONTSTYLE': return t.getFontstyle();
		case 'FONTFAMILY': return t.getFontfamily();
		case 'POSITION': return t.getPosition();
		case 'HEADING': return t.getHeading();
		case 'PAGESIZE': return t.pagesize();
		default: throw new LogoError('ERROR');
		}
	}

	*callProcedure(name, args, line) {
		const proc = this.procs.get(name);
		if (!proc) throw this.withLine(new LogoError('ERR_NAME', [name]), line);
		if (args.length !== proc.params.length) {
			throw this.withLine(new LogoError('ERR_ARGUMENTS', [name, proc.params.length, args.length]), line);
		}
		if (this.depth >= MAX_DEPTH) throw this.withLine(new LogoError('ERR_MAXRECURSION', [MAX_DEPTH]), line);
		const scope = new Scope(this.global, false);
		for (let i = 0; i < args.length; i++) scope.vars.set(proc.params[i], args[i]);
		this.depth++;
		try {
			yield* this.execBlock(proc.body, scope);
		} catch (e) {
			if (e instanceof ReturnSignal) return e.value;
			throw e;
		} finally {
			this.depth--;
		}
		return null;
	}

	argCheck(node, args, min, max) {
		if (args.length < min || args.length > (max === undefined ? min : max)) {
			throw this.withLine(new LogoError('ERR_ARGUMENTS', [this.locale.localName(node.key).toUpperCase(), min, args.length]), node.line);
		}
	}

	num(v) {
		if (typeof v === 'number') return v;
		if (typeof v === 'boolean') return Number(v);
		throw new LogoError('ERROR');
	}

	// int("10cm") and float("10,5cm") understand numbers with units, like the
	// program text does.
	parseNumber(s, integer) {
		const tokens = tokenize(String(s).trim(), this.locale);
		let sign = 1, i = 0;
		if (tokens[i] && tokens[i].type === 'op' && (tokens[i].value === '-' || tokens[i].value === '+')) {
			if (tokens[i].value === '-') sign = -1;
			i++;
		}
		if (!tokens[i] || tokens[i].type !== 'num' || tokens[i + 1].type !== 'eof') throw new LogoError('ERROR');
		const v = sign * tokens[i].value;
		return integer ? Math.trunc(v) : v;
	}

	*callFunction(node, args) {
		const key = node.key;
		const a = args[0];
		switch (key) {
		case 'RANDOM':
			this.argCheck(node, args, 1);
			if (typeof a === 'number') return a * this.random();
			if (isSequence(a) || a instanceof Set) {
				const items = seqItems(a);
				if (items.length === 0) throw new LogoError('ERR_INDEX');
				return items[Math.floor(this.random() * items.length)];
			}
			throw new LogoError('ERROR');
		case 'INT':
			this.argCheck(node, args, 1);
			if (typeof a === 'string') return this.parseNumber(a, true);
			return Math.trunc(this.num(a));
		case 'FLOAT':
			this.argCheck(node, args, 1);
			if (typeof a === 'string') return this.parseNumber(a, false);
			return this.num(a);
		case 'STR':
			this.argCheck(node, args, 1);
			return formatValue(a, this.locale);
		case 'SQRT': this.argCheck(node, args, 1); return Math.sqrt(this.num(a));
		case 'LOG10': this.argCheck(node, args, 1); return Math.log10(this.num(a));
		case 'SIN': this.argCheck(node, args, 1); return Math.sin(this.num(a));
		case 'COS': this.argCheck(node, args, 1); return Math.cos(this.num(a));
		case 'ROUND':
			this.argCheck(node, args, 1, 2);
			return pyRound(this.num(a), args.length > 1 ? this.num(args[1]) : undefined);
		case 'ABS': this.argCheck(node, args, 1); return Math.abs(this.num(a));
		case 'COUNT':
			this.argCheck(node, args, 1);
			if (a instanceof Set) return a.size;
			return seqItems(a).length;
		case 'SET': {
			this.argCheck(node, args, 0, 1);
			const out = new Set();
			if (args.length) for (const v of seqItems(a)) { if (![...out].some((w) => compare('==', w, v))) out.add(v); }
			return out;
		}
		case 'RANGE': {
			this.argCheck(node, args, 1, 3);
			let start = 0, stop, step = 1;
			if (args.length === 1) stop = this.num(a);
			else { start = this.num(a); stop = this.num(args[1]); if (args.length === 3) step = this.num(args[2]); }
			if (step === 0) throw new LogoError('ERROR');
			const out = [];
			for (let v = start; step > 0 ? v < stop : v > stop; v += step) {
				out.push(v);
				if (out.length > 10000000) throw new LogoError('ERR_MEMORY');
			}
			return out;
		}
		case 'LIST':
			this.argCheck(node, args, 0, 1);
			return args.length ? seqItems(a).slice() : [];
		case 'TUPLE':
			this.argCheck(node, args, 0, 1);
			return new Tuple(args.length ? seqItems(a) : []);
		case 'SORTED': {
			this.argCheck(node, args, 1);
			const items = seqItems(a).slice();
			items.sort((x, y) => (compare('<', x, y) ? -1 : compare('>', x, y) ? 1 : 0));
			return items;
		}
		case 'RESUB': {
			this.argCheck(node, args, 3);
			return String(args[2]).replace(pyRegex(a, 'g'), pyReplacement(args[1]));
		}
		case 'RESEARCH': {
			this.argCheck(node, args, 2);
			const m = pyRegex(a).exec(String(args[1]));
			return m ? m[0] : null;
		}
		case 'REFINDALL': {
			this.argCheck(node, args, 2);
			const re = pyRegex(a, 'g');
			const out = [];
			for (const m of String(args[1]).matchAll(re)) {
				if (m.length === 1) out.push(m[0]);
				else if (m.length === 2) out.push(m[1] === undefined ? '' : m[1]);
				else out.push(new Tuple(m.slice(1).map((g) => (g === undefined ? '' : g))));
			}
			return out;
		}
		case 'MIN': case 'MAX': {
			this.argCheck(node, args, 1, 1000);
			const items = args.length === 1 ? seqItems(a) : args;
			if (items.length === 0) throw new LogoError('ERROR');
			let best = items[0];
			for (const v of items) if (compare(key === 'MIN' ? '<' : '>', v, best)) best = v;
			return best;
		}
		case 'INPUT': {
			this.argCheck(node, args, 1);
			const prompt = Array.isArray(a) ? formatValue(a[0], this.locale) : formatValue(a, this.locale);
			const answer = yield { type: 'input', prompt };
			if (answer === null || answer === undefined) throw new HaltSignal();
			return String(answer);
		}
		default:
			if (FUNCTIONS[key] === undefined) throw new LogoError('ERROR');
			throw new LogoError('ERROR');
		}
	}
}
