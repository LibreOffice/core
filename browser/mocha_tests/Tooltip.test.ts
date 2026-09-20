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

describe('Tooltip', function () {
	let container: HTMLElement;
	let trigger: HTMLElement;
	let neighbour: HTMLElement;
	let tooltip: any;
	let showDelay: number;
	let grace: number;

	// Every element jsdom builds has a rectangle of zero size at the origin.
	const BOX = { left: 100, top: 100, right: 180, bottom: 120 };

	function widget(text: string): HTMLElement {
		const elem = document.createElement('div');

		elem.setAttribute('data-cooltip', text);
		document.body.appendChild(elem);
		return elem;
	}

	function shown(): boolean {
		return container.style.visibility === 'visible';
	}

	function pointerAt(x: number, y: number): void {
		document.dispatchEvent(
			new (window as any).MouseEvent('mousemove', { clientX: x, clientY: y }),
		);
	}

	function press(key: string): void {
		window.dispatchEvent(
			new (window as any).KeyboardEvent('keydown', { key: key }),
		);
	}

	beforeEach(function () {
		installFakeClock();
		// docstate.ts hands the jsdom window an app of its own, so the registry
		// the bundle built there is not the one the tooltip reaches for.
		(app as any).timerRegistry = new TimerRegistry();

		container = document.createElement('div');
		document.body.appendChild(container);
		(container as any).getBoundingClientRect = () => BOX;

		trigger = widget('Undo');
		neighbour = widget('Redo');

		tooltip = (window as any).L.control.tooltip({
			container: container,
			window: window,
		});
		showDelay = tooltip._options.timeout;
		grace = tooltip._options.hoverGrace;
	});

	afterEach(function () {
		restoreClock();
		container.remove();
		trigger.remove();
		neighbour.remove();
	});

	it('draws nothing until the pointer has waited out the show delay', function () {
		tooltip.beginShow(trigger);

		advanceClock(showDelay - 1);
		nodeassert.strictEqual(shown(), false);

		advanceClock(1);
		nodeassert.strictEqual(shown(), true);
		nodeassert.strictEqual(container.textContent, 'Undo');
	});

	it('stays for the whole grace period after the pointer leaves', function () {
		tooltip.show(trigger);
		tooltip.beginHide();

		advanceClock(grace - 1);
		nodeassert.strictEqual(shown(), true);

		advanceClock(1);
		nodeassert.strictEqual(shown(), false);
	});

	it('stays for as long as the pointer is on it', function () {
		tooltip.show(trigger);
		tooltip.beginHide();
		advanceClock(grace / 2);

		pointerAt(BOX.left + 10, BOX.top + 10);
		advanceClock(grace * 10);

		nodeassert.strictEqual(shown(), true);
	});

	it('starts the grace period again when the pointer leaves it', function () {
		tooltip.show(trigger);
		tooltip.beginHide();
		pointerAt(BOX.left + 10, BOX.top + 10);
		advanceClock(grace * 10);

		pointerAt(BOX.left - 50, BOX.top - 50);

		advanceClock(grace - 1);
		nodeassert.strictEqual(shown(), true);

		advanceClock(1);
		nodeassert.strictEqual(shown(), false);
	});

	it('closes when the pointer leaves across a widget it crossed too fast', function () {
		tooltip.show(trigger);
		tooltip.beginHide();
		tooltip.beginShow(neighbour);
		tooltip.beginHide();

		advanceClock(grace);

		nodeassert.strictEqual(shown(), false);
	});

	it('closes on Escape without waiting', function () {
		tooltip.show(trigger);

		press('Escape');

		nodeassert.strictEqual(shown(), false);
	});

	it('leaves the tooltip of the next widget alone', function () {
		tooltip.show(trigger);
		tooltip.beginHide();

		tooltip.show(neighbour);
		advanceClock(grace);

		nodeassert.strictEqual(shown(), true);
		nodeassert.strictEqual(container.textContent, 'Redo');
	});
});
