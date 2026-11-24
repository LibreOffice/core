/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// A drawing context for hit tests, as Node has none. It answers from what a Path2DRecorder
/// holds: rectangles, paths added through a transform, and polygons drawn with moveTo and
/// lineTo, which count by the even-odd rule. A point on no stroke is the answer for every
/// stroke. Text measures 10 for each character, as on CanvasRecorder.
class HitTestContext {
	public font: string = '';
	public lineWidth: number = 1;
	public lineJoin: string = 'miter';
	public lineCap: string = 'butt';

	isPointInPath(path: any, x: number, y: number): boolean {
		if (this.insideThePolygons(path, x, y)) return true;
		return path.calls.some((call: any) => {
			if (call.method === 'rect') {
				const [left, top, width, height] = call.args;
				return x >= left && x <= left + width && y >= top && y <= top + height;
			}
			if (call.method === 'addPath') {
				const [inner, m] = call.args;
				const back = new cool.Matrix2D(m.a, m.b, m.c, m.d, m.e, m.f).invert();
				if (!back) return false;
				const at = back.apply(x, y);
				return this.isPointInPath(inner, at.x, at.y);
			}
			return false;
		});
	}

	/// Whether the point lies inside the polygons of the path by the even-odd rule.
	private insideThePolygons(path: any, x: number, y: number): boolean {
		const polygons: number[][][] = [];
		for (const call of path.calls) {
			if (call.method === 'moveTo') polygons.push([call.args]);
			else if (call.method === 'lineTo' && polygons.length)
				polygons[polygons.length - 1].push(call.args);
		}
		let crossings = 0;
		for (const polygon of polygons)
			for (
				let at = 0, before = polygon.length - 1;
				at < polygon.length;
				before = at++
			) {
				const [x1, y1] = polygon[at];
				const [x0, y0] = polygon[before];
				if (y1 > y !== y0 > y && x < ((x0 - x1) * (y - y1)) / (y0 - y1) + x1)
					++crossings;
			}
		return crossings % 2 === 1;
	}

	isPointInStroke(): boolean {
		return false;
	}

	measureText(text: string): TextMetrics {
		return { width: text.length * 10 } as TextMetrics;
	}

	save(): void {}

	restore(): void {}
}
