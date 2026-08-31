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
 * JSDialog.OverflowManager - parent for OverflowGroup, coordinates it's behavior
 */

declare var JSDialog: any;

class OverflowManager {
	parentContainer: HTMLElement;
	data: ContainerWidgetJSON;
	lastMaxWidth: number = -1;
	// content width (scrollWidth) of the container at the end of the last fold pass
	lastContentWidth: number = -1;
	// visible width (offsetWidth) of the container at the end of the last fold pass
	lastContainerWidth: number = -1;
	scheduledRefresh: TaskId = '';
	sizeObserver: ResizeObserver | null = null;
	contentObserver: MutationObserver | null = null;

	constructor(parentContainer: Element, data: ContainerWidgetJSON) {
		this.parentContainer = parentContainer as HTMLElement;
		this.data = data;

		window.addEventListener('resize', this.onResize.bind(this));
		if (app.map) app.map.on('refreshoverflows', this.onRefresh, this);
		else app.console.error('OverflowManager: no app.map available');

		// The widths the fold decision depends on keep changing after the
		// container is built: the parent is laid out late (classic mode, or
		// the notebookbar hidden until edit permission arrives), widget
		// updates and show/hide actions arrive in deferred layouting tasks,
		// and the UI font loads asynchronously. Watch the container and run
		// the fold decision again whenever the measured widths change.
		if (typeof ResizeObserver !== 'undefined') {
			this.sizeObserver = new ResizeObserver(() => {
				if (this.parentContainer.scrollWidth > 0) this.verifyFoldState();
			});
			this.sizeObserver.observe(this.parentContainer);
		}

		if (typeof MutationObserver !== 'undefined') {
			this.contentObserver = new MutationObserver(() => this.verifyFoldState());
			this.contentObserver.observe(this.parentContainer, {
				childList: true,
				subtree: true,
				attributes: true,
				attributeFilter: ['class', 'style'],
			});
		}

		if (document.fonts && document.fonts.ready)
			document.fonts.ready.then(() => this.verifyFoldState());
	}

	// Runs the fold decision again when the current widths differ from the
	// ones recorded at the end of the last onRefresh. When nothing changed
	// this is three property reads and no DOM modification, so it is cheap
	// enough to call from the observers on every content change.
	verifyFoldState() {
		if (!this.parentContainer) return;
		// a scheduled refresh will run the full fold decision anyway
		if (this.scheduledRefresh !== '') return;
		// hidden containers measure as zero width, skip them
		if (this.parentContainer.offsetParent === null) return;

		if (
			this.lastContentWidth === this.parentContainer.scrollWidth &&
			this.lastContainerWidth === this.parentContainer.offsetWidth &&
			this.lastMaxWidth === window.innerWidth
		)
			return;

		this.onRefresh({ force: true } as Event & { force?: boolean });
	}

	calculateMaxWidth(): number {
		const margin = 20; // how many px more has to be visible after last widget
		let nextElement = this.parentContainer.nextSibling as HTMLElement;
		// floating right element after spacer
		if (nextElement && nextElement.classList.contains('ui-spacer'))
			nextElement = nextElement.nextSibling as HTMLElement;

		let nextElementPosition = nextElement ? nextElement.offsetLeft : 0;
		if (nextElementPosition <= 0)
			// is a last visible sibling
			nextElementPosition = this.parentContainer.offsetWidth + margin + 1;
		if (nextElementPosition > window.innerWidth)
			nextElementPosition = window.innerWidth;

		const startPosition = this.parentContainer.offsetLeft;

		return nextElementPosition - startPosition - margin;
	}

	hasOverflow(maxWidth: number): boolean {
		const requiredWidth = this.parentContainer.scrollWidth;
		app.console.debug(
			'overflow manager: "' +
				this.data.id +
				'" max: ' +
				maxWidth +
				' req: ' +
				requiredWidth,
		);

		// Width not known yet -> defer the decision. The ResizeObserver
		// installed in the constructor retriggers the fold decision once the
		// container has been measured; folding now would leave every group
		// collapsed until the next window resize.
		if (requiredWidth === 0) return false;

		return maxWidth < requiredWidth;
	}

	onResize(event: Event) {
		// Ignore resize when the window size has not actually changed.
		if (this.lastMaxWidth === window.innerWidth) return;

		app.console.debug(
			'OverflowManager: onResize, scheduledRefresh = ' +
				(this.scheduledRefresh !== '' ? 'true' : 'false'),
		);
		this.lastMaxWidth = -1;

		if (this.scheduledRefresh !== '') {
			// collapse events
			app.layoutingService.cancelLayoutingTask(this.scheduledRefresh);
		}

		// Force the deferred refresh to actually measure: an intervening
		// synchronous refreshoverflows call (e.g. from a notebookbar tab
		// switch) may already have set lastMaxWidth for this window size
		// from a premature measurement, taken before the newly shown
		// content (icons, fonts) has settled. Without force, onRefresh's
		// lastMaxWidth guard would then skip this later, layout-settled
		// measurement and leave the earlier, possibly wrong fold state in
		// place.
		this.scheduledRefresh = app.layoutingService.appendLayoutingTask(() =>
			this.onRefresh({ force: true } as Event & { force?: boolean }),
		);
	}

	// sometimes we want to call it synchronously as it is already in the task (tab switch)
	onRefresh(event: Event & { force?: boolean }) {
		app.console.debug(
			'OverflowManager: onRefresh, force = ' + (event.force ? 'true' : 'false'),
		);
		this.scheduledRefresh = '';
		if (!this.parentContainer) return;
		if (!event.force && this.lastMaxWidth === window.innerWidth) return;

		// check our visibility
		let parentNode = this.parentContainer;
		while (parentNode) {
			if (parentNode?.classList?.contains('hidden')) return;

			parentNode = parentNode.parentNode as HTMLElement;
		}

		// Bail when any ancestor is display:none (e.g. notebookbar
		// collapsed via #toolbar-row). Measuring a hidden container
		// gives near-zero width, which would fold every group into the
		// hiddenItems wrapper and corrupt the layout that needs to be
		// restored when the notebookbar is shown again.
		if (this.parentContainer.offsetParent === null) return;

		this.lastMaxWidth = window.innerWidth;

		const groups = this.parentContainer.querySelectorAll('.ui-overflow-group');

		// first show all the groups
		groups.forEach((element: OverflowGroupContainer) => {
			if (typeof element.unfoldGroup === 'function') element.unfoldGroup();
		});

		const maxWidth = this.calculateMaxWidth();

		// then hide required
		for (let i = groups.length - 1; i >= 0; i--) {
			const element: OverflowGroupContainer = groups[i];
			if (maxWidth >= 0 && this.hasOverflow(maxWidth)) {
				if (typeof element.foldGroup === 'function') element.foldGroup();
			}
		}

		// remember the widths this fold decision was computed from
		this.lastContentWidth = this.parentContainer.scrollWidth;
		this.lastContainerWidth = this.parentContainer.offsetWidth;
	}
}

JSDialog.OverflowManager = function (
	parentContainer: Element,
	data: ContainerWidgetJSON,
	builder: JSBuilder,
) {
	parentContainer.classList.add('ui-overflow-manager');
	// Just create manager which will attach itself to the container and resize event.
	// Builder will process children as in regular container.
	new OverflowManager(parentContainer, data);
	app.console.debug('Create OverflowManager for: "' + data.id + '"');
	return true;
} as JSWidgetHandler;
