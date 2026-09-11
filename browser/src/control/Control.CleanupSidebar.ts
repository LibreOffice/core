/* -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/* global app JSDialog _ */

/*
 * Control.CleanupSidebar - the Cleanup deck.
 *
 * The deck is a JSDialog component of its own that borrows the sidebar dock. It builds with a
 * builder of its own, keeps its own model, and handles the widget events of its own widgets, so
 * none of them travels to a window in the kit. The dock says when its content changes hands, and
 * the deck stands down when it does.
 */

declare var JSDialog: any;

/// The deck the panel fills. The dock names the deck itself with a '-deck' suffix.
const CLEANUP_DECK_ID = 'cleanup';

/// The deck the panel is carried in: one panel holding the body, drawn as a section that
/// folds shut under its title, the way the panels of a core deck do.
function cleanupDeckJSON(body: Array<WidgetJSON>): DeckWidgetJSON {
	const title = _('Clean Up');

	return {
		id: CLEANUP_DECK_ID + '-deck',
		type: 'deck',
		enabled: true,
		visible: true,
		text: title,
		headerText: title,
		name: CLEANUP_DECK_ID + '-deck',
		children: [
			{
				id: CLEANUP_DECK_ID + '-panel',
				name: CLEANUP_DECK_ID + '-panel',
				text: title,
				visible: true,
				enabled: true,
				expanded: true,
				hidden: false,
				type: 'panel',
				children: body,
			} as PanelWidgetJSON,
		],
	};
}

class CleanupSidebar extends SidebarBase {
	/// True while the dock holds the deck.
	private shown = false;

	constructor(map: MapInterface) {
		super(map, SidebarType.Cleanup);
		this.map.on('dockcontent', this.onDockContent, this);
	}

	onRemove() {
		super.onRemove();
		this.map.off('dockcontent', this.onDockContent, this);
	}

	/// The deck is built and driven in the browser, so the builder is told the window the
	/// other browser-driven decks of this dock use and every widget event stops here.
	protected createBuilder() {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			windowId: WindowId.Notebookbar,
			cssClass: 'jsdialog sidebar',
			useScrollAnimation: false,
			suffix: 'sidebar',
			callback: this.callback.bind(this),
		} as JSBuilderOptions);
	}

	/// The deck borrows the sidebar dock rather than having one of its own.
	protected setupContainer(parentContainer?: HTMLElement) {
		this.container = document.getElementById(
			'sidebar-container',
		) as HTMLElement;
		this.wrapper = document.getElementById(
			'sidebar-dock-wrapper',
		) as HTMLElement;
		this.documentContainer = document.querySelector(
			'#document-container',
		) as HTMLDivElement;
	}

	/// Every widget event of the deck is answered here. There is nothing to answer yet.
	callback(
		objectType: string,
		eventType: string,
		object: any,
		data: any,
		builder: JSBuilder,
	) {
		return true;
	}

	/// Turns the deck on or off.
	public toggle(): void {
		if (this.shown) {
			this.closeSidebar();
			return;
		}

		this.takeOverDock();
		this.buildDeck();
		this.showSidebar();
		this.updatePresentationDeckHighlight(CLEANUP_DECK_ID + '-deck');
	}

	/// Empties the dock for the deck. A core deck showing there is closed in the kit as
	/// well, so the deck commands keep toggling in step with what the user sees.
	private takeOverDock(): void {
		// isVisible is true exactly while a core deck fills the dock
		if (this.isVisible()) app.socket.sendMessage('uno .uno:SidebarHide');
		this.closeSidebar();
	}

	/// The body of the deck, empty for now.
	protected deckBody(): Array<WidgetJSON> {
		return [];
	}

	/// Puts the deck in the dock, built with this component's own builder, and takes the
	/// content of the dock over so that patches meant for another component pass it by.
	private buildDeck(): void {
		if (!this.builder || !this.container) return;

		this.model.fullUpdate({
			id: CLEANUP_DECK_ID,
			jsontype: SidebarType.Cleanup,
			type: 'container',
			dialogid: '0',
			children: [cleanupDeckJSON(this.deckBody())],
		} as any as JSDialogJSON);
		this.markContainerContentOwner();

		// The deck is built away from the page and put in once it is whole.
		const fragment = new DocumentFragment();
		const temporary = window.L.DomUtil.create('div', '', fragment);
		this.builder.build(temporary, [this.model.getSnapshot()], false);

		const container = this.container;
		app.layoutingService.appendLayoutingTask(() =>
			container.replaceChildren(temporary.firstChild),
		);
		this.onResize();
	}

	/// Draws one widget of the deck again from new JSON. The model takes the new state as
	/// well, so a rebuild of the whole deck draws the same thing. True when it was drawn.
	public updateWidget(control: WidgetJSON): boolean {
		if (!this.shown || !this.builder || !this.container) return false;

		this.model.widgetUpdate(control);
		this.builder.updateWidget(this.container, control);
		return true;
	}

	/// The dock this deck borrows now holds something else, or nothing at all. Every way
	/// the deck can go ends here: the toggle closing it, another deck taking the dock, and
	/// the dock being closed by the read-only switch.
	private onDockContent(e: DockContentEvent): void {
		if (e.wrapper !== this.wrapper) return;

		if (e.jsontype === this.allowedJsonType) {
			this.shown = true;
			return;
		}

		if (!this.shown) return;

		this.shown = false;
		this.onHidden();
	}

	/// The dock no longer holds the deck.
	protected onHidden(): void {
		this.updatePresentationDeckHighlight('');
	}
}

JSDialog.CleanupSidebar = function (map: MapInterface) {
	return new CleanupSidebar(map);
};
