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
	/// Renders the image fill primitives with Canvas 2D drawing
	/// operations.
	export class VectorFillGraphicPrimitiveRenderer {
		private _bitmapLookup: BitmapLookup | undefined;

		constructor(bitmapLookup?: BitmapLookup) {
			this._bitmapLookup = bitmapLookup;
		}

		renderFillGraphic(
			context: CanvasRenderingContext2D,
			primitive: FillGraphicPrimitive,
		): void {
			const fill = primitive.fillGraphic;
			const matrix = primitive.matrix;
			if (!fill || !matrix || matrix.length < 6) return;

			const image = this._lookupImage(fill.checksum);
			if (!image) return;

			context.save();
			if (primitive.transparency)
				context.globalAlpha = 1 - primitive.transparency;
			context.transform(
				matrix[0],
				matrix[1],
				matrix[2],
				matrix[3],
				matrix[4],
				matrix[5],
			);
			this._drawFillGraphic(context, fill, image);
			context.restore();
		}

		renderPolyPolygonGraphic(
			context: CanvasRenderingContext2D,
			primitive: PolyPolygonGraphicPrimitive,
		): void {
			const fill = primitive.fillGraphic;
			const path = primitive.path;
			const area = primitive.bounds;
			if (!fill || !path || !area || area.length < 4) return;

			const image = this._lookupImage(fill.checksum);
			if (!image) return;

			const width = area[2] - area[0];
			const height = area[3] - area[1];
			if (!(width > 0 && height > 0)) return;

			context.save();
			if (primitive.transparency)
				context.globalAlpha = 1 - primitive.transparency;
			context.clip(new Path2D(path), 'evenodd');
			// The image sits in the unit square of the polygon's bounds.
			context.translate(area[0], area[1]);
			context.scale(width, height);
			this._drawFillGraphic(
				context,
				fill,
				image,
				primitive.definitionRange,
				area,
			);
			context.restore();
		}

		// Draw an image fill inside the unit square. A definition range
		// other than the area being filled moves the image so it keeps
		// the position and spacing it was given there.
		private _drawFillGraphic(
			context: CanvasRenderingContext2D,
			fill: FillGraphicAttribute,
			image: HTMLImageElement,
			definitionRange?: number[],
			area?: number[],
		): void {
			let range = fill.graphicRange ?? [0, 0, 1, 1];
			if (range.length < 4) return;

			if (definitionRange && area && definitionRange.length >= 4) {
				const definitionWidth = definitionRange[2] - definitionRange[0];
				const definitionHeight = definitionRange[3] - definitionRange[1];
				const areaWidth = area[2] - area[0];
				const areaHeight = area[3] - area[1];
				if (
					areaWidth > 0 &&
					areaHeight > 0 &&
					(definitionWidth !== areaWidth ||
						definitionHeight !== areaHeight ||
						definitionRange[0] !== area[0] ||
						definitionRange[1] !== area[1])
				) {
					const toArea = (
						value: number,
						definitionStart: number,
						definitionSize: number,
						areaStart: number,
						areaSize: number,
					): number =>
						(definitionStart + value * definitionSize - areaStart) / areaSize;
					range = [
						toArea(
							range[0],
							definitionRange[0],
							definitionWidth,
							area[0],
							areaWidth,
						),
						toArea(
							range[1],
							definitionRange[1],
							definitionHeight,
							area[1],
							areaHeight,
						),
						toArea(
							range[2],
							definitionRange[0],
							definitionWidth,
							area[0],
							areaWidth,
						),
						toArea(
							range[3],
							definitionRange[1],
							definitionHeight,
							area[1],
							areaHeight,
						),
					];
				}
			}

			const tileWidth = range[2] - range[0];
			const tileHeight = range[3] - range[1];
			if (!(tileWidth > 0 && tileHeight > 0)) return;

			if (!fill.tiling) {
				context.drawImage(image, range[0], range[1], tileWidth, tileHeight);
				return;
			}

			const pixels = VectorScratchCanvases.pixelsPerUnit(context);
			// A tile under a pixel shows nothing and there would be one
			// per pixel.
			if (tileWidth * pixels < 1 || tileHeight * pixels < 1) return;

			this._iterateTiles(
				range,
				fill.offsetX ?? 0,
				fill.offsetY ?? 0,
				(x: number, y: number) => {
					context.drawImage(image, x, y, tileWidth, tileHeight);
				},
			);
		}

		// Tiles of a repeating fill over the unit square, as the engine
		// lays them out: rows or columns step back to cover the near
		// edge, and an offset shifts every other one.
		private _iterateTiles(
			range: number[],
			offsetX: number,
			offsetY: number,
			visit: (x: number, y: number) => void,
		): void {
			const width = range[2] - range[0];
			const height = range[3] - range[1];
			let startX = range[0];
			let startY = range[1];
			let columnIndex = 0;
			let rowIndex = 0;

			if (startX > 0) {
				const back = Math.floor(startX / width) + 1;
				columnIndex -= back;
				startX -= back * width;
			}
			if (startX + width < 0) {
				const forward = Math.floor(-startX / width);
				columnIndex += forward;
				startX += forward * width;
			}
			if (startY > 0) {
				const back = Math.floor(startY / height) + 1;
				rowIndex -= back;
				startY -= back * height;
			}
			if (startY + height < 0) {
				const forward = Math.floor(-startY / height);
				rowIndex += forward;
				startY += forward * height;
			}

			if (offsetY !== 0) {
				for (let x = startX; x < 1; x += width, columnIndex++) {
					const first =
						columnIndex % 2 ? startY - height + offsetY * height : startY;
					for (let y = first; y < 1; y += height) visit(x, y);
				}
				return;
			}

			for (let y = startY; y < 1; y += height, rowIndex++) {
				const first = rowIndex % 2 ? startX - width + offsetX * width : startX;
				for (let x = first; x < 1; x += width) visit(x, y);
			}
		}

		// Null until the image has arrived and decoded.
		private _lookupImage(checksum: number): HTMLImageElement | null {
			if (!this._bitmapLookup) return null;
			const image = this._bitmapLookup(checksum);
			if (!image || !image.complete) return null;
			return image;
		}
	}
}
