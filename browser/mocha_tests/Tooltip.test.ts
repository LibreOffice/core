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

	function pointerAt(x: number, y: number, buttons: number = 0): void {
		document.dispatchEvent(
			new (window as any).MouseEvent('mousemove', {
				clientX: x,
				clientY: y,
				buttons: buttons,
			}),
		);
	}

	// The tooltip takes no pointer events, so a move over it is aimed at what lies beneath.
	function pointerOver(elem: HTMLElement, x: number, y: number): void {
		elem.dispatchEvent(
			new (window as any).MouseEvent('mousemove', {
				clientX: x,
				clientY: y,
				bubbles: true,
			}),
		);
	}

	// A click inside the tooltip also goes to what lies beneath it.
	function clickOn(elem: HTMLElement, x: number, y: number): void {
		elem.dispatchEvent(
			new (window as any).MouseEvent('click', {
				clientX: x,
				clientY: y,
				bubbles: true,
			}),
		);
	}

	function pointerEvent(
		type: string,
		target: EventTarget,
		buttons: number,
	): void {
		target.dispatchEvent(
			new (window as any).MouseEvent(type, {
				clientX: BOX.left - 50,
				clientY: BOX.top - 50,
				buttons: buttons,
				bubbles: true,
			}),
		);
	}

	function press(key: string): void {
		window.dispatchEvent(
			new (window as any).KeyboardEvent('keydown', { key: key }),
		);
	}

	beforeEach(function () {
		// The tooltip waits for the pointer only while the accessibility setting
		// is on. The checks below need it on.
		(window as any).prefs.set('accessibilityState', true);
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
		// The tooltip keeps listening on the window, so a switched off one ignores the keys.
		tooltip.disable();
		restoreClock();
		(window as any).prefs.remove('accessibilityState');
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

	it('stays while the pointer is on it over another widget', function () {
		tooltip.show(trigger);
		tooltip.beginHide();

		pointerOver(neighbour, BOX.left + 10, BOX.top + 10);
		advanceClock(grace * 10);

		nodeassert.strictEqual(shown(), true);
		nodeassert.strictEqual(container.textContent, 'Undo');
	});

	it('goes at once when the widget under it is clicked', function () {
		tooltip.show(trigger);
		tooltip.beginHide();
		pointerOver(neighbour, BOX.left + 10, BOX.top + 10);

		clickOn(neighbour, BOX.left + 10, BOX.top + 10);

		nodeassert.strictEqual(shown(), false);
		advanceClock(grace * 10);
		nodeassert.strictEqual(shown(), false);
	});

	it('stays when a click lands outside it', function () {
		tooltip.show(trigger);

		clickOn(neighbour, BOX.left - 50, BOX.top - 50);

		nodeassert.strictEqual(shown(), true);
	});

	it('goes at once when dismissed, even with the pointer on it', function () {
		tooltip.show(trigger);
		pointerAt(BOX.left + 10, BOX.top + 10);

		tooltip.dismiss();

		nodeassert.strictEqual(shown(), false);
	});

	it('does not come up later for a widget hovered just before a dismiss', function () {
		tooltip.beginShow(trigger);

		tooltip.dismiss();
		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), false);
	});

	it('shows nothing for a widget the pointer crosses with a button held', function () {
		pointerAt(BOX.left - 50, BOX.top - 50, 1);

		tooltip.beginShow(trigger);
		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), false);
	});

	it('drops a waiting show once a button is pressed and the pointer moves', function () {
		tooltip.beginShow(trigger);
		pointerAt(BOX.left - 50, BOX.top - 50, 1);

		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), false);
	});

	it('shows tooltips again once the button is released', function () {
		pointerAt(BOX.left - 50, BOX.top - 50, 1);
		pointerAt(BOX.left - 50, BOX.top - 50, 0);

		tooltip.beginShow(trigger);
		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), true);
		nodeassert.strictEqual(container.textContent, 'Undo');
	});

	it('knows the button is up when a hover starts before any move', function () {
		pointerAt(BOX.left - 50, BOX.top - 50, 1);
		pointerEvent('mouseover', trigger, 0);

		tooltip.beginShow(trigger);
		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), true);
	});

	it('shows the tooltip of the widget that a drag ends on', function () {
		pointerAt(BOX.left - 50, BOX.top - 50, 1);
		tooltip.beginShow(trigger);

		pointerEvent('mouseup', trigger, 0);
		advanceClock(showDelay);

		nodeassert.strictEqual(shown(), true);
		nodeassert.strictEqual(container.textContent, 'Undo');
	});

	it('shows nothing after a drag that only crossed a widget', function () {
		pointerAt(BOX.left - 50, BOX.top - 50, 1);
		tooltip.beginShow(trigger);
		tooltip.beginHide();

		pointerEvent('mouseup', document, 0);
		advanceClock(showDelay);

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

	describe('without accessibility', function () {
		// Half the show delay, which is what a tooltip that the pointer is not
		// meant to reach stays for.
		let quickGrace: number;

		beforeEach(function () {
			(window as any).prefs.set('accessibilityState', false);
			quickGrace = showDelay / 2;
		});

		it('goes shortly after the pointer leaves the widget', function () {
			tooltip.show(trigger);
			tooltip.beginHide();

			advanceClock(quickGrace - 1);
			nodeassert.strictEqual(shown(), true);

			advanceClock(1);
			nodeassert.strictEqual(shown(), false);
		});

		it('goes even while the pointer rests on it', function () {
			tooltip.show(trigger);
			tooltip.beginHide();

			pointerAt(BOX.left + 10, BOX.top + 10);
			advanceClock(quickGrace);

			nodeassert.strictEqual(shown(), false);
		});

		it('still takes the show delay to come up', function () {
			tooltip.beginShow(trigger);

			advanceClock(showDelay - 1);
			nodeassert.strictEqual(shown(), false);

			advanceClock(1);
			nodeassert.strictEqual(shown(), true);
		});

		it('releases a tooltip the pointer was holding when it is switched off', function () {
			(window as any).prefs.set('accessibilityState', true);
			tooltip.show(trigger);
			tooltip.beginHide();
			pointerAt(BOX.left + 10, BOX.top + 10);
			advanceClock(grace * 10);
			nodeassert.strictEqual(shown(), true);

			(window as any).prefs.set('accessibilityState', false);
			pointerAt(BOX.left + 10, BOX.top + 10);
			advanceClock(quickGrace);

			nodeassert.strictEqual(shown(), false);
		});
	});
});
