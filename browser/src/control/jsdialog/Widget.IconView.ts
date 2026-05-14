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
 * JSDialog.IconView - icon view widget
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'iconview',
 *     singleclickactivate: true,
 *     entries: [
 *         { text: 'some text', tooltip: 'some tooltip', image: 'encoded png', selected: false }
 *     ]
 * }
 */

declare var JSDialog: any;

function _createEntryImage(
	parent: HTMLElement,
	builder: JSBuilder,
	entryData: IconViewEntry,
	image: string,
) {
	const img = window.L.DomUtil.create('img', builder.options.cssClass, parent);
	if (image) img.src = image;

	if (entryData.text) {
		img.alt = entryData.text;
	} else if (entryData.tooltip) {
		img.alt = entryData.tooltip;
	} else {
		img.alt = '';
	}

	// FIXME: not beautiful - would be great to know the dimensions
	// for all of these up-front and do this nicely @ dpiscale for
	// all icon views.
	if (
		parent &&
		parent.parentElement &&
		parent.parentElement.id &&
		parent.parentElement.id.startsWith('stylesview')
	) {
		img.addEventListener('load', () => {
			const ratio = window.devicePixelRatio || 1;
			img.style.width = img.naturalWidth / ratio + 'px';
			img.style.height = img.naturalHeight / ratio + 'px';
		});
	}

	if (entryData.tooltip) img.title = entryData.tooltip;
	else if (entryData.text) img.title = entryData.text;
	else img.title = '';

	setupSize(entryData, img);
}

function _createEntryText(parent: HTMLElement, entryData: IconViewEntry) {
	// Add text below Icon
	window.L.DomUtil.addClass(parent, 'icon-view-item-container');
	const placeholder = window.L.DomUtil.create(
		'span',
		'ui-iconview-entry-title',
		parent,
	);
	placeholder.innerText = entryData.text ? entryData.text : '';
}

function setupSize(entry: IconViewEntry, placeholder: HTMLElement) {
	// Ensure the placeholder is the same size as the image to avoid the dialog changing size
	if (entry.width && entry.height) {
		placeholder.style.width = entry.width + 'px';
		placeholder.style.height = entry.height + 'px';
		placeholder.style.overflow = 'hidden';
		placeholder.style.display = 'block';
	}
}

function _iconViewEntry(
	parentContainer: IconViewElement,
	parentData: IconViewJSON,
	entry: IconViewEntry,
	builder: JSBuilder,
) {
	const disabled = parentData.enabled === false;
	const hasText = entry.text && parentData.textWithIconEnabled;
	const isMultiSelect = parentData.selectionmode === 'multiple';
	const ariaStateAttr = isMultiSelect ? 'aria-selected' : 'aria-checked';

	if (entry.separator && entry.separator === true) {
		if (entry.text) {
			const label = window.L.DomUtil.create(
				'div',
				builder.options.cssClass + ' ui-iconview-separator label',
				parentContainer,
			);
			label.innerText = entry.text;
		} else {
			window.L.DomUtil.create(
				'hr',
				builder.options.cssClass + ' ui-iconview-separator',
				parentContainer,
			);
		}
		return;
	}

	const entryContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-iconview-entry',
		parentContainer,
	);

	//id is needed to find the element to regain focus after widget is updated. see updateWidget in Control.JSDialogBuilder.js
	entryContainer.id = parentData.id + '_' + entry.row;

	entryContainer.setAttribute('role', isMultiSelect ? 'option' : 'radio');
	// By default aria-selected/aria-checked should be false
	entryContainer.setAttribute(ariaStateAttr, 'false');

	if (entry.selected && entry.selected === true) {
		entryContainer.classList.add('selected');
		entryContainer.setAttribute(ariaStateAttr, 'true');
		entryContainer.setAttribute('tabindex', '0');
	} else {
		entryContainer.setAttribute('tabindex', '-1');
	}

	if (entry.ondemand) {
		const placeholder = window.L.DomUtil.create(
			'span',
			builder.options.cssClass,
			entryContainer,
		);
		setupSize(entry, placeholder);

		placeholder.innerText = entry.text ? entry.text : '';
		if (entry.tooltip) placeholder.title = entry.tooltip;
		else if (entry.text) placeholder.title = entry.text;
		else placeholder.title = '';

		parentContainer.requestRenders(entry, placeholder, entryContainer);
		entryContainer.classList.add('ui-iconview-ondemand-placeholder');
	} else {
		_createEntryImage(entryContainer, builder, entry, entry.image);
	}

	if (hasText) _createEntryText(entryContainer, entry);

	if (!disabled) {
		const singleClick = parentData.singleclickactivate === true;
		entryContainer.addEventListener('click', function () {
			parentContainer
				.querySelectorAll('.ui-iconview-entry[tabindex="0"]')
				.forEach(function (el) {
					el.setAttribute('tabindex', '-1');
				});

			parentContainer
				.querySelectorAll('.ui-iconview-entry.selected')
				.forEach(function (el) {
					el.classList.remove('selected');
					el.setAttribute(ariaStateAttr, 'false');
				});

			entryContainer.setAttribute('tabindex', '0');
			entryContainer.focus();
			//avoid re-selecting already selected entry
			if (entryContainer.classList.contains('selected')) return;

			parentContainer.builderCallback('iconview', 'select', entry.row, builder);
			entryContainer.classList.add('selected');
			entryContainer.setAttribute(ariaStateAttr, 'true');

			if (singleClick) {
				parentContainer.builderCallback(
					'iconview',
					'activate',
					entry.row,
					builder,
				);
			}
		});

		entryContainer.addEventListener('contextmenu', function (e: Event) {
			parentContainer
				.querySelectorAll('.ui-iconview-entry')
				.forEach(function (el) {
					el.classList.remove('selected');
					el.setAttribute(ariaStateAttr, 'false');
				});

			parentContainer.builderCallback('iconview', 'select', entry.row, builder);
			entryContainer.classList.add('selected');
			entryContainer.setAttribute(ariaStateAttr, 'true');

			parentContainer.builderCallback(
				'iconview',
				'contextmenu',
				entry.row,
				builder,
			);
			e.preventDefault();
		});

		if (!singleClick) {
			entryContainer.addEventListener('dblclick', function () {
				parentContainer.builderCallback(
					'iconview',
					'activate',
					entry.row,
					builder,
				);
			});
		}
		builder._preventDocumentLosingFocusOnClick(entryContainer);

		const isInNotebookbar = builder.options.cssClass === 'notebookbar';
		entryContainer.addEventListener('keydown', function (e: KeyboardEvent) {
			if (e.key === ' ' || e.code === 'Space')
				parentContainer.builderCallback(
					'iconview',
					'select',
					entry.row,
					builder,
				);
			else if (e.key === 'Enter')
				parentContainer.builderCallback(
					'iconview',
					'activate',
					entry.row,
					builder,
				);
			else if (
				isInNotebookbar &&
				['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)
			) {
				// In a notebookbar, arrows navigate the toolbar — don't send to core.
			} else {
				parentContainer.builderCallback(
					'iconview',
					'keypress',
					JSDialog.getUNOKeyCodeWithModifiers(e, builder.map),
					builder,
				);
			}
		});

		entryContainer.addEventListener('keyup', function (e: KeyboardEvent) {
			parentContainer.builderCallback(
				'iconview',
				'keyrelease',
				JSDialog.getUNOKeyCodeWithModifiers(e, builder.map),
				builder,
			);
		});
	}
}

JSDialog.iconView = function (
	parentContainer: Element,
	data: IconViewJSON,
	builder: JSBuilder,
) {
	const iconview = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-iconview',
		parentContainer,
	);

	iconview.id = data.id;
	const isMultiSelect = data.selectionmode === 'multiple';

	if (isMultiSelect) {
		iconview.setAttribute('role', 'listbox');
		iconview.setAttribute('aria-multiselectable', 'true');
	} else {
		iconview.setAttribute('role', 'radiogroup');
	}

	if (data.labelledBy) {
		const ids = Array.isArray(data.labelledBy)
			? data.labelledBy.join(' ')
			: data.labelledBy;
		iconview.setAttribute('aria-labelledby', ids);
	}

	const disabled = data.enabled === false;
	if (disabled) window.L.DomUtil.addClass(iconview, 'disabled');

	// Do not animate on creation - eg. when opening sidebar with icon view it might move the app
	const firstSelected = $(iconview).children('.selected').get(0);
	if (firstSelected) {
		const offsetTop = firstSelected.offsetTop;
		iconview.scrollTop = offsetTop;
	}

	iconview.updateSelectionImpl = (
		position: number,
		iconViewData: IconViewJSON,
	) => {
		if (!iconViewData.entries) return;

		for (const entry of iconViewData.entries) {
			entry.selected = false;
		}

		if (position >= 0 && iconViewData.entries.length > position) {
			const entry = iconViewData.entries[position];
			if (entry) entry.selected = true;
		}
	};

	iconview.updateSelection = (position: number) => {
		iconview.updateSelectionImpl(position, data);
	};

	const ariaStateAttr = isMultiSelect ? 'aria-selected' : 'aria-checked';

	// position can be -1
	iconview.onSelect = (position: number) => {
		$(iconview)
			.children('.selected')
			.each(function () {
				$(this).removeClass('selected');
				this.setAttribute(ariaStateAttr, 'false');
			});

		const entry =
			position >= 0 && iconview.children.length > position
				? iconview.children[position]
				: null;

		iconview.updateSelection(position);

		if (entry) {
			window.L.DomUtil.addClass(entry, 'selected');
			entry.setAttribute(ariaStateAttr, 'true');

			if (builder.options.useScrollAnimation !== false) {
				const blockOption = JSDialog.ScrollIntoViewBlockOption('nearest');
				entry.scrollIntoView({
					behavior: 'smooth',
					block: blockOption,
					inline: 'nearest',
				});
			} else {
				const offsetTop = entry.offsetTop;
				iconview.scrollTop = offsetTop;
			}
		} else if (position != -1)
			app.console.warn(
				'not found entry: "' + position + '" in: "' + iconview.id + '"',
			);
	};

	iconview.requestRendersImpl = (
		id: string,
		entry: IconViewEntry,
		placeholder: Element,
		entryContainer: Element,
	) => {
		JSDialog.OnDemandRenderer(
			builder,
			id,
			'iconview',
			entry.row,
			placeholder,
			entryContainer,
			entry.text ? entry.text : entry.tooltip,
		);
	};

	iconview.requestRenders = (
		entry: IconViewEntry,
		placeholder: Element,
		entryContainer: Element,
	) => {
		iconview.requestRendersImpl(data.id, entry, placeholder, entryContainer);
	};

	iconview.updateRendersImpl = (
		pos: number,
		id: string,
		where: HTMLElement,
	) => {
		const dropdown = where.querySelectorAll(
			'.ui-iconview-entry, .ui-iconview-separator',
		);
		if (dropdown[pos]) {
			let container = dropdown[pos] as HTMLElement;
			const entry = data.entries[pos];
			const image = builder.rendersCache[id].images[pos];
			const hasText = entry.text && data.textWithIconEnabled;

			container.classList.remove('ui-iconview-ondemand-placeholder');
			container.replaceChildren();
			if (hasText) {
				container = window.L.DomUtil.create(
					'div',
					builder.options.cssClass,
					dropdown[pos],
				);
			}

			_createEntryImage(container, builder, entry, image);
			if (hasText) _createEntryText(container, entry);
		} else {
			app.console.debug('IconView: not found entry: ' + pos);
		}
	};

	iconview.builderCallback = (
		objectType: string,
		eventType: string,
		entryData: any,
		builder: JSBuilder,
	) => {
		builder.callback(objectType, eventType, data, entryData, builder);
	};

	iconview.updateRenders = (pos: number) => {
		iconview.updateRendersImpl(pos, data.id, iconview);
	};

	// In a notebookbar (toolbar), arrow keys are handled by the toolbar's
	// own navigation — radio group entries are navigated like any other
	// toolbar item, without changing selection (WAI-ARIA APG radio-in-toolbar).
	const inNotebookbar = builder.options.cssClass === 'notebookbar';
	if (!inNotebookbar) {
		if (isMultiSelect) JSDialog.KeyboardListNavigation(iconview);
		else JSDialog.KeyboardRadioGroupNavigation(iconview);
	}

	iconview.addEventListener('focusin', function (e: FocusEvent) {
		const target = e.target as HTMLElement;

		/*
		 * when the iconview is shown in a dropdown and is the first
		 * child of the dropdown, it gets selected by default which
		 * is not desirable as that shows a blue frame around the
		 * iconview.
		 */
		if (target === iconview) {
			target.setAttribute('tabindex', '-1');
			return;
		}
	});

	app.layoutingService.appendLayoutingTask(() => {
		const shouldSelectFirstEntry =
			data.entries?.length > 0
				? !data.entries.some((entry) => entry.selected === true)
				: false;
		if (shouldSelectFirstEntry) {
			const firstValid = data.entries.find((entry) => !entry.separator);
			if (firstValid) firstValid.selected = true;
		}

		for (const i in data.entries) {
			_iconViewEntry(iconview, data, data.entries[i], builder);
		}

		// Do not animate on creation - eg. when opening sidebar with icon view it might move the app
		const firstSelected = $(iconview).children('.selected').get(0);
		if (firstSelected) {
			const offsetTop = firstSelected.offsetTop;
			iconview.scrollTop = offsetTop;
		}
	});

	return false;
};
