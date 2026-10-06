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

/*
	Editing the path of an object: the points it is drawn along, the weights that bend the curves
	between them, the thin lines that tie a weight to its point, and the piece of line a drag of
	one of them would leave behind.

	It knows nothing of what holds it, so the same thing serves a polygon, a line, a connector
	and, later, the path an animation follows.
*/
class PolyPolygonEditor implements HandleSource {
	private objectId: number;

	constructor(objectId: number) {
		this.objectId = objectId;
	}

	/// How wide a point of the path is drawn, in core pixels. Smaller than a handle, and drawn
	/// over them, so that a point lying on one of the eight is still the one a press reaches.
	public static pointSize(): number {
		return HandleLook.widthOf('pathPoint');
	}

	/// How wide the weight of a curve is drawn, in core pixels. Smaller again: there are two of
	/// them for every point of a curve, and they should not shout.
	public static weightSize(): number {
		return HandleLook.widthOf('weight');
	}

	/// The path of the object, or nothing where it holds none.
	private path(): cool.ObjectPathPolygon[] | undefined {
		return RenderGeometrySection.objectOf(this.objectId)?.path;
	}

	/*
		The handles of the path: one on every point, and one on every weight of a curve. A point
		is named by the polygon it belongs to and the place it has in it; a weight by the point it
		belongs to and whether it bends the curve coming into that point or the one going out.
	*/
	public handles(): SelectionHandle[] {
		const handles: SelectionHandle[] = [];

		(this.path() ?? []).forEach(
			(polygon: cool.ObjectPathPolygon, which: number) => {
				(polygon.points ?? []).forEach(
					(point: cool.ObjectPathPoint, at: number) => {
						const named = String(which) + '.' + String(at);

						handles.push({
							name: '9.' + named,
							kind: '9',
							pointer: '28',
							point: new cool.Point(point.x, point.y),
						});

						if (point.behindX !== undefined && point.behindY !== undefined)
							handles.push({
								name: '10.' + named + '.in',
								kind: '10',
								pointer: '28',
								point: new cool.Point(point.behindX, point.behindY),
							});

						if (point.aheadX !== undefined && point.aheadY !== undefined)
							handles.push({
								name: '10.' + named + '.out',
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
		What a handle of the path is called says the polygon it belongs to, the place its point
		has in that polygon, and for a weight whether it bends the curve coming into the point or
		the one going out of it. Nothing else speaks that language: the engine is told which
		point moved and where to, not which handle.
	*/
	private static named(handleName: string): {
		polygon: number;
		point: number;
		part: number;
	} | null {
		const named = handleName.split('.');
		if (named[0] !== '9' && named[0] !== '10') return null;

		return {
			polygon: Number(named[1]),
			point: Number(named[2]),
			part: named[0] === '9' ? 0 : named[3] === 'in' ? 1 : 2,
		};
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
			// A weight is named after the point it bends the curve at, with the side it bends
			// written last, so the point it ties to is that name without the side.
			const named = weight.name.split('.');
			const point = points.get('9.' + named[1] + '.' + named[2]);
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
		What the engine is told when a drag of a point or of a weight ends: which polygon, which
		point of it, which of its three parts moved - the point itself, the weight of the curve
		coming in, or the one going out - where it moved to, in twips, and how the curve is to run
		through that point.

		The continuity travels because a polygon holds none: whether a point is a corner or is run
		through smoothly lies in its numbers alone, so the two sides could read the same point
		differently. The client draws the curve one way and says which way, and the model is then
		made to hold it exactly that way.

		Only that travels. The path can hold many thousand points, none of the others changes, so
		none of them is sent and none of them can come back a little different from the way it
		went out.
	*/
	public changeFor(handleName: string, at: cool.Point): string | null {
		const named = PolyPolygonEditor.named(handleName);
		if (!named) return null;

		const point = this.path()?.[named.polygon]?.points?.[named.point];

		return [
			named.polygon,
			named.point,
			named.part,
			Math.round(at.x),
			Math.round(at.y),
			point?.continuity ?? 0,
		].join(',');
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
		const named = PolyPolygonEditor.named(handleName);
		if (!named) return null;

		const points = this.path()?.[named.polygon]?.points ?? [];
		if (named.point >= points.length) return null;

		const stands = points[named.point];

		if (named.part !== 0)
			return {
				polygon: named.polygon,
				at: named.point,
				point: PolyPolygonEditor.withTheWeightMoved(
					stands,
					named.part === 1,
					at,
				),
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

		const named = String(moved.polygon) + '.' + String(moved.at);
		const where: { [name: string]: cool.Point | undefined } = {
			['9.' + named]: new cool.Point(moved.point.x, moved.point.y),
			['10.' + named + '.in']:
				moved.point.behindX === undefined || moved.point.behindY === undefined
					? undefined
					: new cool.Point(moved.point.behindX, moved.point.behindY),
			['10.' + named + '.out']:
				moved.point.aheadX === undefined || moved.point.aheadY === undefined
					? undefined
					: new cool.Point(moved.point.aheadX, moved.point.aheadY),
		};

		return known.map((handle: SelectionHandle) => {
			const point = where[handle.name];
			return point ? { ...handle, point: point } : handle;
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
