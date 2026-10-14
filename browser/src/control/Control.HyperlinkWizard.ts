/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * HyperlinkWizard - the mobile wizard for a link, the small-screen replacement for the link
 * popup.
 */

interface HyperlinkWizardEntry {
	id: string;
	text: string;
	command: string;
	callback: () => void;
}

class HyperlinkWizard {
	public static show(
		url: string,
		linkPosition?: cool.SimplePoint,
		linkIsClientSide = false,
	): void {
		const params = URLPopUpSection.commandParameters(linkPosition);

		const entries: HyperlinkWizardEntry[] = [
			{
				id: 'openlink',
				text: url,
				command: '.uno:OpenHyperlinkOnCursor',
				callback: () => URLPopUpSection.openLink(url),
			},
			{
				id: 'copylink',
				text: _('Copy link location'),
				command: '.uno:CopyHyperlinkLocation',
				callback: () => URLPopUpSection.copyLinkLocation(url, params),
			},
		];

		// a client-side link, as on a PDF page, cannot be edited
		if (app.map._permission !== 'readonly' && !linkIsClientSide) {
			entries.push({
				id: 'editlink',
				text: _('Edit link'),
				command: '.uno:EditHyperlink',
				callback: () => app.map.sendUnoCommand('.uno:EditHyperlink', params),
			});
			entries.push({
				id: 'removelink',
				text: _('Remove link'),
				command: '.uno:RemoveHyperlink',
				callback: () => app.map.sendUnoCommand('.uno:RemoveHyperlink', params),
			});
		}

		const data = {
			id: 'hyperlink',
			type: 'mainmenu',
			enabled: true,
			text: _('Link'),
			executionType: 'menu',
			children: entries.map((entry) => ({
				type: 'menuitem',
				enabled: true,
				executionType: 'callback',
				children: [],
				...entry,
			})),
		};

		// closes on the chosen entry, like a context menu
		window.contextMenuWizard = true;
		app.map.fire('mobilewizard', { data: data });
	}
}
