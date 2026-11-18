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
 * JSDialog.DropDown - build dropdown container with dynamic list
 */

declare var JSDialog: any;

JSDialog.Dropdown = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
) {
	builder.build(parentContainer, data.children, data.vertical === true);

	if (data.gridKeyboardNavigation) {
		JSDialog.KeyboardGridNavigation(parentContainer);
	} else {
		JSDialog.KeyboardListNavigation(parentContainer);
	}
	return false;
};
