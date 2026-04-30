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
 * JSDialog.CustomWidget - base dispatcher for client-rendered widgets.
 *
 * Routes by `customType` to the handler registered via
 * registerCustomWidgetHandler. The widget JSON shape is:
 *
 * {
 *     id: 'widgetid',
 *     type: 'customwidget',
 *     customType: 'widgettype',
 *     data: { ... }
 * }
 */

declare var JSDialog: any;

interface CustomWidgetJSON extends WidgetJSON {
	customType: string;
	data: any;
}

type CustomWidgetHandler = (
	parentContainer: Element,
	data: CustomWidgetJSON,
	builder: JSBuilder,
) => boolean;

const customWidgetHandlers: { [customType: string]: CustomWidgetHandler } = {};

function registerCustomWidgetHandler(
	customType: string,
	handler: CustomWidgetHandler,
): void {
	customWidgetHandlers[customType] = handler;
}

function _customWidgetControl(
	parentContainer: Element,
	data: CustomWidgetJSON,
	builder: JSBuilder,
): boolean {
	const customType = data.customType;
	if (!customType) {
		window.app.console.warn(
			'CustomWidget: missing customType for widget',
			data.id,
		);
		return false;
	}

	const handler = customWidgetHandlers[customType];
	if (!handler) {
		window.app.console.warn(
			'CustomWidget: no handler registered for customType:',
			customType,
		);
		return false;
	}

	return handler(parentContainer, data, builder);
}

JSDialog.customWidget = function (
	parentContainer: Element,
	data: CustomWidgetJSON,
	builder: JSBuilder,
): boolean {
	return _customWidgetControl(parentContainer, data, builder);
};

JSDialog.registerCustomWidgetHandler = registerCustomWidgetHandler;
