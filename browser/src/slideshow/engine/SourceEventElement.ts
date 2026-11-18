// @ts-strict-ignore
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

class SourceEventElement implements MouseClickHandler {
	private sId: string;
	private _canvas: HTMLCanvasElement;
	private bounds: DOMRect;
	private aSlideShow: SlideShowHandler = null;
	private aEventMultiplexer: EventMultiplexer;
	private bClickHandled = false;
	private bIsPointerOver = false;

	constructor(
		sId: string,
		aCanvas: HTMLCanvasElement,
		bounds: DOMRect,
		priority: number,
		aSlideShow: SlideShowHandler,
		aEventMultiplexer: EventMultiplexer,
	) {
		this.sId = sId;
		this._canvas = aCanvas;
		this.bounds = bounds;
		this.aSlideShow = aSlideShow;

		this.aEventMultiplexer = aEventMultiplexer;

		this.aEventMultiplexer.registerMouseClickHandler(this, 1000 + priority);
	}

	getId() {
		return this.sId;
	}

	onMouseMove(cursorX: number, cursorY: number): boolean {
		const bIsOver = this.isOver(cursorX, cursorY);
		if (bIsOver !== this.bIsPointerOver) {
			if (bIsOver) this.onMouseEnter();
			else this.onMouseLeave();
		}
		return bIsOver;
	}

	isOver(x: number, y: number): boolean {
		// console.debug('SourceEventElement.isOver: x: ' + x + ', y: ' + y);
		const bounds = this.bounds;
		return (
			x >= bounds.x &&
			x <= bounds.x + bounds.width &&
			y >= bounds.y &&
			y <= bounds.y + bounds.height
		);
	}

	onMouseEnter() {
		this.bIsPointerOver = true;
		this.setPointerCursor();
	}

	onMouseLeave() {
		this.bIsPointerOver = false;
		this.setDefaultCursor();
	}

	charge() {
		this.bClickHandled = false;
		this.setPointerCursor();
	}

	handleClick(/*aMouseEvent*/) {
		if (!this.bIsPointerOver) return false;

		if (this.bClickHandled) return false;

		this.aEventMultiplexer.notifyEvent(EventTrigger.OnClick, this.getId());
		this.aSlideShow.update();
		this.bClickHandled = true;
		this.setDefaultCursor();
		return true;
	}

	setPointerCursor() {
		if (this.bClickHandled) return;
		this._canvas.style.cursor = 'pointer';
	}

	setDefaultCursor() {
		this._canvas.style.cursor = 'default';
	}
}
