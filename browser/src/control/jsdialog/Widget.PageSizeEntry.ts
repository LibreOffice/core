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
 * Widget.PageSizeEntry.ts
 *
 * A JSDialog "json" widget for rendering the page sizes
 */

declare var JSDialog: any;

interface PageSizeOption {
	id: string;
	text: string;
	uno: string;
}

/// Whether a Calc page size command sets the paper of the current page style.
JSDialog.isCurrentCalcPageSize = function (uno: string): boolean {
	const paper = app.map['stateChangeHandler'].getItemValue('.uno:CalcPageSize');
	const match = /\?PaperFormat:long=(\d+)$/.exec(uno);
	return !!match && paper !== undefined && match[1] === String(paper);
};

function createPageSizeEntryWidget(data: any, builder: any): HTMLElement {
	const sizes: PageSizeOption[] = data.options || [];
	const container = document.createElement('div');
	container.className = 'jsdialog ui-grid';
	container.setAttribute('tabindex', '-1');

	const list = document.createElement('div');
	list.className = 'jsdialog pagesize-list';
	list.setAttribute('role', 'listbox');
	list.setAttribute('tabindex', '-1');
	container.appendChild(list);

	const map = builder.map;
	// Calc reports the paper of its page style. Elsewhere the first size is highlighted.
	const isCalc = map._docLayer.isCalc();

	sizes.forEach((opt, index) => {
		const item = document.createElement('div');
		item.className = 'ui-combobox-entry jsdialog ui-grid-cell';
		item.id = opt.id;
		item.setAttribute('role', 'option');
		const isSelected = isCalc
			? JSDialog.isCurrentCalcPageSize(opt.uno)
			: index === 0;
		if (isSelected) item.classList.add('selected');
		item.setAttribute('aria-selected', isSelected ? 'true' : 'false');
		item.tabIndex = 0;

		item.addEventListener('click', (evt: MouseEvent) => {
			map.sendUnoCommand(opt.uno);
			builder.callback('dialog', 'close', { id: data.id }, null);
		});

		item.addEventListener('keydown', (evt: KeyboardEvent) => {
			if (evt.key === 'Enter') {
				item.click();
			}
		});

		const text = document.createElement('span');
		text.textContent = opt.text;

		item.appendChild(text);
		list.appendChild(item);
	});

	const hr = document.createElement('hr');
	hr.className = 'jsdialog ui-separator horizontal';
	hr.setAttribute('tabindex', '-1');
	container.appendChild(hr);

	const moreSizeContainer = document.createElement('div');
	moreSizeContainer.className = 'ui-combobox-entry jsdialog ui-grid-cell';
	moreSizeContainer.id = 'page-size-more';
	moreSizeContainer.setAttribute('aria-haspopup', 'dialog');
	moreSizeContainer.setAttribute('role', 'button');
	moreSizeContainer.tabIndex = 0;

	moreSizeContainer.addEventListener('click', (evt: MouseEvent) => {
		map.sendUnoCommand('.uno:PageFormatDialog');
		builder.callback('dialog', 'close', { id: data.id }, null);
	});

	moreSizeContainer.addEventListener('keydown', (evt: KeyboardEvent) => {
		if (evt.key === 'Enter') {
			moreSizeContainer.click();
		}
	});

	const text = document.createElement('span');
	text.textContent = _('More Paper Sizes...');
	moreSizeContainer.appendChild(text);
	container.appendChild(moreSizeContainer);

	return container;
}

JSDialog.pageSizeEntry = function (
	parentContainer: Element,
	data: any,
	builder: any,
): boolean {
	const element = createPageSizeEntryWidget(data, builder);
	parentContainer.appendChild(element);

	// Focus the selected size, or the first one when none is selected.
	const focusEntry = (element.querySelector('.ui-combobox-entry.selected') ||
		element.querySelector('.ui-combobox-entry')) as HTMLElement;
	if (focusEntry) {
		focusEntry.focus();
		data.initialSelectedId = focusEntry.id;
	}

	return false;
};
