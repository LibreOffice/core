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
	// A start and an end handle per reference of the edited formula.
	private static referenceHandles: CellSelectionHandle[] = [];
	private static initiated: boolean = false;

	public static initiate() {
		if (app.map._docLayer._docType !== 'spreadsheet')
			// Calc only.
			return;

		this.initiated = true;

		// Cell selection handles (mobile & tablet).
		this.start = new CellSelectionHandle('cell_selection_handle_start', true);
		this.end = new CellSelectionHandle('cell_selection_handle_end', false);
		app.sectionContainer.addSection(this.start);
		app.sectionContainer.addSection(this.end);
	}

	// The range the cell handles sit on, in tile twips: the cell selection, else the cell
	// cursor. Null hides the handles.
	private static markedRectangle(): cool.SimpleRectangle | null {
		const docLayer = app.map._docLayer;

		if (docLayer._cellSelectionArea) return docLayer._cellSelectionArea.clone();

		if (app.calc.cellCursorVisible) {
			Util.ensureValue(app.calc.cellCursorRectangle);
			return app.calc.cellCursorRectangle.clone();
		}

		return null;
	}

	private static placePair(
		start: CellSelectionHandle,
		end: CellSelectionHandle,
		rectangle: cool.SimpleRectangle | null,
	) {
		if (!rectangle) {
			start.setShowSection(false);
			end.setShowSection(false);
			return;
		}

		start.setShowSection(true);
		end.setShowSection(true);

		const offset = start.sectionProperties.circleRadius;
		start.setPosition(rectangle.pX1 - offset, rectangle.pY1 - offset);
		end.setPosition(rectangle.pX2 - offset, rectangle.pY2 - offset);
	}

	private static updateReferenceHandles(
		references: {
			rectangle: cool.SimpleRectangle;
			selection: string | null;
			active: boolean;
		}[],
	) {
		while (this.referenceHandles.length < references.length * 2) {
			const index = this.referenceHandles.length / 2;
			const start = new CellSelectionHandle(
				'reference_handle_start_' + index,
				true,
			);
			const end = new CellSelectionHandle(
				'reference_handle_end_' + index,
				false,
			);
			app.sectionContainer.addSection(start);
			app.sectionContainer.addSection(end);
			this.referenceHandles.push(start, end);
		}

		for (let i = references.length * 2; i < this.referenceHandles.length; i++)
			this.referenceHandles[i].setShowSection(false);

		// The active pair is on top, so a press on a shared corner goes to it.
		const defaultOrder = app.CSections.DefaultForDocumentObjects.drawingOrder;
		let orderChanged = false;
		for (let i = 0; i < references.length; i++) {
			const start = this.referenceHandles[2 * i];
			const end = this.referenceHandles[2 * i + 1];
			start.textSelection = references[i].selection;
			end.textSelection = references[i].selection;
			start.active = references[i].active;
			end.active = references[i].active;
			const order = references[i].active ? defaultOrder + 1 : defaultOrder;
			if (start.drawingOrder !== order) {
				start.drawingOrder = order;
				end.drawingOrder = order;
				orderChanged = true;
			}
			this.placePair(start, end, references[i].rectangle);
		}
		if (orderChanged) app.sectionContainer.applyDrawingOrders();
	}

	public static update() {
		if (window.mode.isDesktop()) return; // Not shown on desktop.

		if (!this.initiated) return;

		// The cell handles hide while a reference has handles.
		const references = app.map._docLayer.getReferencesOnSheet();
		this.updateReferenceHandles(references);
		this.placePair(
			this.start,
			this.end,
			references.length ? null : this.markedRectangle(),
		);
	}
}
