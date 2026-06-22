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
 * JSDialog.KeyboardTabNavigation - handles keyboard-based navigation and selection in tab-based UI components
 */

declare var JSDialog: any;

function isTabVisible(tab: HTMLElement): boolean {
	return !tab.classList.contains('hidden');
}

function findNextVisibleTab(
	tabs: HTMLButtonElement[],
	currentIndex: number,
	backwards: boolean,
): HTMLButtonElement {
	const diff = backwards ? -1 : 1;
	const total = tabs.length;

	for (let i = 1; i <= total; i++) {
		const nextIndex = (currentIndex + diff * i + total) % total;
		const nextTab = tabs[nextIndex];
		if (isTabVisible(nextTab)) {
			return nextTab;
		}
	}

	// Fallback to current tab if no visible one is found (shouldn't happen)
	return tabs[currentIndex];
}

function moveFocusToPreviousTab(
	tabs: HTMLButtonElement[],
	currentIndex: number,
) {
	const currentTab = tabs[currentIndex];
	currentTab.setAttribute('aria-selected', 'false');
	currentTab.setAttribute('tabindex', '-1');

	const nextTab = findNextVisibleTab(tabs, currentIndex, true);
	nextTab.setAttribute('aria-selected', 'true');
	nextTab.removeAttribute('tabindex');
	nextTab.click();
	nextTab.focus(); // Prevent document from taking focus
}

function moveFocusToNextTab(tabs: HTMLButtonElement[], currentIndex: number) {
	const currentTab = tabs[currentIndex];
	currentTab.setAttribute('aria-selected', 'false');
	currentTab.setAttribute('tabindex', '-1');

	const nextTab = findNextVisibleTab(tabs, currentIndex, false);
	nextTab.setAttribute('aria-selected', 'true');
	nextTab.removeAttribute('tabindex');
	nextTab.click();
	nextTab.focus(); // Prevent document from taking focus
}

function moveFocusIntoTabPage(
	contentDivs: HTMLElement[],
	currentTab: HTMLButtonElement,
) {
	if (!currentTab.hasAttribute('aria-controls')) {
		console.warn('Current tab has no aria-controls attribute');
		return;
	}

	const controlledId = currentTab.getAttribute('aria-controls');
	const controlledDiv = contentDivs.find((div) => div.id === controlledId);
	if (controlledDiv) {
		const firstFocusableElement = JSDialog.FindFocusableWithin(
			controlledDiv,
			'next',
		);
		if (firstFocusableElement !== null) {
			firstFocusableElement.focus();
		}
	}
}

function handleTabKeydown(
	event: KeyboardEvent,
	tabs: HTMLButtonElement[],
	contentDivs: HTMLElement[],
	vertical: boolean,
) {
	const currentTab = event.currentTarget as HTMLButtonElement;
	const currentIndex = tabs.indexOf(currentTab);

	if (currentIndex === -1) return;

	// Per the WAI-ARIA tabs pattern the axis that moves between tabs follows
	// the rail orientation: Up/Down for a vertical rail, Left/Right for a
	// horizontal row. The cross axis key enters the panel.
	const prevTabKey = vertical ? 'ArrowUp' : 'ArrowLeft';
	const nextTabKey = vertical ? 'ArrowDown' : 'ArrowRight';
	const enterPanelKey = vertical ? 'ArrowRight' : 'ArrowDown';

	switch (event.key) {
		case prevTabKey:
			moveFocusToPreviousTab(tabs, currentIndex);
			break;

		case nextTabKey:
			moveFocusToNextTab(tabs, currentIndex);
			break;

		case enterPanelKey:
		case 'PageDown':
			moveFocusIntoTabPage(contentDivs, currentTab);
			break;

		case 'Home': {
			const firstIndex = 0;
			var firstTab = tabs[firstIndex];
			if (!isTabVisible(firstTab))
				firstTab = findNextVisibleTab(tabs, firstIndex, false);
			firstTab.focus();
			break;
		}

		case 'End': {
			const lastIndex = tabs.length - 1;
			var lastTab = tabs[lastIndex];
			if (!isTabVisible(lastTab))
				lastTab = findNextVisibleTab(tabs, lastIndex, true);
			lastTab.focus();
			break;
		}

		case 'Escape':
			if (window.app && window.app.map && window.app.map.focus) {
				window.app.map.focus();
			}
			break;
	}
}

JSDialog.KeyboardTabNavigation = function (
	tabs: HTMLButtonElement[],
	contentDivs: HTMLElement[],
	vertical?: boolean,
) {
	// Add keydown listeners to all tabs
	tabs.forEach((tab) => {
		tab.addEventListener('keydown', (event: KeyboardEvent) => {
			handleTabKeydown(event, tabs, contentDivs, vertical === true);
		});
	});
};
