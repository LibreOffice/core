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

/*
 * Component.Toolbar - base class for toolbars
 */

declare var JSDialog: any;

class Toolbar extends JSDialogComponent {
	protected docType: string;
	protected callback: JSDialogCallback;
	protected toolbarElementId: string;
	protected parentContainer: Element; // FIXME: can we drop as we have container in base?
	protected customItems: Array<ToolItemWidgetJSON>;

	constructor(map: MapInterface, name: string, toolbarElementId: string) {
		super(map, name, 'toolbar');

		this.docType = map.getDocType();
		this.customItems = [];
		this.toolbarElementId = toolbarElementId;

		this.createBuilder();
		this.reset();
		this.create();
		this.updateVisibilityForToolbar('');
	}

	getToolItems(): Array<ToolItemWidgetJSON> {
		return [];
	}

	protected createBuilder() {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			cssClass: 'jsdialog',
			noLabelsForUnoButtons: true,
			callback: this.callback ? this.callback.bind(this) : undefined,
			suffix: 'toolbar',
		});
	}

	protected setupContainer(parentContainer?: HTMLElement /* ignored */) {
		this.container = this.parentContainer = window.L.DomUtil.get(
			this.toolbarElementId,
		);

		// In case it contains garbage
		if (this.parentContainer) this.parentContainer.replaceChildren();

		window.L.DomUtil.addClass(this.parentContainer, 'ui-toolbar');
	}

	reset() {
		this.setupContainer(undefined);
	}

	create() {
		this.reset();

		const items = this.getToolItems();
		const json = {
			id: this.toolbarElementId,
			dialogid: this.toolbarElementId,
			jsontype: 'toolbar',
			type: 'toolbox',
			children: items,
		} as JSDialogJSON;

		this.model.fullUpdate(json);
		this.builder.build(
			this.parentContainer,
			this.model.getSnapshot().children,
			undefined,
		);

		JSDialog.MakeScrollable(
			this.parentContainer,
			this.parentContainer.querySelector('div'),
		);
		JSDialog.RefreshScrollables();

		if (this.map.isRestrictedUser()) {
			for (var i = 0; i < items.length; i++) {
				var it = items[i];
				var item = $('#' + it.id)[0];
				this.map.hideRestrictedItems(it, item, item);
			}
		}

		if (this.map.isLockedUser()) {
			for (var i = 0; i < items.length; i++) {
				var it = items[i];
				var item = $('#' + it.id)[0];
				this.map.disableLockedItem(it, item, item);
			}
		}
	}

	hasItem(id: string) {
		return (
			this.getToolItems().filter((item) => {
				return item.id === id;
			}).length > 0
		);
	}

	insertItem(beforeId: string, items: Array<ToolItemWidgetJSON>) {
		this.customItems.push({
			id: 'custom-before-' + beforeId,
			type: 'toolitem',
			beforeId: beforeId,
			items: items,
		});
		this.create();
	}

	getItemElement(id: string) {
		let item = this.parentContainer?.querySelector('[modelId="' + id + '"]');
		if (!item) item = this.parentContainer?.querySelector('[id="' + id + '"]');
		return item;
	}

	isItemHidden(id: string): boolean {
		const item = this.getItemElement(id);
		if (!item) return true;
		return item.classList.contains('hidden');
	}

	showItem(command: string, show: boolean): boolean {
		if (!command) return false;

		if (this.isItemHidden(command) === !show) return true;

		this.builder.executeAction(this.parentContainer, {
			jsontype: 'toolbar',
			control_id: command,
			control: { id: command },
			action_type: show ? 'show' : 'hide',
		} as ActionData);

		app.layoutingService.appendLayoutingTask(() => {
			JSDialog.RefreshScrollables();
		});
	}

	isItemUserHidden(id: string): boolean {
		if (!id) return false;
		const entry = this.model.getById(id);
		return !!(entry && entry.userHidden);
	}

	userHideItem(id: string, hide: boolean): void {
		if (!id) return;

		this.model.widgetUpdate({ id: id, userHidden: hide } as WidgetJSON);

		const el = this.getItemElement(id);
		if (!el) return;
		if (hide) window.L.DomUtil.addClass(el, 'user-hidden');
		else window.L.DomUtil.removeClass(el, 'user-hidden');
	}

	isItemDisabled(id: string): boolean {
		const item = this.getItemElement(id);
		if (!item) return true;
		return (
			item.hasAttribute('disabled') && item.getAttribute('disabled') !== 'false'
		);
	}

	enableItem(command: string, enable: boolean) {
		if (!command) return;

		if (this.isItemDisabled(command) === !enable) return;

		this.builder.executeAction(this.parentContainer, {
			jsontype: 'toolbar',
			control_id: command,
			action_type: enable ? 'enable' : 'disable',
		});
	}

	selectItem(command: string, select: boolean) {
		if (!command) return;

		this.builder.executeAction(this.parentContainer, {
			jsontype: 'toolbar',
			control_id: command,
			action_type: select ? 'select' : 'unselect',
		});
	}

	updateItem(data: ToolItemWidgetJSON) {
		this.builder.updateWidget(this.parentContainer, data);
		this.updateVisibilityForToolbar('');
		app.layoutingService.appendLayoutingTask(() => {
			JSDialog.RefreshScrollables();
		});
	}

	updateVisibilityForToolbar(context: string) {
		const toShow: Array<string> = [];
		const toHide: Array<string> = [];

		const items = this.getToolItems();
		items.forEach((item) => {
			if (
				(window as any).ThisIsTheiOSApp &&
				window.mode.isTablet() &&
				item.iosapptablet === false
			) {
				toHide.push(item.id);
			} else if (
				((window.mode.isSmallScreenDevice() && item.mobile === false) ||
					(window.mode.isTablet() && item.tablet === false) ||
					(window.mode.isDesktop() && item.desktop === false) ||
					(!(window as any).ThisIsAMobileApp &&
						item.mobilebrowser === false)) &&
				!item.hidden
			) {
				toHide.push(item.id);
			} else if (
				((window.mode.isSmallScreenDevice() && item.mobile === true) ||
					(window.mode.isTablet() && item.tablet === true) ||
					(window.mode.isDesktop() && item.desktop === true) ||
					((window as any).ThisIsAMobileApp && item.mobilebrowser === true)) &&
				(item.hidden || item.visible === false)
			) {
				toShow.push(item.id);
			}

			if (context && item.context) {
				if (item.context.indexOf(context) >= 0) toShow.push(item.id);
				else toHide.push(item.id);
			} else if (!context && item.context) {
				if (item.context.indexOf('default') >= 0) toShow.push(item.id);
				else toHide.push(item.id);
			}
		});

		window.app.console.log('explicitly hiding: ' + toHide);
		window.app.console.log('explicitly showing: ' + toShow);

		toHide.forEach((item) => {
			this.showItem(item, false);
		});
		toShow.forEach((item) => {
			this.showItem(item, true);
		});
	}
}

JSDialog.Toolbar = Toolbar;
