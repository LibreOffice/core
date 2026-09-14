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
 * JSDialog.Toolitem - collection of items which can be inserted into toolbox in core
 *                     or used as standalone buttons in JSDialogs. They fire command
 *                     on user interaction (uno command, or dispatch command) and get
 *                     state from the state change multiplexer so we can show selected
 *                     or disabled state.
 */

declare var JSDialog: any;

function _mapDispatchToolItem(
	parentContainer: HTMLElement,
	data: ToolItemWidgetJSON,
	builder: JSBuilder,
) {
	if (!data.command) data.command = data.id;

	if (data.id && !app.isExportEntryVisible(data.id)) return false;

	const backupInlineText = builder.options.useInLineLabelsForUnoButtons;
	if (data.inlineLabel !== undefined)
		builder.options.useInLineLabelsForUnoButtons = data.inlineLabel;

	const control = builder._unoToolButton(parentContainer, data, builder);

	if (data.inlineLabel !== undefined)
		builder.options.useInLineLabelsForUnoButtons = backupInlineText;

	$(control.button).unbind('click');
	$(control.label).unbind('click');

	if (!builder.map.isLockedItem(data)) {
		const handlePressAndHold = (data: ToolItemWidgetJSON) => {
			const scrollingInterval = setInterval(() => {
				app.dispatcher.dispatch(data.command);
			}, 100);

			$(document).one('mouseup', () => {
				clearInterval(scrollingInterval);
			});
		};

		// Handle "Press+Hold" Event
		if (data.pressAndHold) {
			$(control.container).on('mousedown', (e) => {
				if (
					e.button !== 0 || // Only handle left mouse button
					control.container.getAttribute('disabled') !== null
				)
					return;

				const pressAndHoldTimer = setTimeout(() => {
					handlePressAndHold(data);
				}, 500);

				$(document).one('mouseup', () => {
					clearTimeout(pressAndHoldTimer);
				});
			});
		}

		$(control.container).click(() => {
			if (control.container.getAttribute('disabled') === null)
				app.dispatcher.dispatch(data.command);
		});
	}

	builder._preventDocumentLosingFocusOnClick(control.container);
}

function _mapBigDispatchToolItem(
	parentContainer: HTMLElement,
	data: ToolItemWidgetJSON,
	builder: JSBuilder,
) {
	if (!data.command) data.command = data.id;

	const noLabels = builder.options.noLabelsForUnoButtons;
	builder.options.noLabelsForUnoButtons = false;

	const control = builder._unoToolButton(parentContainer, data, builder);

	builder.options.noLabelsForUnoButtons = noLabels;

	$(control.button).unbind('click');
	$(control.label).unbind('click');

	if (!builder.map.isLockedItem(data)) {
		$(control.container).click((e: any) => {
			e.preventDefault();
			app.dispatcher.dispatch(data.command);
		});
	}

	builder._preventDocumentLosingFocusOnClick(control.container);
}

JSDialog.dispatchToolitem = function (
	parentContainer: HTMLElement,
	data: ToolItemWidgetJSON,
	builder: JSBuilder,
) {
	return _mapDispatchToolItem(parentContainer, data, builder);
};

JSDialog.bigDispatchToolitem = function (
	parentContainer: HTMLElement,
	data: ToolItemWidgetJSON,
	builder: JSBuilder,
) {
	return _mapBigDispatchToolItem(parentContainer, data, builder);
};
