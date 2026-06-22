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

	/// the most recent style list, kept so the previews can be re-rendered
	/// when the display density changes
	private lastStyles: any[] = [];

	/// media query that fires when the display density changes
	private dpiMediaQuery: MediaQueryList | null = null;

	constructor(map: any, impl: any) {
		super(map, 'Notebookbar', 'notebookbar');
		this.impl = impl;

		this.createBuilder();
		this.impl.setBuilder(this.builder, this.model);

		this.map.addControl(this.impl);

		this.registerMessageHandlers();
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
			this.map.on('updatetoolbarcommandvalues', this.onCommandValues, this);
		}

		this.registerDPIChangeListener();
	}

	// when we hide the UI
	public onRemove() {
		if (this.builder) {
			this.map.off(
				'commandstatechanged',
				this.builder.onCommandStateChanged,
				this.builder,
			);
			this.map.off('updatetoolbarcommandvalues', this.onCommandValues, this);
		}

		this.unregisterDPIChangeListener();

		if (this.impl) this.impl.onRemove();
	}

	// The style previews are bitmaps converted by core at a fixed pixel
	// density. A density change leaves the cached images blurry until they
	// are re-rendered, so listen for it and rebuild the style box. The query
	// matches a single density value, so it is re-registered after each
	// change to track the new density.
	private onDPIChange = () => {
		if (this.lastStyles.length > 0) this.updateStylesView(this.lastStyles);
		this.registerDPIChangeListener();
	};

	private registerDPIChangeListener() {
		if (!window.matchMedia) return;
		if (this.dpiMediaQuery)
			this.dpiMediaQuery.removeEventListener('change', this.onDPIChange);
		this.dpiMediaQuery = window.matchMedia(
			'(resolution: ' + window.devicePixelRatio + 'dppx)',
		);
		this.dpiMediaQuery.addEventListener('change', this.onDPIChange, {
			once: true,
		});
	}

	private unregisterDPIChangeListener() {
		if (this.dpiMediaQuery) {
			this.dpiMediaQuery.removeEventListener('change', this.onDPIChange);
			this.dpiMediaQuery = null;
		}
	}

	public onCallback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
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
		this.builder?._defaultCallbackHandler(
			objectType,
			eventType,
			object,
			data,
			builder,
		);
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
		if (toolbar) toolbar.outerHTML = '';
		// create toolbar from template
		$('#toolbar-logo').after(this.map.toolbarUpTemplate.cloneNode(true));
		toolbar = window.L.DomUtil.get('toolbar-up');

		this.container = window.L.DomUtil.create(
			'div',
			'notebookbar-scroll-wrapper',
			toolbar,
		);
	}

	protected onJSUpdate(e: any) {
		if (super.onJSUpdate(e)) {
			this.impl?.setInitialized(true);
			return true;
		}
		return false;
	}

	private onCommandValues(e: any) {
		if (e.commandName === '.uno:StyleApply') {
			const commandValues = e.commandValues;
			if (!commandValues) return;

			let styles: any[] = [];
			if (this.map.getDocType() === 'text') {
				styles = commandValues.ParagraphStyles;
			} else if (this.map.getDocType() === 'spreadsheet') {
				styles = commandValues.CellStyles;
			}

			if (styles && styles.length > 0) {
				this.updateStylesView(styles);
			}
		}
	}

	private updateStylesView(styles: any[]) {
		this.lastStyles = styles;

		const widgetData = this.model.getById('stylesview') as IconViewJSON;
		if (!widgetData || !this.builder) return;

		const entries: any[] = [];
		styles.forEach((style: any, index: number) => {
			// The engine sends each style as { text: <localized display name>,
			// id: <programmatic name> }. Display 'text', apply via 'id'.
			const progName: string =
				style && typeof style === 'object' ? style.id : style;
			let localeStyle: string =
				style && typeof style === 'object' ? style.text : style;
			if (typeof progName === 'string' && progName.startsWith('outline')) {
				const parts = progName.split('outline');
				const outlineLevel = parts.length > 1 ? parts[1] : '';
				localeStyle = _('Outline %OutlineLevel%').replace(
					'%OutlineLevel%',
					outlineLevel,
				);
			}
			entries.push({
				text: localeStyle,
				id: progName,
				row: index,
				selected: false,
				ondemand: true,
				width: 96,
				height: 30, // adjust with expected image to not blink
			});
		});

		widgetData.entries = entries;
		if (this.container) {
			this.builder.updateWidget(this.container, widgetData);
		}
	}

	protected onJSAction(e: any) {
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
