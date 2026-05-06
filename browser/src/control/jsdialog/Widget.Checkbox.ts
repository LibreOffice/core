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

declare var JSDialog: any;

// Map a JS value to a UNO type name for SDI dispatch. Add a case here
// when a compound command introduces a field of a new type. Numerics
// are intentionally absent: JS doesn't distinguish long/double, so a
// command with numeric fields needs an explicit type hint elsewhere.
function _unoType(value: any): string {
	switch (typeof value) {
		case 'boolean':
			return 'boolean';
		case 'string':
			return 'string';
		default:
			app.console.warn(
				'_unoType: unsupported field type "' +
					typeof value +
					'", falling back to "string" — PutValue decode will likely fail',
			);
			return 'string';
	}
}

// Build SDI args for a compound command whose top-level parameter is a
// Sequence<PropertyValue> (decoded by the core item's PutValue with
// nMemberId=0). Parameter name == command without ".uno:".
function _buildSequenceArgs(command: string, state: any): any {
	const paramName = command.replace(/^\.uno:/, '');
	const value: any = {};
	for (const key of Object.keys(state)) {
		value[key] = { type: _unoType(state[key]), value: state[key] };
	}
	return {
		[paramName]: { type: '[]com.sun.star.beans.PropertyValue', value },
	};
}

function _createCheckboxContainer(
	parentContainer: HTMLElement,
	data: CheckboxWidgetJSON,
	builder: JSBuilder,
): HTMLDivElement {
	const container = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-checkbox checkbutton',
		parentContainer,
	);
	container.id = data.id;
	return container;
}

function _createCheckboxControl(
	parentContainer: HTMLElement,
	data: CheckboxWidgetJSON,
	builder: JSBuilder,
): HTMLInputElement {
	const checkbox = window.L.DomUtil.create(
		'input',
		builder.options.cssClass + ' ui-checkbox-input',
		parentContainer,
	);
	checkbox.type = 'checkbox';
	checkbox.id = data.id + '-input';
	checkbox.tabIndex = '0';
	return checkbox;
}

function _createCheckboxLabel(
	parentContainer: HTMLElement,
	data: CheckboxWidgetJSON,
	builder: JSBuilder,
): HTMLLabelElement {
	const label = window.L.DomUtil.create(
		'label',
		builder.options.cssClass + ' ui-checkbox-label',
		parentContainer,
	);
	label.id = data.id + '-label';
	label.textContent = builder._cleanText(data.text);
	label.htmlFor = data.id + '-input';
	return label;
}

JSDialog.Checkbox = function (
	parentContainer: HTMLElement,
	data: CheckboxWidgetJSON,
	builder: JSBuilder,
) {
	const container = _createCheckboxContainer(parentContainer, data, builder);
	const checkbox = _createCheckboxControl(container, data, builder);
	let label: HTMLElement | null = null;
	if (data.text) label = _createCheckboxLabel(container, data, builder);
	else
		JSDialog.SetupA11yLabelForLabelableElement(
			parentContainer,
			checkbox,
			data,
			builder,
		);

	checkbox.addEventListener('change', () => {
		if (container.getAttribute('disabled') === 'true') return;

		if (data.command) {
			if (data.commandField) {
				// Compound write: flip one field of the current multi-field
				// state and send the whole object back. The id-side and
				// value-side names match because we're echoing what arrived
				// via commandstatechanged.
				const current = app.map['stateChangeHandler'].getItemValue(
					data.command,
				);
				if (current && typeof current === 'object') {
					const updated = {
						...current,
						[data.commandField]: checkbox.checked,
					};
					app.map.sendUnoCommand(
						data.command,
						_buildSequenceArgs(data.command, updated),
					);
				}
				return;
			}
			app.dispatcher.dispatch(data.command);
			return;
		}

		builder.callback(
			'checkbox',
			'change',
			container,
			checkbox.checked,
			builder,
		);
	});

	const setDisabled = (disable: boolean) => {
		if (disable) {
			container.setAttribute('disabled', 'true');

			checkbox.disabled = true;
			checkbox.setAttribute('aria-disabled', 'true');
		} else {
			container.removeAttribute('disabled');

			checkbox.disabled = false;
			checkbox.removeAttribute('aria-disabled');
		}
	};

	setDisabled(data.enabled === false);

	JSDialog.SynchronizeDisabledState(
		container,
		[checkbox, label].filter(Boolean),
	); // filter(Boolean) removes nulls

	const toggleFunction = () => {
		if (container.getAttribute('disabled') === 'true') return;

		// Compound command read: data.commandField names a property within
		// the command's multi-field state object. Else legacy boolean-string
		// state. Else the static `data.checked`.
		if (data.command) {
			const state = app.map['stateChangeHandler'].getItemValue(data.command);
			if (data.commandField) {
				if (state && typeof state === 'object')
					$(checkbox).prop('checked', !!state[data.commandField]);
			} else if (state === 'true') {
				$(checkbox).prop('checked', true);
			} else if (state === 'false') {
				$(checkbox).prop('checked', false);
			}
		} else if (data.checked === true) {
			$(checkbox).prop('checked', true);
		} else if (data.checked) {
			$(checkbox).prop('checked', false);
		}
	};

	toggleFunction();

	app.map.on(
		'commandstatechanged',
		function (e: any) {
			if (e.commandName === data.command) {
				toggleFunction();
				setDisabled(e.disabled || e.state == 'disabled');
			}
		},
		this,
	);

	if (data.hidden) $(checkbox).hide();
	return false;
};
