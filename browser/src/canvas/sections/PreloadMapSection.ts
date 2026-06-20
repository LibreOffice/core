/* global Proxy _ */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

class PreloadMapSection extends CanvasSectionObject {
	interactable: boolean = false;
	anchor: string[] = ['top', 'left'];
	processingOrder: number = app.CSections.Debug.PreloadMap.processingOrder;
	drawingOrder: number = app.CSections.Debug.PreloadMap.drawingOrder;
	zIndex: number = app.CSections.Debug.PreloadMap.zIndex;
	boundToSection: string = 'tiles';

	constructor() {
		super(app.CSections.Debug.PreloadMap.name);
	}

	onDraw(
		frameCount?: number,
		elapsedTime?: number,
		subsetBounds?: Bounds,
	): void {
		Util.ensureValue(app.activeDocument);
		var docLayer = app.map._docLayer;
		var ctx = docLayer._painter._paintContext();

		var zoom = Math.round(app.map.getZoom());
		var part = docLayer._selectedPart;
		var tileRanges = ctx.paneBoundsList.map(
			RenderManager.pxBoundsToTileRange,
			RenderManager,
		);

		// Get the 'main' view
		var viewRange = tileRanges.length == 4 ? tileRanges[3] : tileRanges[0];

		var canvas = this.context;

		var tileRange = new cool.Bounds(viewRange.min, viewRange.max);

		// stop annoying jitter as the view fits different numbers of tiles.
		var viewWidth = Math.floor(
			(app.sectionContainer.getWidth() + RenderManager.tileSize - 1) /
				RenderManager.tileSize,
		);
		var viewHeight = Math.floor(
			(app.sectionContainer.getHeight() + RenderManager.tileSize - 1) /
				RenderManager.tileSize,
		);

		// writer defaults
		var sizePix: number = 3;
		var numParts = 1;
		var enlargeX = 0.1;
		var enlargeY = 2;
		var mainYMultiply = 10.0;
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

		// Enlarge in each dimension
		tileRange.min.x = tileRange.min.x - Math.floor(viewWidth * enlargeX);
		tileRange.max.x = tileRange.max.x + Math.floor(viewWidth * enlargeX);
		tileRange.min.y = tileRange.min.y - Math.floor(viewHeight * enlargeY);
		tileRange.max.y = tileRange.max.y + Math.floor(viewHeight * enlargeY);

		var preParts = (numParts - 1) / 2;
		var partBounds = new Array(numParts);
		for (var i = 0; i < partBounds.length; ++i) {
			partBounds[i] = new cool.Bounds(tileRange.min, tileRange.max);
			partBounds[i].part = part + i - preParts;
		}

		// current view should be bigger vertically at least
		partBounds[preParts].min.y -= viewHeight * mainYMultiply;
		partBounds[preParts].max.y += viewHeight * mainYMultiply;

		var offx: number = 50;
		var offy: number = 200;
		var voffset: number = 0;
		for (var p = 0; p < partBounds.length; ++p) {
			var range = partBounds[p];
			for (var j = range.min.y; j <= range.max.y; ++j) {
				for (var i: number = range.min.x; i <= range.max.x; ++i) {
					if (i >= 0 && j >= 0 && range.part >= 0) {
						var coords = new TileCoordData(
							i * RenderManager.tileSize,
							j * RenderManager.tileSize,
							zoom,
							range.part,
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
						offx + (i - range.min.x) * sizePix,
						offy + (j - range.min.y) * sizePix + voffset,
						sizePix,
						sizePix,
					);
				}
			}
			// view rectangle
			if (range.part == part) {
				// viewport in tiles - not that accurate.
				canvas.strokeStyle = 'rgba(0, 0, 0, 0.5)';
				canvas.lineWidth = 1.0;
				canvas.strokeRect(
					offx + (viewRange.min.x - range.min.x) * sizePix,
					offy + (viewRange.min.y - range.min.y) * sizePix + voffset,
					viewWidth * sizePix,
					viewHeight * sizePix,
				);
			}

			voffset += sizePix * (range.max.y - range.min.y + 4);
		}
	}
}

app.definitions.preloadMapSection = PreloadMapSection;
