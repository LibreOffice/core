// @ts-strict-ignore
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
/*
 * JSDialog.MobileSearchBar - mobile search bar
 */

/* global _ _UNO */
class MobileSearchBar extends Toolbar {
	constructor(map: any) {
		super(map, 'MobileSearchBar', 'toolbar-search');
	}

	getToolItems(): Array<ToolItemWidgetJSON> {
		return [
			{
				type: 'customtoolitem',
				id: 'hidesearchbar',
				w2icon: 'prevrecord',
				text: _('Hide the search bar'),
			},
			{ type: 'searchedit', id: 'search', placeholder: _('Search'), text: '' },
			{
				type: 'customtoolitem',
				id: 'cancelsearch',
				text: _('Clear the search field'),
				enabled: false,
			},
			{
				type: 'customtoolitem',
				id: 'searchprev',
				text: _UNO('.uno:UpSearch'),
				enabled: false,
				pressAndHold: true,
			},
			{
				type: 'customtoolitem',
				id: 'searchnext',
				text: _UNO('.uno:DownSearch'),
				enabled: false,
				pressAndHold: true,
			},
			{ type: 'spacer', id: 'left' },
		];
	}

	create() {
		const items = this.getToolItems();
		this.builder.build(this.parentContainer, items, undefined);
	}
}

JSDialog.MobileSearchBar = function (map: any) {
	return new MobileSearchBar(map);
};
