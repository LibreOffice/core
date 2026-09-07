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
	/// An axis-aligned rectangle in twips, held as its two corners. A
	/// range never changes once made.
	export class Range2D {
		constructor(
			public readonly minX: number,
			public readonly minY: number,
			public readonly maxX: number,
			public readonly maxY: number,
		) {}

		/// From the four values [minX, minY, maxX, maxY]. Null when there
		/// are fewer than four.
		static fromArray(values: number[] | undefined): Range2D | null {
			if (!values || values.length < 4) return null;
			return new Range2D(values[0], values[1], values[2], values[3]);
		}

		get width(): number {
			return this.maxX - this.minX;
		}

		get height(): number {
			return this.maxY - this.minY;
		}

		get centerX(): number {
			return (this.minX + this.maxX) / 2;
		}

		get centerY(): number {
			return (this.minY + this.maxY) / 2;
		}

		/// True when the range covers no area, including when a corner is
		/// not a number.
		isEmpty(): boolean {
			return !(this.maxX > this.minX) || !(this.maxY > this.minY);
		}

		toArray(): number[] {
			return [this.minX, this.minY, this.maxX, this.maxY];
		}
	}
}
