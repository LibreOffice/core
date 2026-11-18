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
    This class's purpose is to catch the selection rectangle's current properties:
        * Core side sends ".uno:Position" state change for mouse moves (whether there is a selection rectangle or not).
        * When user presses the mouse button and moves the mouse, core side sends the ".uno:Context=Mark objects" state change.
        * Core side sends ".uno:Size" as user moves the mouse (while holding the pressed mouse button).

    * Since mouse position is sent regardless of the selection, we will keep track of last mouse position.
        * No need to do anything special for this. State map variable should have it. Just fetch it as soon as we get "Mark objects" context.
    * We will listen to ".uno:Context=Mark objects" state change.
    * We will activate selection section when there is a selection.
    * Since this class has the data, selection section should be only drawing the rectangle.
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
