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
	/// The target pixels a scratch canvas covers. x and y are its top left
	/// corner.
	export interface ScratchRegion {
		x: number;
		y: number;
		width: number;
		height: number;
	}

	/// Scratch canvases for drawing a subtree before it lands on the target,
	/// one set per nesting level. Also clipping to a rectangle, the transform's
	/// scale, and tile layout.
	export class VectorScratchCanvases {
		private _canvases: HTMLCanvasElement[] = [];
		private _depth = 0;

		// Canvases per effect, and nesting levels that get a set.
		private static readonly _SLOTS_PER_EFFECT = 3;
		private static readonly _MAX_EFFECT_DEPTH = 3;

		/// Drop the canvases. The next drawing makes new ones.
		release(): void {
			this._canvases = [];
		}

		// Run an effect on the slots of its nesting level. Past the last level
		// the fallback runs instead.
		withEffectSlots(
			fallback: (() => void) | undefined,
			work: (slot: number) => void,
		): void {
			if (this._depth >= VectorScratchCanvases._MAX_EFFECT_DEPTH) {
				if (fallback) fallback();
				return;
			}

			const slot = this._depth * VectorScratchCanvases._SLOTS_PER_EFFECT;
			this._depth++;
			try {
				work(slot);
			} finally {
				this._depth--;
			}
		}

		// The slot's canvas at the given size, cleared.
		slotContext(
			slot: number,
			width: number,
			height: number,
		): CanvasRenderingContext2D | null {
			let canvas = this._canvases[slot];
			if (!canvas) {
				canvas = document.createElement('canvas');
				this._canvases[slot] = canvas;
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

		// Pixel for pixel onto the region, with the opacity on top of the
		// target's.
		drawOnto(
			context: CanvasRenderingContext2D,
			scratch: CanvasRenderingContext2D,
			region: ScratchRegion,
			opacity?: number,
		): void {
			context.save();
			context.setTransform(1, 0, 0, 1, 0, 0);
			if (opacity !== undefined && opacity < 1) context.globalAlpha *= opacity;
			context.drawImage(scratch.canvas, region.x, region.y);
			context.restore();
		}

		// The target's transform, with the region's corner as origin.
		static alignWith(
			scratch: CanvasRenderingContext2D,
			context: CanvasRenderingContext2D,
			region: ScratchRegion,
		): void {
			const matrix = context.getTransform();
			scratch.setTransform(
				matrix.a,
				matrix.b,
				matrix.c,
				matrix.d,
				matrix.e - region.x,
				matrix.f - region.y,
			);
		}

		// The whole target, as a region.
		static wholeTarget(context: CanvasRenderingContext2D): ScratchRegion {
			return {
				x: 0,
				y: 0,
				width: context.canvas.width,
				height: context.canvas.height,
			};
		}

		// The target pixels an effect draws on: its bounds, which hold the
		// whole effect, within the target grown by the spread so content just
		// off the edge still blurs in. Without bounds, the grown target. Null
		// when off the target.
		static effectRegion(
			context: CanvasRenderingContext2D,
			bounds: number[] | undefined,
			spread: number,
		): ScratchRegion | null {
			const width = context.canvas.width;
			const height = context.canvas.height;
			if (!(width > 0 && height > 0)) return null;

			// Cap the spread at the target size. One more pixel for the
			// antialiased edge.
			const limit = Math.max(width, height);
			const margin = Math.ceil(Math.min(Math.max(spread, 0), limit)) + 1;
			let minX = -margin;
			let minY = -margin;
			let maxX = width + margin;
			let maxY = height + margin;

			if (bounds && bounds.length >= 4) {
				const matrix = context.getTransform();
				let boundsMinX = Infinity;
				let boundsMinY = Infinity;
				let boundsMaxX = -Infinity;
				let boundsMaxY = -Infinity;
				for (const cornerX of [bounds[0], bounds[2]]) {
					for (const cornerY of [bounds[1], bounds[3]]) {
						const x = matrix.a * cornerX + matrix.c * cornerY + matrix.e;
						const y = matrix.b * cornerX + matrix.d * cornerY + matrix.f;
						boundsMinX = Math.min(boundsMinX, x);
						boundsMinY = Math.min(boundsMinY, y);
						boundsMaxX = Math.max(boundsMaxX, x);
						boundsMaxY = Math.max(boundsMaxY, y);
					}
				}
				// One pixel more for the antialiased edge.
				minX = Math.max(minX, boundsMinX - 1);
				minY = Math.max(minY, boundsMinY - 1);
				maxX = Math.min(maxX, boundsMaxX + 1);
				maxY = Math.min(maxY, boundsMaxY + 1);
			}

			const x = Math.floor(minX);
			const y = Math.floor(minY);
			const regionWidth = Math.ceil(maxX) - x;
			const regionHeight = Math.ceil(maxY) - y;
			if (!(regionWidth > 0 && regionHeight > 0)) return null;
			return { x: x, y: y, width: regionWidth, height: regionHeight };
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
