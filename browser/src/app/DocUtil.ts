/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class DocUtil {
	private static inlineStyleHelper(
		text: string,
		toBeReplaced: string,
		quotes: string,
	): string {
		let startIndex = 1;
		while (startIndex !== -1) {
			startIndex = text.indexOf(toBeReplaced);
			if (startIndex !== -1) {
				const endIndex = text.indexOf(
					quotes,
					startIndex + toBeReplaced.length + 1,
				);
				const subString = text.substr(startIndex, endIndex - startIndex + 1);
				text = text.replace(subString, '');
			}
		}
		return text;
	}

	private static stripStyle(html: string): string {
		let startIndex = 1;

		while (startIndex !== -1) {
			startIndex = html.indexOf('<style ');
			if (startIndex !== -1) {
				const endIndex = html.indexOf('</style>');
				const subString = html.substr(
					startIndex,
					endIndex - startIndex + '</style>'.length,
				);
				html = html.replace(subString, '');
			}
		}

		// Remove also inline styles.
		html = this.inlineStyleHelper(html, 'style="', '"'); // For double quotes.
		html = this.inlineStyleHelper(html, "style='", "'"); // For single quotes.

		return html;
	}

	public static stripHTML(
		html: string,
		domParser: DOMParser = new DOMParser(),
	) {
		html = this.stripStyle(html);
		var tmp = domParser.parseFromString(html, 'text/html').body;
		if (tmp.textContent) return tmp.textContent.trim();
		if (tmp.innerText) return tmp.innerText.trim();
		return '';
	}
}
