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

const sectionName = 'TileInvalidationRectangle';

class InvalidationRectangleSection extends CanvasSectionObject {
	/*
		We don't want visibility issues.
		Since there will be more than one rectangles in this section, position property (thus document section) is not useful anymore.
	*/
	windowSection: boolean = true;
	showSection: boolean = true;
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	drawingOrder: number = app.CSections.DefaultForDocumentObjects.drawingOrder;
	processingOrder: number =
		app.CSections.DefaultForDocumentObjects.processingOrder;
	interactable: boolean = false;

	constructor() {
		super(sectionName);
		this.sectionProperties.rectangleList = [];
	}

	addRectangle(x: number, y: number, width: number, height: number) {
		const rectangleList: Array<any> = this.sectionProperties
			.rectangleList as Array<any>;

		if (rectangleList.length === 5) rectangleList.pop();

		rectangleList.unshift([x, y, width, height]);
	}

	onDraw(
		frameCount?: number,
		elapsedTime?: number,
		subsetBounds?: Bounds,
	): void {
		const rectangleList: Array<any> = this.sectionProperties
			.rectangleList as Array<any>;
		this.context.strokeStyle = 'red';

		const anchor: number[] = this.containerObject.getDocumentAnchor();
		Util.ensureValue(app.activeDocument);
		const xDiff =
			anchor[0] - app.activeDocument.activeLayout.viewedRectangle.pX1;
		const yDiff =
			anchor[1] - app.activeDocument.activeLayout.viewedRectangle.pY1;

		for (let i = 0; i < rectangleList.length; i++) {
			this.context.globalAlpha = 1 - 0.15 * i;
			this.context.strokeRect(
				xDiff + rectangleList[i][0],
				yDiff + rectangleList[i][1],
				rectangleList[i][2],
				rectangleList[i][3],
			);
		}
		this.context.globalAlpha = 1;
	}

	checkDeletion() {
		if (this.sectionProperties.rectangleList.length > 0) {
			this.sectionProperties.rectangleList.pop();
			setTimeout(() => {
				this.checkDeletion();
			}, 1000);
		} else {
			app.layoutingService.appendLayoutingTask(() => {
				app.sectionContainer.removeSection(this.name);
			});
		}
	}

	private static getSection(): InvalidationRectangleSection {
		let section: InvalidationRectangleSection;
		if (app.sectionContainer.doesSectionExist(sectionName))
			section = app.sectionContainer.getSectionWithName(
				sectionName,
			) as InvalidationRectangleSection;
		else {
			section = new InvalidationRectangleSection();
			app.sectionContainer.addSection(section);
			setTimeout(() => {
				section.checkDeletion();
			}, 2000); // Start the cycle.
		}

		return section;
	}

	public static setRectangle(
		x: number,
		y: number,
		width: number,
		height: number,
	) {
		const section = this.getSection();
		section.addRectangle(x, y, width, height);
	}
}
