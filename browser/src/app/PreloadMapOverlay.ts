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
 * PreloadMapOverlay - a debug view of the tile store's state.
 *
 * It owns a canvas of its own above the document, rather than drawing as a
 * section into the document canvas, so that it can show tile state that
 * changes without the document repainting - pre-fetching, in particular.
 */

interface PreloadMapPart {
	part: PartNumber;
	range: cool.Bounds;
	voffset: number; // where this part's block starts down the map
}

interface PreloadMapLayout {
	sizePix: number;
	viewRange: cool.Bounds;
	viewWidth: number;
	viewHeight: number;
	parts: PreloadMapPart[];
	width: number;
	height: number;
}

class PreloadMapOverlay {
	// Where the map sits relative to the start of the document, in CSS pixels.
	private static originX: number = 50;
	private static originY: number = 200;

	private canvas: HTMLCanvasElement;
	private context: CanvasRenderingContext2D;
	private redrawHandle: number = 0;
	private boundOnChanged: () => void = this.onTileStateChanged.bind(this);

	constructor() {
		this.canvas = document.createElement('canvas');
		this.canvas.id = 'preload-map';
		document.getElementById('canvas-container').appendChild(this.canvas);
		this.context = this.canvas.getContext('2d');

		RenderManager.onTileStateChanged(this.boundOnChanged);
		// Scrolling moves the view rectangle, and changing part moves the map
		// to another part's tiles, without any tile itself changing.
		app.map.on('move zoomend resize updateparts setpart', this.boundOnChanged);

		this.onTileStateChanged();
	}

	destroy(): void {
		RenderManager.offTileStateChanged(this.boundOnChanged);
		app.map.off('move zoomend resize updateparts setpart', this.boundOnChanged);
		if (this.redrawHandle) cancelAnimationFrame(this.redrawHandle);
		this.redrawHandle = 0;
		this.canvas.remove();
		this.canvas = null;
		this.context = null;
	}

	/// Coalesce every change landing before the next frame into one draw.
	private onTileStateChanged(): void {
		if (this.redrawHandle) return;
		this.redrawHandle = requestAnimationFrame(() => {
			this.redrawHandle = 0;
			this.draw();
		});
	}

	/// How big a tile square is, which tile range each part shows, and where
	/// each part's block sits down the map.
	private computeLayout(): PreloadMapLayout {
		const docLayer: DocLayerInterface = app.map._docLayer;
		const ctx = docLayer._painter._paintContext();
		const tileRanges = ctx.paneBoundsList.map(
			RenderManager.pxBoundsToTileRange,
			RenderManager,
		);

		// Get the 'main' view
		const viewRange = tileRanges.length == 4 ? tileRanges[3] : tileRanges[0];
		const tileRange = new cool.Bounds(viewRange.min, viewRange.max);

		// stop annoying jitter as the view fits different numbers of tiles.
		const viewWidth = Math.floor(
			(app.sectionContainer.getWidth() + RenderManager.tileSize - 1) /
				RenderManager.tileSize,
		);
		const viewHeight = Math.floor(
			(app.sectionContainer.getHeight() + RenderManager.tileSize - 1) /
				RenderManager.tileSize,
		);

		// writer defaults
		let sizePix: number = 3;
		let numParts = 1;
		let enlargeX = 0.1;
		let enlargeY = 2;
		let mainYMultiply = 10.0;
		if (docLayer.isCalc()) {
			enlargeX = 2;
			enlargeY = 2;
			sizePix = 6;
			numParts = 3;
			mainYMultiply = 2;
		} else if (docLayer.isImpress()) {
			enlargeX = 0.5;
			enlargeY = 0.5;
			mainYMultiply = 0;
			sizePix = 6;
			numParts = 7;
		}

		// Snap a square to a whole number of device pixels, so that the grid
		// stays sharp at fractional device pixel ratios.
		sizePix = Math.round(sizePix * app.dpiScale) / app.dpiScale;

		// Enlarge in each dimension
		tileRange.min.x = tileRange.min.x - Math.floor(viewWidth * enlargeX);
		tileRange.max.x = tileRange.max.x + Math.floor(viewWidth * enlargeX);
		tileRange.min.y = tileRange.min.y - Math.floor(viewHeight * enlargeY);
		tileRange.max.y = tileRange.max.y + Math.floor(viewHeight * enlargeY);

		// The parts carry the part number tiles are keyed by, resolved from
		// the index. An index outside the document gets the invalid part -1.
		const preParts = (numParts - 1) / 2;
		const parts: PreloadMapPart[] = [];
		let voffset = 0;
		for (let i = 0; i < numParts; ++i) {
			const range = new cool.Bounds(tileRange.min, tileRange.max);
			// current view should be bigger vertically at least
			if (i === preParts) {
				range.min.y -= viewHeight * mainYMultiply;
				range.max.y += viewHeight * mainYMultiply;
			}
			parts.push({
				part: docLayer.getPartFromIndex(docLayer._selectedPart + i - preParts),
				range: range,
				voffset: voffset,
			});
			voffset += sizePix * (range.max.y - range.min.y + 4);
		}

		return {
			sizePix: sizePix,
			viewRange: viewRange,
			viewWidth: viewWidth,
			viewHeight: viewHeight,
			parts: parts,
			width: (tileRange.max.x - tileRange.min.x + 1) * sizePix,
			height: voffset,
		};
	}

	/// Place the element where the document starts, and size the backing store
	/// for the device pixel ratio. Drawing then works in CSS pixels.
	private place(layout: PreloadMapLayout): void {
		const scale = app.dpiScale;
		const anchor = app.activeDocument.activeLayout.documentAnchorPosition;
		const style = this.canvas.style;

		style.left = anchor[0] / scale + PreloadMapOverlay.originX + 'px';
		style.top = anchor[1] / scale + PreloadMapOverlay.originY + 'px';
		style.width = layout.width + 'px';
		style.height = layout.height + 'px';

		const width = Math.round(layout.width * scale);
		const height = Math.round(layout.height * scale);
		if (this.canvas.width !== width) this.canvas.width = width;
		if (this.canvas.height !== height) this.canvas.height = height;

		// Assigning width or height resets the transform, so set it each time.
		this.context.setTransform(scale, 0, 0, scale, 0, 0);
		this.context.clearRect(0, 0, layout.width, layout.height);
	}

	private draw(): void {
		if (!app.map || !app.map._docLayer) return;
		Util.ensureValue(app.activeDocument);

		const docLayer: DocLayerInterface = app.map._docLayer;
		const zoom = Math.round(app.map.getZoom());
		const layout = this.computeLayout();
		const canvas = this.context;

		this.place(layout);

		const sizePix = layout.sizePix;
		for (const partLayout of layout.parts) {
			const range = partLayout.range;
			for (let j = range.min.y; j <= range.max.y; ++j) {
				for (let i = range.min.x; i <= range.max.x; ++i) {
					if (i >= 0 && j >= 0 && partLayout.part >= 0) {
						const coords = new TileCoordData(
							i * RenderManager.tileSize,
							j * RenderManager.tileSize,
							zoom,
							partLayout.part,
							app.activeDocument.activeModes[0],
						);
						const tile: Tile = RenderManager.get(coords);

						if (!tile)
							canvas.fillStyle = 'rgba(128, 128, 128, 0.5)'; // grey
						// state of the tile
						else if (!tile.hasContent())
							canvas.fillStyle = 'rgba(255, 0, 0, 0.8)'; // red
						else if (tile.needsFetch())
							canvas.fillStyle = 'rgba(255, 255, 0, 0.8)'; // yellow
						else if (!tile.image)
							canvas.fillStyle = 'rgba(0, 96, 0, 0.8)'; // dark green
						else if (tile.distanceFromView <= 0)
							canvas.fillStyle = 'rgba(0, 255, 0, 0.5)'; // visible
						else {
							const expFactor = RenderManager.getExpiryFactor(tile);
							if (expFactor >= 0)
								// expiry shown by more blue, and less green
								canvas.fillStyle =
									'rgba(0, ' +
									Math.round(192 * (1.0 - expFactor)) +
									', ' +
									Math.round(96 * expFactor) +
									', 0.8)';
						}
					} // outside document range
					else canvas.fillStyle = 'rgba(0, 0, 0, 0.3)'; // dark grey

					canvas.fillRect(
						(i - range.min.x) * sizePix,
						(j - range.min.y) * sizePix + partLayout.voffset,
						sizePix,
						sizePix,
					);
				}
			}

			// view rectangle
			if (partLayout.part == docLayer.getSelectedPart()) {
				// viewport in tiles - not that accurate.
				canvas.strokeStyle = 'rgba(0, 0, 0, 0.5)';
				canvas.lineWidth = 1.0;
				canvas.strokeRect(
					(layout.viewRange.min.x - range.min.x) * sizePix,
					(layout.viewRange.min.y - range.min.y) * sizePix + partLayout.voffset,
					layout.viewWidth * sizePix,
					layout.viewHeight * sizePix,
				);
			}
		}
	}
}

app.definitions.preloadMapOverlay = PreloadMapOverlay;
