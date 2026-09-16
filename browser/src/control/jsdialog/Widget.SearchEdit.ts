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
 * JSDialog.SearchEdit - single line input field for searching inside document
 *
 * Example JSON:
 * {
 *     id: 'id',
 *     type: 'searchedit',
 *     text: 'abc',
 *     placeholder: 'this is shown when empty',
 *     changedCallback: null
 * }
 */

declare var JSDialog: any;

class SearchEditWidget extends EditWidget {
	constructor(
		parentContainer: HTMLElement,
		data: EditWidgetJSON,
		builder: JSBuilder,
		callback: JSDialogCallback,
	) {
		super(parentContainer, data, builder, callback);
	}

	private onSearchInput() {
		this.updateSearchButtons();
		if (window.L.Map.THIS.getDocType() === 'text') {
			// perform the immediate search in Writer
			app.searchService.search(
				this.edit.input.value,
				false,
				'',
				0,
				true /* expand search */,
			);
		}
	}

	private onSearchKeyDown(e: KeyboardEvent) {
		var entry = this.edit.input;
		if (
			(e.keyCode === 71 && e.ctrlKey) ||
			e.keyCode === 114 ||
			e.keyCode === 13
		) {
			if (e.shiftKey) {
				app.searchService.search(entry.value, true);
			} else {
				app.searchService.search(entry.value);
			}
			e.preventDefault();
		} else if (e.ctrlKey && e.keyCode === 70) {
			entry.focus();
			entry.select();
			e.preventDefault();
		} else if (e.keyCode === 27) {
			window.L.Map.THIS.cancelSearch();
		}
	}

	private onSearchFocus() {
		window.L.Map.THIS.fire('searchstart');
		this.updateSearchButtons();
	}

	private onSearchBlur() {
		if (window.mode.isSmallScreenDevice()) return;

		window.L.Map.THIS._onGotFocus();
	}

	// One handler for the page, replaced when the search bar is rebuilt.
	private static viewportResizeHandler: (() => void) | null = null;

	// Viewport size at the previous resize event.
	private lastViewportHeight = 0;
	private lastViewportWidth = 0;

	private tallestViewportHeight = 0;
	private static readonly fullHeightTolerance = 4;

	private onViewportResize() {
		const viewport = window.visualViewport;
		if (!viewport) return;

		const rotated = viewport.width !== this.lastViewportWidth;
		const grown = viewport.height > this.lastViewportHeight;
		this.lastViewportHeight = viewport.height;
		this.lastViewportWidth = viewport.width;

		// The keyboard changes only the height. A rotation changes both.
		if (rotated) {
			this.tallestViewportHeight = viewport.height;
			return;
		}
		this.tallestViewportHeight = Math.max(
			this.tallestViewportHeight,
			viewport.height,
		);

		// Back at full height means the keyboard closed.
		const fullHeight =
			viewport.height >=
			this.tallestViewportHeight - SearchEditWidget.fullHeightTolerance;
		const focused = document.activeElement === this.edit.input;
		if (grown && fullHeight && focused) this.edit.input.blur();
	}

	private updateSearchButtons() {
		var toolbar = window.mode.isSmallScreenDevice()
			? app.map.mobileSearchBar
			: app.map.statusBar;
		if (!toolbar) {
			console.debug('Cannot find search bar');
			return;
		}

		// conditionally disabling until, we find a solution for tdf#108577
		if (this.edit.input.value === '') {
			toolbar.enableItem('searchprev', false);
			toolbar.enableItem('searchnext', false);
			if (window.mode.isSmallScreenDevice()) {
				toolbar.enableItem('cancelsearch', false);
			} else {
				toolbar.showItem('cancelsearch', false);
			}
		} else {
			toolbar.enableItem('searchprev', true);
			toolbar.enableItem('searchnext', true);
			if (window.mode.isSmallScreenDevice()) {
				toolbar.enableItem('cancelsearch', true);
			} else {
				toolbar.showItem('cancelsearch', true);
			}
		}
	}

	protected setupEventListeners() {
		super.setupEventListeners();

		this.edit.input.addEventListener('input', this.onSearchInput.bind(this));
		this.edit.input.addEventListener(
			'keydown',
			this.onSearchKeyDown.bind(this),
		);
		this.edit.input.addEventListener('focus', this.onSearchFocus.bind(this));
		this.edit.input.addEventListener('blur', this.onSearchBlur.bind(this));

		const viewport = window.visualViewport;
		if (window.mode.isSmallScreenDevice() && viewport) {
			this.lastViewportHeight = viewport.height;
			this.lastViewportWidth = viewport.width;
			this.tallestViewportHeight = viewport.height;
			if (SearchEditWidget.viewportResizeHandler)
				viewport.removeEventListener(
					'resize',
					SearchEditWidget.viewportResizeHandler,
				);
			SearchEditWidget.viewportResizeHandler = this.onViewportResize.bind(this);
			viewport.addEventListener(
				'resize',
				SearchEditWidget.viewportResizeHandler,
			);
		}
	}
}

JSDialog.searchEdit = function (
	parentContainer: HTMLElement,
	data: EditWidgetJSON,
	builder: JSBuilder,
	callback: JSDialogCallback,
) {
	const widget = new SearchEditWidget(parentContainer, data, builder, callback);
	return widget.build();
};
