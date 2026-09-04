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

// LibreLogo values follow Python: numbers, strings, lists, tuples, sets and
// booleans, with Python's operators. This module implements the subset a Logo
// program meets: arithmetic, comparison, membership, indexing and slicing,
// sequence repetition and set algebra, plus Python-style text formatting.

export class LogoError extends Error {
	constructor(kind, args) {
		super(kind);
		this.name = 'LogoError';
		this.kind = kind; // ERR_ZERODIVISION, ERR_INDEX, ERR_KEY, ERR_NAME, ERR_ARGUMENTS, ERROR
		this.args = args || [];
		this.line = null;
	}
}

// Python tuples are immutable lists; a frozen array stands in for them.
export class Tuple {
	constructor(items) {
		this.items = Object.freeze(items.slice());
	}
}

export function isList(v) {
	return Array.isArray(v);
}

export function isTuple(v) {
	return v instanceof Tuple;
}

export function isSequence(v) {
	return Array.isArray(v) || v instanceof Tuple || typeof v === 'string';
}

export function seqItems(v) {
	if (Array.isArray(v)) return v;
	if (v instanceof Tuple) return v.items;
	if (typeof v === 'string') return Array.from(v);
	if (v instanceof Set) return Array.from(v);
	throw new LogoError('ERROR');
}

export function truthy(v) {
	if (v === null || v === undefined || v === false) return false;
	if (typeof v === 'number') return v !== 0;
	if (typeof v === 'string') return v.length > 0;
	if (Array.isArray(v)) return v.length > 0;
	if (v instanceof Tuple) return v.items.length > 0;
	if (v instanceof Set) return v.size > 0;
	return true;
}

export function equals(a, b) {
	if (a === b) return true;
	if (typeof a === 'number' && typeof b === 'boolean') return a === Number(b);
	if (typeof a === 'boolean' && typeof b === 'number') return Number(a) === b;
	if (isSequence(a) && isSequence(b) && typeof a !== 'string' && typeof b !== 'string') {
		const x = seqItems(a), y = seqItems(b);
		if (x.length !== y.length) return false;
		if (Array.isArray(a) !== Array.isArray(b)) return false;
		for (let i = 0; i < x.length; i++) if (!equals(x[i], y[i])) return false;
		return true;
	}
	if (a instanceof Set && b instanceof Set) {
		if (a.size !== b.size) return false;
		for (const v of a) if (!setHas(b, v)) return false;
		return true;
	}
	return false;
}

function setHas(set, v) {
	if (set.has(v)) return true;
	for (const w of set) if (equals(v, w)) return true;
	return false;
}

export function compare(op, a, b) {
	if (op === '==') return equals(a, b);
	if (op === '!=') return !equals(a, b);
	if (op === 'in') return contains(b, a);
	if (op === 'not in') return !contains(b, a);
	let c;
	if (typeof a === 'number' && typeof b === 'number') c = a - b;
	else if (typeof a === 'string' && typeof b === 'string') c = a < b ? -1 : a > b ? 1 : 0;
	else if (typeof a === 'boolean' || typeof b === 'boolean') c = Number(a) - Number(b);
	else if (isSequence(a) && isSequence(b)) {
		const x = seqItems(a), y = seqItems(b);
		c = 0;
		for (let i = 0; i < Math.min(x.length, y.length) && c === 0; i++) {
			if (!equals(x[i], y[i])) c = compare('<', x[i], y[i]) ? -1 : 1;
		}
		if (c === 0) c = x.length - y.length;
	} else throw new LogoError('ERROR');
	switch (op) {
	case '<': return c < 0;
	case '>': return c > 0;
	case '<=': return c <= 0;
	case '>=': return c >= 0;
	default: throw new LogoError('ERROR');
	}
}

export function contains(container, item) {
	if (typeof container === 'string') {
		if (typeof item !== 'string') throw new LogoError('ERROR');
		return container.includes(item);
	}
	if (container instanceof Set) return setHas(container, item);
	for (const v of seqItems(container)) if (equals(v, item)) return true;
	return false;
}

function pyMod(a, b) {
	if (b === 0) throw new LogoError('ERR_ZERODIVISION');
	const m = a % b;
	return (m !== 0 && (m < 0) !== (b < 0)) ? m + b : m;
}

function repeat(seq, n) {
	n = Math.trunc(n);
	if (typeof seq === 'string') return n > 0 ? seq.repeat(n) : '';
	const items = seqItems(seq);
	const out = [];
	for (let i = 0; i < n; i++) out.push(...items);
	return seq instanceof Tuple ? new Tuple(out) : out;
}

export function binop(op, a, b) {
	const num = typeof a === 'number' && typeof b === 'number';
	if (typeof a === 'boolean' && (typeof b === 'number' || typeof b === 'boolean')) a = Number(a);
	if (typeof b === 'boolean' && typeof a === 'number') b = Number(b);
	switch (op) {
	case '+':
		if (typeof a === 'number' && typeof b === 'number') return a + b;
		if (typeof a === 'string' && typeof b === 'string') return a + b;
		if (Array.isArray(a) && Array.isArray(b)) return a.concat(b);
		if (a instanceof Tuple && b instanceof Tuple) return new Tuple(a.items.concat(b.items));
		break;
	case '-':
		if (typeof a === 'number' && typeof b === 'number') return a - b;
		if (a instanceof Set && b instanceof Set) return new Set([...a].filter((v) => !setHas(b, v)));
		break;
	case '*':
		if (typeof a === 'number' && typeof b === 'number') return a * b;
		if (typeof a === 'number' && isSequence(b)) return repeat(b, a);
		if (typeof b === 'number' && isSequence(a)) return repeat(a, b);
		break;
	case '/':
		if (typeof a === 'number' && typeof b === 'number') {
			if (b === 0) throw new LogoError('ERR_ZERODIVISION');
			return a / b;
		}
		break;
	case '//':
		if (typeof a === 'number' && typeof b === 'number') {
			if (b === 0) throw new LogoError('ERR_ZERODIVISION');
			return Math.floor(a / b);
		}
		break;
	case '%':
		if (typeof a === 'number' && typeof b === 'number') return pyMod(a, b);
		if (typeof a === 'string') return pyFormat(a, b);
		break;
	case '**':
		if (typeof a === 'number' && typeof b === 'number') return Math.pow(a, b);
		break;
	case '|':
		if (a instanceof Set && b instanceof Set) return new Set([...a, ...[...b].filter((v) => !setHas(a, v))]);
		if (num) return a | b;
		break;
	case '&':
		if (a instanceof Set && b instanceof Set) return new Set([...a].filter((v) => setHas(b, v)));
		if (num) return a & b;
		break;
	case '^':
		if (a instanceof Set && b instanceof Set) {
			return new Set([...[...a].filter((v) => !setHas(b, v)), ...[...b].filter((v) => !setHas(a, v))]);
		}
		if (num) return a ^ b;
		break;
	default:
		break;
	}
	throw new LogoError('ERROR');
}

// Minimal "%s" / "%d" / "%f" formatting for string % value.
function pyFormat(fmt, value) {
	const args = value instanceof Tuple ? value.items : [value];
	let i = 0;
	return fmt.replace(/%([sdif])/g, (m, kind) => {
		const v = args[i++];
		if (kind === 's') return formatValue(v);
		if (kind === 'd' || kind === 'i') return String(Math.trunc(Number(v)));
		return String(Number(v));
	});
}

export function negate(v) {
	if (typeof v === 'number') return -v;
	if (typeof v === 'boolean') return -Number(v);
	throw new LogoError('ERROR');
}

function normIndex(i, length) {
	if (typeof i !== 'number' || !Number.isInteger(i)) throw new LogoError('ERR_INDEX');
	if (i < 0) i += length;
	if (i < 0 || i >= length) throw new LogoError('ERR_INDEX');
	return i;
}

export function index(obj, i) {
	if (typeof obj === 'string') return obj[normIndex(i, obj.length)];
	if (Array.isArray(obj)) return obj[normIndex(i, obj.length)];
	if (obj instanceof Tuple) return obj.items[normIndex(i, obj.items.length)];
	throw new LogoError('ERR_KEY', [formatValue(i)]);
}

export function setIndex(obj, i, value) {
	if (!Array.isArray(obj)) throw new LogoError('ERROR');
	obj[normIndex(i, obj.length)] = value;
}

export function slice(obj, lo, hi) {
	const items = typeof obj === 'string' ? obj : seqItems(obj);
	const n = items.length;
	const clamp = (v, dflt) => {
		if (v === null || v === undefined) return dflt;
		if (typeof v !== 'number') throw new LogoError('ERR_INDEX');
		v = Math.trunc(v);
		if (v < 0) v += n;
		return Math.max(0, Math.min(n, v));
	};
	const a = clamp(lo, 0), b = clamp(hi, n);
	const out = a < b ? items.slice(a, b) : items.slice(0, 0);
	if (obj instanceof Tuple) return new Tuple(out);
	return out;
}

// Python's round(): half to even, returning an integer.
export function pyRound(x, digits) {
	if (digits === undefined || digits === null) {
		const r = Math.round(x);
		if (Math.abs(x - Math.trunc(x)) === 0.5 && r % 2 !== 0) return r - Math.sign(x);
		return r;
	}
	const f = Math.pow(10, digits);
	return Math.round(x * f) / f;
}

// str() as LibreLogo prints values: Python repr for containers with the
// program's quotes, localized booleans and the language's decimal sign.
export function formatValue(v, locale) {
	const decimal = locale ? locale.decimal : '.';
	const fmt = (x, nested) => {
		if (x === null || x === undefined) return 'None';
		if (typeof x === 'boolean') return locale ? locale.localName(x ? 'TRUE' : 'FALSE') : String(x);
		if (typeof x === 'number') {
			if (!Number.isFinite(x)) return x > 0 ? 'inf' : x < 0 ? '-inf' : 'nan';
			let s = Number.isInteger(x) ? String(x) : String(parseFloat(x.toPrecision(12)));
			if (decimal === ',') s = s.replace('.', ',');
			return s;
		}
		if (typeof x === 'string') return nested ? "'" + x.replace(/'/g, "\\'") + "'" : x;
		if (Array.isArray(x)) return '[' + x.map((y) => fmt(y, true)).join(', ') + ']';
		if (x instanceof Tuple) {
			if (x.items.length === 1) return '(' + fmt(x.items[0], true) + ',)';
			return '(' + x.items.map((y) => fmt(y, true)).join(', ') + ')';
		}
		if (x instanceof Set) {
			if (x.size === 0) return (locale ? locale.localName('SET') : 'set') + '()';
			// Python shows small integers and strings in sorted order; do the same.
			const items = [...x];
			if (items.every((y) => typeof y === 'number') || items.every((y) => typeof y === 'string')) {
				items.sort((p, q) => (p < q ? -1 : p > q ? 1 : 0));
			}
			return '{' + items.map((y) => fmt(y, true)).join(', ') + '}';
		}
		return String(x);
	};
	return fmt(v, false);
}
