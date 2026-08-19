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
	/// A canvas a subtree is drawn on before the result lands on the
	/// target. Clipping a target to a rectangle, reading the scale of its
	/// transform and laying out repeated tiles sit here too.
	export class VectorScratchCanvases {
		private _canvas: HTMLCanvasElement | undefined;

		// The scratch canvas sized to the target and cleared.
		context(width: number, height: number): CanvasRenderingContext2D | null {
			let canvas = this._canvas;
			if (!canvas) {
				canvas = document.createElement('canvas');
				this._canvas = canvas;
			}
			const scratch = canvas.getContext('2d');
			if (!scratch) return null;
			if (canvas.width !== width || canvas.height !== height) {
				// Sizing a canvas also clears it and resets its state.
				canvas.width = width;
				canvas.height = height;
			} else {
				scratch.setTransform(1, 0, 0, 1, 0, 0);
				scratch.clearRect(0, 0, width, height);
			}
			return scratch;
		}

		// Pixel for pixel onto the target, whatever transform the
		// target has.
		drawOnto(
			context: CanvasRenderingContext2D,
			scratch: CanvasRenderingContext2D,
		): void {
			context.save();
			context.setTransform(1, 0, 0, 1, 0, 0);
			context.drawImage(scratch.canvas, 0, 0);
			context.restore();
		}

		// Pixels per twip along the x axis of the active transform.
		// Canvas measures a blur radius in pixels whatever the transform
		// says, and a hairline stays one pixel wide at every zoom.
		static pixelsPerUnit(context: CanvasRenderingContext2D): number {
			const matrix = context.getTransform();
			const scale = Math.hypot(matrix.a, matrix.b);
			return scale > 0 ? scale : 1;
		}

		// Tiles of a repeating fill over the unit square, as the engine
		// lays them out: rows or columns step back to cover the near
		// edge, and an offset shifts every other one.
		static iterateTiles(
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

		/// Clip a target to a rectangle, in the coordinates it draws in.
		static clipToRange(
			context: CanvasRenderingContext2D,
			range: Range2D,
		): void {
			context.beginPath();
			context.rect(range.minX, range.minY, range.width, range.height);
			context.clip();
		}
	}
}
