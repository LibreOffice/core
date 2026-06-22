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
 * Objects containing LO style mappings
 */

/* global _ */

window.L.Styles = {
	impressLayout : [
		{ id: 0, text: 'Title Slide', translatedName: _('Title Slide') },
		{ id: 1, text: 'Title, Content', translatedName: _('Title, Content') },
		{ id: 3, text: 'Title and 2 Content', translatedName: _('Title and 2 Content') },
		{ id: 19, text: 'Title Only', translatedName: _('Title Only') },
		{ id: 20, text: 'Blank Slide', translatedName: _('Blank Slide') },
		{ id: 32, text: 'Centered Text', translatedName: _('Centered Text') },
		{ id: 12, text: 'Title, Content and 2 Content', translatedName: _('Title, Content and 2 Content') },
		{ id: 15, text: 'Title, 2 Content and Content', translatedName: _('Title, 2 Content and Content') },
		{ id: 16, text: 'Title, 2 Content over Content', translatedName: _('Title, 2 Content over Content') },
		{ id: 14, text: 'Title, Content over Content', translatedName: _('Title, Content over Content') },
		{ id: 18, text: 'Title, 4 Content', translatedName: _('Title, 4 Content') },
		{ id: 34, text: 'Title, 6 Content', translatedName: _('Title, 6 Content') },
		{ id: 27, text: 'Vertical Title, Text, Chart', translatedName: _('Vertical Title, Text, Chart') },
		{ id: 28, text: 'Vertical Title, Vertical Text', translatedName: _('Vertical Title, Vertical Text') },
		{ id: 29, text: 'Title, Vertical Text', translatedName: _('Title, Vertical Text') },
		{ id: 30, text: 'Title, Vertical Text, Clipart', translatedName: _('Title, Vertical Text, Clipart') }
	],

	insertMode : {
		'true'  : 'Insert',
		'false' : 'Overwrite'
	},

	selectionMode : [
		'Standard selection',
		'Extending selection',
		'Adding selection',
		'Block selection'
	]
};
