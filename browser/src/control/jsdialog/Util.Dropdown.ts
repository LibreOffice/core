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
 * Util.Dropdown - helper to create dropdown menus for JSDialogs
 */

declare var JSDialog;

function _createDropdownId(id: string) {
	return id + '-dropdown';
}

JSDialog.CreateDropdownEntriesId = function (id: string) {
	return id + '-entries';
};

JSDialog.OpenDropdown = function (
	id: string,
	popupParent:
		| string
		| (HTMLElement & { _onDropDown: (open: boolean) => void }),
	entries: Array<ComboBoxEntry>,
	innerCallback: JSDialogMenuCallback,
	popupAnchor: string,
	isSubmenu: boolean,
	earlyCallbackCall?: boolean,
	noDefaultSelection?: boolean,
) {
	const json = {
		id: _createDropdownId(id),
		type: 'dropdown',
		isSubmenu: isSubmenu,
		jsontype: 'dialog',
		popupParent: popupParent,
		popupAnchor: popupAnchor,
		gridKeyboardNavigation: false,
		cancellable: true,
		children: [
			{
				id: JSDialog.CreateDropdownEntriesId(id),
				type: 'grid',
				allyRole: 'listbox',
				cols: 1,
				rows: entries.length,
				children: [] as Array<WidgetJSON>,
			} as GridWidgetJSON,
		],
	} as JSDialogJSON;

	if (
		popupParent &&
		typeof popupParent !== 'string' &&
		typeof popupParent._onDropDown === 'function'
	) {
		popupParent._onDropDown(true);
	}

	const isChecked = function (unoCommand: string) {
		const items = window.L.Map.THIS['stateChangeHandler'];
		const val = items.getItemValue(unoCommand);

		if (val && (val === true || val === 'true')) return true;
		else return false;
	};

	for (let i = 0; i < entries.length; i++) {
		if (entries[i].statusCommand) {
			const items = window.L.Map.THIS['stateChangeHandler'];
			const val = items.getItemValue(entries[i].statusCommand);
			if (val) {
				const index = parseInt(val);
				if (index === i) {
					entries[i].selected = true;
				} else {
					entries[i].selected = false;
				}
			}
		}
	}

	const shouldSelectFirstEntry =
		!noDefaultSelection && entries.length > 0
			? !entries.some((entry) => entry.selected === true)
			: false;
	let initialSelectedId;

	for (let i = 0; i < entries.length; i++) {
		const checkedValue =
			entries[i].checked === undefined
				? undefined
				: entries[i].uno && isChecked('.uno' + entries[i].uno);

		let entry:
			| WidgetJSON
			| SeparatorWidgetJSON
			| HtmlContentJson
			| MenuDefinition
			| ComboBoxEntry
			| null = null;

		switch (entries[i].type) {
			// DEPRECACTED: legacy plain HTML adapter
			case 'html':
				entry = {
					id: id + '-entry-' + i,
					type: 'htmlcontent',
					htmlId: entries[i].htmlId,
					closeCallback: function () {
						JSDialog.CloseDropdown(id);
					},
				} as HtmlContentJson;
				json.gridKeyboardNavigation = true;
				break;

			// dropdown is a colorpicker
			case 'colorpicker':
				entry = entries[i];
				// for color picker we have a "KeyboardGridNavigation" function defined separately to handle custom cases
				json.gridKeyboardNavigation = true;
				break;

			// allows to put regular JSDialog JSON into popup
			case 'json':
				entry =
					typeof entries[i].content !== 'undefined'
						? (entries[i].content as ComboBoxEntry)
						: null;
				initialSelectedId = entry
					? (entry as ComboBoxEntry).initialSelectedId
					: undefined;
				if (entry?.type === 'grid') json.gridKeyboardNavigation = true;
				break;

			// horizontal separator in menu
			case 'separator':
				entry = {
					id: id + '-entry-' + i,
					type: 'separator',
					orientation: 'horizontal',
				} as SeparatorWidgetJSON;
				break;

			// menu and submenu entry
			case 'action':
			case 'menu':
			default:
				entry = {
					id: id + '-entry-' + i,
					type: 'comboboxentry',
					customRenderer: entries[i].customRenderer,
					comboboxId: id,
					pos: i,
					text: entries[i].text,
					hint: entries[i].hint,
					w2icon: entries[i].icon, // FIXME: DEPRECATED
					icon: entries[i].img,
					checked: entries[i].checked || checkedValue,
					selected:
						i === 0 && shouldSelectFirstEntry ? true : entries[i].selected,
					hasSubMenu: !!entries[i].items,
				} as ComboBoxEntry;
				if ((entry as ComboBoxEntry).selected) initialSelectedId = entry.id;
				break;
		}

		if (entry && json?.children?.length) json.children[0].children?.push(entry);
	}

	if (initialSelectedId && json?.children?.length) {
		json.init_focus_id = initialSelectedId;
		(json.children[0] as ComboBoxEntry).initialSelectedId = initialSelectedId;
	}

	const generateCallback = function (
		targetEntries: Array<MenuDefinition>,
	): JSDialogCallback {
		let lastSubMenuOpened: string | null = null;
		const closeLastSubMenu = () => {
			if (!lastSubMenuOpened) return;
			JSDialog.CloseDropdown(lastSubMenuOpened);
			lastSubMenuOpened = null;
		};

		return function (
			objectType: string,
			eventType: string,
			object: any,
			data: any,
			builder: JSBuilder,
		) {
			let pos = -1;
			if (typeof data === 'number') pos = data;
			else pos = data ? parseInt(data.substr(0, data.indexOf(';'))) : -1;
			const entry = targetEntries && pos >= 0 ? targetEntries[pos] : null;
			if (entry) {
				entry.pos = pos;
			}
			const subMenuId = object.id + '-' + pos;

			if (eventType === 'selected' || eventType === 'showsubmenu') {
				if (entry && entry.items) {
					closeLastSubMenu();

					// open submenu
					const dropdown = JSDialog.GetDropdown(object.id);
					const allEntries = dropdown.querySelectorAll('.ui-grid-cell');
					const index = pos + 1;
					const targetEntry =
						allEntries && allEntries.length > index ? allEntries[index] : null;
					JSDialog.OpenDropdown(
						subMenuId,
						targetEntry,
						entry.items,
						generateCallback(entry.items),
						'top-end',
						true,
						earlyCallbackCall,
						noDefaultSelection,
					);
					lastSubMenuOpened = subMenuId;

					app.layoutingService.appendLayoutingTask(() => {
						const dropdown = JSDialog.GetDropdown(subMenuId);
						if (!dropdown) {
							console.debug('Dropdown: missing :' + subMenuId);
							return;
						}
						const container = dropdown.querySelector('.ui-grid');
						JSDialog.MakeFocusCycle(container);
						const focusables = JSDialog.GetFocusableElements(container);
						if (focusables && focusables.length) focusables[0].focus();
					});

					return;
				} else if (eventType === 'selected' && entry && entry.uno) {
					if (earlyCallbackCall && innerCallback) {
						innerCallback(objectType, eventType, object, data, entry);
					} else {
						const uno =
							entry.uno.indexOf('.uno:') === 0
								? entry.uno
								: '.uno:' + entry.uno;
						window.L.Map.THIS.sendUnoCommand(uno);
					}
					JSDialog.CloseAllDropdowns();
					return;
				} else {
					app.console.debug(
						'Dropdown: potential unhandled action: "' + eventType + '"',
					);
				}
			} else if (eventType === 'hidedropdown') {
				closeLastSubMenu();
				JSDialog.CloseDropdown(id);
				return;
			}

			// for multi-level menus last parameter should be used to handle event (it contains selected entry)
			// usually last param is builder see: JSDialogCallback
			if (
				!earlyCallbackCall &&
				innerCallback &&
				innerCallback(objectType, eventType, object, data, entry || builder)
			)
				return;

			if (eventType === 'selected') JSDialog.CloseAllDropdowns();

			// we want to send render request to the default callback -> no warn
			if (eventType === 'render_entry') return;

			app.console.warn(
				'Dropdown: unhandled action: "' +
					eventType +
					'" for entry: "' +
					JSON.stringify(entry) +
					'"',
			);
		};
	};
	if (!isSubmenu) {
		window.L.Map.THIS.fire('closepopups'); // close popups if a dropdown menu is opened
	}
	window.L.Map.THIS.fire('jsdialog', {
		data: json,
		callback: generateCallback(entries),
	});
};

JSDialog.CloseDropdown = function (id: string, focusHandled?: boolean) {
	window.L.Map.THIS.fire('jsdialog', {
		data: {
			id: _createDropdownId(id),
			jsontype: 'dialog',
			action: 'close',
			focusHandled: focusHandled === true,
		},
	});
};

JSDialog.CloseAllDropdowns = function () {
	window.L.Map.THIS.jsdialog.closeAllDropdowns();
	if (window.L.Map.THIS.contextToolbar)
		window.L.Map.THIS.contextToolbar.hideContextToolbar();
};

JSDialog.GetDropdown = function (id: string) {
	// remember it can get some random numbers due to JSDialog.MakeIdUnique
	// TODO: use some register for it
	return document.body.querySelector('[id^="' + id + '"].modalpopup');
};
