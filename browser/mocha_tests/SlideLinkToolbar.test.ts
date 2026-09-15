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

describe('SlideLinkToolbar', function () {
	// The map events the toolbar listens for, by name, so a test can fire one.
	let listeners: Map<string, (e: any) => void>;
	// The actions the toolbar's buttons asked for.
	let dispatched: string[];
	let editable: boolean;
	// The identifier of the slide on show.
	let current: string;
	// The pages that are linked, with what they are linked to.
	let linked: Map<string, { source: string; name: string }>;
	// The pages whose source has moved on since they were read.
	let outdated: Set<string>;
	// The state of every source, as the storage announced it.
	let sourceState: string;
	let toolbar: SlideLinkToolbar;
	let savedDispatcher: any;
	let savedEvents: any;

	// The linked pages, by the identifier each one holds.
	const numbersPart = '{6A1C31B0-0001-4C5B-9E77-2F3D4A5B6C7D}';
	const outlookPart = '{6A1C31B0-0002-4C5B-9E77-2F3D4A5B6C7D}';
	// A page this document does not hold.
	const unknownPart = '{6A1C31B0-0099-4C5B-9E77-2F3D4A5B6C7D}';

	function element(): HTMLElement | null {
		return document.getElementById('slide-link-toolbar');
	}

	// The labels of the actions on show, in order.
	function actions(): string[] {
		const found = element();
		if (!found) return [];
		return Array.from(
			found.querySelectorAll('button.slide-link-toolbar-action'),
		).map((button) => button.textContent || '');
	}

	function action(name: string): HTMLButtonElement | null {
		const found = element();
		return found
			? (found.querySelector(
					'button.slide-link-toolbar-action.' + name,
				) as HTMLButtonElement | null)
			: null;
	}

	function slideShown(part: string): void {
		current = part;
		const listener = listeners.get('setpart');
		nodeassert.ok(listener, 'nothing listens for setpart');
		listener({ selectedPart: 0 });
	}

	// The JSX runtime tells a node from a text child by its class, which the
	// jsdom window has and the Node global does not.
	before(function () {
		if (typeof (globalThis as any).Node === 'undefined')
			(globalThis as any).Node = (window as any).Node;
	});

	beforeEach(function () {
		listeners = new Map();
		dispatched = [];
		editable = true;
		current = numbersPart;
		linked = new Map([
			[numbersPart, { source: 'Sales deck.odp', name: 'Numbers' }],
			[outlookPart, { source: 'Sales deck.odp', name: 'Outlook' }],
		]);
		outdated = new Set();
		sourceState = 'connected';

		const map: any = {
			on: function (name: string, callback: any, context: any) {
				listeners.set(name, callback.bind(context));
			},
			isEditMode: () => editable,
			isPresentationOrDrawing: () => true,
			slideLinks: {
				currentPart: () => current,
				getPageLink: (part: string) => linked.get(part) || null,
				isPageOutdated: (part: string) => outdated.has(part),
				getPageSourceState: (part: string) =>
					linked.has(part) ? sourceState : '',
				isPageUpdatable: (part: string) =>
					linked.has(part) &&
					sourceState !== '' &&
					sourceState !== 'noaccess' &&
					sourceState !== 'missing',
			},
		};

		savedDispatcher = (app as any).dispatcher;
		savedEvents = app.events;
		(app as any).dispatcher = {
			dispatch: (name: string) => dispatched.push(name),
		};
		(app as any).events = {
			on: () => {},
			fire: () => {},
		};

		toolbar = new SlideLinkToolbar(map);
	});

	afterEach(function () {
		const found = element();
		if (found) found.remove();
		(app as any).dispatcher = savedDispatcher;
		(app as any).events = savedEvents;
	});

	it('stays away from a slide that is linked to nothing', function () {
		slideShown(unknownPart);
		nodeassert.equal(element(), null);
	});

	it('names the link of the slide on show, with the actions folded away', function () {
		slideShown(numbersPart);
		const found = element();
		nodeassert.ok(found);
		nodeassert.ok(
			(found.getAttribute('aria-label') || '').indexOf('Sales deck.odp') >= 0,
		);
		nodeassert.deepEqual(actions(), []);
	});

	it('offers an update alone while folded when the source has moved on', function () {
		outdated.add(numbersPart);
		slideShown(numbersPart);
		nodeassert.deepEqual(actions(), ['Update']);
		nodeassert.equal(action('update').disabled, false);
	});

	it('unfolds every action, and folds them away again', function () {
		slideShown(numbersPart);
		toolbar.setExpanded(true);
		nodeassert.deepEqual(actions(), ['Unlink', 'Update']);

		toolbar.setExpanded(false);
		nodeassert.deepEqual(actions(), []);
	});

	it('unfolds from its own chevron', function () {
		slideShown(numbersPart);
		const expander = element().querySelector(
			'button.slide-link-toolbar-expander',
		) as HTMLButtonElement;
		nodeassert.equal(expander.getAttribute('aria-expanded'), 'false');

		expander.click();
		nodeassert.equal(toolbar.isExpanded(), true);
		nodeassert.deepEqual(actions(), ['Unlink', 'Update']);
	});

	it('offers no action to a view that cannot edit', function () {
		editable = false;
		outdated.add(numbersPart);
		slideShown(numbersPart);
		nodeassert.deepEqual(actions(), []);

		toolbar.setExpanded(true);
		nodeassert.deepEqual(actions(), []);
	});

	it('acts on the slide on show', function () {
		slideShown(numbersPart);
		toolbar.setExpanded(true);

		action('unlink').click();
		action('update').click();
		nodeassert.deepEqual(dispatched, ['unlinkslide', 'updatelinkedslide']);
	});

	it('says when the source cannot be read, and offers no update from it', function () {
		sourceState = 'noaccess';
		slideShown(numbersPart);
		toolbar.setExpanded(true);

		const state = element().querySelector('.slide-link-toolbar-state');
		nodeassert.ok(state);
		nodeassert.equal(state.textContent, 'No access');
		nodeassert.equal(action('update').disabled, true);
		// The link can still be taken off.
		nodeassert.deepEqual(actions(), ['Unlink', 'Update']);
		nodeassert.equal(action('unlink').disabled, false);
	});

	it('keeps the focus when it is drawn again', function () {
		slideShown(numbersPart);
		const expander = element().querySelector(
			'button.slide-link-toolbar-expander',
		) as HTMLButtonElement;
		expander.focus();
		expander.click();

		// The chevron was pressed from the keyboard, and the one drawn in its
		// place holds the focus so the actions can be reached with Tab.
		const redrawn = element().querySelector(
			'button.slide-link-toolbar-expander',
		);
		nodeassert.notEqual(redrawn, expander);
		nodeassert.equal(document.activeElement, redrawn);

		// A toolbar that did not hold the focus does not take it.
		(document.activeElement as HTMLElement).blur();
		slideShown(outlookPart);
		nodeassert.notEqual(
			document.activeElement,
			element().querySelector('button.slide-link-toolbar-expander'),
		);
	});

	it('follows the slide on show', function () {
		slideShown(numbersPart);
		nodeassert.ok(element());

		slideShown(unknownPart);
		nodeassert.equal(element(), null);

		slideShown(outlookPart);
		nodeassert.ok(
			(element().getAttribute('aria-label') || '').indexOf('Outlook') >= 0,
		);
	});
});
