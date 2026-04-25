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

/**
 * Widget.PageMarginEntry.ts
 *
 * A JSDialog "json" widget for rendering the page margins presets
 * and the "Custom Margins…" link.
 */

declare var JSDialog: any;

interface PageMarginOption {
	title: string;
	icon: string;
	details: { Top: number; Left: number; Bottom: number; Right: number };
}

interface PageMarginOptions {
	[id: string]: PageMarginOption;
}

JSDialog.PageMarginEntry = function (
	parentContainer: Element,
	data: any,
	builder: any,
): boolean {
	const inchesToHMM = (inches: number) => Math.round(inches * 25.4 * 100);
	const options: PageMarginOptions = data.options;
	const map = builder.map;
	const isCalc = map._docLayer.isCalc();

	const lang = window.coolParams.get('lang') || 'en-US';
	const useImperial = lang === 'en-US' || lang === 'en'; // we need to consider both short form as some user can user lang=en-US using document URL
	const inchToCm = 2.54;

	function formatLocalized(valueInInches: number): string {
		let value = valueInInches;
		let unit = '"';
		if (!useImperial) {
			value = valueInInches * inchToCm;
			unit = ' cm';
		}
		const formatted = new Intl.NumberFormat(lang, {
			maximumFractionDigits: 1,
		}).format(value);
		return `${formatted}${unit}`;
	}

	const onMarginClick = (evt: MouseEvent | KeyboardEvent) => {
		const elm = evt.currentTarget as HTMLElement;
		const key = elm.id;
		if (!key || !options[key]) return;

		const opt = options[key];
		if (map.getDocType() === 'spreadsheet') {
			const cmd =
				`.uno:CalcPageMargin` +
				`?Page.LeftMargin:long=${inchesToHMM(opt.details.Left)}` +
				`&Page.RightMargin=${inchesToHMM(opt.details.Right)}` +
				`&Page.TopMargin:long=${inchesToHMM(opt.details.Top)}` +
				`&Page.BottomMargin=${inchesToHMM(opt.details.Bottom)}`;

			map.sendUnoCommand(cmd);
		} else if (map.getDocType() === 'text') {
			const cmdLR =
				`.uno:PageLRMargin` +
				`?Page.Left:long=${inchesToHMM(opt.details.Left)}` +
				`&Page.Right:long=${inchesToHMM(opt.details.Right)}`;

			const cmdUL =
				`.uno:PageULMargin` +
				`?Page.Upper:long=${inchesToHMM(opt.details.Top)}` +
				`&Page.Lower:long=${inchesToHMM(opt.details.Bottom)}`;

			map.sendUnoCommand(cmdLR);
			map.sendUnoCommand(cmdUL);
		}
		builder.callback('dialog', 'close', { id: data.id }, null);
	};

	Object.keys(options).forEach((key, index) => {
		const opt = options[key];
		const isFirstItem = index === 0;

		const item = document.createElement('div');
		item.className = 'margin-item';
		item.id = key;
		item.setAttribute('role', 'option');
		item.setAttribute('tabindex', '-1');
		item.setAttribute('aria-selected', 'false');
		item.addEventListener('click', onMarginClick);

		item.addEventListener('keydown', function (event: KeyboardEvent) {
			if (event.key === 'Enter' || event.key === ' ') {
				onMarginClick(event);
				event.preventDefault();
			} else if (event.key === 'Tab') {
				JSDialog.CloseDropdown(data.id);
				event.preventDefault();
			}
		});

		if (isFirstItem) {
			item.classList.add('selected');
			item.setAttribute('aria-selected', 'true');

			data.initialSelectedId = item.id;
		}

		const img = document.createElement('img');
		img.className = 'margin-icon';
		img.setAttribute('alt', '');
		const iconName = app.LOUtil.getIconNameOfCommand(opt.icon, true);
		app.LOUtil.setImage(img, iconName, app.map);

		const textWrap = document.createElement('div');
		textWrap.className = 'margin-text-content';

		const title = document.createElement('div');
		title.className = 'margin-title';
		title.textContent = opt.title;

		const values = document.createElement('div');
		values.className = 'margin-values';

		const col1 = document.createElement('div');
		col1.className = 'margin-col';
		const topSpan = document.createElement('span');
		topSpan.textContent = _('Top: {top}').replace(
			'{top}',
			formatLocalized(opt.details.Top),
		);
		const leftSpan = document.createElement('span');
		leftSpan.textContent = _('Left: {left}').replace(
			'{left}',
			formatLocalized(opt.details.Left),
		);
		col1.appendChild(topSpan);
		col1.appendChild(leftSpan);

		const col2 = document.createElement('div');
		col2.className = 'margin-col';
		const bottomSpan = document.createElement('span');
		bottomSpan.textContent = _('Bottom: {bottom}').replace(
			'{bottom}',
			formatLocalized(opt.details.Bottom),
		);
		const rightSpan = document.createElement('span');
		rightSpan.textContent = _('Right: {right}').replace(
			'{right}',
			formatLocalized(opt.details.Right),
		);
		col2.appendChild(bottomSpan);
		col2.appendChild(rightSpan);

		values.appendChild(col1);
		values.appendChild(col2);
		textWrap.appendChild(title);
		textWrap.appendChild(values);

		item.appendChild(img);
		item.appendChild(textWrap);
		parentContainer.appendChild(item);
	});

	const hr = document.createElement('hr');
	hr.className = 'jsdialog ui-separator horizontal';
	parentContainer.appendChild(hr);

	const custom = document.createElement('div');
	custom.className = 'margin-item custom-margins-link';
	custom.id = 'customMarginsLink';
	custom.textContent = _('Custom Margins…');
	custom.setAttribute('aria-selected', 'false');
	custom.setAttribute('aria-haspopup', 'dialog');
	custom.setAttribute('role', 'option');
	custom.setAttribute('tabindex', '-1');

	const customClickEventHdl = () => {
		map.sendUnoCommand(
			isCalc ? '.uno:PageFormatDialog' : '.uno:PageSettingDialog',
		);
		builder.callback('dialog', 'close', { id: data.id }, null);
	};

	custom.addEventListener('click', customClickEventHdl);
	custom.addEventListener('keydown', function (event: KeyboardEvent) {
		if (event.key === 'Enter' || event.key === ' ') {
			customClickEventHdl();
			event.preventDefault();
		} else if (event.key === 'Tab') {
			JSDialog.CloseDropdown(data.id);
			event.preventDefault();
		}
	});
	parentContainer.appendChild(custom);

	return false;
};
