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
 * ColorPicker is used for building a native HTML color picker
 * panel to be used by the properties mobile wizard.
 */

/* global _ */

type ColorTypes = 'BASIC_COLOR' | 'TINT';

interface ColorPickerOptions {
	selectedColor: string;
	noColorControl: boolean;
	autoColorControl: boolean;
	selectionCallback: (color: string) => unknown;
}

interface ColorEvent {
	id: string;
}

class ColorPicker {
	options: ColorPickerOptions;

	_selectedBasicColorIndex: number;
	_selectedTintIndex: number;

	static ID = 0;
	static readonly ID_TAG = 'color-picker-';

	// we need a tight layout in order to be able to show 11 colors as in gdoc
	static readonly BASIC_COLORS = [
		'#000000',
		'#980000',
		'#ff0000',
		'#ff9900',
		'#ffff00',
		'#00ff00',
		'#00ffff' /*'#4a86e8',*/,
		'#0000ff',
		'#9900ff',
		'#ff00ff',
	] as const;

	static readonly TINTS = {
		'#000000': [
			'#000000',
			'#434343',
			'#666666',
			'#888888',
			'#bbbbbb',
			'#dddddd',
			'#eeeeee',
			'#ffffff',
		],
		'#980000': [
			'#5b0f00',
			'#85200c',
			'#a61c00',
			'#980000',
			'#cc4125',
			'#dd7e6b',
			'#e6b8af',
			'#ffffff',
		],
		'#ff0000': [
			'#660000',
			'#990000',
			'#cc0000',
			'#ff0000',
			'#e06666',
			'#ea9999',
			'#f4cccc',
			'#ffffff',
		],
		'#ff9900': [
			'#783f04',
			'#b45f06',
			'#e69138',
			'#ff9900',
			'#f6b26b',
			'#f9cb9c',
			'#fce5cd',
			'#ffffff',
		],
		'#ffff00': [
			'#7f6000',
			'#bf9000',
			'#f1c232',
			'#ffff00',
			'#ffd966',
			'#ffe599',
			'#fff2cc',
			'#ffffff',
		],
		'#00ff00': [
			'#274e13',
			'#38761d',
			'#6aa84f',
			'#00ff00',
			'#93c47d',
			'#b6d7a8',
			'#d9ead3',
			'#ffffff',
		],
		'#00ffff': [
			'#0c343d',
			'#134f5c',
			'#45818e',
			'#00ffff',
			'#76a5af',
			'#a2c4c9',
			'#d0e0e3',
			'#ffffff',
		],
		// '#4a86e8': ['#1c4587', '#1155cc', '#3c78d8', '#4a86e8',
		// 	'#6d9eeb', '#a4c2f4', '#c9daf8', '#ffffff'],
		'#0000ff': [
			'#073763',
			'#0b5394',
			'#3d85c6',
			'#0000ff',
			'#6fa8dc',
			'#9fc5e8',
			'#cfe2f3',
			'#ffffff',
		],
		'#9900ff': [
			'#20124d',
			'#351c75',
			'#674ea7',
			'#9900ff',
			'#8e7cc3',
			'#b4a7d6',
			'#d9d2e9',
			'#ffffff',
		],
		'#ff00ff': [
			'#4c1130',
			'#741b47',
			'#a64d79',
			'#ff00ff',
			'#c27ba0',
			'#d5a6bd',
			'#ead1dc',
			'#ffffff',
		],
	} as const;

	_pickerID: number;
	_basicColorSampleIdTag: string;
	_tintSampleIdTag: string;
	_noColorControlId: string;
	_autoColorControlId: string;

	_selectedColorElement: HTMLElement | undefined;

	_selectedColor: ColorPickerOptions['selectedColor'];
	_selectionCallback: ColorPickerOptions['selectionCallback'];
	_container: ReturnType<ColorPicker['_createControl']>;

	_initialized: boolean;

	_basicColorSelectionMark: HTMLElement;

	constructor(
		selectedColorSample: HTMLElement | undefined,
		options: ColorPickerOptions,
	) {
		this.options = Object.assign(
			{
				selectedColor: '#ff0000',
				noColorControl: true,
				autoColorControl: false,
				selectionCallback: (_: string) => {
					/* no-op */
				},
			},
			options,
		);

		if (this.options.noColorControl && this.options.autoColorControl) {
			this.options.autoColorControl = false;
			window.app.console.warn(
				'ColorPicker: requested both no color and auto color control',
			);
		}

		this._selectedBasicColorIndex = 0;
		this._selectedTintIndex = 0;

		const pickerID = ColorPicker.ID++;

		this._basicColorSampleIdTag =
			ColorPicker.ID_TAG + pickerID + '-basic-color-';
		this._tintSampleIdTag = ColorPicker.ID_TAG + pickerID + '-tint-';
		this._noColorControlId = ColorPicker.ID_TAG + pickerID + '-no-color';
		this._autoColorControlId = ColorPicker.ID_TAG + pickerID + '-auto-color';
		this._createBasicColorSelectionMark();
		this._selectedColorElement = selectedColorSample;
		this._selectedColor = this.options.selectedColor;
		this._selectionCallback = this.options.selectionCallback;
		this._initIndexes();
		this._container = this._createControl();
		this._initialized = true;
	}

	getContainer() {
		return this._container;
	}

	onShow = () => {
		if (!this._initialized || this._selectedColor === '#') return;
		this._initIndexes();
		this._updateTintsView();
	};

	_initIndexes() {
		for (var i = 0; i < this._getBasicColorCount(); ++i) {
			var tintSet = this._getTintSet(i);
			if (!tintSet) return;
			for (var j = 0; j < tintSet.length; ++j) {
				var tint = tintSet[j];
				if (tint === this._selectedColor) {
					this._selectedBasicColorIndex = i;
					this._selectedTintIndex = j;
					return;
				}
			}
		}
	}

	_createControl() {
		var children = [];
		if (this.options.noColorControl || this.options.autoColorControl)
			children.push(this._createPseudoColorControl());
		children.push(this._createBasicColorSamples());
		children.push(this._createTintSamples());
		return {
			type: 'divcontainer',
			style: 'colors-container',
			children: children,
		};
	}

	_createPseudoColorControl() {
		var noColorControl = this.options.noColorControl;
		var icon = {
			type: 'fixedtext',
			text: '',
			style: noColorControl
				? 'no-color-control-icon'
				: 'auto-color-control-icon',
		};
		var label = noColorControl
			? {
					type: 'fixedtext',
					style: 'no-color-control-label',
					text: _('No color'),
				}
			: {
					type: 'fixedtext',
					style: 'auto-color-control-label',
					text: _('Automatic color'),
				};
		var description = { type: 'divcontainer', children: [icon, label] };
		var checked = noColorControl
			? {
					type: 'fixedtext',
					id: this._noColorControlId,
					style: 'no-color-control-mark',
					text: '',
				}
			: {
					type: 'fixedtext',
					id: this._autoColorControlId,
					style: 'auto-color-control-mark',
					text: '',
				};
		var container = {
			type: 'divcontainer',
			style: noColorControl
				? 'colors-container-no-color-row'
				: 'colors-container-auto-color-row',
			handlers: [
				{
					event: 'click',
					handler: window.L.bind(this.onClickPseudoColor, this),
				},
			],
			children: [description, checked],
		};
		return container;
	}

	_createBasicColorSamples() {
		var colorEntries = [];
		for (var k = 0; k < this._getBasicColorCount(); ++k) {
			var selected = k === this._selectedBasicColorIndex;
			var entry: {
				type: 'colorsample';
				id: string;
				selected: boolean;
				color: string;
				size: 'small';
				handlers: [{ event: 'click'; handler: (event: ColorEvent) => void }];
				mark?: HTMLElement;
			} = {
				type: 'colorsample',
				id: this._basicColorSampleIdTag + k,
				selected: selected,
				color: this._getBasicColor(k),
				size: 'small',
				handlers: [
					{
						event: 'click',
						handler: window.L.bind(this.onClickBasicColorSample, this),
					},
				],
			};
			if (selected) {
				entry.mark = this._basicColorSelectionMark;
			}
			colorEntries.push(entry);
		}
		return {
			type: 'divcontainer',
			style: 'colors-container-basic-colors-row',
			children: colorEntries,
		};
	}

	_createTintSamples() {
		var tints = this._getTintSet(this._selectedBasicColorIndex);
		var k,
			selected,
			entry,
			tintRowLength = tints.length / 2;

		// first tints row
		var tintsEntries1 = [];
		for (k = 0; k < tintRowLength; ++k) {
			selected = tints[k] === this._selectedColor;
			entry = {
				type: 'colorsample',
				id: this._tintSampleIdTag + k,
				selected: selected,
				color: tints[k],
				size: 'big',
				handlers: [
					{
						event: 'click',
						handler: window.L.bind(this.onClickTintSample, this),
					},
				],
			};
			tintsEntries1.push(entry);
		}
		var tintsRow1 = {
			type: 'divcontainer',
			style: 'colors-container-tints',
			children: tintsEntries1,
		};

		// second tints row
		var tintsEntries2 = [];
		for (k = tintRowLength; k < tints.length; ++k) {
			selected = tints[k] === this._selectedColor;
			entry = {
				type: 'colorsample',
				id: this._tintSampleIdTag + k,
				selected: selected,
				color: tints[k],
				size: 'big',
				handlers: [
					{
						event: 'click',
						handler: window.L.bind(this.onClickTintSample, this),
					},
				],
			};
			tintsEntries2.push(entry);
		}
		var tintsRow2 = {
			type: 'divcontainer',
			style: 'colors-container-tints',
			children: tintsEntries2,
		};

		return { type: 'divcontainer', children: [tintsRow1, tintsRow2] };
	}

	_createBasicColorSelectionMark() {
		this._basicColorSelectionMark = window.L.DomUtil.create(
			'div',
			'colors-container-basic-color-mark',
			null,
		);
	}

	_getBasicColorCount() {
		return ColorPicker.BASIC_COLORS.length;
	}

	_getBasicColor(
		index: number,
	): (typeof ColorPicker.BASIC_COLORS)[number] | '' {
		if (!(index >= 0 && index < ColorPicker.BASIC_COLORS.length)) {
			return '';
		}
		return ColorPicker.BASIC_COLORS[index];
	}

	_getTintSet(basicColorIndex: number) {
		const basicColor = this._getBasicColor(basicColorIndex);

		if (basicColor === '') return [] as const;

		return ColorPicker.TINTS[basicColor];
	}

	_extractBasicColorIndex(sampleId: string) {
		if (!sampleId.startsWith(this._basicColorSampleIdTag)) return -1;
		var index = parseInt(
			sampleId.substring(this._basicColorSampleIdTag.length),
		);
		if (index < 0 || index >= this._getBasicColorCount()) return -1;
		return index;
	}

	_extractTintIndex(sampleId: string) {
		if (!sampleId.startsWith(this._tintSampleIdTag)) return -1;
		var index = parseInt(sampleId.substring(this._tintSampleIdTag.length));
		if (
			index < 0 ||
			index >= this._getTintSet(this._selectedBasicColorIndex).length
		)
			return -1;
		return index;
	}

	_getColorCode(colorIndex: number, colorType: ColorTypes): string {
		var sampleElem = this._getSampleElement(colorIndex, colorType);
		return sampleElem.name;
	}

	onClickPseudoColor() {
		this._selectedColor = '#';
		this._unselectSample(this._selectedTintIndex, 'TINT');
		this._updatePseudoColorControl(true);
		// transparent is fine for both no color and automatic color
		this._selectionCallback('transparent');
	}

	onClickBasicColorSample(e: ColorEvent) {
		var basicColorIndex = this._extractBasicColorIndex(e.id);
		if (basicColorIndex < 0) return;
		this._selectedBasicColorIndex = this._updateSelectedSample(
			basicColorIndex,
			this._selectedBasicColorIndex,
			'BASIC_COLOR',
		);
		this._updateTintsView();
		this._selectTintIndex(3);
	}

	onClickTintSample(e: ColorEvent) {
		var tintIndex = this._extractTintIndex(e.id);
		if (tintIndex < 0) return;
		this._selectTintIndex(tintIndex);
	}

	_selectTintIndex(tintIndex: number) {
		this._selectedTintIndex = this._updateSelectedSample(
			tintIndex,
			this._selectedTintIndex,
			'TINT',
		);
		this._selectedColor =
			'#' + this._getColorCode(this._selectedTintIndex, 'TINT');
		this._updatePseudoColorControl(false);
		this._updateSelectedColorElement();
		this._selectionCallback(
			this._getColorCode(this._selectedTintIndex, 'TINT'),
		);
	}

	_updateSelectedSample(
		colorIndex: number,
		selectedColorIndex: number,
		colorType: ColorTypes,
	) {
		this._unselectSample(selectedColorIndex, colorType);
		this._selectSample(colorIndex, colorType);
		return colorIndex;
	}

	_unselectSample(colorIndex: number, colorType: ColorTypes) {
		var sampleElem = this._getSampleElement(colorIndex, colorType);
		if (sampleElem && sampleElem.firstChild) {
			if (colorType === 'BASIC_COLOR') {
				sampleElem.removeChild(sampleElem.firstChild);
				window.L.DomUtil.removeClass(
					sampleElem,
					'colors-container-selected-basic-color',
				);
			} else if (colorType === 'TINT') {
				const child: ChildNode & { style?: HTMLElement['style'] } =
					sampleElem.firstChild;

				if (!('style' in child)) {
					throw new Error(
						'Assertion failed: sampleElem.firstChild is not a HTMLElement',
					);
				}

				child.style.visibility = 'hidden';
			}
		}
	}

	_selectSample(colorIndex: number, colorType: ColorTypes) {
		var sampleElem = this._getSampleElement(colorIndex, colorType);
		if (sampleElem) {
			if (colorType === 'BASIC_COLOR') {
				sampleElem.appendChild(this._basicColorSelectionMark);
				window.L.DomUtil.addClass(
					sampleElem,
					'colors-container-selected-basic-color',
				);
			} else if (colorType === 'TINT' && sampleElem.firstChild) {
				const child: ChildNode & { style?: HTMLElement['style'] } =
					sampleElem.firstChild;

				if (!('style' in child)) {
					throw new Error(
						'Assertion failed: sampleElem.firstChild is not a HTMLElement',
					);
				}

				child.style.visibility = 'visible';
			}
		}
	}

	_updateTintsView() {
		var tintSet = this._getTintSet(this._selectedBasicColorIndex);
		if (!tintSet) return;
		for (var i = 0; i < tintSet.length; ++i) {
			var tint = tintSet[i];
			var sampleElem = this._getSampleElement(i, 'TINT');
			if (sampleElem) {
				sampleElem.style.backgroundColor = tint;
				sampleElem.name = tint.substring(1);
				if (sampleElem.firstChild) {
					const child: ChildNode & { style?: HTMLElement['style'] } =
						sampleElem.firstChild;

					if (!('style' in child)) {
						throw new Error(
							'Assertion failed: sampleElem.firstChild is not a HTMLElement',
						);
					}

					if (tint === this._selectedColor) {
						child.style.visibility = 'visible';
					} else {
						child.style.visibility = 'hidden';
					}
				}
			}
		}
	}

	_updatePseudoColorControl(checked: boolean) {
		if (this.options.noColorControl) this._updateNoColorControl(checked);
		else if (this.options.autoColorControl)
			this._updateAutoColorControl(checked);
	}

	_updateNoColorControl(checked: boolean) {
		var noColorElem = window.L.DomUtil.get(this._noColorControlId);
		if (noColorElem) {
			if (noColorElem.checked !== checked) {
				noColorElem.checked = checked;
				if (this._selectedColorElement) {
					if (checked) {
						noColorElem.textContent = '\u2714';
						// update value for the related menu entry
						window.L.DomUtil.addClass(
							this._selectedColorElement,
							'no-color-selected',
						);
						this._selectedColorElement.textContent = '\\';
					} else {
						noColorElem.replaceChildren();
						// update value for the related menu entry
						window.L.DomUtil.removeClass(
							this._selectedColorElement,
							'no-color-selected',
						);
						this._selectedColorElement.replaceChildren();
					}
				}
			}
		}
	}

	_updateAutoColorControl(checked: boolean) {
		var autoColorElem: HTMLInputElement = window.L.DomUtil.get(
			this._autoColorControlId,
		);
		if (autoColorElem) {
			if (autoColorElem.checked !== checked) {
				autoColorElem.checked = checked;
				if (this._selectedColorElement) {
					if (checked) {
						autoColorElem.textContent = '\u2714';
						// update value for the related menu entry
						window.L.DomUtil.addClass(
							this._selectedColorElement,
							'auto-color-selected',
						);
						this._selectedColorElement.textContent = '\\';
					} else {
						autoColorElem.replaceChildren();
						// update value for the related menu entry
						window.L.DomUtil.removeClass(
							this._selectedColorElement,
							'auto-color-selected',
						);
						this._selectedColorElement.replaceChildren();
					}
				}
			}
		}
	}

	_getSampleElement(index: number, type: ColorTypes): HTMLFormElement {
		var sampleId;
		if (type === 'BASIC_COLOR') {
			sampleId = this._basicColorSampleIdTag + index;
		} else if (type === 'TINT') {
			sampleId = this._tintSampleIdTag + index;
		}
		return window.L.DomUtil.get(sampleId);
	}

	_updateSelectedColorElement() {
		if (this._selectedColorElement) {
			this._selectedColorElement.style.backgroundColor = this._selectedColor;
		}
	}
}
