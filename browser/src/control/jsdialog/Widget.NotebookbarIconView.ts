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

function _createButtonForNotebookbarIconview(
	parentContainer: Element,
	id: string,
	buttonClass: string,
	icon: string,
	ariaLabel: string,
	builder: JSBuilder,
	onClickCallback: any,
	accessibility?: NotebookbarAccessibilityDescriptor,
	opensPopup?: boolean,
) {
	const container = window.L.DomUtil.create(
		'div',
		builder.options.cssClass +
			' unotoolbutton ui-content unospan no-label ui-iconview-button',
		parentContainer,
	);
	container.id = id;

	// create the button
	const button = window.L.DomUtil.create(
		'button',
		'ui-content unobutton ' + buttonClass,
		container,
	);
	button.id = id + '-button';
	if (accessibility?.combination) button.accessKey = accessibility?.combination;

	if (opensPopup) button.setAttribute('aria-haspopup', 'dialog');

	const a11yData: WidgetJSON = {
		id: id,
		type: 'iconview-button',
		aria: {
			label: ariaLabel,
		},
	};
	JSDialog.SetupA11yLabelForNonLabelableElement(button, a11yData, builder);

	// add the icon
	const buttonImage = window.L.DomUtil.create(
		'img',
		'ui-iconview-button-icon',
		button,
	);
	buttonImage.alt = '';
	app.LOUtil.setImage(buttonImage, icon, builder.map);

	// set the onclick callback
	button.onclick = onClickCallback;
}

function _getDropdownContent(data: IconViewListJSON, builder: JSBuilder) {
	const dropdownContent: Array<MenuDefinition> = [];
	for (const child of data.children) {
		// get up-to-date copy from model, TODO: more clean way to do that
		const childData = (
			builder?.options?.mobileWizard as any
		)?.getWidgetSnapshot(child.id);

		dropdownContent.push({
			id: 'dropdown-entry-' + child.id,
			type: 'json',
			content: childData ? childData : undefined,
		});
	}

	if (
		data.children.length === 1 &&
		data.children[0].id === 'tablestyles_design'
	) {
		// These entries are built anew every time the dropdown opens, so they take
		// the state their command is already known to be in. Waiting for the next
		// state change would leave them usable while the gallery beside them is not.
		const isCommandEnabled = (command: string) =>
			builder.map.stateChangeHandler.getItemValue(command) !== 'disabled';

		dropdownContent.push(
			{
				id: 'dropdown-entry-tablestyles-separator',
				type: 'json',
				content: {
					type: 'separator',
					id: 'tablestyles-actions-separator',
					orientation: 'horizontal',
				} as SeparatorWidgetJSON,
			},
			{
				id: 'dropdown-entry-new-table-style',
				type: 'json',
				content: {
					id: 'new-table-style',
					type: 'customtoolitem',
					text: _('New Table Style...'),
					command: '.uno:NewTableStyle',
					icon: 'lc_newtablestyle.svg',
					enabled: isCommandEnabled('.uno:NewTableStyle'),
				} as ToolItemWidgetJSON,
			},
			{
				id: 'dropdown-entry-clear-table-style',
				type: 'json',
				content: {
					id: 'clear-table-style',
					type: 'customtoolitem',
					text: _('Clear Style'),
					command: '.uno:ClearTableStyle',
					icon: 'lc_cleartablestyle.svg',
					enabled: isCommandEnabled('.uno:ClearTableStyle'),
				} as ToolItemWidgetJSON,
			},
		);
	}

	if (data.children.length === 1 && data.children[0].id === 'stylesview') {
		dropdownContent.push(
			{
				id: 'dropdown-entry-stylesview',
				type: 'json',
				content: {
					type: 'separator',
					id: 'iconview-button-separator',
					orientation: 'horizontal',
				} as SeparatorWidgetJSON,
			},
			{
				id: 'dropdown-entry-more',
				type: 'json',
				content: {
					id: 'format-style-list-dialog',
					type: 'customtoolitem',
					text: _('Open Styles Sidebar'),
					command: 'showstylelistdeck',
					icon: 'lc_stylepreviewmore.svg',
				} as ToolItemWidgetJSON,
			},
		);
	}

	return dropdownContent;
}

JSDialog.notebookbarIconViewList = function (
	parentContainer: Element,
	data: IconViewListJSON,
	builder: JSBuilder,
) {
	const rootNode = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-iconview-root',
		parentContainer,
	);
	rootNode.id = data.id;

	const commonContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-iconview-window',
		rootNode,
	);
	commonContainer.id = data.id + '-window';

	let firstIconViewIndex = 0;

	while (data.children && data.children[firstIconViewIndex]) {
		if (data.children[firstIconViewIndex].type === 'iconview') break;
		firstIconViewIndex++;
	}

	const iconViewData = data.children[firstIconViewIndex];

	// we insert into DOM only the first iconview (rest is accessible only in the dropdown)
	JSDialog.iconView(commonContainer, iconViewData, builder);
	// builder will not do it for us - we manage children (return is false in this handler)
	builder.postProcess(commonContainer, iconViewData);

	const iconview = commonContainer.querySelector('.ui-iconview');
	if (!iconview) {
		app.console.error('IconView cannot be created: ' + data.id);
		return false;
	}

	const buttonsContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-iconview-buttons-container',
		rootNode,
	);
	buttonsContainer.id = data.id + '-buttons-container';

	const horizontal = data.horizontal === true;
	if (horizontal)
		window.L.DomUtil.addClass(rootNode, 'ui-iconview-root-horizontal');

	// be aware the child iconviews can get update and be replaced in DOM
	// we need to use firstChild to get correct instance at the time of execution
	const scrollBySteps = (steps: number) => {
		const current = commonContainer.firstChild as HTMLElement;
		if (horizontal)
			current.scrollBy({
				left: steps * current.offsetWidth,
				behavior: 'smooth',
			});
		else
			current.scrollBy({
				top: steps * current.offsetHeight,
				behavior: 'smooth',
			});
	};

	const scrollUpCallback = () => {
		scrollBySteps(-1);
	};

	const scrollDownCallback = () => {
		scrollBySteps(1);
	};

	const notebookbarIconViewCallback = (
		objectType: string,
		eventType: string,
		object: any,
		entry_data: string,
	) => {
		builder.callback(objectType, eventType, object, entry_data, builder);
		/*
			the dropdown can have controls to trigger dialogs
			or sidebars. when that happens, we want the dropdown
			to move out of our way.
		*/
		if (objectType !== 'iconview') JSDialog.CloseAllDropdowns();
	};

	const openDropdown = (popupParent: HTMLElement) => {
		JSDialog.CloseAllDropdowns();

		JSDialog.OpenDropdown(
			data.id,
			popupParent,
			_getDropdownContent(data, builder),
			notebookbarIconViewCallback,
		);
	};

	const expanderCallback = () => {
		openDropdown(rootNode);
	};

	_createButtonForNotebookbarIconview(
		buttonsContainer,
		data.id + '-scroll-up',
		'ui-iconview-scroll-up-button',
		horizontal ? 'lc_prevrecord.svg' : 'lc_searchprev.svg',
		horizontal ? _('Scroll left') : _('Scroll up'),
		builder,
		scrollUpCallback,
	);

	_createButtonForNotebookbarIconview(
		buttonsContainer,
		data.id + '-scroll-down',
		'ui-iconview-scroll-down-button',
		horizontal ? 'lc_nextrecord.svg' : 'lc_searchnext.svg',
		horizontal ? _('Scroll right') : _('Scroll down'),
		builder,
		scrollDownCallback,
	);

	_createButtonForNotebookbarIconview(
		buttonsContainer,
		data.id + '-expand',
		'ui-iconview-expander-button',
		'lc_iconviewexpander.svg',
		_('More options'),
		builder,
		expanderCallback,
		{
			focusBack: true,
			combination: data.expanderAccessKey || 'SD',
			de: null,
		},
		true /* opensPopup */,
	);

	// below, when given, is the element the dropdown opens under. Without it the dropdown
	// takes the place the stylesheet gives it, over the gallery.
	const setupOpenedDropdown = (
		overlayParent: HTMLElement,
		below?: HTMLElement,
	) => {
		app.layoutingService.appendLayoutingTask(() => {
			app.layoutingService.appendLayoutingTask(() => {
				const expander = JSDialog.GetDropdown(data.id);
				if (!expander) {
					app.console.error(
						'iconview._onDropDown: expander missing: "' + data.id + '"',
					);
					return;
				}
				const overlay = expander.parentNode;
				overlay.style.position = 'fixed';
				overlay.style.zIndex = '20000';
				overlayParent.appendChild(overlay);
				if (below)
					expander.style.setProperty(
						'margin-top',
						below.getBoundingClientRect().bottom + 'px',
						'important',
					);

				// setup correct callbacks for rendering actions for fresh instance
				// both in notebookbar and inside dropdown
				const currentIconView = commonContainer.firstChild as IconViewElement;
				currentIconView.updateRenders = (pos: number) => {
					currentIconView.updateRendersImpl(
						pos,
						iconViewData.id,
						currentIconView,
					);
					currentIconView.updateRendersImpl(pos, iconViewData.id, expander);
				};
			});
		});
	};

	rootNode._onDropDown = function (opened: boolean) {
		if (opened) setupOpenedDropdown(rootNode);
	};

	const gallery = rootNode as IconViewGalleryElement;

	// When its overflow group is folded the gallery is not displayed, and
	// the button standing in for the group opens the dropdown instead.
	gallery.openFoldedDropdown = (
		button: HTMLElement,
		overlayParent: HTMLElement,
	) => {
		openDropdown(button);
		setupOpenedDropdown(overlayParent, button);
	};

	// The overflow manager narrows the gallery before it folds any group,
	// null gives back the width from the stylesheet. That width is set with
	// !important on the iconview, which is replaced on every update, so the
	// width goes on as an !important inline style and again on each new one.
	let galleryWidth: number | null = null;
	const applyGalleryWidth = () => {
		const current = commonContainer.firstChild as HTMLElement;
		if (!current) return;
		for (const property of ['width', 'min-width']) {
			if (galleryWidth === null) current.style.removeProperty(property);
			else
				current.style.setProperty(property, galleryWidth + 'px', 'important');
		}
	};
	new MutationObserver(applyGalleryWidth).observe(commonContainer, {
		childList: true,
	});

	// The minimum width is measured from the entries, which arrive after the notebookbar is
	// built, so the fold decision runs again when the first ones come.
	const entriesObserver = new MutationObserver(() => {
		if (!commonContainer.querySelector('.ui-iconview-entry')) return;
		entriesObserver.disconnect();
		app.layoutingService.appendLayoutingTask(() =>
			builder.map.fire('refreshoverflows', { force: true }),
		);
	});
	if (!commonContainer.querySelector('.ui-iconview-entry'))
		entriesObserver.observe(commonContainer, {
			childList: true,
			subtree: true,
		});

	gallery.setGalleryWidth = (width: number | null) => {
		galleryWidth = width;
		applyGalleryWidth();
	};

	gallery.getGalleryWidth = () => {
		const current = commonContainer.firstChild as HTMLElement;
		return current ? current.offsetWidth : 0;
	};

	// The narrowest width depends only on the entries, so it is measured once for each iconview
	// and number of entries, and measured again when the iconview is replaced or entries arrive.
	let measuredMinWidth: {
		iconView: HTMLElement;
		entries: number;
		width: number;
	} | null = null;

	// the narrowest the gallery gets: one entry
	gallery.getGalleryMinWidth = () => {
		const current = commonContainer.firstChild as HTMLElement;
		if (!current) return 0;

		const width = current.offsetWidth;
		const entries = current.querySelectorAll(':scope > .ui-iconview-entry');
		if (entries.length <= 1) return width;

		if (
			measuredMinWidth &&
			measuredMinWidth.iconView === current &&
			measuredMinWidth.entries === entries.length
		)
			return Math.min(width, measuredMinWidth.width);

		const style = window.getComputedStyle(current);
		const frame =
			current.offsetWidth -
			current.clientWidth +
			parseFloat(style.paddingLeft) +
			parseFloat(style.paddingRight);
		// An entry stretches to fill its column, so it is measured with the gallery squeezed to
		// nothing, where it has the narrowest width the grid allows. The width then does not
		// depend on how many columns the full gallery happens to have.
		for (const property of ['width', 'min-width'])
			current.style.setProperty(property, '0px', 'important');
		const entryWidth = (entries[0] as HTMLElement).offsetWidth;
		applyGalleryWidth();

		measuredMinWidth = {
			iconView: current,
			entries: entries.length,
			width: Math.ceil(entryWidth + frame),
		};
		return Math.min(width, measuredMinWidth.width);
	};

	/*
		close dropdown when the window is resized. this
		is to prevent dropdown from hanging in the corner
		when the overflowgroups collapse displacing the
		underlying iconview.
	*/

	// update indexes on resize
	const resizeObserver = new ResizeObserver(() => {
		JSDialog.UpdateIconViewIndexes(commonContainer.firstChild as HTMLElement);
		const dropdown = JSDialog.GetDropdown(data.id);
		if (dropdown) JSDialog.CloseDropdown(data.id);
	});

	resizeObserver.observe(rootNode);

	if (data.nameFromIconView) {
		const nameGroupAfterIconView = () => {
			const current = commonContainer.firstChild as HTMLElement;
			const name = current ? current.getAttribute('aria-label') : null;
			const group = rootNode.closest('.ui-overflow-group');
			if (!name || !group) return;

			const caption = group.querySelector(
				'.ui-overflow-group-label',
			) as HTMLElement;
			if (caption) caption.innerText = name;
			group
				.querySelector('.ui-overflow-group-inner')
				?.setAttribute('aria-label', name);
		};

		new MutationObserver(nameGroupAfterIconView).observe(commonContainer, {
			childList: true,
		});
		app.layoutingService.appendLayoutingTask(nameGroupAfterIconView);
	}

	// Do not animate on creation - eg. when opening sidebar with icon view it might move the app
	const firstSelected = $(iconview).children('.selected').get(0);
	if (firstSelected) {
		const offsetTop = firstSelected.offsetTop;
		iconview.scrollTop = offsetTop;
	}

	return false;
};
