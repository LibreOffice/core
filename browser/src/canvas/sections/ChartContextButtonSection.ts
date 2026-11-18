/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class ChartContextButtonSection extends GenericButtonSection {
	static readonly namePrefix: string = 'ChartContextButton_';
	static readonly classNames: string[] = [
		'general-select-theme',
		'chart-save-to-new-theme',
	];
	static readonly unoCommands: string[] = [
		'uno .uno:SelectTheme',
		'uno .uno:ChartSaveToNewTheme',
	];
	// Predefined sizes in pixel
	static readonly sizeButtons: number = 32;
	static readonly sizeSpaceBetweenButtons: number = 5;

	constructor(buttonType: number) {
		super(
			ChartContextButtonSection.namePrefix + buttonType,
			ChartContextButtonSection.sizeButtons,
			ChartContextButtonSection.sizeButtons,
			ChartContextButtonSection.classNames[buttonType],
			ChartContextButtonSection.unoCommands[buttonType],
		);

		this.sectionProperties.buttonType = buttonType;
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
					ChartContextButtonSection.sizeSpaceBetweenButtons,
			),
			Math.round(
				GraphicSelection.rectangle.y1 * app.twipsToPixels +
					this.sectionProperties.buttonType *
						(ChartContextButtonSection.sizeButtons +
							ChartContextButtonSection.sizeSpaceBetweenButtons),
			),
		];
	}

	showChartContextToolbar(): void {
		this.setShowSection(true);
	}
}
