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

// SheetSwitchViewRestore is used to store the last view position of a sheet
// before a sheet switch so that when the user switches back to previously used
// sheets we can restore the last scroll position of that sheet.
export class SheetSwitchViewRestore {

	private map: any;
	private docLayer: any;

	// centerOfSheet maps from sheet id to center of sheet view.
	private centerOfSheet: Map<number, any>;
	private mayRestore: boolean;
	private restorePart: number;
	private setPartRecvd: boolean;
	private currentSheetIndexReassigned: boolean;

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	constructor (map: any) {

		this.map = map;
		this.docLayer = this.map._docLayer;

		this.centerOfSheet = new Map<number, any>();
		this.mayRestore = false;
		this.restorePart = -1;
		this.setPartRecvd = false;
		this.currentSheetIndexReassigned = false;

	}

	public save (toPart: number): void {
		if (!this.currentSheetIndexReassigned) {
			this.centerOfSheet.set(this.docLayer._selectedPart as number, this.map.getCenter());
		} else {
			this.currentSheetIndexReassigned = false;
		}
		this.mayRestore = this.centerOfSheet.has(toPart);
		this.restorePart = this.mayRestore ? toPart : -1;
		this.setPartRecvd = false;
	}

	public updateOnSheetMoved(oldIndex: number, newIndex: number): void {
		if (oldIndex < 0 || newIndex < 0 || oldIndex === newIndex)
			return;

		const movedSheetCenter = this.centerOfSheet.get(oldIndex);

		if (oldIndex < newIndex) {
			for (let i = oldIndex; i < newIndex; ++i) {
				const center = this.centerOfSheet.get(i + 1);
				if (center)
					this.centerOfSheet.set(i, center);
				else
					this.centerOfSheet.delete(i);
			}
		} else {
			for (let i = oldIndex; i > newIndex; --i) {
				const center = this.centerOfSheet.get(i - 1);
				if (center)
					this.centerOfSheet.set(i, center);
				else
					this.centerOfSheet.delete(i);
			}
		}

		if (movedSheetCenter)
			this.centerOfSheet.set(newIndex, movedSheetCenter);
		else
			this.centerOfSheet.delete(newIndex);
	}

	public updateOnSheetInsertion(index: number): void {
		if (index < 0)
			return;

		// when insert a sheet
		this.centerOfSheet.set(this.docLayer._selectedPart as number, this.map.getCenter());

		const numberOfSheets: number = this.map.getNumberOfParts();
		for (let i = numberOfSheets; i > index; --i) {
			const center = this.centerOfSheet.get(i - 1);
			if (center)
				this.centerOfSheet.set(i, center);
			else
				this.centerOfSheet.delete(i);
		}
		this.centerOfSheet.delete(index);

		const currentSheetNumber: number = this.map.getCurrentPartNumber();
		this.currentSheetIndexReassigned = index <= currentSheetNumber;
		if (this.currentSheetIndexReassigned) {
			this.centerOfSheet.set(currentSheetNumber + 1, this.map.getCenter());
		}
	}

	public updateOnSheetDeleted(index: number): void {
		if (index < 0)
			return;

		const numberOfSheets: number = this.map.getNumberOfParts();
		for (let i = index; i < numberOfSheets; ++i) {
			const center = this.centerOfSheet.get(i + 1);
			if (center)
				this.centerOfSheet.set(i, center);
			else
				this.centerOfSheet.delete(i);
		}

		const currentSheetNumber: number = this.map.getCurrentPartNumber();
		this.currentSheetIndexReassigned = index <= currentSheetNumber;
		if (index < currentSheetNumber) {
			this.centerOfSheet.set(currentSheetNumber - 1, this.map.getCenter());
		}
	}

	public gotSetPart(part: number): void {

		this.setPartRecvd = (part === this.restorePart);
	}

	// This resets the flags but not the center map.
	private reset (): void {

		this.restorePart = -1;
		this.mayRestore = false;
	}

	private restoreView (): void {

		const center = this.centerOfSheet.get(this.restorePart);
		if (center === undefined) {
			this.reset();
			return;
		}

		this.map._resetView(center, this.map._zoom);

		// Keep restoring view for every cell-cursor messages until we get this
		// call after receiving cell-cursor msg after setpart incoming msg.
		// Because it is guaranteed that cell-cursor messages belong to the new part
		// after setpart(incoming) msg.
		if (this.setPartRecvd)
			this.reset();
	}

	// This should be called to restore sheet's last scroll position if necessary and
	// returns whether the map should scroll to current cursor.
	public tryRestore(duplicateCursor: boolean, currentPart: number): boolean {

		let shouldScrollToCursor = false;
		const attemptRestore = (this.mayRestore && currentPart === this.restorePart);

		if (attemptRestore) {
			if (this.setPartRecvd && duplicateCursor)
				this.reset();
			if (!this.setPartRecvd)
				this.restoreView();
		}

		if ((!attemptRestore || this.setPartRecvd) && !duplicateCursor)
			shouldScrollToCursor = true;

		return shouldScrollToCursor;
	}
}

}
