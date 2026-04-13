// @ts-strict-ignore
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
 * JSDialog.ColorPicker - color picker for desktop
 */

declare var JSDialog: any;

interface ColorPaletteWidgetData {
	id: string;
	command: string;
}

interface ThemeColor {
	Value: string;
	Name: string;
	Data: {
		ThemeIndex: number;
		Transformations: { Type: string; Value: number }[];
	};
}

type ColorEntry = {
	hexCode: string;
	name: string;
};

type ColorItem = string;
type CoreColorPalette = Array<Array<{ Value: ColorItem }>>;
type ColorPalette = Array<Array<ColorItem>>;

function findColorName(hexCode: string): string {
	const color = window.app.colorNames.find(
		(c: ColorEntry) => c.hexCode.toLowerCase() === hexCode.toLowerCase(),
	);
	return color ? color.name : _('Unknown color');
}

function getCurrentPaletteName(): string {
	const palette = window.prefs.get('colorPalette');

	if (
		palette === undefined ||
		window.app.colorPalettes[palette] === undefined
	) {
		return 'StandardColors';
	}

	return palette;
}

// TODO: we don't need to use that format now - simplify?
function toW2Palette(corePalette: CoreColorPalette): ColorPalette {
	const pal: ColorPalette = [];

	if (!corePalette) return pal;

	for (let i = 0; i < corePalette.length; i++) {
		const row = [];
		for (let j = 0; j < corePalette[i].length; j++) {
			row.push(String(corePalette[i][j].Value).toUpperCase());
		}
		pal.push(row);
	}
	return pal;
}

function sanitizePaletteRow(row: string) {
	if (row !== undefined) {
		try {
			const json = JSON.parse(row);
			return json.filter((color: string | null) => color !== null);
		} catch (e) {
			console.error('Cannot parse palette row from cache: "' + row + '" :' + e);
		}
	}
	return null;
}

function generatePalette(paletteName: string) {
	const colorPalette = toW2Palette(
		window.app.colorPalettes[paletteName].colors,
	);
	const customColorRow = sanitizePaletteRow(window.prefs.get('customColor'));
	const recentRow = sanitizePaletteRow(window.prefs.get('recentColor'));

	if (customColorRow) colorPalette.push(customColorRow);
	else colorPalette.push(['F2F2F2', 'F2F2F2', 'F2F2F2', 'F2F2F2', 'F2F2F2']); // custom colors (up to 4)

	if (recentRow) {
		colorPalette.push(recentRow);
	} else {
		colorPalette.push([
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
			'F2F2F2',
		]); // recent colors (up to 8)
	}

	return colorPalette;
}

function createColor(
	parentContainer: HTMLElement,
	builder: JSBuilder,
	colorItem: ColorItem,
	index: string,
	themeData: any,
	widgetData: ColorPaletteWidgetData,
	isCurrent: boolean,
	themeColors: ThemeColor[],
	groupName: string,
): Element {
	const color = window.L.DomUtil.create(
		'input',
		builder.options.cssClass + ' ui-color-picker-entry',
		parentContainer,
	);
	color.type = 'radio';
	color.name = groupName;
	color.value = colorItem;
	color.style.backgroundColor = '#' + colorItem;
	color.setAttribute('index', index);
	color.innerHTML = isCurrent ? '&#149;' : '&#160;';
	if (themeData) color.setAttribute('theme', themeData);

	// Set color tooltips
	var colorTooltip;
	if (themeData) {
		const found = themeColors.find(
			(item: ThemeColor) =>
				item.Value.toLowerCase() === colorItem.toLowerCase(),
		);
		if (found) colorTooltip = found.Name;
	}
	if (!colorTooltip && window.app.colorNames)
		colorTooltip = findColorName(colorItem);
	if (!colorTooltip) colorTooltip = _('Unknown color');

	if (window.enableAccessibility) {
		color.setAttribute('aria-label', colorTooltip);
	} else {
		color.setAttribute('data-cooltip', colorTooltip);
	}

	// Assuming 'color' is your target HTMLElement
	color.addEventListener('click', (event: MouseEvent) => {
		handleColorSelection(
			event.target as HTMLElement, // The clicked element
			builder, // Pass the builder object
			widgetData, // Pass the widget data
		);
	});
	color.addEventListener('keydown', (event: KeyboardEvent) => {
		if (event.code === 'Enter') {
			event.preventDefault();
			handleColorSelection(
				event.target as HTMLElement, // The clicked element
				builder, // Pass the builder object
				widgetData, // Pass the widget data
			);
		}
	});
	window.L.control.attachTooltipEventListener(color, builder.map);

	return color;
}

function handleColorSelection(
	target: HTMLElement,
	builder: JSBuilder,
	widgetData: ColorPaletteWidgetData,
) {
	const palette = generatePalette(getCurrentPaletteName());
	const colorCode = target.getAttribute('value');
	const themeData = target.getAttribute('theme');

	if (colorCode != null) {
		JSDialog.sendColorCommand(builder, widgetData, colorCode, themeData);
		builder.callback(
			'colorpicker',
			'hidedropdown',
			widgetData,
			themeData ? themeData : colorCode,
			builder,
		);
	} else {
		JSDialog.sendColorCommand(builder, widgetData, 'transparent');
		builder.callback(
			'colorpicker',
			'hidedropdown',
			widgetData,
			'transparent',
			builder,
		);
	}
	// Update the recent colors list
	const recentRow = palette[palette.length - 1];
	if (recentRow.indexOf(colorCode) !== -1)
		recentRow.splice(recentRow.indexOf(colorCode), 1);
	recentRow.unshift(colorCode);
	window.prefs.set('recentColor', JSON.stringify(recentRow));
}

function createAutoColorButton(
	parentContainer: HTMLElement,
	data: ColorPaletteWidgetData,
	builder: JSBuilder,
) {
	const label = window.L.DomUtil.create('label', '', parentContainer);
	label.textContent = _('Color') + ':';
	label.htmlFor = 'transparent-color-button';

	const hasTransparent =
		data.command !== '.uno:FontColor' && data.command !== '.uno:Color';
	const buttonText = hasTransparent ? _('No fill') : _('Automatic');
	const autoButton = window.L.DomUtil.create(
		'button',
		builder.options.cssClass + ' ui-pushbutton auto-color-button',
		parentContainer, // Append button to the newly created div
	);
	autoButton.id = 'transparent-color-button';
	autoButton.innerText = buttonText;
	autoButton.tabIndex = '0';
	autoButton.focus();

	autoButton.addEventListener('click', () => {
		JSDialog.sendColorCommand(builder, data, 'transparent');
		builder.callback('colorpicker', 'hidedropdown', data, '-1', builder);
	});
}

function createPaletteSwitch(
	parentContainer: HTMLElement,
	builder: JSBuilder,
): HTMLSelectElement {
	const label = window.L.DomUtil.create('label', '', parentContainer);
	label.textContent = _('Palette') + ':';
	label.htmlFor = 'color-palette-listbox';

	const paletteListbox = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-listbox-container color-palette-selector',
		parentContainer,
	);
	const listbox = window.L.DomUtil.create(
		'select',
		builder.options.cssClass + ' ui-listbox',
		paletteListbox,
	);

	listbox.id = 'color-palette-listbox';
	listbox.setAttribute('aria-label', _('Color palette'));
	listbox.setAttribute('tabindex', '0');

	for (const i in window.app.colorPalettes) {
		const paletteOption = window.L.DomUtil.create('option', '', listbox);
		if (i === getCurrentPaletteName())
			paletteOption.setAttribute('selected', 'selected');
		paletteOption.value = i;
		paletteOption.innerText = window.app.colorPalettes[i].name;
	}

	window.L.DomUtil.create(
		'span',
		builder.options.cssClass + ' ui-listbox-arrow',
		paletteListbox,
	);

	return listbox;
}

function updatePalette(
	paletteName: string,
	data: ColorPaletteWidgetData,
	builder: JSBuilder,
	paletteContainer: HTMLElement,
	customContainer: HTMLElement,
	recentContainer: HTMLElement,
) {
	const hexColor = String(JSDialog.getCurrentColor(data, builder));
	const currentColor: ColorItem = hexColor.toUpperCase().replace('#', '');

	const palette = generatePalette(paletteName);
	const detailedPalette = window.app.colorPalettes[paletteName].colors;
	const themeColors: ThemeColor[] =
		window.app.colorPalettes['ThemeColors'].colors.flat();

	paletteContainer.style.gridTemplateRows =
		'repeat(' + (palette.length - 2) + ', auto)';
	paletteContainer.style.gridTemplateColumns =
		'repeat(' + palette[0].length + ', auto)';

	paletteContainer.replaceChildren();
	for (let i = 0; i < palette.length - 2; i++) {
		for (let j = 0; j < palette[i].length; j++) {
			const themeData = detailedPalette[i][j].Data
				? JSON.stringify(detailedPalette[i][j].Data)
				: undefined;

			createColor(
				paletteContainer,
				builder,
				palette[i][j],
				i + ':' + j,
				themeData,
				data,
				currentColor == palette[i][j],
				themeColors,
				'palette-color',
			);
		}
	}

	customContainer.replaceChildren();

	const customInput = window.L.DomUtil.create('input', '', customContainer);
	customInput.id = 'ui-color-picker-custom-input';
	customInput.setAttribute('aria-label', _('Enter custom color in hex format'));
	customInput.placeholder = '#FFF000';
	customInput.maxlength = 7;
	customInput.type = 'text';

	customInput.addEventListener('change', () => {
		let color = customInput.value;
		if (color.indexOf('#') === 0) color = color.substr(1);

		if (color.length != 6) {
			customInput.value = '';
			return;
		}

		const customColorRow = palette[palette.length - 2];
		if (customColorRow.indexOf(color) !== -1) {
			customColorRow.splice(customColorRow.indexOf(color), 1);
		}
		customColorRow.unshift(color.toUpperCase());
		window.prefs.set('customColor', JSON.stringify(customColorRow));
		updatePalette(
			paletteName,
			data,
			builder,
			paletteContainer,
			customContainer,
			recentContainer,
		);
	});

	const customColors = palette[palette.length - 2];
	for (let i = 0; i < customColors.length && i < 4; i++) {
		createColor(
			customContainer,
			builder,
			customColors[i],
			'8:' + i,
			undefined,
			data,
			currentColor == customColors[i],
			themeColors,
			'custom-color',
		);
	}

	recentContainer.replaceChildren();
	const recentColors = palette[palette.length - 1];
	for (let i = 0; i < recentColors.length && i < 8; i++) {
		createColor(
			recentContainer,
			builder,
			recentColors[i],
			'9:' + i,
			undefined,
			data,
			currentColor == recentColors[i],
			themeColors,
			'recent-color',
		);
	}
}

JSDialog.colorPicker = function (
	parentContainer: Element,
	data: ColorPaletteWidgetData,
	builder: JSBuilder,
) {
	const container = window.L.DomUtil.create(
		'div',
		'ui-color-picker',
		parentContainer,
	);
	container.id = data.id;
	container.tabIndex = '-1'; // focus should be on first element in grid for color picker

	const header = window.L.DomUtil.create(
		'div',
		'ui-color-picker-header',
		container,
	);

	createAutoColorButton(header, data, builder);

	const listbox = createPaletteSwitch(header, builder);

	const paletteContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-color-picker-palette',
		container,
	);

	const customContainer = window.L.DomUtil.createWithId(
		'div',
		'ui-color-picker-custom',
		container,
	);

	const recentLabel = window.L.DomUtil.create('span', '', container);
	recentLabel.id = 'ui-color-picker-recent-label';
	recentLabel.innerText = _('Recent');

	const recentContainer = window.L.DomUtil.createWithId(
		'div',
		'ui-color-picker-recent',
		container,
	);
	recentContainer.setAttribute('role', 'radiogroup');
	recentContainer.setAttribute('aria-labelledby', recentLabel.id);

	updatePalette(
		getCurrentPaletteName(),
		data,
		builder,
		paletteContainer,
		customContainer,
		recentContainer,
	);

	listbox.addEventListener('change', () => {
		const newPaletteName = listbox.value;
		window.prefs.set('colorPalette', newPaletteName);
		updatePalette(
			newPaletteName,
			data,
			builder,
			paletteContainer,
			customContainer,
			recentContainer,
		);
	});

	JSDialog.MakeFocusCycle(container);

	return false;
};
