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
	The caret that marks where a dragged text selection would land. The document reports that
	place as a cursor with a drop position. The caret shows only while the drag runs.
*/
class DropCaretSection extends HTMLObjectSection {
	static readonly sectionName = 'drop caret';

	// Width of the caret bar, in CSS pixels.
	static readonly caretWidth = 2;

	constructor() {
		super(
			DropCaretSection.sectionName,
			DropCaretSection.caretWidth,
			0,
			new cool.SimplePoint(0, 0),
			'text-drop-caret',
			false,
		);
	}

	// Shows the caret at the given cursor rectangle, in twips.
	public static showAt(rectangle: cool.SimpleRectangle): void {
		let section: DropCaretSection;
		if (app.sectionContainer.doesSectionExist(DropCaretSection.sectionName)) {
			section = app.sectionContainer.getSectionWithName(
				DropCaretSection.sectionName,
			) as DropCaretSection;
		} else {
			section = new DropCaretSection();
			app.sectionContainer.addSection(section);
		}

		section.size = [
			DropCaretSection.caretWidth * app.dpiScale,
			rectangle.pHeight,
		];
		section.getHTMLObject().style.height =
			rectangle.pHeight / app.dpiScale + 'px';
		section.setPosition(rectangle.pX1, rectangle.pY1);
		section.setShowSection(true);

		// The object is hidden while it lies outside the visible area.
		section.onNewDocumentTopLeft();
		app.sectionContainer.requestReDraw();
	}

	public static hide(): void {
		if (!app.sectionContainer.doesSectionExist(DropCaretSection.sectionName))
			return;

		const section = app.sectionContainer.getSectionWithName(
			DropCaretSection.sectionName,
		) as DropCaretSection;
		if (!section.showSection) return;

		section.setShowSection(false);
		app.sectionContainer.requestReDraw();
	}
}
