// @ts-strict-ignore
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
	Calc only.
*/

/* global $ app */

namespace cool {

export class CornerHeader extends CanvasSectionObject {
	anchor: any = [[app.CSections.ColumnGroup.name, 'bottom', 'top'], [app.CSections.RowGroup.name, 'right', 'left']];
	size: number[] = [48 * app.dpiScale, 19 * app.dpiScale]; // These values are static.
	processingOrder: number = app.CSections.CornerHeader.processingOrder;
	drawingOrder: number = app.CSections.CornerHeader.drawingOrder;
	zIndex: number = app.CSections.CornerHeader.zIndex;
	sectionProperties: any = { cursor: 'pointer' }

	_map: any;
	_textColor: string;

	constructor() { super(app.CSections.CornerHeader.name); }

	onInitialize():void {
		this._map = window.L.Map.THIS;

		this._map.on('darkmodechanged', this._initCornerHeaderStyle, this);
		this._initCornerHeaderStyle();
	}

	onClick(): void {
		selectAllAndRestoreFocus(this._map);
	}

	onMouseEnter(): void {
		this.containerObject.getCanvasStyle().cursor = this.sectionProperties.cursor;
	}

	onMouseLeave(): void {
		this.containerObject.getCanvasStyle().cursor = 'default';
	}

	_initCornerHeaderStyle(): void {
		const baseElem = document.getElementsByTagName('body')[0];
		const elem = window.L.DomUtil.create('div', 'spreadsheet-header-row', baseElem);
		this._textColor = window.L.DomUtil.getStyle(elem, 'color');
		this.backgroundColor = window.L.DomUtil.getStyle(elem, 'background-color'); // This is a section property.
		this.borderColor = window.L.DomUtil.getStyle(elem, 'border-top-color'); // This is a section property.
		window.L.DomUtil.remove(elem);
	}
}

}
