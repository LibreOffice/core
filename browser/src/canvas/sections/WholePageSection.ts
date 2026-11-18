/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
	A section that lies over the whole page wherever the document is shown, and takes the mouse
	there. It knows the place on the page of a point on it.
*/
abstract class WholePageSection extends CanvasSectionObject {
	documentObject: boolean = true;

	constructor(name: string) {
		super(name);
		this.size = app.sectionContainer.getDocumentAnchorSection().size;
	}

	/// The section covers the page wherever the document is shown.
	isHit(point: number[]): boolean {
		const page = this.containerObject.getDocumentAnchorSection();
		return (
			point[0] >= page.myTopLeft[0] &&
			point[0] <= page.myTopLeft[0] + page.size[0] &&
			point[1] >= page.myTopLeft[1] &&
			point[1] <= page.myTopLeft[1] + page.size[1]
		);
	}

	// The section is added while the document is on screen, so it places itself right away.
	onInitialize(): void {
		this.onNewDocumentTopLeft();
	}

	onNewDocumentTopLeft(): void {
		Util.ensureValue(app.activeDocument);
		const viewed = app.activeDocument.activeLayout.viewedRectangle;
		this.setPosition(viewed.pX1, viewed.pY1);
	}

	/// The place on the page, in twips, of a point on the section.
	protected inTheDocument(point: cool.SimplePoint): cool.Point {
		const inDocument = point.clone();
		inDocument.pX += this.position[0];
		inDocument.pY += this.position[1];
		return new cool.Point(inDocument.x, inDocument.y);
	}
}
