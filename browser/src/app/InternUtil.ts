/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

type InternPointLike = cool.SimplePoint | { x: number; y: number };

/// Static methods needed for converting point position to internal units(twips for now) and back.
class InternPointUtil {
	public static SCALE = 1.2;
	public static BASE_ZOOM = 10;
	public static scale(zoom: number): number {
		return zoom ? Math.pow(InternPointUtil.SCALE, zoom) : 1;
	}

	public static flexConstruct(
		x: any,
		y?: number,
	): cool.SimplePoint | undefined {
		if (x === undefined || x === null) {
			return undefined;
		}

		if (x instanceof cool.SimplePoint) {
			return x;
		}

		const pt = new cool.SimplePoint(0, 0);
		let done: boolean = false;

		if (
			Array.isArray(x) &&
			x.length === 2 &&
			typeof x[0] === 'number' &&
			typeof x[1] === 'number'
		) {
			pt.x = x[0];
			pt.y = x[1];
			done = true;
		}

		if (typeof x === 'object' && 'x' in x && 'y' in x) {
			pt.x = x.x;
			pt.y = x.y;
			done = true;
		}

		if (typeof x === 'number' && typeof y === 'number') {
			pt.x = x;
			pt.y = y;
			done = true;
		}

		if (!done) {
			return undefined;
		}

		return pt;
	}

	// Needed in Map.prototype.unproject()
	// constructs Twips point from css pixel point at a given zoom.
	public static pointToIntern(
		point: cool.Point,
		zoom: number,
	): cool.SimplePoint {
		const multFactor =
			(15.0 * app.dpiScale) /
			InternPointUtil.scale(zoom - InternPointUtil.BASE_ZOOM);
		return new cool.SimplePoint(point.x * multFactor, point.y * multFactor);
	}

	// Needed in Map.prototype.project()
	// constructs css pixel point from Intern at a given zoom.
	public static internToPoint(
		intern: cool.SimplePoint,
		zoom: number,
	): cool.Point {
		const multFactor =
			InternPointUtil.scale(zoom - InternPointUtil.BASE_ZOOM) /
			(15.0 * app.dpiScale);
		return new cool.Point(intern.x * multFactor, intern.y * multFactor);
	}

	// used in Map.prototype.rescale(), only makes sense for pixel points.
	// (cool.Point, number, number) -> cool.Point
	public static rescale(
		point: cool.Point,
		oldZoom: number,
		newZoom: number,
	): cool.Point {
		const scale = InternPointUtil.scale(newZoom - oldZoom);
		return cool.Point.toPoint(point.x * scale, point.y * scale);
	}

	// used in Map.prototype.distance()
	public static distance(
		intern1: cool.SimplePoint,
		intern2: cool.SimplePoint,
	): number {
		const dx = intern2.x - intern1.x;
		const dy = intern2.y - intern1.y;
		return Math.sqrt(dx * dx + dy * dy);
	}
}

/// Static methods needed for converting rectangular region to internal units(twips for now) and back.
class InternBoundsUtil {
	// Assumes everything is already in internal units(twips for now).
	public static flexConstruct(
		a:
			| undefined
			| cool.SimpleRectangle
			| cool.SimplePoint
			| cool.SimplePoint[]
			| number[]
			| number[][],
		b?: cool.SimplePoint | number[],
	): cool.SimpleRectangle | undefined {
		if (!a) {
			return undefined;
		}

		if (a instanceof cool.SimpleRectangle) {
			return a;
		}

		const rect = new cool.SimpleRectangle(0, 0, 0, 0);
		let done1: boolean = false;
		let done2: boolean = false;

		if (a instanceof cool.SimplePoint) {
			rect.x1 = a.x;
			rect.y1 = a.y;
			done1 = true;
		} else if (
			Array.isArray(a) &&
			a.length >= 2 &&
			typeof a[0] === 'number' &&
			typeof a[1] === 'number'
		) {
			rect.x1 = a[0];
			rect.y1 = a[1];
			done1 = true;
		} else if (
			Array.isArray(a) &&
			a.length >= 2 &&
			a[0] instanceof cool.SimplePoint &&
			a[1] instanceof cool.SimplePoint
		) {
			rect.x1 = a[0].x;
			rect.y1 = a[0].y;
			rect.x2 = a[1].x;
			rect.y2 = a[1].y;
			done1 = true;
			done2 = true;
		} else if (
			Array.isArray(a) &&
			a.length >= 2 &&
			Array.isArray(a[0]) &&
			Array.isArray(a[1])
		) {
			rect.x1 = a[0][0];
			rect.y1 = a[0][1];
			rect.x2 = a[1][0];
			rect.y2 = a[1][1];
			done1 = true;
			done2 = true;
		}

		if (!done1) {
			return undefined;
		}

		if (done2) {
			return rect;
		}

		if (b instanceof cool.SimplePoint) {
			rect.x2 = b.x;
			rect.y2 = b.y;
			done2 = true;
		} else if (
			Array.isArray(b) &&
			typeof b[0] === 'number' &&
			typeof b[1] === 'number'
		) {
			rect.x2 = b[0];
			rect.y2 = b[1];
			done2 = true;
		} else if (typeof b === 'undefined') {
			rect.x2 = rect.x1;
			rect.y2 = rect.y1;
			done2 = true;
		}

		if (!done2) {
			return undefined;
		}

		return rect;
	}

	public static getTopLeft(bounds: cool.SimpleRectangle): cool.SimplePoint {
		return new cool.SimplePoint(bounds.x1, bounds.y1);
	}

	public static getBottomRight(bounds: cool.SimpleRectangle): cool.SimplePoint {
		return new cool.SimplePoint(bounds.x2, bounds.y2);
	}
}
