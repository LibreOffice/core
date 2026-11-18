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
 * JSDialog.ButtonBox - button box widget which groups buttons, setups the layout,
 *                      can be found on the bottom of dialog with standard buttons like: ok, cancel
 */

declare var JSDialog: any;

JSDialog.buttonBox = function (
	parentContainer: Element,
	data: ContainerWidgetJSON,
	builder: JSBuilder,
) {
	var container = window.L.DomUtil.create(
		'div',
		builder.options.cssClass +
			' ui-button-box ' +
			(data.layoutstyle ? data.layoutstyle : ''),
		parentContainer,
	);
	container.id = data.id;

	var leftAlignButtons = [];
	var rightAlignButton = [];

	if (data.children) {
		for (const i in data.children) {
			const child = data.children[i];
			if (child.id === 'help') leftAlignButtons.push(child);
			else rightAlignButton.push(child);
		}
	}

	var left = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-button-box-left',
		container,
	);

	for (const i in leftAlignButtons) {
		const child = leftAlignButtons[i];
		if (builder._controlHandlers[child.type]) {
			builder._controlHandlers[child.type](left, child, builder);
			builder.postProcess(left, child);
		}
	}

	var right = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-button-box-right',
		container,
	);
	if (data.layoutstyle && data.layoutstyle === 'end')
		window.L.DomUtil.addClass(container, 'end');

	for (const i in rightAlignButton) {
		const child = rightAlignButton[i];
		if (builder._controlHandlers[child.type]) {
			builder._controlHandlers[child.type](right, child, builder);
			builder.postProcess(right, child);
		}
	}

	if (data.vertical === true) {
		left.style.display = 'grid';
		left.style.margin = 'auto';
		right.style.display = 'grid';
		right.style.margin = 'auto';
	}

	return false;
};
