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

		this.changeHere((path) => PathEditing.withoutThePoint(path, place));
		this.sink.deletePoints(places);
		return true;
	}

	/// Adds a point on the edge nearest to that place, in twips on the page.
	public insertPointAt(at: cool.Point): void {
		const made: { addition?: PathPointAddition } = {};
		this.changeHere((path) => {
			const added = PathEditing.withAPointAt(path, at, this.sink.wholeTwips);
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
			place && polygon ? PathEditing.nextOf(polygon, place.point) : null;
		if (place && next !== null) {
			// The weights a new curve gets leave the other weights of their points as they are.
			this.changeWeightsHere(
				place.polygon,
				[place.point, next],
				curved ? 'curve' : 'line',
				0,
				(path) => PathEditing.withTheEdgeCurved(path, place, curved),
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
			(path) => PathEditing.withTheContinuity(path, place, continuity),
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
					PathEditing.roundWhatMoved(before[at], after[at]);
			return result;
		});
		if (!changed) return;

		const found = PathEditing.weightsChanged(
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

	/// The weights of the point that are not where they were, at whole twips.
	private static roundWhatMoved(
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
	private static weightsChanged(
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

	/// Splits the path at the point a handle stands for. A closed path is opened there, and an
	/// open one becomes two objects.
	public split(handleName: string): void {
		const place = this.pointOf(handleName);
		if (!place) return;
		const at = { polygon: place.polygon, point: place.point };
		if (place.of.closed === true) {
			if (this.changeHere((path) => PathEditing.splitAt(path, place)))
				this.sink.setClosed(false, at);
			return;
		}

		// This path keeps the part before the point, and the second object has the rest.
		let after = undefined as cool.ObjectPathPolygon | undefined;
		const before = this.changeHere((path) => {
			const pieces = PathEditing.splitAt(path, place);
			after = pieces?.[1];
			return pieces ? pieces.slice(0, 1) : null;
		});
		if (before && after) this.sink.split(at, [after]);
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
		const path = this.path();
		if (!path) return;

		const simplification = PathEditing.simplification(path, pointsOnALine);
		if (!simplification.weights.length && !simplification.points.length) return;

		this.changeHere((changed) =>
			PathEditing.withTheSimplification(changed, simplification),
		);
		this.sink.simplify(simplification);
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

	/// Without that point; a polygon left with fewer than two points goes. Nothing where no
	/// polygon is left, since the path keeps one.
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
		An open polygon becomes two that meet at the point, the part before it and the part
		after it. Only a path of one polygon is split, and an open one not at its ends.
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
		What simplifying the path removes. A curved edge runs straight when its weights lie on
		the line between its ends, and its weights go. With pointsOnALine the points that lie on a
		straight line between their neighbours go too, from the last to the first in each
		polygon, so the place of each one stays as it is until it goes.
	*/
	private static simplification(
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
				const next = PathEditing.nextOf(flattened, at);
				if (next === null) return;
				const edge = PathEditing.edgeOf(point, points[next]);
				if (
					!edge.curved ||
					!PathEditing.onTheLine(edge.start, edge.end, edge.out, reach) ||
					!PathEditing.onTheLine(edge.start, edge.end, edge.into, reach)
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
			const kept = new Set(PathEditing.keptOffALine(flattened, reach));
			for (let at = points.length - 1; at >= 0; --at)
				if (!kept.has(at)) simplification.points.push({ polygon, point: at });
		});
		return simplification;
	}

	/// The path without the weights and the points that the simplification removes.
	private static withTheSimplification(
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
			if (!PathEditing.withoutThePoint(path, place)) return null;
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

	/*
		The places of the points of a polygon that do not lie on a straight line between their
		neighbours. An open polygon keeps its ends, and a closed one at least three points.

		It walks the polygon once. A point goes when it, and every point gone since the last one
		kept, lie on the line from that one to the next point. So each line left stays close to
		all the points it takes the place of. A closed polygon keeps its first point during the
		walk and looks at it at the end, against the points kept on each side of it.
	*/
	private static keptOffALine(
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

		return kept;
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
		inside an edge rather than on one of its ends, and the addition that names it. A curve is
		split there into two curves, so the path keeps its shape and the new point runs smoothly
		through. With wholeTwips, the new point and the weights are rounded to whole twips, so the
		path shown is close to the one the model will hold, and a point that lands on an end of
		the edge once rounded adds nothing.
	*/
	private static withAPointAt(
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
	public readonly wholeTwips: boolean = true;
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
				ObjectPathSink.itemsOf(moves),
		);
	}

	/// Each point is "-polygon,point,0", where 0 names the point itself.
	public deletePoints(places: PathPlace[]): void {
		const items = places.map(
			(place: PathPlace) => '-' + [place.polygon, place.point, 0].join(','),
		);
		this.send('setobjectpoints', 'changes=' + items.join(';') + ' as=delete');
	}

	/// The object is the one selected, so it goes the way the selected objects are deleted.
	public deleteObject(): void {
		if (this.readyToSend()) GraphicSelection.deleteTheObjectsHere();
	}

	/*
		The new point goes first and the weights after it, which name their points by the places
		they have in the path with the new point. The points that do not change are not named, as
		for a move.
	*/
	public addPoint(addition: PathPointAddition): void {
		const point = [
			addition.polygon,
			addition.point,
			addition.x,
			addition.y,
		].join(',');
		const weights = addition.weights.length
			? ';' + ObjectPathSink.itemsOf(addition.weights)
			: '';
		this.send('setobjectpoints', 'changes=+' + point + weights + ' as=add');
	}

	/// The moves go first and the removals after them, so a weight that is removed stays
	/// removed even where a move makes the curve run smoothly through its point.
	public changeWeights(change: PathWeightsChange): void {
		const items = change.removals.map(
			(removal: PathWeightRemoval) =>
				'-' + [removal.polygon, removal.point, removal.part].join(','),
		);
		if (change.moves.length)
			items.unshift(ObjectPathSink.itemsOf(change.moves));
		this.send(
			'setobjectpoints',
			'changes=' + items.join(';') + ' as=' + change.step,
		);
	}

	/*
		The second object takes an id reserved for this view, where one is at hand. It then shows
		at once as its own object, drawn as its path in hairlines, and it is selected, so it can
		be moved away. The engine's report of it replaces it. Without a reserved id the second
		object shows when the engine reports it.
	*/
	public split(place: PathPlace, after: cool.ObjectPathPolygon[]): void {
		const newId = ReservedObjectIds.take();
		const sent = this.send(
			'splitobjectpath',
			ObjectPathSink.parametersOf(place) +
				(newId === undefined ? '' : ' newid=' + String(newId)),
		);
		if (sent && newId !== undefined) this.standInAsSplitOff(newId, after);
	}

	/// The second object of a split stands as created, under its id and selected, in the place
	/// of the first object among its group and layer.
	private standInAsSplitOff(
		newId: number,
		path: cool.ObjectPathPolygon[],
	): void {
		const part = RenderGeometrySection.currentPart();
		const first = RenderGeometrySection.objectOf(this.objectId);
		if (!part) return;

		const box = PathDrawing.rangeOf(path);
		const created: cool.SlideObject = {
			id: newId,
			parent: first?.parent,
			layer: first?.layer,
			objectKind: first?.objectKind,
			x: box.minX,
			y: box.minY,
			width: box.width,
			height: box.height,
			path: path,
			primitives: [
				{
					type: cool.PolygonHairlinePrimitive.type,
					color: HandleLook.outlineColour,
					path: PathDrawing.asSvg(path),
				} as cool.PolygonHairlinePrimitive,
			],
		};
		if (
			RenderManager.addProvisionalObject(
				part.partId,
				created,
				PathEditing.longestWait,
			)
		)
			GraphicSelection.selectAndShow([newId]);
	}

	/// Closing or opening names a state, so the outbox takes it. Opening at a point moves the
	/// points around, so the changes before it go first.
	public setClosed(closed: boolean, at?: PathPlace): void {
		this.send(
			'setobjectpathclosed',
			'closed=' +
				String(closed) +
				(at ? ' at=' + String(at.polygon) + ',' + String(at.point) : ''),
			at !== undefined,
		);
	}

	/// The weights go first, named by their places in the path as it is, and the points after
	/// them.
	public simplify(simplification: PathSimplification): void {
		const items = simplification.weights
			.map(
				(weight: PathWeightRemoval) =>
					'-' + [weight.polygon, weight.point, weight.part].join(','),
			)
			.concat(
				simplification.points.map(
					(place: PathPlace) => '-' + [place.polygon, place.point, 0].join(','),
				),
			);
		this.send('setobjectpoints', 'changes=' + items.join(';') + ' as=simplify');
	}

	public pathChangedHere(_path: cool.ObjectPathPolygon[]): void {
		// Every change goes to the engine by itself, as the change it is.
	}

	/// The handles stand on the path as shown, so they are worked out again.
	public shownPathChanged(): void {
		this.selection.refresh();
		app.sectionContainer?.requestReDraw();
	}

	/// The moves as the items of a setobjectpoints message.
	private static itemsOf(moves: PathPointMove[]): string {
		return moves
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
			.join(';');
	}

	/// The parameters that name a point of the path to the engine.
	private static parametersOf(place: PathPlace): string {
		return 'polygon=' + String(place.polygon) + ' point=' + String(place.point);
	}

	/*
		Answers whether a change can go to the engine now. It cannot while an undo is on its way,
		and the hold back is shown then. Otherwise the keyboard leaves the handles, and with
		sendAhead the changes waiting in the outbox go first. A change that names a state goes
		into the outbox behind them, which keeps the order and lets a later state of the same
		thing take its place.
	*/
	private readyToSend(sendAhead: boolean = true): boolean {
		if (ChangeOutbox.undoOnItsWay()) {
			this.selection.showTheHoldBack();
			return false;
		}

		this.selection.travel.leave();
		if (sendAhead) ChangeOutbox.sendEverythingAhead();
		return true;
	}

	/// Answers whether the message went out.
	private send(
		command: string,
		parameters: string,
		sendAhead: boolean = true,
	): boolean {
		if (!this.readyToSend(sendAhead)) return false;

		app.socket.sendMessage(
			command + ' id=' + String(this.objectId) + ' ' + parameters,
		);
		return true;
	}
}
