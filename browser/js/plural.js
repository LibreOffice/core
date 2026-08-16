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
 * Plural-aware translation:
 *
 *     _n('%n slide', '%n slides', count)
 *
 * English picks between two forms on count === 1, but Russian has three forms
 * and Arabic six, so a call site must never choose the form itself. _n() hands
 * both English forms to the translator as a single gettext plural entry, picks
 * the form the target language asks for, and substitutes the count for every
 * %n in it. Any other placeholder (%1, {0}) stays for the caller to fill in.
 *
 * The forms live in the same per-language table as every other string, keyed
 * the way a MO file keys a plural entry: the two source strings joined by a
 * NUL, mapping to the translated forms joined by NUL. The language's
 * Plural-Forms header travels along under the reserved NUL key, so the very
 * expression the translators saw in Weblate decides the form here. Both are
 * written by util/po2json.py.
 *
 * The expression is a small C one, e.g.
 *
 *     nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 ... ? 1 : 2);
 *
 * and is parsed into a tree of closures below rather than handed to Function()
 * or eval(), which the CSP a picky integrator may set would refuse to run.
 */

/* global global */

(function (global) {
	'use strict';

	// A NUL can never occur in a msgid, so it separates the parts of a plural
	// entry and marks the key the plural rule itself is stored under.
	const SEP = '\u0000';
	const RULE_KEY = SEP + 'plural-forms';

	// What the English sources themselves use: two forms, "one" is exactly 1.
	const DEFAULT_RULE = {
		nplurals: 2,
		select: function (n) {
			return n === 1 ? 0 : 1;
		},
	};

	// Parsed rules, keyed by the Plural-Forms header they came from.
	const ruleCache = {};

	const TWO_CHAR_OPERATORS = ['&&', '||', '==', '!=', '<=', '>='];
	const ONE_CHAR_OPERATORS = '()?:<>!%*/+-';

	function isDigit(c) {
		return c >= '0' && c <= '9';
	}

	function tokenize(expression) {
		const tokens = [];
		let i = 0;

		while (i < expression.length) {
			const c = expression.charAt(i);
			const two = expression.substring(i, i + 2);

			if (c === ' ' || c === '\t' || c === '\n' || c === '\r') {
				i++;
			} else if (isDigit(c)) {
				let end = i;
				while (end < expression.length && isDigit(expression.charAt(end)))
					end++;
				tokens.push({
					type: 'number',
					value: parseInt(expression.substring(i, end), 10),
				});
				i = end;
			} else if (c === 'n') {
				tokens.push({ type: 'n' });
				i++;
			} else if (TWO_CHAR_OPERATORS.indexOf(two) !== -1) {
				tokens.push({ type: two });
				i += 2;
			} else if (ONE_CHAR_OPERATORS.indexOf(c) !== -1) {
				tokens.push({ type: c });
				i++;
			} else {
				// Anything else means we misread the header - refuse the whole
				// expression rather than guess at what it meant.
				return null;
			}
		}

		return tokens;
	}

	// C truncates towards zero on integer division, JavaScript does not.
	function divide(a, b) {
		const quotient = a / b;
		return quotient < 0 ? Math.ceil(quotient) : Math.floor(quotient);
	}

	function combine(operator, left, right) {
		switch (operator) {
			case '*':
				return function (n) {
					return left(n) * right(n);
				};
			case '/':
				return function (n) {
					return divide(left(n), right(n));
				};
			case '%':
				return function (n) {
					return left(n) % right(n);
				};
			case '+':
				return function (n) {
					return left(n) + right(n);
				};
			case '-':
				return function (n) {
					return left(n) - right(n);
				};
			case '<':
				return function (n) {
					return left(n) < right(n) ? 1 : 0;
				};
			case '>':
				return function (n) {
					return left(n) > right(n) ? 1 : 0;
				};
			case '<=':
				return function (n) {
					return left(n) <= right(n) ? 1 : 0;
				};
			case '>=':
				return function (n) {
					return left(n) >= right(n) ? 1 : 0;
				};
			case '==':
				return function (n) {
					return left(n) === right(n) ? 1 : 0;
				};
			case '!=':
				return function (n) {
					return left(n) !== right(n) ? 1 : 0;
				};
			case '&&':
				return function (n) {
					return left(n) && right(n) ? 1 : 0;
				};
			default:
				return function (n) {
					return left(n) || right(n) ? 1 : 0;
				};
		}
	}

	// Recursive descent over the operators C gives us, loosest binding first.
	function compile(tokens) {
		let pos = 0;

		function peek() {
			return pos < tokens.length ? tokens[pos].type : null;
		}

		function accept(type) {
			if (peek() !== type) return false;
			pos++;
			return true;
		}

		function expect(type) {
			if (!accept(type)) throw new Error('expected ' + type);
		}

		function primary() {
			if (accept('(')) {
				const inner = conditional();
				expect(')');
				return inner;
			}
			if (accept('!')) {
				const operand = primary();
				return function (n) {
					return operand(n) ? 0 : 1;
				};
			}
			if (accept('-')) {
				const operand = primary();
				return function (n) {
					return -operand(n);
				};
			}
			if (accept('n')) {
				return function (n) {
					return n;
				};
			}
			if (peek() === 'number') {
				const value = tokens[pos++].value;
				return function () {
					return value;
				};
			}
			throw new Error('unexpected token');
		}

		function level(operators, next) {
			return function () {
				let left = next();
				for (;;) {
					const operator = peek();
					if (operators.indexOf(operator) === -1) return left;
					pos++;
					left = combine(operator, left, next());
				}
			};
		}

		const multiplicative = level(['*', '/', '%'], primary);
		const additive = level(['+', '-'], multiplicative);
		const relational = level(['<', '>', '<=', '>='], additive);
		const equality = level(['==', '!='], relational);
		const logicalAnd = level(['&&'], equality);
		const logicalOr = level(['||'], logicalAnd);

		function conditional() {
			const condition = logicalOr();
			if (!accept('?')) return condition;
			const whenTrue = conditional();
			expect(':');
			const whenFalse = conditional();
			return function (n) {
				return condition(n) ? whenTrue(n) : whenFalse(n);
			};
		}

		const expression = conditional();
		if (pos !== tokens.length) throw new Error('trailing tokens');
		return expression;
	}

	// "nplurals=3; plural=(n==1 ? 0 : ...);" -> { nplurals, select } or null.
	// \b keeps the 'plural' inside 'nplurals' from matching.
	function parseRule(header) {
		const counted = /nplurals\s*=\s*([0-9]+)/.exec(header);
		const formula = /\bplural\s*=\s*([^;]+)/.exec(header);
		if (!counted || !formula) return null;

		const nplurals = parseInt(counted[1], 10);
		const tokens = tokenize(formula[1]);
		if (!nplurals || !tokens || !tokens.length) return null;

		try {
			return { nplurals: nplurals, select: compile(tokens) };
		} catch (e) {
			return null;
		}
	}

	function currentRule() {
		const header = RULE_KEY.toLocaleString();
		// Untranslated, so the lookup handed the key straight back: the
		// strings are the English sources and take the English rule.
		if (header === RULE_KEY) return DEFAULT_RULE;

		if (!Object.prototype.hasOwnProperty.call(ruleCache, header))
			ruleCache[header] = parseRule(header) || DEFAULT_RULE;

		return ruleCache[header];
	}

	global._n = function (singular, plural, count) {
		const number = Number(count);
		const value = isFinite(number) ? number : 0;

		const key = singular + SEP + plural;
		const entry = key.toLocaleString();
		let text;

		if (entry === key) {
			text = value === 1 ? singular : plural;
		} else {
			const forms = entry.split(SEP);
			const index = currentRule().select(Math.abs(value));
			text = forms[index] || forms[0];
		}

		return text.split('%n').join(String(value));
	};

	// For the tests, which have neither a browser nor a localization table.
	global.PluralForms = {
		parseRule: parseRule,
		defaultRule: DEFAULT_RULE,
	};
})(typeof window !== 'undefined' ? window : global);
