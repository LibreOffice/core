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
 * AutoCompletePopup - base class for mention, formula auto complete, auto fill popup and auto fill preview popup
 */

/* global app */

interface Point {
	x: number;
	y: number;
}

interface FireEvent {
	data?: any;
}

interface CloseMessageEvent extends FireEvent {
	typingMention?: boolean;
}
interface MentionEvent extends FireEvent {
	triggerKey?: string;
}

abstract class AutoCompletePopup {
	protected map: any;
	protected newPopupData: PopupData;
	protected data: MessageEvent<any>;
	protected popupId: string;
	protected isSmallScreenDevice: boolean;

	constructor(popupId: string, map: any) {
		this.map = map;
		this.popupId = popupId;
		this.newPopupData = {
			children: [
				{
					id: 'container',
					type: 'container',
					enabled: true,
					children: new Array<WidgetJSON>(),
					vertical: true,
				} as any as WidgetJSON,
			] as Array<WidgetJSON>,
			jsontype: 'dialog',
			type: 'modalpopup',
			cancellable: true,
			popupParent: '_POPOVER_',
			clickToClose: '_POPOVER_',
			id: this.popupId,
			persistKeyboard: true,
		} as PopupData;

		this.isSmallScreenDevice = (<any>window).mode.isSmallScreenDevice();
		this.onAdd();
		this.map.on('closepopup', this.closePopup, this);
	}

	abstract onAdd(): void;

	closePopup(): void {
		var popupExists = window.L.DomUtil.get(this.popupId);
		if (!popupExists) return;

		this.map.jsdialog.focusToLastElement(this.popupId);
		this.map.jsdialog.clearDialog(this.popupId);
	}

	abstract getPopupEntries(ev: FireEvent): any[];

	getPopupJSON(control: any, framePos: any): PopupData {
		return {
			jsontype: 'dialog',
			id: this.popupId,
			action: 'update',
			control: control,
			posx: framePos.x,
			posy: framePos.y,
			children: undefined,
		} as any as PopupData;
	}

	getTreeJSON(): TreeWidgetJSON {
		return {
			id: this.popupId + 'List',
			type: 'treelistbox',
			text: '',
			enabled: true,
			singleclickactivate: false,
			fireKeyEvents: true,
			entries: [] as Array<TreeEntryJSON>,
			noSearchField: true,
		} as TreeWidgetJSON;
	}

	sendUpdate(data: any): void {
		this.map.fire('jsdialogupdate', {
			data: data,
			callback: this.callback.bind(this),
		});
	}

	sendJSON(data: any): void {
		const fireEvent = this.isSmallScreenDevice ? 'mobilewizard' : 'jsdialog';
		this.map.fire(fireEvent, {
			data: data,
			callback: this.callback.bind(this),
		});
	}

	getCursorPosition(): Point {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		);
		if (commentSection?.getActiveEdit()) {
			const caretRect = window
				.getSelection()
				.getRangeAt(0)
				.getBoundingClientRect();
			const mapRect = this.map._container.getBoundingClientRect();
			return new cool.Point(
				caretRect.left - mapRect.left,
				caretRect.bottom - mapRect.top,
			);
		}

		const currPos = {
			x: app.file.textCursor.rectangle.cX1,
			y: app.file.textCursor.rectangle.cY2,
		};
		const origin = this.map.getPixelOrigin();
		const panePos = this.map._getMapPanePos();
		return new cool.Point(
			Math.round(currPos.x + panePos.x - origin.x),
			Math.round(currPos.y + panePos.y - origin.y),
		);
	}

	openPopup(ev: FireEvent): void {
		const entries = this.getPopupEntries(ev);
		if (entries.length === 0) return;

		const cursorPos = this.getCursorPosition();
		const control = this.getTreeJSON();

		if (this.newPopupData.serverSyncSelection !== undefined) {
			control.serverSyncSelection = this.newPopupData.serverSyncSelection;
		}

		if (window.L.DomUtil.get(this.popupId + 'List')) {
			const data = this.getPopupJSON(control, cursorPos);
			(data.control as TreeWidgetJSON).entries = entries;
			this.sendUpdate(data);
			return;
		}

		if (window.L.DomUtil.get(this.popupId)) this.closePopup();

		const data = this.newPopupData;
		data.children[0].children[0] = control;
		(data.children[0].children[0] as TreeWidgetJSON).entries = entries;

		const isSpreadsheetRTL = this.map._docLayer.isCalcRTL();
		const canvasEl = this.map._docLayer._canvas.getBoundingClientRect();
		const offsetX = isSpreadsheetRTL
			? 0
			: app.sectionContainer.getSectionWithName(app.CSections.RowHeader.name)
					.size[0];
		const offsetY = app.sectionContainer.getSectionWithName(
			app.CSections.ColumnHeader.name,
		).size[1];

		if (isSpreadsheetRTL) cursorPos.x = this.map._size.x - cursorPos.x;

		data.posx = cursorPos.x + offsetX + canvasEl.left;
		data.posy = cursorPos.y + offsetY + canvasEl.top;

		this.sendJSON(data);
	}

	public getPopupId() {
		return this.popupId;
	}

	callback(objectType: any, eventType: any, object: any, index: number) {
		if (eventType === 'keydown') {
			if (object.key !== 'Tab' && object.key !== 'Shift') {
				this.map.focus();
				return true;
			}
		}
		return false;
	}
}
