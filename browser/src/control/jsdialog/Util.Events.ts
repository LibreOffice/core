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
 * Util.Events - helper for handling events
 */

declare var JSDialog: any;

function addOnClick(element: Element, callback: EventListener) {
	['click', 'keypress'].forEach((eventType) => {
		element.addEventListener(eventType, (event: Event) => {
			if (eventType === 'keypress') {
				const keyEvent: KeyboardEvent = event as KeyboardEvent;
				if (
					keyEvent.key !== 'Enter' &&
					keyEvent.key !== 'Space' &&
					keyEvent.key !== ' '
				) {
					return;
				}
			}

			callback(event);
		});
	});
}

JSDialog.AddOnClick = addOnClick;
