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
 * The spinfield constructors:
 * 	- JSDialog.baseSpinField
 * 	- JSDialog.spinfieldControl
 * 	- JSDialog.metricfieldControl
 * 	- JSDialog.formattedfieldControl
 *
 * The helper functions used elsewhere, like in JSDialogBuilder for example:
 * 	- _parseSpinFieldValue
 * 	- _formatSpinFieldValue
 * 	- _setSpinFieldValue
 */

declare var JSDialog: any;

interface SpinFieldContainer extends HTMLDivElement {
	_step: number;
	_min?: number;
	_max?: number;
	_unit: string;
}

type SpinFieldControls = {
	container: SpinFieldContainer;
	spinfield: HTMLInputElement;
};

/* for now we have this temporary type to prevent typescript error
 * in the `builder._controlHandlers['...'](...)` statement. we
 * have this because `MobileWizardBuilder` calls this function
 * and overrides the constructor for 'basespinfield'. when MobileWizardBuilder
 * gets converted to typescript, hopefully we would merge the two implementations
 * and then remove this. */
type BaseSpinFieldConstructor = (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	_customCallback: JSDialogCallback,
) => SpinFieldControls;

let _decimal: string;
let _minusSign: string;

const _ensureInitialized = function () {
	if (_decimal === undefined) {
		_decimal = app.localeService.getDecimalSeparator() || '.';
		_minusSign = app.localeService.getMinusSign() || '-';
	}
};

const getDecimal = function () {
	_ensureInitialized();
	return _decimal;
};

const getMinusSign = function () {
	_ensureInitialized();
	return _minusSign;
};

const _numericCharPattern = function () {
	return new RegExp('[0-9\\' + getDecimal() + '\\' + getMinusSign() + ']');
};

const _preventNonNumericalInput = function (e: KeyboardEvent) {
	e = e || (window.event as KeyboardEvent);
	var charCode = typeof e.which == 'undefined' ? e.keyCode : e.which;
	var charStr = String.fromCharCode(charCode);
	if (!charStr.match(_numericCharPattern()) && charCode !== 13)
		return e.preventDefault();

	var value = (e.target as any).value;
	if (!value) return;

	// no dup
	if (getDecimal() === charStr || getMinusSign() === charStr) {
		if (value.indexOf(charStr) > -1) return e.preventDefault();
	}
};

const _spinFieldStep = function (
	div: SpinFieldContainer,
	spinfield: HTMLInputElement,
	direction: number,
) {
	const step = div._step || 1;
	const min = div._min;
	const max = div._max;
	const unit = div._unit || '';
	let current = parseFloat(JSDialog._parseSpinFieldValue(spinfield.value));
	if (isNaN(current)) current = 0;

	let newVal = current + direction * step;
	const precision = _getStepPrecision(step);
	newVal = parseFloat(newVal.toFixed(precision));

	if (min != undefined && newVal < min) newVal = min;
	if (max != undefined && newVal > max) newVal = max;

	JSDialog._setSpinFieldValue(
		spinfield,
		JSDialog._formatSpinFieldValue(newVal, unit),
		newVal,
	);
	spinfield.dispatchEvent(new Event('change'));
};

const _extractUnits = function (text: string) {
	if (!text) return '';
	return text.replace(/[\d.-]/g, '').trim();
};

const _cleanValueFromUnits = function (text: string) {
	if (!text) return '';
	return text.replace(/[^\d.-]/g, '').trim();
};

const _clampSpinFieldValue = function (
	container: SpinFieldContainer,
	displayValue: string,
) {
	let value = JSDialog._parseSpinFieldValue(displayValue);
	let num = parseFloat(value);
	if (!isNaN(num)) {
		if (container._min != undefined && num < container._min)
			num = container._min;
		if (container._max != undefined && num > container._max)
			num = container._max;
		value = '' + num;
	}
	return value;
};

const _getStepPrecision = function (step: number) {
	const str = '' + Math.abs(step);
	const dot = str.indexOf('.');
	return dot >= 0 ? str.length - dot - 1 : 0;
};

const listenNumericChanges = function (
	_data: any,
	builder: JSBuilder,
	controls: SpinFieldControls,
	customCallback: JSDialogCallback,
) {
	controls.spinfield.addEventListener('change', function () {
		if (controls.container.hasAttribute('disabled')) return;
		const value = _clampSpinFieldValue(controls.container, this.value);
		if (customCallback) {
			customCallback('spinfield', 'change', controls.container, value, builder);
		} else {
			builder.callback(
				'spinfield',
				'change',
				controls.container,
				value,
				builder,
			);
			builder.callback(
				'spinfield',
				'value',
				controls.container,
				value,
				builder,
			);
		}
	});
};

JSDialog.baseSpinField = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	_customCallback: JSDialogCallback,
): SpinFieldControls {
	const div: SpinFieldContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' spinfieldcontainer',
		parentContainer,
	);
	div.id = data.id;

	const spinfield: HTMLInputElement = window.L.DomUtil.create(
		'input',
		builder.options.cssClass + ' spinfield',
		div,
	);
	spinfield.id = data.id + '-input';
	spinfield.type = 'text';
	spinfield.inputMode = 'decimal';
	spinfield.setAttribute('role', 'spinbutton');
	spinfield.setAttribute('spellcheck', 'false');
	spinfield.dir = document.documentElement.dir;
	spinfield.tabIndex = 0;
	spinfield.setAttribute('autocomplete', 'off');

	if (data.label) {
		const fixedTextData = { text: data.label, labelFor: data.id };
		/* todo: no need to add _fixedtextControl to the builder interface,
		 * it will be converted to typescript soon. */
		(builder as any)._fixedtextControl(parentContainer, fixedTextData, builder);
	} else {
		JSDialog.SetupA11yLabelForLabelableElement(
			parentContainer,
			spinfield,
			data,
			builder,
		);
	}

	var unitStr = '';
	if (data.unit && data.unit !== ':') {
		unitStr = builder._unitToVisibleString(data.unit);
	} else if (!data.unit) {
		const textForUnits =
			data.text || (data.value != undefined ? '' + data.value : '');
		if (textForUnits) {
			var extracted = _extractUnits(textForUnits);
			if (extracted) unitStr = builder._unitToVisibleString(extracted);
		}
	}
	div._unit = unitStr;
	if (unitStr) div.dataset.unit = unitStr;

	if (data.min != undefined) {
		div._min = data.min;
		spinfield.setAttribute('aria-valuemin', data.min);
	}

	if (data.max != undefined) {
		div._max = data.max;
		spinfield.setAttribute('aria-valuemax', data.max);
	}

	div._step = data.step != undefined ? data.step : 1;

	const isDisabled = data.enabled === false;
	spinfield.setAttribute('aria-disabled', isDisabled.toString());
	if (isDisabled) {
		div.setAttribute('disabled', 'true');
		spinfield.setAttribute('disabled', 'true');
	}

	if (data.readOnly === true) $(spinfield).attr('readOnly', 'true');
	if (data.hidden) $(spinfield).hide();

	if (!window.L.Browser.cypressTest)
		spinfield.onkeypress = window.L.bind(_preventNonNumericalInput, builder);

	const cssClass = builder.options.cssClass;
	const buttons = window.L.DomUtil.create(
		'div',
		cssClass + ' spinfieldbuttons',
		div,
	);

	const up = window.L.DomUtil.create(
		'button',
		cssClass + ' spinfieldbutton-up',
		buttons,
	);
	up.type = 'button';
	up.tabIndex = -1;
	up.setAttribute('aria-label', _('Increment value'));

	const down = window.L.DomUtil.create(
		'button',
		cssClass + ' spinfieldbutton-down',
		buttons,
	);
	down.type = 'button';
	down.tabIndex = -1;
	down.setAttribute('aria-label', _('Decrement value'));

	if (isDisabled) {
		up.setAttribute('disabled', 'true');
		down.setAttribute('disabled', 'true');
	}

	// With native <input type="number"> the browser's built-in spin
	// buttons followed the input's disabled state automatically. Our
	// custom buttons need explicit synchronization.
	JSDialog.SynchronizeDisabledState(div, [spinfield, up, down]);

	up.addEventListener('mousedown', function (e: MouseEvent) {
		e.preventDefault();
	});

	down.addEventListener('mousedown', function (e: MouseEvent) {
		e.preventDefault();
	});

	up.addEventListener('click', function () {
		_spinFieldStep(div, spinfield, 1);
	});

	down.addEventListener('click', function () {
		_spinFieldStep(div, spinfield, -1);
	});

	spinfield.addEventListener('keydown', function (e: KeyboardEvent) {
		const ctrlKey =
			window.L.Browser.mac || window.ThisIsTheiOSApp ? e.metaKey : e.ctrlKey;
		if (e.key === 'ArrowUp') {
			e.preventDefault();
			_spinFieldStep(div, spinfield, 1);
		} else if (e.key === 'ArrowDown') {
			e.preventDefault();
			_spinFieldStep(div, spinfield, -1);
		} else if (e.key === 'Home' && ctrlKey && div._min != undefined) {
			e.preventDefault();
			var unit = div._unit || '';
			JSDialog._setSpinFieldValue(
				spinfield,
				JSDialog._formatSpinFieldValue(div._min, unit),
				div._min,
			);
			spinfield.dispatchEvent(new Event('change'));
		} else if (e.key === 'End' && ctrlKey && div._max != undefined) {
			e.preventDefault();
			var unit = div._unit || '';
			JSDialog._setSpinFieldValue(
				spinfield,
				JSDialog._formatSpinFieldValue(div._max, unit),
				div._max,
			);
			spinfield.dispatchEvent(new Event('change'));
		}
	});

	const controls: SpinFieldControls = {
		container: div,
		spinfield: spinfield,
	};

	return controls;
};

JSDialog.spinfieldControl = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	customCallback: JSDialogCallback,
) {
	/* mobile also defines a constructor for 'basespinfield'. */
	const baseSpinFieldCallback = builder._controlHandlers[
		'basespinfield'
	] as unknown as BaseSpinFieldConstructor;

	const controls: SpinFieldControls = baseSpinFieldCallback(
		parentContainer,
		data,
		builder,
		customCallback,
	);

	const updateFunction = function () {
		if (data.text != undefined) var value = data.text;
		else if (data.children && data.children.length)
			value = data.children[0].text;

		const numeric = _cleanValueFromUnits(value);
		JSDialog._setSpinFieldValue(
			controls.spinfield,
			JSDialog._formatSpinFieldValue(numeric, controls.container._unit),
			numeric,
		);
	};

	controls.spinfield.addEventListener('change', function () {
		if (controls.container.hasAttribute('disabled')) return;
		const value = _clampSpinFieldValue(controls.container, this.value);
		if (customCallback)
			customCallback('spinfield', 'change', controls.container, value, builder);
		else
			builder.callback('spinfield', 'set', controls.container, value, builder);
	});

	updateFunction();
	return false;
};

const _createBaseSpinField = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	customCallback: JSDialogCallback,
) {
	/* mobile also defines a constructor for 'basespinfield'. */
	const baseSpinFieldCtr = builder._controlHandlers[
		'basespinfield'
	] as unknown as BaseSpinFieldConstructor;

	const controls = baseSpinFieldCtr(
		parentContainer,
		data,
		builder,
		customCallback,
	) as SpinFieldControls;

	if (!window.L.Browser.cypressTest && !window.L.Browser.chrome) {
		controls.spinfield.onkeypress = window.L.bind(
			_preventNonNumericalInput,
			builder,
		);
	}

	let style = '';
	if (data.widthChars && data.widthChars > 0) {
		style = 'width: ' + data.widthChars + 'ch;';
	}

	if (data.halign) {
		style += 'justify-self: ' + data.halign + ';';
	}

	if (style) {
		controls.container.style.cssText = style;
	}

	listenNumericChanges(data, builder, controls, customCallback);

	const value = parseFloat(data.value);
	JSDialog._setSpinFieldValue(
		controls.spinfield,
		JSDialog._formatSpinFieldValue(value, controls.container._unit),
		value,
	);

	return false;
};

JSDialog.metricfieldControl = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	customCallback: JSDialogCallback,
) {
	return _createBaseSpinField(parentContainer, data, builder, customCallback);
};

JSDialog.formattedfieldControl = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
	customCallback: JSDialogCallback,
) {
	if (!data.unit && data.text) {
		const units = data.text.split(' ');
		if (units.length == 2) {
			data.unit = units[1];
		}
	}

	if (!data.unit && data.text) {
		data.unit = _extractUnits(data.text.toString());
	}

	return _createBaseSpinField(parentContainer, data, builder, customCallback);
};

JSDialog._parseSpinFieldValue = function (displayValue: string) {
	if (!displayValue) return '';
	var pattern = _numericCharPattern();
	var value = '';
	for (var i = 0; i < displayValue.length; i++) {
		if (displayValue[i].match(pattern)) value += displayValue[i];
	}
	if (getDecimal() !== '.') value = value.replace(getDecimal(), '.');
	if (getMinusSign() !== '-') value = value.replace(getMinusSign(), '-');
	return value;
};

JSDialog._formatSpinFieldValue = function (value: number, unit: string) {
	var str = '' + value;
	if (getDecimal() !== '.') str = str.replace('.', getDecimal());
	if (unit) {
		var noSpace =
			unit === '°' || unit === '"' || unit === '\u2033' || unit === '%';
		return noSpace ? str + unit : str + ' ' + unit;
	}
	return str;
};

JSDialog._setSpinFieldValue = function (
	spinfield: HTMLInputElement,
	displayValue: string,
	numericValue: number,
) {
	spinfield.value = displayValue;
	const num = parseFloat(
		numericValue != undefined
			? numericValue
			: JSDialog._parseSpinFieldValue(displayValue),
	);
	if (!isNaN(num)) spinfield.setAttribute('aria-valuenow', num.toString());
	if (displayValue && displayValue !== '' + num)
		spinfield.setAttribute('aria-valuetext', displayValue);
	else spinfield.removeAttribute('aria-valuetext');
};
