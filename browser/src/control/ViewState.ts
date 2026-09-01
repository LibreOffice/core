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
 * ViewState puts a page back where the view before it stood, from the position the
 * server sends in a viewposition message after the status that follows the load.
 */

class ViewState {
	private map: any;

	constructor(map: any) {
		this.map = map;
	}

	public onViewPositionMsg(textMsg: string): void {
		const values: { [key: string]: number } = {};
		for (const token of textMsg.substring('viewposition:'.length).split(' ')) {
			const at = token.indexOf('=');
			if (at <= 0) continue;
			const value = parseInt(token.substring(at + 1), 10);
			if (!isNaN(value)) values[token.substring(0, at)] = value;
		}

		window.app.console.log(
			'ViewState: restoring edit mode ' +
				values.editmode +
				', zoom ' +
				values.zoompercent +
				', scroll ' +
				values.x +
				',' +
				values.y,
		);

		// A document the reader was editing comes back editable. One that is
		// read-only in its own right stays so.
		if (values.editmode && !this.map.isEditMode() && !app.isReadOnly())
			this.map.setPermission('edit');

		// The zoom goes on first: the scroll offset converts from twips to pixels
		// at that zoom.
		if (values.zoompercent > 0 && app.activeDocument) {
			const zoom = app.activeDocument.getZoomIndex(values.zoompercent);
			if (typeof zoom === 'number' && zoom !== this.map.getZoom())
				this.map.setZoom(zoom);
		}

		if (typeof values.x === 'number' && typeof values.y === 'number') {
			const area = new cool.SimpleRectangle(
				values.x,
				values.y,
				values.width || 0,
				values.height || 0,
			);
			this.scrollTo(area.pX1, area.pY1);
		}
	}

	// The offset goes on through the same two fields a reconnect uses, so a later
	// cursor update reaffirms it instead of overriding it.
	private scrollTo(pixelX: number, pixelY: number): void {
		const layout =
			app.activeDocument && app.activeDocument.activeLayout
				? app.activeDocument.activeLayout
				: null;
		if (!layout || layout.type === 'ViewLayoutMultiPage') return;

		const docLayer = this.map._docLayer;
		if (!docLayer) return;

		docLayer._savedScrollPos = { pX1: pixelX, pY1: pixelY };
		docLayer._restoringViewScroll = true;
		layout.scrollTo(pixelX, pixelY);
	}
}
