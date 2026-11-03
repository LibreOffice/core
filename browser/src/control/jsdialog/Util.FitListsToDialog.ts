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
 * Util.FitListsToDialog - makes the lists of a dialog shorter when the dialog is taller than the
 * window, so the lists scroll instead of the whole dialog.
 */

declare var JSDialog: any;

// The variable holds the tallest height that any list in the dialog may take, in pixels. Unset,
// a list takes the rows it asks for, up to 40% of the window.
const listMaxHeightVariable = '--dialog-list-max-height';

// A list is never shorter than this, as set in jsdialogs.css.
const minimumListHeight = 150;

// The lists are left at a cap at most this many pixels below the tallest cap that fits.
const searchPrecision = 4;

// How many pixels the content is taller than the dialog, or zero or less when it fits.
function overflowOf(content: HTMLElement): number {
	return content.scrollHeight - content.clientHeight - 1;
}

JSDialog.FitListsToDialog = function (content: HTMLElement): void {
	const lists = content.querySelectorAll<HTMLElement>(
		'.ui-treeview:not(.sidebar)',
	);
	if (!lists.length) return;

	content.style.removeProperty(listMaxHeightVariable);
	let tooTallOverflow = overflowOf(content);
	if (tooTallOverflow <= 0) return;

	let tallestList = 0;
	lists.forEach((list) => {
		tallestList = Math.max(tallestList, list.offsetHeight);
	});
	if (tallestList <= minimumListHeight) return;

	// Every list in the dialog gets the same cap, so lists side by side, above each other or on
	// different tab pages all count. Reading the overflow after a new cap lays out the page, so
	// the search below reads it as few times as it can.
	const overflowAt = (height: number): number => {
		content.style.setProperty(listMaxHeightVariable, height + 'px');
		return overflowOf(content);
	};

	// Lowering the cap by one pixel shrinks the overflow by one pixel for each list in the column
	// of lists that sets the height of the dialog. So the number of lists is the fastest rate, and
	// a first step at that rate stays above the tallest cap that fits. Each further step uses the
	// rate measured between the last two caps, which is exact while the same lists are capped.
	let tooTall = tallestList;
	let rate = lists.length;
	let fits = 0;
	while (!fits) {
		const cap =
			rate > 0
				? Math.max(
						minimumListHeight,
						Math.floor(tooTall - tooTallOverflow / rate),
					)
				: minimumListHeight;
		const capOverflow = overflowAt(cap);
		if (capOverflow <= 0) {
			fits = cap;
		} else if (cap === minimumListHeight) {
			// The dialog does not fit even with the shortest lists, so they stay at that height.
			return;
		} else {
			// When the overflow did not shrink, something other than the lists sets the height of
			// the dialog, and the next step goes straight to the shortest lists.
			const measured = (tooTallOverflow - capOverflow) / (tooTall - cap);
			rate = Math.min(measured, lists.length);
			tooTall = cap;
			tooTallOverflow = capOverflow;
		}
	}

	// Once the cap reaches lists that were shorter than it, those shrink too and the overflow
	// falls faster, so the last step can land below the tallest cap that fits. The first check is
	// just above the cap that fits, as a step usually lands on the answer, then the range halves.
	let probe = fits + searchPrecision;
	while (tooTall - fits > searchPrecision) {
		if (overflowAt(probe) > 0) tooTall = probe;
		else fits = probe;
		probe = Math.floor((fits + tooTall) / 2);
	}
	content.style.setProperty(listMaxHeightVariable, fits + 'px');
};
