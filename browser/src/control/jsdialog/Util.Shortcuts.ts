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
 * Shortcuts for the classic toolbar & the Notebookbar.
 *
 * UNO command shortcuts are generated from core's Accelerators.xcu by
 * scripts/unoshortcuts.py into browser/src/unoshortcuts.js.
 */

declare var JSDialog: any;
declare var unoShortcutsMap: any;
declare var unoShortcutsL10N: any;
declare var unoShortcutsModifierL10N: any;

// Non-UNO IDs that share a shortcut with a UNO command.
const UNO_ALIASES: Record<string, string> = {
	save: '.uno:Save',
	print: '.uno:Print',
	insertcomment: '.uno:InsertAnnotation',
	hyperlinkdialog: '.uno:HyperlinkDialog',
	inserthyperlink: '.uno:HyperlinkDialog',
};

// Non-UNO IDs with explicit shortcuts not from Accelerators.xcu.
const EXPLICIT_ALIASES: Record<string, string> = {
	search: 'Ctrl+F',
	'home-search': 'Ctrl+F',
	'keyboard-shortcuts': 'Ctrl+Shift+?',
};

class ShortcutsUtil {
	private shortcutMap: Map<string, string> = new Map();

	constructor() {
		// Load shortcuts generated from core's Accelerators.xcu.
		if (typeof unoShortcutsMap !== 'undefined') {
			for (const [command, shortcut] of Object.entries(unoShortcutsMap)) {
				this.shortcutMap.set(command, shortcut as string);
			}
		}

		// Apply per-language shortcut overrides from Accelerators.xcu.
		// Core defines different key combos per language
		if (typeof unoShortcutsL10N !== 'undefined') {
			for (const [lang, overrides] of Object.entries(unoShortcutsL10N)) {
				if ((String as any).locale.startsWith(lang)) {
					const o = overrides as Record<string, string>;
					for (const [command, shortcut] of Object.entries(o)) {
						this.shortcutMap.set(command, shortcut);
					}
					break;
				}
			}
		}

		// Aliases: non-UNO IDs that resolve to a UNO command's shortcut.
		for (const [alias, unoCmd] of Object.entries(UNO_ALIASES)) {
			const shortcut = this.shortcutMap.get(unoCmd);
			if (shortcut) {
				this.shortcutMap.set(alias, shortcut);
			}
		}

		// Explicit aliases not derived from the XCU.
		for (const [alias, shortcut] of Object.entries(EXPLICIT_ALIASES)) {
			this.shortcutMap.set(alias, shortcut);
		}
	}

	public hasShortcut(command: string): boolean {
		return this.shortcutMap.has(command);
	}

	/**
	 * Returns the text with appended shortcut for a given text and UNO Command.
	 * @param {string} text - The text to localize.
	 * @param {string} shortcut - The shortcut to localize.
	 * @returns {string} - The localized text with the shortcut.
	 */
	public localizeModifiers(text: string): string {
		if (typeof unoShortcutsModifierL10N !== 'undefined') {
			for (const [lang, replacements] of Object.entries(
				unoShortcutsModifierL10N,
			)) {
				if (String.locale.startsWith(lang)) {
					for (const [eng, loc] of Object.entries(
						replacements as Record<string, string>,
					)) {
						text = text.replace(eng, loc as string);
					}
					break;
				}
			}
		}
		return text;
	}

	public getShortcut(text: string, command: string): string {
		let shortcut = this.shortcutMap.get(command);
		if (!shortcut) return text;

		shortcut = this.localizeModifiers(shortcut);

		var newText =
			_(text).replace('~', '') +
			' (' +
			app.util.replaceCtrlAltInMac(shortcut) +
			')';

		return newText;
	}
}

JSDialog.ShortcutsUtil = new ShortcutsUtil();
