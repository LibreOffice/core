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

/*
 * Util.FormulaBarSelectionHandle - a selection handle for the calc formulabar
 * Contrast with TextSelectionHandle section
 */

interface DraggableDragEvent {
	originalEvent: MouseEvent | TouchEvent;
}

type FormulaBarSelectionHandleSide = 'start' | 'end';
class FormulaBarSelectionHandle {
	readonly width: number = 30;
	private visible: boolean = false;
	private side: FormulaBarSelectionHandleSide;

	private element: HTMLDivElement;
	private wrapper: HTMLDivElement;

	private draggable: any;

	onDrag?: (point: cool.Point) => void;
	onDragEnd?: (event: cool.Point) => void;

	/**
	 * @param selectionLayer The layer that the formulabar's custom selections are placed in - we'll display relative to this element
	 * @param side The side of the formula bar that this handle is on
	 */
	constructor(
		wrapper: HTMLDivElement,
		handleLayer: HTMLDivElement,
		side: FormulaBarSelectionHandleSide,
	) {
		this.side = side;
		this.element = window.L.DomUtil.create(
			'div',
			`formulabar-selection-handle-${side}`,
			handleLayer,
		);
		this.wrapper = wrapper;
		this.wrapper.addEventListener('scroll', () => this.requestVisible());

		this.draggable = new window.L.Draggable(
			this.element,
			this.element,
			true,
			true, // no3d - required because we're doing some weird relative positioning and a 3d transform will end up in the wrong place
		);
		this.draggable
			.on(
				{
					drag: this._onDrag.bind(this),
					dragend: this._onDragEnd.bind(this),
				},
				this,
			)
			.enable();
	}

	setPosition(point: cool.SimplePoint) {
		let offsetX = 0;
		if (this.side === 'start') {
			offsetX = -this.width;
		}

		const lPoint = new cool.Point(point.x + offsetX, point.y); // "what you're doing is gross and horrible Skyler and I hate it", "yeah well, Draggable doesn't work with SimplePoints so ig we both have to deal..."
		// also for some reason the twips value is correct here? I'm not going to fix what ain't...
		window.L.DomUtil.setPosition(this.element, lPoint, true);
		// Not using 3d transforms since as we're doing weird relative position hacks to get here :)
		// Using DomUtil as we need to in order to be draggable later
	}

	_onDrag(event: DraggableDragEvent) {
		// update selection position, set in Widget.FormulabarEdit.js so it can be repeatedly overidden...
		if (this.onDrag) {
			const point = window.L.DomUtil.getPosition(this.element);
			this.onDrag(point);
		}
	}

	_onDragEnd(_: unknown) {
		// update selection position, set in Widget.FormulabarEdit.js so it can be repeatedly overidden...
		if (this.onDragEnd) {
			const point = window.L.DomUtil.getPosition(this.element);
			this.onDragEnd(point);
		}
	}

	/**
	 * This is 'requestVisible' rather than 'setVisible' because it doesn't provide a guarentee that or when the element
	 * will become visible
	 *
	 * It, for example, *will not* become visible if the touchscreen is not being used or if the document is read-only
	 *
	 * If the 'visible' parameter is not specified, this will recalculate visibility based on the most recent provided
	 * visibility and the current state of the world (e.g. for if this scrolls into view). Most of the time you won't need
	 * to do this manually
	 *
	 * @param visible Whether the handle should be visible. Leave undefined to recalculate visibility based on the previously specified value
	 */
	requestVisible(visible?: boolean) {
		if (visible !== undefined) {
			this.visible = visible;
		}

		app.layoutingService.appendLayoutingTask(() => {
			if (!this.visible || !window.touch.currentlyUsingTouchscreen()) {
				this.element.style.visibility = 'hidden';
				return;
			}

			const wrapperRect = this.wrapper.getBoundingClientRect();
			const elementRect = this.element.getBoundingClientRect();
			// The handle top sits on the bottom of the line it marks. Handles are hidden if the
			// middle of that line is outside of the formula bar.
			const lineHeight =
				parseFloat(getComputedStyle(this.element).lineHeight) || 0;
			const lineMiddle = elementRect.top - lineHeight / 2;
			if (wrapperRect.top > lineMiddle || wrapperRect.bottom < lineMiddle) {
				this.element.style.visibility = 'hidden';
				return;
			}

			this.element.style.visibility = 'visible';
		});
	}
}
