// @ts-strict-ignore
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
This file defines the SaveState class, which handles the logic for managing the document's save state.
It controls the display of the saving status, saved status, and triggers the associated animations and icon changes.
*/

class SaveState {
	map: any;
	saveEle: HTMLElement;
	saveIconEl: HTMLElement;

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	constructor(map: any) {
		this.map = map;
		this.initialize();
	}

	initialize() {
		this.saveEle = document.querySelector('[id^="save"].unotoolbutton');
		if (this.saveEle) {
			this.saveIconEl = this.saveEle.querySelector('img');
			// Reloading the shortcuts bar on user join can rebuild save icon
			if (app.file.modified) this.saveEle.classList.add('savemodified');
		} else {
			app.console.debug('SaveState: no save icon - might be hidden');
			this.saveIconEl = null;
		}
	}

	// Function to show the saving status
	showSavingStatus(): void {
		if (window.mode.isSmallScreenDevice()) return;

		if (!this.saveEle) this.initialize();

		// Only do saving animation if any content is modified in document
		if (this.saveEle && this.saveEle.classList.contains('savemodified')) {
			this.saveEle.classList.remove('savemodified');
			// Dynamically set the content string for saving state
			const savingText = _('Saving');
			this.saveEle.style.setProperty('--save-state', `"${savingText}"`);
			this.saveEle.classList.add('saving');
			this.saveIconEl?.classList.add('rotate-icon'); // Start the icon rotation
			this.saveEle.setAttribute('disabled', 'true'); // Disable the button
		}
	}

	// Function to show the saved status
	showSavedStatus(): void {
		if (window.mode.isSmallScreenDevice()) return;

		if (!this.saveEle) this.initialize();

		if (this.saveEle && !this.saveEle.classList.contains('savemodified')) {
			this.saveEle.classList.remove('saving');
			this.saveIconEl?.classList.remove('rotate-icon'); // Stop the icon rotation
			// Dynamically set the content string for saved state
			const savedText = _('Saved');
			this.saveEle.style.setProperty('--save-state', `"${savedText}"`);
			this.saveEle.classList.add('saved');
			// Add some delay to show "saved" status, then hide this info
			setTimeout(() => {
				this.saveEle.classList.remove('saved');
				this.saveEle.removeAttribute('disabled'); // Enable the button
			}, 2000);
		}
	}

	showModifiedStatus(): void {
		if (!this.saveEle) this.initialize();
		if (this.saveEle) {
			this.saveEle.classList.remove('saving');
			this.saveEle.classList.remove('saved');
			this.saveIconEl?.classList.remove('rotate-icon'); // Stop the icon rotation
			this.saveEle.removeAttribute('disabled'); // Enable the button
			this.saveEle.classList.add('savemodified');
		}
	}
}

app.definitions.saveState = SaveState;
