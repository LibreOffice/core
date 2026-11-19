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

// The Title Case rule for the labels of the UI.
namespace TitleCase {
	// Words that stay lower case inside a Title Case label: articles, and the short
	// conjunctions and prepositions. Up, off and out are left out, since in a label
	// they are part of the verb, as in Scroll Up.
	export const minorWords = [
		'a',
		'an',
		'the',
		'and',
		'but',
		'or',
		'nor',
		'yet',
		'as',
		'at',
		'by',
		'for',
		'from',
		'in',
		'into',
		'of',
		'on',
		'onto',
		'per',
		'to',
		'via',
		'with',
	];

	// Units are written the way they are abbreviated after a number, as in 0.75 pt.
	export const units = ['pt', 'cm', 'mm', 'px', 'em', 'ms'];

	// The words of a label that should start with a capital and do not. The first
	// word is always capitalised, and a word that starts with no letter is skipped.
	export function lowerCaseWords(label: string): string[] {
		const words = label.split(/\s+/);

		return words.filter(function (word: string, at: number) {
			// A file extension, as in (.pdf), is written the way the file name has it.
			if (/^[^A-Za-z0-9]*\.[A-Za-z0-9]+[^A-Za-z0-9]*$/.test(word)) return false;

			if (
				at > 0 &&
				/[0-9][^A-Za-z0-9]*$/.test(words[at - 1]) &&
				units.indexOf(word.replace(/[^A-Za-z]/g, '')) >= 0
			)
				return false;

			let bare = word;

			// Punctuation has no case, a letter or a digit ends the skip.
			while (
				bare &&
				!/[0-9]/.test(bare.charAt(0)) &&
				bare.charAt(0).toLowerCase() === bare.charAt(0).toUpperCase()
			)
				bare = bare.slice(1);

			const first = bare.charAt(0);

			if (!first || first === first.toUpperCase()) return false;

			return at === 0 || minorWords.indexOf(bare.toLowerCase()) < 0;
		});
	}
}
