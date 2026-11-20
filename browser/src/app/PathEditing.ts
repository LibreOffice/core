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
	A point added to a path: the place it takes, where it lies, in whole twips, and the weights
	that change with it - its own two and those of its neighbours that bend the edge it is added
	on - named by their places in the path that has the new point.
*/
interface PathPointAddition extends PathPlace {
	x: number;
	y: number;
	weights: PathPointMove[];
}

/// A weight of a point removed: 1 the weight of the curve coming in, 2 the one going out.
interface PathWeightRemoval extends PathPlace {
	part: number;
}

/*
	What simplifying a path removes, as one step: the weights of the curved edges that run
	straight, named by their places in the path as it is, and then the points that lie on a
	straight line between their neighbours. Each point is named by its place in the path without
	the points before it.
*/
interface PathSimplification {
	weights: PathWeightRemoval[];
	points: PathPlace[];
}

/*
	The weights of a path changed by one step: the weights that moved, those that were removed,
	and the name of the step in the undo list.
*/
interface PathWeightsChange {
	moves: PathPointMove[];
	removals: PathWeightRemoval[];
	step: 'continuity' | 'curve' | 'line';
}

/*
	Where the changes of a path go. The path being edited can belong to an object, be drawn
	anew, or be a lasso or the path an animation follows; each of them takes the changes its own
	way.
*/
interface PathSink {
	/// True where the points and the weights the sink takes are in whole twips, false where
	/// they can lie at any fraction of a twip.
	readonly wholeTwips: boolean;
	/// Points or weights moved, by a drag or by keys.
	movePoints(moves: PathPointMove[]): void;
	/// Points are deleted, as one step, in the order given. Each is named by its place in the
	/// path without the points before it.
	deletePoints(places: PathPlace[]): void;
	/// The object the path belongs to is deleted.
	deleteObject(): void;
	/// A point is added, with the weights that change with it.
	addPoint(addition: PathPointAddition): void;
	/// Weights moved or were removed, as one step: the edge after a point became a curve or a
	/// line, or the curve runs through a point another way.
	changeWeights(change: PathWeightsChange): void;
	/// The open path is split into two objects at a point. The second object has the path
	/// after, the part after the point.
	split(place: PathPlace, after: cool.ObjectPathPolygon[]): void;
	/// The path shown changed here before the change came back, or went back to the one given;
	/// what shows it is to be drawn again.
	shownPathChanged(): void;
	/// The whole path as a change made here leaves it, where it is edited, handed over before
	/// that change itself.
	pathChangedHere(path: cool.ObjectPathPolygon[]): void;
	/// The path is closed, or opened. A closed path opened at a point has that point at both
	/// of its ends.
	setClosed(closed: boolean, at?: PathPlace): void;
	/// Weights and points are removed, as one step, in the order given.
	simplify(simplification: PathSimplification): void;
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
	is when the change came back. How a change makes the new path is told by PathGeometry.
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
	public static readonly longestWait: number = 5000;

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
		Changes the path here as the change will change it, so it shows at once, and answers the
		changed path. A change made while another one has not come back goes on from that one.
		Nothing changes where the change answers nothing.
	*/
	private changeHere(
		change: (path: cool.ObjectPathPolygon[]) => cool.ObjectPathPolygon[] | null,
	): cool.ObjectPathPolygon[] | null {
		const path = this.path();
		if (!path) return null;

		const changed = change(
			path.map((polygon: cool.ObjectPathPolygon) => ({
				...polygon,
				points: (polygon.points ?? []).map((point: cool.ObjectPathPoint) => ({
					...point,
				})),
			})),
		);
		if (!changed) return null;

		this.sink.pathChangedHere(changed);
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
		return changed;
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
	public paint(
		section: CanvasSectionObject,
		shown: SelectionHandle[],
		colour: string = HandleLook.colorOfTheKind.pathPoint,
	): void {
		const points = new Map<string, SelectionHandle>();
		const weights: SelectionHandle[] = [];

		for (const handle of shown) {
			if (handle.kind === '9') points.set(handle.name, handle);
			else if (handle.kind === '10') weights.push(handle);
		}
		this.paintPartNumbers(section.context, points, colour);
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
		Writes the number of each polygon beside the point it starts at, from 1, so the order of
		the polygons can be seen; a path of one polygon gets none. The number stands off the
		point away from the line, in the colour given with a white edge, and goes along with the
		point while it is dragged, as it is placed from the points as they are shown.
	*/
	private paintPartNumbers(
		context: CanvasRenderingContext2D,
		points: Map<string, SelectionHandle>,
		colour: string,
	): void {
		const path = this.path() ?? [];
		if (path.length < 2) return;

		const size = Math.round(11 * app.dpiScale);
		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);
		context.font = 'bold ' + String(size) + 'px sans-serif';
		context.textAlign = 'center';
		context.textBaseline = 'middle';
		context.lineJoin = 'round';
		context.lineWidth = 3 * app.dpiScale;
		context.strokeStyle = '#FFFFFF';
		context.fillStyle = colour;

		path.forEach((polygon: cool.ObjectPathPolygon, which: number) => {
			const at = PathEditing.partNumberAt(polygon, which, points);
			if (!at) return;
			const written = new cool.SimplePoint(at.x, at.y);
			const away = at.away.multiplyBy(
				ShapeHandlesSection.handleSize() + size / 2,
			);
			const text = String(which + 1);
			context.strokeText(text, written.vX + away.x, written.vY + away.y);
			context.fillText(text, written.vX + away.x, written.vY + away.y);
		});

		context.restore();
	}

	/*
		Where the number of a polygon goes: its first point as shown, in twips, and the way away
		from the line there, as a vector of length one on the screen. That way points against
		the edges that meet at the point, the one going out and, for a closed polygon, the one
		coming in, so the number does not sit on either. Nothing where the point is not shown.
	*/
	private static partNumberAt(
		polygon: cool.ObjectPathPolygon,
		which: number,
		points: Map<string, SelectionHandle>,
	): { x: number; y: number; away: cool.Point } | null {
		const count = (polygon.points ?? []).length;
		const start = points.get(PathEditing.pointName(which, 0))?.point;
		if (!start || count < 2) return null;

		// the way along an edge from the start, by the point at its other end as shown
		const towards = (point: number): cool.Point | null => {
			const other = points.get(PathEditing.pointName(which, point))?.point;
			const along = other?.subtract(start);
			const length = along?.length() ?? 0;
			return along && length ? along.divideBy(length) : null;
		};
		const out = towards(1);
		const back = polygon.closed ? towards(count - 1) : null;

		let away = new cool.Point(0, 0);
		if (out) away = away.subtract(out);
		if (back) away = away.subtract(back);
		// edges going out and back in on one line leave no way away from both, so the number
		// goes to the side of the edge going out
		if (away.length() < 1e-6)
			away = out ? new cool.Point(out.y, -out.x) : new cool.Point(0, -1);

		return { x: start.x, y: start.y, away: away.divideBy(away.length()) };
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

	/*
		Deletes the point a handle stands for. A polygon has two points at least, so where one
		point would be left, that point is deleted too. A path that would be left with one point
		or none deletes its object. Answers whether the handle stands for a point; for a weight
		nothing is done.
	*/
	public deletePoint(handleName: string): boolean {
		const place = this.pointOf(handleName);
		if (!place) return false;

		const count = place.of.points.length;
		if (place.path.length === 1 && count <= 2) {
			this.sink.deleteObject();
			return true;
		}

		const places: PathPlace[] = [
			{ polygon: place.polygon, point: place.point },
		];
		// the point left is the first one once the other is gone
		if (count === 2) places.push({ polygon: place.polygon, point: 0 });

		this.changeHere((path) => PathGeometry.withoutThePoint(path, place));
		this.sink.deletePoints(places);
		return true;
	}

	/// Adds a point on the edge nearest to that place, in twips on the page.
	public insertPointAt(at: cool.Point): void {
		const made: { addition?: PathPointAddition } = {};
		this.changeHere((path) => {
			const added = PathGeometry.withAPointAt(path, at, this.sink.wholeTwips);
			made.addition = added?.addition;
			return added?.path ?? null;
		});
		if (made.addition) this.sink.addPoint(made.addition);
	}

	/// Makes the edge after the point a handle stands for a curve, or a line.
	public setSegmentCurved(handleName: string, curved: boolean): void {
		const place = this.pointOf(handleName);
		const polygon = place ? this.path()?.[place.polygon] : undefined;
		const next =
			place && polygon ? PathGeometry.nextOf(polygon, place.point) : null;
		if (place && next !== null) {
			// The weights a new curve gets leave the other weights of their points as they are.
			this.changeWeightsHere(
				place.polygon,
				[place.point, next],
				curved ? 'curve' : 'line',
				0,
				(path) => PathGeometry.withTheEdgeCurved(path, place, curved),
			);
		}
	}

	/// Sets how the curve runs through the point a handle stands for.
	public setContinuity(handleName: string, continuity: number): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		this.changeWeightsHere(
			place.polygon,
			[place.point],
			'continuity',
			continuity,
			(path) => PathGeometry.withTheContinuity(path, place, continuity),
		);
	}

	/*
		Changes the weights of the points that a step touches, in one polygon, here, and sends
		the weights of those points that differ afterwards, every move with that continuity.
		Where the sink takes whole twips, a weight the step moved is rounded here, so the path
		shown is close to the one the model will hold. Nothing is sent when no weight differs.
	*/
	private changeWeightsHere(
		polygon: number,
		touched: number[],
		step: PathWeightsChange['step'],
		continuity: number,
		change: (path: cool.ObjectPathPolygon[]) => cool.ObjectPathPolygon[] | null,
	): void {
		const before = this.path()?.[polygon]?.points ?? [];
		const changed = this.changeHere((path) => {
			const result = change(path);
			const after = result?.[polygon]?.points ?? [];
			if (this.sink.wholeTwips)
				for (const at of touched)
					PathGeometry.roundWhatMoved(before[at], after[at]);
			return result;
		});
		if (!changed) return;

		const found = PathGeometry.weightsChanged(
			polygon,
			touched,
			before,
			changed[polygon]?.points ?? [],
			step,
			continuity,
		);
		if (found.moves.length || found.removals.length)
			this.sink.changeWeights(found);
	}

	/// Splits the path at the point a handle stands for. A closed path is opened there, and an
	/// open one becomes two objects.
	public split(handleName: string): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		const at = { polygon: place.polygon, point: place.point };
		if (place.of.closed === true) {
			if (this.changeHere((path) => PathGeometry.splitAt(path, place)))
				this.sink.setClosed(false, at);
			return;
		}

		// This path keeps the part before the point, and the second object has the rest.
		let after = undefined as cool.ObjectPathPolygon | undefined;
		const before = this.changeHere((path) => {
			const pieces = PathGeometry.splitAt(path, place);
			after = pieces?.[1];
			return pieces ? pieces.slice(0, 1) : null;
		});
		if (before && after) this.sink.split(at, [after]);
	}

	/// Closes the path, or opens it again where it is closed.
	public switchClosed(): void {
		const path = this.path();
		if (!path) return;
		const closed = !PathGeometry.isClosed(path);
		this.changeHere((changed) => PathGeometry.closedOrOpened(changed, closed));
		this.sink.setClosed(closed);
	}

	/// Turns every curved edge that runs straight into a line. With pointsOnALine, it also
	/// deletes the points that lie on a straight line between their neighbours.
	public simplify(pointsOnALine: boolean): void {
		const path = this.path();
		if (!path) return;

		const simplification = PathGeometry.simplification(path, pointsOnALine);
		if (!simplification.weights.length && !simplification.points.length) return;

		this.changeHere((changed) =>
			PathGeometry.withTheSimplification(changed, simplification),
		);
		this.sink.simplify(simplification);
	}

	/// A closed path can always be opened. An open path can be closed when one of its polygons
	/// has at least three points.
	public canBeClosedOrOpened(): boolean {
		const path = this.path() ?? [];
		return (
			PathGeometry.isClosed(path) ||
			path.some(
				(polygon: cool.ObjectPathPolygon) =>
					PathGeometry.pointsOnceClosed(polygon) >= 3,
			)
		);
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
				point: PathGeometry.withTheWeightMoved(stands, named.part === 1, at),
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
				const next = PathGeometry.nextOf(path[which], point);
				if (next === null) continue;
				const near = PathGeometry.nearestOnEdge(
					PathGeometry.edgeOf(points[point], points[next]),
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
							PathGeometry.shifted(point, by),
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
