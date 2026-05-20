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

class ServerCommand {
	// TODO: make mandatory fields non-optional.
	public filename?: string;
	public url?: string;
	public type?: string;
	public mode?: number;
	public id?: string;
	public errorCmd?: string;
	public errorCode?: string;
	public errorKind?: string;
	public errorDetail?: string;
	public jail?: string;
	public dir?: string;
	public downloadid?: string;
	public name?: string;
	public port?: string;
	public font?: string;
	public char?: string;
	public viewid?: string;
	public nviewid?: string;
	public params?: string[];
	public rendercount?: number;
	public wireId?: string;
	public title?: string;
	public dialogwidth?: string;
	public dialogheight?: string;
	public rectangle?: string;
	public hiddenparts?: number[];
	public rtlParts?: number[];
	public protectedParts?: number[];
	public hash?: string;
	public nopng?: boolean;
	public username?: string;
	public pageRectangleList?: number[][];
	public lastcolumn?: number;
	public lastrow?: number;
	public readonly?: number;
	public externallinksdisabled?: boolean;

	public x?: number;
	public y?: number;
	public tileWidth?: number;
	public tileHeight?: number;
	public width?: number;
	public height?: number;
	public zoom?: number;
	public part?: number;
	public parts?: number;
	public selectedPart?: number;

	constructor(msg: string, map: MapZoomInterface) {
		this.parse(msg, map);
	}

	public static getParameterValue(s: string): string | undefined {
		const i = s.indexOf('=');
		if (i === -1) return undefined;
		return s.substring(i + 1);
	}

	private parse(msg: string, map: MapZoomInterface): void {
		const tokens = msg.split(/[ \n]+/);
		for (let i = 0; i < tokens.length; i++) {
			if (tokens[i].substring(0, 9) === 'tileposx=') {
				this.x = parseInt(tokens[i].substring(9));
			} else if (tokens[i].substring(0, 9) === 'tileposy=') {
				this.y = parseInt(tokens[i].substring(9));
			} else if (tokens[i].substring(0, 2) === 'x=') {
				this.x = parseInt(tokens[i].substring(2));
			} else if (tokens[i].substring(0, 2) === 'y=') {
				this.y = parseInt(tokens[i].substring(2));
			} else if (tokens[i].substring(0, 10) === 'tilewidth=') {
				this.tileWidth = parseInt(tokens[i].substring(10));
			} else if (tokens[i].substring(0, 11) === 'tileheight=') {
				this.tileHeight = parseInt(tokens[i].substring(11));
			} else if (tokens[i].substring(0, 6) === 'width=') {
				this.width = parseInt(tokens[i].substring(6));
			} else if (tokens[i].substring(0, 7) === 'height=') {
				this.height = parseInt(tokens[i].substring(7));
			} else if (tokens[i].substring(0, 5) === 'part=') {
				this.part = parseInt(tokens[i].substring(5));
			} else if (tokens[i].substring(0, 6) === 'parts=') {
				this.parts = parseInt(tokens[i].substring(6));
			} else if (tokens[i].substring(0, 5) === 'mode=') {
				this.mode = parseInt(tokens[i].substring(5));
			} else if (tokens[i].substring(0, 8) === 'current=') {
				this.selectedPart = parseInt(tokens[i].substring(8));
			} else if (tokens[i].substring(0, 3) === 'id=') {
				// remove newline characters
				this.id = tokens[i].substring(3).replace(/(\r\n|\n|\r)/gm, '');
			} else if (tokens[i].substring(0, 5) === 'type=') {
				// remove newline characters
				this.type = tokens[i].substring(5).replace(/(\r\n|\n|\r)/gm, '');
			} else if (tokens[i].substring(0, 4) === 'cmd=') {
				this.errorCmd = tokens[i].substring(4);
			} else if (tokens[i].substring(0, 5) === 'code=') {
				this.errorCode = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 5) === 'kind=') {
				this.errorKind = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 12) === 'errordetail=') {
				this.errorDetail = decodeURIComponent(tokens[i].substring(12));
			} else if (tokens[i].substring(0, 5) === 'jail=') {
				this.jail = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 4) === 'dir=') {
				this.dir = tokens[i].substring(4);
			} else if (tokens[i].substring(0, 11) === 'downloadid=') {
				this.downloadid = tokens[i].substring(11);
			} else if (tokens[i].substring(0, 5) === 'name=') {
				this.name = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 9) === 'filename=') {
				this.filename = tokens[i].substring(9);
			} else if (tokens[i].substring(0, 5) === 'port=') {
				this.port = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 5) === 'font=') {
				this.font = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 5) === 'char=') {
				this.char = tokens[i].substring(5);
			} else if (tokens[i].substring(0, 4) === 'url=') {
				this.url = tokens[i].substring(4);
			} else if (tokens[i].substring(0, 7) === 'viewid=') {
				this.viewid = tokens[i].substring(7);
			} else if (tokens[i].substring(0, 8) === 'nviewid=') {
				this.nviewid = tokens[i].substring(8);
			} else if (tokens[i].substring(0, 7) === 'params=') {
				this.params = tokens[i].substring(7).split(',');
			} else if (tokens[i].substring(0, 12) === 'rendercount=') {
				this.rendercount = parseInt(tokens[i].substring(12));
			} else if (tokens[i].startsWith('wid=')) {
				this.wireId = ServerCommand.getParameterValue(tokens[i]);
			} else if (tokens[i].substring(0, 6) === 'title=') {
				this.title = tokens[i].substring(6);
			} else if (tokens[i].substring(0, 12) === 'dialogwidth=') {
				this.dialogwidth = tokens[i].substring(12);
			} else if (tokens[i].substring(0, 13) === 'dialogheight=') {
				this.dialogheight = tokens[i].substring(13);
			} else if (tokens[i].substring(0, 10) === 'rectangle=') {
				this.rectangle = tokens[i].substring(10);
			} else if (tokens[i].substring(0, 12) === 'hiddenparts=') {
				const hiddenparts = tokens[i].substring(12).split(',');
				this.hiddenparts = hiddenparts.map((item: string) => parseInt(item));
			} else if (tokens[i].startsWith('rtlparts=')) {
				const rtlParts = tokens[i].substring(9).split(',');
				this.rtlParts = rtlParts.map((item: string) => parseInt(item));
			} else if (tokens[i].startsWith('protectedparts=')) {
				const protectedParts = tokens[i].substring(15).split(',');
				this.protectedParts = protectedParts.map((item: string) =>
					parseInt(item),
				);
			} else if (tokens[i].startsWith('hash=')) {
				this.hash = tokens[i].substring('hash='.length);
			} else if (tokens[i] === 'nopng') {
				this.nopng = true;
			} else if (tokens[i].substring(0, 9) === 'username=') {
				this.username = tokens[i].substring(9);
			} else if (tokens[i].startsWith('pagerectangles=')) {
				const pageRectangleList = tokens[i].substring(15).split(';');
				this.pageRectangleList = pageRectangleList.map(function (
					rect: string,
				): number[] {
					const rectCoords = rect.split(',');
					return [
						parseInt(rectCoords[0]),
						parseInt(rectCoords[1]),
						parseInt(rectCoords[2]),
						parseInt(rectCoords[3]),
					];
				});
			} else if (tokens[i].startsWith('lastcolumn=')) {
				this.lastcolumn = parseInt(tokens[i].substring(11));
			} else if (tokens[i].startsWith('lastrow=')) {
				this.lastrow = parseInt(tokens[i].substring(8));
			} else if (tokens[i].startsWith('readonly=')) {
				this.readonly = parseInt(tokens[i].substring(9));
			}
		}
		if (this.tileWidth && this.tileHeight && map._docLayer) {
			const defaultZoom = map.options.zoom;
			const scale = this.tileWidth / map._docLayer.options.tileWidthTwips;
			// scale = 1.2 ^ (defaultZoom - zoom)
			// zoom = defaultZoom -log(scale) / log(1.2)
			this.zoom = Math.round(defaultZoom - Math.log(scale) / Math.log(1.2));
		}
	}
}
