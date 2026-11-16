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

/// How a path is drawn: whether it may hold several polygons, whether each of them closes when
/// it ends, and whether its edges keep to steps of 45 degrees by themselves.
interface PathDrawingKind {
	multi: boolean;
	closes: boolean;
	inSteps: boolean;
}

/// What a step of drawing did: the path goes on, a part of it ended, the path is finished, or
/// nothing is left of it.
type PathDrawingOutcome = 'goesOn' | 'partEnded' | 'finished' | 'nothingLeft';

/*
	A path being drawn point by point, in twips: the parts drawn so far, the part being drawn, and
	the point that follows the mouse until it is set. It knows nothing of the mouse or of where the
	path goes once finished; whoever draws with it tells it where the mouse is and what it did.

	A point is set when the button comes up, and follows the mouse while it is pressed, so it can
	be corrected. A part ends when the one who draws says so, or when its own start point is hit,
	which sets no point there. The parts of one path are all closed or all open, as an object
	holds them, and the kind says which. A part that ends with too few points is dropped. A path of one polygon is finished as
	soon as its part ends; one of several goes on with the next part.
*/
class PathDrawing {
	private kind: PathDrawingKind;
	/// The parts drawn so far, each ended.
	private parts: cool.ObjectPathPolygon[] = [];
	/// The points of the part being drawn; empty before its first point is set.
	private points: cool.ObjectPathPoint[] = [];
	/// Where the mouse places the next point, once placed by the rules; null off the page.
	private next: cool.Point | null = null;
	/// How near the start point of the part the mouse is drawn onto it, in twips.
	private reach: number;

	constructor(kind: PathDrawingKind, reach: number) {
		this.kind = kind;
		this.reach = reach;
	}

	/// Whether no point of the part being drawn is set yet, so the mouse shows where it would
	/// start.
	public beforeThePart(): boolean {
		return this.points.length === 0;
	}

	/// The start point of the part where the next point lies on it, which would end the part;
	/// null elsewhere.
	public startOnTheMouse(): cool.Point | null {
		return this.onTheStart() ? this.next : null;
	}

	/// Whether the next point lies on the start point of the part.
	private onTheStart(): boolean {
		const start = this.points[0];
		return (
			!!start &&
			!!this.next &&
			this.points.length >= 2 &&
			this.next.x === start.x &&
			this.next.y === start.y
		);
	}

	/*
		Places the next point where the mouse is, the place given already snapped: on the start
		point of the part where it is within reach, else in steps of 45 degrees from the last point
		where the kind keeps to them or turnedOver asks for them, else where it is.
	*/
	public moveTo(at: cool.Point, turnedOver: boolean): cool.Point {
		const start = this.points[0];
		const last = this.points[this.points.length - 1];
		let placed = at;

		if (
			start &&
			this.points.length >= 2 &&
			new cool.Point(start.x, start.y).subtract(at).length() <= this.reach
		)
			placed = new cool.Point(start.x, start.y);
		else if (last && this.kind.inSteps !== turnedOver) {
			const x = at.x - last.x;
			const y = at.y - last.y;
			const step = Math.PI / 4;
			const angle = Math.round(Math.atan2(y, x) / step) * step;
			const length = Math.hypot(x, y);
			placed = new cool.Point(
				last.x + Math.cos(angle) * length,
				last.y + Math.sin(angle) * length,
			);
		}

		this.next = placed;
		return placed;
	}

	/// The mouse left the page: the point that follows it is not shown.
	public mouseLeft(): void {
		this.next = null;
	}

	/*
		Sets the next point where it is placed and, with endsThePart, ends the part after it. On
		the start point of the part it only ends the part, with no second point there. A part ends
		closed where the kind closes its parts.
	*/
	public setThePoint(endsThePart: boolean): PathDrawingOutcome {
		if (!this.next) return 'goesOn';

		if (this.onTheStart()) return this.endThePart(this.kind.closes);

		const last = this.points[this.points.length - 1];
		if (!last || last.x !== this.next.x || last.y !== this.next.y)
			this.points.push({ x: this.next.x, y: this.next.y });

		return endsThePart ? this.endThePart(this.kind.closes) : 'goesOn';
	}

	/// Finishes the path: the part being drawn ends as the kind ends its parts.
	public finish(): PathDrawingOutcome {
		if (this.points.length) this.endThePart(this.kind.closes);
		return this.path().length ? 'finished' : 'nothingLeft';
	}

	/*
		Ends the part being drawn, closed or open. A part with too few points for that - one
		point, or two where it closes - is dropped. A path of one polygon is then finished; one of
		several goes on with a part not begun.
	*/
	private endThePart(closed: boolean): PathDrawingOutcome {
		const enough = this.points.length >= (closed ? 3 : 2);
		if (enough) this.parts.push({ closed: closed, points: this.points });
		this.points = [];

		if (!this.kind.multi) return enough ? 'finished' : 'nothingLeft';
		return enough ? 'partEnded' : 'goesOn';
	}

	/// Takes back the last point, of the part before where the part being drawn has none, which
	/// is then drawn on. Nothing is left once the last point of the first part goes.
	public back(): PathDrawingOutcome {
		if (!this.points.length) {
			const before = this.parts.pop();
			if (!before) return 'nothingLeft';
			this.points = before.points;
		}

		this.points.pop();
		return this.points.length || this.parts.length ? 'goesOn' : 'nothingLeft';
	}

	/*
		Takes away the part being drawn, or where none is being drawn the part before. A path of one
		polygon is given up with it; of several, nothing is left once no part is.
	*/
	public dropThePart(): PathDrawingOutcome {
		if (!this.kind.multi) {
			this.parts = [];
			this.points = [];
			return 'nothingLeft';
		}

		if (this.points.length) this.points = [];
		else this.parts.pop();
		return this.parts.length ? 'goesOn' : 'nothingLeft';
	}

	/// The path as finished so far: the parts ended. Nothing where none is.
	public path(): cool.ObjectPathPolygon[] {
		return this.parts;
	}

	/// The path as it is shown while drawing: the parts ended, the part being drawn, and the piece
	/// from its last point to where the next one would be set.
	public shown(): cool.ObjectPathPolygon[] {
		const drawing = this.next ? this.points.concat([this.next]) : this.points;
		return drawing.length >= 2
			? this.parts.concat([{ closed: false, points: drawing }])
			: this.parts;
	}

	/// The rectangle around every point of the path, weights included, in twips.
	public static rangeOf(path: cool.ObjectPathPolygon[]): cool.Range2D {
		const points: cool.Point[] = [];
		for (const polygon of path)
			for (const point of polygon.points ?? []) {
				points.push(new cool.Point(point.x, point.y));
				if (point.behindX !== undefined && point.behindY !== undefined)
					points.push(new cool.Point(point.behindX, point.behindY));
				if (point.aheadX !== undefined && point.aheadY !== undefined)
					points.push(new cool.Point(point.aheadX, point.aheadY));
			}
		return cool.Range2D.fromPoints(points);
	}

	/*
		The path in whole twips in the form the path of an object comes from the engine, point by
		point with its weights, which is how the engine takes it back as well.
	*/
	public static asJson(path: cool.ObjectPathPolygon[]): string {
		const round = (value: number | undefined) =>
			value === undefined ? undefined : Math.round(value);
		return JSON.stringify({
			path: path.map((polygon: cool.ObjectPathPolygon) => ({
				closed: polygon.closed ? true : undefined,
				points: (polygon.points ?? []).map((point: cool.ObjectPathPoint) => ({
					x: Math.round(point.x),
					y: Math.round(point.y),
					behindX: round(point.behindX),
					behindY: round(point.behindY),
					aheadX: round(point.aheadX),
					aheadY: round(point.aheadY),
				})),
			})),
		});
	}

	/*
		The path as an SVG path in whole twips, one M for every polygon, a C for a curved edge and
		an L for a straight one, and a Z for a closed polygon, which is its straight closing edge
		where that is straight. Nothing where the path is empty.
	*/
	public static asSvg(path: cool.ObjectPathPolygon[]): string {
		const at = (x: number, y: number) =>
			String(Math.round(x)) + ' ' + String(Math.round(y));

		return path
			.map((polygon: cool.ObjectPathPolygon) => {
				const points = polygon.points ?? [];
				if (!points.length) return '';
				const edges = polygon.closed ? points.length : points.length - 1;
				let written = 'M ' + at(points[0].x, points[0].y);
				for (let edge = 0; edge < edges; ++edge) {
					const from = points[edge];
					const to = points[(edge + 1) % points.length];
					if (from.aheadX !== undefined || to.behindX !== undefined)
						written +=
							' C ' +
							at(from.aheadX ?? from.x, from.aheadY ?? from.y) +
							' ' +
							at(to.behindX ?? to.x, to.behindY ?? to.y) +
							' ' +
							at(to.x, to.y);
					else if (edge < points.length - 1) written += ' L ' + at(to.x, to.y);
				}
				return polygon.closed ? written + ' Z' : written;
			})
			.filter((written: string) => written.length)
			.join(' ');
	}
}
