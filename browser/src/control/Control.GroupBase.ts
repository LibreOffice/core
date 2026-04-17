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

export interface GroupEntry {
	level: number,
	index: number,
	startPos: number,
	endPos: number,
	hidden: boolean
}

export interface GroupEntryStrings {
	level: string,
	index: string,
	startPos: string,
	endPos: string,
	hidden: string
}

/*
	This file is Calc only. This is the base class for Control.RowGroup and Control.ColumnGroup files.

	This class is an extended version of "CanvasSectionObject".
*/

type GroupColors = { backgroundColor: string, borderColor: string, textColor?: string, strokeColor?: string };

export abstract class GroupBase extends CanvasSectionObject {
	_map: any;
	_textColor: string;
	_cachedColors: GroupColors;
	_getFont: () => string;
	_levelSpacing: number;
	_groupHeadSize: number;
	_groups: Array<Array<GroupEntry>>;
	isRemoved: boolean = false;

	constructor (name: string) {
		super(name);
		this._textColor = null;
		this._cachedColors = null;
	}

	// This function is called by CanvasSectionContainer when the section is added to the sections list.
	onInitialize(): void {
		this._map = window.L.Map.THIS;
		this.sectionProperties.docLayer = this._map._docLayer;
		this._groups = null;

		// group control styles
		this._groupHeadSize = Math.round(12 * app.dpiScale);
		this._levelSpacing = app.roundedDpiScale;

		this._map.on('sheetgeometrychanged', this.update, this);
		this._map.on('viewrowcolumnheaders', this.update, this);
		this._map.on('darkmodechanged', this._updateColors, this);
		this._createFont();
		this._updateColors();
		this.update();
		this.isRemoved = false;
	}

	// override in subclasses
	abstract update(): void;

	// Create font for the group headers. Group headers are on the left side of corner header.
	_createFont(): void {
		const baseElem = document.getElementsByTagName('body')[0];
		const elem = window.L.DomUtil.create('div', 'spreadsheet-header-row', baseElem);

		const fontFamily = window.L.DomUtil.getStyle(elem, 'font-family');
		this._getFont = () => {
			return Math.round(this._groupHeadSize * 0.8) + 'px ' + fontFamily;
		};
		window.L.DomUtil.remove(elem);
	}

	public getColors(): GroupColors {
		if (!this._cachedColors) {
			const baseElem = document.getElementsByTagName('body')[0];
			const elem = window.L.DomUtil.create('div', 'spreadsheet-header-row', baseElem);
			const isDark = window.prefs.getBoolean('darkTheme');

			this.backgroundColor = window.L.DomUtil.getStyle(elem, 'background-color');
			this.borderColor = this.backgroundColor;

			this._textColor = window.L.DomUtil.getStyle(elem, 'color');
			window.L.DomUtil.remove(elem);
			this._cachedColors = {
				backgroundColor: this.backgroundColor,
				borderColor: this.borderColor,
				textColor: this._textColor,
				strokeColor: isDark ? 'white' : 'black'
			};
		}

		return this._cachedColors;
	}

	protected _updateColors(): void {
		this._cachedColors = null; // reset cache
		this.getColors(); // refresh the cache
	}

	// This returns the required width for the section.
	_computeSectionWidth(): number {
		return this._levelSpacing + (this._groupHeadSize + this._levelSpacing) * (this._groups.length + 1);
	}

	// This function puts data into a good shape for use of this class.
	_collectGroupsData (groups: Array<GroupEntryStrings>): void {
		let level: number, groupEntry: GroupEntry;

		const lastGroupIndex = new Array(groups.length);
		const firstChildGroupIndex = new Array(groups.length);
		let lastLevel = -1;
		for (let i = 0; i < groups.length; ++i) {
			// a new group start
			const groupData = groups[i];
			level = parseInt(groupData.level) - 1;
			if (!this._groups[level]) {
				this._groups[level] = [];
			}
			let startPos = parseInt(groupData.startPos);
			const endPos = parseInt(groupData.endPos);
			const isHidden = !!parseInt(groupData.hidden);
			if (!isHidden) {
				let moved = false;
				// if the first child is collapsed the parent head has to be top-aligned with the child
				if (level < lastLevel && firstChildGroupIndex[lastLevel] !== undefined) {
					const childGroupEntry = this._groups[lastLevel][firstChildGroupIndex[lastLevel]];
					if (childGroupEntry.hidden) {
						if (startPos > childGroupEntry.startPos && startPos < childGroupEntry.endPos) {
							startPos = childGroupEntry.startPos;
							moved = true;
						}
					}
				}
				// if 2 groups belonging to the same level are contiguous and the first group is collapsed,
				// the second one has to be shifted as much as possible in order to avoid overlapping.
				if (!moved && lastGroupIndex[level] !== undefined) {
					const prevGroupEntry = this._groups[level][lastGroupIndex[level]];
					if (prevGroupEntry.hidden) {
						if (startPos <= prevGroupEntry.endPos) {
							startPos = prevGroupEntry.endPos + this._groupHeadSize;
						}
					}
				}
			}
			groupEntry = {
				level: level,
				index: parseInt(groupData.index),
				startPos: startPos,
				endPos: endPos,
				hidden: isHidden
			};
			this._groups[level][parseInt(groupData.index)] = groupEntry;
			lastGroupIndex[level] = groupData.index;
			if (level > lastLevel) {
				firstChildGroupIndex[level] = groupData.index;
				lastLevel = level;
			}
			else if (level === lastLevel) {
				firstChildGroupIndex[level + 1] = undefined;
			}
		}
	}

	_isParentGroupVisible(group_: GroupEntry): boolean {
		if (group_.hidden === false) {
			if (group_.level > 0) {
				// This recursive call is needed.
				// Because first upper group may have been expanded and second upper group may have been collapsed.
				// If one of the upper groups is not expanded, this function should return false.
				if (this._isPreviousGroupVisible(group_.level, group_.startPos, group_.endPos, group_.hidden)) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				return true;
			}
		}
		else {
			return false;
		}
	}

	// If previous group is visible (expanded), current group's plus sign etc. will be drawn. If previous group is not expanded, current group's plus sign etc. won't be drawn.
	_isPreviousGroupVisible(level: number, startPos: number, endPos: number, hidden: boolean): boolean {
		for (let i = 0; i < this._groups.length; i++) {
			var parentGroup;

			// find the correct parent group level
			if (i == level - 1) {
				for (const group in this._groups[i]) {
					if (Object.prototype.hasOwnProperty.call(this._groups[i], group)) {
						const group_ = this._groups[i][group];

						// parent group is expanded
						if ((startPos != endPos) && (startPos >= group_.startPos && endPos <= group_.endPos)) {
							return this._isParentGroupVisible(group_);
						}
						// parent group is collapsed and has a '-' sign
						else if ((startPos == endPos) && hidden == false) {
							if ((startPos == group_.startPos && endPos == group_.endPos)) {
								parentGroup = group_;
								// special condition: parent group is found, return.
								return this._isParentGroupVisible(parentGroup);
							}
							else if ((startPos == group_.startPos && endPos != group_.endPos)) {
								parentGroup = group_;
							}
							else if ((startPos != group_.startPos && endPos == group_.endPos)) {
								parentGroup = group_;
							}
							else if ((startPos > group_.startPos && endPos < group_.endPos)) {
								parentGroup = group_;
							}
						}
						// parent group is collapsed and has a '+' sign
						else if ((startPos == endPos) && hidden == true) {
							if ((startPos == group_.startPos && endPos == group_.endPos)) {
								parentGroup = group_;
							}
							else if ((startPos == group_.startPos && endPos != group_.endPos)) {
								parentGroup = group_;
							}
							else if ((startPos != group_.startPos && endPos == group_.endPos)) {
								parentGroup = group_;
							}
							else if ((startPos > group_.startPos && endPos < group_.endPos)) {
								parentGroup = group_;
							}
						}
					}
				}
				if (parentGroup !== undefined) {
					return this._isParentGroupVisible(parentGroup);
				}
			}
		}
	}

	drawGroupBoxes(startX: number, startY: number, hidden: boolean): void {
		// By forcing _groupHeadSize to be an even number, we guarantee that (_groupHeadSize / 2) is a solid integer.
		// This prevents the canvas from anti-aliasing (blurring) the line across two distinct pixels.
		if (this._groupHeadSize % 2 !== 0) {
			this._groupHeadSize++;
		}

		// draw head
		this.context.beginPath();
		this.context.fillStyle = this.backgroundColor;
		this.context.fillRect(this.transformRectX(startX, this._groupHeadSize), startY, this._groupHeadSize, this._groupHeadSize);
		this.context.strokeStyle = this.getColors().strokeColor;
		this.context.lineWidth = 1.0;
		this.context.strokeRect(this.transformRectX(startX + 0.5, this._groupHeadSize), startY + 0.5, this._groupHeadSize, this._groupHeadSize);

		if (!hidden) {
			// draw '-'
			this.context.beginPath();
			this.context.moveTo(startX + 0.5 + this._groupHeadSize * 0.25, startY + 0.5 + this._groupHeadSize / 2);
			this.context.lineTo(startX + 0.5 + this._groupHeadSize * 0.75, startY + 0.5 + this._groupHeadSize / 2);
			this.context.stroke();
		}
		else {
			// draw '+'
			this.context.beginPath();
			// horizontal
			this.context.moveTo(startX + 0.5 + this._groupHeadSize * 0.25, startY + 0.5 + this._groupHeadSize / 2);
			this.context.lineTo(startX + 0.5 + this._groupHeadSize * 0.75, startY + 0.5 + this._groupHeadSize / 2);
			this.context.stroke();
			// vertical
			this.context.moveTo(startX + 0.5 + this._groupHeadSize / 2, startY + 0.5 + this._groupHeadSize * 0.25);
			this.context.lineTo(startX + 0.5 + this._groupHeadSize / 2, startY + 0.5 + this._groupHeadSize * 0.75);
			this.context.stroke();
		}
	}

	drawGroupControl (entry: GroupEntry): void {
		return;
	}

	// This calls drawing functions related to tails and plus & minus signs etc.
	drawOutline(): void {
		if (this._groups) {
			for (let i = 0; i < this._groups.length; i++) {
				if (this._groups[i]) {
					for (const group in this._groups[i]) {
						if (Object.prototype.hasOwnProperty.call(this._groups[i], group)) {
							// always draw the first level
							if (this._groups[i][group].level == 0) {
								this.drawGroupControl(this._groups[i][group]);
							}
							else if (this._isPreviousGroupVisible(this._groups[i][group].level, this._groups[i][group].startPos, this._groups[i][group].endPos, this._groups[i][group].hidden)) {
								this.drawGroupControl(this._groups[i][group]);
							}
						}
					}
				}
			}
		}
	}

	drawLevelHeader (level: number): void {
		return;
	}

	// This function calls drawing function for related to headers of groups. Headers are drawn on the left of corner header.
	drawLevelHeaders(): void {
		for (let i = 0; i < this._groups.length + 1; ++i) {
			this.drawLevelHeader(i);
		}
	}

	/// In Calc RTL mode, x-coordinate of a given rectangle of given width is horizontally mirrored
	transformRectX (xcoord: number, rectWidth: number): number {
		return this.isCalcRTL() ? this.size[0] - xcoord - rectWidth : xcoord;
	}

	/// In Calc RTL mode, x-coordinate of a given point is horizontally mirrored
	transformX (xcoord: number): number {
		return this.isCalcRTL() ? this.size[0] - xcoord : xcoord;
	}

	/**
	 * Checks if the given point is within the bounds of the rectangle defined by
	 * startX, startY, endX, endY. If mirrorX is true then point is horizontally
	 * mirrored before checking.
	 */
	isPointInRect (point: cool.SimplePoint, startX: number, startY: number, endX: number, endY: number, mirrorX: boolean): boolean {
		const x = mirrorX ? this.size[0] - point.pX : point.pX;
		const y = point.pY;

		return (x > startX && x < endX && y > startY && y < endY);
	}

	onDraw(): void {
		this.drawOutline();
		this.drawLevelHeaders();
	}

	findClickedGroup (point: cool.SimplePoint): GroupEntry {
		return null;
	}

	findClickedLevel (point: cool.SimplePoint): number {
		return -1;
	}

	onMouseMove (point: cool.SimplePoint): void {
		// If mouse is above a group header or a group control, we change the cursor.
		if (this.findClickedGroup(point) !== null || this.findClickedLevel(point) !== -1)
			this.context.canvas.style.cursor = 'pointer';
		else
			this.context.canvas.style.cursor = 'default';
	}

	_updateOutlineState(group: Partial<GroupEntry>): void {
		return;
	}

	onClick (point: cool.SimplePoint): void {
		// User may have clicked on one of the level headers.
		const level = this.findClickedLevel(point);
		if (level !== -1) {
			this._updateOutlineState({level: level, index: -1}); // index: -1 targets all groups (there may be multiple separate row groups.).
		}
		else {
			// User may have clicked on one of the group control boxes (boxes with plus / minus symbols).
			var group = this.findClickedGroup(point);

			if (group) {
				this._updateOutlineState(group);
			}
		}
	}

	// returns [startX, endX, startY, endY]
	getTailsGroupRect (group: GroupEntry): number[] {
		return [0, 0, 0, 0];
	}

	findTailsGroup (point: cool.SimplePoint): GroupEntry {
		const mirrorX = this.isCalcRTL();
		for (let i = 0; i < this._groups.length; i++) {
			if (this._groups[i]) {
				for (const group in this._groups[i]) {
					if (Object.prototype.hasOwnProperty.call(this._groups[i], group)) {
						const group_ = this._groups[i][group];
						const rect = this.getTailsGroupRect(group_);
						const startX = rect[0];
						const startY = rect[2];
						const endX = rect[1];
						const endY = rect[3];

						if (this.isPointInRect(point, startX, startY, endX, endY, mirrorX)) {
							return group_;
						}
					}
				}
			}
		}
	}

	/* Double clicking on a group's tail closes it. */
	onDoubleClick (point: cool.SimplePoint): void {
		const group = this.findTailsGroup(point);
		if (group)
			this._updateOutlineState(group);
	}

	onRemove(): void {
		this.isRemoved = true;
		this.containerObject.getSectionWithName(app.CSections.RowHeader.name).position[0] = 0;
		this.containerObject.getSectionWithName(app.CSections.CornerHeader.name).position[0] = 0;
	}
}
}
