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
 * JSDialog.OverflowGroup - can hide items from last to first if requested and
 *                          add dropdown menu to access them instead
 */

declare var JSDialog: any;

function migrateItems(from: HTMLElement, to: HTMLElement) {
	if (!from) {
		app.console.debug('overflow manager: no source for migration');
		return;
	}

	const items = from.querySelectorAll(':scope > *');

	items.forEach((button: Element) => {
		const htmlButton = button as HTMLElement;
		to.appendChild(htmlButton);
	});

	const parentContainerData = to.getBoundingClientRect();
	if (parentContainerData.right > window.innerWidth) {
		setupOverflowMenuPosition(parentContainerData, to);
	}
}

function setupOverflowMenuPosition(
	parentContainerData: DOMRect,
	overflowDialog: HTMLElement,
) {
	// Overflow amount
	const overflowAmount = parentContainerData.right - window.innerWidth;

	// Read current inline margin (not computed style)
	const currentMarginInlineStart = parseFloat(
		overflowDialog.style.marginInlineStart || '0',
	);

	// Adjust
	const newMarginInlineStart = currentMarginInlineStart - overflowAmount - 4; // extra 4px is for the added padding for overflowpoup
	overflowDialog.style.marginInlineStart = `${newMarginInlineStart}px`;
}

function createMoreButton(
	id: string,
	more: MoreOptions,
	groupLabel: HTMLElement,
): HTMLElement {
	const moreOptionsButton = document.createElement('button');
	moreOptionsButton.className = 'ui-content unobutton';
	moreOptionsButton.id = `${id}-more-button`;
	moreOptionsButton.setAttribute('aria-label', `More options for ${id}`);
	moreOptionsButton.setAttribute('aria-haspopup', 'dialog');

	const img = document.createElement('img');
	img.alt = '';
	app.LOUtil.setImage(img, 'lc_overflow-morebutton.svg', app.map);
	moreOptionsButton.appendChild(img);

	const unoToolButtonDiv = document.createElement('div');
	unoToolButtonDiv.id = `${id}-more`;
	unoToolButtonDiv.className = 'unotoolbutton ui-overflow-group-more';
	unoToolButtonDiv.tabIndex = -1;
	unoToolButtonDiv.setAttribute('data-cooltip', `More options for ${id}`);
	unoToolButtonDiv.appendChild(moreOptionsButton);

	const expanderIconRightDiv = document.createElement('div');
	expanderIconRightDiv.className = 'ui-expander-icon-right jsdialog sidebar';
	expanderIconRightDiv.appendChild(unoToolButtonDiv);

	if (groupLabel.parentElement) {
		groupLabel.parentElement.appendChild(expanderIconRightDiv);
	}

	moreOptionsButton.addEventListener('click', (e) => {
		e.stopPropagation();
		e.preventDefault();
		if (more.command.startsWith('.uno:')) app.map.sendUnoCommand(more.command);
		else app.dispatcher.dispatch(more.command);
	});

	return expanderIconRightDiv;
}

function setupOverflowMenu(
	overflowGroupContainer: HTMLElement,
	overflowGroupContentContainer: HTMLElement,
	bottomBar: HTMLElement,
	id: string,
	more: MoreOptions | undefined,
) {
	const overflowMenuButton = overflowGroupContainer.querySelector(
		'[id^="overflow-button"]',
	) as HTMLElement;
	overflowMenuButton.style.display = 'none';

	const groupLabel = overflowGroupContainer.querySelector(
		'.ui-overflow-group-label',
	) as HTMLElement;

	if (more) {
		createMoreButton(id, more, groupLabel);
	}

	// keeps hidden items
	const hiddenItems = window.L.DomUtil.create(
		'div',
		'hidden-overflow-container',
		overflowGroupContainer,
	);
	hiddenItems.style.display = 'none';
	overflowGroupContainer.classList.add(
		'ui-overflow-group-container-with-label',
	);

	// keeps original content
	const originalTopbar =
		overflowGroupContentContainer.querySelectorAll(':scope > *');

	// hide/show content on resize
	const overflowMenuHandler = (hideContent: boolean) => {
		overflowGroupContentContainer.replaceChildren();
		hiddenItems.replaceChildren();

		if (hideContent) {
			// top row container
			const topRow = document.createElement('div');
			topRow.classList.add('top-row-overflow-group');
			originalTopbar.forEach((el) => topRow.append(el));
			overflowGroupContentContainer.append(topRow, bottomBar ?? null);
			migrateItems(overflowGroupContentContainer, hiddenItems);
		} else {
			// show normally
			originalTopbar.forEach((el) => overflowGroupContentContainer.append(el));
			hiddenItems.replaceChildren(); // do not allow to duplicate buttons
			if (bottomBar) {
				// make sure bottom bar gets it's original position after unfold
				overflowGroupContainer?.append(bottomBar);
			}
		}
	};

	const dropdownId = 'overflow-button-' + id;
	let isCollapsed = false;

	(overflowGroupContainer as OverflowGroupContainer).isCollapsed = () => {
		return isCollapsed;
	};

	(overflowGroupContainer as OverflowGroupContainer).foldGroup = () => {
		if (isCollapsed) return;
		app.console.debug('overflow manager: fold group: ' + id);
		JSDialog.CloseDropdown(dropdownId);

		overflowMenuHandler(true);
		overflowGroupContainer.classList.remove(
			'ui-overflow-group-container-with-label',
		);
		overflowMenuButton.style.display = '';
		isCollapsed = true;
	};

	(overflowGroupContainer as OverflowGroupContainer).unfoldGroup = () => {
		if (!isCollapsed) return;

		app.console.debug('overflow manager: unfold group: ' + id);
		JSDialog.CloseDropdown(dropdownId);

		overflowMenuButton.style.display = 'none';
		overflowGroupContainer.classList.add(
			'ui-overflow-group-container-with-label',
		);
		overflowMenuHandler(false);
		isCollapsed = false;
	};

	const updateAriaExpandedAttr = (
		container: HTMLElement,
		expectedValue: boolean,
	) => {
		const buttons = container.querySelectorAll<HTMLElement>(
			`[aria-expanded="${!expectedValue}"]`,
		);
		buttons.forEach((button: HTMLElement) => {
			button.setAttribute('aria-expanded', String(expectedValue));
		});
	};

	// fill the updated menu after it is open
	(overflowMenuButton as any)._onDropDown = (opened: boolean) => {
		if (opened) {
			overflowMenuButton.classList.add('overflow-dropdown-active');
			updateAriaExpandedAttr(overflowMenuButton, opened);

			// we need to schedule it 2 times as the first one happens just before
			// layouting task adds menu container to the DOM
			app.layoutingService.appendLayoutingTask(() => {
				app.layoutingService.appendLayoutingTask(() => {
					const menu = JSDialog.GetDropdown(dropdownId);
					if (!menu) {
						app.console.error(
							'overflow manager: menu missing: "' + dropdownId + '"',
						);
						return;
					}

					// only proceed if hiddenItems has content
					const hiddenItemsChildren =
						hiddenItems.querySelectorAll(':scope > *');
					if (hiddenItemsChildren.length === 0) {
						app.console.debug(
							'overflow manager: no items to migrate for "' + dropdownId + '"',
						);
						return;
					}

					// move overflow to the NB structure to be targeted by onJSUpdate and onJSAction
					const overflowNode = menu.parentNode;
					overflowNode.style.position = 'fixed';
					overflowNode.style.zIndex = '20000';

					// calculate baseline-aligned position specifically for overflow menus
					const tabContainer = overflowMenuButton.closest(
						'.ui-tabs-content, [id$="-container"]',
					);

					if (tabContainer) {
						// use tab container's bottom as baseline for overflow menus only
						const tabRect = tabContainer.getBoundingClientRect();
						menu.style.marginTop = tabRect.bottom + 'px';
					}

					overflowGroupContainer.appendChild(overflowNode);
					menu.replaceChildren();
					menu.classList.add('ui-toolbar');
					menu.classList.add('ui-overflow-group-popup');

					migrateItems(hiddenItems, menu);
					menu.addEventListener('keydown', function (e: KeyboardEvent) {
						let key;
						if (e.key === 'Tab') {
							key = e.shiftKey ? 'ArrowLeft' : 'ArrowRight';
						} else {
							key = e.key;
						}

						if (
							['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(key)
						) {
							var currentElement = e.target as HTMLElement;
							const isElementOfTypeInput =
								currentElement.tagName === 'INPUT' ||
								currentElement.tagName === 'TEXTAREA';

							if (
								!isElementOfTypeInput ||
								(isElementOfTypeInput && e.key === 'Tab')
							) {
								const elementToFocus = JSDialog.FindNextElementInContainer(
									menu,
									currentElement,
									key,
								);
								if (elementToFocus) {
									elementToFocus.focus();
								} else {
									// When ray-casting reaches container boundaries
									const forwardDir =
										key === 'ArrowRight' || key === 'ArrowDown';
									const focusables = JSDialog.GetFocusableElements(menu);
									if (focusables && focusables.length > 0) {
										const currentIndex = focusables.indexOf(currentElement);
										let targetIndex = forwardDir
											? currentIndex + 1
											: currentIndex - 1;

										if (targetIndex < 0) targetIndex = focusables.length - 1;
										else if (targetIndex >= focusables.length) targetIndex = 0;

										(focusables[targetIndex] as HTMLElement).focus();
									}
								}
								// If we handled the event here, stop propagation and prevent default
								// to avoid reaching to global notebookbar keyboard handling
								e.stopPropagation();
								e.preventDefault();
							}
						}
					});
				});
			});
		} else {
			overflowMenuButton.classList.remove('overflow-dropdown-active');
			updateAriaExpandedAttr(overflowMenuButton, opened);

			const menu = JSDialog.GetDropdown(dropdownId);
			migrateItems(menu, hiddenItems);
		}
	};
}

function findFirstToolitem(
	items: Array<WidgetJSON> | undefined,
): WidgetJSON | null {
	if (!items) return null;

	for (const item of items) {
		if (
			item.type.indexOf('toolitem') >= 0 ||
			item.type.indexOf('colorlistbox') >= 0 ||
			item.type.indexOf('menubutton') >= 0 ||
			item.type.indexOf('checkbox') >= 0
		)
			return item;
		else if (item.children && item.children.length) {
			const innerItem = findFirstToolitem(item.children);
			if (innerItem !== null) return innerItem;
		}
	}
	return null;
}

// TODO: add correct type
function getToolitemIcon(item: WidgetJSON | null): string {
	if (!item) return 'lc_menuoverflow.svg';

	if ((item as any).icon !== undefined) return (item as any).icon;
	else return app.LOUtil.getIconNameOfCommand((item as any).command);
}

JSDialog.OverflowGroup = function (
	parentContainer: Element,
	data: OverflowGroupWidgetJSON,
	builder: JSBuilder,
) {
	const overflowGroupContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass +
			' ui-overflow-group' +
			(data.nofold ? ' nofold' : ''),
		parentContainer,
	);
	overflowGroupContainer.id = data.id;

	const overflowGroupInnerContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-overflow-group-inner',
		overflowGroupContainer,
	);
	overflowGroupInnerContainer.id = data.id + '-inner';
	overflowGroupInnerContainer.setAttribute('role', 'group');

	const contentContainer = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-overflow-group-content',
		overflowGroupInnerContainer,
	);
	contentContainer.classList.add(data.vertical ? 'vertical' : 'horizontal');
	contentContainer.id = data.id + '-content';

	const bottomBar = window.L.DomUtil.create(
		'div',
		builder.options.cssClass + ' ui-overflow-group-bottom',
		overflowGroupContainer,
	);
	bottomBar.id = data.id + '-bottom';

	const label = window.L.DomUtil.create(
		'span',
		builder.options.cssClass + ' ui-overflow-group-label',
		bottomBar,
	);
	label.id = data.id + '-label';
	if (data.name) label.innerText = data.name;

	if (data.name) {
		overflowGroupInnerContainer.setAttribute('aria-label', data.name);
	}

	// content
	if (data.children) builder.build(contentContainer, data.children, false);
	else app.console.error('OverflowGroup: no content provided');

	// first toolitem in the group
	const firstItem = findFirstToolitem(data.children);
	if (!firstItem) {
		app.console.warn(
			'OverflowGroup: First toolitem inside overflow group not found',
		);
	}

	// placeholder menu for a dropdown
	const builtMenu = [
		{
			type: 'json',
			content: {
				id: 'overflow-group-placeholder-' + data.id,
				type: 'toolbox',
				children: [
					{
						id: 'dummy-overflow-button-required-for-focus',
						type: 'pushbutton',
						text: 'X',
					},
				],
			} as WidgetJSON,
		},
		{ type: 'separator' },
	] as Array<MenuDefinition>;
	builder._menus.set(data.id, builtMenu);

	if (data.nofold === true) return false;

	// button
	const id = 'overflow-button-' + data.id;
	builder.build(
		overflowGroupInnerContainer,
		[
			{
				type: 'menubutton',
				id: id,
				text: data.name ? data.name : firstItem ? (firstItem as any).text : '',
				icon: data.icon ? data.icon : getToolitemIcon(firstItem),
				command: firstItem ? (firstItem as any).command : '',
				noLabel: !data.name,
				menu: builtMenu,
				applyCallback:
					firstItem && firstItem.type === 'toolitem'
						? () => {
								firstItem.type.includes('custom')
									? app.dispatcher.dispatch((firstItem as any).command)
									: app.map.sendUnoCommand((firstItem as any).command);
							}
						: undefined,
			} as MenuButtonWidgetJSON,
		],
		false,
	);

	setupOverflowMenu(
		overflowGroupContainer,
		contentContainer,
		bottomBar,
		data.id,
		data.more,
	);

	return false;
} as JSWidgetHandler;
