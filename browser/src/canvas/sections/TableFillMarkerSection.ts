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
/* See CanvasSectionContainer.ts for explanations. */

app.definitions.TableFillMarkerSection = class TableFillMarkerSection extends (
	AutoFillBaseSection
) {
	processingOrder: number = app.CSections.TableFillMarker.processingOrder;
	drawingOrder: number = app.CSections.TableFillMarker.drawingOrder;
	zIndex: number = app.CSections.TableFillMarker.zIndex;

	constructor() {
		super(app.CSections.TableFillMarker.name);
	}

	public calculatePositionViaCellSelection(point: Array<number>) {
		// not used for TableFillMarkerSection
	}

	// Give bottom right position of the bottom right cell of table style area, in core pixels.
	// It is Called with null parameter when table fill marker is not visible.
	public calculatePositionViaCellCursor(point: Array<number>) {
		this.sectionProperties.cellCursorPoint = point;
		this.setMarkerPosition();
	}

	protected isFillAreaVisible(): boolean {
		return !!this.sectionProperties.docLayer._tableAutoFillAreaPixels;
	}

	public onMouseEnter() {
		this.context.canvas.style.cursor = 'se-resize';
	}

	public onDoubleClick(point: cool.SimplePoint, e: MouseEvent) {
		// no-op: TableFillMarker does not respond to double-click
	}
};
