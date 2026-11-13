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
*/
class PathEditing implements HandleSource {
	private source: () => cool.ObjectPathPolygon[] | undefined;
	private sink: PathSink;

	constructor(
		source: () => cool.ObjectPathPolygon[] | undefined,
		sink: PathSink,
	) {
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

	/// The path being edited, or nothing where there is none.
	public path(): cool.ObjectPathPolygon[] | undefined {
		return this.source();
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

		section.context.save();
		section.context.setTransform(1, 0, 0, 1, 0, 0);
		section.context.globalAlpha = HandleLook.helperLineAlpha;
		section.context.lineWidth = app.dpiScale;
		section.context.beginPath();

		for (const weight of weights) {
			const place = PathEditing.named(weight.name);
			const point =
				place && points.get(PathEditing.pointName(place.polygon, place.point));
			if (!point) continue;

			const from = new cool.SimplePoint(point.point.x, point.point.y);
			const to = new cool.SimplePoint(weight.point.x, weight.point.y);
			section.context.moveTo(from.vX, from.vY);
			section.context.lineTo(to.vX, to.vY);
		}

		section.context.strokeStyle = HandleLook.helperLineDarkColour;
		section.context.stroke();

		const dash = HandleLook.helperLineDash * app.dpiScale;
		section.context.setLineDash([dash, dash]);
		section.context.strokeStyle = HandleLook.helperLineColour;
		section.context.stroke();

		section.context.restore();
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

		this.sink.deletePoint(place);
		return true;
	}

	/// Adds a point on the edge nearest to that place, in twips on the page.
	public insertPointAt(at: cool.Point): void {
		this.sink.insertPointAt(at);
	}

	/// Makes the edge after the point a handle stands for a curve, or a line.
	public setSegmentCurved(handleName: string, curved: boolean): void {
		const place = this.pointOf(handleName);
		if (place) this.sink.setSegmentCurved(place, curved);
	}

	/// Sets how the curve runs through the point a handle stands for.
	public setContinuity(handleName: string, continuity: number): void {
		const place = this.pointOf(handleName);
		if (place) this.sink.setContinuity(place, continuity);
	}

	/// Splits the path at the point a handle stands for.
	public split(handleName: string): void {
		const place = this.pointOf(handleName);
		if (place) this.sink.split(place);
	}

	/// Closes the path, or opens it again where it is closed.
	public switchClosed(): void {
		const path = this.path();
		if (path) this.sink.setClosed(!PathEditing.isClosed(path));
	}

	/// Turns every curved edge that runs straight into a line. With pointsOnALine, it also
	/// deletes the points that lie on a straight line between their neighbours.
	public simplify(pointsOnALine: boolean): void {
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
