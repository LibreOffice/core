/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

interface PositionInterface {
	x: number;
	y: number;
}

interface MenuEntry {
	enabled: string;
	type: string;
	text?: string;
	menu?: MenuEntry[];
	command: string;
	checked?: string;
	checktype?: string;
}

interface CtxtCommandEntry {
	name: string;
	isHtmlName: boolean;
	icon?: string;
	command?: string;
	items?: Record<string, CtxtValueType>;
}

type CtxtValueType = CtxtCommandEntry | string;

interface MenuCfg {
	entries: Array<ComboBoxEntry>;
	innerCallback: JSDialogMenuCallback;
}

class ContextMenuControl extends JSControl {
	private _prevMousePos: PositionInterface | null = null;
	private _currMousePos: PositionInterface | null = null;
	private _menuID: string = 'jsd-context-menu';
	private _menuPosID: string = 'canvas-context-menupos';
	private _menuPosEl: HTMLElement | null = null;
	private _autoFillContextMenu: boolean = false;
	private _spellingContextMenu: boolean = false;
	public hasContextMenu: boolean = false;

	constructor(options?: Record<string, any>) {
		super(options);
		this.options.allowlist = JSDialog.MenuCommands.allowlist;
		this.options.mobileDenylist = JSDialog.MenuCommands.mobileDenylist;
	}

	public onAdd(map: MapInterface): HTMLElement | null {
		this._prevMousePos = null;
		this._autoFillContextMenu = false;

		map._contextMenu = this;
		map.on('locontextmenu', this._onContextMenu, this);
		map.on('mousedown', this._onMouseDown, this);
		map.on('mouseup', this._onMouseUp, this);
		map.on('keydown', this._onKeyDown, this);
		map.on('closepopups', this._onClosePopup, this);
		return null;
	}

	private _onClosePopup() {
		if (!this.hasContextMenu) {
			return;
		}

		if (this._autoFillContextMenu) {
			this._autoFillContextMenu = false;
			app.map._docLayer._resetReferencesMarks();
		}
		JSDialog.CloseDropdown(this._menuID);
		this.hasContextMenu = false;

		// Give the stolen focus back to map
		app.map.focus();
	}

	private _onMouseDown(): void {
		if (app.activeDocument && app.activeDocument.mouseControl)
			this._prevMousePos =
				app.activeDocument.mouseControl.getMousePagePosition();

		this._onClosePopup();
	}

	private _onMouseUp(): void {
		if (app.activeDocument && app.activeDocument.mouseControl)
			this._currMousePos =
				app.activeDocument.mouseControl.getMousePagePosition();
	}

	private _onKeyDown(e: Event) {
		if ((e as any).originalEvent.keyCode === 27 /* ESC */) {
			this._onClosePopup();
		}
	}

	private _onContextMenu(obj: any): void {
		Util.ensureValue(this._map);
		const map = this._map;
		if (!map.isEditMode()) {
			return;
		}

		this._amendContextMenuData(obj);

		const contextMenu = this._createContextMenuStructure(obj);

		if (Object.keys(contextMenu).length == 0) {
			// We can sometimes end up filtering out everything in the menu ... in this case, there's nothing to display
			return;
		}

		this._spellingContextMenu = false;
		this._autoFillContextMenu = false;
		for (const menuItem in contextMenu) {
			if (menuItem.indexOf('.uno:SpellCheckIgnore') !== -1) {
				this._spellingContextMenu = true;
				break;
			} else if (menuItem.indexOf('.uno:AutoFill') !== -1) {
				// we should close the autofill preview popup before open autofill context menu
				map.fire('closeautofillpreviewpopup');
				this._autoFillContextMenu = true;
				break;
			}
		}

		if (window.mode.isSmallScreenDevice()) {
			window.contextMenuWizard = true;
			const menuData =
				window.L.Control.JSDialogBuilder.getMenuStructureForMobileWizard(
					contextMenu,
					true,
					'',
				);
			map.fire('mobilewizard', { data: menuData });
		} else {
			this._addMenu(contextMenu);
			$('#' + this._menuPosID).focus();
			this.hasContextMenu = true;
		}
	}

	private _addMenu(contextMenu: Record<string, CtxtValueType>): void {
		Util.ensureValue(this._map);
		const map = this._map;
		Util.ensureValue(app.activeDocument);
		Util.ensureValue(app.activeDocument.mouseControl);
		const position = app.activeDocument.mouseControl.getMouseCanvasPosition();
		Util.ensureValue(position);
		const container = document.getElementById('canvas-container');
		Util.ensureValue(container);
		if (!this._menuPosEl) {
			this._menuPosEl = document.createElement('div');
			this._menuPosEl.id = this._menuPosID;
			container.append(this._menuPosEl);
		}
		const menuPosEl = this._menuPosEl;
		menuPosEl.style.position = 'absolute';
		menuPosEl.style.zIndex = '1500';
		menuPosEl.style.left = position.x + 'px';
		menuPosEl.style.top = position.y + 'px';
		const entries = this._getMenuEntries(contextMenu);
		const callback = (
			objectType: string,
			eventType: string,
			object: any,
			data: any,
			entry: JSBuilder | MenuDefinition,
		): boolean => {
			const key = (entry as MenuDefinition).id;
			if (key === '.uno:InsertAnnotation') {
				app.map.insertComment();
				return false;
			}

			if (key === 'saveimagetowopi') {
				var ext =
					GraphicSelection.extraInfo &&
					GraphicSelection.extraInfo.graphicExtension
						? GraphicSelection.extraInfo.graphicExtension
						: 'png';
				map._saveImageToWopi = true;
				map.openSaveAs(ext);
				return false;
			}

			const unoentry = entry as MenuDefinition;
			Util.ensureValue(unoentry.uno);
			const uno =
				unoentry.uno.indexOf('.uno:') === 0
					? unoentry.uno
					: '.uno:' + unoentry.uno;
			if (
				app.map._clip === undefined ||
				!app.map._clip.filterExecCopyPaste(uno)
			) {
				app.map.sendUnoCommand(uno);
				// For spelling context menu we need to remove selection
				if (this._spellingContextMenu) app.map._docLayer._clearSelections();
				// Give the stolen focus back to map
				app.map.focus();
			}

			this._map?.focus();
			return false;
		};

		JSDialog.OpenDropdown(
			this._menuID,
			menuPosEl,
			entries,
			callback,
			'',
			false,
			true /* earlyCallbackCall? */,
		);
	}

	private _getMenuEntries(
		contextMenu: Record<string, CtxtValueType>,
	): MenuDefinition[] {
		const entries: MenuDefinition[] = [];
		for (const [command, value] of Object.entries(contextMenu)) {
			if (typeof value === 'string') {
				entries.push({
					id: command,
					type: 'separator',
				});
				continue;
			}

			if (!value.items) {
				entries.push({
					id: command,
					uno: command,
					type: 'comboboxentry',
					text: value.name,
					img: command,
				});
				continue;
			}

			const items = this._getMenuEntries(value.items);
			entries.push({
				id: command,
				uno: command,
				type: 'comboboxentry',
				text: value.name,
				items: items,
				img: command,
			});
		}

		return entries;
	}

	private _amendContextMenuData(obj: any): void {
		// Add a 'delete' entry  for graphic selection on desktop and mobile device (in browser or app).
		if (GraphicSelection.hasActiveSelection()) {
			const menu: MenuEntry[] = obj.menu;
			let insertIndex = -1;
			menu.forEach(function (item: MenuEntry, index: number) {
				if (item.command === '.uno:Paste') {
					insertIndex = index + 1;
				}
			});

			if (insertIndex != -1) {
				menu.splice(insertIndex, 0, {
					text: _('Delete'),
					type: 'command',
					command: '.uno:Delete',
					enabled: 'true',
				});
			}

			Util.ensureValue(this._map);
			// Add "Save Image to Server" after SaveGraphic when WOPI supports save-as
			if (!this._map['wopi'].UserCanNotWriteRelative) {
				let saveGraphicIndex = -1;
				menu.forEach((item: MenuEntry, index: number) => {
					if (item.command === '.uno:SaveGraphic') {
						saveGraphicIndex = index + 1;
					}
				});

				if (saveGraphicIndex != -1) {
					menu.splice(saveGraphicIndex, 0, {
						text: _('Save Image to Server'),
						type: 'command',
						command: 'saveimagetowopi',
						enabled: 'true',
					});
				}
			}
		}
	}

	private _createContextMenuStructure(obj: any): Record<string, CtxtValueType> {
		Util.ensureValue(this._map);
		const docType: string = this._map.getDocType();
		const contextMenu: Record<string, any> = {};
		let sepIdx = 1;
		let itemName: string;
		let subMenuIdx = 1;
		let isLastItemText = false;
		const menu: MenuEntry[] = obj.menu;
		for (const idx in menu) {
			const item = menu[idx];
			if (item.enabled === 'false') {
				continue;
			}

			// If the command was hidden with the Hide_Command postmessage...
			if (this._map.uiManager.hiddenCommands[item.command]) {
				continue;
			}

			// reduce Paste Special submenu
			if (
				item.type === 'menu' &&
				item.text &&
				item.text.replace('~', '') === 'Paste Special' &&
				item.menu &&
				item.menu.length
			) {
				item.text = _('Paste Special');
				item.command = '.uno:PasteSpecial';
				item.type = item.menu[0].type;
				item.menu = undefined;
			}

			if (
				item.type === 'command' &&
				item.text &&
				item.text.replace('~', '') === 'Copy Cells'
			) {
				item.text = _('Copy Cells');
				item.command = '.uno:AutoFill?Copy:bool=true';
			}

			if (
				item.type === 'command' &&
				item.text &&
				item.text.replace('~', '') === 'Fill Series'
			) {
				item.text = _('Fill Series');
				item.command = '.uno:AutoFill?Copy:bool=false';
			}

			if (item.type === 'separator') {
				if (isLastItemText) {
					contextMenu['sep' + sepIdx++] = '';
				}
				isLastItemText = false;
			} else if (item.type === 'command') {
				// Custom commands (not .uno:) injected by _amendContextMenuData
				if (item.command && !item.command.startsWith('.uno:')) {
					Util.ensureValue(item.text);
					itemName = item.text;
					contextMenu[item.command] = {
						name: _(itemName),
						isHtmlName: false,
					};
					isLastItemText = true;
					continue;
				}

				// Only show allowlisted items
				// Command name (excluding '.uno:') starts from index = 5
				let commandName = item.command.substring(5);

				// Command might have paramateres (e.g. .uno:SpellCheckIgnore?Type:string=Grammar)
				let hasParam = false;
				if (commandName.indexOf('?') !== -1) {
					commandName = commandName.substring(0, commandName.indexOf('?'));
					hasParam = true;
				}

				// We use a special character dialog in spelling context menu with a parameter
				if (commandName === 'FontDialog' && !hasParam) continue;

				Util.ensureValue(this.options.allowlist);
				Util.ensureValue(this.options.mobileDenylist);
				if (
					commandName !== 'None' &&
					this.options.allowlist.general.indexOf(commandName) === -1 &&
					(this._map['wopi'].HideChangeTrackingControls ||
						this.options.allowlist.tracking.indexOf(commandName) === -1) &&
					!(
						docType === 'text' &&
						this.options.allowlist.text.indexOf(commandName) !== -1
					) &&
					!(
						docType === 'spreadsheet' &&
						this.options.allowlist.spreadsheet.indexOf(commandName) !== -1
					) &&
					!(
						docType === 'presentation' &&
						this.options.allowlist.presentation.indexOf(commandName) !== -1
					) &&
					!(
						docType === 'drawing' &&
						this.options.allowlist.drawing.indexOf(commandName) !== -1
					)
				) {
					continue;
				}

				if (
					window.mode.isSmallScreenDevice() &&
					this.options.mobileDenylist.indexOf(commandName) !== -1
				)
					continue;

				if (commandName == 'None' && !item.text) continue;

				if (
					hasParam ||
					commandName === 'None' ||
					commandName === 'FontDialogForParagraph' ||
					commandName === 'Delete' ||
					commandName == 'PasteSpecial' ||
					commandName === 'UpdateCurIndex' ||
					commandName === 'RemoveTableOf' ||
					commandName === 'EditCurIndex'
				) {
					Util.ensureValue(item.text);
					// These commands have a custom item.text, don't overwrite
					// that with a label based on 'item.command'.
					itemName = window.removeAccessKey(item.text);
					itemName = itemName.replace(' ', '\u00a0');
				} else {
					// Get the translated text associated with the command
					itemName = _UNO(item.command, docType, true);
				}

				contextMenu[item.command] = {
					name: _(itemName),
					isHtmlName: true,
				};

				if (item.checktype === 'checkmark') {
					if (item.checked === 'true') {
						contextMenu[item.command]['icon'] = 'lo-checkmark';
					}
				} else if (item.checktype === 'radio') {
					if (item.checked === 'true') {
						contextMenu[item.command]['icon'] = 'radio';
					}
				}

				isLastItemText = true;
			} else if (item.type === 'menu') {
				Util.ensureValue(item.text);
				itemName = item.text;
				const submenu = this._createContextMenuStructure(item);
				// ignore submenus with all items disabled
				if (Object.keys(submenu).length === 0) {
					continue;
				}

				contextMenu['submenu' + subMenuIdx++] = {
					name: _(itemName)
						.replace(/\(~[A-Za-z]\)/, '')
						.replace('~', ''),
					command: item.command,
					items: submenu,
				};
				isLastItemText = true;
			}
		}

		// Remove separator, if present, at the end
		const lastItem =
			Object.keys(contextMenu)[Object.keys(contextMenu).length - 1];
		if (lastItem !== undefined && lastItem.startsWith('sep')) {
			delete contextMenu[lastItem];
		}

		return contextMenu;
	}

	// Prevents right mouse button's mouseup event from triggering menu item accidentally.
	public stopRightMouseUpEvent(): void {
		const menuItems = document.getElementsByClassName('ui-combobox-entry');

		for (let i = 0; i < menuItems.length; i++) {
			menuItems[i].addEventListener('mouseup', function (eo: Event) {
				const e = eo as MouseEvent;
				if (e.button == 2)
					// Is a right mouse button event?
					e.stopPropagation();
			});
		}
	}
}
