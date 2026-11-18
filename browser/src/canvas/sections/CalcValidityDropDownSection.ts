/* global Proxy _ */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

class CalcValidityDropDown extends HTMLObjectSection {
	public static dropDownArrowSize = 16; // Size of the validity drop-down arrow in CSS pixels.

	zIndex: number = app.CSections.CalcValidityDropDown.zIndex;

	constructor (documentPosition: cool.SimplePoint, visible: boolean = true) {
		super(app.CSections.CalcValidityDropDown.name, CalcValidityDropDown.dropDownArrowSize, CalcValidityDropDown.dropDownArrowSize, documentPosition, 'spreadsheet-drop-down-marker', visible);

		this.sectionProperties.mouseEntered = false;
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.sectionProperties.mouseEntered = true;
	}

	public onMouseLeave() {
		this.sectionProperties.mouseEntered = false;
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		e.stopPropagation();
		e.preventDefault();
		this.stopPropagating();

		const _validatedCellAddress = (app.map._docLayer as { _validatedCellAddress?: null | cool.SimplePoint })._validatedCellAddress;
		if (_validatedCellAddress && app.calc.cellCursorVisible && _validatedCellAddress.equals(app.calc.cellAddress?.toArray() ?? [])) {
			app.map.sendUnoCommand('.uno:DataSelect');
		}
	}
}
