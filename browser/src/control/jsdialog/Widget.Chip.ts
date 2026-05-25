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
 * JSDialog.Chip - a single rounded pill with a primary clickable body
 * (icon + label, selectable via aria-pressed) and zero or more trailing
 * action buttons that sit inside the same pill.
 *
 * Example JSON:
 * {
 *     id: 'my-chip',
 *     type: 'chip',
 *     text: 'Zen',
 *     icon: '🧘',                          // optional: emoji or lc_*.svg
 *     pressed: false,                      // optional: drives aria-pressed
 *     enabled: true,
 *     aria: { label: 'Zen tone' },
 *     actions: [                           // optional: 0..n trailing actions
 *         {
 *             id: 'my-chip-edit',
 *             text: '✎',                   // or image: 'lc_edit.svg'
 *             aria: { label: 'Edit' },
 *         },
 *     ],
 * }
 *
 * Clicking the main body fires
 *   builder.callback('chip', 'click', container, undefined, builder)
 * where `container` is the chip's wrapper <div> and carries the chip's id.
 * Each action button fires
 *   builder.callback('pushbutton', 'click', { id: action.id }, undefined, builder)
 * so existing consumers that route by id-prefix work unchanged.
 *
 * NOTE: the action-callback's third argument is a synthetic stub - only
 * the `id` field is set. Action consumers should route purely by id and
 * MUST NOT treat that argument as a real DOM container.
 */

declare var JSDialog: any;

function _chipIsLCIcon(str: string): boolean {
	return typeof str === 'string' && str.indexOf('lc_') === 0;
}

JSDialog.chip = function (
	parentContainer: Element,
	data: ChipWidgetJSON,
	builder: JSBuilder,
) {
	const container = window.L.DomUtil.create(
		'div',
		'ui-chip ' + builder.options.cssClass,
		parentContainer,
	);
	container.id = data.id;

	const main = window.L.DomUtil.create(
		'button',
		'ui-chip-main ' + builder.options.cssClass,
		container,
	) as HTMLButtonElement;
	main.type = 'button';
	main.id = data.id + '-main';
	main.setAttribute('tabindex', '0');
	main.setAttribute('aria-pressed', data.pressed === true ? 'true' : 'false');

	if (data.icon) {
		const iconSpan = window.L.DomUtil.create('span', 'ui-chip-icon', main);
		if (_chipIsLCIcon(data.icon)) {
			const img = window.L.DomUtil.create('img', '', iconSpan);
			app.LOUtil.setImage(img, data.icon, builder.map);
			img.alt = '';
		} else {
			iconSpan.textContent = data.icon;
		}
	}

	const textSpan = window.L.DomUtil.create('span', 'ui-chip-text', main);
	textSpan.textContent = builder._cleanText(data.text || '');

	const ariaLabel = data.aria && data.aria.label;
	if (ariaLabel) {
		main.setAttribute('aria-label', ariaLabel);
	}

	const isDisabled = data.enabled === false;
	if (isDisabled) {
		container.setAttribute('disabled', 'true');
		main.setAttribute('disabled', 'true');
		main.setAttribute('aria-disabled', 'true');
	}

	main.onclick = () => {
		if (isDisabled) return;
		builder.callback('chip', 'click', container, undefined, builder);
	};

	if (data.actions && data.actions.length > 0) {
		for (const action of data.actions) {
			const actionBtn = window.L.DomUtil.create(
				'button',
				'ui-chip-action ' + builder.options.cssClass,
				container,
			) as HTMLButtonElement;
			actionBtn.type = 'button';
			actionBtn.id = action.id;
			actionBtn.setAttribute('tabindex', '0');

			if (action.image) {
				const img = window.L.DomUtil.create('img', '', actionBtn);
				if (_chipIsLCIcon(action.image)) {
					app.LOUtil.setImage(img, action.image, builder.map);
				} else {
					img.src = action.image;
				}
				img.alt = '';
			} else if (action.text) {
				actionBtn.textContent = builder._cleanText(action.text);
			}

			if (action.aria && action.aria.label) {
				actionBtn.setAttribute('aria-label', action.aria.label);
			}

			const actionDisabled = action.enabled === false;
			if (actionDisabled) {
				actionBtn.setAttribute('disabled', 'true');
				actionBtn.setAttribute('aria-disabled', 'true');
			}

			const actionId = action.id;
			actionBtn.onclick = () => {
				if (actionDisabled) return;
				builder.callback(
					'pushbutton',
					'click',
					{ id: actionId },
					undefined,
					builder,
				);
			};
		}
	}

	return false;
};
