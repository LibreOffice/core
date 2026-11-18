/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class GenericButtonSection extends HTMLObjectSection {
	_lastZoom: number = 0;
	_unoCommand: string = '';

	constructor(
		sectionName: string,
		width: number,
		height: number,
		extraClass: string,
		unoCommand: string,
	) {
		super(
			sectionName,
			width,
			height,
			new cool.SimplePoint(0, 0),
			extraClass,
			true,
		);

		this._unoCommand = unoCommand;
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		this.getHTMLObject()?.classList.add('hovered');
	}

	public onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		this.getHTMLObject()?.classList.remove('hovered');
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopEvents(e);
	}

	public onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopEvents(e);
	}

	private stopEvents(e: MouseEvent) {
		this.stopPropagating();

		// We shouldn't need below 2 when we remove map element.
		e.preventDefault();
		e.stopImmediatePropagation();
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		app.socket.sendMessage(this._unoCommand);
	}

	calculatePositionPixel(): Array<number> {
		// if not overloaded return default top-left position
		return [0, 0];
	}

	updatePosition(): void {
		const origZoom = app.map.getZoom();
		// Position goes wrong when zoom change so update it when zoom changed.
		if (this._lastZoom != origZoom) {
			this._lastZoom = origZoom;
			const newPosition = this.calculatePositionPixel();
			this.setPosition(newPosition[0], newPosition[1]);
		}
	}

	// catch zoom, and scroll events ..
	adjustHTMLObjectPosition() {
		this.updatePosition();
		super.adjustHTMLObjectPosition();
	}

	public forceNextReposition(): void {
		this._lastZoom = 0;
	}
}
