/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
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

function _createHeader(id: string, text: string): WidgetJSON {
	const characters = Array.from(text);
	let endStart = characters.length - 12;
	for (let i = endStart; i > endStart - 8 && i > 0; i--) {
		if (characters[i - 1] === ' ') {
			endStart = i;
			break;
		}
	}
	if (characters.length <= 16)
		return {
			id: id,
			type: 'fixedtext',
			text: text,
			tooltip: text,
		} as TextWidget;

	// The two parts are laid out side by side, which drops a plain space at the end of the first
	// one, so a space at that end is made non-breaking and stays between the two parts.
	const start = characters.slice(0, endStart).join('').replace(/ $/, '\u00a0');

	return {
		id: id,
		type: 'container',
		tooltip: text,
		children: [
			{
				id: id + '-start',
				type: 'fixedtext',
				text: start,
			} as TextWidget,
			{
				id: id + '-end',
				type: 'fixedtext',
				text: characters.slice(endStart).join(''),
			} as TextWidget,
		],
	} as ContainerWidgetJSON;
}

// A restricted user does not see an entry whose command is blocked, and a locked user sees it
// locked.
function _isRestrictedEntry(entry: MenuDefinition): boolean {
	const map = window.L.Map.THIS;
	return !!entry.uno && map.isRestrictedUser() && map.isRestrictedItem(entry);
}

function _isLockedEntry(entry: MenuDefinition): boolean {
	const map = window.L.Map.THIS;
	return !!entry.uno && map.isLockedUser() && map.isLockedItem(entry);
}

JSDialog.CreateDropdownEntriesId = function (id: string) {
	return id + '-entries';
};

/*
	The grid that lists the entries of a dropdown, as JSON, with what the dropdown needs to know of
	it: whether the arrow keys move across it as a grid, the entry the focus starts on, and the
	entry that is shown as text above the list. A grid that has a header names it as the text that
	describes it.
*/
function _createEntriesGrid(
	id: string,
	entries: Array<ComboBoxEntry>,
	noDefaultSelection?: boolean,
): {
	grid: GridWidgetJSON;
	gridKeyboardNavigation: boolean;
	focusId?: string;
	header?: ComboBoxEntry;
} {
	const children: Array<WidgetJSON> = [];
	const grid = {
		id: JSDialog.CreateDropdownEntriesId(id),
		type: 'grid',
		allyRole: 'listbox',
		cols: 1,
		rows: entries.length,
		children: children,
	} as GridWidgetJSON;
	let gridKeyboardNavigation = false;

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
			const uno = entries[i].uno;
			const boolMatch = uno ? uno.match(/[?&]\w+:bool=(true|false)/) : null;
			if (val !== undefined && boolMatch) {
				const target = boolMatch[1] === 'true';
				const expectedState = target ? 'isLandscape' : 'isPortrait';
				entries[i].checked =
					String(val).toLowerCase() === expectedState.toLowerCase();
			} else if (val) {
				const index = parseInt(val);
				if (index === i) {
					entries[i].selected = true;
				} else {
					entries[i].selected = false;
				}
			}
		}
	}

	const header = entries.find((entry) => entry.type === 'header');
	const firstEntryIndex = entries[0]?.type === 'header' ? 1 : 0;
	const shouldSelectFirstEntry =
		!noDefaultSelection && entries.length > 0
			? !entries.some((entry) => entry.selected === true)
			: false;
	let initialSelectedId;
	let checkedFocusId;

	for (let i = 0; i < entries.length; i++) {
		const checkedValue =
			entries[i].checked === undefined
				? undefined
				: (entries[i].uno ? isChecked('.uno' + entries[i].uno) : false) ||
					(entries[i].action ? isChecked(String(entries[i].action)) : false);

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
				gridKeyboardNavigation = true;
				break;

			// dropdown is a colorpicker
			case 'colorpicker':
				entry = entries[i];
				// for color picker we have a "KeyboardGridNavigation" function defined separately to handle custom cases
				gridKeyboardNavigation = true;
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
				// A grid, or a widget that wraps a grid (the new slide layout
				// picker wraps one so it can also show the Overview button),
				// navigates with grid keys. Otherwise list navigation lets
				// arrow + Tab + arrow select more than one cell in a
				// single-choice grid.
				if (
					entry?.type === 'grid' ||
					(entry as NewSlideLayoutEntryWidgetJSON)?.gridContent?.type === 'grid'
				)
					gridKeyboardNavigation = true;
				break;

			// shown as text above the entries, outside the list
			case 'header':
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
			default: {
				if (_isRestrictedEntry(entries[i])) break;
				const isEnabled = entries[i].isEnabled;
				const locked = _isLockedEntry(entries[i]);
				entry = {
					id: id + '-entry-' + i,
					type: 'comboboxentry',
					customRenderer: entries[i].customRenderer,
					comboboxId: id,
					pos: i,
					text: entries[i].text,
					hint: entries[i].hint,
					shortcut: entries[i].shortcut,
					w2icon: entries[i].icon, // FIXME: DEPRECATED
					icon: entries[i].img,
					checked: entries[i].checked || checkedValue,
					enabled: isEnabled ? isEnabled() : entries[i].enabled,
					selected:
						i === firstEntryIndex && shouldSelectFirstEntry
							? true
							: entries[i].selected,
					hasSubMenu: !!entries[i].items,
					class: locked
						? [entries[i].class, 'locking-disabled'].filter(Boolean).join(' ')
						: entries[i].class,
				} as ComboBoxEntry;
				if ((entry as ComboBoxEntry).selected) initialSelectedId = entry.id;
				if (
					(entry as ComboBoxEntry).checked &&
					!(entry as ComboBoxEntry).selected
				)
					checkedFocusId = entry.id;
				break;
			}
		}

		if (entries[i].hidden) entry = null;

		if (entry) children.push(entry);

		if (entries[i].separatorAfter)
			children.push({
				id: id + '-separator-' + i,
				type: 'separator',
				orientation: 'horizontal',
			} as SeparatorWidgetJSON);
	}

	const focusId = initialSelectedId || checkedFocusId;
	if (focusId)
		(grid as WidgetJSON as ComboBoxEntry).initialSelectedId = focusId;

	if (header)
		grid.aria = { label: header.aria?.label, describedBy: id + '-header' };

	return { grid, gridKeyboardNavigation, focusId, header };
}

/*
	The callback of a dropdown with these entries. An entry with items opens them as a submenu
	beside it, and only one submenu of a dropdown is open at a time. Which one is open is asked of
	the open dropdowns, so a callback made again for new entries knows it as well.
*/
function _createDropdownCallback(
	id: string,
	targetEntries: Array<MenuDefinition>,
	innerCallback: JSDialogMenuCallback,
	earlyCallbackCall?: boolean,
	noDefaultSelection?: boolean,
): JSDialogCallback {
	const subMenuIdOf = (pos: number) => id + '-' + pos;
	const closeOpenSubMenus = () => {
		targetEntries.forEach((entry: MenuDefinition, pos: number) => {
			if (entry.items && JSDialog.IsDropdownOpen(subMenuIdOf(pos)))
				JSDialog.CloseDropdown(subMenuIdOf(pos));
		});
	};

	return function (
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		// The entry position comes as a number or a 'pos;...' string; other
		// payloads belong to the widget inside the dropdown and carry no
		// entry position.
		let pos = -1;
		if (typeof data === 'number') pos = data;
		else if (typeof data === 'string')
			pos = parseInt(data.substr(0, data.indexOf(';')));
		const entry = targetEntries && pos >= 0 ? targetEntries[pos] : null;
		if (entry) {
			entry.pos = pos;
		}
		const subMenuId = subMenuIdOf(pos);

		if (eventType === 'selected' || eventType === 'showsubmenu') {
			if (entry && entry.items) {
				// The submenu for this entry is already open; keep it.
				if (JSDialog.IsDropdownOpen(subMenuId)) return;

				closeOpenSubMenus();

				// open submenu
				// Use the entry's known ID to find the DOM element that will become
				// popupParent. Positional lookup via querySelectorAll('.ui-grid-cell')
				// is unreliable because the grid renderer inserts an empty placeholder
				// cell at position 0, shifting every real entry by one slot.
				const dropdown = JSDialog.GetDropdown(id);
				const entryId = id + '-entry-' + pos;
				const targetEntry = dropdown
					? dropdown.querySelector('[id="' + entryId + '"]')
					: null;

				JSDialog.OpenDropdown(
					subMenuId,
					targetEntry,
					entry.items,
					innerCallback,
					'top-end',
					true,
					earlyCallbackCall,
					noDefaultSelection,
				);

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
			} else if (eventType === 'selected' && entry && _isLockedEntry(entry)) {
				JSDialog.CloseAllDropdowns();
				window.L.Map.THIS.openUnlockPopup(entry.uno);
				return;
			} else if (eventType === 'selected' && entry && entry.uno) {
				if (earlyCallbackCall && innerCallback) {
					innerCallback(objectType, eventType, object, data, entry);
				} else {
					const uno =
						entry.uno.indexOf('.uno:') === 0 ? entry.uno : '.uno:' + entry.uno;
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
			closeOpenSubMenus();
			JSDialog.CloseDropdown(id);
			return;
		} else if (eventType === 'hideothersubmenu') {
			closeOpenSubMenus();
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
}

JSDialog.OpenDropdown = function (
	id: string,
	popupParent:
		string | (HTMLElement & { _onDropDown: (open: boolean) => void }),
	entries: Array<ComboBoxEntry>,
	innerCallback: JSDialogMenuCallback,
	popupAnchor: string,
	isSubmenu: boolean,
	earlyCallbackCall?: boolean,
	noDefaultSelection?: boolean,
) {
	const content = _createEntriesGrid(id, entries, noDefaultSelection);

	const json = {
		id: _createDropdownId(id),
		type: 'dropdown',
		isSubmenu: isSubmenu,
		jsontype: 'dialog',
		popupParent: popupParent,
		popupAnchor: popupAnchor,
		gridKeyboardNavigation: content.gridKeyboardNavigation,
		cancellable: true,
		children: [content.grid],
	} as JSDialogJSON;

	if (
		popupParent &&
		typeof popupParent !== 'string' &&
		typeof popupParent._onDropDown === 'function'
	) {
		popupParent._onDropDown(true);
	}

	if (content.focusId) json.init_focus_id = content.focusId;

	if (content.header) {
		json.children = [
			{
				id: id + '-content',
				type: 'container',
				vertical: true,
				children: [
					_createHeader(id + '-header', content.header.text || ''),
					content.grid,
				],
			} as ContainerWidgetJSON,
		];
	}

	if (!isSubmenu) {
		window.L.Map.THIS.fire('closepopups'); // close popups if a dropdown menu is opened
		window.L.Map.THIS.fire('dropdownopen', {
			id: id,
			popupParent: popupParent,
		});
	}
	window.L.Map.THIS.fire('jsdialog', {
		data: json,
		callback: _createDropdownCallback(
			id,
			entries,
			innerCallback,
			earlyCallbackCall,
			noDefaultSelection,
		),
	});
};

/*
	Shows new entries in a dropdown that is open, in their place: the dropdown stays where it is,
	a submenu of it stays open and the focus stays on the entry it was on. The entries keep the
	order they had, and the callback and the flags are the ones the dropdown was opened with.
	Nothing happens for a dropdown that is not open.
*/
JSDialog.UpdateDropdown = function (
	id: string,
	entries: Array<ComboBoxEntry>,
	innerCallback: JSDialogMenuCallback,
	earlyCallbackCall?: boolean,
	noDefaultSelection?: boolean,
) {
	if (!JSDialog.IsDropdownOpen(id)) return;

	// The focus is where the user left it, so no entry is selected on its own.
	const content = _createEntriesGrid(id, entries, true);

	window.L.Map.THIS.fire('jsdialogupdate', {
		data: {
			id: _createDropdownId(id),
			jsontype: 'dialog',
			action: 'update',
			control: content.grid,
		},
		callback: _createDropdownCallback(
			id,
			entries,
			innerCallback,
			earlyCallbackCall,
			noDefaultSelection,
		),
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

// True from the moment the dropdown is opened until it is closed, by any path.
JSDialog.IsDropdownOpen = function (id: string): boolean {
	const jsdialog = window.L.Map.THIS.jsdialog;
	return !!jsdialog && !!jsdialog.dialogs[_createDropdownId(id)];
};

JSDialog.CloseAllDropdowns = function () {
	window.L.Map.THIS.jsdialog.closeAllDropdowns();
	if (window.L.Map.THIS.contextToolbar)
		window.L.Map.THIS.contextToolbar.hideContextToolbar();
};

JSDialog.GetDropdown = function (id: string) {
	// remember it can get some random numbers due to JSDialog.MakeIdUnique
	// TODO: use some register for it
	const selector = '[id^="' + id + '"].modalpopup';
	let dropdown = document.body.querySelector(selector);
	if (!dropdown) {
		JSDialog.ForEachPopoutDocument((childDocument: Document) => {
			if (!dropdown) dropdown = childDocument.body.querySelector(selector);
		});
	}
	return dropdown;
};
