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
	Base class for the sub sections (handles) of ShapeHandlesSection.
	Holds the field declarations and the common setup shared by the
	scaling, custom and rotation handle sub sections.
*/

abstract class ShapeHandleSubSection extends CanvasSectionObject {
	processingOrder: number =
		app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number =
		app.CSections.DefaultForDocumentObjects.drawingOrder + 1; // Handle events before the parent section.
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	documentObject: boolean = true;

	constructor(
		parentHandlerSection: ShapeHandlesSection,
		sectionName: string,
		size: number[],
		documentPosition: cool.SimplePoint,
		ownInfo: any,
	) {
		super(sectionName);

		this.size = size;

		this.sectionProperties.position = documentPosition.clone();
		this.sectionProperties.parentHandlerSection = parentHandlerSection;
		this.sectionProperties.ownInfo = ownInfo;
	}

	onInitialize(): void {
		this.setPosition(
			this.sectionProperties.position.pX,
			this.sectionProperties.position.pY,
		);
	}

	onTextCursorVisibility(event: any): void {
		if (event.detail.visible) {
			this.setShowSection(false);
			this.interactable = false;
		} else {
			this.setShowSection(true);
			this.interactable = true;
		}
	}
}
