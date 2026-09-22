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
	// The browser always binds Ctrl+F to open the find bar globally,
	// regardless of locale (locale-specific additional bindings are extra).
	// Do not use vnd.sun.star.findbar:FocusToFindbar here: its XCU entry
	// reflects the desktop shortcut, which differs from the browser binding
	// in some locales (e.g. German desktop uses Shift+Alt+F).
	find: 'Ctrl+F',
	switchtoedit: 'Ctrl+Shift+E',
};

class ShortcutsUtil {
	// One map per document type, built the first time that type asks.
	private shortcutMaps: Map<string, Map<string, string>> = new Map();

	/** The document type the shortcuts are being asked about. */
	private docType(): string {
		const map = (window as any).app ? (window as any).app.map : null;
		return map && map._docLayer ? map._docLayer._docType : '';
	}

	private buildMap(docType: string): Map<string, string> {
		const shortcutMap = new Map<string, string>();

		// Shortcuts generated from core's Accelerators.xcu: what every
		// document type shares, then what this one says instead, then
		// what this UI language says instead of that.  An empty string
		// means the command has no shortcut here.
		const layers: Record<string, string>[] = [];
		if (typeof unoShortcutsMap !== 'undefined') {
			layers.push(unoShortcutsMap);
		}
		if (typeof unoShortcutsDocType !== 'undefined') {
			layers.push(unoShortcutsDocType[docType] || {});
		}
		if (typeof unoShortcutsL10N !== 'undefined') {
			for (const [lang, overrides] of Object.entries(unoShortcutsL10N)) {
				if ((String as any).locale.startsWith(lang)) {
					layers.push(overrides[docType] || {});
					break;
				}
			}
		}
		for (const layer of layers) {
			for (const [command, shortcut] of Object.entries(layer)) {
				shortcutMap.set(command, shortcut);
			}
		}

		// Aliases: non-UNO IDs that resolve to a UNO command's shortcut.
		for (const [alias, unoCmd] of Object.entries(UNO_ALIASES)) {
			const shortcut = shortcutMap.get(unoCmd);
			if (shortcut) {
				shortcutMap.set(alias, shortcut);
			}
		}

		// Explicit aliases not derived from the XCU.
		for (const [alias, shortcut] of Object.entries(EXPLICIT_ALIASES)) {
			shortcutMap.set(alias, shortcut);
		}

		return shortcutMap;
	}

	private map(): Map<string, string> {
		const docType = this.docType();
		let shortcutMap = this.shortcutMaps.get(docType);
		if (!shortcutMap) {
			shortcutMap = this.buildMap(docType);
			this.shortcutMaps.set(docType, shortcutMap);
		}
		return shortcutMap;
	}

	public hasShortcut(command: string): boolean {
		return !!this.map().get(command);
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
		const shortcut = this.map().get(command);
		if (!shortcut) return undefined;
		return app.util.replaceCtrlAltInMac(this.localizeModifiers(shortcut));
	}
}

JSDialog.ShortcutsUtil = new ShortcutsUtil();
