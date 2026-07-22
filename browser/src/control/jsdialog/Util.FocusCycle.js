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
 * JSDialog.FocusCycle - focus related functions
 */

/* global app JSDialog $ cool */

function isAnyInputFocused() {
	if (!app.map)
		return false;

	if (app.map.hasFocus())
		return false;

	var hasTunneledDialogOpened = app.map.dialog ? app.map.dialog.hasOpenedDialog() : false;
	var hasJSDialogOpened = app.map.jsdialog ? app.map.jsdialog.hasDialogOpened() : false;
	var hasJSDialogFocused = window.L.DomUtil.hasClass(document.activeElement, 'jsdialog');
	var commentHasFocus = cool.Comment.isAnyFocus();
	var inputHasFocus = $('input:focus').length > 0 || $('textarea.jsdialog:focus').length > 0;

	return hasTunneledDialogOpened || hasJSDialogOpened || hasJSDialogFocused
		|| commentHasFocus || inputHasFocus;
}

function getFocusableElements(container) {
	if (!container)
		return null;

	var ret = container.querySelectorAll('[tabIndex="0"]:not(.jsdialog-begin-marker, .jsdialog-end-marker):not([disabled]):not(.hidden)');
	if (!ret.length)
		ret = container.querySelectorAll('input:not([disabled]):not(.hidden)');
	if (!ret.length)
		ret = container.querySelectorAll('textarea:not([disabled]):not(.hidden)');
	if (!ret.length)
		ret = container.querySelectorAll('select:not([disabled]):not(.hidden)');
	if (!ret.length)
		ret = container.querySelectorAll('button:not([disabled]):not(.hidden)');

	ret = Array.from(ret).filter(function(elem) { return elem.checkVisibility() });

	return ret;
}

// Utility function to check if an element is focusable
// This function different from getFocusableElements. This only checks, if the current element is focusable or not.
function isFocusable(element) {
	if (!element) return false;

	// Check if element is focusable (e.g. input, button, link etc.)
	const focusableElements = [
		'a[href]:not([disabled]):not(.hidden)',
		'button:not([disabled]):not(.hidden)',
		'textarea:not([disabled]):not(.hidden)',
		'input[type="text"]:not([disabled]):not(.hidden)',
		'input[type="search"]:not([disabled]):not(.hidden)',
		'input:not([type]):not([disabled]):not(.hidden)', // no explicit type defaults to text but the above doesn't catch it
		'input[type="radio"]:not([disabled]):not(.hidden)',
		'input[type="checkbox"]:not([disabled]):not(.hidden)',
		'select:not([disabled]):not(.hidden)',
		'[tabindex]:not([tabindex="-1"]):not(.jsdialog-begin-marker):not(.jsdialog-end-marker):not([disabled]):not(.hidden)',
		'[role="radiogroup"] [role="radio"]:not([disabled]):not(.hidden)',
		'[role="listbox"] [role="option"]:not([disabled]):not(.hidden)',
	];

	return focusableElements.some((selector) => element.matches(selector));
}

/// close tab focus switching in cycle inside container
function makeFocusCycle(container, failedToFindFocusFunc) {
	var beginMarker = window.L.DomUtil.create('div', 'jsdialog autofilter jsdialog-begin-marker');
	var endMarker = window.L.DomUtil.create('div', 'jsdialog autofilter jsdialog-end-marker');

	beginMarker.tabIndex = 0;
	endMarker.tabIndex = 0;

	container.insertBefore(beginMarker, container.firstChild);
	container.appendChild(endMarker);

	container.addEventListener('focusin', function(event) {
		if (event.target == endMarker) {
			var focusables = getFocusableElements(container);
			if (focusables && focusables.length) {
				focusables[0].focus();
				return;
			}
		} else if (event.target == beginMarker) {
			var focusables = getFocusableElements(container);
			if (focusables && focusables.length) {
				focusables[focusables.length - 1].focus();
				return;
			}
		}

		if ((event.target == endMarker || event.target == beginMarker) && failedToFindFocusFunc)
			failedToFindFocusFunc();
	});
}

function findFocusableElement(element, direction) {
	// check if no element present the just return null
	if (!element) return null;

	// Check the current element if it is focusable
	if (isFocusable(element)) return element;

	// Check if sibling is focusable or contains focusable elements
	const focusableInSibling = findFocusableWithin(element, direction);
	if (focusableInSibling) return focusableInSibling;

	return findNextFocusableSiblingElement(element, direction);
}

// Helper function to find the first focusable element within an element
function findFocusableWithin(element, direction){
	const focusableElements = Array.from(element.querySelectorAll('*'));
	return direction === 'next'
		? (focusableElements.find(isFocusable))
		: (focusableElements.reverse().find(isFocusable));
}

function findNextFocusableSiblingElement(element, direction) {
	// Depending on the direction, find the next or previous sibling
	const sibling =
		direction === 'next'
			? element.nextElementSibling
			: element.previousElementSibling;

	// Recursively check the next or previous sibling of the current sibling is focusable
	return findFocusableElement(sibling, direction);
}

// Helper function to find the current active element is a input TEXTAREA
function isTextInputField(currentActiveElement){
	return (currentActiveElement.tagName === 'INPUT' && (currentActiveElement.type === 'text' || currentActiveElement.type === 'search')) || currentActiveElement.tagName === 'TEXTAREA';
}

// Find focusable parent element using DOM traversal
function findFocusableParent(container, currentElement, element, arrowUp) {
	if (!element)
		return null;
	else if (element.tagName === 'NAV' && arrowUp) {
		return element;
	}
	else if (element.tabIndex === -1 || element.tagName == 'A') {
		return findFocusableParent(container, currentElement, element.parentNode, arrowUp);
	}
	else if (container.contains(element) && currentElement !== element && !currentElement.contains(element) && !element.disabled) {
		return element;
	}
	else
		return null;
}

// Ray cast to find next element in a given direction
function rayCastToNextElement(container, currentElement, boundingRectangle, startX, startY, diffX, diffY, arrowUp) {
	let count = 0;
	let foundElement;
	while (count <= 60) {
		count++;
		startX += diffX;
		startY += diffY;

		foundElement = document.elementFromPoint(startX, startY);
		foundElement = findFocusableParent(container, currentElement, foundElement, arrowUp);
		if (foundElement) break;

		// If we are here, we'll try secondary and tertiary rays.
		if (diffX === 0) {
			foundElement = document.elementFromPoint(boundingRectangle.left, startY);
			foundElement = findFocusableParent(container, currentElement, foundElement, arrowUp);
			if (foundElement) break;

			foundElement = document.elementFromPoint(boundingRectangle.right, startY);
			foundElement = findFocusableParent(container, currentElement, foundElement, arrowUp);
			if (foundElement) break;
		}
		else if (diffY === 0) {
			foundElement = document.elementFromPoint(startX, boundingRectangle.top);
			foundElement = findFocusableParent(container, currentElement, foundElement, arrowUp);
			if (foundElement) break;

			foundElement = document.elementFromPoint(startX, boundingRectangle.bottom);
			foundElement = findFocusableParent(container, currentElement, foundElement, arrowUp);
			if (foundElement) break;
		}
	}

	if (count === 60)
		return null;
	else
		return foundElement;
}

// Find next element in container using ray casting
function findNextElementInContainer(container, currentElement, direction) {
	let boundingRectangle = currentElement.getBoundingClientRect();
	let startX = boundingRectangle.left + (boundingRectangle.right - boundingRectangle.left) / 2;
	let startY = boundingRectangle.top + (boundingRectangle.bottom - boundingRectangle.top) / 2;

	let diffX = 0;
	let diffY = 0;

	// Ray casting sensitivity for spatial navigation
	var rayCastingSensitivity = 10; // Pixels

	if (direction === 'ArrowLeft' || direction === 'ArrowRight')
		diffX = direction === 'ArrowRight' ? rayCastingSensitivity : (rayCastingSensitivity * -1);

	if (direction === 'ArrowUp' || direction === 'ArrowDown')
		diffY = direction === 'ArrowDown' ? rayCastingSensitivity : (rayCastingSensitivity * -1);

	return rayCastToNextElement(container, currentElement, boundingRectangle, startX, startY, diffX, diffY, direction === 'ArrowUp');
}

JSDialog.IsAnyInputFocused = isAnyInputFocused;
JSDialog.GetFocusableElements = getFocusableElements;
JSDialog.MakeFocusCycle = makeFocusCycle;
JSDialog.FindFocusableElement = findFocusableElement;
JSDialog.FindFocusableWithin = findFocusableWithin;
JSDialog.FindNextFocusableSiblingElement = findNextFocusableSiblingElement;
JSDialog.IsFocusable = isFocusable;
JSDialog.IsTextInputField = isTextInputField;
JSDialog.FindFocusableParent = findFocusableParent;
JSDialog.RayCastToNextElement = rayCastToNextElement;
JSDialog.FindNextElementInContainer = findNextElementInContainer;
