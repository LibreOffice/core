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
	/// A polygon of straight edges in 2D, as the drawing layer's B2DPolygon without curves: its
	/// points in order, and whether the last one goes back to the first. A polygon never changes
	/// once made, each operation returns a new one.
	export class Polygon2D {
		constructor(
			public readonly points: Point[],
			public readonly closed: boolean = true,
		) {}

		/// From the coordinates one after another, x then y.
		static fromFlat(values: number[], closed: boolean): Polygon2D {
			const points: Point[] = [];
			for (let at = 0; at + 1 < values.length; at += 2)
				points.push(new Point(values[at], values[at + 1]));
			return new Polygon2D(points, closed);
		}

		range(): Range2D {
			return Range2D.fromPoints(this.points);
		}

		transformed(mapping: Matrix2D): Polygon2D {
			return new Polygon2D(
				this.points.map((point: Point) => mapping.apply(point.x, point.y)),
				this.closed,
			);
		}

		reversed(): Polygon2D {
			return new Polygon2D(this.points.slice().reverse(), this.closed);
		}

		/// The polygon that far on the way to the other one, point by point: this one at 0, the
		/// other at 1. Both have the same number of points.
		interpolated(other: Polygon2D, share: number): Polygon2D {
			return new Polygon2D(
				this.points.map((point: Point, at: number) => {
					const theirs = other.points[at];
					return new Point(
						point.x + (theirs.x - point.x) * share,
						point.y + (theirs.y - point.y) * share,
					);
				}),
				this.closed,
			);
		}

		/// Each point moved that far along the way between the normals of its two edges, as the
		/// drawing layer grows a polygon; a negative width moves the points the other way.
		grownInNormalDirection(width: number): Polygon2D {
			if (!width) return this;
			const across = (x: number, y: number): Point => {
				const length = Math.hypot(x, y);
				return length ? new Point(-y / length, x / length) : new Point(0, 0);
			};
			const points = this.points;
			return new Polygon2D(
				points.map((current: Point, at: number) => {
					const before = points[(at + points.length - 1) % points.length];
					const next = points[(at + 1) % points.length];
					const back = across(before.x - current.x, before.y - current.y);
					const forth = across(next.x - current.x, next.y - current.y);
					const x = back.x - forth.x;
					const y = back.y - forth.y;
					const length = Math.hypot(x, y);
					return length ?
							new Point(
								current.x + (x / length) * width,
								current.y + (y / length) * width,
							)
						:	new Point(current.x, current.y);
				}),
				this.closed,
			);
		}

		/// Whether the point lies inside the area the polygon closes, by the even-odd rule.
		isInside(x: number, y: number): boolean {
			const points = this.points;
			let inside = false;
			for (
				let at = 0, before = points.length - 1;
				at < points.length;
				before = at++
			) {
				const one = points[at];
				const other = points[before];
				if (
					one.y > y !== other.y > y &&
					x < ((other.x - one.x) * (y - one.y)) / (other.y - one.y) + one.x
				)
					inside = !inside;
			}
			return inside;
		}

		/// Adds the polygon to a path, closed where it is closed.
		addTo(path: Path2D): void {
			this.points.forEach((point: Point, at: number) =>
				at ? path.lineTo(point.x, point.y) : path.moveTo(point.x, point.y),
			);
			if (this.closed && this.points.length > 2) path.closePath();
		}
	}

	/// Several polygons in 2D that make one area together, as the drawing layer's B2DPolyPolygon
	/// without curves: an outline and its holes, by the even-odd rule.
	export class PolyPolygon2D {
		constructor(public readonly polygons: Polygon2D[]) {}

		range(): Range2D {
			return Range2D.fromPoints(
				this.polygons.flatMap((polygon: Polygon2D) => polygon.points),
			);
		}

		transformed(mapping: Matrix2D): PolyPolygon2D {
			return new PolyPolygon2D(
				this.polygons.map((polygon: Polygon2D) => polygon.transformed(mapping)),
			);
		}

		/// Scaled by the same factor both ways about the middle of the range of all polygons.
		scaledOnTheCenter(scale: number): PolyPolygon2D {
			const range = this.range();
			return this.transformed(
				new Matrix2D(1, 0, 0, 1, -range.centerX, -range.centerY)
					.scale(scale, scale)
					.translate(range.centerX, range.centerY),
			);
		}

		interpolated(other: PolyPolygon2D, share: number): PolyPolygon2D {
			return new PolyPolygon2D(
				this.polygons.map((polygon: Polygon2D, at: number) =>
					polygon.interpolated(other.polygons[at], share),
				),
			);
		}

		grownInNormalDirection(width: number): PolyPolygon2D {
			return new PolyPolygon2D(
				this.polygons.map((polygon: Polygon2D) =>
					polygon.grownInNormalDirection(width),
				),
			);
		}

		/// Whether the point lies inside the area: inside an odd number of the polygons, as the
		/// drawing layer counts it.
		isInside(x: number, y: number): boolean {
			let inside = 0;
			for (const polygon of this.polygons) if (polygon.isInside(x, y)) ++inside;
			return inside % 2 === 1;
		}

		/// Puts the polygons in 3D, in the plane where z is 0, mapped by the given transform.
		inSpace(place: Matrix3D): PolyPolygon3D {
			return new PolyPolygon3D(
				this.polygons.map(
					(polygon: Polygon2D) =>
						new Polygon3D(
							polygon.points.map((point: Point) =>
								place.apply(new Point3D(point.x, point.y, 0)),
							),
							polygon.closed,
						),
				),
			);
		}

		addTo(path: Path2D): void {
			for (const polygon of this.polygons) polygon.addTo(path);
		}
	}
}
