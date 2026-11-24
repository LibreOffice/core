/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

declare var glMatrix: any;

namespace cool {
	/// A 3D transform as a 4x4 matrix, read from and written as its sixteen values row by row,
	/// as the engine writes them. The last row is the one of the perspective; where it is other
	/// than 0, 0, 0, 1, a point is divided by its fourth value after the transform, as the
	/// drawing layer does it.
	///
	/// The work is gl-matrix's, on arrays of doubles, so the numbers of a camera keep their
	/// precision; gl-matrix holds a matrix column by column. A matrix never changes once made.
	/// Each operation returns a new matrix that applies this one first and the operation after
	/// it.
	export class Matrix3D {
		private constructor(private readonly columns: Float64Array) {}

		static readonly IDENTITY = Matrix3D.fromArray([
			1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
		]) as Matrix3D;

		/// The move by that much along each axis.
		static translation(x: number, y: number, z: number): Matrix3D {
			return Matrix3D.fromArray([
				1,
				0,
				0,
				x,
				0,
				1,
				0,
				y,
				0,
				0,
				1,
				z,
				0,
				0,
				0,
				1,
			]) as Matrix3D;
		}

		/// The turn about the y axis by the angle in radians, turned the way the drawing layer
		/// turns: the x axis towards the minus z axis.
		static turnAboutY(angle: number): Matrix3D {
			const cos = Math.cos(angle);
			const sin = Math.sin(angle);
			return Matrix3D.fromArray([
				cos,
				0,
				sin,
				0,
				0,
				1,
				0,
				0,
				-sin,
				0,
				cos,
				0,
				0,
				0,
				0,
				1,
			]) as Matrix3D;
		}

		/// From the sixteen values row by row. Null when there are fewer.
		static fromArray(values: number[] | undefined): Matrix3D | null {
			if (!values || values.length < 16) return null;
			const columns = new Float64Array(16);
			glMatrix.mat4.transpose(columns, values.slice(0, 16));
			return new Matrix3D(columns);
		}

		get(row: number, column: number): number {
			return this.columns[column * 4 + row];
		}

		/// This transform followed by outer.
		then(outer: Matrix3D): Matrix3D {
			const product = new Float64Array(16);
			glMatrix.mat4.multiply(product, outer.columns, this.columns);
			return new Matrix3D(product);
		}

		/// The point after the transform, divided by its fourth value.
		apply(point: Point3D): Point3D {
			const out = new Float64Array(3);
			glMatrix.vec3.transformMat4(
				out,
				[point.x, point.y, point.z],
				this.columns,
			);
			return new Point3D(out[0], out[1], out[2]);
		}

		/// The transform that undoes this one, or null where this one cannot be undone.
		invert(): Matrix3D | null {
			const inverse = new Float64Array(16);
			return glMatrix.mat4.invert(inverse, this.columns) ?
					new Matrix3D(inverse)
				:	null;
		}

		toArray(): number[] {
			const rows = new Float64Array(16);
			glMatrix.mat4.transpose(rows, this.columns);
			return Array.from(rows);
		}
	}
}
