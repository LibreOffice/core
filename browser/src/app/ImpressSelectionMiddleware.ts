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
    On Impress, when user presses mouse button and starts dragging, they start a selection.
    Which objects to select are determined by a rectangle. The objects inside the rectangle are selected.
    When user presses the mouse button and moves the mouse, they start to draw a selection rectangle.
    When they release the mouse button, rectangle is drawn.
    Core side sends ".uno:MarkingObjects=true" while the mouse draws a frame to mark objects, and
    ".uno:MarkingObjects=false" when the frame is gone. This class switches the selection section
    on while the last state is true.
*/

class ImpressSelectionMiddleware {
	private selectionRectangleSection: SelectionRectangle | null = null;

	constructor() {
		this.addSectionToCanvas();
	}

	private addSectionToCanvas() {
		if (
			app.sectionContainer &&
			app.sectionContainer.getDocumentAnchorSection() !== null
		) {
			this.selectionRectangleSection = new SelectionRectangle();
			app.sectionContainer.addSection(
				this.selectionRectangleSection as SelectionRectangle,
			);
		} else {
			setTimeout(() => {
				this.addSectionToCanvas();
			}, 200);
		}
	}

	public activate() {
		if (this.selectionRectangleSection)
			this.selectionRectangleSection.setActive(true);
	}

	public deactivate() {
		if (this.selectionRectangleSection)
			this.selectionRectangleSection.setActive(false);
	}
}
