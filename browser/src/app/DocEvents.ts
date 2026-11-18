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
    This is for listening to / firing document events.
    Will be useful for custom events but native events should also be bound via this bridge for maintainability.
    Search "app.events.on" and "app.events.fire" for examples.

    For firing custom events:
        app.events.fire('myCustomEvent', {..details});
*/

type eventCallback = (...args: any[]) => void;

class DocEvents {
	static documentContainerId = 'document-container';
	container: HTMLDivElement;

	constructor() {
		this.initiate();
	}

	public initiate() {
		this.container = document.getElementById(
			DocEvents.documentContainerId,
		) as HTMLDivElement;

		if (!this.container) console.error('DocEvents initation failed.');

		// Resize event for a div element is not fired. We'll use a resize-observer for catching the event.
		new ResizeObserver(this.fire.bind(this, 'resize')).observe(this.container);
	}

	public fire(eventType: string, details: any) {
		const newEvent = new CustomEvent(eventType, { detail: details });
		this.container.dispatchEvent(newEvent);
	}

	public on(eventType: string, callback: eventCallback) {
		this.container.addEventListener(eventType, callback);
	}

	public off(eventType: string, callback: eventCallback) {
		this.container.removeEventListener(eventType, callback);
	}
}
