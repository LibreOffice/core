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
	This class is used for managing the accessibility keys of compact view control.
*/
interface UIAccessibilityItem {
	id?: string;
	children?: UIAccessibilityItem[];
	accessibility?: {
		combination?: string;
		focusBack?: boolean;
	};
}

interface AccessibilityItem {
	id: string;
	combination: string;
	focusBack: boolean;
}

class CompactViewAccessibility {
	private initialized: boolean = false;
	private accessKeysInitialized: boolean = false;
	private combination: string | null = null;
	private infoBoxes: HTMLElement[] = [];
	private contentList: AccessibilityItem[] = [];
	private fixedContentList: AccessibilityItem[] = [];
	private mayShowInfoBoxes: boolean = false;
	private accessibilityInputElement: HTMLInputElement | null = null;

	public initialize(): void {
		if (!window.mode.isDesktop()) return;

		if (!this.initialized) {
			this.initAccessibilityInputElement();
			this.initialized = true;
		}
	}

	private initAccessibilityInputElement(): void {
		this.accessibilityInputElement = document.createElement('input');
		// type='submit' prevents screen readers from announcing "editable blank"
		this.accessibilityInputElement.type = 'submit';
		// role='menubar' prevents screen readers from announcing "Submit button"
		this.accessibilityInputElement.setAttribute('role', 'menubar');
		this.accessibilityInputElement.style.width = '0';
		this.accessibilityInputElement.style.height = '0';
		this.accessibilityInputElement.id = 'compactViewAccessibilityInput';
		this.accessibilityInputElement.onblur = this.hideInfoBoxes.bind(this);
		this.accessibilityInputElement.onkeydown = this.onInputKeyDown.bind(this);
		this.accessibilityInputElement.onkeyup = this.onInputKeyUp.bind(this);
		this.accessibilityInputElement.autocomplete = 'off';

		const container = document.createElement('div');
		container.style.width = '0';
		container.style.height = '0';
		container.style.overflow = 'hidden';
		container.appendChild(this.accessibilityInputElement);

		document.body.insertBefore(container, document.body.firstChild);
	}

	private collectMenuItems(): void {
		const menubar = app.map.menubar;
		if (!menubar) return;

		const menuItems = menubar.options[app.map.getDocType()];
		if (!Array.isArray(menuItems)) return;

		for (const item of menuItems) {
			if (!item.accessibility?.combination || !item.id) continue;

			const anchor = document.getElementById(
				'menu-' + item.id,
			) as HTMLElement | null;
			if (!anchor || anchor.offsetParent === null) continue;

			this.contentList.push({
				id: 'menu-' + item.id,
				combination: item.accessibility.combination,
				focusBack: false,
			});
		}
	}

	private addInfoBox(
		anchorElement: HTMLElement,
		combination: string,
	): HTMLElement | null {
		if (!anchorElement || anchorElement.offsetParent === null) return null;

		const infoBox = document.createElement('div');
		infoBox.classList.add('accessibility-info-box');
		infoBox.textContent = combination;

		const rect = anchorElement.getBoundingClientRect();
		infoBox.style.top = rect.bottom - 5 + 'px';
		infoBox.style.left = rect.left + 'px';

		document.body.appendChild(infoBox);
		return infoBox;
	}

	private removeAllInfoBoxes(): void {
		const boxes = document.getElementsByClassName('accessibility-info-box');
		for (let i = boxes.length - 1; i >= 0; i--) {
			document.body.removeChild(boxes[i]);
		}
		this.infoBoxes = [];
	}

	private createContentList(): void {
		this.contentList = [];

		const topToolbar = app.map.topToolbar;
		if (topToolbar) {
			const items = topToolbar.getToolItems() as UIAccessibilityItem[];
			if (items) this.collectItems(items);
		}

		const formulabar = app.map.formulabar;
		if (formulabar) {
			const formulaItems =
				formulabar.getFormulaItems() as UIAccessibilityItem[];
			if (formulaItems) this.collectItems(formulaItems);
		}

		const addressInput = app.map.addressInputField;
		if (addressInput) {
			const addressItems =
				addressInput.getAddressItems() as UIAccessibilityItem[];
			if (addressItems) this.collectItems(addressItems);
		}

		this.addFixedElements();
	}

	private findElement(id: string): HTMLElement | null {
		if (id.startsWith('menu-')) return document.getElementById(id);

		const modelId = document.querySelector<HTMLElement>(`[modelid="${id}"]`);
		if (modelId) return modelId;

		return document.getElementById(id);
	}

	private cleanId(id: string): string {
		const separatorPos = id.indexOf(':');
		if (separatorPos === -1) return id;

		return id.substring(0, separatorPos);
	}

	private collectItems(items: UIAccessibilityItem[]): void {
		for (const item of items) {
			if (item.accessibility?.combination && item.id) {
				this.contentList.push({
					id: this.cleanId(item.id),
					combination: item.accessibility.combination,
					focusBack: item.accessibility.focusBack !== false,
				});
			}
			if (item.children) this.collectItems(item.children);
		}
	}

	private addFixedElements(): void {
		this.fixedContentList = [];
		const containerList = ['viewModeDropdownButton', 'userListHeader'];
		for (const containerId of containerList) {
			const container = document.getElementById(containerId);
			if (!container) continue;

			const buttons = container.querySelectorAll<HTMLElement>('button');
			for (const button of Array.from(buttons)) {
				if (button.accessKey && button.id) {
					this.fixedContentList.push({
						id: button.id,
						combination: button.accessKey,
						focusBack: true,
					});
				}
			}
		}
	}

	private addInfoBoxesForItems(items: AccessibilityItem[]): void {
		for (const item of items) {
			const anchor = this.findElement(item.id);
			if (!anchor) continue;

			anchor.accessKey = item.combination;
			const infoBox = this.addInfoBox(anchor, item.combination);
			if (infoBox) this.infoBoxes.push(infoBox);
		}
	}

	private showInfoBoxes(): void {
		this.removeAllInfoBoxes();

		if (!this.accessKeysInitialized) {
			this.createContentList();
			this.collectMenuItems();
			this.accessKeysInitialized = true;
		}

		this.addInfoBoxesForItems(this.contentList);
		this.addInfoBoxesForItems(this.fixedContentList);

		document.body.classList.add('activate-info-boxes');

		// Focus the hidden input to capture subsequent key events
		this.accessibilityInputElement?.focus();
	}

	private hideInfoBoxes(): void {
		this.combination = null;
		this.removeAllInfoBoxes();
		document.body.classList.remove('activate-info-boxes');

		// Clear the input to reset the state for the next Alt press
		if (this.accessibilityInputElement) {
			this.accessibilityInputElement.value = '';
		}
	}

	private focusToMap(): void {
		this.hideInfoBoxes();
		app.map.focus();
	}

	private filterInfoBoxes(): void {
		const boxes = document.getElementsByClassName('accessibility-info-box');
		for (let i = 0; i < boxes.length; i++) {
			const box = boxes[i] as HTMLElement;
			if (this.combination && !box.textContent?.startsWith(this.combination)) {
				box.classList.add('filtered_out');
			} else {
				box.classList.remove('filtered_out');
			}
		}
	}

	private isAllFilteredOut(): boolean {
		const visible = document.querySelectorAll(
			'.accessibility-info-box:not(.filtered_out)',
		);
		return visible.length === 0;
	}

	private findMatchingItem(): AccessibilityItem | null {
		if (!this.combination) return null;

		for (const item of this.contentList) {
			if (item.combination === this.combination) return item;
		}

		for (const item of this.fixedContentList) {
			if (item.combination === this.combination) return item;
		}
		return null;
	}

	private findFirstSubmenuItem(element: HTMLElement): HTMLElement | null {
		const submenu = element.querySelector<HTMLElement>(':scope > ul');
		if (!submenu) return null;

		const firstSubmenuItem = submenu.querySelector<HTMLElement>('li > a');
		if (!firstSubmenuItem) return null;

		return firstSubmenuItem;
	}

	private activateItem(item: AccessibilityItem): void {
		const element = this.findElement(item.id);
		if (!element) return;

		// Menu elem
		if (element.closest('#main-menu')) {
			const menuItem = element.querySelector<HTMLElement>('a');
			if (menuItem) {
				menuItem.click();
				const firstSubmenuItem = this.findFirstSubmenuItem(element);
				firstSubmenuItem?.focus();
			}
			this.hideInfoBoxes();
			return;
		}

		// Combobox
		const input = element.querySelector<HTMLElement>('input');
		if (input) {
			this.hideInfoBoxes();
			input.focus();
			return;
		}

		// Menu button or regular button
		const arrow = element.querySelector<HTMLElement>('.arrowbackground');
		const button = element.querySelector<HTMLElement>('button.unobutton');
		const target = arrow ?? button ?? element;
		target.click();

		if (item.focusBack) this.focusToMap();
	}

	private onInputKeyDown(event: KeyboardEvent): void {
		// Hide boxes on Ctrl to allow normal keyboard shortcuts
		if (event.ctrlKey) {
			this.hideInfoBoxes();
		}

		if (event.key === 'Tab') {
			event.preventDefault();
		}
	}

	private onInputKeyUp(event: KeyboardEvent): void {
		const key = event.key.toUpperCase();
		event.preventDefault();
		event.stopPropagation();

		if (key === 'ESCAPE' || key === 'ALT') {
			if (this.combination === null) {
				this.focusToMap();
			} else {
				this.hideInfoBoxes();
			}
			return;
		}

		// Ignore shift alone
		if (key === 'SHIFT') return;

		// Focus first menu item
		if (key === 'TAB') {
			const firstMenuItem = document.querySelector<HTMLElement>(
				'#main-menu > li > a[tabindex="0"]',
			);
			if (firstMenuItem) {
				this.hideInfoBoxes();
				firstMenuItem.focus();
			}
			return;
		}

		// Append key to combination
		if (this.combination === null) {
			this.combination = key;
		} else {
			this.combination += key;
		}

		// Check for an exact match first
		const matched = this.findMatchingItem();
		if (matched) {
			this.activateItem(matched);
			return;
		}

		// Filter the visible boxes to show only those still matching
		this.filterInfoBoxes();

		// We checked the pressed key against available combinations
		// If there is no match, focus back to map
		if (this.isAllFilteredOut()) {
			this.focusToMap();
		}
	}

	public onDocumentKeyDown(event: KeyboardEvent): void {
		if (document.body.dataset.userinterfacemode !== 'classic') return;

		this.mayShowInfoBoxes = event.key.toUpperCase() === 'ALT';
	}

	public onDocumentKeyUp(event: KeyboardEvent): void {
		if (document.body.dataset.userinterfacemode !== 'classic') return;

		if (app.map && app.map.jsdialog && app.map.jsdialog.hasDialogOpened())
			return;

		if (event.key.toUpperCase() === 'ALT') {
			if (!this.mayShowInfoBoxes) return;

			if (this.infoBoxes.length > 0) {
				this.focusToMap();
			} else {
				this.showInfoBoxes();
			}
		}
	}
}

app.UI.compactViewAccessibility = new CompactViewAccessibility();
