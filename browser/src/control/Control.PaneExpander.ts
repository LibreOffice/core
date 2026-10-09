// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

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
 * PaneExpander - the shared expand state for the navigation sidebar and the
 * slide import pane. Maximizing covers the document area with the open
 * panels: with both open they share it side by side, each showing its
 * slides as a grid, split at the draggable divider; with one open that
 * panel fills the area alone. The expand toggle in either panel's header
 * switches the one shared state, so both toggles reflect it through
 * aria-pressed. The current state is a single class on
 * #main-document-content, which the CSS turns into the overlay layout.
 *
 * While maximized the navigation sidebar's slide list switches to a grid
 * of larger thumbnails. Pressing Escape returns to the normal layout.
 */

/* global app _ */

type PaneExpandMode = 'normal' | 'expanded';

class PaneExpander {
	// The panels the maximized layout can hold, by element id. A panel is
	// open while it carries the visible class.
	private static readonly PANEL_IDS: string[] = [
		'navigation-sidebar',
		'slide-import-dock-wrapper',
	];

	private map: any;
	private mainContent: HTMLElement;
	private mode: PaneExpandMode = 'normal';

	constructor(map: any) {
		this.map = map;
		this.mainContent = document.getElementById('main-document-content');
		document.addEventListener('keydown', this.onKeyDown.bind(this), true);
		this.map.on('themechanged', this.updateToggles, this);
		// A document loads in the normal side-by-side layout. When one is
		// opened in place of another the maximized state would otherwise
		// carry over, so clear it as the new document's layer comes up.
		this.map.on('doclayerinit', this.reset, this);
	}

	public getMode(): PaneExpandMode {
		return this.mode;
	}

	public toggle(): void {
		this.setMode(this.mode === 'expanded' ? 'normal' : 'expanded');
	}

	public reset(): void {
		this.setMode('normal');
	}

	// A panel is closing. The maximized layout survives while another panel
	// stays open (its grid takes the whole area); once the last one goes the
	// layout has nowhere to live, so it returns to normal.
	public onPanelClosing(closingPanelId: string): void {
		if (this.mode !== 'expanded') return;
		for (const id of PaneExpander.PANEL_IDS) {
			if (id === closingPanelId) continue;
			const panel = document.getElementById(id);
			if (panel && panel.classList.contains('visible')) return;
		}
		this.reset();
	}

	private setMode(mode: PaneExpandMode): void {
		if (this.mode === mode || !this.mainContent) return;
		this.mainContent.classList.toggle('panes-expanded', mode === 'expanded');
		this.mode = mode;
		// The slide list is a grid only while the panels fill the document
		// area; otherwise it stays the narrow vertical strip.
		this.applyGridMode(mode === 'expanded');
		this.updateToggles();
		if (this.map.paneSplitter) this.map.paneSplitter.onModeChange();
		if (this.map.navigator) this.map.navigator.updateFloatingButtonVisibility();
	}

	private applyGridMode(enabled: boolean): void {
		const preview = this.map._docLayer && this.map._docLayer._preview;
		if (preview && typeof preview.setGridMode === 'function')
			preview.setGridMode(enabled);
		if (this.map.slideImportPane) this.map.slideImportPane.setGridMode(enabled);
	}

	// Every panel's expand toggle carries the shared class, so one query
	// keeps them all reflecting the one shared mode.
	private updateToggles(): void {
		const expanded = this.mode === 'expanded';
		document.querySelectorAll('.navigation-expand-button').forEach((button) => {
			PaneExpander.setToggleState(button, expanded);
		});
	}

	// The toggle keeps one accessible name, and aria-pressed tells whether the panels are
	// maximized. The tooltip and the icon name the action that a press performs from the given
	// state.
	public static setToggleState(button: Element, expanded: boolean): void {
		button.setAttribute('aria-label', _('Maximize to grid'));
		button.setAttribute('aria-pressed', expanded ? 'true' : 'false');
		button.setAttribute(
			'data-cooltip',
			expanded ? _('Exit grid') : _('Maximize to grid'),
		);
		const img = button.querySelector('img');
		if (!img) return;
		const name = expanded ? 'collapse-slides.svg' : 'expand-slides.svg';
		img.src = app.LOUtil.getImageURL(name);
	}

	// Escape returns to the normal layout before any panel handles it as a
	// request to close itself, so the first press only un-maximizes. An open
	// menu, dropdown or dialog gets the first press instead, so a transient
	// popup still closes on Escape while a panel is maximized.
	private onKeyDown(e: KeyboardEvent): void {
		if (e.key !== 'Escape' || this.mode === 'normal') return;
		if (this.isTransientPopupOpen()) return;
		this.reset();
		// A tooltip on screen shows the text it read when it opened, which named the action of
		// the layout just left, so it closes with that layout.
		if (this.map.tooltip) this.map.tooltip.hide();
		e.preventDefault();
		e.stopPropagation();
	}

	private isTransientPopupOpen(): boolean {
		const ui = this.map.uiManager;
		if (ui && ui.isAnyContextMenuOpened && ui.isAnyContextMenuOpened())
			return true;
		if (
			this.map.jsdialog &&
			this.map.jsdialog.hasDialogOpened &&
			this.map.jsdialog.hasDialogOpened()
		)
			return true;
		if (
			this.map.dialog &&
			this.map.dialog.hasOpenedDialog &&
			this.map.dialog.hasOpenedDialog()
		)
			return true;
		return false;
	}
}
