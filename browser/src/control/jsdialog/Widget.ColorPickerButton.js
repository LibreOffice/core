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
 * JSDialog.ColorPickerButton - split button with last color preview which opens color picker
 */

/* global app JSDialog */

function parseHexColor(color) {
	if (color === 'transparent') return -1;
	else return parseInt('0x' + color);
}

function getDefaultColorForCommand(command) {
	if (command == '.uno:CharBackColor') return -1;
	else if (command == '.uno:BackgroundColor') return -1;
	return 0;
}

/// returns string, if color is transparent we get -1
function toHexColor(colorInt) {
	var colorInt = parseInt(colorInt);
	if (colorInt < 0) return '' + colorInt;

	var colorHex = colorInt.toString(16);

	while (colorHex != '#' && colorHex.length < 6) colorHex = '0' + colorHex;

	if (colorHex[0] != '#') colorHex = '#' + colorHex;

	return colorHex;
}

JSDialog.getCurrentColor = function (data, builder) {
	var selectedColor = builder.map['stateChangeHandler'].getItemValue(
		data.command,
	);

	if (!selectedColor || selectedColor < 0)
		selectedColor = builder._getUnoStateForItemId(data.id, builder);

	if (!selectedColor || selectedColor < 0)
		selectedColor = getDefaultColorForCommand(data.command);

	return toHexColor(parseInt(selectedColor));
};

JSDialog.sendColorCommand = function (builder, data, color, themeData) {
	var gradientItem;

	// complex color properties

	if (data.id === 'fillgrad1') {
		gradientItem =
			builder.map['stateChangeHandler'].getItemValue('.uno:FillGradient');
		gradientItem.startcolor = color;
		builder.map.sendUnoCommand(
			'.uno:FillGradient?FillGradientJSON:string=' +
				JSON.stringify(gradientItem),
		);
		return;
	} else if (data.id === 'fillgrad2') {
		gradientItem =
			builder.map['stateChangeHandler'].getItemValue('.uno:FillGradient');
		gradientItem.endcolor = color;
		builder.map.sendUnoCommand(
			'.uno:FillGradient?FillGradientJSON:string=' +
				JSON.stringify(gradientItem),
		);
		return;
	} else if (data.id === 'fillattr2') {
		gradientItem = builder.map['stateChangeHandler'].getItemValue(
			'.uno:FillPageGradient',
		);
		gradientItem.startcolor = color;
		builder.map.sendUnoCommand(
			'.uno:FillPageGradient?FillPageGradientJSON:string=' +
				JSON.stringify(gradientItem),
		);
		return;
	} else if (data.id === 'fillattr3') {
		gradientItem = builder.map['stateChangeHandler'].getItemValue(
			'.uno:FillPageGradient',
		);
		gradientItem.endcolor = color;
		builder.map.sendUnoCommand(
			'.uno:FillPageGradient?FillPageGradientJSON:string=' +
				JSON.stringify(gradientItem),
		);
		return;
	}

	// simple numeric color values

	if (data.id === 'fillattr') {
		data.command = '.uno:FillPageColor';
	} else if (data.id === 'LB_GLOW_COLOR') {
		data.id = 'GlowColor';
	} else if (data.id === 'LB_SHADOW_COLOR') {
		data.command = '.uno:FillShadowColor';
	}

	var params = {};
	var colorParameterID = data.command.replace('.uno:', '') + '.Color';
	var themeParameterID =
		data.command.replace('.uno:', '') + '.ComplexColorJSON';

	var parsed = parseHexColor(color);

	params[colorParameterID] = {
		type: 'long',
		value: parsed ? parsed : parseInt(color),
	};

	if (themeData != null) {
		params[themeParameterID] = {
			type: 'string',
			value: themeData,
		};
	}

	builder.map['stateChangeHandler'].setItemValue(
		data.command,
		params[colorParameterID].value,
	);
	builder.map.sendUnoCommand(data.command, params);
	app.colorLastSelection[data.command] = color;
};

JSDialog.colorPickerButton = function (parentContainer, data, builder) {
	var commandOverride =
		data.command === '.uno:Color' && builder.map.getDocType() === 'text';
	if (commandOverride) data.command = '.uno:FontColor';

	var titleOverride = builder._getTitleForControlWithId(data.id);
	if (titleOverride) data.text = titleOverride;

	data.id = data.id
		? data.id
		: data.command
			? data.command.replace('.uno:', '')
			: undefined;
	data.text = builder._cleanText(data.text);

	if (data.command) {
		var applyFunction = function (event) {
			event.preventDefault();
			event.stopPropagation();

			var colorToApply = app.colorLastSelection[data.command];
			if (!colorToApply || colorToApply === '#') return;

			var color =
				colorToApply.indexOf('#') === 0 ? colorToApply.substr(1) : colorToApply;

			JSDialog.sendColorCommand(builder, data, color);
		};

		// add menu id for dropdown
		if (data.id.indexOf(':ColorPickerMenu') === -1)
			data.id = data.id + ':ColorPickerMenu';
		data.noLabel = true;

		// make it a split button
		data.applyCallback = applyFunction;

		var menubutton = builder._controlHandlers['menubutton'](
			parentContainer,
			data,
			builder,
		);

		if (typeof menubutton === 'object') {
			window.L.DomUtil.addClass(
				menubutton.container,
				data.class ? data.class + ' has-colorpicker' : 'has-colorpicker',
			);

			var valueNode = window.L.DomUtil.create(
				'span',
				'selected-color',
				menubutton.button,
			);
			valueNode.addEventListener('click', applyFunction);

			var updateFunction = function () {
				if (app.colorLastSelection[data.command] !== undefined) {
					var selectedColor = app.colorLastSelection[data.command];
				} else {
					selectedColor = JSDialog.getCurrentColor(data, builder);
					app.colorLastSelection[data.command] = selectedColor;
				}

				if (parseInt(selectedColor) === -1) selectedColor = 'transparent';

				valueNode.style.backgroundColor =
					selectedColor[0] !== '#' && selectedColor !== 'transparent'
						? '#' + selectedColor
						: selectedColor;

				// Make sure the border around the color indicator is not too bright
				// when the color is black so to avoid weird contrast artifacts
				if (
					valueNode.style.backgroundColor == '#000000' ||
					valueNode.style.backgroundColor == 'rgb(0, 0, 0)'
				) {
					valueNode.style.borderColor = '#6a6a6a';
				} else {
					valueNode.style.borderColor = 'var(--color-border)';
				}

				// Update the button tooltip with the current color name
				if (menubutton.container && data.text) {
					var colorName = '';
					var hexColor = selectedColor;
					if (hexColor && hexColor[0] === '#') hexColor = hexColor.substr(1);
					if (hexColor && hexColor !== 'transparent' && window.app.colorNames) {
						var entry = window.app.colorNames.find(function (c) {
							return c.hexCode.toLowerCase() === hexColor.toLowerCase();
						});
						if (entry) colorName = entry.name;
					}
					var tooltip = data.text;
					if (colorName) tooltip += ' (' + colorName + ')';
					menubutton.container.setAttribute('data-cooltip', tooltip);
				}
			};

			builder.map.on(
				'commandstatechanged',
				function (e) {
					if (e.commandName === data.command) {
						updateFunction();
					}
				},
				this,
			);

			updateFunction();
		}
	}

	return false;
};
