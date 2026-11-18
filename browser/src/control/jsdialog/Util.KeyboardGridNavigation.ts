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
 * JSDialog.KeyboardGridNavigation - handles keyboard-based focus and selection in grid-based UI components
 */

declare var JSDialog: any;

function handleKeyboardNavigation(
	event: KeyboardEvent,
	currentElement: HTMLElement,
	parentContainer: HTMLElement,
) {
	switch (event.key) {
		case 'ArrowRight':
			moveFocus(parentContainer, currentElement, 'next', 'horizontal');
			event.preventDefault();
			break;
		case 'ArrowLeft':
			moveFocus(parentContainer, currentElement, 'previous', 'horizontal');
			event.preventDefault();
			break;
		case 'ArrowDown':
			moveFocus(parentContainer, currentElement, 'next', 'vertical');
			event.preventDefault();
			break;
		case 'ArrowUp':
			moveFocus(parentContainer, currentElement, 'previous', 'vertical');
			event.preventDefault();
			break;
		default:
			break;
	}
}

function moveFocus(
	parentContainer: HTMLElement,
	currentElement: HTMLElement,
	direction: 'next' | 'previous',
	axis: 'horizontal' | 'vertical',
) {
	const focusableElements = Array.from(
		JSDialog.GetFocusableElements(parentContainer),
	) as HTMLElement[];

	const [currentRow, currentColumn] = getRowColumn(currentElement);

	let targetRow = currentRow;
	let targetColumn = currentColumn;

	if (axis === 'horizontal') {
		if (direction === 'next') {
			targetColumn++;
			if (
				!focusableElements.find(
					(el) =>
						getRowColumn(el)[0] === currentRow &&
						getRowColumn(el)[1] === targetColumn,
				)
			) {
				targetColumn = 0;
			}
		} else {
			targetColumn--;
			if (targetColumn < 0) {
				targetColumn =
					focusableElements.filter((el) => getRowColumn(el)[0] === currentRow)
						.length - 1;
			}
		}
	} else if (axis === 'vertical') {
		if (direction === 'next') {
			targetRow++;
		} else {
			targetRow--;
		}

		// If the target column does not exist in the target row, move to the last column in the target row
		const elementsInTargetRow = focusableElements.filter(
			(el) => getRowColumn(el)[0] === targetRow,
		);
		if (elementsInTargetRow.length > 0) {
			if (targetColumn >= elementsInTargetRow.length) {
				targetColumn = elementsInTargetRow.length - 1;
			}
		}
	}

	const updateAriaSelected = (elem: HTMLElement, value: string) => {
		if (elem.hasAttribute('aria-selected')) {
			elem.setAttribute('aria-selected', value);
		}
	};

	const targetElement = focusableElements.find(
		(el) =>
			getRowColumn(el)[0] === targetRow && getRowColumn(el)[1] === targetColumn,
	);

	updateAriaSelected(currentElement, 'false');

	if (targetElement) {
		updateAriaSelected(targetElement, 'true');
		targetElement.focus();
	} else {
		// Handle edge cases by moving to adjacent sibling elements if no exact match is found
		const potentialCurrentElement =
			axis === 'vertical' ? parentContainer : currentElement;

		const nextElement =
			direction === 'next'
				? JSDialog.FindFocusableElement(
						potentialCurrentElement.nextElementSibling,
						'next',
					)
				: JSDialog.FindFocusableElement(
						potentialCurrentElement.previousElementSibling,
						'previous',
					);
		if (nextElement) {
			updateAriaSelected(nextElement, 'true');
			nextElement.focus();
		}
	}
}

function getRowColumn(element: HTMLElement): [number, number] {
	const index = element.getAttribute('index');
	if (!index) return [-1, -1]; // we will never have this kind of index this is why we are passing negative values here
	const [row, column] = index.split(':').map(Number);
	return [row, column];
}

JSDialog.KeyboardGridNavigation = function (container: HTMLElement) {
	container.addEventListener('keydown', (event: KeyboardEvent) => {
		const activeElement = document.activeElement as HTMLElement;
		if (!JSDialog.IsTextInputField(activeElement)) {
			handleKeyboardNavigation(
				event,
				activeElement,
				activeElement.parentElement,
			);
		}
	});
};
