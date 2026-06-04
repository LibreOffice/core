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
// CSS requires

var $ = require('jquery');
global.$ = global.jQuery = $;

require('l10n-for-node');

// On the desktop and mobile apps the Options dialog runs in a file:// iframe
// where l10n-for-node's <link rel="localizations"> loader does not work, so the
// translations are supplied as a bundled window.LOCALIZATIONS table (see
// l10n-settings.js / util/create-settings-l10n-js.py). Mirror global.js and look
// the string up there. Online has no window.LOCALIZATIONS and keeps the
// l10n-for-node behavior unchanged.
if (window.LOCALIZATIONS) {
	String.prototype.toLocaleString = function () {
		const string = this.valueOf();
		return Object.prototype.hasOwnProperty.call(window.LOCALIZATIONS, string)
			? window.LOCALIZATIONS[string]
			: string;
	};
}

global._ = function (string) {
	return string.toLocaleString();
};

global.l10nstrings = require('./admin.strings.js');

global.d3 = require('d3');
global.Admin = require('admin-src.js');
