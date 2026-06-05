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

/**
 * Widget.NewSlideLayoutEntry.ts
 *
 * A JSDialog "json" widget for rendering the slide layout presets
 * and the "New Slide Layout" link.
 */
declare var JSDialog: any;

function createSlideLayoutEntryWidget(
	data: NewSlideLayoutEntryWidgetJSON,
	builder: any,
): HTMLElement {
	const container = document.createElement('div');
	container.className = 'slidelayout-popup-container';
	// Build the grid first
	const gridData = data.gridContent; // The grid from generateLayoutPopupGrid
	const gridContainer = document.createElement('div');
	builder.build(gridContainer, [gridData], false);
	container.appendChild(gridContainer);

	const hr = document.createElement('hr');
	hr.className = 'jsdialog ui-separator horizontal';
	container.appendChild(hr);

	const button = document.createElement('div');
	button.className = 'slidelayout-button';
	button.id = 'newcanvasslide';
	button.textContent = _('Overview');
	button.setAttribute('role', 'button');
	button.setAttribute('tabindex', '0');
	button.addEventListener('click', () => {
		builder.map.sendUnoCommand('.uno:InsertCanvasSlide'); // or your command
		builder.callback('dialog', 'close', { id: data.id }, null);
	});
	container.appendChild(button);
	return container;
}

JSDialog.slideLayoutEntry = function (
	parentContainer: Element,
	data: NewSlideLayoutEntryWidgetJSON,
	builder: any,
) {
	const widget = createSlideLayoutEntryWidget(data, builder);
	parentContainer.appendChild(widget);
	return false;
};
