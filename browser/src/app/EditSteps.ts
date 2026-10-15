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
	The steps editing goes by in a view that draws from the objects: how far a key moves an object
	or a handle, how far a key scrolls the view, and the angles a turn snaps to. Each has a default
	and can be set in the client's preferences under the name given beside it.

	Every one of them has four states, the same everywhere: Ctrl, Shift, Alt and no modifier.
	Where several are held the first of Ctrl, Shift and Alt counts. A move and a scroll go from
	Ctrl as the largest over Shift and no modifier to Alt, one pixel as it is seen at the zoom of
	the moment. A turn goes from Ctrl over Shift to Alt; with no modifier the mouse turns freely
	and a key by the smallest step.

	A step that is a distance in the document grows with the page. Up to ten times the size of an
	A4 page it is as given; from ten times on it is ten times that, from a hundred times on a
	hundred times that, and so on, so a page of ten metres by ten is moved in steps that suit it
	and the steps stay round numbers.
*/
class EditSteps {
	/// The longer side of an A4 page, in twips, which the page factor is measured against.
	private static readonly a4LongerSide: number = (297 * 1440) / 25.4;

	/// A millimetre, in twips.
	private static readonly millimetre: number = 1440 / 25.4;

	/// The value of that preference where it is a number above nought, the default otherwise.
	private static setting(name: string, byDefault: number): number {
		const value = window.prefs?.getNumber(name, byDefault) ?? byDefault;
		return value > 0 ? value : byDefault;
	}

	/// One pixel as it is seen, in twips: the twips of a core pixel are the twips of a screen
	/// pixel only where the screen is not scaled, so the scale goes in.
	private static pixel(): number {
		return app.pixelsToTwips * app.dpiScale;
	}

	/// What a distance in the document is multiplied by for the page shown: one up to ten times
	/// an A4 page, then the power of ten below how many times larger the page is.
	public static pageFactor(): number {
		const size = app.activeDocument?.fileSize;
		if (!size) return 1;

		const times = Math.max(size.x, size.y) / EditSteps.a4LongerSide;
		if (!(times >= 10)) return 1;

		return Math.pow(10, Math.floor(Math.log10(times)));
	}

	/*
		How far a key moves an object or a handle, in twips: 25 millimetres with Ctrl
		(vectorKeyStepControl), 10 with Shift (vectorKeyStepShift), 1 with no modifier
		(vectorKeyStep), each grown with the page, and one pixel with Alt. The office takes the
		last three.
	*/
	public static moveStep(event: KeyboardEvent): number {
		let millimetres: number;
		if (event.ctrlKey)
			millimetres = EditSteps.setting('vectorKeyStepControl', 25);
		else if (event.shiftKey)
			millimetres = EditSteps.setting('vectorKeyStepShift', 10);
		else if (event.altKey) return EditSteps.pixel();
		else millimetres = EditSteps.setting('vectorKeyStep', 1);

		return millimetres * EditSteps.millimetre * EditSteps.pageFactor();
	}

	/*
		How far a key scrolls the view along each axis, in twips, for a view of that size in twips.
		The office scrolls by a share of what is in view rather than by a distance, so a step
		means the same at every zoom: half of it with Ctrl (vectorScrollStepControl), the office's
		page, a quarter with Shift (vectorScrollStepShift), a twentieth with no modifier
		(vectorScrollStep), the office's line, and one pixel with Alt.
	*/
	public static scrollStep(
		event: KeyboardEvent,
		viewWidth: number,
		viewHeight: number,
	): cool.Point {
		let share: number;
		if (event.ctrlKey)
			share = EditSteps.setting('vectorScrollStepControl', 0.5);
		else if (event.shiftKey)
			share = EditSteps.setting('vectorScrollStepShift', 0.25);
		else if (event.altKey) {
			const pixel = EditSteps.pixel();
			return new cool.Point(pixel, pixel);
		} else share = EditSteps.setting('vectorScrollStep', 0.05);

		return new cool.Point(viewWidth * share, viewHeight * share);
	}

	/*
		The step in degrees a turn snaps to, counted from the page: 45 with Ctrl
		(vectorTurnSnapControl), 15 with Shift (vectorTurnSnapShift), 5 with Alt
		(vectorTurnSnapAlt). With no modifier the mouse has no step, and a key the one below.
	*/
	public static turnSnap(event?: {
		ctrlKey?: boolean;
		shiftKey?: boolean;
		altKey?: boolean;
	}): number | null {
		if (event?.ctrlKey) return EditSteps.setting('vectorTurnSnapControl', 45);
		if (event?.shiftKey) return EditSteps.setting('vectorTurnSnapShift', 15);
		if (event?.altKey) return EditSteps.setting('vectorTurnSnapAlt', 5);
		return null;
	}

	/*
		The angle on from that one by a key: the next step of that size up for a way of 1, the
		next one down for -1. A hair is allowed for, so an angle that stands on a step goes on to
		the next one.
	*/
	public static nextStep(angle: number, step: number, way: number): number {
		const inSteps = angle / step;
		const next =
			way > 0 ? Math.floor(inSteps + 1e-6) + 1 : Math.ceil(inSteps - 1e-6) - 1;
		return next * step;
	}

	/// The step in degrees a key with no modifier turns to, counted from the page
	/// (vectorKeyTurnStep).
	public static keyTurnStep(): number {
		return EditSteps.setting('vectorKeyTurnStep', 1);
	}
}
