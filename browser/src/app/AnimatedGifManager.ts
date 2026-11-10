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
 * AnimatedGifManager plays the selected animated images (for example animated
 * GIFs) over the document while editing. The tiles show the first frame of
 * every animated image, so the ones not selected stand still at it, in their
 * place in the stacking order.
 */

class AnimatedGifManager {
	private _map: any;

	private _sections: Map<number, AnimatedGifSection> = new Map();
	private _presentationInfo: any = null;

	constructor(map: any) {
		this._map = map;
		map.on('docloaded', this._onDocLoaded, this);
		map.on('presentationinfo', this._onPresentationInfo, this);
		map.on('updateparts', this._onUpdateParts, this);
		map.on('zoomend', this._onZoomEnd, this);
		app.events.on('contextchange', this._onContextChange.bind(this));
		app.events.on(
			'graphicselectionchanged',
			this._onGraphicSelectionChanged.bind(this),
		);
	}

	// Ask for the presentation info on load so animated images already in the
	// file start playing without a prior edit.
	private _onDocLoaded() {
		const docType = this._map.getDocType();
		if (
			(docType === 'presentation' || docType === 'drawing') &&
			app.socket.connected()
		) {
			app.socket.sendMessage('getpresentationinfo');
		}
	}

	private _onPresentationInfo(info: any) {
		this._presentationInfo = info;
		this._reconcileCurrentPart();
	}

	private _onUpdateParts() {
		this._reconcileCurrentPart();
	}

	private _onContextChange() {
		this._reconcileCurrentPart();
	}

	private _onGraphicSelectionChanged() {
		this._reconcileCurrentPart();
	}

	// Picks the selected gifs of the part on screen out of the stored
	// presentation info and brings the overlays in line with them.
	private _reconcileCurrentPart() {
		if (!this._presentationInfo || !this._presentationInfo.slides) return;
		const docLayer = this._map._docLayer;
		const part = docLayer ? docLayer._selectedPart : 0;
		// The presentation info lists the slides of the normal view.
		// The master and notes views show other pages under the same part numbers
		const normalView =
			!app.activeDocument || app.activeDocument.isModeActive(0);
		const slide = normalView ? this._presentationInfo.slides[part] : null;
		const gifs: GifInfo[] = (slide && slide.gifs) || [];
		const selected = GraphicSelection.selectedObjectIDs;
		this._reconcile(gifs.filter((gif) => selected.includes(gif.id)));
	}

	// Brings the overlay sections in line with the given list: repositions the
	// ones still present, adds the new ones, and removes the rest.
	private _reconcile(gifs: GifInfo[]) {
		const wanted = new Set<number>();
		let changed = false;

		for (const gif of gifs) {
			wanted.add(gif.id);
			const rectangle = [gif.x, gif.y, gif.width, gif.height];
			const mirror = gif.mirror ?? 0;
			const existing = this._sections.get(gif.id);

			if (existing && existing.getUrl() === gif.url) {
				if (
					existing.mirror !== mirror ||
					!this._sameRectangle(
						existing.sectionProperties.twipRectangle,
						rectangle,
					)
				) {
					existing.mirror = mirror;
					existing.updateTwipRectangle(rectangle);
					changed = true;
				}
			} else {
				if (existing) {
					app.sectionContainer.removeSection(existing.name);
					this._sections.delete(gif.id);
				}
				const section = new AnimatedGifSection(
					'AnimatedGif-' + String(gif.id),
					rectangle,
					gif.url,
				);
				section.mirror = mirror;
				this._sections.set(gif.id, section);
				app.sectionContainer.addSection(section);
				changed = true;
			}
		}

		for (const [id, section] of this._sections) {
			if (!wanted.has(id)) {
				app.sectionContainer.removeSection(section.name);
				this._sections.delete(id);
				changed = true;
			}
		}

		if (changed) app.sectionContainer.requestReDraw();
	}

	private _sameRectangle(a: number[], b: number[]): boolean {
		return a.length === b.length && a.every((value, i) => value === b[i]);
	}

	// The twip-to-pixel scale changes with zoom, so recompute every overlay's
	// position and size from its stored rectangle.
	private _onZoomEnd() {
		if (this._sections.size === 0) return;
		for (const section of this._sections.values()) {
			section.updateTwipRectangle(section.sectionProperties.twipRectangle);
		}
		app.sectionContainer.requestReDraw();
	}
}
