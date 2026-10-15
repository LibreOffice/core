/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class CellSelectionMarkers {
	static start: CellSelectionHandle;
	static end: CellSelectionHandle;
	private static initiated: boolean = false;

	public static initiate() {
		if (app.map._docLayer._docType !== 'spreadsheet')
			// Calc only.
			return;

		this.initiated = true;

		// Cell selection handles (mobile & tablet).
		this.start = new CellSelectionHandle('cell_selection_handle_start');
		this.end = new CellSelectionHandle('cell_selection_handle_end');
		app.sectionContainer.addSection(this.start);
		app.sectionContainer.addSection(this.end);
	}

	// The range the handles sit on, in tile twips: the active formula reference, else the cell
	// selection, else the cell cursor. Null hides the handles.
	private static markedRectangle(): cool.SimpleRectangle | null {
		const docLayer = app.map._docLayer;

		const reference = docLayer.getActiveReferenceRectangle();
		if (reference) return reference.clone();

		if (docLayer._cellSelectionArea) return docLayer._cellSelectionArea.clone();

		if (app.calc.cellCursorVisible) {
			Util.ensureValue(app.calc.cellCursorRectangle);
			return app.calc.cellCursorRectangle.clone();
		}

		return null;
	}

	public static update() {
		if (window.mode.isDesktop()) return; // Not shown on desktop.

		if (!this.initiated) return;

		const cellRectangle = this.markedRectangle();

		if (cellRectangle) {
			this.start.setShowSection(true);
			this.end.setShowSection(true);

			const posStart = new cool.SimplePoint(cellRectangle.x1, cellRectangle.y1);
			const posEnd = new cool.SimplePoint(cellRectangle.x2, cellRectangle.y2);

			const offset = this.start.sectionProperties.circleRadius;
			this.start.setPosition(posStart.pX - offset, posStart.pY - offset);
			this.end.setPosition(posEnd.pX - offset, posEnd.pY - offset);
		} else {
			this.start.setShowSection(false);
			this.end.setShowSection(false);
		}
	}
}
