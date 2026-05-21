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

// Non-UNO IDs that share a shortcut with a UNO (or findbar protocol)
// command.  Routing them through the locale-aware shortcut table keeps
// tooltips in sync with what the user actually has to press.
const UNO_ALIASES: Record<string, string> = {
	save: '.uno:Save',
	print: '.uno:Print',
	insertcomment: '.uno:InsertAnnotation',
	hyperlinkdialog: '.uno:HyperlinkDialog',
	inserthyperlink: '.uno:HyperlinkDialog',
	search: 'vnd.sun.star.findbar:FocusToFindbar',
	'home-search': 'vnd.sun.star.findbar:FocusToFindbar',
};

// Non-UNO IDs with explicit shortcuts not from Accelerators.xcu.
const EXPLICIT_ALIASES: Record<string, string> = {
	'keyboard-shortcuts': 'Ctrl+Shift+?',
};

class ShortcutsUtil {
	private shortcutMap: Map<string, string> = new Map();

	constructor() {
		// Load shortcuts generated from core's Accelerators.xcu.
		if (typeof unoShortcutsMap !== 'undefined') {
			for (const [command, shortcut] of Object.entries(unoShortcutsMap)) {
				this.shortcutMap.set(command, shortcut);
			}
		}

		// Apply per-language shortcut overrides from Accelerators.xcu.
		// Core defines different key combos per language
		if (typeof unoShortcutsL10N !== 'undefined') {
			for (const [lang, overrides] of Object.entries(unoShortcutsL10N)) {
				if ((String as any).locale.startsWith(lang)) {
					for (const [command, shortcut] of Object.entries(overrides)) {
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
					for (const [eng, loc] of Object.entries(replacements)) {
						text = text.replace(eng, loc);
					}
					break;
				}
			}
		}
		return text;
	}

	public getTooltipLabel(text: string, command: string): string {
		const shortcut = this.getShortcutText(command);
		if (!shortcut) return text;

		return _(text).replace('~', '') + ' (' + shortcut + ')';
	}

	public getMenuLabel(text: string, command: string): string {
		const shortcut = this.getShortcutText(command);
		if (!shortcut) return text;

		return (
			_(text).replace('~', '') +
			' <span class="shortcut">' +
			shortcut +
			'</span>'
		);
	}

	/**
	 * Return the locale-aware shortcut display string for a UNO command,
	 * or undefined if none is registered. On Mac, "Ctrl"/"Alt" are replaced
	 * with the corresponding Mac symbols.
	 */
	public getShortcutText(command: string): string | undefined {
		const shortcut = this.shortcutMap.get(command);
		if (!shortcut) return undefined;
		return app.util.replaceCtrlAltInMac(this.localizeModifiers(shortcut));
	}
}

JSDialog.ShortcutsUtil = new ShortcutsUtil();
