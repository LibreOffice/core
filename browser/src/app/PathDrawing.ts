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

/// How a path is drawn: whether it may hold several polygons, whether each of them closes when
/// it ends, whether its edges keep to steps of 45 degrees by themselves, whether its points get
/// weights, so its edges are curves, and whether each part is drawn by hand in one stroke.
interface PathDrawingKind {
	multi: boolean;
	closes: boolean;
	inSteps: boolean;
	curves?: boolean;
	freehand?: boolean;
}

/// What a step of drawing did: the path goes on, a part of it ended, the path is finished, or
/// nothing is left of it.
type PathDrawingOutcome = 'goesOn' | 'partEnded' | 'finished' | 'nothingLeft';

/*
	A path being drawn point by point, in twips: the parts drawn so far, the part being drawn, and
	the point that follows the mouse until it is set. It knows nothing of the mouse or of where the
	path goes once finished; whoever draws with it tells it where the mouse is and what it did.

	A point of a polygon is set when the button comes up, and follows the mouse while it is
	pressed, so it can be corrected. A point of a curve is set when the button goes down, and the
	mouse then pulls out its two weights, the one behind mirroring the one ahead; without a pull
	the point is a corner. A part ends when the one who draws says so, or when its own start point
	is hit, which sets no point there and closes it. The parts of one path are all closed or all
	open, as an object holds them: a filled kind closes every part, and for an unfilled one the
	first part decides. Where it was closed on its start point, every later part closes too;
	where it ended open, every later part does, and its start point draws no mouse onto it. A part
	that ends with too few points is dropped.
	A path of one polygon is finished as soon as its part ends; one of several goes on with the
	next part.

	A part drawn by hand is one stroke: the places the mouse goes through while the button is
	down, which are fitted with curves when the stroke ends. A stroke of a kind that closes ends
	by itself, closed, when it comes back to where it began.
*/
class PathDrawing {
	private kind: PathDrawingKind;
	/// The parts drawn so far, each ended.
	private parts: cool.ObjectPathPolygon[] = [];
	/// The points of the part being drawn; empty before its first point is set.
	private points: cool.ObjectPathPoint[] = [];
	/// Where the mouse places the next point, once placed by the rules; null off the page.
	private next: cool.Point | null = null;
	/// How near the start point of the part the mouse is drawn onto it, in twips; a pull shorter
	/// than that leaves a point of a curve a corner as well.
	private reach: number;
	/// The point of a curve whose weights the mouse pulls, by its index in the part; null while
	/// none is pulled.
	private pulled: number | null = null;
	/// Whether the press that pulls was on the start point, which ends the part when the button
	/// comes up.
	private pressedOnTheStart: boolean = false;
	/// Whether the pull went far enough to give weights.
	private pulledOut: boolean = false;
	/// Whether the start point of the part is held off, so it draws no mouse onto it and a stroke
	/// coming back to it does not close.
	private startHeldOff: boolean = false;

	/// Holds the start point of the part off, or lets it draw the mouse onto it again.
	public holdOffTheStart(off: boolean): void {
		this.startHeldOff = off;
	}

	/// Where the press for a curve was, as the mouse was then, not snapped.
	private pressedAt: cool.Point | null = null;
	/// The places of the stroke being drawn by hand, in twips; null while none is drawn.
	private stroke: cool.Point[] | null = null;
	/// Whether the stroke went further than the reach from where it began, so coming back there
	/// closes it.
	private strokeLeftItsStart: boolean = false;
	/// The stroke being drawn as fitted with curves, and how many places it had then; null while
	/// it is not fitted.
	private fittedStroke: {
		places: number;
		points: cool.ObjectPathPoint[];
	} | null = null;

	/// The least step from one place of a stroke to the next, how far the curves fitted to it may
	/// stray from it, and over how long a run a turn is looked at for a corner, in twips.
	private strokeStep: number = 0;
	private strokeTolerance: number = 0;
	private cornerReach: number = 0;

	/// Says how fine a stroke drawn by hand is taken, in twips.
	public strokesTaken(
		step: number,
		tolerance: number,
		cornerReach: number,
	): void {
		this.strokeStep = step;
		this.strokeTolerance = tolerance;
		this.cornerReach = cornerReach;
	}

	/// Begins a stroke drawn by hand at that place.
	public strokeFrom(at: cool.Point): void {
		this.fittedStroke = null;
		this.stroke = [at];
		this.strokeLeftItsStart = false;
		this.next = null;
	}

	/// Whether a stroke is being drawn by hand.
	public stroking(): boolean {
		return this.stroke !== null;
	}

	/*
		Takes the stroke on to that place, where it lies the least step from the place before.
		Where the kind closes its parts and the stroke comes back within the reach of where it
		began, it ends there, closed.
	*/
	public strokeTo(at: cool.Point): PathDrawingOutcome {
		const stroke = this.stroke;
		if (!stroke) return 'goesOn';
		const start = stroke[0];
		const last = stroke[stroke.length - 1];
		if (last.distanceTo(at) < this.strokeStep) return 'goesOn';

		const nearTheStart = start.distanceTo(at) <= this.reach;
		if (!nearTheStart && start.distanceTo(at) > 2 * this.reach)
			this.strokeLeftItsStart = true;
		if (
			this.kind.closes &&
			!this.startHeldOff &&
			nearTheStart &&
			this.strokeLeftItsStart
		)
			return this.strokeEnd();

		stroke.push(at);
		return 'goesOn';
	}

	/*
		The stroke being drawn, fitted with curves as it will be when it ends, so it shows smooth
		while it is drawn. It is fitted again only once it has a new place.
	*/
	private strokeFitted(): cool.ObjectPathPoint[] {
		const stroke = this.stroke ?? [];
		if (this.fittedStroke?.places !== stroke.length)
			this.fittedStroke = {
				places: stroke.length,
				points: PathFit.fit(
					stroke,
					this.strokeTolerance,
					this.cornerReach,
					false,
				),
			};
		return this.fittedStroke.points;
	}

	/// Whether the stroke being drawn has gone anywhere, so it is a stroke and not a click.
	public strokeWent(): boolean {
		return (this.stroke?.length ?? 0) >= 2;
	}

	/// Ends the stroke being drawn and makes a part of it, fitted with curves.
	public strokeEnd(): PathDrawingOutcome {
		const stroke = this.stroke;
		this.stroke = null;
		if (!stroke) return 'goesOn';
		this.points = PathFit.fit(
			stroke,
			this.strokeTolerance,
			this.cornerReach,
			this.kind.closes,
		);
		return this.endThePart(this.kind.closes);
	}

	/// Gives up the stroke being drawn.
	public strokeDropped(): void {
		this.stroke = null;
	}

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
			this.closing() !== false &&
			!this.startHeldOff &&
			!!start &&
			!!this.next &&
			this.points.length >= 2 &&
			this.next.x === start.x &&
			this.next.y === start.y
		);
	}

	/// Whether the parts close: every part of a filled kind; for an unfilled one as its first
	/// part ended, and undefined while there is none, so the first one may still close.
	private closing(): boolean | undefined {
		return this.kind.closes ? true : this.parts[0]?.closed;
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
			this.closing() !== false &&
			!this.startHeldOff &&
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
		the start point of the part it only ends the part, closed, with no second point there. A
		part ended otherwise closes where the parts close.
	*/
	public setThePoint(endsThePart: boolean): PathDrawingOutcome {
		if (!this.next) return 'goesOn';

		if (this.onTheStart()) return this.endThePart(true);

		const last = this.points[this.points.length - 1];
		if (!last || last.x !== this.next.x || last.y !== this.next.y)
			this.points.push({ x: this.next.x, y: this.next.y });

		return endsThePart ? this.endThePart(!!this.closing()) : 'goesOn';
	}

	/*
		A press for a curve at that place, as the mouse is: sets the next point where it is placed,
		or takes hold of the start point of the part where the press is on it, and the mouse then
		pulls the weights of that point. On the last point again it pulls that one.
	*/
	public press(at: cool.Point): void {
		if (!this.next) return;
		this.pressedAt = at;
		this.pressedOnTheStart = this.onTheStart();
		this.pulledOut = false;
		const last = this.points[this.points.length - 1];
		if (this.pressedOnTheStart) this.pulled = 0;
		else {
			if (!last || last.x !== this.next.x || last.y !== this.next.y)
				this.points.push({ x: this.next.x, y: this.next.y });
			this.pulled = this.points.length - 1;
		}
		this.next = null;
	}

	/*
		The mouse pulls the weights of the point pressed to that place: the one ahead goes there,
		the one behind as far the other way, in steps of 45 degrees where turnedOver asks for
		them. The pull counts once the mouse went further than the reach from where the press was,
		so a press that snapped the point is no pull; a pull back nearer than the reach to the
		point takes the weights away again.
	*/
	public pull(to: cool.Point, turnedOver: boolean): void {
		const point = this.pulled === null ? undefined : this.points[this.pulled];
		if (!point) return;

		let x = to.x - point.x;
		let y = to.y - point.y;
		const length = Math.hypot(x, y);
		if (turnedOver) {
			const step = Math.PI / 4;
			const angle = Math.round(Math.atan2(y, x) / step) * step;
			x = Math.cos(angle) * length;
			y = Math.sin(angle) * length;
		}

		this.pulledOut =
			this.pulledOut ||
			(!!this.pressedAt && to.distanceTo(this.pressedAt) > this.reach);
		if (!this.pulledOut) return;
		if (length <= this.reach) {
			PathDrawing.cornerAt(point);
			return;
		}
		point.aheadX = point.x + x;
		point.aheadY = point.y + y;
		point.behindX = point.x - x;
		point.behindY = point.y - y;
	}

	/*
		The button of a press for a curve came up. On the start point it ends the part; where the
		part closes and the start point was not pulled, the edge into it carries on as the edge out
		of it begins, its weight behind mirroring the one ahead. With endsThePart the part ends
		after the point.
	*/
	public release(endsThePart: boolean): PathDrawingOutcome {
		if (this.pulled === null) return 'goesOn';
		const onTheStart = this.pressedOnTheStart;
		this.pulled = null;
		this.pressedOnTheStart = false;

		if (onTheStart) {
			const start = this.points[0];
			if (
				!this.pulledOut &&
				start.aheadX !== undefined &&
				start.aheadY !== undefined
			) {
				start.behindX = 2 * start.x - start.aheadX;
				start.behindY = 2 * start.y - start.aheadY;
			}
			return this.endThePart(true);
		}
		return endsThePart ? this.endThePart(!!this.closing()) : 'goesOn';
	}

	/// Whether a point of a curve is held by a press, its weights following the mouse.
	public pulling(): boolean {
		return this.pulled !== null;
	}

	/// The point of a curve whose weights are shown: the one held by a press, else the last one
	/// of the part being drawn, which the next edge starts from; null where there is none.
	public pointOfTheWeights(): cool.ObjectPathPoint | null {
		if (!this.kind.curves) return null;
		return this.points[this.pulled ?? this.points.length - 1] ?? null;
	}

	/// Takes the weights of that point away, so the curve turns a corner there.
	private static cornerAt(point: cool.ObjectPathPoint): void {
		delete point.aheadX;
		delete point.aheadY;
		delete point.behindX;
		delete point.behindY;
	}

	/// Finishes the path: the part being drawn ends as the kind ends its parts, a stroke being
	/// drawn by hand too.
	public finish(): PathDrawingOutcome {
		if (this.stroke && this.strokeWent()) {
			const outcome = this.strokeEnd();
			if (outcome === 'finished' || outcome === 'nothingLeft') return outcome;
		}
		this.stroke = null;
		if (this.points.length) this.endThePart(!!this.closing());
		return this.path().length ? 'finished' : 'nothingLeft';
	}

	/*
		Ends the part being drawn, closed or open. A part with too few points for that is dropped:
		one point, or two straight ones where it closes. A path of one polygon is then finished;
		one of several goes on with a part not begun. The weights behind the first point and ahead
		of the last belong to no edge of an open part and go.
	*/
	private endThePart(closed: boolean): PathDrawingOutcome {
		this.pulled = null;
		this.pressedOnTheStart = false;
		const count = this.points.length;
		const curved = this.points.some(
			(point: cool.ObjectPathPoint) =>
				point.aheadX !== undefined || point.behindX !== undefined,
		);
		const enough = closed ? count >= 3 || (count === 2 && curved) : count >= 2;
		if (enough) {
			if (!closed) {
				delete this.points[0].behindX;
				delete this.points[0].behindY;
				delete this.points[count - 1].aheadX;
				delete this.points[count - 1].aheadY;
			}
			this.parts.push({ closed: closed, points: this.points });
		}
		this.points = [];

		if (!this.kind.multi) return enough ? 'finished' : 'nothingLeft';
		return enough ? 'partEnded' : 'goesOn';
	}

	/// Takes back the last point, of the part before where the part being drawn has none, which
	/// is then drawn on. Nothing is left once the last point of the first part goes.
	public back(): PathDrawingOutcome {
		// a part drawn by hand goes as a whole
		if (this.kind.freehand) return this.dropThePart();
		this.pulled = null;
		this.pressedOnTheStart = false;
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
		this.pulled = null;
		// a stroke being drawn is the part being drawn
		if (this.stroke) {
			this.stroke = null;
			if (!this.kind.multi) this.parts = [];
			return this.parts.length ? 'goesOn' : 'nothingLeft';
		}
		this.pressedOnTheStart = false;
		if (!this.kind.multi) {
			this.parts = [];
			this.points = [];
			return 'nothingLeft';
		}

		if (this.points.length) this.points = [];
		else this.parts.pop();
		return this.parts.length ? 'goesOn' : 'nothingLeft';
	}

	/*
		Stops with what is done: the part being drawn goes, a stroke too, and the path of the parts
		ended is finished, or nothing is left where none is. A path of one polygon is given up.
	*/
	public stopHere(): PathDrawingOutcome {
		this.pulled = null;
		this.pressedOnTheStart = false;
		this.stroke = null;
		this.points = [];
		if (!this.kind.multi) this.parts = [];
		return this.parts.length ? 'finished' : 'nothingLeft';
	}

	/// The path as finished so far: the parts ended. Nothing where none is.
	public path(): cool.ObjectPathPolygon[] {
		return this.parts;
	}

	/// The path as it is shown while drawing: the parts ended, the part being drawn, and the piece
	/// from its last point to where the next one would be set. While the start point of a part
	/// that closes is pulled, the part shows closed.
	public shown(): cool.ObjectPathPolygon[] {
		if (this.stroke)
			return this.stroke.length >= 2
				? this.parts.concat([{ closed: false, points: this.strokeFitted() }])
				: this.parts;
		const drawing = this.next ? this.points.concat([this.next]) : this.points;
		const closing = this.pressedOnTheStart;
		return drawing.length >= 2
			? this.parts.concat([{ closed: closing, points: drawing }])
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
