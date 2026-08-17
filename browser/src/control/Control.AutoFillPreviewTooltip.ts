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
 * Control.AutoFillPreviewTooltip - class for tooltip of cell previews during auto fill
 */

/* global app */

class AutoFillPreviewTooltip extends AutoCompletePopup {
	usageText: string;
	declare newPopupData: PopupData;

	constructor(map: any) {
		super('autoFillPreviewTooltip', map);
	}

	getPopupEntries(ev: FireEvent): Array<TreeEntryJSON> {
		return [];
	}

	onAdd() {
		this.newPopupData.isAutoFillPreviewTooltip = true;
		this.newPopupData.noOverlay = true;
		this.newPopupData.id = 'autoFillPreviewTooltip';

		this.map.on(
			'openautofillpreviewpopup',
			this.openAutoFillPreviewPopup,
			this,
		);
		this.map.on(
			'closeautofillpreviewpopup',
			this.closeAutoFillPreviewPopup,
			this,
		);
	}

	getSimpleTextJSON(cellValue: string): TextWidget {
		return {
			id: this.popupId + 'fixedtext',
			type: 'fixedtext',
			text: cellValue,
			enabled: false,
		} as TextWidget;
	}

	openAutoFillPreviewPopup(ev: FireEvent): void {
		// calculate the popup position
		var cellRange = this.map._docLayer._parseCellRange(
			JSON.stringify(ev.data.celladdress),
		);
		ev.data.celladdress = this.map._docLayer
			._cellRangeToTwipRect(cellRange)
			.toRectangle();

		ev.data.celladdress = new cool.SimplePoint(
			parseInt(ev.data.celladdress[0]),
			parseInt(ev.data.celladdress[1]),
		);

		// documentToViewX/Y already account for a frozen row/column: a cell
		// in the frozen pane keeps its position regardless of how far the
		// rest of the sheet has scrolled. Subtracting the raw scroll offset
		// unconditionally, as this used to do, moved the tooltip for a cell
		// in the frozen pane as if it had scrolled along with the rest.
		const layout = app.activeDocument.activeLayout;
		const viewX = layout.documentToViewX(ev.data.celladdress);
		const viewY = layout.documentToViewY(ev.data.celladdress);
		ev.data.celladdress.pX = viewX;
		ev.data.celladdress.pY = viewY;

		const entry = ev.data.text;
		let data: PopupData;

		if (entry.length > 0) {
			this.closeAutoFillPreviewPopup();
			const control = this.getSimpleTextJSON(entry);
			if (window.L.DomUtil.get(this.popupId + 'fixedtext')) {
				data = this.getPopupJSON(control, {
					x: ev.data.celladdress.cX,
					y: ev.data.celladdress.cY,
				});
				this.sendUpdate(data);
				return;
			}

			if (window.L.DomUtil.get(this.popupId)) this.closeAutoFillPreviewPopup();
			data = Object.assign({}, this.newPopupData);
			data.children[0].children[0] = control;

			// add position
			data.posx = ev.data.celladdress.cX;
			data.posy = ev.data.celladdress.cY;
			this.sendJSON(data);
		}
	}

	closeAutoFillPreviewPopup(): void {
		super.closePopup();
	}
}
