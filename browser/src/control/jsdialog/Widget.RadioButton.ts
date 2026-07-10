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
 * JSDialog.RadioButton - radio button widget with label
 */

declare var JSDialog: any;

function radiobuttonControl(
	parentContainer: HTMLElement,
	data: RadioButtonWidget,
	builder: JSBuilder,
) {
	const container = window.L.DomUtil.createWithId(
		'div',
		data.id,
		parentContainer,
	);
	window.L.DomUtil.addClass(container, 'radiobutton ui-radiobutton');
	window.L.DomUtil.addClass(container, builder.options.cssClass);

	const radiobutton = window.L.DomUtil.create('input', '', container);
	radiobutton.type = 'radio';
	radiobutton.id = data.id + '-input';
	radiobutton.tabIndex = '0';

	if (data.image) {
		const image = window.L.DomUtil.create('img', '', radiobutton);
		image.src = data.image;
		window.L.DomUtil.addClass(container, 'has-image');
	}

	if (data.group) radiobutton.name = data.group;

	const radiobuttonLabel = window.L.DomUtil.createWithId(
		'label',
		data.id + '-label',
		container,
	);
	radiobuttonLabel.textContent = builder._cleanText(data.text);
	radiobuttonLabel.htmlFor = data.id + '-input';

	const toggleFunction = () => {
		builder.callback(
			'radiobutton',
			'change',
			container,
			$(radiobutton).prop('checked'),
			builder,
		);
	};

	$(radiobuttonLabel).click(() => {
		if (radiobutton.hasAttribute('disabled')) return;

		$(radiobutton).prop('checked', true);
		toggleFunction.bind({ checked: true })();
	});

	const isDisabled = data.enabled === false;
	if (isDisabled) {
		radiobutton.setAttribute('disabled', 'disabled');
		radiobutton.setAttribute('aria-disabled', 'true');
		container.setAttribute('disabled', 'true');
	}

	// Runtime 'enable'/'disable' actions toggle the 'disabled' attribute on
	// the container (data.id), but the actual state lives on the inner
	// <input> (data.id + '-input'). Mirror the container's state onto the
	// input (and label) so set_sensitive() from the core actually takes
	// effect - otherwise a disabled radio button can never be re-enabled.
	JSDialog.SynchronizeDisabledState(container, [radiobutton, radiobuttonLabel]);

	if (data.checked === true) $(radiobutton).prop('checked', true);

	radiobutton.addEventListener('change', toggleFunction);

	if (data.hidden) $(radiobutton).hide();

	return false;
}

JSDialog.RadioButton = function (
	parentContainer: HTMLElement,
	data: RadioButtonWidget,
	builder: JSBuilder,
) {
	return radiobuttonControl(parentContainer, data, builder);
};
