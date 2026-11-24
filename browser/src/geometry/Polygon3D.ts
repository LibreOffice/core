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

namespace cool {
	/// A polygon of straight edges in 3D, as the drawing layer's B3DPolygon: its points in order,
	/// and whether the last one goes back to the first. A polygon never changes once made, each
	/// operation returns a new one.
	export class Polygon3D {
		constructor(
			public readonly points: Point3D[],
			public readonly closed: boolean = true,
		) {}

		/// From the coordinates one after another, x, y then z.
		static fromFlat(values: number[], closed: boolean): Polygon3D {
			const points: Point3D[] = [];
			for (let at = 0; at + 2 < values.length; at += 3)
				points.push(new Point3D(values[at], values[at + 1], values[at + 2]));
			return new Polygon3D(points, closed);
		}

		/// The normal by Newell's way, which holds for any planar polygon, convex or not. Its
		/// length is twice the area of the polygon, nothing for one with no area.
		normal(): Point3D {
			let x = 0;
			let y = 0;
			let z = 0;
			const points = this.points;
			for (let at = 0; at < points.length; ++at) {
				const one = points[at];
				const next = points[(at + 1) % points.length];
				x += (one.y - next.y) * (one.z + next.z);
				y += (one.z - next.z) * (one.x + next.x);
				z += (one.x - next.x) * (one.y + next.y);
			}
			return new Point3D(x, y, z);
		}

		/// The length of the longest edge, the one back to the start included.
		longestEdge(): number {
			let longest = 0;
			for (let at = 0; at < this.points.length; ++at)
				longest = Math.max(
					longest,
					this.points[at]
						.minus(this.points[(at + 1) % this.points.length])
						.length(),
				);
			return longest;
		}

		/// Whether all points lie on one line or in one place, so the polygon has no area.
		isFlat(): boolean {
			const longest = this.longestEdge();
			return this.normal().length() <= 1e-9 * longest * longest;
		}

		transformed(mapping: Matrix3D): Polygon3D {
			return new Polygon3D(
				this.points.map((point: Point3D) => mapping.apply(point)),
				this.closed,
			);
		}

		reversed(): Polygon3D {
			return new Polygon3D(this.points.slice().reverse(), this.closed);
		}

		/// Whether the other one has the same points in the same order, each within the
		/// tolerance.
		equals(other: Polygon3D, tolerance: number): boolean {
			return (
				this.points.length === other.points.length &&
				this.points.every(
					(point: Point3D, at: number) =>
						point.minus(other.points[at]).length() <= tolerance,
				)
			);
		}

		/// The polygon seen from the front, its depth left out.
		flattened(): Polygon2D {
			return new Polygon2D(
				this.points.map((point: Point3D) => new Point(point.x, point.y)),
				this.closed,
			);
		}
	}

	/// Several polygons in 3D that make one area together, as the drawing layer's
	/// B3DPolyPolygon: a face with its holes, or the outline of a slice.
	export class PolyPolygon3D {
		constructor(public readonly polygons: Polygon3D[]) {}

		transformed(mapping: Matrix3D): PolyPolygon3D {
			return new PolyPolygon3D(
				this.polygons.map((polygon: Polygon3D) => polygon.transformed(mapping)),
			);
		}

		/// Each polygon turned round, as the drawing layer flips a face to look the other way.
		reversed(): PolyPolygon3D {
			return new PolyPolygon3D(
				this.polygons.map((polygon: Polygon3D) => polygon.reversed()),
			);
		}

		equals(other: PolyPolygon3D, tolerance: number): boolean {
			return (
				this.polygons.length === other.polygons.length &&
				this.polygons.every((polygon: Polygon3D, at: number) =>
					polygon.equals(other.polygons[at], tolerance),
				)
			);
		}

		flattened(): PolyPolygon2D {
			return new PolyPolygon2D(
				this.polygons.map((polygon: Polygon3D) => polygon.flattened()),
			);
		}
	}
}
