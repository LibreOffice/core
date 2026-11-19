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

/// A point of a path, named by the polygon it belongs to and its place in that polygon.
interface PathPlace {
	polygon: number;
	point: number;
}

/*
	A point or a weight of a path moved: which polygon, which point of it, which of its three
	parts moved - 0 the point itself, 1 the weight of the curve coming in, 2 the one going out -
	where it moved to, in twips, and how the curve runs through that point: 0 a corner, 1 smooth,
	2 symmetric.
*/
interface PathPointMove extends PathPlace {
	part: number;
	x: number;
	y: number;
	continuity: number;
}

/*
	Where the changes of a path go. The path being edited can belong to an object, be drawn
	anew, or be a lasso or the path an animation follows; each of them takes the changes its own
	way.
*/
interface PathSink {
	/// Points or weights moved, by a drag or by keys.
	movePoints(moves: PathPointMove[]): void;
	/// A point is deleted.
	deletePoint(place: PathPlace): void;
	/// A point is added on the edge nearest to that place, in twips.
	insertPointAt(at: cool.Point): void;
	/// The edge after a point becomes a curve or a line.
	setSegmentCurved(place: PathPlace, curved: boolean): void;
	/// The curve runs through a point as a corner (0), smoothly (1) or symmetrically (2).
	setContinuity(place: PathPlace, continuity: number): void;
	/// The path is split into two at a point.
	split(place: PathPlace): void;
	/// The path shown changed here before the change came back, or went back to the one given;
	/// what shows it is to be drawn again.
	shownPathChanged(): void;
	/// The path is closed, or opened.
	setClosed(closed: boolean): void;
	/// Every curved edge that runs straight becomes a line; with pointsOnALine, the points
	/// that lie on a straight line between their neighbours go as well.
	simplify(pointsOnALine: boolean): void;
}

/*
	What a point of a path offers to be done with it: whether there is an edge after it and
	whether that edge is curved, which weights the point has, how the curve runs through it, and
	whether the path can be split there.
*/
interface PathPointFacts {
	hasAnEdgeOut: boolean;
	curved: boolean;
	weightIn: boolean;
	weightOut: boolean;
	continuity: number;
	canBeSplit: boolean;
}

/*
	Editing a path: the points it is drawn along, the weights that bend the curves between them,
	the thin lines that tie a weight to its point, the piece of line a drag of one of them would
	leave behind, and every change that can be made to the path.

	It knows nothing of what holds the path: it reads the path from where it is given and hands
	every change to a sink, so the same thing serves the path of an object, a path being drawn,
	a lasso and the path an animation follows.

	A change shows at once: the path is changed here the way the one who takes the change will
	change it, and that path is the one edited and shown until the path given changes, which
	is when the change came back.
*/
class PathEditing implements HandleSource {
	private source: () => cool.ObjectPathPolygon[] | undefined;
	private sink: PathSink;
	/// What the path belongs to, which names the path changed here across editors made anew.
	private key: string;

	/// The paths changed here, by what they belong to: the path changed, the path given it was
	/// changed from, and the wait after which it goes without an answer.
	private static changedHere = new Map<
		string,
		{
			path: cool.ObjectPathPolygon[];
			from: cool.ObjectPathPolygon[] | undefined;
			wait: ReturnType<typeof setTimeout>;
		}
	>();

	/// How long a path changed here is shown without an answer, in milliseconds.
	private static readonly longestWait: number = 5000;

	constructor(
		key: string,
		source: () => cool.ObjectPathPolygon[] | undefined,
		sink: PathSink,
	) {
		this.key = key;
		this.source = source;
		this.sink = sink;
	}

	/// How wide a point of the path is drawn, in core pixels. Smaller than a handle, and drawn
	/// over them, so that a point lying on one of the eight is still the one a press reaches.
	public static pointSize(): number {
		return HandleLook.widthOf('pathPoint');
	}

	/*
		The path whose points can be edited when these objects are selected, or undefined. That
		is the case when one object with a path is selected and it can be resized. Moving a point
		changes the size of the object, so an object protected against resizing is left out.
	*/
	public static editedPath(
		objectIds: number[],
	): cool.ObjectPathPolygon[] | undefined {
		if (objectIds.length !== 1 || !ObjectHandles.canBeResized(objectIds))
			return undefined;

		return RenderGeometrySection.objectOf(objectIds[0])?.path;
	}

	/// Whether a path is closed, meaning it has polygons and every one of them is closed, as the
	/// engine reads a path.
	public static isClosed(path: cool.ObjectPathPolygon[] | undefined): boolean {
		return (
			!!path?.length &&
			path.every((polygon: cool.ObjectPathPolygon) => polygon.closed === true)
		);
	}

	/// The path being edited, or nothing where there is none: the path changed here while the
	/// change has not come back, else the path given.
	public path(): cool.ObjectPathPolygon[] | undefined {
		const given = this.source();
		const changed = PathEditing.changedHere.get(this.key);
		if (!changed) return given;
		if (changed.from === given) return changed.path;

		PathEditing.forget(this.key);
		return given;
	}

	/// The whole path as changed here, while the change has not come back; nothing else.
	public changedOutline(): cool.ObjectPathPolygon[] | null {
		return this.path() === this.source() ? null : (this.path() ?? null);
	}

	private static forget(key: string): void {
		const changed = PathEditing.changedHere.get(key);
		if (!changed) return;
		clearTimeout(changed.wait);
		PathEditing.changedHere.delete(key);
	}

	/*
		Changes the path here as the change will change it, so it shows at once. A change made
		while another one has not come back goes on from that one. Nothing changes where the
		change answers nothing.
	*/
	private changeHere(
		change: (path: cool.ObjectPathPolygon[]) => cool.ObjectPathPolygon[] | null,
	): void {
		const path = this.path();
		if (!path) return;

		const changed = change(
			path.map((polygon: cool.ObjectPathPolygon) => ({
				...polygon,
				points: (polygon.points ?? []).map((point: cool.ObjectPathPoint) => ({
					...point,
				})),
			})),
		);
		if (!changed) return;

		const before = PathEditing.changedHere.get(this.key);
		const from = before ? before.from : this.source();
		PathEditing.forget(this.key);
		const key = this.key;
		const sink = this.sink;
		PathEditing.changedHere.set(key, {
			path: changed,
			from: from,
			wait: setTimeout(() => {
				PathEditing.forget(key);
				sink.shownPathChanged();
			}, PathEditing.longestWait),
		});
		sink.shownPathChanged();
	}

	/*
		The names of the handles of a path. A point is named by the polygon it belongs to and the
		place it has in it; a weight by the point it belongs to and whether it bends the curve
		coming into that point or the one going out. Nothing else speaks that language: what is
		changed is told by the place of the point, not by the name of a handle.
	*/
	private static pointName(polygon: number, point: number): string {
		return '9.' + String(polygon) + '.' + String(point);
	}

	private static weightName(
		polygon: number,
		point: number,
		comingIn: boolean,
	): string {
		return (
			'10.' +
			String(polygon) +
			'.' +
			String(point) +
			(comingIn ? '.in' : '.out')
		);
	}

	/// The place and the part a handle names: 0 the point, 1 the weight coming in, 2 the one
	/// going out. Nothing for a name that names no point of a path.
	private static named(handleName: string): {
		polygon: number;
		point: number;
		part: number;
	} | null {
		const named = handleName.split('.');
		if (named[0] !== '9' && named[0] !== '10') return null;

		const polygon = Number(named[1]);
		const point = Number(named[2]);
		if (!(polygon >= 0) || !(point >= 0)) return null;

		return {
			polygon: polygon,
			point: point,
			part: named[0] === '9' ? 0 : named[3] === 'in' ? 1 : 2,
		};
	}

	/// Whether the handle of that name is a point or a weight of that polygon.
	public static belongsToPolygon(handleName: string, polygon: number): boolean {
		return PathEditing.named(handleName)?.polygon === polygon;
	}

	/// The point that a handle stands for, with its polygon and the whole path. It is null for a
	/// weight, and for a point that the path no longer has.
	public pointOf(handleName: string): {
		polygon: number;
		point: number;
		of: cool.ObjectPathPolygon;
		path: cool.ObjectPathPolygon[];
	} | null {
		const place = PathEditing.named(handleName);
		const path = this.path();
		const of = place && path?.[place.polygon];
		if (!place || place.part !== 0 || !path || !of?.points?.[place.point])
			return null;

		return { polygon: place.polygon, point: place.point, of, path };
	}

	/// The handles of the path: one on every point, and one on every weight of a curve.
	public handles(): SelectionHandle[] {
		const handles: SelectionHandle[] = [];

		(this.path() ?? []).forEach(
			(polygon: cool.ObjectPathPolygon, which: number) => {
				(polygon.points ?? []).forEach(
					(point: cool.ObjectPathPoint, at: number) => {
						handles.push({
							name: PathEditing.pointName(which, at),
							kind: '9',
							pointer: '28',
							point: new cool.Point(point.x, point.y),
						});

						if (point.behindX !== undefined && point.behindY !== undefined)
							handles.push({
								name: PathEditing.weightName(which, at, true),
								kind: '10',
								pointer: '28',
								point: new cool.Point(point.behindX, point.behindY),
							});

						if (point.aheadX !== undefined && point.aheadY !== undefined)
							handles.push({
								name: PathEditing.weightName(which, at, false),
								kind: '10',
								pointer: '28',
								point: new cool.Point(point.aheadX, point.aheadY),
							});
					},
				);
			},
		);

		return handles;
	}

	/*
		Draws the thin line that ties every weight to the point it belongs to. It is drawn twice,
		a dark line and the blue one dashed over it, so that one of the two stands out on any
		background, light or dark, and still does not compete with the handles. The handles it is
		given are the handles as they stand, so while a drag runs the lines follow what it moves.
	*/
	public paint(section: CanvasSectionObject, shown: SelectionHandle[]): void {
		const points = new Map<string, SelectionHandle>();
		const weights: SelectionHandle[] = [];

		for (const handle of shown) {
			if (handle.kind === '9') points.set(handle.name, handle);
			else if (handle.kind === '10') weights.push(handle);
		}
		if (!weights.length) return;

		const lines: { from: cool.Point; to: cool.Point }[] = [];
		for (const weight of weights) {
			const place = PathEditing.named(weight.name);
			const point =
				place && points.get(PathEditing.pointName(place.polygon, place.point));
			if (point) lines.push({ from: point.point, to: weight.point });
		}
		PathEditing.paintWeightLines(section.context, lines);
	}

	/*
		Draws a point of a path that is no selection's, with the weights it has, in twips, as point
		editing shows them: the point and the weights as their handles, in the size configured for
		handles, and the dashed lines between them. Nothing for a point without a weight.
	*/
	public static paintPointWithWeights(
		context: CanvasRenderingContext2D,
		point: cool.ObjectPathPoint,
	): void {
		const weights: cool.Point[] = [];
		if (point.behindX !== undefined && point.behindY !== undefined)
			weights.push(new cool.Point(point.behindX, point.behindY));
		if (point.aheadX !== undefined && point.aheadY !== undefined)
			weights.push(new cool.Point(point.aheadX, point.aheadY));
		if (!weights.length) return;

		const at = new cool.Point(point.x, point.y);
		PathEditing.paintWeightLines(
			context,
			weights.map((weight: cool.Point) => ({ from: at, to: weight })),
		);

		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);
		context.lineWidth = 1;
		const handle = (kind: HandleKind, place: cool.Point) => {
			context.beginPath();
			context.strokeStyle = HandleLook.outlineColour;
			context.fillStyle = HandleLook.lay(
				context,
				kind,
				new cool.SimplePoint(place.x, place.y),
				0,
			);
			context.closePath();
			context.fill();
			context.stroke();
		};
		weights.forEach((weight: cool.Point) => handle('weight', weight));
		handle('pathPoint', at);
		context.restore();
	}

	/// Draws the lines from points to their weights, given in twips, as dashed helper lines.
	public static paintWeightLines(
		context: CanvasRenderingContext2D,
		lines: { from: cool.Point; to: cool.Point }[],
	): void {
		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);
		context.globalAlpha = HandleLook.helperLineAlpha;
		context.lineWidth = app.dpiScale;
		context.beginPath();

		for (const line of lines) {
			const from = new cool.SimplePoint(line.from.x, line.from.y);
			const to = new cool.SimplePoint(line.to.x, line.to.y);
			context.moveTo(from.vX, from.vY);
			context.lineTo(to.vX, to.vY);
		}

		context.strokeStyle = HandleLook.helperLineDarkColour;
		context.stroke();

		const dash = HandleLook.helperLineDash * app.dpiScale;
		context.setLineDash([dash, dash]);
		context.strokeStyle = HandleLook.helperLineColour;
		context.stroke();

		context.restore();
	}

	/*
		Hands the sink where a drag of a point or of a weight ended, or where keys took it, as the
		one change it is.

		The continuity travels because a polygon holds none: whether a point is a corner or is run
		through smoothly lies in its numbers alone, so the two sides could read the same point
		differently. The client draws the curve one way and says which way, and the model is then
		made to hold it exactly that way.

		Only that travels. The path can hold many thousand points, none of the others changes, so
		none of them is sent and none of them can come back a little different from the way it
		went out.
	*/
	public moveTo(handleName: string, at: cool.Point): void {
		const named = PathEditing.named(handleName);
		if (!named) return;

		const point = this.path()?.[named.polygon]?.points?.[named.point];
		if (!point) return;

		this.sink.movePoints([
			{
				polygon: named.polygon,
				point: named.point,
				part: named.part,
				x: Math.round(at.x),
				y: Math.round(at.y),
				continuity: point.continuity ?? 0,
			},
		]);
	}

	/// What the point a handle stands for offers to be done with it, or nothing for a weight and
	/// for a point the path no longer has.
	public factsAt(handleName: string): PathPointFacts | null {
		const place = this.pointOf(handleName);
		if (!place) return null;

		const points = place.of.points;
		const point = points[place.point];
		const next = points[(place.point + 1) % points.length];
		const closed = place.of.closed === true;
		const hasAnEdgeOut = place.point + 1 < points.length || closed;

		return {
			hasAnEdgeOut: hasAnEdgeOut,
			curved:
				hasAnEdgeOut &&
				(point.aheadX !== undefined || next.behindX !== undefined),
			weightIn: point.behindX !== undefined,
			weightOut: point.aheadX !== undefined,
			// a point that reports no choice is a corner
			continuity: point.continuity ?? 0,
			// a path of one polygon can be split at any point of a closed polygon, and at any
			// point of an open polygon except its two ends
			canBeSplit:
				place.path.length === 1 &&
				points.length >= 3 &&
				(closed || (place.point > 0 && place.point + 1 < points.length)),
		};
	}

	/// Deletes the point a handle stands for. Answers whether the handle stands for a point;
	/// for a weight nothing is done.
	public deletePoint(handleName: string): boolean {
		const place = this.pointOf(handleName);
		if (!place) return false;

		this.changeHere((path) => PathEditing.withoutThePoint(path, place));
		this.sink.deletePoint(place);
		return true;
	}

	/// Adds a point on the edge nearest to that place, in twips on the page.
	public insertPointAt(at: cool.Point): void {
		this.changeHere((path) => PathEditing.withAPointAt(path, at));
		this.sink.insertPointAt(at);
	}

	/// Makes the edge after the point a handle stands for a curve, or a line.
	public setSegmentCurved(handleName: string, curved: boolean): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		this.changeHere((path) =>
			PathEditing.withTheEdgeCurved(path, place, curved),
		);
		this.sink.setSegmentCurved(place, curved);
	}

	/// Sets how the curve runs through the point a handle stands for.
	public setContinuity(handleName: string, continuity: number): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		this.changeHere((path) =>
			PathEditing.withTheContinuity(path, place, continuity),
		);
		this.sink.setContinuity(place, continuity);
	}

	/// Splits the path at the point a handle stands for.
	public split(handleName: string): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		this.changeHere((path) => PathEditing.splitAt(path, place));
		this.sink.split(place);
	}

	/// Closes the path, or opens it again where it is closed.
	public switchClosed(): void {
		const path = this.path();
		if (!path) return;
		const closed = !PathEditing.isClosed(path);
		this.changeHere((changed) => PathEditing.closedOrOpened(changed, closed));
		this.sink.setClosed(closed);
	}

	/// Turns every curved edge that runs straight into a line. With pointsOnALine, it also
	/// deletes the points that lie on a straight line between their neighbours.
	public simplify(pointsOnALine: boolean): void {
		this.changeHere((path) => PathEditing.simplified(path, pointsOnALine));
		this.sink.simplify(pointsOnALine);
	}

	/// A closed path can always be opened. An open path can be closed when one of its polygons
	/// has at least three points.
	public canBeClosedOrOpened(): boolean {
		const path = this.path() ?? [];
		return (
			PathEditing.isClosed(path) ||
			path.some(
				(polygon: cool.ObjectPathPolygon) =>
					PathEditing.pointsOnceClosed(polygon) >= 3,
			)
		);
	}

	/// The number of points the polygon has once it is closed. Points at the end that equal the
	/// first point are not counted, because a closed polygon does not repeat its first point.
	private static pointsOnceClosed(polygon: cool.ObjectPathPolygon): number {
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

	// The changes of a path as the engine makes them, on a copy of the path. Each answers the
	// changed path, or nothing where the engine changes more than the path.

	/// Without that point; a polygon left with fewer than two points goes. A path left with no
	/// polygon takes the object with it, which is more than the path.
	private static withoutThePoint(
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
	private static nextOf(
		polygon: cool.ObjectPathPolygon,
		point: number,
	): number | null {
		const count = (polygon.points ?? []).length;
		if (point + 1 < count) return point + 1;
		return polygon.closed && count > 1 ? 0 : null;
	}

	/// The edge after that point made a curve, its new weights on the line at one third and two
	/// thirds of it, or made a line, without weights.
	private static withTheEdgeCurved(
		path: cool.ObjectPathPolygon[],
		place: PathPlace,
		curved: boolean,
	): cool.ObjectPathPolygon[] | null {
		const polygon = path[place.polygon];
		const next = polygon ? PathEditing.nextOf(polygon, place.point) : null;
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
	private static withTheContinuity(
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
		An open polygon becomes two that meet at the point, which the engine makes two objects
		of; until its answer both show here as parts of this path. Only a path of one polygon is
		split, and an open one not at its ends.
	*/
	private static splitAt(
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
		Every curved edge that runs straight made a line: one whose weights lie on the line
		between its ends. With pointsOnALine the points that lie on a straight line between their
		neighbours go too, walking each polygon once as the engine does: a point goes when it and
		every point gone since the last one kept lie on the line from that one to the next. An
		open polygon keeps its ends, a closed one at least three points.
	*/
	private static simplified(
		path: cool.ObjectPathPolygon[],
		pointsOnALine: boolean,
	): cool.ObjectPathPolygon[] | null {
		// the engine allows one of its units, a hundredth of a millimetre, in twips
		const reach = 1440 / 2540;

		for (const polygon of path) {
			const points = polygon.points ?? [];
			points.forEach((point: cool.ObjectPathPoint, at: number) => {
				const next = PathEditing.nextOf(polygon, at);
				if (next === null) return;
				const edge = PathEditing.edgeOf(point, points[next]);
				if (
					edge.curved &&
					PathEditing.onTheLine(edge.start, edge.end, edge.out, reach) &&
					PathEditing.onTheLine(edge.start, edge.end, edge.into, reach)
				) {
					delete point.aheadX;
					delete point.aheadY;
					delete points[next].behindX;
					delete points[next].behindY;
				}
			});

			if (pointsOnALine)
				polygon.points = PathEditing.withoutPointsOnALine(polygon, reach);
		}
		return path;
	}

	/// Whether a point lies within reach of the straight line between two others, and between
	/// them.
	private static onTheLine(
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

	/// The points of a polygon without those on a straight line between their neighbours, by the
	/// engine's walk.
	private static withoutPointsOnALine(
		polygon: cool.ObjectPathPolygon,
		reach: number,
	): cool.ObjectPathPoint[] {
		const points = polygon.points ?? [];
		const count = points.length;
		const closed = polygon.closed === true;
		const least = closed ? 3 : 2;
		if (count <= least) return points;

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
					!PathEditing.onTheLine(
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

		return kept.map((at: number) => points[at]);
	}

	/// Every polygon closed, without points at its end that repeat its first, or every one open.
	private static closedOrOpened(
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
		inside an edge rather than on one of its ends. A curve is split there into two curves,
		so the path keeps its shape and the new point runs smoothly through.
	*/
	private static withAPointAt(
		path: cool.ObjectPathPolygon[],
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		let best: {
			polygon: number;
			point: number;
			cut: number;
			distance: number;
		} | null = null;

		path.forEach((polygon: cool.ObjectPathPolygon, which: number) => {
			(polygon.points ?? []).forEach((_point, point: number) => {
				const next = PathEditing.nextOf(polygon, point);
				if (next === null) return;
				const edge = PathEditing.edgeOf(
					polygon.points[point],
					polygon.points[next],
				);
				const nearest = PathEditing.nearestOnEdge(edge, at);
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
		const next = PathEditing.nextOf(path[found.polygon], found.point) as number;
		const from = points[found.point];
		const to = points[next];
		const edge = PathEditing.edgeOf(from, to);
		const t = found.cut;
		const between = (a: cool.Point, b: cool.Point) =>
			a.add(b.subtract(a).multiplyBy(t));

		let added: cool.ObjectPathPoint;
		if (edge.curved) {
			const a = between(edge.start, edge.out);
			const b = between(edge.out, edge.into);
			const c = between(edge.into, edge.end);
			const ab = between(a, b);
			const bc = between(b, c);
			const middle = between(ab, bc);
			from.aheadX = a.x;
			from.aheadY = a.y;
			to.behindX = c.x;
			to.behindY = c.y;
			added = {
				x: middle.x,
				y: middle.y,
				behindX: ab.x,
				behindY: ab.y,
				aheadX: bc.x,
				aheadY: bc.y,
				continuity: 1,
			};
		} else {
			const middle = between(edge.start, edge.end);
			added = { x: middle.x, y: middle.y };
		}

		points.splice(found.point + 1, 0, added);
		return path;
	}

	/// An edge from one point to the next: its ends, its weights where it is curved, else its
	/// ends again.
	private static edgeOf(
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
	private static nearestOnEdge(
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
	private static withTheWeightMoved(
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

	/*
		The point a drag has hold of, as it stands while the drag runs: a point takes its weights
		along with it, a weight goes where the drag has it and lets the other one of its point
		swing as far as the curve there asks. Nothing where the name belongs to no point of the
		path.
	*/
	private movedPoint(
		handleName: string,
		at: cool.Point,
	): { polygon: number; at: number; point: cool.ObjectPathPoint } | null {
		const named = PathEditing.named(handleName);
		if (!named) return null;

		const points = this.path()?.[named.polygon]?.points ?? [];
		if (named.point >= points.length) return null;

		const stands = points[named.point];

		if (named.part !== 0)
			return {
				polygon: named.polygon,
				at: named.point,
				point: PathEditing.withTheWeightMoved(stands, named.part === 1, at),
			};

		const across = at.x - stands.x;
		const down = at.y - stands.y;
		const carried = (one: number | undefined, by: number) =>
			one === undefined ? undefined : one + by;

		return {
			polygon: named.polygon,
			at: named.point,
			point: {
				...stands,
				x: at.x,
				y: at.y,
				behindX: carried(stands.behindX, across),
				behindY: carried(stands.behindY, down),
				aheadX: carried(stands.aheadX, across),
				aheadY: carried(stands.aheadY, down),
			},
		};
	}

	/*
		The handles as they stand while a drag of a point or of a weight runs, from the handles as
		they stand when nothing runs. The point being dragged and the two weights of that point are
		shown where the drag leaves them, so a weight that swings because the curve runs smoothly
		through its point is seen swinging. Everything else is handed back untouched, the eight
		that frame the object among it. Nothing where the drag has hold of no point of the path.
	*/
	public handlesDuring(
		handleName: string,
		at: cool.Point,
		known: SelectionHandle[],
	): SelectionHandle[] | null {
		const moved = this.movedPoint(handleName, at);
		if (!moved) return null;

		const point = moved.point;
		const where: { [name: string]: cool.Point | undefined } = {
			[PathEditing.pointName(moved.polygon, moved.at)]: new cool.Point(
				point.x,
				point.y,
			),
			[PathEditing.weightName(moved.polygon, moved.at, true)]:
				point.behindX === undefined || point.behindY === undefined
					? undefined
					: new cool.Point(point.behindX, point.behindY),
			[PathEditing.weightName(moved.polygon, moved.at, false)]:
				point.aheadX === undefined || point.aheadY === undefined
					? undefined
					: new cool.Point(point.aheadX, point.aheadY),
		};

		return known.map((handle: SelectionHandle) => {
			const placed = where[handle.name];
			return placed ? { ...handle, point: placed } : handle;
		});
	}

	/// The whole path with the point or the weight a handle names where a drag leaves it, as the
	/// handles show it while the drag runs; nothing where the name belongs to no point of the path.
	public pathFor(
		handleName: string,
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const moved = this.movedPoint(handleName, at);
		const path = this.path();
		if (!moved || !path) return null;

		return path.map((polygon: cool.ObjectPathPolygon, which: number) =>
			which !== moved.polygon
				? polygon
				: {
						...polygon,
						points: (polygon.points ?? []).map(
							(point: cool.ObjectPathPoint, index: number) =>
								index === moved.at ? moved.point : point,
						),
					},
		);
	}

	/*
		The polygon whose line runs nearest to that place, in twips, where it runs no farther away
		than reach; nothing where no line comes that near. The line of a closed polygon runs back
		to its first point too.
	*/
	public polygonAt(at: cool.Point, reach: number): number | null {
		let found: { polygon: number; distance: number } | null = null;
		const path = this.path() ?? [];

		for (let which = 0; which < path.length; ++which) {
			const points = path[which].points ?? [];
			for (let point = 0; point < points.length; ++point) {
				const next = PathEditing.nextOf(path[which], point);
				if (next === null) continue;
				const near = PathEditing.nearestOnEdge(
					PathEditing.edgeOf(points[point], points[next]),
					at,
				);
				if (
					near.distance <= reach &&
					(!found || near.distance < found.distance)
				)
					found = { polygon: which, distance: near.distance };
			}
		}

		return found ? found.polygon : null;
	}

	/// The whole path with that polygon moved by that offset, its points and their weights
	/// together; nothing where the path has no such polygon.
	public pathWithPolygonMoved(
		polygon: number,
		by: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const path = this.path();
		if (!path?.[polygon]) return null;

		return path.map((one: cool.ObjectPathPolygon, which: number) =>
			which !== polygon
				? one
				: {
						...one,
						points: (one.points ?? []).map((point: cool.ObjectPathPoint) =>
							cool.MotionPaths.shifted(point, by),
						),
					},
		);
	}

	/*
		Moves that polygon by that offset, as one change: every point of it goes to the sink, and
		the weights go along with their points. The path shows the move at once.
	*/
	public movePolygon(polygon: number, by: cool.Point): void {
		const moved = this.pathWithPolygonMoved(polygon, by);
		if (!moved) return;

		this.changeHere(() => moved);
		this.sink.movePoints(
			moved[polygon].points.map((point: cool.ObjectPathPoint, at: number) => ({
				polygon: polygon,
				point: at,
				part: 0,
				x: Math.round(point.x),
				y: Math.round(point.y),
				continuity: point.continuity ?? 0,
			})),
		);
	}

	/*
		The pieces of line that meet at the point a drag has hold of, with what it moved where the
		drag has it: the one that comes to the point and the one that leaves it. The rest of the
		path stays where it is, so drawing it again would say nothing.

		A drag of a weight shows the same two pieces, since a weight bends the curve on one side
		of its point and the eye needs the point and its neighbours to read the bend.
	*/
	public outlineFor(
		handleName: string,
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const moved = this.movedPoint(handleName, at);
		if (!moved) return null;

		const polygon = this.path()?.[moved.polygon];
		const points = polygon?.points ?? [];
		const closed = polygon?.closed === true;
		const dragged = moved.at;

		const before =
			dragged > 0 ? dragged - 1 : closed ? points.length - 1 : undefined;
		const after =
			dragged < points.length - 1 ? dragged + 1 : closed ? 0 : undefined;

		const drawn: cool.ObjectPathPoint[] = [];
		if (before !== undefined) drawn.push(points[before]);
		drawn.push(moved.point);
		if (after !== undefined) drawn.push(points[after]);

		return drawn.length > 1 ? [{ closed: false, points: drawn }] : null;
	}
}

/*
	The changes of the path of an object, sent to the engine. A move of points travels at once.
	Every other change names points by their index in the path the client has, so it is not sent
	while an undo or a redo is on its way, and point moves that still wait are sent first. Such a
	change can add or remove points and weights, so the keyboard leaves the points.
*/
class ObjectPathSink implements PathSink {
	private objectId: number;
	private selection: SelectionSection;

	constructor(objectId: number, selection: SelectionSection) {
		this.objectId = objectId;
		this.selection = selection;
	}

	/*
		The object the points belong to and one item for every point or weight that moved. The
		points that did not move are not named at all, so a path of many thousand points still
		travels as the few numbers that changed, and the ones left alone come back exactly as they
		went out.
	*/
	public movePoints(moves: PathPointMove[]): void {
		if (!moves.length) return;

		app.socket.sendMessage(
			'setobjectpoints id=' +
				String(this.objectId) +
				' changes=' +
				moves
					.map((move: PathPointMove) =>
						[
							move.polygon,
							move.point,
							move.part,
							move.x,
							move.y,
							move.continuity,
						].join(','),
					)
					.join(';'),
		);
	}

	public deletePoint(place: PathPlace): void {
		this.send('deleteobjectpoint', ObjectPathSink.parametersOf(place));
	}

	public insertPointAt(at: cool.Point): void {
		this.send(
			'insertobjectpoint',
			'x=' + String(Math.round(at.x)) + ' y=' + String(Math.round(at.y)),
		);
	}

	public setSegmentCurved(place: PathPlace, curved: boolean): void {
		this.send(
			'setobjectsegmentcurved',
			ObjectPathSink.parametersOf(place) + ' curved=' + String(curved),
		);
	}

	public setContinuity(place: PathPlace, continuity: number): void {
		this.send(
			'setobjectpointcontinuity',
			ObjectPathSink.parametersOf(place) + ' continuity=' + String(continuity),
		);
	}

	public split(place: PathPlace): void {
		this.send('splitobjectpath', ObjectPathSink.parametersOf(place));
	}

	public setClosed(closed: boolean): void {
		this.send('setobjectpathclosed', 'closed=' + String(closed));
	}

	public simplify(pointsOnALine: boolean): void {
		this.send('simplifyobjectpath', 'points=' + String(pointsOnALine));
	}

	/// The handles stand on the path as shown, so they are worked out again.
	public shownPathChanged(): void {
		this.selection.refresh();
		app.sectionContainer?.requestReDraw();
	}

	/// The parameters that name a point of the path to the engine.
	private static parametersOf(place: PathPlace): string {
		return 'polygon=' + String(place.polygon) + ' point=' + String(place.point);
	}

	private send(command: string, parameters: string): void {
		if (ChangeOutbox.undoOnItsWay()) {
			this.selection.showTheHoldBack();
			return;
		}

		this.selection.travel.leave();
		ChangeOutbox.sendEverythingAhead();
		app.socket.sendMessage(
			command + ' id=' + String(this.objectId) + ' ' + parameters,
		);
	}
}
