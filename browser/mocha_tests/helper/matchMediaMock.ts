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

// jsdom does not implement window.matchMedia, so tests that exercise
// resolution media queries install this controllable stand-in and drive the
// change events by hand.

interface FakeMediaListener {
	fn: () => void;
	once: boolean;
}

class FakeMediaQueryList {
	media: string;
	matches = false;
	addCount = 0;
	removeCount = 0;
	private listeners: FakeMediaListener[] = [];

	constructor(media: string) {
		this.media = media;
	}

	addEventListener(
		_type: string,
		fn: () => void,
		options?: { once?: boolean },
	): void {
		this.addCount++;
		this.listeners.push({ fn, once: !!(options && options.once) });
	}

	removeEventListener(_type: string, fn: () => void): void {
		this.removeCount++;
		this.listeners = this.listeners.filter((l) => l.fn !== fn);
	}

	// Deliver a change event, mirroring the browser's { once: true } handling
	// by removing a one-shot listener before it runs.
	fireChange(): void {
		const current = this.listeners.slice();
		for (const l of current) {
			if (l.once) this.listeners = this.listeners.filter((x) => x !== l);
			l.fn();
		}
	}

	get listenerCount(): number {
		return this.listeners.length;
	}
}

// Every FakeMediaQueryList handed out since the last installFakeMatchMedia(),
// in creation order, so tests can inspect and fire them.
let fakeMediaQueries: FakeMediaQueryList[] = [];
let savedMatchMedia: any;

function installFakeMatchMedia(): void {
	fakeMediaQueries = [];
	savedMatchMedia = (window as any).matchMedia;
	(window as any).matchMedia = (media: string): any => {
		const query = new FakeMediaQueryList(media);
		fakeMediaQueries.push(query);
		return query;
	};
}

function restoreMatchMedia(): void {
	(window as any).matchMedia = savedMatchMedia;
}

function setDevicePixelRatio(value: number): void {
	Object.defineProperty(window, 'devicePixelRatio', {
		configurable: true,
		value: value,
	});
}
