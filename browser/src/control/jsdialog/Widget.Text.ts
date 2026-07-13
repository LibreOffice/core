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
 * JSDialog.StaticText - static text rendering with span
 */

declare var JSDialog: any;

function staticTextControl(
	parentContainer: HTMLElement,
	data: TextWidget,
	builder: JSBuilder,
) {
	var statictext = window.L.DomUtil.create(
		'span',
		builder.options.cssClass + ' static-label',
		parentContainer,
	);

	if (data.text) statictext.textContent = builder._cleanText(data.text);
	else if (data.html) statictext.innerHTML = app.LOUtil.sanitize(data.html);

	statictext.id = data.id;
	if (data.style && data.style.length) {
		window.L.DomUtil.addClass(statictext, data.style);
	} else {
		window.L.DomUtil.addClass(statictext, 'ui-text');
	}

	if (data.hidden) $(statictext).hide();

	if (data.allyRole) statictext.setAttribute('role', data.allyRole);
	if (data.ariaLive) statictext.setAttribute('aria-live', data.ariaLive);

	return false;
}

JSDialog.StaticText = function (
	parentContainer: HTMLElement,
	data: TextWidget,
	builder: JSBuilder,
) {
	return staticTextControl(parentContainer, data, builder);
};
