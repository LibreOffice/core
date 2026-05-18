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
	interface HeaderFooterBoundaryMessage {
		page?: string;
		header?: string;
		footer?: string;
		hide?: boolean;
	}

	export class HeaderFooterBoundarySection extends CanvasSectionObject {
		processingOrder: number =
			app.CSections.HeaderFooterBoundary.processingOrder;
		drawingOrder: number = app.CSections.HeaderFooterBoundary.drawingOrder;
		zIndex: number = app.CSections.HeaderFooterBoundary.zIndex;
		interactable: boolean = false;
		documentObject: boolean = true;

		constructor() {
			super(app.CSections.HeaderFooterBoundary.name);
			this.sectionProperties.pageRectangle = null;
			this.sectionProperties.headerRectangle = null;
			this.sectionProperties.footerRectangle = null;
		}

		public onInitialize(): void {
			app.map.on('zoomend', () => {
				this.setPositionAndSize();
				app.sectionContainer.requestReDraw();
			});
		}

		public update(json: HeaderFooterBoundaryMessage): void {
			if (json.hide) {
				this.sectionProperties.pageRectangle = null;
				this.sectionProperties.headerRectangle = null;
				this.sectionProperties.footerRectangle = null;
			} else {
				this.sectionProperties.pageRectangle = this.parseRectangle(json.page);
				this.sectionProperties.headerRectangle = this.parseRectangle(
					json.header,
				);
				this.sectionProperties.footerRectangle = this.parseRectangle(
					json.footer,
				);
			}

			this.setPositionAndSize();
			app.sectionContainer.requestReDraw();
		}

		private parseRectangle(rect: string | undefined): Array<number> | null {
			if (!rect) return null;

			var matches = rect.match(/-?\d+/g);
			if (!matches || matches.length < 4) return null;

			return [
				parseInt(matches[0]),
				parseInt(matches[1]),
				parseInt(matches[2]),
				parseInt(matches[3]),
			];
		}

		private setPositionAndSize(): void {
			var page: Array<number> = this.sectionProperties.pageRectangle;
			if (!page) {
				this.size = [0, 0];
				this.setPosition(0, 0);
				return;
			}
			this.size = [
				Math.round(page[2] * app.twipsToPixels),
				Math.round(page[3] * app.twipsToPixels),
			];
			this.setPosition(
				Math.round(page[0] * app.twipsToPixels),
				Math.round(page[1] * app.twipsToPixels),
			);
		}

		public onResize(): void {
			this.setPositionAndSize();
		}

		private separatorY(yTwip: number): number {
			return Math.round(yTwip * app.twipsToPixels) - this.position[1];
		}

		private drawSeparator(y: number): void {
			this.context.save();
			this.context.strokeStyle = window.prefs.getBoolean('darkTheme')
				? '#ffffff'
				: '#000000';
			this.context.lineWidth = this.getLineWidth();
			this.context.setLineDash([
				120 * app.twipsToPixels,
				90 * app.twipsToPixels,
			]);
			this.context.beginPath();
			this.context.moveTo(0, y);
			this.context.lineTo(this.size[0], y);
			this.context.stroke();
			this.context.restore();
		}

		private drawLabel(text: string, y: number, below: boolean): void {
			var dark: boolean = window.prefs.getBoolean('darkTheme');
			var fontSize: number = Math.round(195 * app.twipsToPixels);
			var paddingX: number = Math.round(120 * app.twipsToPixels);
			var paddingY: number = Math.round(60 * app.twipsToPixels);
			var marginLeft: number = Math.round(150 * app.twipsToPixels);

			this.context.save();
			this.context.font = fontSize + 'px monospace';

			var boxWidth: number =
				this.context.measureText(text).width + 2 * paddingX;
			var boxHeight: number = fontSize + 2 * paddingY;
			var halfLine: number = this.getLineWidth() / 2;
			var boxY: number = below ? y + halfLine : y - boxHeight - halfLine;

			this.context.fillStyle = dark ? '#404040' : '#d9d9d9';
			this.context.fillRect(marginLeft, boxY, boxWidth, boxHeight);

			this.context.fillStyle = dark ? '#ffffff' : '#000000';
			this.context.textAlign = 'left';
			this.context.textBaseline = 'middle';
			this.context.fillText(text, marginLeft + paddingX, boxY + boxHeight / 2);
			this.context.restore();
		}

		public onDraw(): void {
			if (!this.sectionProperties.pageRectangle) return;

			var header: Array<number> = this.sectionProperties.headerRectangle;
			if (header) {
				// boundary line = bottom edge of the header frame
				var headerY: number = this.separatorY(header[1] + header[3]);
				this.drawSeparator(headerY);
				this.drawLabel(_('Header'), headerY, true);
			}

			var footer: Array<number> = this.sectionProperties.footerRectangle;
			if (footer) {
				// boundary line = top edge of the footer frame
				var footerY: number = this.separatorY(footer[1]);
				this.drawSeparator(footerY);
				this.drawLabel(_('Footer'), footerY, false);
			}
		}
	}
}

app.definitions.HeaderFooterBoundarySection = cool.HeaderFooterBoundarySection;
