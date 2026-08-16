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

describe('Plural', function () {
	// The rule parser and _n() from js/plural.js. In the browser `window` is
	// the global object, so a call site just writes _n(); under mocha the two
	// are separate and only the window carries what plural.js installed.
	const PluralForms = (window as any).PluralForms;
	const _n: (singular: string, plural: string, count: number) => string = (
		window as any
	)._n;

	// Plural-Forms headers as they stand in po/ui-*.po today.
	const RULES: { [lang: string]: string } = {
		de: 'nplurals=2; plural=n != 1;',
		fr: 'nplurals=2; plural=n > 1;',
		ja: 'nplurals=1; plural=0;',
		ru:
			'nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4' +
			' && (n%100<10 || n%100>=20) ? 1 : 2);',
		sl:
			'nplurals=4; plural=n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 ||' +
			' n%100==4 ? 2 : 3;',
		cs:
			'nplurals=4; plural=(n == 1 && n % 1 == 0) ? 0 : (n >= 2 && n <= 4' +
			' && n % 1 == 0) ? 1: (n % 1 != 0 ) ? 2 : 3;',
		ar:
			'nplurals=6; plural=n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 &&' +
			' n%100<=10 ? 3 : n%100>=11 ? 4 : 5;',
	};

	// The form each language picks for these counts, produced by evaluating the
	// same expressions as JavaScript and freezing the result. That cross-check
	// shares JS operator semantics, so it does not cover the one place C
	// differs - integer division, which no shipped rule uses.
	const COUNTS = [0, 1, 2, 3, 5, 11, 21, 22, 101, 111];
	const EXPECTED: { [lang: string]: number[] } = {
		de: [1, 0, 1, 1, 1, 1, 1, 1, 1, 1],
		fr: [0, 0, 1, 1, 1, 1, 1, 1, 1, 1],
		ja: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
		ru: [2, 0, 1, 1, 2, 2, 0, 1, 0, 2],
		sl: [3, 0, 1, 2, 3, 3, 3, 3, 0, 3],
		cs: [3, 0, 1, 1, 3, 3, 3, 3, 3, 3],
		ar: [0, 1, 2, 3, 3, 4, 4, 4, 5, 4],
	};

	describe('PluralForms.parseRule()', function () {
		Object.keys(RULES).forEach(function (lang) {
			it(lang + ' picks the documented form for each count', function () {
				const rule = PluralForms.parseRule(RULES[lang]);
				nodeassert.notEqual(rule, null);
				nodeassert.deepEqual(
					COUNTS.map(function (n) {
						return rule.select(n);
					}),
					EXPECTED[lang],
				);
			});

			it(lang + ' never picks a form outside nplurals', function () {
				const rule = PluralForms.parseRule(RULES[lang]);
				for (let n = 0; n <= 300; n++) {
					nodeassert.ok(rule.select(n) >= 0);
					nodeassert.ok(rule.select(n) < rule.nplurals);
				}
			});
		});

		it('reads nplurals, not the plural inside it', function () {
			nodeassert.equal(PluralForms.parseRule(RULES.ar).nplurals, 6);
		});

		it('rejects a header it cannot parse', function () {
			nodeassert.equal(PluralForms.parseRule(''), null);
			nodeassert.equal(PluralForms.parseRule('nplurals=2;'), null);
			nodeassert.equal(
				PluralForms.parseRule('nplurals=2; plural=n != ;'),
				null,
			);
			nodeassert.equal(
				PluralForms.parseRule('nplurals=2; plural=system("rm");'),
				null,
			);
		});
	});

	describe('_n()', function () {
		const NUL = String.fromCharCode(0);

		// Stand in for the table l10n.js builds out of the ui-*.json of the
		// language in use.
		function withTranslations(table: { [key: string]: string }, body: any) {
			const original = String.prototype.toLocaleString;
			String.prototype.toLocaleString = function () {
				const value = this.valueOf();
				return Object.prototype.hasOwnProperty.call(table, value)
					? table[value]
					: value;
			};
			try {
				body();
			} finally {
				String.prototype.toLocaleString = original;
			}
		}

		it('falls back to the English forms when untranslated', function () {
			nodeassert.equal(_n('%n slide', '%n slides', 1), '1 slide');
			nodeassert.equal(_n('%n slide', '%n slides', 0), '0 slides');
			nodeassert.equal(_n('%n slide', '%n slides', 7), '7 slides');
		});

		it('substitutes every %n and leaves other placeholders alone', function () {
			nodeassert.equal(
				_n('run %n %1 command', 'run %n %1 commands', 3),
				'run 3 %1 commands',
			);
		});

		it('picks the translated form the language asks for', function () {
			const table: { [key: string]: string } = {};
			table[NUL + 'plural-forms'] = RULES.ru;
			table['%n slide' + NUL + '%n slides'] =
				'%n slajd' + NUL + '%n slajda' + NUL + '%n slajdov';

			withTranslations(table, function () {
				nodeassert.equal(_n('%n slide', '%n slides', 1), '1 slajd');
				nodeassert.equal(_n('%n slide', '%n slides', 2), '2 slajda');
				nodeassert.equal(_n('%n slide', '%n slides', 5), '5 slajdov');
				nodeassert.equal(_n('%n slide', '%n slides', 21), '21 slajd');
				nodeassert.equal(_n('%n slide', '%n slides', 111), '111 slajdov');
			});
		});

		it('uses the English rule when the language ships no rule', function () {
			const table: { [key: string]: string } = {};
			table['%n slide' + NUL + '%n slides'] = 'Folie' + NUL + 'Folien';

			withTranslations(table, function () {
				nodeassert.equal(_n('%n slide', '%n slides', 1), 'Folie');
				nodeassert.equal(_n('%n slide', '%n slides', 2), 'Folien');
			});
		});

		it('stays on a form that exists when the table is short', function () {
			const table: { [key: string]: string } = {};
			table[NUL + 'plural-forms'] = RULES.ru;
			table['%n slide' + NUL + '%n slides'] = '%n slajd';

			withTranslations(table, function () {
				nodeassert.equal(_n('%n slide', '%n slides', 5), '5 slajd');
			});
		});
	});
});
