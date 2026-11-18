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
 * JSDialog.Progressbar - progress bar widget
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'progressbar',
 *     value: 30,
 *     maxValue: 100
 * }
 */

/* global JSDialog $ */

JSDialog.progressbar = function (parentContainer, data, builder) {
	const div = window.L.DomUtil.createWithId('div', data.id, parentContainer);
	window.L.DomUtil.addClass(div, 'ui-progressbar');
	window.L.DomUtil.addClass(div, builder.options.cssClass);

	const progressbar = window.L.DomUtil.create('progress', builder.options.cssClass, div);
	progressbar.id = data.id + '-progress';
	progressbar.tabIndex = '0';

	JSDialog.SetupA11yLabelForLabelableElement(parentContainer, progressbar, data, builder);

	if (data.value !== undefined)
		progressbar.value = data.value;
	else
		progressbar.value = 0;

	if (data.maxValue !== undefined)
		progressbar.max = data.maxValue;
	else
		progressbar.max = 100;

	const isComplete = progressbar.value === progressbar.max;

	if (data.infinite === true && !isComplete) {
		window.L.DomUtil.addClass(div, 'infinite');
		div.setAttribute('data-progress-type', 'infinite');
	} else {
		div.removeAttribute('data-progress-type');
		div.classList.remove('infinite');
	}

	if (data.enabled === 'false' || data.enabled === false) {
		$(progressbar).prop('disabled', true);
	}

	if (data.hidden)
		$(progressbar).hide();

	return false;
};
