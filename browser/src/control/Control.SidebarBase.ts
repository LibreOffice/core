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
 * Control.SidebarBase
 */

/* global app */
declare var JSDialog: any;

enum SidebarType {
	Sidebar = 'sidebar', // core
	Navigator = 'navigator', // core
	QuickFind = 'quickfind', // core
	Notebookbar = 'notebookbar', // online side panel, which sens messages to NB in core
	Cleanup = 'cleanup', // online side panel built and driven in the browser
}

/// What a dock now holds: the element the dock hangs on, and the JSON type of the component
/// whose content fills it, or null once the dock is closed.
interface DockContentEvent {
	wrapper: HTMLElement;
	jsontype: string | null;
}

abstract class SidebarBase extends JSDialogComponent {
	type: SidebarType;
	resizeTaskId: TaskId | null = null;

	documentContainer: HTMLDivElement;
	wrapper: HTMLElement;

	constructor(map: MapInterface, type: SidebarType) {
		super(map, type, type);
		this.type = type;
		this.onAdd(map);
	}

	protected createBuilder() {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			windowId: WindowId.Sidebar,
			cssClass: `jsdialog sidebar`, // use sidebar css for now, maybe have separate css for navigator later
			useScrollAnimation: false, // icon views cause jump on sidebar open
			suffix: 'sidebar',
			callback: this.callback.bind(this),
		} as JSBuilderOptions);
	}

	protected setupContainer(parentContainer?: HTMLElement) {
		if (!this.container) {
			this.container = window.L.DomUtil.createWithId(
				'div',
				`${this.type}-container`,
				$(`#${this.type}-panel`).get(0),
			);
		}
		this.wrapper = document.getElementById(`${this.type}-dock-wrapper`);
		this.documentContainer = document.querySelector('#document-container');
	}

	onAdd(map: MapInterface) {
		this.map = map;
		this.createBuilder();
		this.setupContainer(undefined);

		app.events.on('resize', this.onResize.bind(this));

		this.registerMessageHandlers();
	}

	onRemove() {
		this.unregisterMessageHandlers();
	}

	/// this is used to determine if we need to send uno command - only for core decks
	isVisible(): boolean {
		const node = $(this.wrapper);
		return node.hasClass('visible') && node.hasClass('coreBased');
	}

	/// checks if this sidebar is core-based (uses WindowId.Sidebar)
	protected isCoreBased(): boolean {
		return this.builder && this.builder.windowId === WindowId.Sidebar;
	}

	/// shared implementation for showing/hiding sidebar
	protected showSidebarImpl(show: boolean) {
		const wrapper = $(this.wrapper);

		if (show) {
			wrapper.addClass('visible');
			if (this.isCoreBased()) {
				wrapper.addClass('coreBased');
			}
		} else {
			wrapper.removeClass('visible');
			wrapper.removeClass('coreBased');

			if (!this.map.editorHasFocus()) {
				this.map.fire('editorgotfocus');
				this.map.focus();
			}
		}

		const upperCaseType = this.type[0].toUpperCase() + this.type.slice(1);
		this.map.uiManager.setDocTypePref('Show' + upperCaseType, show);

		// Several components share one dock, and only the one that opened or closed it knows
		// that the content changed hands. The word goes out from here, the single place a dock
		// is shown and hidden, so a component that shares a dock is told rather than asking.
		this.map.fire('dockcontent', {
			wrapper: this.wrapper,
			jsontype: show ? this.allowedJsonType : null,
		} as DockContentEvent);
	}

	closeSidebar() {
		this.showSidebarImpl(false);
	}

	showSidebar() {
		this.showSidebarImpl(true);
	}

	protected onJSUpdate(e: { data: UpdateData }) {
		// reduce unwanted warnings in console
		if (e?.data?.control.id === 'addonimage') {
			window.app.console.log(
				'Ignored update for control: ' + e.data.control.id,
			);
			return false;
		}

		return super.onJSUpdate(e);
	}

	protected onJSAction(e: { data: ActionData }) {
		const innerData = e?.data?.data;
		const controlId = innerData?.control_id;

		if (controlId) {
			// Panels share the same name for main containers, do not execute actions for them
			// if panel has to be shown or hidden, full update will appear
			if (
				controlId.indexOf('contents') === 0 ||
				controlId.indexOf('titlebar') === 0 ||
				controlId.indexOf('expander') === 0 ||
				controlId.indexOf('addonimage') === 0
			) {
				window.app.console.log(
					'Ignored action: ' +
						innerData.action_type +
						' for control: ' +
						controlId,
				);
				return false;
			}
		}

		return super.onJSAction(e);
	}

	/// The presentation sidebar/panel toolbar buttons are mutually exclusive, so
	/// only the button of the active deck stays highlighted. Core reports the
	/// state of its own decks, but the decks the browser builds itself are opened
	/// from the notebookbar and core is never told they took over the sidebar.
	/// Their highlight, and clearing the core deck button they replace, is
	/// driven here.
	updatePresentationDeckHighlight(currentDeck: string) {
		if (this.map.getDocType() !== 'presentation') return;

		const panelCommandForDeck: { [key: string]: string } = {
			'transitions-deck': 'transitiondeck',
			'animations-deck': 'animationdeck',
			'cleanup-deck': 'cleanupdeck',
		};
		const coreDeckCommands = [
			'.uno:SidebarDeck.PropertyDeck',
			'.uno:ModifyPage',
			'.uno:CustomAnimation',
			'.uno:MasterSlidesPanel',
		];

		const activePanelCommand = panelCommandForDeck[currentDeck];
		const stateHandler = this.map['stateChangeHandler'];

		const setHighlight = (command: string, active: boolean) => {
			const value = active ? 'true' : 'false';
			if (stateHandler && stateHandler.getItemValue(command) === value) return;
			this.map.fire('commandstatechanged', {
				commandName: command,
				state: value,
			});
		};

		Object.values(panelCommandForDeck).forEach((command) =>
			setHighlight(command, command === activePanelCommand),
		);

		// A notebookbar panel has taken over the sidebar: core still thinks its
		// last deck is open and keeps reporting it active, so clear those buttons.
		if (activePanelCommand)
			coreDeckCommands.forEach((command) => setHighlight(command, false));
	}

	markNavigatorTreeView(data: WidgetJSON): boolean {
		if (!data) return false;

		if (data.type === 'treelistbox') {
			(data as TreeWidgetJSON).draggable = false;
			return true;
		}

		for (const i in data.children) {
			if (this.markNavigatorTreeView(data.children[i])) {
				return true;
			}
		}

		return false;
	}
	onResize() {
		if (this.resizeTaskId)
			app.layoutingService.cancelLayoutingTask(this.resizeTaskId);

		this.resizeTaskId = app.layoutingService.appendLayoutingTask(() => {
			if (this.container) {
				(this.container as HTMLElement).style.height =
					this.wrapper.getBoundingClientRect().height + 'px';
			}
			this.resizeTaskId = null;
		});
	}

	callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		return builder._defaultCallbackHandler(
			objectType,
			eventType,
			object,
			data,
			builder,
		);
	}
}
JSDialog.SidebarBase = SidebarBase;
