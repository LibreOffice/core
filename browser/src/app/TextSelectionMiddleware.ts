/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class TextSelections {
	// Below 4 are used for text selection handles, which are visible where mouse pointer doesn't exist (search "pointer: fine" in css).
	private static start: TextSelectionHandle;
	private static end: TextSelectionHandle;
	private static startRectangle: cool.SimpleRectangle | null = null; // This is the rectangle sent from the core side.
	private static endRectangle: cool.SimpleRectangle | null = null; // This is the rectangle sent from the core side.

	private static active: boolean = false; // Is there a text selection or not.

	public static initiate() {
		this.start = new TextSelectionHandle(
			'selection_start_handle',
			30,
			44,
			new cool.SimplePoint(0, 0),
			'text-selection-handle-start',
			false,
		);
		this.end = new TextSelectionHandle(
			'selection_end_handle',
			30,
			44,
			new cool.SimplePoint(0, 0),
			'text-selection-handle-end',
			false,
		);

		app.sectionContainer.addSection(this.start);
		app.sectionContainer.addSection(this.end);
	}

	public static deactivate() {
		if (!this.active) return;

		this.active = false;
		app.map.fire('textselectionchange', { active: this.active });
		this.start.setShowSection(false);
		this.end.setShowSection(false);

		// Schedule a redraw if there's not any.
		app.sectionContainer.requestReDraw();
	}

	public static activate() {
		if (this.active) return;

		this.active = true;
		app.map.fire('textselectionchange', { active: this.active });

		if (this.startRectangle && this.endRectangle) {
			this.start.setShowSection(true);
			this.end.setShowSection(true);
			this.updateMarkers();
		}
	}

	public static isActive(): boolean {
		return this.active;
	}

	public static showHandles() {
		if (this.start.isSectionShown()) this.start.setOpacity(1);

		if (this.end.isSectionShown()) this.end.setOpacity(1);
	}

	public static hideHandles() {
		if (this.start.isSectionShown()) this.start.setOpacity(0);

		if (this.end.isSectionShown()) this.end.setOpacity(0);
	}

	public static getStartRectangle(): cool.SimpleRectangle | null {
		return this.startRectangle ? this.startRectangle.clone() : null;
	}

	public static getEndRectangle(): cool.SimpleRectangle | null {
		return this.endRectangle ? this.endRectangle.clone() : null;
	}

	public static setStartRectangle(rectangle: cool.SimpleRectangle) {
		this.startRectangle = rectangle;
		this.updateMarkers();
	}

	public static setEndRectangle(rectangle: cool.SimpleRectangle) {
		this.endRectangle = rectangle;
		this.updateMarkers();
	}

	private static updateMarkers() {
		if (!this.isActive()) return;

		if (this.startRectangle === null || this.endRectangle === null) return;

		var startPos = { x: this.startRectangle.pX1, y: this.startRectangle.pY2 };
		var endPos = { x: this.endRectangle.pX1, y: this.endRectangle.pY2 };

		if (app.map._docLayer.isCalcRTL()) {
			Util.ensureValue(app.activeDocument);
			// Mirror position from right to left.
			startPos.x =
				app.activeDocument.activeLayout.viewedRectangle.pX2 -
				(startPos.x - app.activeDocument.activeLayout.viewedRectangle.pX1);
			endPos.x =
				app.activeDocument.activeLayout.viewedRectangle.pX2 -
				(endPos.x - app.activeDocument.activeLayout.viewedRectangle.pX1);
		}

		const oldStart = this.start.getPosition();
		const oldEnd = this.end.getPosition();

		startPos.x -= 30 * app.dpiScale;
		this.start.setPosition(startPos.x, startPos.y);
		const newStart = this.start.getPosition();

		this.end.setPosition(endPos.x, endPos.y);
		const newEnd = this.end.getPosition();

		if (
			app.map._docLayer.isCalcRTL() &&
			(newStart.y < newEnd.y ||
				(newStart.y <= newEnd.y && newStart.x < newEnd.x))
		) {
			// If the start handle is actually closer to the end of the selection, reverse positions (Right To Left case).
			this.start.setPosition(newEnd.pX, newEnd.pY);
			this.end.setPosition(newStart.pX, newStart.pY);
		} else if (
			!app.map._docLayer.isCalcRTL() &&
			(oldEnd.distanceTo(newStart.toArray()) < 20 ||
				oldStart.distanceTo(newEnd.toArray()) < 20)
		) {
			/*
				If the start handle is actually closer to the end of the selection, reverse positions.
				This seems to be a core side issue to me. I think the start and end positions are switched but the handlers aren't on the core side.
			*/
			this.switchStartEndHandles();
		}

		// Schedule a redraw if there's not any.
		app.sectionContainer.requestReDraw();
	}

	public static switchStartEndHandles() {
		const temp = this.start;
		this.start = this.end;
		this.end = temp;
	}

	public static dispose() {
		app.sectionContainer.removeSection(this.start.name);
		app.sectionContainer.removeSection(this.end.name);
	}
}

(window as any).TextSelections = TextSelections;
