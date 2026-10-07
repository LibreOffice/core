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

/*
	The run of a connector between its two ends, worked out here the way the engine works it out,
	from the same numbers: the kind of run, the distances it keeps from the objects it is tied to,
	the line deltas a user dragged into it, how each end holds on to its object, and the objects'
	rectangles and glue points. It is the engine's routing (SdrEdgeObj::ImpCalcEdgeTrack and its
	helpers) carried over as it stands, case by case, so that a connector can be drawn where it
	will be while the object it hangs on is being dragged, before the engine's answer comes.

	Everything here is in twips. The engine works in its model's finer unit and in whole numbers,
	so a run worked out here can differ from the engine's by a fraction of a twip here and there,
	never by a corner.
*/

namespace cool {
	/// What one end of a connector is tied to: the object's two rectangles and its glue points,
	/// with the mapping that puts a glue point onto the page. Absent for a free end.
	export interface RouteNode {
		snap: Range2D;
		bound: Range2D;
		transform: Matrix2D;
		gluePoints: ObjectGluePoint[];
		/// The four glue points at the middles of the sides, where they lie on the object.
		sideGluePoints: ObjectPlace[];
	}
}

/// Which ways a connector may leave a point, as the engine holds them: a set of the four sides.
/// Nothing set means the way is worked out from where the point lies.
const enum Escape {
	Smart = 0,
	Left = 1,
	Right = 2,
	Top = 4,
	Bottom = 8,
	Horizontal = 3,
	Vertical = 12,
	All = 15,
}

/// Which of the lines of an orthogonal run a user offset belongs to.
const enum LineCode {
	Obj1Line2,
	Obj1Line3,
	Obj2Line2,
	Obj2Line3,
	MiddleLine,
}

/// A point of the run as the engine builds it: a point the run goes through, or a weight of the
/// curve, or a point the curve runs symmetrically through.
interface TrackPoint {
	x: number;
	y: number;
	flag: 'normal' | 'control' | 'symmetric';
}

/*
	What the routing learns about a run and carries from one run to the next: the angles the
	run leaves its two ends at, how many lines it has at each end, where its middle line starts,
	and the offsets a user dragged into those lines.
*/
class RouteInfo {
	public obj1Line2: cool.Point = new cool.Point(0, 0);
	public obj1Line3: cool.Point = new cool.Point(0, 0);
	public obj2Line2: cool.Point = new cool.Point(0, 0);
	public obj2Line3: cool.Point = new cool.Point(0, 0);
	public middleLine: cool.Point = new cool.Point(0, 0);
	public angle1: number = 0;
	public angle2: number = 0;
	public obj1Lines: number = 0;
	public obj2Lines: number = 0;
	/// The number of the point the middle line starts at, or none.
	public middleLineAt: number | undefined = undefined;
	public curveFollowsOOXML: boolean = false;

	public clone(): RouteInfo {
		const copy = new RouteInfo();
		copy.obj1Line2 = this.obj1Line2.clone();
		copy.obj1Line3 = this.obj1Line3.clone();
		copy.obj2Line2 = this.obj2Line2.clone();
		copy.obj2Line3 = this.obj2Line3.clone();
		copy.middleLine = this.middleLine.clone();
		copy.angle1 = this.angle1;
		copy.angle2 = this.angle2;
		copy.obj1Lines = this.obj1Lines;
		copy.obj2Lines = this.obj2Lines;
		copy.middleLineAt = this.middleLineAt;
		copy.curveFollowsOOXML = this.curveFollowsOOXML;
		return copy;
	}

	public offsetPoint(code: LineCode): cool.Point {
		switch (code) {
			case LineCode.Obj1Line2:
				return this.obj1Line2;
			case LineCode.Obj1Line3:
				return this.obj1Line3;
			case LineCode.Obj2Line2:
				return this.obj2Line2;
			case LineCode.Obj2Line3:
				return this.obj2Line3;
			default:
				return this.middleLine;
		}
	}

	public polygonIndex(code: LineCode, track: TrackPoint[]): number {
		switch (code) {
			case LineCode.Obj1Line2:
				return 1;
			case LineCode.Obj1Line3:
				return 2;
			case LineCode.Obj2Line2:
				return track.length - 3;
			case LineCode.Obj2Line3:
				return track.length - 4;
			default:
				return this.middleLineAt ?? 0;
		}
	}

	public isHorizontalLine(code: LineCode, track: TrackPoint[]): boolean {
		let index = this.polygonIndex(code, track);
		let horizontal = this.angle1 === 0 || this.angle1 === 18000;
		if (code === LineCode.Obj2Line2 || code === LineCode.Obj2Line3) {
			index = track.length - index;
			horizontal = this.angle2 === 0 || this.angle2 === 18000;
		}
		if ((index & 1) === 1) horizontal = !horizontal;
		return horizontal;
	}

	public setLineOffset(
		code: LineCode,
		track: TrackPoint[],
		value: number,
	): void {
		const point = this.offsetPoint(code);
		if (this.isHorizontalLine(code, track)) point.y = value;
		else point.x = value;
	}
}

class ConnectorRouter {
	/// The ways a glue point says a connector leaves the object there.
	private static escapeOf(wayOut: string | undefined): Escape {
		switch (wayOut) {
			case 'left':
				return Escape.Left;
			case 'right':
				return Escape.Right;
			case 'top':
				return Escape.Top;
			case 'bottom':
				return Escape.Bottom;
			case 'horizontal':
				return Escape.Horizontal;
			case 'vertical':
				return Escape.Vertical;
			default:
				return Escape.Smart;
		}
	}

	private static center(rect: cool.Range2D): cool.Point {
		return new cool.Point(rect.centerX, rect.centerY);
	}

	/*
		The way a connector leaves a point that says none itself, worked out from where the point
		lies on the object's snap rectangle: on a side it leaves through that side, in a corner
		through either of the two sides meeting there, in the middle any way.
	*/
	private static escapeAngleOf(
		node: cool.RouteNode | undefined,
		point: cool.Point,
	): Escape {
		if (!node) return Escape.All;
		const rect = node.snap;
		const dxl = point.x - rect.minX;
		const dyo = point.y - rect.minY;
		const dxr = rect.maxX - point.x;
		const dyu = rect.maxY - point.y;
		// Two of the model's units is what the engine takes as "the same", which is about three
		// twips.
		const same = 3;
		const xMiddle = Math.abs(dxl - dxr) < same;
		const yMiddle = Math.abs(dyo - dyu) < same;
		const dx = Math.min(dxl, dxr);
		const dy = Math.min(dyo, dyu);
		const diagonal = Math.abs(dx - dy) < same;
		if (xMiddle && yMiddle) return Escape.All;
		if (diagonal) {
			let ret: number = Escape.Smart;
			if (yMiddle) ret |= Escape.Vertical;
			if (xMiddle) ret |= Escape.Horizontal;
			const side = dxl < dxr ? Escape.Left : Escape.Right;
			ret |= side | (dyo < dyu ? Escape.Top : Escape.Bottom);
			return ret;
		}
		if (dx < dy) {
			if (xMiddle) return Escape.Horizontal;
			return dxl < dxr ? Escape.Left : Escape.Right;
		}
		if (yMiddle) return Escape.Vertical;
		return dyo < dyu ? Escape.Top : Escape.Bottom;
	}

	/*
		Where one end holds on to its object, and which ways it may leave there: the glue point the
		tie names, one of the four middles of the sides or one of the four corners for a tie the
		engine chose itself, or nothing where the tie names a point the object does not have.
	*/
	private static tiePoint(
		node: cool.RouteNode,
		tie: cool.ConnectorTie,
		number: number,
	): { point: cool.Point; escape: Escape } | undefined {
		if (tie.autoVertex) {
			// The object gives the four itself. The middles of its box stand in where it gave
			// none.
			const side = node.sideGluePoints[number];
			if (side)
				return {
					point: node.transform.apply(side.x, side.y),
					escape: Escape.Smart,
				};

			const rect = node.snap;
			const middle = ConnectorRouter.center(rect);
			const at =
				number === 0
					? new cool.Point(middle.x, rect.minY)
					: number === 1
						? new cool.Point(rect.maxX, middle.y)
						: number === 2
							? new cool.Point(middle.x, rect.maxY)
							: new cool.Point(rect.minX, middle.y);
			return { point: at, escape: Escape.Smart };
		}
		if (tie.autoCorner) {
			const rect = node.bound;
			const at =
				number === 0
					? new cool.Point(rect.minX, rect.minY)
					: number === 1
						? new cool.Point(rect.maxX, rect.minY)
						: number === 2
							? new cool.Point(rect.maxX, rect.maxY)
							: new cool.Point(rect.minX, rect.maxY);
			return { point: at, escape: Escape.Smart };
		}
		const glue = node.gluePoints.find(
			(one: cool.ObjectGluePoint) => one.id === number,
		);
		if (!glue) return undefined;
		return {
			point: node.transform.apply(glue.x, glue.y),
			escape: ConnectorRouter.escapeOf(glue.wayOut),
		};
	}

	private static directionOf(angle: number): Escape {
		return angle === 0
			? Escape.Right
			: angle === 9000
				? Escape.Top
				: angle === 18000
					? Escape.Left
					: Escape.Bottom;
	}

	/*
		The run of a connector, as the points of its path. The two ends come as what they are tied
		to, or as nothing for a free end, and a free end stays where its tie says it lies. The
		deltas a user dragged into the lines are laid on in a second pass, since which line they
		belong to is only known once the run has been worked out the first time. Nothing comes
		back for a run that was set from outside: the engine lays no such run out, and neither
		does this.
	*/
	public static route(
		routing: cool.ConnectorRouting,
		startNode: cool.RouteNode | undefined,
		endNode: cool.RouteNode | undefined,
	): cool.ObjectPathPoint[] | undefined {
		if (routing.trackUserDefined) return undefined;

		const placeOf = (tie: cool.ConnectorTie): cool.Point =>
			new cool.Point(tie.x ?? 0, tie.y ?? 0);
		const first = placeOf(routing.tieAtStart);
		const last = placeOf(routing.tieAtEnd);

		let info = new RouteInfo();
		info.curveFollowsOOXML = routing.curveFollowsOOXML === true;

		let track = ConnectorRouter.best(
			routing,
			first,
			last,
			startNode,
			endNode,
			info,
		);
		info = track.info;

		// The deltas a user dragged in, laid on the lines the first pass found.
		const deltas = routing.lineDeltas;
		if (routing.kind === 'ortho' || routing.kind === 'bezier') {
			let n = 0;
			const lay = (code: LineCode) => {
				if (n < 3) info.setLineOffset(code, track.points, deltas[n++] ?? 0);
			};
			if (info.obj1Lines >= 2) lay(LineCode.Obj1Line2);
			if (info.obj1Lines >= 3) lay(LineCode.Obj1Line3);
			if (info.middleLineAt !== undefined) lay(LineCode.MiddleLine);
			if (info.obj2Lines >= 3) lay(LineCode.Obj2Line3);
			if (info.obj2Lines >= 2) lay(LineCode.Obj2Line2);
		} else if (routing.kind === 'three') {
			const horizontal1 = info.angle1 === 0 || info.angle1 === 18000;
			const horizontal2 = info.angle2 === 0 || info.angle2 === 18000;
			if (horizontal1) info.obj1Line2.x = deltas[0] ?? 0;
			else info.obj1Line2.y = deltas[0] ?? 0;
			if (horizontal2) info.obj2Line2.x = deltas[1] ?? 0;
			else info.obj2Line2.y = deltas[1] ?? 0;
		}

		if (deltas.some((delta: number) => delta !== 0))
			track = ConnectorRouter.best(
				routing,
				first,
				last,
				startNode,
				endNode,
				info,
			);

		return ConnectorRouter.asPath(track.points);
	}

	/*
		The best run over every way the two ends may leave their points: every combination of the
		four sides each end allows is tried, and for an end the engine ties to the best of the
		four middles itself, each of those four as well. The run of the least cost wins, the first
		one found where they tie.
	*/
	private static best(
		routing: cool.ConnectorRouting,
		first: cool.Point,
		last: cool.Point,
		startNode: cool.RouteNode | undefined,
		endNode: cool.RouteNode | undefined,
		info: RouteInfo,
	): { points: TrackPoint[]; info: RouteInfo } {
		const distances = routing.nodeDistances;
		const against = (node: cool.RouteNode) =>
			routing.againstSnapRectangles ? node.snap : node.bound;

		let bound1: cool.Range2D;
		let beware1: cool.Range2D;
		if (startNode) {
			bound1 = against(startNode);
			beware1 = bound1.expand(distances[0] ?? 0, distances[1] ?? 0);
		} else {
			bound1 = new cool.Range2D(first.x, first.y, first.x, first.y);
			beware1 = bound1;
		}

		let bound2: cool.Range2D;
		let beware2: cool.Range2D;
		if (endNode) {
			bound2 = against(endNode);
			beware2 = bound2.expand(distances[2] ?? 0, distances[3] ?? 0);
		} else {
			bound2 = new cool.Range2D(last.x, last.y, last.x, last.y);
			beware2 = bound2;
		}

		const tie1 = routing.tieAtStart;
		const tie2 = routing.tieAtEnd;
		const auto1 = startNode !== undefined && tie1.bestVertex === true;
		const auto2 = endNode !== undefined && tie2.bestVertex === true;
		const count1 = auto1 ? 4 : 1;
		const count2 = auto2 ? 4 : 1;

		let bestPoints: TrackPoint[] = [];
		let bestQuality = Number.POSITIVE_INFINITY;
		let bestInfo = info.clone();

		for (let number1 = 0; number1 < count1; ++number1) {
			let point1 = first;
			let escape1: Escape = Escape.All;
			if (startNode) {
				const tie = auto1 ? { ...tie1, autoVertex: true } : tie1;
				const found = ConnectorRouter.tiePoint(
					startNode,
					tie,
					auto1 ? number1 : (tie1.point ?? 0),
				);
				if (found) {
					point1 = found.point;
					escape1 = found.escape;
					if (escape1 === Escape.Smart)
						escape1 = ConnectorRouter.escapeAngleOf(startNode, point1);
				}
			}
			for (let number2 = 0; number2 < count2; ++number2) {
				let point2 = last;
				let escape2: Escape = Escape.All;
				if (endNode) {
					const tie = auto2 ? { ...tie2, autoVertex: true } : tie2;
					const found = ConnectorRouter.tiePoint(
						endNode,
						tie,
						auto2 ? number2 : (tie2.point ?? 0),
					);
					if (found) {
						point2 = found.point;
						escape2 = found.escape;
						if (escape2 === Escape.Smart)
							escape2 = ConnectorRouter.escapeAngleOf(endNode, point2);
					}
				}
				for (let angle1 = 0; angle1 < 36000; angle1 += 9000) {
					if (!(escape1 & ConnectorRouter.directionOf(angle1))) continue;
					for (let angle2 = 0; angle2 < 36000; angle2 += 9000) {
						if (!(escape2 & ConnectorRouter.directionOf(angle2))) continue;

						const tryInfo = info.clone();
						const tried = ConnectorRouter.track(
							routing.kind,
							point1,
							angle1,
							bound1,
							beware1,
							point2,
							angle2,
							bound2,
							beware2,
							tryInfo,
						);
						if (tried.quality < bestQuality) {
							bestPoints = tried.points;
							bestQuality = tried.quality;
							bestInfo = tryInfo;
						}
					}
				}
			}
		}

		return { points: bestPoints, info: bestInfo };
	}

	/// The run from a point out of its object towards the meeting point, as the engine builds it:
	/// out through the side the angle names, up to the height of the meeting point, and round
	/// the object where the meeting point lies behind it.
	private static objectToCenter(
		start: cool.Point,
		angle: number,
		rect: cool.Range2D,
		meeting: cool.Point,
	): TrackPoint[] {
		const points: TrackPoint[] = [{ x: start.x, y: start.y, flag: 'normal' }];
		const right = angle === 0;
		const top = angle === 9000;
		const left = angle === 18000;
		const bottom = angle === 27000;

		const p1 = start.clone();
		if (left) p1.x = rect.minX;
		if (right) p1.x = rect.maxX;
		if (top) p1.y = rect.minY;
		if (bottom) p1.y = rect.maxY;

		const p2 = p1.clone();
		if (left && meeting.x <= p2.x) p2.x = meeting.x;
		if (right && meeting.x >= p2.x) p2.x = meeting.x;
		if (top && meeting.y <= p2.y) p2.y = meeting.y;
		if (bottom && meeting.y >= p2.y) p2.y = meeting.y;
		points.push({ x: p2.x, y: p2.y, flag: 'normal' });

		const p3 = p2.clone();
		if ((left && meeting.x > p2.x) || (right && meeting.x < p2.x)) {
			if (meeting.y < p2.y) {
				p3.y = rect.minY;
				if (meeting.y < p3.y) p3.y = meeting.y;
			} else {
				p3.y = rect.maxY;
				if (meeting.y > p3.y) p3.y = meeting.y;
			}
			points.push({ x: p3.x, y: p3.y, flag: 'normal' });
			if (p3.y !== meeting.y) {
				p3.x = meeting.x;
				points.push({ x: p3.x, y: p3.y, flag: 'normal' });
			}
		}
		if ((top && meeting.y > p2.y) || (bottom && meeting.y < p2.y)) {
			if (meeting.x < p2.x) {
				p3.x = rect.minX;
				if (meeting.x < p3.x) p3.x = meeting.x;
			} else {
				p3.x = rect.maxX;
				if (meeting.x > p3.x) p3.x = meeting.x;
			}
			points.push({ x: p3.x, y: p3.y, flag: 'normal' });
			if (p3.x !== meeting.x) {
				p3.y = meeting.y;
				points.push({ x: p3.x, y: p3.y, flag: 'normal' });
			}
		}
		return points;
	}

	private static same(
		a: TrackPoint | cool.Point,
		b: TrackPoint | cool.Point,
	): boolean {
		return a.x === b.x && a.y === b.y;
	}

	/*
		One run for one way of leaving each end, with its cost. This is the engine's routing for
		a given pair of angles, step by step: the meeting point between the objects, the run out of
		each object towards it, the two joined, the shape named, the cost summed up, and the user's
		offsets laid on the lines. The cost counts the length, the corners, and every time the run
		cuts through an object or leaves its end the wrong way.
	*/
	private static track(
		kind: cool.ConnectorRouting['kind'],
		pt1: cool.Point,
		angle1: number,
		boundRect1: cool.Range2D,
		bewareRect1: cool.Range2D,
		pt2: cool.Point,
		angle2: number,
		boundRect2: cool.Range2D,
		bewareRect2: cool.Range2D,
		info: RouteInfo,
	): { points: TrackPoint[]; quality: number } {
		const rts1 = angle1 === 0;
		const obn1 = angle1 === 9000;
		const lks1 = angle1 === 18000;
		const unt1 = angle1 === 27000;
		const hor1 = lks1 || rts1;
		const ver1 = obn1 || unt1;
		const rts2 = angle2 === 0;
		const obn2 = angle2 === 9000;
		const lks2 = angle2 === 18000;
		const unt2 = angle2 === 27000;
		const hor2 = lks2 || rts2;
		const ver2 = obn2 || unt2;

		info.angle1 = angle1;
		info.angle2 = angle2;
		info.obj1Lines = 1;
		info.obj2Lines = 1;
		info.middleLineAt = undefined;

		const aPt1 = pt1.clone();
		const aPt2 = pt2.clone();
		let aBewareRect1 = bewareRect1;
		let aBewareRect2 = bewareRect2;
		const aBoundRect1 = boundRect1;
		const aBoundRect2 = boundRect2;
		const meeting = new cool.Point(
			(aPt1.x + aPt2.x) / 2,
			(aPt1.y + aPt2.y) / 2,
		);

		const manhattan = (
			a: TrackPoint | cool.Point,
			b: TrackPoint | cool.Point,
		) => Math.abs(a.x - b.x) + Math.abs(a.y - b.y);

		if (kind === 'one') {
			return {
				points: [
					{ x: pt1.x, y: pt1.y, flag: 'normal' },
					{ x: pt2.x, y: pt2.y, flag: 'normal' },
				],
				quality: manhattan(pt1, pt2),
			};
		}

		if (kind === 'three') {
			const points: TrackPoint[] = [
				{ x: pt1.x, y: pt1.y, flag: 'normal' },
				{ x: pt1.x, y: pt1.y, flag: 'normal' },
				{ x: pt2.x, y: pt2.y, flag: 'normal' },
				{ x: pt2.x, y: pt2.y, flag: 'normal' },
			];
			if (rts1) points[1].x = aBewareRect1.maxX;
			if (obn1) points[1].y = aBewareRect1.minY;
			if (lks1) points[1].x = aBewareRect1.minX;
			if (unt1) points[1].y = aBewareRect1.maxY;
			if (rts2) points[2].x = aBewareRect2.maxX;
			if (obn2) points[2].y = aBewareRect2.minY;
			if (lks2) points[2].x = aBewareRect2.minX;
			if (unt2) points[2].y = aBewareRect2.maxY;
			const quality =
				manhattan(points[1], points[0]) +
				manhattan(points[2], points[1]) +
				manhattan(points[3], points[2]);
			info.obj1Lines = 2;
			info.obj2Lines = 2;
			if (hor1) points[1].x += info.obj1Line2.x;
			else points[1].y += info.obj1Line2.y;
			if (hor2) points[2].x += info.obj2Line2.x;
			else points[2].y += info.obj2Line2.y;
			return { points, quality };
		}

		let intersections = 0;
		{
			const c1 = ConnectorRouter.center(aBewareRect1);
			const c2 = ConnectorRouter.center(aBewareRect2);
			if (
				aBewareRect1.minX <= aBewareRect2.maxX &&
				aBewareRect1.maxX >= aBewareRect2.minX
			) {
				const n1 = Math.max(aBewareRect1.minX, aBewareRect2.minX);
				const n2 = Math.min(aBewareRect1.maxX, aBewareRect2.maxX);
				meeting.x = (n1 + n2) / 2;
			} else if (c1.x < c2.x) {
				meeting.x = (aBewareRect1.maxX + aBewareRect2.minX) / 2;
			} else {
				meeting.x = (aBewareRect1.minX + aBewareRect2.maxX) / 2;
			}
			if (
				aBewareRect1.minY <= aBewareRect2.maxY &&
				aBewareRect1.maxY >= aBewareRect2.minY
			) {
				const n1 = Math.max(aBewareRect1.minY, aBewareRect2.minY);
				const n2 = Math.min(aBewareRect1.maxY, aBewareRect2.maxY);
				meeting.y = (n1 + n2) / 2;
			} else if (c1.y < c2.y) {
				meeting.y = (aBewareRect1.maxY + aBewareRect2.minY) / 2;
			} else {
				meeting.y = (aBewareRect1.minY + aBewareRect2.maxY) / 2;
			}

			const xMin = Math.min(aBewareRect1.minX, aBewareRect2.minX);
			const xMax = Math.max(aBewareRect1.maxX, aBewareRect2.maxX);
			const yMin = Math.min(aBewareRect1.minY, aBewareRect2.minY);
			const yMax = Math.max(aBewareRect1.maxY, aBewareRect2.maxY);
			const bewareOverlap =
				aBewareRect1.maxX > aBewareRect2.minX &&
				aBewareRect1.minX < aBewareRect2.maxX &&
				aBewareRect1.maxY > aBewareRect2.minY &&
				aBewareRect1.minY < aBewareRect2.maxY;

			let mainCase = 3;
			if (angle1 === angle2) mainCase = 1;
			else if ((hor1 && hor2) || (ver1 && ver2)) mainCase = 2;

			if (mainCase === 1) {
				// Both leave the same way.
				if (ver1) meeting.x = (aPt1.x + aPt2.x) / 2;
				if (hor1) meeting.y = (aPt1.y + aPt2.y) / 2;
				const x1Ok = aPt1.x <= aBewareRect2.minX || aPt1.x >= aBewareRect2.maxX;
				const x2Ok = aPt2.x <= aBewareRect1.minX || aPt2.x >= aBewareRect1.maxX;
				const y1Ok = aPt1.y <= aBewareRect2.minY || aPt1.y >= aBewareRect2.maxY;
				const y2Ok = aPt2.y <= aBewareRect1.minY || aPt2.y >= aBewareRect1.maxY;
				if (
					lks1 &&
					(y1Ok || aBewareRect1.minX < aBewareRect2.maxX) &&
					(y2Ok || aBewareRect2.minX < aBewareRect1.maxX)
				)
					meeting.x = xMin;
				if (
					rts1 &&
					(y1Ok || aBewareRect1.maxX > aBewareRect2.minX) &&
					(y2Ok || aBewareRect2.maxX > aBewareRect1.minX)
				)
					meeting.x = xMax;
				if (
					obn1 &&
					(x1Ok || aBewareRect1.minY < aBewareRect2.maxY) &&
					(x2Ok || aBewareRect2.minY < aBewareRect1.maxY)
				)
					meeting.y = yMin;
				if (
					unt1 &&
					(x1Ok || aBewareRect1.maxY > aBewareRect2.minY) &&
					(x2Ok || aBewareRect2.maxY > aBewareRect1.minY)
				)
					meeting.y = yMax;
			} else if (mainCase === 2) {
				// Both leave along one axis, facing or not.
				if (hor1) {
					const bewR1 = rts1 ? aBewareRect1 : aBewareRect2;
					const bewR2 = rts1 ? aBewareRect2 : aBewareRect1;
					const bndR1 = rts1 ? aBoundRect1 : aBoundRect2;
					const bndR2 = rts1 ? aBoundRect2 : aBoundRect1;
					if (bewR1.maxY > bewR2.minY && bewR1.minY < bewR2.maxY) {
						if (bewR1.maxX > bewR2.minX) {
							let case29Direct = false;
							const case29 = bewR1.maxX > bewR2.minX;
							if (bndR1.maxX <= bndR2.minX) {
								if (
									(aPt1.y > aBewareRect2.minY && aPt1.y < aBewareRect2.maxY) ||
									(aPt2.y > aBewareRect1.minY && aPt2.y < aBewareRect1.maxY)
								)
									case29Direct = true;
							}
							if (!case29Direct) {
								const topLong =
									Math.abs(yMin - meeting.y) <= Math.abs(yMax - meeting.y);
								meeting.y = topLong ? yMin : yMax;
								if (case29) {
									if (
										ConnectorRouter.center(bewR1).y <
											ConnectorRouter.center(bewR2).y !==
										topLong
									)
										meeting.x = bewR2.maxX;
									else meeting.x = bewR1.minX;
								}
							} else {
								// A direct run of three lines has to go through the distances
								// kept, so they are scaled down by rule of three.
								const want1 = bewR1.maxX - bndR1.maxX;
								const want2 = bndR2.minX - bewR2.minX;
								const space = bndR2.minX - bndR1.maxX;
								const get1 =
									want1 + want2 === 0
										? 0
										: Math.round((want1 * space) / (want1 + want2));
								const get2 = space - get1;
								if (rts1) {
									aBewareRect1 = new cool.Range2D(
										aBewareRect1.minX,
										aBewareRect1.minY,
										aBewareRect1.maxX + (get1 - want1),
										aBewareRect1.maxY,
									);
									aBewareRect2 = new cool.Range2D(
										aBewareRect2.minX - (get2 - want2),
										aBewareRect2.minY,
										aBewareRect2.maxX,
										aBewareRect2.maxY,
									);
								} else {
									aBewareRect2 = new cool.Range2D(
										aBewareRect2.minX,
										aBewareRect2.minY,
										aBewareRect2.maxX + (get1 - want1),
										aBewareRect2.maxY,
									);
									aBewareRect1 = new cool.Range2D(
										aBewareRect1.minX - (get2 - want2),
										aBewareRect1.minY,
										aBewareRect1.maxX,
										aBewareRect1.maxY,
									);
								}
								intersections++;
							}
						}
					}
				} else if (ver1) {
					const bewR1 = unt1 ? aBewareRect1 : aBewareRect2;
					const bewR2 = unt1 ? aBewareRect2 : aBewareRect1;
					const bndR1 = unt1 ? aBoundRect1 : aBoundRect2;
					const bndR2 = unt1 ? aBoundRect2 : aBoundRect1;
					if (bewR1.maxX > bewR2.minX && bewR1.minX < bewR2.maxX) {
						if (bewR1.maxY > bewR2.minY) {
							let case29Direct = false;
							const case29 = bewR1.maxY > bewR2.minY;
							if (bndR1.maxY <= bndR2.minY) {
								if (
									(aPt1.x > aBewareRect2.minX && aPt1.x < aBewareRect2.maxX) ||
									(aPt2.x > aBewareRect1.minX && aPt2.x < aBewareRect1.maxX)
								)
									case29Direct = true;
							}
							if (!case29Direct) {
								const leftLong =
									Math.abs(xMin - meeting.x) <= Math.abs(xMax - meeting.x);
								meeting.x = leftLong ? xMin : xMax;
								if (case29) {
									if (
										ConnectorRouter.center(bewR1).x <
											ConnectorRouter.center(bewR2).x !==
										leftLong
									)
										meeting.y = bewR2.maxY;
									else meeting.y = bewR1.minY;
								}
							} else {
								const want1 = bewR1.maxY - bndR1.maxY;
								const want2 = bndR2.minY - bewR2.minY;
								const space = bndR2.minY - bndR1.maxY;
								const get1 =
									want1 + want2 === 0
										? 0
										: Math.round((want1 * space) / (want1 + want2));
								const get2 = space - get1;
								if (unt1) {
									aBewareRect1 = new cool.Range2D(
										aBewareRect1.minX,
										aBewareRect1.minY,
										aBewareRect1.maxX,
										aBewareRect1.maxY + (get1 - want1),
									);
									aBewareRect2 = new cool.Range2D(
										aBewareRect2.minX,
										aBewareRect2.minY - (get2 - want2),
										aBewareRect2.maxX,
										aBewareRect2.maxY,
									);
								} else {
									aBewareRect2 = new cool.Range2D(
										aBewareRect2.minX,
										aBewareRect2.minY,
										aBewareRect2.maxX,
										aBewareRect2.maxY + (get1 - want1),
									);
									aBewareRect1 = new cool.Range2D(
										aBewareRect1.minX,
										aBewareRect1.minY - (get2 - want2),
										aBewareRect1.maxX,
										aBewareRect1.maxY,
									);
								}
								intersections++;
							}
						}
					}
				}
			} else {
				// One leaves along one axis, the other along the other.
				let tmpR1 = aBewareRect1;
				let tmpR2 = aBewareRect2;
				if (bewareOverlap) {
					tmpR1 = aBoundRect1;
					tmpR2 = aBoundRect2;
				}
				if (
					(((rts1 && tmpR1.maxX <= aPt2.x) || (lks1 && tmpR1.minX >= aPt2.x)) &&
						((unt2 && tmpR2.maxY <= aPt1.y) ||
							(obn2 && tmpR2.minY >= aPt1.y))) ||
					(((rts2 && tmpR2.maxX <= aPt1.x) || (lks2 && tmpR2.minX >= aPt1.x)) &&
						((unt1 && tmpR1.maxY <= aPt2.y) || (obn1 && tmpR1.minY >= aPt2.y)))
				) {
					// The objects face each other openly: two lines do.
					if (hor1) {
						meeting.x = aPt2.x;
						meeting.y = aPt1.y;
					} else {
						meeting.x = aPt1.x;
						meeting.y = aPt2.y;
					}
					aBewareRect1 = tmpR1;
					aBewareRect2 = tmpR2;
				} else if (
					(((rts1 && aBewareRect1.maxX > aBewareRect2.minX) ||
						(lks1 && aBewareRect1.minX < aBewareRect2.maxX)) &&
						((unt2 && aBewareRect2.maxY > aBewareRect1.minY) ||
							(obn2 && aBewareRect2.minY < aBewareRect1.maxY))) ||
					(((rts2 && aBewareRect2.maxX > aBewareRect1.minX) ||
						(lks2 && aBewareRect2.minX < aBewareRect1.maxX)) &&
						((unt1 && aBewareRect1.maxY > aBewareRect2.minY) ||
							(obn1 && aBewareRect1.minY < aBewareRect2.maxY)))
				) {
					// The ways out point away from the other object or miss its back.
					if (rts1 || rts2) meeting.x = xMax;
					if (lks1 || lks2) meeting.x = xMin;
					if (unt1 || unt2) meeting.y = yMax;
					if (obn1 || obn2) meeting.y = yMin;
				}
			}
		}

		const xp1 = ConnectorRouter.objectToCenter(
			aPt1,
			angle1,
			aBewareRect1,
			meeting,
		);
		const xp2 = ConnectorRouter.objectToCenter(
			aPt2,
			angle2,
			aBewareRect2,
			meeting,
		);
		let xp1Count = xp1.length;
		let xp2Count = xp2.length;
		info.obj1Lines = xp1Count - 1;
		info.obj2Lines = xp2Count - 1;

		const ep1 = xp1[xp1Count - 1];
		const ep2 = xp2[xp2Count - 1];
		const insertMeeting = ep1.x !== ep2.x && ep1.y !== ep2.y;
		const horizontalE1 = ep1.y === xp1[xp1Count - 2].y;
		const horizontalE2 = ep2.y === xp2[xp2Count - 2].y;
		if (
			ConnectorRouter.same(ep1, ep2) &&
			((horizontalE1 && horizontalE2 && ep1.y === ep2.y) ||
				(!horizontalE1 && !horizontalE2 && ep1.x === ep2.x))
		) {
			// An 'I': the two runs meet head on and the meeting point is dropped from both.
			xp1Count--;
			xp1.splice(xp1Count, 1);
			xp2Count--;
			xp2.splice(xp2Count, 1);
		}
		if (insertMeeting) {
			xp1.push({ x: meeting.x, y: meeting.y, flag: 'normal' });
			// The meeting point adds two lines, and either may become the middle line.
			if (info.obj1Lines === info.obj2Lines) {
				info.obj1Lines++;
				info.obj2Lines++;
			} else if (info.obj1Lines > info.obj2Lines) {
				info.obj2Lines++;
				info.middleLineAt = xp1Count - 1;
			} else {
				info.obj1Lines++;
				info.middleLineAt = xp1Count;
			}
		} else if (!ConnectorRouter.same(ep1, ep2) && xp1Count + xp2Count >= 4) {
			// Joining the two ends adds a line, which becomes the middle line.
			info.middleLineAt = xp1Count - 1;
		}

		let num = xp2.length;
		if (
			xp1Count > 1 &&
			xp2Count > 1 &&
			ConnectorRouter.same(xp1[xp1Count - 1], xp2[xp2Count - 1])
		)
			num--;
		while (num > 0) {
			num--;
			xp1.push({ ...xp2[num] });
		}

		const points = xp1;
		let pointCount = points.length;

		let form = '?';
		if (pointCount === 2) form = 'I';
		else if (pointCount === 3) form = 'L';
		else if (pointCount === 4) form = angle1 === angle2 ? 'U' : 'Z';
		else if (pointCount === 6) {
			if (angle1 !== angle2) {
				const p1 = points[1];
				const p2 = points[2];
				const p3 = points[3];
				const p4 = points[4];
				if (p1.y === p2.y) form = p1.x < p2.x === p3.x < p4.x ? 'S' : 'C';
				else form = p1.y < p2.y === p3.y < p4.y ? 'S' : 'C';
			} else form = '4';
		}

		if (form === 'I' || form === 'L' || form === 'Z' || form === 'U') {
			info.obj1Lines = 1;
			info.obj2Lines = 1;
			info.middleLineAt = form === 'Z' || form === 'U' ? 1 : undefined;
		} else if (form === 'S' || form === 'C') {
			info.obj1Lines = 2;
			info.obj2Lines = 2;
			info.middleLineAt = 2;
		}

		// The cost: the length, the corners, and every time the run cuts through an object or
		// leaves its end the wrong way.
		let quality = 0;
		let pt0 = points[0];
		for (let n = 1; n < pointCount; ++n) {
			quality += manhattan(points[n], pt0);
			pt0 = points[n];
		}
		let corners = pointCount;
		if (form === 'Z') {
			corners = 2;
			let n1 = manhattan(points[1], points[0]);
			const n2 = manhattan(points[2], points[1]);
			let n3 = manhattan(points[3], points[2]);
			let better = 0;
			n1 += n3;
			n3 = n2 / 4;
			if (n1 >= n2) better = 6;
			else if (n1 >= 3 * n3) better = 4;
			else if (n1 >= 2 * n3) better = 2;
			if (points[0].y !== points[1].y) better++;
			quality = quality > better ? quality - better : 0;
		}
		if (corners >= 3) quality += corners * 0x01000000;
		if (pointCount >= 2) {
			const p1 = new cool.Point(
				points[1].x - points[0].x,
				points[1].y - points[0].y,
			);
			const p2 = {
				x: points[pointCount - 2].x - points[pointCount - 1].x,
				y: points[pointCount - 2].y - points[pointCount - 1].y,
			};
			let ang1 = 0;
			if (p1.x < 0) ang1 = 18000;
			if (p1.y > 0) ang1 = 27000;
			if (p1.y < 0) ang1 = 9000;
			if (p1.x !== 0 && p1.y !== 0) ang1 = 1;
			let ang2 = 0;
			if (p2.x < 0) ang2 = 18000;
			if (p2.y > 0) ang2 = 27000;
			if (p2.y < 0) ang2 = 9000;
			if (p2.x !== 0 && p2.y !== 0) ang2 = 1;
			if (ang1 !== angle1) intersections++;
			if (ang2 !== angle2) intersections++;
		}
		// The check is done against the rectangles as they were given, since one of them may have
		// been scaled down for the run.
		const inside = (p: TrackPoint, r: cool.Range2D) =>
			p.x > r.minX && p.x < r.maxX && p.y > r.minY && p.y < r.maxY;
		pt0 = points[0];
		for (let i = 0; i < pointCount; ++i) {
			const p = points[i];
			const b1 = inside(p, bewareRect1);
			const b2 = inside(p, bewareRect2);
			const before = intersections;
			if (i === 0 || i === pointCount - 1) {
				if (b1 && b2) intersections++;
			} else {
				if (b1) intersections++;
				if (b2) intersections++;
			}
			if (i > 0 && before === intersections) {
				const crosses = (r: cool.Range2D) =>
					pt0.y === p.y
						? pt0.y > r.minY &&
							pt0.y < r.maxY &&
							((pt0.x <= r.minX && p.x >= r.maxX) ||
								(p.x <= r.minX && pt0.x >= r.maxX))
						: pt0.x > r.minX &&
							pt0.x < r.maxX &&
							((pt0.y <= r.minY && p.y >= r.maxY) ||
								(p.y <= r.minY && pt0.y >= r.maxY));
				if (crosses(bewareRect1)) intersections++;
				if (crosses(bewareRect2)) intersections++;
			}
			pt0 = p;
		}
		if (pointCount <= 1) intersections++;
		quality += intersections * 0x10000000;

		// The user's offsets go onto the lines they belong to.
		const shift = (code: LineCode, by: cool.Point) => {
			const index = info.polygonIndex(code, points);
			if (index < 0 || index + 1 >= points.length) return;
			if (info.isHorizontalLine(code, points)) {
				points[index].y += by.y;
				points[index + 1].y += by.y;
			} else {
				points[index].x += by.x;
				points[index + 1].x += by.x;
			}
		};
		if (info.middleLineAt !== undefined)
			shift(LineCode.MiddleLine, info.middleLine);
		if (info.obj1Lines >= 2) shift(LineCode.Obj1Line2, info.obj1Line2);
		if (info.obj1Lines >= 3) shift(LineCode.Obj1Line3, info.obj1Line3);
		if (info.obj2Lines >= 2) shift(LineCode.Obj2Line2, info.obj2Line2);
		if (info.obj2Lines >= 3) shift(LineCode.Obj2Line3, info.obj2Line3);

		if (kind !== 'bezier' || pointCount <= 2) return { points, quality };

		if (info.curveFollowsOOXML) {
			// The weights lie on the lines of the orthogonal run, as the OOXML routing wants it.
			const segmentPoint = (end: number, factor: number): TrackPoint => ({
				x: points[end - 1].x + factor * (points[end].x - points[end - 1].x),
				y: points[end - 1].y + factor * (points[end].y - points[end - 1].y),
				flag: 'control',
			});
			points.splice(pointCount - 1, 0, segmentPoint(pointCount - 1, 0.5));
			for (let segment = pointCount - 2; segment > 1; --segment) {
				const control = segmentPoint(segment, 0.25);
				const normal = {
					...segmentPoint(segment, 0.5),
					flag: 'normal' as const,
				};
				const replaced = segmentPoint(segment, 0.75);
				points[segment] = replaced;
				points.splice(segment, 0, control, normal);
			}
			points[1] = segmentPoint(1, 0.5);
			return { points, quality };
		}

		// The office's own curve: the ends of the lines become the weights.
		const dx1 = points[1].x - points[0].x;
		const dy1 = points[1].y - points[0].y;
		const dx2 = points[pointCount - 2].x - points[pointCount - 1].x;
		const dy2 = points[pointCount - 2].y - points[pointCount - 1].y;
		if (form === 'L') {
			points[1].flag = 'control';
			points.splice(2, 0, { ...points[1], flag: 'control' });
			pointCount = points.length;
			points[1].x -= dx1 / 3;
			points[1].y -= dy1 / 3;
			points[pointCount - 2].x -= dx2 / 3;
			points[pointCount - 2].y -= dy2 / 3;
		} else if (pointCount >= 4 && pointCount <= 6) {
			points[1].flag = 'control';
			points[pointCount - 2].flag = 'control';
			points[1].x += dx1 / 2;
			points[1].y += dy1 / 2;
			points[pointCount - 2].x += dx2 / 2;
			points[pointCount - 2].y += dy2 / 2;
			if (pointCount === 5) {
				const center = { ...points[2] };
				const dx1b = center.x - points[1].x;
				const dy1b = center.y - points[1].y;
				const dx2b = center.x - points[3].x;
				const dy2b = center.y - points[3].y;
				points.splice(2, 0, { ...center, flag: 'control' });
				points[3].flag = 'symmetric';
				points.splice(4, 0, { ...center, flag: 'control' });
				points[2].x -= dx1b / 2;
				points[2].y -= dy1b / 2;
				points[3].x -= (dx1b + dx2b) / 4;
				points[3].y -= (dy1b + dy2b) / 4;
				points[4].x -= dx2b / 2;
				points[4].y -= dy2b / 2;
			}
			if (pointCount === 6) {
				const pt1b = { ...points[2] };
				const pt2b = { ...points[3] };
				points.splice(2, 0, { ...pt1b, flag: 'control' });
				points.splice(5, 0, { ...pt2b, flag: 'control' });
				const dx = pt1b.x - pt2b.x;
				const dy = pt1b.y - pt2b.y;
				points[3].x -= dx / 2;
				points[3].y -= dy / 2;
				points[3].flag = 'symmetric';
				points.splice(4, 1);
			}
		}
		return { points, quality };
	}

	/// The run as the points of a path: a weight ahead of or behind the point it belongs to.
	private static asPath(track: TrackPoint[]): cool.ObjectPathPoint[] {
		const path: cool.ObjectPathPoint[] = [];
		let pendingAhead: TrackPoint | undefined;
		let pendingBehind: TrackPoint | undefined;
		for (const point of track) {
			if (point.flag === 'control') {
				if (!pendingAhead && path.length) pendingAhead = point;
				else pendingBehind = point;
				continue;
			}
			const one: cool.ObjectPathPoint = { x: point.x, y: point.y };
			if (point.flag === 'symmetric') one.continuity = 2;
			if (pendingAhead && path.length) {
				const previous = path[path.length - 1];
				previous.aheadX = pendingAhead.x;
				previous.aheadY = pendingAhead.y;
			}
			if (pendingBehind) {
				one.behindX = pendingBehind.x;
				one.behindY = pendingBehind.y;
			}
			pendingAhead = undefined;
			pendingBehind = undefined;
			path.push(one);
		}
		return path;
	}

	/// The two rectangles and the glue points of an object, for a connector tied to it.
	public static nodeOf(
		object: cool.SlideObject | undefined,
	): cool.RouteNode | undefined {
		if (!object?.transform) return undefined;
		const transform = cool.Matrix2D.fromArray(object.transform);
		if (!transform) return undefined;
		const snap = new cool.Range2D(0, 0, 1, 1).transform(transform);
		const bound =
			object.x !== undefined && object.y !== undefined
				? new cool.Range2D(
						object.x,
						object.y,
						object.x + (object.width ?? 0),
						object.y + (object.height ?? 0),
					)
				: snap;
		return {
			snap,
			bound,
			transform,
			gluePoints: object.gluePoints ?? [],
			sideGluePoints: object.sideGluePoints ?? [],
		};
	}

	/*
		The connectors of the part that hold on to any of these objects with an end. Each
		connector is named once however many of its ends hold on; its own tie nodes say which
		ends do. A run set from outside counts too: the engine lets go of that mark as soon as a
		tied object moves or an end is moved, and lays the run out.
	*/
	public static tiedTo(
		data: cool.VectorPrimitivesData,
		objectIds: number[],
	): number[] {
		const held = new Set(objectIds);
		const found: number[] = [];
		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object?.routing) continue;
			if (
				(object.tiedToAtStart !== undefined &&
					held.has(object.tiedToAtStart)) ||
				(object.tiedToAtEnd !== undefined && held.has(object.tiedToAtEnd))
			)
				found.push(id);
		}
		return found;
	}

	/*
		The run of a connector while some of the objects it may hold on to stand moved by a
		mapping: an end tied to one of those is routed from where the mapping puts the object, an
		end tied to anything else from where the object stands, and a free end stays. Nothing for
		a run set from outside.
	*/
	public static routeWithMoved(
		data: cool.VectorPrimitivesData,
		connector: cool.SlideObject,
		moved: Set<number>,
		matrix: cool.Matrix2D | null,
	): cool.ObjectPathPoint[] | undefined {
		if (!connector.routing) return undefined;

		const nodeFor = (
			objectId: number | undefined,
		): cool.RouteNode | undefined => {
			if (objectId === undefined) return undefined;
			const node = ConnectorRouter.nodeOf(data.objects.get(objectId));
			if (!node || !matrix || !moved.has(objectId)) return node;
			return ConnectorRouter.nodeThrough(node, matrix);
		};

		// A move lays the run out whether or not it was set from outside, as the engine does.
		let routing: cool.ConnectorRouting = {
			...connector.routing,
			trackUserDefined: false,
		};

		// A connector that is itself among the moved has its free ends moved. A tied end stays
		// with its object, moved or not.
		if (matrix && connector.id !== undefined && moved.has(connector.id)) {
			const movedTie = (tie: cool.ConnectorTie): cool.ConnectorTie => {
				if (tie.x === undefined || tie.y === undefined) return tie;
				const at = matrix.apply(tie.x, tie.y);
				return { ...tie, x: at.x, y: at.y };
			};
			routing = {
				...routing,
				tieAtStart: movedTie(routing.tieAtStart),
				tieAtEnd: movedTie(routing.tieAtEnd),
			};
		}

		return ConnectorRouter.route(
			routing,
			nodeFor(connector.tiedToAtStart),
			nodeFor(connector.tiedToAtEnd),
		);
	}

	/*
		The run of a connector while one of its ends is being dragged: that end holds on to the
		given object the way the tie says, or lies free where the tie says, and the other end
		stays as it is. A run set from outside is laid out all the same, as the engine lays it
		out once an end is moved.
	*/
	public static routeWithAnEnd(
		data: cool.VectorPrimitivesData,
		connector: cool.SlideObject,
		atStart: boolean,
		tie: cool.ConnectorTie,
		tiedTo: number | undefined,
	): cool.ObjectPathPoint[] | undefined {
		if (!connector.routing) return undefined;

		const routing: cool.ConnectorRouting = {
			...connector.routing,
			trackUserDefined: false,
			tieAtStart: atStart ? tie : connector.routing.tieAtStart,
			tieAtEnd: atStart ? connector.routing.tieAtEnd : tie,
		};
		const nodeFor = (objectId: number | undefined) =>
			objectId === undefined
				? undefined
				: ConnectorRouter.nodeOf(data.objects.get(objectId));

		return ConnectorRouter.route(
			routing,
			nodeFor(atStart ? tiedTo : connector.tiedToAtStart),
			nodeFor(atStart ? connector.tiedToAtEnd : tiedTo),
		);
	}

	/// The node as a mapping leaves it: its snap rectangle round the mapped unit square, its
	/// bound rectangle round the mapped corners of the old one, its glue points on the mapped
	/// object.
	private static nodeThrough(
		node: cool.RouteNode,
		matrix: cool.Matrix2D,
	): cool.RouteNode {
		const transform = node.transform.then(matrix);
		return {
			transform,
			snap: new cool.Range2D(0, 0, 1, 1).transform(transform),
			bound: node.bound.transform(matrix),
			gluePoints: node.gluePoints,
			sideGluePoints: node.sideGluePoints,
		};
	}

	/// Draws a run as a hairline on a context set up in twips, curves where the run has them.
	public static strokeRun(
		context: CanvasRenderingContext2D,
		points: cool.ObjectPathPoint[],
		color: string,
	): void {
		if (points.length < 2) return;
		context.save();
		context.strokeStyle = color;
		context.lineWidth = 1 / app.twipsToPixels;
		context.beginPath();
		context.moveTo(points[0].x, points[0].y);
		for (let n = 1; n < points.length; ++n) {
			const from = points[n - 1];
			const to = points[n];
			const c1x = from.aheadX ?? from.x;
			const c1y = from.aheadY ?? from.y;
			const c2x = to.behindX ?? to.x;
			const c2y = to.behindY ?? to.y;
			if (from.aheadX !== undefined || to.behindX !== undefined)
				context.bezierCurveTo(c1x, c1y, c2x, c2y, to.x, to.y);
			else context.lineTo(to.x, to.y);
		}
		context.stroke();
		context.restore();
	}

	/// The run of a connector as the engine drew it: the first line its primitives draw, taken
	/// from them by the line geometry extractor, so nothing here knows how the tree is built.
	public static trackOf(
		object: cool.SlideObject,
	): cool.ObjectPathPoint[] | undefined {
		const lines = cool.VectorLineGeometryExtractor.extract(object.primitives);
		return lines.length ? lines[0].points : undefined;
	}

	/// How far a point lies off the line through two others, in the same unit.
	private static offTheLine(
		point: cool.ObjectPathPoint,
		a: cool.ObjectPathPoint,
		b: cool.ObjectPathPoint,
	): number {
		const start = new cool.Point(a.x, a.y);
		const along = new cool.Point(b.x, b.y).subtract(start);
		const away = new cool.Point(point.x, point.y).subtract(start);
		const length = along.length();
		if (!length) return away.length();
		return Math.abs(away.cross(along)) / length;
	}

	/*
		A check of this routing against the engine's: every connector of the part that carries its
		routing numbers is routed here from the objects as they stand, and the result is held
		against the line the engine drew for it, which is the engine's own run. The two ends are
		only held to lie on our first and last line, since an arrow head shortens the line the
		engine draws there. A corner off by more than a few twips, or another number of corners,
		is reported on the console with the connector's id. It is run from the debug panel's
		Vector Overlays tool.
	*/
	public static checkAgainstTheEngine(data: cool.VectorPrimitivesData): void {
		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object?.routing) continue;
			const engine = ConnectorRouter.trackOf(object);
			if (!engine || engine.length < 2) continue;

			const routed = ConnectorRouter.route(
				object.routing,
				ConnectorRouter.nodeOf(
					object.tiedToAtStart !== undefined
						? data.objects.get(object.tiedToAtStart)
						: undefined,
				),
				ConnectorRouter.nodeOf(
					object.tiedToAtEnd !== undefined
						? data.objects.get(object.tiedToAtEnd)
						: undefined,
				),
			);

			if (!routed) {
				console.log(
					'ROUTEPROBE id=' + String(id) + ' kept, the run was set from outside',
				);
				continue;
			}

			let worst = 0;
			if (routed.length !== engine.length || routed.length < 2)
				worst = Number.POSITIVE_INFINITY;
			else {
				const n = engine.length;
				for (let i = 1; i < n - 1; ++i)
					worst = Math.max(
						worst,
						Math.abs(routed[i].x - engine[i].x),
						Math.abs(routed[i].y - engine[i].y),
					);
				worst = Math.max(
					worst,
					ConnectorRouter.offTheLine(engine[0], routed[0], routed[1]),
					ConnectorRouter.offTheLine(
						engine[n - 1],
						routed[n - 2],
						routed[n - 1],
					),
				);
			}

			console.log(
				'ROUTEPROBE id=' +
					String(id) +
					' kind=' +
					object.routing.kind +
					' engine=' +
					String(engine.length) +
					' routed=' +
					String(routed.length) +
					' worst=' +
					(worst === Number.POSITIVE_INFINITY ? 'count' : worst.toFixed(1)) +
					(worst > 4 ? ' MISMATCH' : ''),
			);
			if (worst > 4)
				console.log(
					'ROUTEPROBE engine ' +
						JSON.stringify(
							engine.map((p) => [Math.round(p.x), Math.round(p.y)]),
						) +
						' routed ' +
						JSON.stringify(
							routed.map((p) => [Math.round(p.x), Math.round(p.y)]),
						),
				);
		}
	}
}
