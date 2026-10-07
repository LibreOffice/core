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
	One handle of a selection. The name is what the handle is called wherever it is spoken about,
	and whoever makes the handles says what it is made of: a handle the engine moves carries the
	kind, the polygon and the point the engine knows it by, a handle of a path carries the place
	its point has in that path. The point is in twips.
*/
interface SelectionHandle {
	name: string;
	kind: string;
	pointer: string;
	point: cool.Point;
	/** Shown, and doing nothing. A handle that stands for something the objects will not have
	 * done to them is still drawn, so that the selection can be seen, but it is drawn pale, the
	 * mouse cannot take hold of it and the keyboard walks past it.
	 */
	deactivated?: boolean;
	/// True for a handle that is lit: the point an end being dragged would tie itself to.
	lit?: boolean;
	/** Which way the handle is drawn away from the place it stands on, one step along each of
	 * the two axes of the selection, with a nought for an axis it does not move along. It is
	 * only for the drawing and for the hit test: what a drag does goes from the place itself.
	 * Absent for a handle that is drawn where it stands.
	 */
	awayFromItsPlace?: cool.Point;
	/// How far it is drawn away, in core pixels, where that is not the distance the eight move
	/// out by. A crop mark moves in by half of itself, so its box lies on the mark.
	awayBy?: number;
	/// The angle the selection's own across axis stands at, in radians, for a handle that may be
	/// drawn turned with the selection. Absent for one that is always drawn upright.
	turnedBy?: number;
	/// The number the object itself gives the handle, for the handles that are told apart by
	/// nothing else. Absent where the name already says which one it is.
	at?: number;
	/// The name of the group of handles it came from, which is what draws it, what a press on it
	/// asks and what a key on it asks. Absent for a handle that stands outside the groups.
	group?: string;
	/// How far it may be moved, on the square of the object it belongs to. Absent for a handle
	/// that is held nowhere.
	rails?: cool.ObjectHandleRails;
}

/*
	Something that has handles: a selected object, a selection of several, the editor of a
	polyPolygon. One of them can hold others, so that an object being edited offers its own
	handles and the points of a path that belongs to it at the same time.
*/
interface HandleSource {
	/// The handles it has, in the order they are drawn and traveled.
	handles(): SelectionHandle[];
}

/*
	The points a connector can tie itself to: the ones someone added to an object, and the four an
	object falls back on, which lie in the middle of each side of the box around it.

	They are shown and no more: there is no name by which one of them could be spoken about, so the
	keyboard passes them by and the mouse cannot take hold of one.
*/
class GluePointHandles {
	/// One point to tie to, at that place, of the kind that says where it comes from.
	private static handleAt(x: number, y: number, kind: string): SelectionHandle {
		return {
			name: '',
			kind: kind,
			pointer: '0',
			point: new cool.Point(x, y).round(),
		};
	}

	/*
		The points someone added to that object, where the object carries them. They are named by
		the place they have in the object's own list, so the keyboard can walk them, and the
		mouse finds them where they lie.
	*/
	public static ownPointsOf(objectId: number): SelectionHandle[] {
		const object = RenderGeometrySection.objectOf(objectId);
		const transform = RenderGeometrySection.transformOf(objectId);
		if (!transform) return [];

		return (object?.gluePoints ?? []).map(
			(point: { x: number; y: number }, at: number) => {
				const onThePage = transform.apply(point.x, point.y);

				return {
					...GluePointHandles.handleAt(onThePage.x, onThePage.y, 'GluePoint'),
					name: 'GluePoint.' + String(at),
				};
			},
		);
	}

	/*
		The four points an object falls back on, in the middle of each of its sides: the upper,
		the right, the lower and the left one. Where they lie is the object's own business - a
		rectangle or a picture turns and shears them with itself, other objects take the sides of
		the upright box they stand in - so they are taken from the object as it gives them. Data
		that carries none falls back on the middles of the upright box.
	*/
	public static defaultPointsOf(objectId: number): SelectionHandle[] {
		const transform = RenderGeometrySection.transformOf(objectId);
		if (!transform) return [];

		const fallback = 'DefaultGluePoint';

		// The object gives the four itself. The middles of its box stand in where it gave none.
		const sides = RenderGeometrySection.objectOf(objectId)?.sideGluePoints;
		if (sides?.length === 4)
			return sides.map((side: cool.ObjectPlace) => {
				const at = transform.apply(side.x, side.y);
				return GluePointHandles.handleAt(at.x, at.y, fallback);
			});

		const box = cool.Range2D.fromPoints(
			[
				[0, 0],
				[1, 0],
				[1, 1],
				[0, 1],
			].map(([x, y]: number[]) => transform.apply(x, y)),
		);

		return [
			GluePointHandles.handleAt(box.centerX, box.minY, fallback),
			GluePointHandles.handleAt(box.maxX, box.centerY, fallback),
			GluePointHandles.handleAt(box.centerX, box.maxY, fallback),
			GluePointHandles.handleAt(box.minX, box.centerY, fallback),
		];
	}
}

/*
	The handles of a selection of objects, worked out from the geometry the client holds: the
	eight that frame the selection, and the ones that shape a single object, its corner radius
	and the points a custom shape is shaped by.
*/
class ObjectHandles implements HandleSource {
	private objectIds: number[];

	constructor(objectIds: number[]) {
		this.objectIds = objectIds;
	}

	public handles(): SelectionHandle[] {
		if (!this.objectIds.length) return [];

		if (this.isATiedConnector())
			return [...this.shapingHandles(), ...this.ownGluePoints()];

		const framing = ObjectHandles.framingOf(this.objectIds);
		if (!framing) return [];

		return [...framing, ...this.shapingHandles(), ...this.ownGluePoints()];
	}

	/*
		The points someone added to the object for a connector to tie itself to. They belong to the
		object, so they are shown while it is selected, after the handles that shape it and before
		the points of its path.
	*/
	private ownGluePoints(): SelectionHandle[] {
		if (this.objectIds.length !== 1) return [];

		return GluePointHandles.ownPointsOf(this.objectIds[0]);
	}

	/*
		Where on the object's own square each of the eight that frame it sits: 0 and 0 is its
		upper left corner and 1 and 1 its lower right one. A handle in the middle of a side sits
		at a half on the axis it does not move on.
	*/
	private static readonly placesOfTheKinds: { [kind: string]: number[] } = {
		'1': [0, 0],
		'2': [0.5, 0],
		'3': [1, 0],
		'4': [0, 0.5],
		'5': [1, 0.5],
		'6': [0, 1],
		'7': [0.5, 1],
		'8': [1, 1],
	};

	/// Where one of the eight of that kind sits on the object's own square. Nothing for a kind
	/// that is not one of the eight.
	public static placeOfKind(kind: string): cool.Point | undefined {
		const place = ObjectHandles.placesOfTheKinds[kind];
		return place ? new cool.Point(place[0], place[1]) : undefined;
	}

	/*
		The handles a change that turns the object over can put in the place of the given one:
		itself, the one it reflects onto across the square, the one down it, and the one across
		and down both. A handle in the middle of a side reflects onto itself on the axis it sits
		in the middle of, so only it and the one facing it come out.
	*/
	public static facing(kind: string): string[] {
		const mine = ObjectHandles.placeOfKind(kind);
		if (!mine) return [kind];

		const wanted = [
			new cool.Point(mine.x, mine.y),
			new cool.Point(1 - mine.x, mine.y),
			new cool.Point(mine.x, 1 - mine.y),
			new cool.Point(1 - mine.x, 1 - mine.y),
		];

		return Object.keys(ObjectHandles.placesOfTheKinds).filter((one: string) => {
			const place = ObjectHandles.placeOfKind(one);
			return wanted.some((other: cool.Point) => place && other.equals(place));
		});
	}

	/// The number the drawing layer gives a connector among the kinds of object.
	private static readonly connectorKind: number = 24;

	/*
		Where a handle that keeps a length of its own stands once the object has been given a
		change: as far along its rail as it was, on the rail as the change leaves it. The corner
		radius of a rectangle is such a handle - the radius is a length the object keeps, so the
		handle does not grow with the object - where a point a custom shape is shaped by is a
		share of the shape and travels with it, needing none of this.

		Nothing for a handle that travels, or for one whose rail is not a line.
	*/
	public static keptAlongItsRail(
		rails: cool.ObjectHandleRails | undefined,
		point: cool.Point,
		was: cool.Matrix2D,
		becomes: cool.Matrix2D,
	): cool.Point | undefined {
		if (!rails?.keepsItsLength) return undefined;

		const least = rails.leastAcross;
		const most = rails.mostAcross;
		if (!least || !most) return undefined;

		// Where the rail starts on the page, and the way it runs from there.
		const ends = (mapping: cool.Matrix2D) => {
			const from = mapping.apply(least.x, least.y);
			return {
				from: from,
				along: mapping.apply(most.x, most.y).subtract(from),
			};
		};

		const before = ends(was);
		const after = ends(becomes);
		const length = after.along.length();
		if (!length) return undefined;

		const howFar = Math.min(point.distanceTo(before.from), length);

		return after.from
			.add(after.along.divideBy(length).multiplyBy(howFar))
			.round();
	}

	/*
		What may be done to the objects of a selection: it may be moved only where every one of
		them may be moved, and the same for the other two. The objects say it themselves, so a
		selection of several is not one answer for all of them any more.

		A group is asked about along with everything it holds. What may be done to it is what may
		be done to all of them: a group holding one object whose place is held fast cannot be
		moved, however little the group itself says about it.
	*/
	private static allOfThem(objectIds: number[]): number[] {
		return RenderGeometrySection.theOnesThatHaveASay(objectIds);
	}

	public static canBeMoved(objectIds: number[]): boolean {
		return ObjectHandles.allOfThem(objectIds).every(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeMoved !== true,
		);
	}

	public static canBeResized(objectIds: number[]): boolean {
		return ObjectHandles.allOfThem(objectIds).every(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeResized !== true,
		);
	}

	public static canBeTurned(objectIds: number[]): boolean {
		return ObjectHandles.allOfThem(objectIds).every(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeTurned !== true,
		);
	}

	/// Whether the object stands at a right angle to the page, turned by a multiple of ninety
	/// degrees or not at all. A resize with two different factors shears no such object.
	private static standsAtARightAngle(objectId: number): boolean {
		const transform = RenderGeometrySection.transformOf(objectId);
		if (!transform) return true;

		const quarterTurns = transform.rotation() / (Math.PI / 2);
		return Math.abs(quarterTurns - Math.round(quarterTurns)) < 0.0001;
	}

	/*
		Whether a resize of these objects is laid through a square that is not an object's own: a
		selection of several stands on the box round them all, and a group on its own box, both
		upright on the page. A member turned off the right angles is sheared by two different
		factors through such a square. A single object that holds nothing is resized on its own
		square and shears never, however it is turned.
	*/
	private static resizesThroughABox(objectIds: number[]): boolean {
		return (
			objectIds.length > 1 ||
			(objectIds.length === 1 &&
				RenderGeometrySection.holdsObjects(objectIds[0]))
		);
	}

	/*
		Whether a resize of these objects keeps the ratio unless the user asks otherwise: the
		office holds a resize in ratio as soon as one object says it allows no free resize, which
		a shape says once it is turned at all. That matters only where the resize goes through a
		box and the object stands turned in it; a single object, or a member at a right angle, is
		not sheared by a free resize and does not ask for the ratio.
	*/
	public static keepsTheRatioByDefault(objectIds: number[]): boolean {
		if (!ObjectHandles.resizesThroughABox(objectIds)) return false;

		return ObjectHandles.allOfThem(objectIds).some(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeResizedFreely ===
					true && !ObjectHandles.standsAtARightAngle(objectId),
		);
	}

	/*
		Whether a resize of these objects keeps the ratio whatever the user asks: one of them
		cannot hold a shear at all and stands turned in the box the resize goes through, so two
		different factors would bring in a shear it has nowhere to put.
	*/
	public static keepsTheRatioAlways(objectIds: number[]): boolean {
		if (!ObjectHandles.resizesThroughABox(objectIds)) return false;

		return ObjectHandles.allOfThem(objectIds).some(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeSheared === true &&
				!ObjectHandles.standsAtARightAngle(objectId),
		);
	}

	public static canBeMirrored(objectIds: number[]): boolean {
		return ObjectHandles.allOfThem(objectIds).every(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId)?.cannotBeMirrored !== true,
		);
	}

	/*
		Gives each of the eight the way it is drawn away from the place it stands on: one step
		along each of the two axes of the selection, worked out from where the eight stand now.
		The axes are the selection's own and not the page's, so a handle stays outside whatever
		angle the selection stands at, and a change that turns the selection over turns the axes
		over with it, so the handles keep facing outwards through a mirror.

		A handle that was not given a way to face is left as it is, which is every handle the
		engine sends.
	*/
	public static facedAwayFromTheirPlaces(
		handles: SelectionHandle[],
	): SelectionHandle[] {
		const placeOf = (kind: string) =>
			handles.find((one: SelectionHandle) => one.kind === kind)?.point;
		const upperLeft = placeOf('1');
		const upperRight = placeOf('3');
		const lowerLeft = placeOf('6');
		if (!upperLeft || !upperRight || !lowerLeft) return handles;

		// The way from one place to the other, one long.
		const step = (from: cool.Point, to: cool.Point): cool.Point => {
			const along = to.subtract(from);
			return along.divideBy(along.length() || 1);
		};
		const across = step(upperLeft, upperRight);
		const down = step(upperLeft, lowerLeft);

		return handles.map((one: SelectionHandle) => {
			const place = ObjectHandles.placeOfKind(one.kind);
			if (!place || !one.awayFromItsPlace) return one;

			const sideways = Math.sign(place.x - 0.5);
			const downwards = Math.sign(place.y - 0.5);

			return {
				...one,
				awayFromItsPlace: across
					.multiplyBy(sideways)
					.add(down.multiplyBy(downwards)),
				turnedBy: Math.atan2(across.y, across.x),
			};
		});
	}

	/// Whether the object of that id is a connector.
	public static isAConnector(objectId: number): boolean {
		return (
			RenderGeometrySection.objectOf(objectId)?.objectKind ===
			ObjectHandles.connectorKind
		);
	}

	/// Whether the object of that id is a connector that is tied to an object at either end.
	public static isATiedConnector(objectId: number): boolean {
		if (!ObjectHandles.isAConnector(objectId)) return false;

		const object = RenderGeometrySection.objectOf(objectId);

		return object?.tiedAtStart === true || object?.tiedAtEnd === true;
	}

	/// Whether what is selected is one connector that is tied to an object.
	private isATiedConnector(): boolean {
		return (
			this.objectIds.length === 1 &&
			ObjectHandles.isATiedConnector(this.objectIds[0])
		);
	}

	/*
		The eight handles that frame what is selected, from the upper left one to the lower right
		one, worked out from the objects the client holds. One object is framed by its own
		mapping, so the handles turn with it; several are framed by one upright box around them
		all, and the whole selection is scaled by it. Nothing where the client holds no geometry
		for what is selected.
	*/
	public static framingOf(
		objectIds: number[],
		square?: cool.Matrix2D,
	): SelectionHandle[] | undefined {
		/*
			A connector that is tied to an object runs from where it is tied, so scaling it or
			turning it leads nowhere: it is offered the points along its way alone, and neither
			the eight nor the handle that turns them, which is made from the eight.
		*/
		if (objectIds.length === 1 && ObjectHandles.isATiedConnector(objectIds[0]))
			return undefined;

		return new ObjectHandles(objectIds).framingHandles(square);
	}

	/// The handles that shape the objects, which only one object on its own has.
	public static shapingOf(objectIds: number[]): SelectionHandle[] {
		return new ObjectHandles(objectIds).shapingHandles();
	}

	/*
		The eight, laid on the square handed in where one is - the selection's own square, which
		remembers how it has been turned and turned over - and otherwise on the object's mapping
		for one object, or on the upright box round them all for several.
	*/
	private framingHandles(
		square?: cool.Matrix2D,
	): SelectionHandle[] | undefined {
		const mapping = square ?? this.framingMapping();
		if (!mapping) return undefined;

		// A framing handle sits on each corner of the unit square and halfway along each side,
		// taken onto the page by the mapping, from the upper left one to the lower right one.
		const corners = Object.values(ObjectHandles.placesOfTheKinds).map(
			([x, y]: number[]) => mapping.apply(x, y),
		);

		// The pointer the engine asks for at each of the eight, in the same order.
		const pointers = [11, 7, 12, 9, 10, 13, 8, 14];

		/*
			An object without width or height would have handles on top of each other, so only the
			ones that stand apart are made: the corners go where there is both width and height,
			the handle in the middle of a side where the side has a length, and an object that is
			a point keeps the upper left one alone. The engine draws them by the same rule.
		*/
		const [upperLeft, upperRight, lowerLeft] = [
			corners[0],
			corners[2],
			corners[5],
		];
		const hasWidth = !upperLeft.equals(upperRight);
		const hasHeight = !upperLeft.equals(lowerLeft);

		const wanted = (kind: number): boolean => {
			if (!hasWidth && !hasHeight) return kind === 1;
			// The corners, and the middle of a side across the direction that has a length.
			if (kind === 2 || kind === 7) return hasWidth;
			if (kind === 4 || kind === 5) return hasHeight;
			return hasWidth && hasHeight;
		};

		/*
			What draws the handles expects the whole set of eight and reads them by kind, so an
			object that would have fewer of them is left to the engine, which sends the handles
			that object really has.
		*/
		if (![1, 2, 3, 4, 5, 6, 7, 8].every(wanted)) return undefined;

		// The eight are what a scale takes hold of, so where the objects may not be made larger
		// or smaller they are shown and do nothing.
		const dead = !ObjectHandles.canBeResized(this.objectIds);

		return ObjectHandles.facedAwayFromTheirPlaces(
			corners.map((corner: cool.Point, index: number) => ({
				name: String(index + 1) + '.0.0',
				kind: String(index + 1),
				pointer: String(pointers[index]),
				point: corner.round(),
				deactivated: dead,
				awayFromItsPlace: new cool.Point(0, 0),
			})),
		);
	}

	/*
		The mapping of the unit square onto what the framing handles stand around. One object is
		framed by its own mapping, so the handles turn with it; several are framed by one upright
		box around them all. Nothing where the client holds no geometry for what is selected.
	*/
	private framingMapping(): cool.Matrix2D | null {
		if (this.objectIds.length === 1)
			return RenderGeometrySection.transformOf(this.objectIds[0]);

		let box: cool.Range2D | null = null;
		for (const objectId of this.objectIds) {
			const one = RenderGeometrySection.boxOf(objectId);
			if (!one) return null;

			box = box ? box.union(one) : one;
		}

		return box ? cool.Matrix2D.fromRange(box) : null;
	}

	/*
		The handles that shape one object, which it carries itself: the corner radius of a
		rectangle and the points a custom shape is shaped by. A selection of several objects
		shapes none of them, it only frames them.
	*/
	private shapingHandles(): SelectionHandle[] {
		if (this.objectIds.length !== 1) return [];

		const object = RenderGeometrySection.objectOf(this.objectIds[0]);

		return (object?.handles ?? []).map((handle: cool.ObjectHandle) => ({
			name:
				String(handle.kind) +
				'.' +
				String(handle.polygon ?? 0) +
				'.' +
				String(handle.point ?? 0),
			kind: String(handle.kind),
			pointer: '28',
			point: new cool.Point(handle.x, handle.y),
			at: handle.at,
			rails: handle.rails,
		}));
	}
}
