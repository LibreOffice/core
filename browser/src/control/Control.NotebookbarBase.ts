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
 * Control.NotebookbarBase - component for tabbed menu on the top of application
 */

class NotebookbarBase extends JSDialogComponent {
	/// reference to old JS Notebookbar
	impl: any = null;

	/// rebuilds the style box at the new density so the previews are re-rendered
	private dpiListener: DPIChangeListener = new DPIChangeListener(() => {
		this.reloadStylesView();
	});

	constructor(map: any, impl: any) {
		super(map, 'Notebookbar', 'notebookbar');
		this.impl = impl;

		this.createBuilder();
		this.impl.setBuilder(this.builder, this.model);

		this.map.addControl(this.impl);

		this.registerMessageHandlers();
	}

	/// Whether the notebookbar takes the widget data of the engine. The notebookbar of a kind of
	/// document active in place in the document has no notebookbar in the engine.
	private followsCore: boolean = true;

	/// Leaves the widget data of the engine to the notebookbar of the document. The items of this
	/// one are still shown and hidden, which the client asks for with actions of its own.
	public ignoreCoreUpdates() {
		this.followsCore = false;
		this.map.off('jsdialogupdate', this.onJSUpdate, this);
	}

	// when we show the UI
	public onAdd() {
		this.setupContainer(undefined);
		this.impl.create(this.container);
		if (this.builder) {
			this.map.on(
				'commandstatechanged',
				this.builder.onCommandStateChanged,
				this.builder,
			);
		}
		if (!this.followsCore) {
			this.map.on('commandstatechanged', this.onFontStateChanged, this);
			this.showFontStates();
		}

		this.dpiListener.start();
	}

	// when we hide the UI
	public onRemove() {
		this.map.off('commandstatechanged', this.onFontStateChanged, this);
		if (this.builder) {
			this.map.off(
				'commandstatechanged',
				this.builder.onCommandStateChanged,
				this.builder,
			);
		}

		this.dpiListener.stop();

		if (this.impl) this.impl.onRemove();
	}

	public onCallback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		if (!this.followsCore && this.applyFontCommand(eventType, object, data))
			return;
		if (this.impl && this.impl.onCallback) {
			const consumed = this.impl.onCallback(
				objectType,
				eventType,
				object,
				data,
				builder,
			);
			if (consumed) return;
		}
		return this.builder?._defaultCallbackHandler(
			objectType,
			eventType,
			object,
			data,
			builder,
		);
	}

	/// The font boxes of a notebookbar without widget data from the engine show the font name and
	/// size that the commands of the document active in place report.
	private onFontStateChanged(e: { commandName: string; state: string }) {
		if (e.commandName === '.uno:CharFontName')
			this.setFontBoxText('fontnamecombobox', e.state);
		else if (e.commandName === '.uno:FontHeight')
			this.setFontBoxText('fontsizecombobox', e.state === '0' ? '' : e.state);
	}

	/// Fills the font boxes with the fonts of the document and the last font name and size that
	/// the commands reported.
	private showFontStates() {
		const fontNameBox: any = this.container?.querySelector('#fontnamecombobox');
		const fonts = (this.map as any).getToolbarCommandValues?.(
			'.uno:CharFontName',
		);
		if (fontNameBox?.updateEntries && fonts && typeof fonts === 'object')
			fontNameBox.updateEntries(Object.keys(fonts));
		for (const command of ['.uno:CharFontName', '.uno:FontHeight']) {
			const state = this.map.stateChangeHandler.getItemValue(command);
			if (state !== undefined && state !== null)
				this.onFontStateChanged({ commandName: command, state: String(state) });
		}
	}

	private setFontBoxText(id: string, text: string) {
		const box: any = this.container?.querySelector('#' + id);
		if (box?.onSetText) box.onSetText(text);
	}

	/// Applies a font name or size picked in a font box with a command, which reaches the document
	/// active in place. Answers whether the event was the pick of a font box.
	private applyFontCommand(eventType: string, object: any, data: any): boolean {
		const isFontName = object?.id === 'fontnamecombobox';
		if (!isFontName && object?.id !== 'fontsizecombobox') return false;
		let value: string;
		if (eventType === 'selected') value = data.substr(data.indexOf(';') + 1);
		else if (eventType === 'change') value = data;
		else return false;
		if (isFontName) (this.map as any).applyFont(value);
		else (this.map as any).applyFontSize(value);
		this.map.focus();
		return true;
	}

	protected createBuilder() {
		this.builder = new window.L.control.notebookbarBuilder({
			windowId: WindowId.Notebookbar,
			mobileWizard: this.impl,
			map: this.map,
			cssClass: 'notebookbar',
			useSetTabs: true,
			suffix: 'notebookbar',
			callback: this.onCallback.bind(this),
		});
	}

	protected setupContainer(parentContainer?: HTMLElement) {
		// remove old toolbar
		let toolbar = window.L.DomUtil.get('toolbar-up');
		if (toolbar) toolbar.remove();
		// create toolbar from template
		const toolbarLogo = document.getElementById('toolbar-logo');
		if (toolbarLogo)
			toolbarLogo.after(this.map.toolbarUpTemplate.cloneNode(true));
		toolbar = window.L.DomUtil.get('toolbar-up');

		this.container = window.L.DomUtil.create(
			'div',
			'notebookbar-scroll-wrapper',
			toolbar,
		);
	}

	protected onJSUpdate(e: { data: UpdateData }) {
		if (
			e.data &&
			e.data.jsontype === this.allowedJsonType &&
			e.data.control &&
			this.impl?.onCoreWidgetUpdate?.(e.data.control)
		)
			return true;

		if (super.onJSUpdate(e)) {
			this.impl?.setInitialized(true);
			return true;
		}
		return false;
	}

	private reloadStylesView() {
		const widgetData = this.model.getById('stylesview') as IconViewJSON;
		if (!widgetData || !this.builder) return;

		if (this.container) this.builder.updateWidget(this.container, widgetData);
	}

	protected onJSAction(e: { data: ActionData }) {
		const actionType = e.data?.data?.action_type;
		if (!this.followsCore && actionType !== 'show' && actionType !== 'hide')
			return false;
		if (super.onJSAction(e)) {
			this.impl?.setInitialized(true);
			return true;
		}
		return false;
	}

	/// used to get full model
	public getFullJSON(): any[] {
		return this.impl?.getFullJSON();
	}

	// shortcuts

	/// used to get shortcut items
	public getDefaultToolItems(): any[] {
		return this.impl?.getDefaultToolItems();
	}

	public insertButtonToShortcuts(button: ToolItemWidgetJSON) {
		this.impl?.insertButtonToShortcuts(button);
	}

	public reloadShortcutsBar() {
		this.impl?.reloadShortcutsBar();
	}

	public showNotebookbarCommand(commandId: string, show: boolean): boolean {
		return this.impl?.showNotebookbarCommand(commandId, show);
	}

	// tabs

	public getTabs() {
		return this.impl?.getTabs();
	}

	public setTabs(tabs: any[]) {
		this.impl?.setTabs();
	}

	public showTabs() {
		this.impl?.showTabs();
	}

	public hideTabs() {
		this.impl?.hideTabs();
	}

	public selectTab(tabName: string) {
		this.impl?.selectTab(tabName);
	}

	public refreshContextTabsVisibility() {
		this.impl?.refreshContextTabsVisibility();
	}

	// customization

	public showItem(id: string, show?: boolean): boolean {
		return this.impl?.showItem(id /* no show used */);
	}

	public hideItem(id: string): boolean {
		return this.impl?.hideItem(id);
	}
}

JSDialog.NotebookbarBase = function (map: any, impl: any) {
	return new NotebookbarBase(map, impl);
};
