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

// Localized LibreLogo vocabulary: command keywords, constants, units, quote
// characters and messages for one language, built from l10n/logo-l10n.json
// (see tools/gen-l10n.py). Keys follow LibreLogo.py's properties files.

// The keys that are neither commands nor constants: they never become keywords.
const NON_KEYWORDS = new Set([
	'LIBRELOGO', 'ERROR', 'ERR_ZERODIVISION', 'ERR_NAME', 'ERR_ARGUMENTS',
	'ERR_BLOCK', 'ERR_KEY', 'ERR_INDEX', 'ERR_STOP', 'ERR_MAXRECURSION',
	'ERR_MEMORY', 'ERR_NOTAPROGRAM', 'DECIMAL', 'LEFTSTRING', 'RIGHTSTRING',
	'PT', 'INCH', 'MM', 'CM', 'HOUR', 'DEG',
]);

// Color name key -> RGB, in LibreLogo's identifier order (PENCOLOR [5] is RED).
export const COLORS = [
	['BLACK', 0x000000], ['SILVER', 0xc0c0c0], ['GRAY', 0x808080],
	['WHITE', 0xffffff], ['MAROON', 0x800000], ['RED', 0xff0000],
	['PURPLE', 0x800080], ['FUCHSIA', 0xff00ff], ['GREEN', 0x008000],
	['LIME', 0x00ff00], ['OLIVE', 0x808000], ['YELLOW', 0xffff00],
	['NAVY', 0x000080], ['BLUE', 0x0000ff], ['TEAL', 0x008080],
	['AQUA', 0x00ffff], ['PINK', 0xffc0cb], ['TOMATO', 0xff6347],
	['ORANGE', 0xffa500], ['GOLD', 0xffd700], ['VIOLET', 0x9400d3],
	['SKYBLUE', 0x87ceeb], ['CHOCOLATE', 0xd2691e], ['BROWN', 0xa52a2a],
	['INVISIBLE', 0xffffffff],
];

// String constants that are matched case-insensitively against string
// arguments (PENSTYLE "dashed", FONTWEIGHT "bold", ...) rather than parsed
// as keywords.
export const STRING_CONSTANTS = COLORS.map((c) => c[0]).concat([
	'NONE', 'BEVEL', 'MITER', 'ROUNDED', 'SOLID', 'DASH', 'DOTTED', 'BOLD',
	'ITALIC', 'UPRIGHT', 'NORMAL', 'SQUARE',
]);

// LibreLogo language codes look like hu, pt_BR, ca_valencia. Accept BCP47
// style input too (pt-BR, zh-Hant-TW) and return the candidates to try.
export function languageCandidates(lang) {
	const out = [];
	if (lang) {
		const norm = String(lang).replace(/-/g, '_');
		const parts = norm.split('_');
		if (parts.length >= 2) {
			// zh_Hant_TW -> zh_TW, pt_BR -> pt_BR
			out.push(parts[0] + '_' + parts[parts.length - 1]);
			out.push(norm);
		} else {
			out.push(norm);
		}
		out.push(parts[0]);
		if (parts[0] === 'en') out.push('en_US');
	}
	out.push('en_US');
	return out;
}

// One language for the Logo program: its keyword names, error messages, colour
// names, decimal separator and quotation marks, with English always accepted.
export class LogoLocale {
	constructor(table, lang) {
		this.table = table;
		this.lang = 'en_US';
		this.values = {};
		const candidates = languageCandidates(lang);
		// Pick the most specific candidate that has any own translation:
		for (const cand of candidates) {
			if (cand === 'en_US') break;
			let hit = false;
			for (const key in table) {
				if (Object.prototype.hasOwnProperty.call(table[key], cand)) {
					hit = true;
					break;
				}
			}
			if (hit) {
				this.lang = cand;
				break;
			}
		}
		const base = this.lang.split('_')[0];
		for (const key in table) {
			const entry = table[key];
			this.values[key] = entry[this.lang] !== undefined ? entry[this.lang]
				: entry[base] !== undefined ? entry[base]
				: entry.en_US;
		}
		this.decimal = this.values.DECIMAL || '.';
		// The language's quotes plus the English ones, so “...” works everywhere.
		this.leftQuotes = (this.values.LEFTSTRING + table.LEFTSTRING.en_US).replace(/\|/g, '');
		this.rightQuotes = (this.values.RIGHTSTRING + table.RIGHTSTRING.en_US).replace(/\|/g, '');

		// lowercase keyword alternative -> canonical key
		this.keywords = new Map();
		for (const key in this.values) {
			if (NON_KEYWORDS.has(key)) continue;
			for (const alt of this.alternatives(key)) {
				if (alt && !this.keywords.has(alt)) this.keywords.set(alt, key);
			}
		}
		// Keywords are case-insensitive, so let English always work as well:
		// a localized program can still use FORWARD etc.
		for (const key in table) {
			if (NON_KEYWORDS.has(key)) continue;
			for (const alt of table[key].en_US.split('|')) {
				const a = alt.trim().toLowerCase();
				if (a && !this.keywords.has(a)) this.keywords.set(a, key);
			}
		}

		// lowercase color name -> rgb
		this.colors = new Map();
		for (const [key, rgb] of COLORS) {
			for (const alt of this.alternatives(key)) this.colors.set(alt, rgb);
			for (const alt of table[key].en_US.split('|')) this.colors.set(alt.trim().toLowerCase(), rgb);
		}

		// Units: alternative -> factor to points. "IN" ends with '"' in English.
		this.units = [];
		const addUnit = (key, factor) => {
			const alts = new Set(this.alternatives(key).concat(table[key].en_US.toLowerCase().split('|')));
			for (const alt of alts) if (alt) this.units.push([alt, factor]);
		};
		addUnit('PT', 1);
		addUnit('INCH', 72);
		addUnit('MM', 72 / 25.4);
		addUnit('CM', 720 / 25.4);
		addUnit('HOUR', 30); // 12h = 360°, so 1h = 30°
		addUnit('DEG', 1);
		// Longest alternatives first so "cm" is not shadowed by a shorter unit:
		this.units.sort((a, b) => b[0].length - a[0].length);

		// FOR ... IN: some languages spell IN as a suffix ("-ban"), see LibreLogo.py.
		this.inSuffixes = this.alternatives('IN').filter((a) => a.startsWith('-')).map((a) => a.slice(1));
	}

	// All localized alternatives of a key, lowercased.
	alternatives(key) {
		const v = this.values[key];
		if (v === undefined) return [];
		return v.split('|').map((s) => s.trim().toLowerCase()).filter((s) => s.length > 0);
	}

	// The first localized name of a key, as LibreLogo.py's __locname__.
	localName(key) {
		const v = this.values[key];
		if (v === undefined) return key;
		return v.split('|')[0];
	}

	// Canonical key of a keyword, or undefined for variables and other words.
	keywordOf(word) {
		return this.keywords.get(word.toLowerCase());
	}

	// Does string `s` match the string constant `key` (case-insensitively)?
	matchesConstant(s, key) {
		if (typeof s !== 'string') return false;
		const low = s.toLowerCase();
		return this.alternatives(key).includes(low)
			|| this.table[key].en_US.toLowerCase().split('|').includes(low);
	}

	// A localized message with %s / %d placeholders filled in.
	message(key, ...args) {
		let s = this.values[key] || key;
		for (const a of args) s = s.replace(/%[sd]/, String(a));
		return s;
	}
}
