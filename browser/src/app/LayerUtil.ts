/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// CStyleData is used to obtain CSS property values from style data
// stored in DOM elements in the form of custom CSS properties/variables.
class CStyleData extends BaseClass {
	private _div: Element;

	constructor(styleDataDiv: Element) {
		super();
		this._div = styleDataDiv;
	}

	public getPropValue(name: string): string {
		return getComputedStyle(this._div).getPropertyValue(name);
	}

	public getIntPropValue(name: string): number {
		return parseInt(this.getPropValue(name));
	}

	public getFloatPropValue(name: string): number {
		return parseFloat(this.getPropValue(name));
	}

	public getFloatPropWithoutUnit(name: string): number {
		let value = this.getPropValue(name);
		if (value.indexOf('px')) value = value.split('px')[0];
		return parseFloat(value);
	}
}

// CSelections is used to add/modify/clear selections (text/cell-area(s)/ole)
// on canvas using polygons (CPolygon).
class CSelections extends BaseClass {
	private _pointSet: CPointSet;
	private _overlay: CanvasOverlay;
	private _styleData: CStyleData;
	private _map: MapInterface;
	private _name: string;
	private _isView: boolean;
	private _viewId: number;
	private _isText: boolean;
	private _isOle: boolean;
	private _selection?: CPolygon | CDarkOverlay | CCellSelection;

	constructor(
		pointSet: CPointSet,
		canvasOverlay: CanvasOverlay,
		selectionsDataDiv: Element,
		map: MapInterface,
		isView: boolean,
		viewId: number,
		selectionType: string,
	) {
		super();
		this._pointSet = pointSet ? pointSet : new CPointSet();
		this._overlay = canvasOverlay;
		this._styleData = new CStyleData(selectionsDataDiv);
		this._map = map;
		this._name = 'selections' + (isView ? '-viewid-' + viewId : '');
		this._isView = isView;
		this._viewId = viewId;
		this._isText = selectionType === 'text';
		this._isOle = selectionType === 'ole';
		this._selection = undefined;
		this._updateSelection();
	}

	public empty(): boolean {
		return !this._pointSet || this._pointSet.empty();
	}

	public clear(): void {
		this.setPointSet(new CPointSet());
	}

	public setPointSet(pointSet: CPointSet): void {
		this._pointSet = pointSet;
		this._updateSelection();
	}

	public contains(corePxPoint: cool.Point) {
		if (!this._selection) return false;

		return (this._selection as CCellSelection | CPolygon).anyRingBoundContains(
			corePxPoint,
		);
	}

	public getBounds(): cool.Bounds {
		return (this._selection as CCellSelection | CPolygon).getBounds();
	}

	private _updateSelection(): void {
		let attributes: any;
		if (!this._selection) {
			if (!this._isOle) {
				const fillColor = this._isView
					? app.LOUtil.rgbToHex(this._map.getViewColor(this._viewId))
					: this._styleData.getPropValue('background-color');
				const opacity = this._styleData.getFloatPropValue('opacity');
				const weight =
					this._styleData.getFloatPropWithoutUnit('border-top-width');
				attributes = this._isText
					? {
							viewId: this._isView ? this._viewId : undefined,
							groupType: PathGroupType.TextSelection,
							name: this._name,
							pointerEvents: 'none',
							fillColor: fillColor,
							fillOpacity: opacity,
							color: fillColor,
							opacity: 0.6,
							stroke: true,
							fill: true,
							weight: 1.0,
						}
					: {
							viewId: this._isView ? this._viewId : undefined,
							name: this._name,
							pointerEvents: 'none',
							color: fillColor,
							fillColor: fillColor,
							fillOpacity: opacity,
							opacity: 1.0,
							weight: Math.round(weight * app.dpiScale),
						};
			} else {
				attributes = {
					pointerEvents: 'none',
					fillColor: 'black',
					fillOpacity: 0.25,
					weight: 0,
					opacity: 0.25,
				};
			}

			if (this._isText) {
				this._selection = new CPolygon(this._pointSet, attributes);
			} else if (this._isOle) {
				this._selection = new CDarkOverlay(this._pointSet, attributes);
			} else {
				this._selection = new CCellSelection(this._pointSet, attributes);
			}

			if (this._isText) this._overlay.initPath(this._selection as CPolygon);
			else
				this._overlay.initPathGroup(
					this._selection as CCellSelection | CDarkOverlay,
				);
			return;
		}

		this._selection.setPointSet(this._pointSet);
	}

	public remove(): void {
		if (!this._selection) return;
		if (this._isText) this._overlay.removePath(this._selection as CPolygon);
		else
			this._overlay.removePathGroup(
				this._selection as CDarkOverlay | CCellSelection,
			);
	}
}

// CReferences is used to store and manage the CPath's of all
// references in the current sheet.
class CReferences extends BaseClass {
	private _overlay: CanvasOverlay;
	private _marks: CPath[];

	constructor(canvasOverlay: CanvasOverlay) {
		super();
		this._overlay = canvasOverlay;
		this._marks = [];
	}

	public addMark(mark: CPath): void {
		this._overlay.initPath(mark);
		this._marks.push(mark);
	}

	public hasMark(mark: CPath): boolean {
		for (let i = 0; i < this._marks.length; ++i) {
			if (mark.getBounds().equals(this._marks[i].getBounds())) return true;
		}

		return false;
	}

	public empty(): boolean {
		return this._marks.length === 0;
	}

	public clear(): void {
		for (let i = 0; i < this._marks.length; ++i)
			this._overlay.removePath(this._marks[i]);
		this._marks = [];
	}
}
