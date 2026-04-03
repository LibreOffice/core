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
 * JSDialog.fixedtextControl - fixed label
 */

declare var JSDialog: any;

JSDialog.fixedtextControl = function (
	parentContainer: HTMLElement,
	data: TextWidget,
	builder: JSBuilder,
) {
	// Check if this label should render as static content(i.e. span) instead of interactive label
	if (
		!data.labelFor ||
		!JSDialog.GetFormControlTypesInLO().has(data.labelForType)
	) {
		return JSDialog.StaticText(parentContainer, data, builder);
	}

	const fixedtext = window.L.DomUtil.create(
		'label',
		builder.options.cssClass,
		parentContainer,
	) as HTMLLabelElement;
	fixedtext.htmlFor = data.labelFor + '-input';
	fixedtext.id = data.id;

	if (data.text) fixedtext.textContent = builder._cleanText(data.text);
	else if (data.html) fixedtext.innerHTML = data.html;

	if (data.xalign) {
		fixedtext.style.cssText = 'text-align:' + data.xalign + ';';
	}

	const accKey = builder._getAccessKeyFromText(data.text);
	builder._stressAccessKey(fixedtext, accKey);

	const updateLabelForAttribute = function (
		label: HTMLLabelElement,
		labelledControl: any,
	) {
		const isLabelable = JSDialog.GetFormControlTypesInCO().has(
			labelledControl.nodeName,
		);
		const isHiddenInput =
			labelledControl.nodeName === 'INPUT' && labelledControl.type === 'hidden';

		// For labelable element always use htmlFor
		if (isLabelable && !isHiddenInput) {
			labelledControl.removeAttribute('aria-labelledby');
			labelledControl.removeAttribute('aria-label');
			label.htmlFor = labelledControl.id;
			return;
		}

		// For non-labelable element or hidden input always use aria-labelledby
		labelledControl.setAttribute('aria-labelledby', label.id);
		label.removeAttribute('for');
	};

	app.layoutingService.appendLayoutingTask(function () {
		if (!data.labelFor) return;

		const labelledControl = document.getElementById(data.labelFor);
		if (labelledControl) {
			let target = labelledControl;

			const input = labelledControl.querySelector('input');
			if (input) target = input;

			const select = labelledControl.querySelector('select');
			if (select) target = select;

			builder._setAccessKey(target, accKey);
		}

		// we need to schedule it again as some elements are not yet available
		// i.e. pop-ups: Double click on Chart->Sidebar->Colors
		app.layoutingService.appendLayoutingTask(function () {
			if (!data.labelFor) return;

			const targetElement =
				document.getElementById(
					data.labelFor + '-input-' + builder.options.suffix,
				) ||
				document.getElementById(data.labelFor + '-input') ||
				document.getElementById(data.labelFor);

			// Reference label to target element correctly
			if (targetElement) updateLabelForAttribute(fixedtext, targetElement);
		});
	});

	if (data.style && data.style.length) {
		window.L.DomUtil.addClass(fixedtext, data.style);
	} else {
		window.L.DomUtil.addClass(fixedtext, 'ui-text');
	}

	if (data.hidden) $(fixedtext).hide();

	return false;
};
