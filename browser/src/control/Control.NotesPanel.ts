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
 * JSDialog.NotesPanel - the speaker notes editor, in a pane below the slide.
 *
 * The pane is a jsdialog surface of its own, separate from the canvas: the engine builds an
 * editengine custom widget over the notes outliner and sends it under the "notespanel" JSON type.
 *
 * The pane is opened and closed with .uno:BottomPaneImpress, which is what makes the engine create
 * or drop the notes view shell that owns the widget.
 */

declare var JSDialog: any;

const NOTES_PANEL_MIN_HEIGHT = 60;
const NOTES_PANEL_DEFAULT_HEIGHT = 180;

class NotesPanel extends JSDialogComponent {
	private wrapper: HTMLElement | null = null;
	private column: HTMLElement | null = null;
	private splitter: HTMLElement | null = null;
	private dragStartY = 0;
	private dragStartHeight = 0;

	constructor(map: MapInterface) {
		super(map, 'notespanel', 'notespanel');
		this.onAdd(map);
	}

	protected createBuilder() {
		this.builder = new window.L.control.jsDialogBuilder({
			mobileWizard: this,
			map: this.map,
			windowId: WindowId.NotesPanel,
			cssClass: 'jsdialog notespanel',
			useScrollAnimation: false,
			suffix: 'notespanel',
		} as JSBuilderOptions);
	}

	protected setupContainer() {
		this.container =
			document.getElementById('notespanel-container') ?? undefined;
		this.wrapper = document.getElementById('notespanel-dock-wrapper');
		this.column = document.getElementById('document-and-notes');
		this.splitter = document.getElementById('notespanel-splitter');

		if (this.splitter)
			this.splitter.addEventListener(
				'pointerdown',
				this.onSplitterDown.bind(this),
			);
	}

	onAdd(map: MapInterface) {
		this.map = map;
		this.createBuilder();
		this.setupContainer();

		this.map.on('notespanel', this.onNotesPanel, this);
		this.registerMessageHandlers();

		this.applyStoredHeight();
		this.refreshState();
	}

	onRemove() {
		this.map.off('notespanel', this.onNotesPanel, this);
		this.unregisterMessageHandlers();
	}

	/// The engine sends the whole tree when the notes view shell appears, and single widget
	/// updates after that. Arrival of the tree is what opens the pane.
	private onNotesPanel(e: { data: JSDialogJSON }) {
		if (!this.container || !this.builder) return;

		this.container.replaceChildren();
		this.builder.build(this.container, [e.data], false);
		this.setVisible(true);
	}

	public isVisible(): boolean {
		return !!this.wrapper && this.wrapper.classList.contains('visible');
	}

	/// Shows or hides the pane. The engine side follows from .uno:BottomPaneImpress, which the
	/// caller sends; this only moves the client layout.
	public setVisible(show: boolean) {
		if (!this.wrapper) return;

		this.wrapper.classList.toggle('visible', show);
		if (this.column) this.column.classList.toggle('notes-visible', show);

		if (!show && this.container) this.container.replaceChildren();

		this.map?.uiManager.setDocTypePref('ShowNotesPanel', show);
		app.events.fire('resize', null);

		this.refreshState();
	}

	/// Broadcasts which of the three notes views is active, so the View Notes menu can mark the
	/// current entry and the status bar Notes button can show its pressed state. Exactly one of
	/// bottom panel, handout page and hidden is active at a time.
	public refreshState() {
		if (!this.map) return;

		const bottom = this.isVisible();
		const handout = !!(app.impress && app.impress.notesMode);

		this.map.fire('commandstatechanged', {
			commandName: 'notespanel',
			state: bottom ? 'true' : 'false',
		});
		this.map.fire('commandstatechanged', {
			commandName: 'notespanelbottom',
			state: bottom ? 'true' : 'false',
		});
		this.map.fire('commandstatechanged', {
			commandName: 'notespanelhandout',
			state: handout ? 'true' : 'false',
		});
		this.map.fire('commandstatechanged', {
			commandName: 'notespanelhidden',
			state: !bottom && !handout ? 'true' : 'false',
		});
	}

	/// Asks the engine to open or close the notes pane.
	public toggle() {
		this.map.sendUnoCommand('.uno:BottomPaneImpress');
		if (this.isVisible()) this.setVisible(false);
	}

	private heightPrefKey(): string {
		return this.map.getDocType() + '.NotesPanelHeight';
	}

	private applyStoredHeight() {
		if (!this.wrapper) return;

		const stored = window.prefs.get(
			this.heightPrefKey(),
			NOTES_PANEL_DEFAULT_HEIGHT,
		);
		const height = parseInt(stored, 10);
		this.wrapper.style.height =
			(isNaN(height) ? NOTES_PANEL_DEFAULT_HEIGHT : height) + 'px';
	}

	private onSplitterDown(event: PointerEvent) {
		if (!this.wrapper || !this.splitter) return;

		event.preventDefault();
		this.dragStartY = event.clientY;
		this.dragStartHeight = this.wrapper.getBoundingClientRect().height;
		this.splitter.setPointerCapture(event.pointerId);

		const onMove = (moveEvent: PointerEvent) => this.onSplitterMove(moveEvent);
		const onUp = (upEvent: PointerEvent) => {
			this.splitter?.releasePointerCapture(upEvent.pointerId);
			this.splitter?.removeEventListener('pointermove', onMove);
			this.splitter?.removeEventListener('pointerup', onUp);
			this.storeHeight();
		};

		this.splitter.addEventListener('pointermove', onMove);
		this.splitter.addEventListener('pointerup', onUp);
	}

	private onSplitterMove(event: PointerEvent) {
		if (!this.wrapper || !this.column) return;

		// Dragging up grows the pane, so the delta is subtracted.
		const available = this.column.getBoundingClientRect().height;
		const wanted = this.dragStartHeight - (event.clientY - this.dragStartY);
		const height = Math.max(
			NOTES_PANEL_MIN_HEIGHT,
			Math.min(wanted, available * 0.7),
		);

		this.wrapper.style.height = height + 'px';
		app.events.fire('resize', null);
	}

	private storeHeight() {
		if (!this.wrapper) return;
		window.prefs.set(
			this.heightPrefKey(),
			Math.round(this.wrapper.getBoundingClientRect().height),
		);
	}
}

JSDialog.NotesPanel = function (map: MapInterface) {
	return new NotesPanel(map);
};
