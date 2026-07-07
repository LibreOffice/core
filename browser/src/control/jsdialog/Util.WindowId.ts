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
 * Util.WindowId - resolve the window id a JSDialog builder talks to, and tell
 *                 core-backed windows apart from browser-only ones.
 */

declare var JSDialog: any;

// A builder owns a core-side window when it carries a window id of its own
function builderHasOwnWindow(builder: JSBuilder): boolean {
	return builder.windowId !== null && builder.windowId !== undefined;
}

// The window id used to address the matching component on the core side
function resolveWindowId(builder: JSBuilder): WindowId | number | string {
	if (builderHasOwnWindow(builder)) return builder.windowId as number;
	if (window.mobileDialogId !== undefined) return window.mobileDialogId;
	if (window.sidebarId !== undefined) return window.sidebarId;
	return -1;
}

// A window is browser-only when it has no counterpart on the core side
function isBrowserOnlyWindow(builder: JSBuilder): boolean {
	return (
		!builderHasOwnWindow(builder) &&
		window.sidebarId === undefined &&
		window.mobileDialogId === undefined
	);
}

JSDialog.resolveWindowId = resolveWindowId;
JSDialog.isBrowserOnlyWindow = isBrowserOnlyWindow;
