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
// @ts-strict-ignore

/*
 * CPolyUtil contains utility functions for polygons.
 */

namespace CPolyUtil {

	export function rectanglesToPointSet(rectangles: Array<Array<cool.Point>>, unitConverter: (point: cool.Point) => cool.Point): CPointSet {
		/* An Implementation based on O'ROURKE, Joseph. "Uniqueness of orthogonal connect-the-dots."
		   Machine Intelligence and Pattern Recognition. Vol. 6. North-Holland, 1988. 97-104.
		   http://www.science.smith.edu/~jorourke/Papers/OrthoConnect.pdf
		*/
		// Helper function for sorted array insert.
		function sortedIndex(array: Array<cool.Point>, value: cool.Point, compare: (a: cool.Point, b: cool.Point) => number) : number {
			let low = 0;
			let high = array.length;
			while (low < high) {
				const mid = (low + high) >>> 1;
				if (compare(value, array[mid]) > 0) low = mid + 1;
				else high = mid;
			}
			return low;
		}

		// Glue rectangles if the space between them is less than eps
		const eps = 20;
		const pointsX = new Array<cool.Point>();
		const pointsY = new Array<cool.Point>();
		for (let i = 0; i < rectangles.length; i++) {
			for (let j = 0; j < rectangles[i].length; j++) {
				pointsX.splice(sortedIndex(pointsX, rectangles[i][j], (a, b) => a.x - b.x), 0, rectangles[i][j]);
				pointsY.splice(sortedIndex(pointsY, rectangles[i][j], (a, b) => a.y - b.y), 0, rectangles[i][j]);
			}
		}

		let lastPointX = 0;
		let lastPointY = 0;
		for (let i = 1; i < pointsX.length; ++i) {
			if (Math.abs(pointsX[lastPointX].x - pointsX[i].x) < eps)
				pointsX[i].x = pointsX[lastPointX].x;
			else
				lastPointX = i;
			if (Math.abs(pointsY[lastPointY].y - pointsY[i].y) < eps)
				pointsY[i].y = pointsY[lastPointY].y;
			else
				lastPointY = i;
		}

		// cool.Point comparison function for sorting a list of CPoints w.r.t x-coordinate.
		// When the points have same x-coordinate break tie based on y-coordinates.
		function xThenY(ap: cool.Point, bp: cool.Point): number {
			if (ap.x < bp.x || (ap.x === bp.x && ap.y < bp.y)) {
				return -1;
			}
			else if (ap.x === bp.x && ap.y === bp.y) {
				return 0;
			}
			else {
				return 1;
			}
		}

		// cool.Point comparison function for sorting a list of CPoints w.r.t y-coordinate.
		// When the points have same y-coordinate break tie based on x-coordinates.
		function yThenX(ap: cool.Point, bp: cool.Point): number {

			if (ap.y < bp.y || (ap.y === bp.y && ap.x < bp.x)) {
				return -1;
			}
			else if (ap.x === bp.x && ap.y === bp.y) {
				return 0;
			}
			else {
				return 1;
			}
		}

		// Collect points and horizontal and vertical edges.
		const points = new Set<cool.Point>();
		for (const point of pointsX)
			if (!points.delete(point))
				points.add(point);

		var sortX = Array.from(points.values()).sort(xThenY);
		var sortY = Array.from(points.values()).sort(yThenX);

		var edgesH = new Map<cool.Point, cool.Point>();
		var edgesV = new Map<cool.Point, cool.Point>();

		var len = points.size;
		let i = 0;
		while (i < len) {
			var currY = sortY[i].y;
			while (i < len && sortY[i].y === currY) {
				edgesH.set(sortY[i], sortY[i + 1]);
				edgesH.set(sortY[i + 1], sortY[i]);
				i += 2;
			}
		}

		i = 0;
		while (i < len) {
			var currX = sortX[i].x;
			while (i < len && sortX[i].x === currX) {
				edgesV.set(sortX[i], sortX[i + 1]);
				edgesV.set(sortX[i + 1], sortX[i]);
				i += 2;
			}
		}

		var polygons = new Array<CPointSet>();

		while (edgesH.size > 0) {
			var p: Array<[cool.Point, number]> = [[edgesH.keys().next().value, 0]];
			while (true) {
				var curr = p[p.length - 1][0];
				var e = p[p.length - 1][1];
				if (e === 0) {
					var nextVertex = edgesV.get(curr);
					edgesV.delete(curr);
					p.push([nextVertex, 1]);
				}
				else {
					var nextVertex = edgesH.get(curr);
					edgesH.delete(curr);
					p.push([nextVertex, 0]);
				}
				if (p[p.length - 1][0].equals(p[0][0]) && p[p.length - 1][1] === p[0][1]) {
					p.pop();
					break;
				}
			}
			var polygon = new Array<cool.Point>();
			for (i = 0; i < p.length; i++) {
				polygon.push(unitConverter(p[i][0]));
				edgesH.delete(p[i][0]);
				edgesV.delete(p[i][0]);
			}
			polygon.push(unitConverter(p[0][0]));
			polygons.push(CPointSet.fromPointArray(polygon));
		}
		return CPointSet.fromSetArray(polygons);
	}
}
