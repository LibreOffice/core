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
	This is for user views.
	Supposed to hold view data like selected text, viewID, view color, username etc.

	FOR NOW: This only holds the selected text data.
*/
class DocumentViewBase {
	public readonly viewID: number;
	private color: string;
	private _selectionSection: TextSelectionSection;
	public hasTextSelection: boolean = false;

	constructor(viewID: number) {
		this.viewID = viewID;
		this.color = app.LOUtil.rgbToHex(app.LOUtil.getViewIdColor(this.viewID));
		this._selectionSection = new TextSelectionSection(
			String(this.viewID) + '-text-selections',
			0,
			0,
			this.color,
		);
		// Remove any stale section with the same name so addSection succeeds.
		if (app.sectionContainer.doesSectionExist(this._selectionSection.name))
			app.sectionContainer.removeSection(this._selectionSection.name);
		app.sectionContainer.addSection(this._selectionSection);
		this._selectionSection.setShowSection(false);
	}

	public get selectionSection() {
		return this._selectionSection;
	}

	private getSeparatePolygonsFromGroupOfRectangles(
		rectangles: Array<number[]>,
	) {
		const groups: any = [];
		groups.length = 0;

		for (let i = rectangles.length - 1; i > -1; i--) {
			if (rectangles[i][3] === 0 || rectangles[i][2] === 0)
				rectangles.splice(i, 1); // Remove the 0-width or 0-height rectangle from the list.
		}

		// Sort the rectangles.
		rectangles.sort((a, b) => {
			return a[1] > b[1] ? 1 : a[1] === b[1] ? 0 : -1;
		});

		while (rectangles.length > 0) {
			if (groups.length === 0) {
				const rectangle = rectangles.splice(0, 1)[0];
				groups.push([rectangle]);
			} else {
				const group = groups[groups.length - 1];
				const rectangle = group[group.length - 1];

				// Max y of current group. We will check if next rectangle can join this group.
				// If it can not, then it will start a new, separate polygon.
				const yMaxOfGroup = rectangle[1] + rectangle[3];

				const next = rectangles[0];
				const xOverlap = !(
					rectangle[0] + rectangle[2] <= next[0] ||
					next[0] + next[2] <= rectangle[0]
				);

				if (Math.abs(yMaxOfGroup - next[1]) < 5 && xOverlap)
					// 5 is the twips tolerance we pick.
					group.push(rectangles.splice(0, 1)[0]);
				else groups.push([rectangles.splice(0, 1)[0]]);
			}
		}

		return groups;
	}

	public setColor(color: string) {
		this.color = color;
		this.selectionSection.color = this.color;
	}

	public clearTextSelection() {
		this.updateSelectionRawData(-1, -1, []);
	}

	public updateSelectionRawData(
		mode: number,
		part: number,
		rectangles: Array<Array<number>>,
	) {
		Util.ensureValue(app.activeDocument);

		if (rectangles.length > 0) {
			this.hasTextSelection = true;

			const rawPolygons =
				this.getSeparatePolygonsFromGroupOfRectangles(rectangles);
			let minX = Number.POSITIVE_INFINITY;
			let maxX = Number.NEGATIVE_INFINITY;
			let minY = Number.POSITIVE_INFINITY;
			let maxY = Number.NEGATIVE_INFINITY;

			const polygons: Array<Array<cool.SimplePoint>> = [];

			for (let i = 0; i < rawPolygons.length; i++) {
				const points: number[] = cool.rectanglesToPolygon(rawPolygons[i]);

				const simplePoints: cool.SimplePoint[] = [];

				for (let j = 0; j < points.length - 1; j += 2) {
					simplePoints.push(
						new cool.SimplePoint(points[j], points[j + 1], part),
					);
					if (points[j] < minX) minX = points[j];
					if (points[j] > maxX) maxX = points[j];
					if (points[j + 1] < minY) minY = points[j + 1];
					if (points[j + 1] > maxY) maxY = points[j + 1];
				}

				polygons.push(simplePoints);
			}

			const position = [
				Math.round(minX * app.twipsToPixels),
				Math.round(minY * app.twipsToPixels),
			];
			const size = [
				Math.round((maxX - minX) * app.twipsToPixels),
				Math.round((maxY - minY) * app.twipsToPixels),
			];

			this.selectionSection.setSelectionInfo(mode, part, polygons);
			this.selectionSection.size = size;

			this.selectionSection.setPosition(position[0], position[1]);

			if (
				app.activeDocument.isModeActive(mode) &&
				part === app.map._docLayer._selectedPart
			)
				this.selectionSection.setShowSection(true);
			else this.selectionSection.setShowSection(false);
		} else if (rectangles.length === 0) {
			this.hasTextSelection = false;
			this.selectionSection.setSelectionInfo(mode, part, []);
			this.selectionSection.setShowSection(false);
		}
	}
}
