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

class TableMiddleware {
	static markerid = 0;

	currentTableData: any = {};
	lastTableMarkerJson = '';
	currentTableMarkerJson = '';

	public tableResizeColumnMarkers: Array<TableResizeMarkerSection> = [];
	public tableResizeRowMarkers: Array<TableResizeMarkerSection> = [];
	public tableSelectionColumnMarkers: Array<TableSelectMarkerSection> = [];
	public tableSelectionRowMarkers: Array<TableSelectMarkerSection> = [];
	public tableInsertMarkers: Array<TableInsertMarkerSection> = [];
	public tableCornerResizeMarker: TableCornerResizeMarkerSection | null = null;
	public resizeMarkerMaxApproximation = 10 * app.dpiScale; // in pixels

	private tableAnchorSection: ShapeHandleAnchorSubSection | null = null;
	private tableMarkersDragged = false;
	private resizeMarkerWidth = 0;
	private resizeMarkerHeight = 0;
	private selectionMarkerWidth = 24;
	private selectionMarkerHeight = 24;
	private addButtonWidth = 16;
	private addButtonHeight = 16;
	private addButtonOffset = 10;

	public setupTableOverlay() {
		app.map.on('messagesdone', this.updateTableMarkers, this);
		app.map.on('zoomend', this.onZoomForTableMarkers, this);
		this.initializeStyles();
	}

	public hasTableSelection() {
		if (!this.currentTableData) return false;
		return (
			this.currentTableData.rows != null ||
			this.currentTableData.columns != null
		);
	}

	public onZoomForTableMarkers() {
		this.lastTableMarkerJson = '';
		this.updateTableMarkers();
	}

	public onTableSelectedMsg(txtMessage: string) {
		if (!app.map.isEditMode()) {
			this.clearTableMarkers();
			return;
		}

		if (this.tableMarkersDragged === true) {
			return;
		}

		// Parse the message
		txtMessage = txtMessage.substring('tableselected:'.length + 1);
		const message = JSON.parse(txtMessage);
		this.currentTableMarkerJson = txtMessage;
		this.currentTableData = message;

		app.map.fire('resettopbottompagespacing', {
			disableMarker: this.hasTableSelection(),
		});
	}

	private initializeStyles(): void {
		const bodyElement = document.getElementsByTagName('body')[0];

		const element = document.createElement('div');
		element.className = 'table-column-resize-marker';
		bodyElement.appendChild(element);

		const style = window.getComputedStyle(element, null);
		this.resizeMarkerWidth = parseInt(
			style.getPropertyValue('width').replace('px', ''),
		);
		this.resizeMarkerHeight = parseInt(
			style.getPropertyValue('height').replace('px', ''),
		);

		element.remove();
	}

	private clearTableMarkers() {
		var markerIndex;
		for (
			markerIndex = 0;
			markerIndex < this.tableResizeColumnMarkers.length;
			markerIndex++
		) {
			app.sectionContainer.removeSection(
				this.tableResizeColumnMarkers[markerIndex].name,
			);
		}
		this.tableResizeColumnMarkers = [];

		for (
			markerIndex = 0;
			markerIndex < this.tableResizeRowMarkers.length;
			markerIndex++
		) {
			app.sectionContainer.removeSection(
				this.tableResizeRowMarkers[markerIndex].name,
			);
		}
		this.tableResizeRowMarkers = [];

		for (
			markerIndex = 0;
			markerIndex < this.tableSelectionColumnMarkers.length;
			markerIndex++
		) {
			app.sectionContainer.removeSection(
				this.tableSelectionColumnMarkers[markerIndex].name,
			);
		}
		this.tableSelectionColumnMarkers = [];

		for (
			markerIndex = 0;
			markerIndex < this.tableSelectionRowMarkers.length;
			markerIndex++
		) {
			app.sectionContainer.removeSection(
				this.tableSelectionRowMarkers[markerIndex].name,
			);
		}
		this.tableSelectionRowMarkers = [];

		for (
			markerIndex = 0;
			markerIndex < this.tableInsertMarkers.length;
			markerIndex++
		) {
			app.sectionContainer.removeSection(
				this.tableInsertMarkers[markerIndex].name,
			);
		}
		this.tableInsertMarkers = [];

		if (this.tableCornerResizeMarker) {
			app.sectionContainer.removeSection(this.tableCornerResizeMarker.name);
			this.tableCornerResizeMarker = null;
		}

		if (this.tableAnchorSection) {
			app.sectionContainer.removeSection(this.tableAnchorSection.name);
			this.tableAnchorSection = null;
		}
	}

	private addResizeMarker(markerType: string, x: number, y: number) {
		const position = new cool.SimplePoint(x, y);

		const index =
			markerType === 'column'
				? this.tableResizeColumnMarkers.length
				: this.tableResizeRowMarkers.length;
		const showSection = !(
			app.map._docLayer._docType === 'presentation' &&
			markerType === 'column' &&
			index === 0
		);

		const marker = new TableResizeMarkerSection(
			'marker-' + String(TableMiddleware.markerid++),
			this.resizeMarkerWidth,
			this.resizeMarkerHeight,
			position,
			markerType === 'column'
				? 'table-column-resize-marker'
				: 'table-row-resize-marker',
			markerType === 'column' ? 'column' : 'row',
			index,
			showSection,
		);

		app.sectionContainer.addSection(marker);

		if (markerType === 'column') this.tableResizeColumnMarkers.push(marker);
		else this.tableResizeRowMarkers.push(marker);

		return marker;
	}

	private addResizeMarkers() {
		if (!this.currentTableData.columns) return;

		const yForColumnMarkers =
			this.currentTableData.rows.tableOffset -
			(this.resizeMarkerHeight + this.selectionMarkerHeight) *
				app.dpiScale *
				app.pixelsToTwips;

		/// The first column marker.
		this.addResizeMarker(
			'column',
			this.currentTableData.columns.tableOffset +
				this.currentTableData.columns.left -
				this.resizeMarkerWidth * 0.5 * app.pixelsToTwips * app.dpiScale,
			yForColumnMarkers,
		);

		// Other column markers.
		for (let i = 0; i < this.currentTableData.columns.entries.length; i++) {
			this.addResizeMarker(
				'column',
				this.currentTableData.columns.tableOffset +
					this.currentTableData.columns.entries[i].position -
					this.resizeMarkerWidth * 0.5 * app.pixelsToTwips * app.dpiScale,
				yForColumnMarkers,
			);
		}

		// The last column marker.
		this.addResizeMarker(
			'column',
			this.currentTableData.columns.tableOffset +
				this.currentTableData.columns.right -
				this.resizeMarkerWidth * 0.5 * app.pixelsToTwips * app.dpiScale,
			yForColumnMarkers,
		);

		const xForRowMarkers =
			this.currentTableData.columns.tableOffset +
			this.currentTableData.columns.left -
			(this.resizeMarkerWidth + this.selectionMarkerWidth) *
				app.dpiScale *
				app.pixelsToTwips;

		// Row markers.
		for (let i = 0; i < this.currentTableData.rows.entries.length; i++) {
			this.addResizeMarker(
				'row',
				xForRowMarkers,
				this.currentTableData.rows.tableOffset +
					this.currentTableData.rows.entries[i].position -
					this.resizeMarkerHeight * 0.5 * app.dpiScale * app.pixelsToTwips,
			);
		}

		// The last row marker.
		this.addResizeMarker(
			'row',
			xForRowMarkers,
			this.currentTableData.rows.tableOffset +
				this.currentTableData.rows.right -
				this.resizeMarkerHeight * 0.5 * app.dpiScale * app.pixelsToTwips,
		);
	}

	private addSelectionMarker(
		markerType: string,
		x: number,
		y: number,
		width: number,
		height: number,
	) {
		const position = new cool.SimplePoint(x, y);

		const marker = new TableSelectMarkerSection(
			'marker-' + String(TableMiddleware.markerid++),
			width,
			height,
			position,
			'table-select-marker ' +
				(markerType === 'column'
					? 'table-select-marker--column'
					: 'table-select-marker--row'),
			markerType,
		);

		app.sectionContainer.addSection(marker);

		if (markerType === 'column') this.tableSelectionColumnMarkers.push(marker);
		else this.tableSelectionRowMarkers.push(marker);

		return marker;
	}

	private addSelectionMarkers() {
		const columnsLength = this.currentTableData.columns.entries.length;
		const rowsLength = this.currentTableData.rows.entries.length;

		const yForColumnSelectionMarkers =
			this.currentTableData.rows.tableOffset -
			this.selectionMarkerHeight * app.dpiScale * app.pixelsToTwips;

		// First column selection marker.
		let firstSelectionMarkerWidth;

		if (columnsLength > 0)
			firstSelectionMarkerWidth =
				((this.currentTableData.columns.entries[0].position -
					this.currentTableData.columns.left) *
					app.twipsToPixels) /
				app.dpiScale;
		else
			firstSelectionMarkerWidth =
				((this.currentTableData.columns.right -
					this.currentTableData.columns.left) *
					app.twipsToPixels) /
				app.dpiScale;

		this.addSelectionMarker(
			'column',
			this.currentTableData.columns.tableOffset +
				this.currentTableData.columns.left,
			yForColumnSelectionMarkers,
			firstSelectionMarkerWidth,
			this.selectionMarkerHeight,
		);

		for (let i = 1; i < columnsLength; i++) {
			this.addSelectionMarker(
				'column',
				this.currentTableData.columns.tableOffset +
					this.currentTableData.columns.entries[i - 1].position,
				yForColumnSelectionMarkers,
				((this.currentTableData.columns.entries[i].position -
					this.currentTableData.columns.entries[i - 1].position) *
					app.twipsToPixels) /
					app.dpiScale,
				this.selectionMarkerHeight,
			);
		}

		// Last column selection marker.
		if (columnsLength > 0) {
			const lastSelectionMarkerWidth =
				((this.currentTableData.columns.right -
					this.currentTableData.columns.entries[columnsLength - 1].position) *
					app.twipsToPixels) /
				app.dpiScale;

			this.addSelectionMarker(
				'column',
				this.currentTableData.columns.tableOffset +
					this.currentTableData.columns.entries[columnsLength - 1].position,
				yForColumnSelectionMarkers,
				lastSelectionMarkerWidth,
				this.selectionMarkerHeight,
			);
		}

		const xForRowlectionMarkers =
			this.currentTableData.columns.tableOffset +
			this.currentTableData.columns.left -
			this.selectionMarkerWidth * app.dpiScale * app.pixelsToTwips;

		let firstSelectionMarkerHeight;

		if (rowsLength > 0)
			firstSelectionMarkerHeight =
				((this.currentTableData.rows.entries[0].position -
					this.currentTableData.rows.left) *
					app.twipsToPixels) /
				app.dpiScale;
		else
			firstSelectionMarkerHeight =
				((this.currentTableData.rows.right - this.currentTableData.rows.left) *
					app.twipsToPixels) /
				app.dpiScale;

		// First row selection marker.
		this.addSelectionMarker(
			'row',
			xForRowlectionMarkers,
			this.currentTableData.rows.tableOffset + this.currentTableData.rows.left,
			this.selectionMarkerWidth,
			firstSelectionMarkerHeight,
		);

		for (let i = 1; i < this.currentTableData.rows.entries.length; i++) {
			this.addSelectionMarker(
				'row',
				xForRowlectionMarkers,
				this.currentTableData.rows.tableOffset +
					this.currentTableData.rows.entries[i - 1].position,
				this.selectionMarkerWidth,
				((this.currentTableData.rows.entries[i].position -
					this.currentTableData.rows.entries[i - 1].position) *
					app.twipsToPixels) /
					app.dpiScale,
			);
		}

		// Last row selection marker.
		if (rowsLength > 0) {
			this.addSelectionMarker(
				'row',
				xForRowlectionMarkers,
				this.currentTableData.rows.tableOffset +
					this.currentTableData.rows.entries[
						this.currentTableData.rows.entries.length - 1
					].position,
				this.selectionMarkerWidth,
				((this.currentTableData.rows.right -
					this.currentTableData.rows.entries[
						this.currentTableData.rows.entries.length - 1
					].position) *
					app.twipsToPixels) /
					app.dpiScale,
			);
		}
	}

	private addMoveMarker() {
		let x = parseInt(this.currentTableData.rectangle.x);
		let y = parseInt(this.currentTableData.rectangle.y);

		const anchorSize = ShapeHandleAnchorSubSection.tableAnchorIconSize;

		x -= Math.round(anchorSize[0] * app.pixelsToTwips);
		y -= Math.round(anchorSize[1] * app.pixelsToTwips);

		this.tableAnchorSection = new ShapeHandleAnchorSubSection(
			null,
			'table-anchor',
			anchorSize,
			new cool.SimplePoint(x, y),
			null,
		);
		app.sectionContainer.addSection(this.tableAnchorSection);
	}

	private updateMoveMarkerVisibility() {
		if (
			this.currentTableData &&
			this.currentTableMarkerJson === this.lastTableMarkerJson &&
			this.currentTableData.rows &&
			this.currentTableData.columns &&
			app.map.getDocType() === 'presentation' &&
			this.currentTableData.rectangle &&
			!this.tableAnchorSection
		)
			this.addMoveMarker();
	}

	public getTableTopY(): number {
		if (!this.currentTableData) return 0;
		else
			return Math.round(
				(this.currentTableData.rows.tableOffset +
					this.currentTableData.rows.left) *
					app.twipsToPixels,
			);
	}

	public getTableBottomY(): number {
		if (!this.currentTableData) return 0;
		else
			return Math.round(
				(this.currentTableData.rows.tableOffset +
					this.currentTableData.rows.right) *
					app.twipsToPixels,
			);
	}

	public getTableLeftX(): number {
		if (!this.currentTableData) return 0;
		else
			return Math.round(
				(this.currentTableData.columns.tableOffset +
					this.currentTableData.columns.left) *
					app.twipsToPixels,
			);
	}

	public getTableRightX(): number {
		if (!this.currentTableData) return 0;
		else
			return Math.round(
				(this.currentTableData.columns.tableOffset +
					this.currentTableData.columns.right) *
					app.twipsToPixels,
			);
	}

	private addInsertMarkers() {
		// Column insert marker.
		const insertColumnX =
			this.getTableRightX() * app.pixelsToTwips +
			this.addButtonOffset * app.dpiScale * app.pixelsToTwips;
		const insertColumnY =
			this.currentTableData.rows.tableOffset + this.currentTableData.rows.left;
		const insertColumnWidth = this.addButtonWidth;
		const insertColumnHeight =
			((this.currentTableData.rows.right - this.currentTableData.rows.left) *
				app.twipsToPixels) /
			app.dpiScale;

		const insertColumnMarkerSection = new TableInsertMarkerSection(
			'column',
			new cool.SimplePoint(insertColumnX, insertColumnY),
		);
		insertColumnMarkerSection.setMarkerSize(
			insertColumnWidth,
			insertColumnHeight,
		);
		app.sectionContainer.addSection(insertColumnMarkerSection);

		this.tableInsertMarkers.push(insertColumnMarkerSection);

		// Row insert marker.
		const insertRowX =
			this.currentTableData.columns.tableOffset +
			this.currentTableData.columns.left;
		const insertRowY =
			this.getTableBottomY() * app.pixelsToTwips +
			this.addButtonOffset * app.dpiScale * app.pixelsToTwips;
		const insertRowWidth =
			((this.currentTableData.columns.right -
				this.currentTableData.columns.left) *
				app.twipsToPixels) /
			app.dpiScale;
		const insertRowHeight = this.addButtonHeight;

		const insertRowMarkerSection = new TableInsertMarkerSection(
			'row',
			new cool.SimplePoint(insertRowX, insertRowY),
		);
		insertRowMarkerSection.setMarkerSize(insertRowWidth, insertRowHeight);
		app.sectionContainer.addSection(insertRowMarkerSection);

		this.tableInsertMarkers.push(insertRowMarkerSection);
	}

	/*
		The button between the two insert markers, in the lower right corner. A table that is a
		shape of its own, as in Impress, comes with its rectangle and can be resized as a whole.
	*/
	private addCornerResizeMarker() {
		if (!this.currentTableData.rectangle) return;

		const offset = this.addButtonOffset * app.dpiScale * app.pixelsToTwips;
		const x = this.getTableRightX() * app.pixelsToTwips + offset;
		const y = this.getTableBottomY() * app.pixelsToTwips + offset;

		this.tableCornerResizeMarker = new TableCornerResizeMarkerSection(
			new cool.SimplePoint(x, y),
			this.addButtonWidth,
			this.addButtonOffset,
		);
		app.sectionContainer.addSection(this.tableCornerResizeMarker);
	}

	private ensureDataIntegrity() {
		// Core Impress sends the integer data in string type.
		this.currentTableData.columns.tableOffset = parseInt(
			this.currentTableData.columns.tableOffset,
		);
		this.currentTableData.rows.tableOffset = parseInt(
			this.currentTableData.rows.tableOffset,
		);

		this.currentTableData.columns.left = parseInt(
			this.currentTableData.columns.left,
		);
		this.currentTableData.rows.left = parseInt(this.currentTableData.rows.left);

		this.currentTableData.columns.right = parseInt(
			this.currentTableData.columns.right,
		);
		this.currentTableData.rows.right = parseInt(
			this.currentTableData.rows.right,
		);

		for (let i = 0; i < this.currentTableData.columns.entries.length; i++) {
			this.currentTableData.columns.entries[i].max = parseInt(
				this.currentTableData.columns.entries[i].max,
			);
			this.currentTableData.columns.entries[i].min = parseInt(
				this.currentTableData.columns.entries[i].min,
			);
			this.currentTableData.columns.entries[i].position = parseInt(
				this.currentTableData.columns.entries[i].position,
			);
		}

		for (let i = 0; i < this.currentTableData.rows.entries.length; i++) {
			this.currentTableData.rows.entries[i].max = parseInt(
				this.currentTableData.rows.entries[i].max,
			);
			this.currentTableData.rows.entries[i].min = parseInt(
				this.currentTableData.rows.entries[i].min,
			);
			this.currentTableData.rows.entries[i].position = parseInt(
				this.currentTableData.rows.entries[i].position,
			);
		}

		if (this.currentTableData.rectangle) {
			this.currentTableData.rectangle.width = parseInt(
				this.currentTableData.rectangle.width,
			);
			this.currentTableData.rectangle.height = parseInt(
				this.currentTableData.rectangle.height,
			);
			this.currentTableData.rectangle.x = parseInt(
				this.currentTableData.rectangle.x,
			);
			this.currentTableData.rectangle.y = parseInt(
				this.currentTableData.rectangle.y,
			);
		}
	}

	private updateTableMarkers() {
		if (this.currentTableData === undefined) return; // not writer, no table selected yet etc.

		if (this.lastTableMarkerJson === this.currentTableMarkerJson) {
			this.updateMoveMarkerVisibility();
			return; // identical table setup.
		}

		this.lastTableMarkerJson = this.currentTableMarkerJson;

		this.clearTableMarkers();

		if (this.currentTableData && this.currentTableData.columns) {
			this.ensureDataIntegrity();
			this.updateMoveMarkerVisibility();
			this.addResizeMarkers();
			this.addSelectionMarkers();
			this.addInsertMarkers();
			this.addCornerResizeMarker();
		}
	}
}
