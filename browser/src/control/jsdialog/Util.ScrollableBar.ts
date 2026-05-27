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
 * JSDialog.ScrollableBar - helper for creating toolbars with scrolling left/right
 */

declare var JSDialog: any;

let pendingTask: TaskId | null = null;
let resizePriorityTask: TaskId | null = null;

function createScrollButtons(parent: Element, scrollable: Element) {
	window.L.DomUtil.addClass(scrollable, 'ui-scroll-wrapper');

	const left = window.L.DomUtil.create('div', 'ui-scroll-left', parent);
	const right = window.L.DomUtil.create('div', 'ui-scroll-right', parent);

	JSDialog.AddOnClick(left, () => {
		const scroll = $(scrollable).scrollLeft() - 300;
		$(scrollable).animate({ scrollLeft: scroll }, 300);
		setTimeout(function () {
			JSDialog.RefreshScrollables();
		}, 350);
	});

	JSDialog.AddOnClick(right, () => {
		const scroll = $(scrollable).scrollLeft() + 300;
		$(scrollable).animate({ scrollLeft: scroll }, 300);
		setTimeout(function () {
			JSDialog.RefreshScrollables();
		}, 350);
	});
}

function showArrow(arrow: HTMLElement, show: boolean) {
	if (show) arrow.style.setProperty('display', 'block');
	else arrow.style.setProperty('display', 'none');
}

function setupResizeHandler(container: Element, scrollable: Element) {
	const left = container.querySelector('.ui-scroll-left') as HTMLElement;
	const right = container.querySelector('.ui-scroll-right') as HTMLElement;
	var isRTL: boolean = document.documentElement.dir === 'rtl';
	var timer: any; // for shift + mouse wheel up/down

	const handlerImpl = () => {
		pendingTask = null;

		const rootContainer = scrollable.querySelector('div');
		if (!rootContainer) return;

		if (rootContainer.scrollWidth > window.innerWidth) {
			// we have overflowed content
			const direction = isRTL ? -1 : 1;
			if (direction * scrollable.scrollLeft > 0) {
				if (isRTL) showArrow(right, true);
				else showArrow(left, true);
			} else if (isRTL) showArrow(right, false);
			else showArrow(left, false);

			if (
				direction * scrollable.scrollLeft <
				rootContainer.scrollWidth - window.innerWidth - 1
			) {
				if (isRTL) showArrow(left, true);
				else showArrow(right, true);
			} else if (isRTL) showArrow(left, false);
			else showArrow(right, false);
		} else {
			showArrow(left, false);
			showArrow(right, false);
		}
	};

	const handler = () => {
		if (pendingTask) app.layoutingService.cancelLayoutingTask(pendingTask);
		pendingTask = app.layoutingService.appendLayoutingTask(handlerImpl);
	};

	// handler for toolbar and statusbar
	// runs if shift + mouse wheel up/down are used
	const wheelHandler = (e: WheelEvent) => {
		const rootContainer = scrollable.querySelector('div');
		if (
			!rootContainer ||
			(!e.shiftKey &&
				// let horizontal scrolling through
				e.deltaX == 0)
		)
			return;

		clearTimeout(timer);
		// wait until mouse wheel stops scrolling
		timer = setTimeout(function () {
			JSDialog.RefreshScrollables();
		}, 350);
	};

	window.addEventListener('resize', handler);
	window.addEventListener('scroll', handler);
	scrollable.addEventListener('wheel', wheelHandler);
}

function setupPriorityStatusHandler(scrollable: Element, toolItems: any[]) {
	const handlerImpl = function () {
		resizePriorityTask = null;
		const rootContainer = scrollable.querySelector('div');
		if (!rootContainer) return;

		const statusBarItems = Array.from(rootContainer.children) as HTMLElement[];
		// Match DOM items to toolItems by ID and set data-priority
		statusBarItems.forEach((domItem) => {
			const toolItem = toolItems.find((item) => {
				const itemIdBase = item.id.split(':')[0]; // The base ID with a possible suffix for example 'languagestatus:LanguageStatusMenu'
				return (
					itemIdBase === domItem.id || item.id + '-container' === domItem.id
				);
			});

			if (toolItem && toolItem.dataPriority) {
				domItem.setAttribute('data-priority', toolItem.dataPriority);
			} else {
				domItem.removeAttribute('data-priority');
			}
		});

		// Reset visibility of hidden statuses
		statusBarItems.forEach((item) => {
			item.classList.remove('status-hidden');
		});

		const availableWidth = scrollable.parentElement
			? scrollable.parentElement.clientWidth
			: window.innerWidth;
		let contentWidth = rootContainer.scrollWidth;

		if (contentWidth > availableWidth) {
			// Group items by data-priority
			const itemsByPriorityLevel: { [key: string]: HTMLElement[] } = {};
			statusBarItems.forEach((item) => {
				if (item.hasAttribute('data-priority')) {
					const priority = item.getAttribute('data-priority') || '0';
					if (!itemsByPriorityLevel[priority]) {
						itemsByPriorityLevel[priority] = [];
					}
					itemsByPriorityLevel[priority].push(item);
				}
			});

			const priorityLevels = Object.keys(itemsByPriorityLevel)
				.map(Number)
				.sort((a, b) => b - a);

			let remainingWidthToFit = contentWidth - availableWidth;
			// Hide items with the same priority level
			for (const priority of priorityLevels) {
				const itemsAtPriority = itemsByPriorityLevel[priority];
				if (!itemsAtPriority) continue;

				// Calculate total width of the items
				const combinedWidthAtPriority = itemsAtPriority.reduce(
					(sum, item) => sum + item.offsetWidth,
					0,
				);

				if (remainingWidthToFit > 0) {
					itemsAtPriority.forEach((item) => {
						item.classList.add('status-hidden');
					});
					remainingWidthToFit -= combinedWidthAtPriority;
					contentWidth -= combinedWidthAtPriority;
				} else {
					break;
				}
			}
		}
	}.bind(this);

	const handler = () => {
		if (resizePriorityTask)
			app.layoutingService.cancelLayoutingTask(resizePriorityTask);
		resizePriorityTask = app.layoutingService.appendLayoutingTask(handlerImpl);
	};
	window.addEventListener('resize', handler);
}

JSDialog.MakeScrollable = function (parent: Element, scrollable: Element) {
	window.L.DomUtil.addClass(scrollable, 'ui-scrollable-content');
	createScrollButtons(parent, scrollable);
	setupResizeHandler(parent, scrollable);
};

JSDialog.MakeStatusPriority = function (scrollable: Element, toolItems: any[]) {
	window.L.DomUtil.addClass(scrollable, 'ui-scrollable-content');
	setupPriorityStatusHandler(scrollable, toolItems);
};

JSDialog.RefreshScrollables = function () {
	app.console.debug('JSDialog.RefreshScrollables');
	window.dispatchEvent(new Event('resize'));
};
