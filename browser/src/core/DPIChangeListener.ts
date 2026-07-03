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
 * DPIChangeListener - runs a callback when the display density changes, by
 * browser zoom or by moving the window to a monitor with a different scale.
 *
 * A resolution media query matches a single dppx value, so it stops firing
 * once the density moves away from that value. The listener re-registers
 * itself after each change to track the new density.
 */

class DPIChangeListener {
	private query: MediaQueryList | null = null;
	private readonly onChange: () => void;

	constructor(onChange: () => void) {
		this.onChange = onChange;
	}

	start(): void {
		if (!window.matchMedia) return;
		this.stop();
		this.query = window.matchMedia(
			'(resolution: ' + window.devicePixelRatio + 'dppx)',
		);
		this.query.addEventListener('change', this.handleChange, {
			once: true,
		});
	}

	stop(): void {
		if (this.query) {
			this.query.removeEventListener('change', this.handleChange);
			this.query = null;
		}
	}

	private handleChange = () => {
		this.onChange();
		// The query only matched the previous density, so track the new one.
		this.start();
	};
}
