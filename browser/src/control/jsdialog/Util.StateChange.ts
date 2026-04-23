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
 * Util.StateChange - helper for defining enable / disable callback
 */

declare var JSDialog: any;

type StateChangeCallback = (enabled: boolean) => void;

function onStateChange(element: Element, callback: StateChangeCallback) {
	const enabledCallback = function (mutations: Array<MutationRecord>) {
		for (const i in mutations) {
			if (mutations[i].attributeName === 'disabled') {
				const htmlElement = mutations[i].target as HTMLElement;
				const enable = htmlElement.getAttribute('disabled') !== 'true';
				callback(enable);
			}
		}
	};

	const enableObserver = new MutationObserver(enabledCallback);
	enableObserver.observe(element, { attributeFilter: ['disabled'], attributeOldValue: true });
}

function moveFocusFromDisabledElement(widget: Element) {
	const dialog = widget.closest('.jsdialog-window');
	if (!dialog) return;
	const next = JSDialog.FindFocusableElement(widget.nextElementSibling, 'next');
	if (next) {
		next.focus();
		return;
	}
	const prev = JSDialog.FindFocusableElement(widget.previousElementSibling, 'prev');
	if (prev) {
		prev.focus();
		return;
	}
	const focusable = JSDialog.GetFocusableElements(dialog);
	if (focusable && focusable.length) {
		focusable[0].focus();
	}
}

function synchronizeDisabledState(source: Element, targets: Array<Element>) {
	const enabledCallback = function (enable: boolean) {
		app.layoutingService.appendLayoutingTask(() => {
		for (const i in targets) {
				if (enable) {
					targets[i].removeAttribute('disabled');
					targets[i].removeAttribute('aria-disabled');
				} else {
					const hadFocus = targets[i].contains(document.activeElement);
					targets[i].setAttribute('disabled', 'true');
					targets[i].setAttribute('aria-disabled', 'true');
					if (hadFocus)
						moveFocusFromDisabledElement(source);
				}
			}
		});
	};
	onStateChange(source, enabledCallback);
}

JSDialog.OnStateChange = onStateChange;
JSDialog.SynchronizeDisabledState = synchronizeDisabledState;
