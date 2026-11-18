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
 * JSDialog.KeyboardListNavigation - handles keyboard-based focus and selection in list-based UI components
 */

declare var JSDialog: any;

function KeyboardListNavigation(
	event: KeyboardEvent,
	currentElement: HTMLElement,
) {
	switch (event.key) {
		case 'ArrowDown':
			moveToFocusableEntry(currentElement, 'next');
			event.preventDefault();
			break;
		case 'ArrowUp':
			moveToFocusableEntry(currentElement, 'previous');
			event.preventDefault();
			break;
		default:
			break;
	}
}
// Helper function to move focus and apply selection class
function moveToFocusableEntry(
	currentElement: HTMLElement,
	direction: 'next' | 'previous',
) {
	const updateAriaSelected = (elem: HTMLElement, value: string) => {
		if (elem.hasAttribute('aria-selected')) {
			elem.setAttribute('aria-selected', value);
		}
	};

	const updateAriaActiveDescendant = (elem: HTMLElement) => {
		const listbox = elem.closest('[role="listbox"]');
		if (listbox && elem.id) {
			listbox.setAttribute('aria-activedescendant', elem.id);
		}
	};

	// If the current element is focused but not selected, add 'selected' class and return
	if (
		document.activeElement === currentElement &&
		!currentElement.classList.contains('selected') &&
		direction === 'next'
	) {
		currentElement.classList.add('selected');
		updateAriaSelected(currentElement, 'true');
		updateAriaActiveDescendant(currentElement);
		return;
	}

	const siblingElement = JSDialog.FindNextFocusableSiblingElement(
		currentElement,
		direction,
	);

	if (siblingElement) {
		(siblingElement as HTMLElement).focus();
		siblingElement.classList.add('selected');
		updateAriaSelected(siblingElement, 'true');
		updateAriaActiveDescendant(siblingElement);

		currentElement.classList.remove('selected');
		updateAriaSelected(currentElement, 'false');
	}
}

JSDialog.KeyboardListNavigation = function (container: HTMLElement) {
	container.addEventListener('keydown', (event: KeyboardEvent) => {
		const allowedKeys = ['ArrowDown', 'ArrowUp'];
		if (!allowedKeys.includes(event.key)) {
			return;
		}

		const activeElement = document.activeElement as HTMLElement;
		if (!JSDialog.IsTextInputField(activeElement)) {
			KeyboardListNavigation(event, activeElement);
		}
	});
};
