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

		/// The mapping that takes the unit square onto the range: the corner 0, 0 to its minimum
		/// and the corner 1, 1 to its maximum.
		static fromRange(range: Range2D): Matrix2D {
			return new Matrix2D(
				range.width,
				0,
				0,
				range.height,
				range.minX,
				range.minY,
			);
		}

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

		/// How long the transform makes a step of one along the x axis.
		lengthOfXAxis(): number {
			return Math.hypot(this.a, this.b);
		}

		/// How long the transform makes a step of one along the y axis.
		lengthOfYAxis(): number {
			return Math.hypot(this.c, this.d);
		}

		/// The angle in radians the transform turns the x axis by, as the canvas counts angles.
		rotation(): number {
			return Math.atan2(this.b, this.a);
		}

		/// Whether the other one is the same transform, to within the given difference in each of
		/// the six values.
		equals(other: Matrix2D, tolerance: number): boolean {
			const theirs = other.toArray();
			return this.toArray().every(
				(value: number, at: number) => Math.abs(value - theirs[at]) < tolerance,
			);
		}

		/// The transform that undoes this one, or null where this one cannot be undone, which is
		/// a transform that flattens everything onto a line or a point.
		invert(): Matrix2D | null {
			const determinant = this.a * this.d - this.b * this.c;
			if (!determinant) return null;

			return new Matrix2D(
				this.d / determinant,
				-this.b / determinant,
				-this.c / determinant,
				this.a / determinant,
				(this.c * this.f - this.d * this.e) / determinant,
				(this.b * this.e - this.a * this.f) / determinant,
			);
		}

		/// The point after the transform.
		apply(x: number, y: number): cool.Point {
			return new cool.Point(
				this.a * x + this.c * y + this.e,
				this.b * x + this.d * y + this.f,
			);
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
