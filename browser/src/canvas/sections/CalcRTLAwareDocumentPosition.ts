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

// Position helper for Calc document-object sections whose hit area must
// follow the tile section's width rather than the full canvas width.
//
// The base CanvasSectionObject.setPosition mirrors x around the full
// canvas width when isCalcRTL() is true, but the grid is mirrored
// around the tile section's width (which excludes the row header).
// Compute the canvas position from the LTR document pixel and, in RTL,
// mirror it around the tile section so myTopLeft lands on the visible
// cell. This keeps the section's hit area aligned with what the user
// sees on screen.
function setCalcRTLAwareDocumentObjectPosition(
	section: CanvasSectionObject,
	x: number,
	y: number,
): void {
	if (section.documentObject !== true || !section.containerObject) return;

	x = Math.round(x);
	y = Math.round(y);

	section.position[0] = x;
	section.position[1] = y;
	section.documentPosition = cool.SimplePoint.fromCorePixels([x, y]);

	Util.ensureValue(app.activeDocument);
	const positionAddition =
		app.activeDocument.activeLayout.viewedRectangle.clone();
	const documentAnchor = section.containerObject.getDocumentAnchor();

	if (app.isXOrdinateInFrozenPane(x)) positionAddition.pX1 = 0;
	if (app.isYOrdinateInFrozenPane(y)) positionAddition.pY1 = 0;

	let canvasX = documentAnchor[0] + x - positionAddition.pX1;
	if (app.calc.isRTL()) {
		const tileWidth =
			section.containerObject.getDocumentAnchorSection().size[0];
		canvasX = 2 * documentAnchor[0] + tileWidth - canvasX - section.size[0];
	}
	section.myTopLeft[0] = canvasX;
	section.myTopLeft[1] = documentAnchor[1] + y - positionAddition.pY1;

	const isVisible = section.containerObject.isDocumentObjectVisible(section);
	if (isVisible !== section.isVisible) {
		section.isVisible = isVisible;
		section.onDocumentObjectVisibilityChange();
	}

	if (section.containerObject.testing)
		section.containerObject.createUpdateSingleDivElement(section);
}
