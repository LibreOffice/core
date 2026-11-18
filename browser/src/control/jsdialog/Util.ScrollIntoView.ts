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
 * Util.ScrollIntoView - helper for scrolling the embdded scrollable containers so they will
 *                       show the desired element on screen, for example IconView uses it
 */

declare var JSDialog: any;

JSDialog.ScrollIntoViewBlockOption = function (option: string) {
	if (option === 'nearest' || option === 'center') {
		// compatibility with older firefox
		const match = window.navigator.userAgent.match(/Firefox\/([0-9]+)\./);
		const firefoxVer = match ? parseInt(match[1]) : 58;
		const blockOption = firefoxVer >= 58 ? option : 'start';
		return blockOption;
	}

	return option;
};
