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
	scheduledRefresh: TaskId = '';
	initialSizeResizeObserver: ResizeObserver | null = null;

	constructor(parentContainer: Element, data: ContainerWidgetJSON) {
		this.parentContainer = parentContainer as HTMLElement;
		this.data = data;

		window.addEventListener('resize', this.onResize.bind(this));
		if (app.map) app.map.on('refreshoverflows', this.onRefresh, this);
		else app.console.error('OverflowManager: no app.map available');

		// In classic mode the first onRefresh can fire before the parent has
		// been laid out, leaving scrollWidth at 0 and every group folded.
		// Re-run once the container actually has a measurable width.
		if (typeof ResizeObserver !== 'undefined') {
			this.initialSizeResizeObserver = new ResizeObserver(() => {
				if (this.parentContainer.scrollWidth > 0) {
					this.initialSizeResizeObserver?.disconnect();
					this.initialSizeResizeObserver = null;
					this.onRefresh({ force: true } as Event & { force?: boolean });
				}
			});
			this.initialSizeResizeObserver.observe(this.parentContainer);
		}
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
		// installed in the constructor will retrigger onRefresh once the
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

		this.scheduledRefresh = app.layoutingService.appendLayoutingTask(() =>
			this.onRefresh(event),
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
