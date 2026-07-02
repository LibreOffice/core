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

namespace cool {

export type SplitPanesOptions = {
	maxHorizontalSplitPercent: number;
	maxVerticalSplitPercent: number;
}

export type PaneStatus = {
	xFixed: boolean;
	yFixed: boolean;
}

/*
 * SplitPanesContext stores positions/sizes/objects related to split panes.
 */
export class SplitPanesContext {

	private static options: SplitPanesOptions = {
		maxHorizontalSplitPercent: 70,
		maxVerticalSplitPercent: 70,
	};

	protected _docLayer: any;
	protected _map: any;
	protected _splitPos: Point;

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	constructor(docLayer: any) {
		console.assert(docLayer, 'no docLayer!');
		console.assert(docLayer._map, 'no map!');

		this._docLayer = docLayer;
		this._map = docLayer._map;
		this._setDefaults();
	}

	protected _setDefaults(): void {
		this._splitPos = new Point(0, 0);
	}

	public get options(): SplitPanesOptions {
		return SplitPanesContext.options;
	}

	public getMaxSplitPosX(): number {
		const rawMax = Math.floor(app.sectionContainer.getWidth() * this.options.maxHorizontalSplitPercent / 100);
		return this._docLayer.getSnapDocPosX(rawMax);
	}

	public getMaxSplitPosY(): number {
		const rawMax = Math.floor(app.sectionContainer.getHeight() * this.options.maxVerticalSplitPercent / 100);
		return this._docLayer.getSnapDocPosY(rawMax);
	}

	public setSplitPos(splitX: number, splitY: number, forceUpdate: boolean = false): void {

		var xchanged = this.setHorizSplitPos(splitX, forceUpdate, true /* noFire */);
		var ychanged = this.setVertSplitPos(splitY, forceUpdate, true /* noFire */);
		if (xchanged || ychanged) {
			this._map.fire('splitposchanged');
			const section = app.sectionContainer.getSectionWithName(app.CSections.Splitter.name);
			if (section) {
				section.setPosition(0, 0); // To refresh myTopLeft property.
			}
		}
	}

	public getSplitPos(): Point {
		return this._splitPos.divideBy(app.dpiScale as number);
	}

	public justifySplitPos(split: number, isHoriz: boolean): number {
		if (split <= 0) {
			return 0;
		}

		var maxSplitPos = isHoriz ? this.getMaxSplitPosX() : this.getMaxSplitPosY();
		if (split >= maxSplitPos) {
			return maxSplitPos;
		}

		return isHoriz ? this._docLayer.getSnapDocPosX(split) :
			this._docLayer.getSnapDocPosY(split);
	}

	public setHorizSplitPos(splitX: number, forceUpdate: boolean, noFire: boolean): boolean {

		console.assert(typeof splitX === 'number', 'splitX must be a number');

		if (this._splitPos.x === splitX) {
			return false;
		}

		var changed = false;
		var newX = this.justifySplitPos(splitX, true /* isHoriz */);
		if (newX !== this._splitPos.x) {
			this._splitPos.x = newX;
			changed = true;
		}

		app.calc.splitCoordinate.pX = newX;

		if (!noFire)
			this._map.fire('splitposchanged');

		return changed;
	}

	public setVertSplitPos(splitY: number, forceUpdate: boolean, noFire: boolean): boolean {

		console.assert(typeof splitY === 'number', 'splitY must be a number');

		if (this._splitPos.y === splitY) {
			return false;
		}

		var changed = false;
		var newY = this.justifySplitPos(splitY, false /* isHoriz */);
		if (newY !== this._splitPos.y) {
			this._splitPos.y = newY;
			changed = true;
		}

		app.calc.splitCoordinate.pY = newY;

		if (!noFire)
			this._map.fire('splitposchanged');

		return changed;
	}

	public getPanesProperties(): PaneStatus[] {
		var paneStatusList: PaneStatus[] = [];
		if (this._splitPos.x && this._splitPos.y) {
			// top-left pane
			paneStatusList.push({
				xFixed: true,
				yFixed: true,
			});
		}

		if (this._splitPos.y) {
			// top-right pane or top half pane
			paneStatusList.push({
				xFixed: false,
				yFixed: true,
			});
		}

		if (this._splitPos.x) {
			// bottom-left pane or left half pane
			paneStatusList.push({
				xFixed: true,
				yFixed: false,
			});
		}

		// bottom-right/bottom-half/right-half pane or the full pane (when there are no split-panes active)
		paneStatusList.push({
			xFixed: false,
			yFixed: false,
		});

		return paneStatusList;
	}

	// When view is split by horizontal and/or vertical line(s), there are up to 4 different parts of the file visible on the screen.
	// This function returns the viewed parts' coordinates as simple rectangles.
	public getViewRectangles(): cool.SimpleRectangle[] {
		const viewRectangles: cool.SimpleRectangle[] = new Array<cool.SimpleRectangle>();
		viewRectangles.push(app.activeDocument.activeLayout.viewedRectangle.clone()); // If view is not splitted, this will be the only view rectangle.

		/*
			|----------------------------|
			| initial [0]  |  topright   |
			|              |             |
			|--------------|-------------|
			| bottomleft   |  bottomright|
			|              |             |
			|----------------------------|

		*/

		if (this._splitPos.x) { // Vertical split.
			// There is vertical split, narrow down the initial view.
			viewRectangles[0].pX1 = 0;
			viewRectangles[0].pX2 = this._splitPos.x;

			const topRightPane: cool.SimpleRectangle = app.activeDocument.activeLayout.viewedRectangle.clone();
			const width = app.activeDocument.activeLayout.viewedRectangle.pWidth - viewRectangles[0].pWidth;
			topRightPane.pX1 = app.activeDocument.activeLayout.viewedRectangle.pX2 - width;
			topRightPane.pWidth = width;
			viewRectangles.push(topRightPane);
		}

		if (this._splitPos.y) {
			// There is a horizontal split, narrow down the initial view.
			viewRectangles[0].pY1 = 0;
			viewRectangles[0].pY2 = this._splitPos.y;

			const bottomLeftPane = app.activeDocument.activeLayout.viewedRectangle.clone();
			const height = app.activeDocument.activeLayout.viewedRectangle.pHeight - viewRectangles[0].pHeight;
			bottomLeftPane.pY1 = app.activeDocument.activeLayout.viewedRectangle.pY2 - height;
			bottomLeftPane.pHeight = height;
			viewRectangles.push(bottomLeftPane);
		}

		// If both splitters are active, don't let them overlap and add the bottom right pane.
		if (this._splitPos.x && this._splitPos.y) {
			viewRectangles[1].pY1 = 0;
			viewRectangles[1].pY2 = this._splitPos.y;

			viewRectangles[2].pX1 = 0;
			viewRectangles[2].pX2 = this._splitPos.x;

			const bottomRightPane = app.activeDocument.activeLayout.viewedRectangle.clone();
			const width = app.activeDocument.activeLayout.viewedRectangle.pWidth - viewRectangles[0].pWidth;
			const height = app.activeDocument.activeLayout.viewedRectangle.pHeight - viewRectangles[0].pHeight;
			bottomRightPane.pX1 = app.activeDocument.activeLayout.viewedRectangle.pX2 - width;
			bottomRightPane.pWidth = width;
			bottomRightPane.pY1 = app.activeDocument.activeLayout.viewedRectangle.pY2 - height;
			bottomRightPane.pHeight = height;

			viewRectangles.push(bottomRightPane);
		}

		return viewRectangles;
	}

	// returns all the pane rectangles for the provided full-map area (all in core pixels).
	public getPxBoundList(pxBounds?: Bounds): Bounds[] {
		if (!pxBounds) {
			const vr = app.activeDocument.activeLayout.viewedRectangle;
			pxBounds = new Bounds(new Point(vr.pX1, vr.pY1), new Point(vr.pX1 + vr.pWidth, vr.pY1 + vr.pHeight));
		}
		var topLeft = pxBounds.getTopLeft();
		var bottomRight = pxBounds.getBottomRight();
		var boundList: Bounds[] = [];

		if (this._splitPos.x && this._splitPos.y) {
			// top-left pane
			boundList.push(new Bounds(
				new Point(0, 0),
				this._splitPos
			));
		}

		if (this._splitPos.y) {
			// top-right pane or top half pane
			boundList.push(new Bounds(
				new Point(topLeft.x + this._splitPos.x, 0),
				new Point(bottomRight.x, this._splitPos.y)
			));
		}

		if (this._splitPos.x) {
			// bottom-left pane or left half pane
			boundList.push(new Bounds(
				new Point(0, topLeft.y + this._splitPos.y),
				new Point(this._splitPos.x, bottomRight.y)
			));
		}

		if (!boundList.length) {
			// the full pane (when there are no split-panes active)
			boundList.push(new Bounds(
				topLeft,
				bottomRight
			));
		} else {
			// bottom-right/bottom-half/right-half pane
			boundList.push(new Bounds(
				topLeft.add(this._splitPos),
				bottomRight
			));
		}

		return boundList;
	}

	public intersectsVisible(areaPx: Bounds): boolean {
		const vr = app.activeDocument.activeLayout.viewedRectangle;
		var pixBounds = new Bounds(new Point(vr.pX1, vr.pY1), new Point(vr.pX1 + vr.pWidth, vr.pY1 + vr.pHeight));
		var boundList = this.getPxBoundList(pixBounds);
		for (var i = 0; i < boundList.length; ++i) {
			if (areaPx.intersects(boundList[i])) {
				return true;
			}
		}

		return false;
	}
}

}
