/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class DiagramButtonSection extends GenericButtonSection {
	static readonly namePrefix: string = 'DiagramContextButton_';
	static readonly className: string = 'general-select-theme';
	static readonly unoCommand: string = 'uno .uno:EditDiagram';
	// Predefined sizes in pixel
	static readonly sizeButtons: number = 32;
	static readonly sizeSpaceBetweenButtons: number = 5;
	// fixed size pixels for the frame distance
	halfWidthPixels: number = 0;

	constructor(_halfWidthPixels: number) {
		super(
			DiagramButtonSection.namePrefix,
			DiagramButtonSection.sizeButtons,
			DiagramButtonSection.sizeButtons,
			DiagramButtonSection.className,
			DiagramButtonSection.unoCommand,
		);

		this.halfWidthPixels = _halfWidthPixels;
		this.sectionProperties.lastInputEvent = null;

		// force update Position
		this.updatePosition();
	}

	calculatePositionPixel(): Array<number> {
		Util.ensureValue(GraphicSelection.rectangle);
		// calculate & return top-left position
		return [
			Math.round(
				GraphicSelection.rectangle.x2 * app.twipsToPixels +
					this.halfWidthPixels +
					DiagramButtonSection.sizeSpaceBetweenButtons,
			),
			Math.round(GraphicSelection.rectangle.y1 * app.twipsToPixels),
		];
	}
}
