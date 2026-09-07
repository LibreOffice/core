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
	/// A 2D affine transform with its six values in canvas order:
	///
	///     | a  c  e |
	///     | b  d  f |
	///     | 0  0  1 |
	///
	/// A matrix never changes once made. Each operation returns a new
	/// matrix that applies this one first and the operation after it.
	export class Matrix2D {
		constructor(
			public readonly a: number,
			public readonly b: number,
			public readonly c: number,
			public readonly d: number,
			public readonly e: number,
			public readonly f: number,
		) {}

		static readonly IDENTITY = new Matrix2D(1, 0, 0, 1, 0, 0);

		/// From the six values in canvas order [a, b, c, d, e, f]. Null
		/// when there are fewer than six.
		static fromArray(values: number[] | undefined): Matrix2D | null {
			if (!values || values.length < 6) return null;
			return new Matrix2D(
				values[0],
				values[1],
				values[2],
				values[3],
				values[4],
				values[5],
			);
		}

		/// This transform followed by outer.
		then(outer: Matrix2D): Matrix2D {
			return new Matrix2D(
				outer.a * this.a + outer.c * this.b,
				outer.b * this.a + outer.d * this.b,
				outer.a * this.c + outer.c * this.d,
				outer.b * this.c + outer.d * this.d,
				outer.a * this.e + outer.c * this.f + outer.e,
				outer.b * this.e + outer.d * this.f + outer.f,
			);
		}

		scale(x: number, y: number): Matrix2D {
			return this.then(new Matrix2D(x, 0, 0, y, 0, 0));
		}

		translate(x: number, y: number): Matrix2D {
			return this.then(new Matrix2D(1, 0, 0, 1, x, y));
		}

		/// Turn by the angle in radians about the given point.
		rotateAround(centerX: number, centerY: number, angle: number): Matrix2D {
			const cos = Math.cos(angle);
			const sin = Math.sin(angle);
			return this.then(
				new Matrix2D(
					cos,
					sin,
					-sin,
					cos,
					centerX - cos * centerX + sin * centerY,
					centerY - sin * centerX - cos * centerY,
				),
			);
		}

		/// The point after the transform.
		apply(x: number, y: number): { x: number; y: number } {
			return {
				x: this.a * x + this.c * y + this.e,
				y: this.b * x + this.d * y + this.f,
			};
		}

		/// Multiply the context's current transform by this one.
		applyTo(context: CanvasRenderingContext2D): void {
			context.transform(this.a, this.b, this.c, this.d, this.e, this.f);
		}

		toArray(): number[] {
			return [this.a, this.b, this.c, this.d, this.e, this.f];
		}
	}
}
