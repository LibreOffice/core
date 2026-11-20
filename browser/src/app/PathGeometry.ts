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

/*
	The geometry of a path: functions of the polygons of a path, each point with its weights and
	how the curve runs through it, that answer a new path, a place on it or a fact about it. They
	keep no state and know nothing of who shows or holds the path, so every part that works on a
	path - point editing, drawing, motion paths - shares them.

	The changes of a path work on a copy of the path, as the engine makes them. Each answers the
	changed path, or nothing where the engine changes more than the path.
*/
class PathGeometry {
	/// Whether a path is closed, meaning it has polygons and every one of them is closed, as the
	/// engine reads a path.
	public static isClosed(path: cool.ObjectPathPolygon[] | undefined): boolean {
		return (
			!!path?.length &&
			path.every((polygon: cool.ObjectPathPolygon) => polygon.closed === true)
		);
	}

	/// The number of points the polygon has once it is closed. Points at the end that equal the
	/// first point are not counted, because a closed polygon does not repeat its first point.
	public static pointsOnceClosed(polygon: cool.ObjectPathPolygon): number {
		const points = polygon.points ?? [];
		let count = points.length;
		while (
			count > 1 &&
			points[count - 1].x === points[0].x &&
			points[count - 1].y === points[0].y
		)
			--count;
		return count;
	}

	/// Without that point; a polygon left with fewer than two points goes. Nothing where no
	/// polygon is left, since the path keeps one.
	public static withoutThePoint(
		path: cool.ObjectPathPolygon[],
		place: PathPlace,
	): cool.ObjectPathPolygon[] | null {
		const points = path[place.polygon]?.points;
		if (!points || place.point >= points.length) return null;

		points.splice(place.point, 1);
		if (points.length < 2) path.splice(place.polygon, 1);
		return path.length ? path : null;
	}

	/// The point after that one in its polygon, or nothing for the last point of an open one.
	public static nextOf(
		polygon: cool.ObjectPathPolygon,
		point: number,
	): number | null {
		const count = (polygon.points ?? []).length;
		if (point + 1 < count) return point + 1;
		return polygon.closed && count > 1 ? 0 : null;
	}

	/// The edge after that point made a curve, its new weights on the line at one third and two
	/// thirds of it, or made a line, without weights.
	public static withTheEdgeCurved(
		path: cool.ObjectPathPolygon[],
		place: PathPlace,
		curved: boolean,
	): cool.ObjectPathPolygon[] | null {
		const polygon = path[place.polygon];
		const next = polygon ? PathGeometry.nextOf(polygon, place.point) : null;
		if (!polygon || next === null) return null;

		const from = polygon.points[place.point];
		const to = polygon.points[next];
		if (curved) {
			if (from.aheadX === undefined || from.aheadY === undefined) {
				from.aheadX = from.x + (to.x - from.x) / 3;
				from.aheadY = from.y + (to.y - from.y) / 3;
			}
			if (to.behindX === undefined || to.behindY === undefined) {
				to.behindX = from.x + (2 * (to.x - from.x)) / 3;
				to.behindY = from.y + (2 * (to.y - from.y)) / 3;
			}
		} else {
			delete from.aheadX;
			delete from.aheadY;
			delete to.behindX;
			delete to.behindY;
		}
		return path;
	}

	/*
		How the curve runs through that point set as the drawing layer sets it: a corner puts
		each weight a third of the way to the neighbour on its side; smooth sets the two weights
		on one line through the point, each keeping its length; symmetric gives them the length
		of the two together, halved, as well. A point without both weights stays as it is for
		smooth and symmetric.
	*/
	public static withTheContinuity(
		path: cool.ObjectPathPolygon[],
		place: PathPlace,
		continuity: number,
	): cool.ObjectPathPolygon[] | null {
		const polygon = path[place.polygon];
		const points = polygon?.points ?? [];
		const point = points[place.point];
		if (!point) return null;

		const count = points.length;
		const at = new cool.Point(point.x, point.y);
		const prev = points[(place.point + count - 1) % count];
		const next = points[(place.point + 1) % count];
		const hasIn = point.behindX !== undefined && point.behindY !== undefined;
		const hasOut = point.aheadX !== undefined && point.aheadY !== undefined;
		const third = (to: cool.ObjectPathPoint) =>
			at.add(new cool.Point(to.x, to.y).subtract(at).divideBy(3));

		if (continuity === 0) {
			if (hasIn) {
				if (!polygon.closed && place.point === 0) {
					delete point.behindX;
					delete point.behindY;
				} else {
					const weight = third(prev);
					point.behindX = weight.x;
					point.behindY = weight.y;
				}
			}
			if (hasOut) {
				const weight = third(next);
				point.aheadX = weight.x;
				point.aheadY = weight.y;
			}
			delete point.continuity;
			return path;
		}

		// a point without both weights keeps them, but takes the way the curve runs through it
		if (!hasIn || !hasOut) {
			point.continuity = continuity as 1 | 2;
			return path;
		}

		const comingIn = new cool.Point(
			point.behindX as number,
			point.behindY as number,
		).subtract(at);
		const goingOut = new cool.Point(
			point.aheadX as number,
			point.aheadY as number,
		).subtract(at);
		const inLength = comingIn.length();
		const outLength = goingOut.length();
		if (!inLength || !outLength) return path;

		const inWay = comingIn.divideBy(inLength);
		const outWay = goingOut.divideBy(outLength);
		const cross = inWay.x * outWay.y - inWay.y * outWay.x;
		const facing =
			Math.abs(cross) < 1e-9 && inWay.x * outWay.x + inWay.y * outWay.y < 0;
		const common = (inLength + outLength) / 2;
		const set = (behind: cool.Point, ahead: cool.Point) => {
			point.behindX = behind.x;
			point.behindY = behind.y;
			point.aheadX = ahead.x;
			point.aheadY = ahead.y;
		};

		if (facing) {
			if (continuity === 2)
				set(
					at.add(inWay.multiplyBy(common)),
					at.subtract(inWay.multiplyBy(common)),
				);
			else if (Math.abs(inLength - outLength) < 1e-9) {
				// equal lengths would be symmetric; smooth takes a third of each edge
				const prevLength =
					new cool.Point(prev.x, prev.y).subtract(at).length() / 3;
				const nextLength =
					new cool.Point(next.x, next.y).subtract(at).length() / 3;
				set(
					at.add(inWay.multiplyBy(prevLength)),
					at.add(outWay.multiplyBy(nextLength)),
				);
			}
		} else {
			// the weights stand across the line halfway between their two ways
			const sum = inWay.add(outWay);
			const sumLength = sum.length();
			const across = sumLength
				? new cool.Point(-sum.y / sumLength, sum.x / sumLength)
				: new cool.Point(-inWay.y, inWay.x);
			const inLong = continuity === 2 ? common : inLength;
			const outLong = continuity === 2 ? common : outLength;
			if (cross > 0)
				set(
					at.subtract(across.multiplyBy(inLong)),
					at.add(across.multiplyBy(outLong)),
				);
			else
				set(
					at.add(across.multiplyBy(inLong)),
					at.subtract(across.multiplyBy(outLong)),
				);
		}
		point.continuity = continuity as 1 | 2;
		return path;
	}

	/*
		Split at a point. A closed polygon opens there, and the point becomes both of its ends.
		An open polygon becomes two that meet at the point, the part before it and the part
		after it. Only a path of one polygon is split, and an open one not at its ends.
	*/
	public static splitAt(
		path: cool.ObjectPathPolygon[],
		place: PathPlace,
	): cool.ObjectPathPolygon[] | null {
		const polygon = path[place.polygon];
		const points = polygon?.points ?? [];
		const at = place.point;
		if (path.length !== 1 || points.length < 3) return null;

		const without = (
			point: cool.ObjectPathPoint,
			side: 'behind' | 'ahead',
		): cool.ObjectPathPoint => {
			const kept = { ...point };
			if (side === 'behind') {
				delete kept.behindX;
				delete kept.behindY;
			} else {
				delete kept.aheadX;
				delete kept.aheadY;
			}
			delete kept.continuity;
			return kept;
		};

		if (polygon.closed) {
			const around = points.slice(at).concat(points.slice(0, at));
			return [
				{
					closed: false,
					points: [without(around[0], 'behind')]
						.concat(around.slice(1))
						.concat([without(points[at], 'ahead')]),
				},
			];
		}

		if (at === 0 || at + 1 >= points.length) return null;
		return [
			{
				closed: false,
				points: points.slice(0, at).concat([without(points[at], 'ahead')]),
			},
			{
				closed: false,
				points: [without(points[at], 'behind')].concat(points.slice(at + 1)),
			},
		];
	}

	/*
		What simplifying the path removes. A curved edge runs straight when its weights lie on
		the line between its ends, and its weights go. With pointsOnALine the points that lie on a
		straight line between their neighbours go too, from the last to the first in each
		polygon, so the place of each one stays as it is until it goes.
	*/
	public static simplification(
		path: cool.ObjectPathPolygon[],
		pointsOnALine: boolean,
	): PathSimplification {
		// The path is in whole twips, so a point placed on a line can be as far as the diagonal of
		// a twip away from it.
		const reach = Math.SQRT2;
		const simplification: PathSimplification = { weights: [], points: [] };

		path.forEach((given: cool.ObjectPathPolygon, polygon: number) => {
			// The points are looked at without the weights that go.
			const points = (given.points ?? []).map(
				(point: cool.ObjectPathPoint) => ({ ...point }),
			);
			const flattened = { ...given, points: points };
			points.forEach((point: cool.ObjectPathPoint, at: number) => {
				const next = PathGeometry.nextOf(flattened, at);
				if (next === null) return;
				const edge = PathGeometry.edgeOf(point, points[next]);
				if (
					!edge.curved ||
					!PathGeometry.onTheLine(edge.start, edge.end, edge.out, reach) ||
					!PathGeometry.onTheLine(edge.start, edge.end, edge.into, reach)
				)
					return;

				if (point.aheadX !== undefined)
					simplification.weights.push({ polygon, point: at, part: 2 });
				if (points[next].behindX !== undefined)
					simplification.weights.push({ polygon, point: next, part: 1 });
				delete point.aheadX;
				delete point.aheadY;
				delete points[next].behindX;
				delete points[next].behindY;
			});

			if (!pointsOnALine) return;
			const kept = new Set(PathGeometry.keptOffALine(flattened, reach));
			for (let at = points.length - 1; at >= 0; --at)
				if (!kept.has(at)) simplification.points.push({ polygon, point: at });
		});
		return simplification;
	}

	/// The path without the weights and the points that the simplification removes.
	public static withTheSimplification(
		path: cool.ObjectPathPolygon[],
		simplification: PathSimplification,
	): cool.ObjectPathPolygon[] | null {
		for (const weight of simplification.weights) {
			const point = path[weight.polygon]?.points?.[weight.point];
			if (!point) return null;
			if (weight.part === 1) {
				delete point.behindX;
				delete point.behindY;
			} else {
				delete point.aheadX;
				delete point.aheadY;
			}
		}
		for (const place of simplification.points)
			if (!PathGeometry.withoutThePoint(path, place)) return null;
		return path;
	}

	/// Whether a point lies within reach of the straight line between two others, and between
	/// them.
	public static onTheLine(
		from: cool.Point,
		to: cool.Point,
		at: cool.Point,
		reach: number,
	): boolean {
		const along = to.subtract(from);
		const length2 = along.x * along.x + along.y * along.y;
		if (!length2) return at.subtract(from).length() <= reach;
		const cut =
			((at.x - from.x) * along.x + (at.y - from.y) * along.y) / length2;
		if (cut < 0 || cut > 1) return false;
		return from.add(along.multiplyBy(cut)).subtract(at).length() <= reach;
	}

	/*
		The places of the points of a polygon that do not lie on a straight line between their
		neighbours. An open polygon keeps its ends, and a closed one at least three points.

		It walks the polygon once. A point goes when it, and every point gone since the last one
		kept, lie on the line from that one to the next point. So each line left stays close to
		all the points it takes the place of. A closed polygon keeps its first point during the
		walk and looks at it at the end, against the points kept on each side of it.
	*/
	public static keptOffALine(
		polygon: cool.ObjectPathPolygon,
		reach: number,
	): number[] {
		const points = polygon.points ?? [];
		const count = points.length;
		const closed = polygon.closed === true;
		const least = closed ? 3 : 2;
		if (count <= least)
			return points.map((_point: cool.ObjectPathPoint, at: number) => at);

		const straight = (point: cool.ObjectPathPoint) =>
			point.aheadX === undefined && point.behindX === undefined;
		const placeOf = (point: cool.ObjectPathPoint) =>
			new cool.Point(point.x, point.y);
		// whether every point between two lies on the line from the one to the other
		const runsStraight = (from: number, to: number): boolean => {
			if (points[from].aheadX !== undefined || points[to].behindX !== undefined)
				return false;
			for (let at = (from + 1) % count; at !== to; at = (at + 1) % count)
				if (
					!straight(points[at]) ||
					!PathGeometry.onTheLine(
						placeOf(points[from]),
						placeOf(points[to]),
						placeOf(points[at]),
						reach,
					)
				)
					return false;
			return true;
		};

		const kept = [0];
		let left = count;
		const end = closed ? count : count - 1;
		for (let at = 1; at < end; ++at) {
			if (left > least && runsStraight(kept[kept.length - 1], (at + 1) % count))
				--left;
			else kept.push(at);
		}
		if (!closed) kept.push(count - 1);
		else if (
			kept.length > least &&
			runsStraight(kept[kept.length - 1], kept[1])
		)
			kept.shift();

		return kept;
	}

	/// Every polygon closed, without points at its end that repeat its first, or every one open.
	public static closedOrOpened(
		path: cool.ObjectPathPolygon[],
		closed: boolean,
	): cool.ObjectPathPolygon[] | null {
		for (const polygon of path) {
			const points = polygon.points ?? [];
			if (closed)
				while (
					points.length > 1 &&
					points[points.length - 1].x === points[0].x &&
					points[points.length - 1].y === points[0].y
				)
					points.pop();
			polygon.closed = closed;
		}
		return path;
	}

	/*
		A point added on the edge nearest to that place, where the place nearest to it lies
		inside an edge rather than on one of its ends, and the addition that names it. A curve is
		split there into two curves, so the path keeps its shape and the new point runs smoothly
		through. With wholeTwips, the new point and the weights are rounded to whole twips, so the
		path shown is close to the one the model will hold, and a point that lands on an end of
		the edge once rounded adds nothing.
	*/
	public static withAPointAt(
		path: cool.ObjectPathPolygon[],
		at: cool.Point,
		wholeTwips: boolean,
	): { path: cool.ObjectPathPolygon[]; addition: PathPointAddition } | null {
		let best: {
			polygon: number;
			point: number;
			cut: number;
			distance: number;
		} | null = null;

		path.forEach((polygon: cool.ObjectPathPolygon, which: number) => {
			(polygon.points ?? []).forEach((_point, point: number) => {
				const next = PathGeometry.nextOf(polygon, point);
				if (next === null) return;
				const edge = PathGeometry.edgeOf(
					polygon.points[point],
					polygon.points[next],
				);
				const nearest = PathGeometry.nearestOnEdge(edge, at);
				if (!best || nearest.distance < best.distance)
					best = { polygon: which, point: point, ...nearest };
			});
		});

		const found = best as {
			polygon: number;
			point: number;
			cut: number;
			distance: number;
		} | null;
		if (!found || found.cut <= 1e-6 || found.cut >= 1 - 1e-6) return null;

		const points = path[found.polygon].points;
		const next = PathGeometry.nextOf(
			path[found.polygon],
			found.point,
		) as number;
		const from = points[found.point];
		const to = points[next];
		const edge = PathGeometry.edgeOf(from, to);
		const t = found.cut;
		const between = (a: cool.Point, b: cool.Point) =>
			a.add(b.subtract(a).multiplyBy(t));
		const whole = (a: cool.Point) =>
			wholeTwips ? new cool.Point(Math.round(a.x), Math.round(a.y)) : a;

		// The weights of the two halves of the curve: a and ab bend the half before the new
		// point, bc and c the half after it.
		const outHalf = between(edge.start, edge.out);
		const across = between(edge.out, edge.into);
		const intoHalf = between(edge.into, edge.end);
		const beforeMiddle = between(outHalf, across);
		const afterMiddle = between(across, intoHalf);
		const a = whole(outHalf);
		const ab = whole(beforeMiddle);
		const bc = whole(afterMiddle);
		const c = whole(intoHalf);
		const middle = whole(
			edge.curved
				? between(beforeMiddle, afterMiddle)
				: between(edge.start, edge.end),
		);
		if (
			(middle.x === from.x && middle.y === from.y) ||
			(middle.x === to.x && middle.y === to.y)
		)
			return null;

		// The new point comes right after the point the edge starts from. The point the edge
		// ends at moves one place on, unless the edge closes the polygon and ends at its first
		// point.
		const polygon = found.polygon;
		const place = found.point + 1;
		const toPlace = next === 0 ? 0 : next + 1;
		const added: cool.ObjectPathPoint = { x: middle.x, y: middle.y };
		const weights: PathPointMove[] = [];

		if (edge.curved) {
			// The weight of a neighbour stays on the line it was on and gets shorter, so a curve
			// that ran smoothly or symmetrically through the neighbour runs smoothly through it.
			if (from.aheadX !== undefined) {
				from.aheadX = a.x;
				from.aheadY = a.y;
				if (from.continuity) from.continuity = 1;
				weights.push({
					polygon: polygon,
					point: found.point,
					part: 2,
					x: a.x,
					y: a.y,
					continuity: from.continuity ?? 0,
				});
			}
			if (to.behindX !== undefined) {
				to.behindX = c.x;
				to.behindY = c.y;
				if (to.continuity) to.continuity = 1;
				weights.push({
					polygon: polygon,
					point: toPlace,
					part: 1,
					x: c.x,
					y: c.y,
					continuity: to.continuity ?? 0,
				});
			}

			added.behindX = ab.x;
			added.behindY = ab.y;
			added.aheadX = bc.x;
			added.aheadY = bc.y;
			added.continuity = 1;
			weights.push(
				{
					polygon: polygon,
					point: place,
					part: 1,
					x: ab.x,
					y: ab.y,
					continuity: 1,
				},
				{
					polygon: polygon,
					point: place,
					part: 2,
					x: bc.x,
					y: bc.y,
					continuity: 1,
				},
			);
		}

		points.splice(place, 0, added);
		return {
			path: path,
			addition: {
				polygon: polygon,
				point: place,
				x: middle.x,
				y: middle.y,
				weights: weights,
			},
		};
	}

	/// An edge from one point to the next: its ends, its weights where it is curved, else its
	/// ends again.
	public static edgeOf(
		from: cool.ObjectPathPoint,
		to: cool.ObjectPathPoint,
	): {
		start: cool.Point;
		out: cool.Point;
		into: cool.Point;
		end: cool.Point;
		curved: boolean;
	} {
		const start = new cool.Point(from.x, from.y);
		const end = new cool.Point(to.x, to.y);
		const out =
			from.aheadX !== undefined && from.aheadY !== undefined
				? new cool.Point(from.aheadX, from.aheadY)
				: start;
		const into =
			to.behindX !== undefined && to.behindY !== undefined
				? new cool.Point(to.behindX, to.behindY)
				: end;
		return { start, out, into, end, curved: out !== start || into !== end };
	}

	/// The place on an edge nearest to a point, as the share of the edge before it, and how far
	/// it is. A curve is searched in steps, then more finely around the nearest step.
	public static nearestOnEdge(
		edge: {
			start: cool.Point;
			out: cool.Point;
			into: cool.Point;
			end: cool.Point;
			curved: boolean;
		},
		at: cool.Point,
	): { cut: number; distance: number } {
		if (!edge.curved) {
			const along = edge.end.subtract(edge.start);
			const length2 = along.x * along.x + along.y * along.y;
			const cut = length2
				? Math.max(
						0,
						Math.min(
							1,
							((at.x - edge.start.x) * along.x +
								(at.y - edge.start.y) * along.y) /
								length2,
						),
					)
				: 0;
			const on = edge.start.add(along.multiplyBy(cut));
			return { cut, distance: on.subtract(at).length() };
		}

		const pointAt = (t: number) => {
			const u = 1 - t;
			return new cool.Point(
				u * u * u * edge.start.x +
					3 * u * u * t * edge.out.x +
					3 * u * t * t * edge.into.x +
					t * t * t * edge.end.x,
				u * u * u * edge.start.y +
					3 * u * u * t * edge.out.y +
					3 * u * t * t * edge.into.y +
					t * t * t * edge.end.y,
			);
		};
		const distanceAt = (t: number) => pointAt(t).subtract(at).length();

		const steps = 32;
		let cut = 0;
		for (let step = 1; step <= steps; ++step)
			if (distanceAt(step / steps) < distanceAt(cut)) cut = step / steps;

		let low = Math.max(0, cut - 1 / steps);
		let high = Math.min(1, cut + 1 / steps);
		for (let round = 0; round < 30; ++round) {
			const first = low + (high - low) / 3;
			const second = high - (high - low) / 3;
			if (distanceAt(first) < distanceAt(second)) high = second;
			else low = first;
		}
		cut = (low + high) / 2;
		return { cut, distance: distanceAt(cut) };
	}

	/*
		The point as it stands once one of its weights has been dragged to a place. A curve that
		runs smoothly through the point keeps its two weights on one line through it, and one that
		runs symmetrically through keeps them the same length as well, so the weight that was not
		dragged swings around the point to answer the one that was. That is what the engine does
		with the change it is sent, and the ghost shows it beforehand.
	*/
	public static withTheWeightMoved(
		stands: cool.ObjectPathPoint,
		comingIn: boolean,
		at: cool.Point,
	): cool.ObjectPathPoint {
		const moved = comingIn
			? { ...stands, behindX: at.x, behindY: at.y }
			: { ...stands, aheadX: at.x, aheadY: at.y };

		const otherX = comingIn ? stands.aheadX : stands.behindX;
		const otherY = comingIn ? stands.aheadY : stands.behindY;
		if (
			stands.continuity === undefined ||
			otherX === undefined ||
			otherY === undefined
		)
			return moved;

		const point = new cool.Point(stands.x, stands.y);
		const away = at.subtract(point);
		const awayLength = away.length();
		if (!awayLength) return moved;

		// The one that gives way keeps its own length where the curve is only smooth, and takes
		// the length of the dragged one where it is symmetric as well.
		const length =
			stands.continuity === 2
				? awayLength
				: new cool.Point(otherX, otherY).subtract(point).length();

		const back = point.subtract(away.divideBy(awayLength).multiplyBy(length));

		return comingIn
			? { ...moved, aheadX: back.x, aheadY: back.y }
			: { ...moved, behindX: back.x, behindY: back.y };
	}

	/// That point moved by that offset, its weights too where it has them, and everything
	/// else it says, how the curve runs through it among it, as it was.
	public static shifted(
		point: cool.ObjectPathPoint,
		by: cool.Point,
	): cool.ObjectPathPoint {
		const moved: cool.ObjectPathPoint = {
			...point,
			x: point.x + by.x,
			y: point.y + by.y,
		};
		if (point.behindX !== undefined) moved.behindX = point.behindX + by.x;
		if (point.behindY !== undefined) moved.behindY = point.behindY + by.y;
		if (point.aheadX !== undefined) moved.aheadX = point.aheadX + by.x;
		if (point.aheadY !== undefined) moved.aheadY = point.aheadY + by.y;
		return moved;
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

	/// The weights of the point that are not where they were, at whole twips.
	public static roundWhatMoved(
		was: cool.ObjectPathPoint | undefined,
		point: cool.ObjectPathPoint | undefined,
	): void {
		if (!was || !point) return;
		if (point.behindX !== was.behindX || point.behindY !== was.behindY) {
			if (point.behindX !== undefined)
				point.behindX = Math.round(point.behindX);
			if (point.behindY !== undefined)
				point.behindY = Math.round(point.behindY);
		}
		if (point.aheadX !== was.aheadX || point.aheadY !== was.aheadY) {
			if (point.aheadX !== undefined) point.aheadX = Math.round(point.aheadX);
			if (point.aheadY !== undefined) point.aheadY = Math.round(point.aheadY);
		}
	}

	/*
		The weights of the touched points of one polygon that differ between before and after,
		which hold the same points: a weight that is in after at another place is a move, with
		that continuity, and one that is only in before is a removal.
	*/
	public static weightsChanged(
		polygon: number,
		touched: number[],
		before: cool.ObjectPathPoint[],
		after: cool.ObjectPathPoint[],
		step: PathWeightsChange['step'],
		continuity: number,
	): PathWeightsChange {
		const change: PathWeightsChange = { moves: [], removals: [], step: step };
		for (const at of touched) {
			const was = before[at];
			const point = after[at];
			if (!was || !point) continue;

			const weights = [
				{
					part: 1,
					x: point.behindX,
					y: point.behindY,
					wasX: was.behindX,
					wasY: was.behindY,
				},
				{
					part: 2,
					x: point.aheadX,
					y: point.aheadY,
					wasX: was.aheadX,
					wasY: was.aheadY,
				},
			];
			for (const weight of weights) {
				if (weight.x === undefined || weight.y === undefined) {
					if (weight.wasX !== undefined)
						change.removals.push({
							polygon: polygon,
							point: at,
							part: weight.part,
						});
				} else if (weight.x !== weight.wasX || weight.y !== weight.wasY) {
					change.moves.push({
						polygon: polygon,
						point: at,
						part: weight.part,
						x: weight.x,
						y: weight.y,
						continuity: continuity,
					});
				}
			}
		}
		return change;
	}

	/// The place as far from that point as the other one, in the direction from it of the
	/// nearest multiple of 45 degrees, in twips.
	public static inStepsOfFortyFiveDegrees(
		from: cool.Point,
		to: cool.Point,
	): cool.Point {
		const x = to.x - from.x;
		const y = to.y - from.y;
		const step = Math.PI / 4;
		const angle = Math.round(Math.atan2(y, x) / step) * step;
		const length = Math.hypot(x, y);
		return new cool.Point(
			from.x + Math.cos(angle) * length,
			from.y + Math.sin(angle) * length,
		);
	}
}
