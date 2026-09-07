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
	/// target. Clipping a target to a rectangle sits here too.
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
