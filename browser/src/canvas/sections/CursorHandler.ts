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

// This is used for showing mobile cursor handler. User can grab and move this section's HTML object to move the text cursor.

class CursorHandler extends HTMLObjectSection {
	public static objectWidth = 30; // cursor-handler CSS width and height.
	public static objectHeight = 44;

	constructor() {
		super(
			app.CSections.CursorHandler.name,
			CursorHandler.objectWidth,
			CursorHandler.objectHeight,
			new cool.SimplePoint(0, 0),
			'cursor-handler',
			false,
		);

		this.sectionProperties.lastPosition = null;
	}

	onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name
		) {
			this.stopPropagating();
			e.stopPropagation();

			if (!this.sectionProperties.lastPosition)
				this.sectionProperties.lastPosition = this.position.slice();
			else {
				this.setPosition(
					this.sectionProperties.lastPosition[0] + dragDistance[0],
					this.sectionProperties.lastPosition[1] + dragDistance[1],
				);
			}
		}
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (
			this.containerObject.isDraggingSomething() &&
			this.containerObject.targetSection === this.name
		) {
			app.map._docLayer._postMouseEvent(
				'buttondown',
				Math.round(this.position[0] * app.pixelsToTwips),
				Math.round(this.position[1] * app.pixelsToTwips),
				1,
				1,
				0,
			);
			app.map._docLayer._postMouseEvent(
				'buttonup',
				Math.round(this.position[0] * app.pixelsToTwips),
				Math.round(this.position[1] * app.pixelsToTwips),
				1,
				1,
				0,
			);

			// Put the handle under the caret now. The cursor update from core moves it again
			// when the caret moved.
			this.moveUnderCaret();
		}
		this.sectionProperties.lastPosition = null;
	}

	// Place the handle directly below the text cursor.
	public moveUnderCaret(): void {
		Util.ensureValue(app.file.textCursor.rectangle);
		this.setPosition(
			app.file.textCursor.rectangle.pX1,
			app.file.textCursor.rectangle.pY2,
		);
	}

	setOpacity(value: number) {
		this.getHTMLObject().style.opacity = value;
	}
}
