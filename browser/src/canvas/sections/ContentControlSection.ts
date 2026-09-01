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

export class ContentControlSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.ContentControl.processingOrder;
	drawingOrder: number = app.CSections.ContentControl.drawingOrder;
	zIndex: number = app.CSections.ContentControl.zIndex;
	interactable: boolean = false;
	documentObject: boolean = true;

	map: any;

	constructor() {
		super(app.CSections.ContentControl.name);

		this.map = window.L.Map.THIS;
		this.sectionProperties.json = null;
		this.sectionProperties.datePicker = false;
		this.sectionProperties.picturePicker = null;
		this.sectionProperties.polygon = null;
		this.sectionProperties.dropdownSection = null;
		this.sectionProperties.dropdownMarkerWidth = 22;
		this.sectionProperties.dropdownMarkerHeight = 22;
	}

	public onInitialize(): void {
		this.map.on('darkmodechanged', this.changeBorderStyle, this);

		var container = window.L.DomUtil.createWithId('div', 'datepicker');
		container.style.zIndex = '12';
		container.style.position = 'absolute';
		document.getElementById('document-container').appendChild(container);
		this.sectionProperties.picturePicker = false;
	}

	private setDatePickerVisibility(visible: boolean): void {
		this.sectionProperties.datePicker = visible;

		if (this.sectionProperties.dropdownSection)
			this.sectionProperties.dropdownSection.sectionProperties.datePicker = visible;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public drawContentControl(json: any) {
		this.removeDropdownSection();

		this.sectionProperties.json = json;
		this.setDatePickerVisibility(false);
		this.sectionProperties.picturePicker = false;

		if (json.date) {
			$.datepicker.setDefaults($.datepicker.regional[(<any>window).langParamLocale.language]);
			$('#datepicker').datepicker({
				onSelect: function (date: any, datepicker: any) {
					if (date != '') {
						app.socket.sendMessage('contentcontrolevent type=date selected=' + date);
					}
				}
			});
			$('#datepicker').hide();
		} else
			$('#datepicker').datepicker('destroy');

		if (json.action === 'show')
			this.preparePolygon();
		else if (json.action === 'hide')
			this.sectionProperties.polygon = null;
		else if (json.action === 'change-picture') {
			this.sectionProperties.picturePicker = true;
			if (!this.map.wopi.EnableInsertRemoteImage)
				app.LOUtil.openFilePicker('insertgraphic');
			else
				this.map.fire('postMessage', {msgId: 'UI_InsertGraphic'});
		}

		this.setPositionAndSize();

		if (this.sectionProperties.json.items || json.date) {
			this.addDropDownSection();

			if (json.date) this.setDatePickerVisibility(true);
		}

		app.sectionContainer.requestReDraw();
	}

	private setPositionAndSize (): void {
		if (!this.sectionProperties.json || !this.sectionProperties.json.rectangles)
			return;

		var rectangles: Array<number>[] = this.getRectangles(this.sectionProperties.json.rectangles);
		this.setPositionAndSizeFromTwipRectangles(rectangles);
	}

	public onResize (): void {
		this.setPositionAndSize();
	}

	public preparePolygon(): void {
		if (!this.sectionProperties.json.rectangles)
			return;

		// Parse rectangles first.
		const rectangleArray = this.getRectangles(this.sectionProperties.json.rectangles);

		this.sectionProperties.polygon = cool.rectanglesToPolygon(rectangleArray, app.twipsToPixels);

		this.changeBorderStyle();
	}

	public onDraw(): void {
		if (!this.sectionProperties.json)
			return;

		if (this.sectionProperties.polygon)
			this.drawPolygon();

		var text:string = this.sectionProperties.json.alias;
		if (text) {
			var rectangles: Array<number>[] = this.getRectangles(this.sectionProperties.json.rectangles);
			var x: number = rectangles[rectangles.length-1][0] * app.twipsToPixels;
			var y: number = rectangles[rectangles.length-1][1] * app.twipsToPixels;

			// fixed height for alias tag
			var h: number = 20;
			var startX: number = x - this.position[0] + 5;
			var startY: number = y - this.position[1];
			var padding: number = 10;
			var fontStyle = getComputedStyle(document.body).getPropertyValue('--cool-font').split(',')[0].replace(/'/g, '');
			var fontSize = getComputedStyle(document.body).getPropertyValue('--default-font-size');
			var font = fontSize + ' ' + fontStyle;
			var textWidth: number = app.util.getTextWidth(text, font) + padding;

			// draw rectangle with backgroundcolor
			this.context.beginPath();
			this.context.fillStyle = '#E6FFFF';
			this.context.font = font;
			this.context.fillRect(startX, startY - h, textWidth, h);

			// add text to the rectangle
			this.context.textAlign = 'center';
			this.context.textBaseline = 'middle';
			this.context.fillStyle = '#026296';
			this.context.fillText(text, startX + textWidth / 2, startY - h / 2);

			// draw borders around the rectangle
			this.context.strokeStyle = '#026296';
			this.context.lineWidth = app.dpiScale;
			this.context.strokeRect(startX - 0.5, startY - h - 0.5, textWidth, h);

		}
	}

	public onNewDocumentTopLeft (): void {
		this.setPositionAndSize();
	}

	private removeDropdownSection() {
		if (this.sectionProperties.dropdownSection)
			app.sectionContainer.removeSection(this.sectionProperties.dropdownSection.name);
	}

	private addDropDownSection(): void {
		this.sectionProperties.dropdownSection = new ContentControlDropdownSubSection(
			'dropdown' + String(Math.random()),
			new cool.SimplePoint((this.position[0] + this.size[0]) * app.pixelsToTwips, (this.position[1]) * app.pixelsToTwips),
			true,
			this.sectionProperties.dropdownMarkerWidth,
			this.sectionProperties.dropdownMarkerHeight
		);

		this.sectionProperties.dropdownSection.size[0] = this.sectionProperties.dropdownMarkerWidth * app.dpiScale;
		this.sectionProperties.dropdownSection.size[1] = this.size[1];
		this.sectionProperties.dropdownSection.sectionProperties.json = this.sectionProperties.json;
		this.sectionProperties.dropdownSection.sectionProperties.parent = this;
		app.sectionContainer.addSection(this.sectionProperties.dropdownSection);
		this.sectionProperties.dropdownSection.adjustHTMLObjectPosition();
		this.sectionProperties.dropdownSection.setShowSection(true);
	}

	private getRectangles(rect: string): Array<number>[] {
		var rectangles: Array<number>[] = [];
		//convert string to number coordinates
		var matches = rect.match(/\d+/g);
		if (matches !== null) {
			for (var i: number = 0; i < matches.length; i += 4) {
				rectangles.push([parseInt(matches[i]), parseInt(matches[i + 1]), parseInt(matches[i + 2]), parseInt(matches[i + 3])]);
			}
		}
		return rectangles;
	}

}

}

app.definitions.ContentControlSection = cool.ContentControlSection;

