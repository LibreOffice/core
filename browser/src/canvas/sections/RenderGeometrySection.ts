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
	A change laid over an object here that the engine has not answered yet: the change itself and
	where it left the object, as the mapping that places it and the box it takes up. An answer from
	the engine is held against both.
*/
class PendingChange {
	public readonly change: cool.Matrix2D;
	private leftAt: cool.Matrix2D;
	private leftIn: cool.Range2D;

	private constructor(
		change: cool.Matrix2D,
		leftAt: cool.Matrix2D,
		leftIn: cool.Range2D,
	) {
		this.change = change;
		this.leftAt = leftAt;
		this.leftIn = leftIn;
	}

	/// The change as it left the object it was just laid over, or null where the object has no
	/// mapping to hold an answer against.
	public static leftOn(
		change: cool.Matrix2D,
		object: cool.SlideObject,
	): PendingChange | null {
		const leftAt = cool.Matrix2D.fromArray(object.transform);
		return leftAt
			? new PendingChange(change, leftAt, PendingChange.boxOf(object))
			: null;
	}

	/// Takes in where the change leaves the object after it was laid over it again.
	public laidAgainOn(object: cool.SlideObject): void {
		this.leftAt = cool.Matrix2D.fromArray(object.transform) ?? this.leftAt;
		this.leftIn = PendingChange.boxOf(object);
	}

	/// Whether the engine answered with the mapping this change left the object at. The engine
	/// rounds what it answers to a small part of a twip.
	public answeredBy(answer: cool.Matrix2D): boolean {
		return answer.equals(this.leftAt, 1.5);
	}

	/*
		Whether the object stands in the box this change left it in. A mapping and its turned-over
		twin draw the same box, and an object that cannot hold a mirror answers with the twin, so
		the box is what such an answer is held against. The box the engine sends is the painted
		one and carries the line width, which a change laid on here does not scale, so a little is
		allowed for.
	*/
	public answeredInTheBox(object: cool.SlideObject): boolean {
		if (object.x === undefined || object.y === undefined) return false;

		const box = this.leftIn;
		const give = 20 + 0.01 * Math.max(box.width, box.height);
		const near = (value: number, other: number) =>
			Math.abs(value - other) < give;

		return (
			near(object.x, box.minX) &&
			near(object.y, box.minY) &&
			near(object.width ?? 0, box.width) &&
			near(object.height ?? 0, box.height)
		);
	}

	/// The box the object takes up now.
	private static boxOf(object: cool.SlideObject): cool.Range2D {
		const x = object.x ?? 0;
		const y = object.y ?? 0;

		return new cool.Range2D(
			x,
			y,
			x + (object.width ?? 0),
			y + (object.height ?? 0),
		);
	}
}

/*
	Draws two hairline rectangles per object of the part a view shows, each in the object's own
	color: the upright box the object paints into, and the unit rectangle put through the object's
	transformation, which shows the rotation and the shear. It is a way to see the geometry the
	client holds for each object, which is what a hit test and a client-side move work from.

	The rectangles come from the vector rendering cache, so nothing is asked of the engine for them
	and an object with no primitives of its own, such as a group, still shows its box. It draws
	after the section that paints the page and before the overlay, so the cursors, selections and
	handles the overlay draws stay on top.

	The same cache answers what lies under a point. The mouse pointer is worked out from that rather
	than asked of the engine, and the object under the pointer is outlined along the geometry that
	answered. The section is there whenever the document is drawn from vector primitives, so both
	always are. The rectangles per object and the name of what was hit are drawn only while the
	debug panel's "Vector Overlays" tool is on.
*/

/* global app RenderManager Cursor */

/// An object of the page and what of it a hit test found.
interface RenderGeometryHit {
	id: number;
	hit: cool.HitResult;
}

class RenderGeometrySection extends CanvasSectionObject {
	zIndex: number = app.CSections.RenderGeometry.zIndex;
	drawingOrder: number = app.CSections.RenderGeometry.drawingOrder;
	processingOrder: number = app.CSections.RenderGeometry.processingOrder;
	boundToSection: string = 'tiles';
	interactable: boolean = false;

	// True once the class listens to the vector cache. The section comes and goes with the
	// debug tool while the listener stays for the life of the page, so the listener belongs
	// to the class and is registered once.
	private static _listening = false;

	// True once the class follows the pointer. One listener serves every instance this section
	// has over the life of the page.
	private static _followsMouse = false;

	// How far from the geometry a point still counts as a hit, in pixels. Two pixels is what an
	// editing view allows, so a hairline can be picked without landing on it exactly.
	private static readonly hitTolerancePixels = 2;

	// Small dashes, so the outline reads as something following the pointer rather than as part
	// of the document.
	private static readonly hoverDashes = [3, 3];

	// The answer of the last hit test, with the point and the content it was asked about. The
	// pointer, the outline and the name under the pointer ask about the same point for the same
	// frame, and the objects are walked once for all of them.
	private static _lastHit:
		| {
				x: number;
				y: number;
				data: cool.VectorPrimitivesData;
				version: number | undefined;
				hit: RenderGeometryHit | undefined;
		  }
		| undefined;

	// What the outline under the pointer was last drawn for. The document is drawn again only
	// when the object under the pointer, or what of it is hit, changes.
	private static _drawnHitKey = '';

	constructor() {
		super(app.CSections.RenderGeometry.name);

		// The cache fills and changes on its own, so a redraw follows what arrives, and the
		// remembered hit test answer is dropped.
		if (!RenderGeometrySection._listening) {
			RenderGeometrySection._listening = true;
			RenderManager.onVectorChanged((answered?: number[]) => {
				RenderGeometrySection._lastHit = undefined;

				// An answer from the engine is held against what was laid on here first, so
				// that what is shown is the newest state and not a stale answer.
				RenderGeometrySection.takeUpTheAnswers(answered);

				// The handles of a selection are worked out from the objects, so they follow
				// what arrives as well.
				GraphicSelection.refreshLocalHandles();
				if (app.sectionContainer) app.sectionContainer.requestReDraw();
			});
		}

		/*
			The outline marks the object under the pointer, so the pointer moving onto or off an
			object is a reason to draw again, and so is the pointer leaving the document: the
			container forgets where the mouse is then, and the outline goes with it. The document
			canvas is where the mouse is watched, so the pointer moving elsewhere on the page
			changes nothing here.
		*/
		if (!RenderGeometrySection._followsMouse) {
			RenderGeometrySection._followsMouse = true;
			const canvas = document.getElementById('document-canvas');
			canvas?.addEventListener('mousemove', () =>
				RenderGeometrySection.redrawWhenHitMoved(),
			);
			canvas?.addEventListener('mouseleave', () => {
				RenderGeometrySection._drawnHitKey = '';
				if (app.sectionContainer) app.sectionContainer.requestReDraw();
			});
		}
	}

	/// Asks for a redraw when the object under the pointer, or what of it is hit, is not the
	/// one the outline was last drawn for.
	private static redrawWhenHitMoved(): void {
		const section = app.sectionContainer?.getSectionWithName(
			app.CSections.RenderGeometry.name,
		) as RenderGeometrySection | undefined;
		if (!section) return;
		const mouse: number[] = section.containerObject.getMousePosition();
		const hit = mouse ? section.hitObject(mouse) : undefined;
		const key = hit ? String(hit.id) + ':' + hit.hit.kind : '';
		if (key === RenderGeometrySection._drawnHitKey) return;
		app.sectionContainer.requestReDraw();
	}

	/*
		An object keeps its color for as long as it lives, so a rectangle that moves or resizes is
		easy to follow. Stepping the hue by the golden angle puts neighboring ids far apart.
	*/
	public static colorOfObject(id: number): string {
		return 'hsl(' + ((id * 137.508) % 360).toFixed(1) + ', 100%, 45%)';
	}

	/// The content version the connector routing was last checked against the engine's for.
	private static routingCheckedAt: number | undefined = undefined;

	/*
		The four corners of the unit rectangle put through the transformation, in twips. They are
		transformed here rather than by the canvas so the line stays one pixel wide whatever the
		object's scale is.
	*/
	public static unitRectangleCorners(transform: number[]): number[][] {
		const matrix = cool.Matrix2D.fromArray(transform);
		if (!matrix) return [];

		return [
			[0, 0],
			[1, 0],
			[1, 1],
			[0, 1],
		].map(([x, y]: number[]) => {
			const corner = matrix.apply(x, y);
			return [corner.x, corner.y];
		});
	}

	/// The part the view shows, from the vector rendering cache, or nothing while it is on its
	/// way or the view shows no part.
	public static currentPart(): cool.VectorPrimitivesData | undefined {
		// The file based view stacks every page of the document on screen, while these
		// are the rectangles of the one page a view shows.
		const docLayer = app.map?._docLayer;
		if (!docLayer || app.file.fileBasedView || !app.activeDocument)
			return undefined;
		return RenderManager.requestPart(
			docLayer._selectedPart,
			app.activeDocument.activeModes[0],
		);
	}

	/*
		Where the page sits in this section's own coordinates: the document anchor less the part of
		the document that is scrolled out of sight. Both the drawing and the hit test go through it.
	*/
	private documentOffset(): number[] | undefined {
		if (!app.activeDocument || !this.containerObject) return undefined;

		const anchor: number[] = this.containerObject.getDocumentAnchor();
		return [
			anchor[0] - app.activeDocument.activeLayout.viewedRectangle.pX1,
			anchor[1] - app.activeDocument.activeLayout.viewedRectangle.pY1,
		];
	}

	/** The object under the given point and what of it was hit, or nothing when no object is hit.
	 *
	 * The point arrives in this section's own coordinates. An object is asked in two steps:
	 * whether the point lies in the object's own unit rectangle, which is cheap and covers rotation
	 * and shear, and then which of the object's primitives the point hits, asked one by one. The
	 * objects are asked from the front, so the answer is the object a click would reach first.
	 *
	 * The page itself and the text of a running edit are left out: they are how the document looks
	 * behind and over the objects, and they cannot be hit. So are the objects on a
	 * layer the view hides.
	 */
	public hitObject(point: number[]): RenderGeometryHit | undefined {
		const offset = this.documentOffset();
		if (!offset) return undefined;

		return this.hitObjectInDocument(
			(point[0] - offset[0]) * app.pixelsToTwips,
			(point[1] - offset[1]) * app.pixelsToTwips,
		);
	}

	/// The same answer for a point in document coordinates, in twips. The point asked about
	/// last is answered from memory while the content is the one it was asked about.
	public hitObjectInDocument(
		x: number,
		y: number,
	): RenderGeometryHit | undefined {
		const data = RenderGeometrySection.currentPart();
		if (!data) return undefined;

		const last = RenderGeometrySection._lastHit;
		if (
			last &&
			last.x === x &&
			last.y === y &&
			last.data === data &&
			last.version === data.version
		)
			return last.hit;

		const hit = this.walkObjectsAt(data, x, y);
		RenderGeometrySection._lastHit = {
			x: x,
			y: y,
			data: data,
			version: data.version,
			hit: hit,
		};
		return hit;
	}

	private walkObjectsAt(
		data: cool.VectorPrimitivesData,
		x: number,
		y: number,
	): RenderGeometryHit | undefined {
		const tolerance =
			RenderGeometrySection.hitTolerancePixels * app.pixelsToTwips;
		const position: cool.HitPosition = {
			x: x,
			y: y,
			toleranceX: tolerance,
			toleranceY: tolerance,
		};

		for (let index = data.order.length - 1; index >= 0; --index) {
			const id = data.order[index];
			const object = data.objects.get(id);
			// An entry with a kind stands for the page or for a running edit, and the copy of a
			// master placeholder is the master's to own, not the slide's.
			if (!object || object.kind !== undefined || !object.transform) continue;
			if (object.masterContent) continue;
			if (
				object.layer !== undefined &&
				!RenderManager.isLayerVisible(object.layer)
			)
				continue;

			// The box the object paints into is upright and comes with the entry, so a point
			// outside it, and outside the tolerance around it, is settled without building a
			// path. Most objects of a page are settled here.
			const range: cool.HitRange = {
				minX: object.x ?? 0,
				minY: object.y ?? 0,
				maxX: (object.x ?? 0) + (object.width ?? 0),
				maxY: (object.y ?? 0) + (object.height ?? 0),
			};
			if (
				x < range.minX - tolerance ||
				x > range.maxX + tolerance ||
				y < range.minY - tolerance ||
				y > range.maxY + tolerance
			)
				continue;

			if (!cool.VectorHitTest.isInUnitSquare(object.transform, position))
				continue;

			const hit = cool.VectorHitTest.hit(
				object.primitives ?? [],
				position,
				range,
			);
			if (hit) return { id: id, hit: hit };
		}

		return undefined;
	}

	/*
		The mapping that puts a place on the object onto the page, in twips: 0 and 0 is the upper
		left corner of the object and 1 and 1 the lower right one. Its inverse takes a place on the
		page back onto the object. Nothing for an object the client does not hold, or one that
		says no mapping of its own.
	*/
	public static transformOf(objectId: number): cool.Matrix2D | null {
		return cool.Matrix2D.fromArray(
			RenderGeometrySection.objectOf(objectId)?.transform,
		);
	}

	/// The box the object takes up on the page, in twips. Nothing for an object the client does
	/// not hold.
	public static boxOf(objectId: number): cool.Range2D | undefined {
		const object = RenderGeometrySection.objectOf(objectId);
		return object ? RenderGeometrySection.boxOfTheObject(object) : undefined;
	}

	/// The box that object takes up on the page, in twips. Nothing where it says no place.
	private static boxOfTheObject(
		object: cool.SlideObject,
	): cool.Range2D | undefined {
		if (object.x === undefined || object.y === undefined) return undefined;

		return new cool.Range2D(
			object.x,
			object.y,
			object.x + (object.width ?? 0),
			object.y + (object.height ?? 0),
		);
	}

	/*
		Lays a change over the object the client holds: what it draws, the mapping that places it,
		the box it takes up and the points it was given to tie a connector to all come out as the
		change leaves them.

		The client works a drag out itself and sends what it did, so the object it holds is
		changed here rather than waiting for the engine to answer with its own version of it.
		That version replaces this one whenever it arrives, so a change the engine made
		differently puts itself right then.
	*/
	public static changeObject(objectId: number, matrix: cool.Matrix2D): void {
		const object = RenderGeometrySection.objectOf(objectId);
		if (!object) return;

		RenderGeometrySection.layOverTheObject(object, matrix);

		const pending = PendingChange.leftOn(matrix, object);
		if (pending) {
			const laid = RenderGeometrySection.pending.get(objectId) ?? [];
			laid.push(pending);
			RenderGeometrySection.pending.set(objectId, laid);
		}

		// Whoever keeps a picture of the page drew it before this change, so it is told to draw
		// it again. The handles are worked out afresh on every draw and need no telling.
		RenderManager.changedHere();
	}

	/*
		The changes laid over each object here that the engine has not answered yet, by object id,
		oldest first. The engine answers one change at a time and its answers can arrive after
		further changes were laid on here; an answer taken as it comes would show the object as it
		was a step or two ago until the next answer.
	*/
	private static pending: Map<number, PendingChange[]> = new Map();

	/*
		Takes up an answer from the engine for the objects it replaced: the ones named, or every
		object with a change pending where none are named. The answer is held against the
		mappings the pending changes left the object at, newest first. Where it matches one, the
		changes up to that one are answered and the ones after it are laid on again over the
		answer, so the object shows the newest state made here. Where it matches none the engine
		went its own way, and its answer stands as it is with nothing laid on.
	*/
	private static takeUpTheAnswers(answered?: number[]): void {
		const objectIds = answered ?? [...RenderGeometrySection.pending.keys()];

		// A whole part arriving may have changed anything, predicted or not.
		if (!answered) GraphicSelection.unpredictedAnswer();

		for (const objectId of objectIds) {
			const laid = RenderGeometrySection.pending.get(objectId);
			if (!laid) {
				GraphicSelection.unpredictedAnswer(objectId);
				continue;
			}

			const object = RenderGeometrySection.objectOf(objectId);
			const answer = cool.Matrix2D.fromArray(object?.transform);
			if (!object || !answer) {
				RenderGeometrySection.pending.delete(objectId);
				continue;
			}

			// The mapping is the surer of the two, so it is asked first and the box only where
			// no mapping matches.
			let matched = -1;
			for (let at = laid.length - 1; at >= 0 && matched < 0; --at)
				if (laid[at].answeredBy(answer)) matched = at;
			for (let at = laid.length - 1; at >= 0 && matched < 0; --at)
				if (laid[at].answeredInTheBox(object)) matched = at;

			if (matched < 0) GraphicSelection.unpredictedAnswer(objectId);

			const rest = matched < 0 ? [] : laid.slice(matched + 1);
			for (const one of rest) {
				RenderGeometrySection.layOverTheObject(object, one.change);
				one.laidAgainOn(object);
			}

			if (rest.length) RenderGeometrySection.pending.set(objectId, rest);
			else RenderGeometrySection.pending.delete(objectId);
		}
	}

	/// Lays the change over that object, which is what changing one comes down to.
	public static layOverTheObject(
		object: cool.SlideObject,
		matrix: cool.Matrix2D,
	): void {
		const wasDrawnBy = cool.Matrix2D.fromArray(object.transform);

		/*
			What it draws is wrapped in the change rather than being written out again point by
			point: the tree is drawn, reached and measured through it all the same. Where the
			whole of what it draws already hangs under one change, that one is laid over instead
			of another wrap around it, so a second drag before the engine answers leaves one
			change and not a stack of them.
		*/
		const drawn = object.primitives ?? [];
		const only =
			drawn.length === 1 && drawn[0].type === cool.TransformPrimitive.type
				? (drawn[0] as cool.TransformPrimitive)
				: undefined;

		const under = cool.Matrix2D.fromArray(only?.matrix);
		if (only && under) only.matrix = under.then(matrix).toArray();
		else if (drawn.length)
			object.primitives = [
				{
					type: cool.TransformPrimitive.type,
					matrix: matrix.toArray(),
					children: drawn,
				} as cool.Primitive,
			];

		/*
			The change goes into the mapping whole, turn over and all, so that what is held here
			says the same as what is drawn. The engine answers with a mapping worked out the
			drawing layer's own way, and that one takes the place of this as soon as it arrives:
			the engine says what the object is, and the handles are worked out afresh from
			whatever it says.
		*/
		const mapping = cool.Matrix2D.fromArray(object.transform);
		if (mapping) object.transform = mapping.then(matrix).toArray();

		const box = RenderGeometrySection.boxOfTheObject(object);
		if (box) {
			const reached = cool.Range2D.fromPoints([
				matrix.apply(box.minX, box.minY),
				matrix.apply(box.maxX, box.minY),
				matrix.apply(box.maxX, box.maxY),
				matrix.apply(box.minX, box.maxY),
			]);

			object.x = reached.minX;
			object.y = reached.minY;
			object.width = reached.width;
			object.height = reached.height;
		}

		/*
			The handles the object carries are held where they lie on the page, so they move with
			it. The points to tie a connector to are held on the object itself and need nothing.
		*/
		const stood = new Map<cool.ObjectHandle, cool.Point>();
		for (const one of object.handles ?? []) {
			stood.set(one, new cool.Point(one.x, one.y));

			const at = matrix.apply(one.x, one.y);
			one.x = at.x;
			one.y = at.y;
		}

		/*
			A handle that keeps a length of its own does not travel with the object but is laid
			out again on it: the corner radius of a rectangle stays the size it is while the
			object grows. Its place has to be read before the handles above are carried along,
			so the one before the change is kept and used here.
		*/
		const drawnBy = cool.Matrix2D.fromArray(object.transform);
		if (wasDrawnBy && drawnBy) {
			for (const one of object.handles ?? []) {
				const place = ObjectHandles.keptAlongItsRail(
					one.rails,
					stood.get(one) ?? new cool.Point(one.x, one.y),
					wasDrawnBy,
					drawnBy,
				);
				if (!place) continue;

				one.x = place.x;
				one.y = place.y;
			}
		}

		// The path is held where it lies on the page, so it moves with the object.
		for (const polygon of object.path ?? []) {
			for (const point of polygon.points ?? []) {
				const at = matrix.apply(point.x, point.y);
				point.x = at.x;
				point.y = at.y;

				if (point.behindX !== undefined && point.behindY !== undefined) {
					const behind = matrix.apply(point.behindX, point.behindY);
					point.behindX = behind.x;
					point.behindY = behind.y;
				}

				if (point.aheadX !== undefined && point.aheadY !== undefined) {
					const ahead = matrix.apply(point.aheadX, point.aheadY);
					point.aheadX = ahead.x;
					point.aheadY = ahead.y;
				}
			}
		}
	}

	/*
		Whether the object of that id is drawn under the given document point, in twips. It asks
		the object's own primitives, so a thin line answers for the line and not for the box
		around it, and it answers for that one object whatever is drawn over it.
	*/
	public static objectHitAt(objectId: number, x: number, y: number): boolean {
		const object = RenderGeometrySection.objectOf(objectId);
		if (!object) return false;

		const tolerance =
			RenderGeometrySection.hitTolerancePixels * app.pixelsToTwips;

		return (
			cool.VectorHitTest.hit(
				object.primitives ?? [],
				{ x: x, y: y, toleranceX: tolerance, toleranceY: tolerance },
				{
					minX: object.x ?? 0,
					minY: object.y ?? 0,
					maxX: (object.x ?? 0) + (object.width ?? 0),
					maxY: (object.y ?? 0) + (object.height ?? 0),
				},
			) !== undefined
		);
	}

	/// Whether an object of the page is under the given point.
	isHit(point: number[]): boolean {
		return this.hitObject(point) !== undefined;
	}

	onDraw(): void {
		// The page section skips the animation frames too, so the two stay lined up.
		if (this.containerObject.isInZoomAnimation()) return;

		const data = RenderGeometrySection.currentPart();
		if (!data) return;

		// The section container has translated the context to the document anchor, so the scroll
		// offset alone maps the part origin into place, as it does for the page itself.
		Util.ensureValue(app.activeDocument);
		const viewedRectangle = app.activeDocument.activeLayout.viewedRectangle;
		const xDiff = -viewedRectangle.pX1;
		const yDiff = -viewedRectangle.pY1;
		const scale = app.twipsToPixels;

		this.outlineHitUnderMouse(data, xDiff, yDiff);

		// The rectangles and the name of the hit are feedback about the geometry the client holds,
		// so they follow the debug panel's "Vector Overlays" tool.
		if (!app.map._debug.renderGeometryOn) return;

		// The routing of the connectors is held against the engine's once per content, while the
		// overlays are on. What it finds goes to the console.
		if (RenderGeometrySection.routingCheckedAt !== data.version) {
			RenderGeometrySection.routingCheckedAt = data.version;
			ConnectorRouter.checkAgainstTheEngine(data);
		}

		this.context.save();
		this.context.lineWidth = 1;
		// Four fifths see-through: these describe every object at once, so they stay in the
		// background of what the pointer is being told about.
		this.context.globalAlpha = 0.2;

		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object) continue;

			this.context.strokeStyle = RenderGeometrySection.colorOfObject(id);

			if (
				object.x !== undefined &&
				object.y !== undefined &&
				object.width !== undefined &&
				object.height !== undefined
			) {
				// The half pixel offset puts the one pixel wide line on a pixel rather than across
				// two.
				this.context.strokeRect(
					xDiff + Math.round(object.x * scale) + 0.5,
					yDiff + Math.round(object.y * scale) + 0.5,
					Math.round(object.width * scale),
					Math.round(object.height * scale),
				);
			}

			if (object.transform)
				this.strokeUnitRectangle(object.transform, xDiff, yDiff);
		}

		this.nameHitUnderMouse(data, xDiff, yDiff);

		this.context.restore();
	}

	/*
		The color the elements one interacts with are drawn in, which the palette keeps as the
		color of a border under the pointer. It is the light gray the document engine draws
		boundaries and hit geometry in.
	*/
	private static hoverColor(): string {
		const token = getComputedStyle(document.documentElement)
			.getPropertyValue('--color-border-darker')
			.trim();

		return token || '#c0bfbc';
	}

	/*
		Draws around what the pointer found: the geometry the hit test answered with, which for a
		rounded rectangle or a polygon is that object's own outline, and the object's unit rectangle
		through its transformation when the answer carries no geometry.
	*/
	private outlineHitUnderMouse(
		data: cool.VectorPrimitivesData,
		xDiff: number,
		yDiff: number,
	): void {
		const mouse: number[] = this.containerObject.getMousePosition();
		const hit = mouse ? this.hitObject(mouse) : undefined;
		RenderGeometrySection._drawnHitKey = hit
			? String(hit.id) + ':' + hit.hit.kind
			: '';
		if (!hit) return;

		// An object that is selected is marked out by its handles already, so the outline under
		// the pointer is kept for the objects that are not selected.
		if (GraphicSelection.selectedObjectIDs.includes(hit.id)) return;

		const object = data.objects.get(hit.id);
		if (!object?.transform) return;

		const shape =
			hit.hit.path ?? cool.VectorHitTest.pathOfUnitSquare(object.transform);
		if (!shape) return;

		// The geometry is in document twips, the drawing is in this section's pixels.
		const outline = new Path2D();
		outline.addPath(shape, {
			a: app.twipsToPixels,
			b: 0,
			c: 0,
			d: app.twipsToPixels,
			e: xDiff,
			f: yDiff,
		});

		this.context.save();
		this.context.strokeStyle = RenderGeometrySection.hoverColor();
		this.context.lineWidth = 1;
		this.context.setLineDash(RenderGeometrySection.hoverDashes);
		this.context.stroke(outline);
		this.context.restore();
	}

	/*
		Writes what the hit test answers for the object under the pointer above that object's
		rectangle: which geometry of it was found, and whether that geometry is one that is carried
		to be hit rather than seen, one that only an editing view shows, or the boundary of a fill.
		So moving the pointer over a document says whether the client finds an object exactly where
		the object is painted.
	*/
	private nameHitUnderMouse(
		data: cool.VectorPrimitivesData,
		xDiff: number,
		yDiff: number,
	): void {
		const mouse: number[] = this.containerObject.getMousePosition();
		const hit = mouse ? this.hitObject(mouse) : undefined;
		if (!hit) return;

		const object = data.objects.get(hit.id);
		if (!object || object.x === undefined || object.y === undefined) return;

		const flags: string[] = [];
		if (hit.hit.hidden) flags.push('hidden');
		if (hit.hit.editView) flags.push('editview');
		if (hit.hit.edge) flags.push('edge');

		this.context.globalAlpha = 1;
		this.context.fillStyle = RenderGeometrySection.colorOfObject(hit.id);
		this.context.font = '11px sans-serif';
		this.context.fillText(
			hit.hit.kind +
				' id=' +
				String(hit.id) +
				(flags.length ? ' (' + flags.join(', ') + ')' : ''),
			xDiff + Math.round(object.x * app.twipsToPixels) + 2,
			yDiff + Math.round(object.y * app.twipsToPixels) - 3,
		);
	}

	private strokeUnitRectangle(
		transform: number[],
		xDiff: number,
		yDiff: number,
	): void {
		const scale = app.twipsToPixels;
		const corners = RenderGeometrySection.unitRectangleCorners(transform);

		this.context.beginPath();
		corners.forEach(([x, y]: number[], index: number) => {
			const pX = xDiff + Math.round(x * scale) + 0.5;
			const pY = yDiff + Math.round(y * scale) + 0.5;
			if (index === 0) this.context.moveTo(pX, pY);
			else this.context.lineTo(pX, pY);
		});
		this.context.closePath();
		this.context.stroke();
	}

	/*
		The pointer the objects ask for at a document position, in the names the engine sends for
		them: 'text' where the text of an object is what lies under the point, 'move' where the
		object itself does, and 'default' where there is no object. It is the same answer the hit
		test gives, so what the pointer says and what the hit test found are one thing.
	*/
	public static pointerAt(x: number, y: number): string {
		const section = RenderGeometrySection.ofPage();

		if (!section) return 'default';

		const hit = section.hitObjectInDocument(x, y);
		if (!hit) return 'default';

		return hit.hit.kind === 'text' ? 'text' : 'move';
	}

	/// The section holding the objects of the page, while the document is drawn from them.
	private static ofPage(): RenderGeometrySection | undefined {
		return app.sectionContainer?.getSectionWithName(
			app.CSections.RenderGeometry.name,
		) as RenderGeometrySection;
	}

	/*
		The box every object of the page paints into, in twips, by the id of the object. An object
		on a layer the view hides is left out, as is the page itself, a running text edit and the
		page's copy of a master placeholder, none of which are objects to reckon with.
	*/
	public static objectBoxes(): Map<number, number[]> {
		const boxes = new Map<number, number[]>();
		const data = RenderGeometrySection.currentPart();
		if (!data) return boxes;

		for (const id of data.order) {
			const object = data.objects.get(id);
			if (!object || object.kind !== undefined || object.masterContent)
				continue;
			if (
				object.layer !== undefined &&
				!RenderManager.isLayerVisible(object.layer)
			)
				continue;
			if (object.width === undefined || object.height === undefined) continue;

			boxes.set(id, [
				object.x ?? 0,
				object.y ?? 0,
				object.width,
				object.height,
			]);
		}

		return boxes;
	}

	/*
		The group the object sits in, 0 for an object directly on the page, or nothing when the
		page holds no such object. Two objects belong in one selection when this answers the same
		for both of them.
	*/
	public static parentOf(objectId: number): number | undefined {
		const object = RenderGeometrySection.objectOf(objectId);
		return object ? (object.parent ?? 0) : undefined;
	}

	/*
		The outermost object the given one belongs to: the group at the top of the chain of groups
		above it, or the object itself where it is in none. A press on something inside a group
		takes hold of the group, as it does in the office, and only stepping into the group
		reaches what is inside it.

		Standing inside a group stops the climb there, so a press then takes hold of what that
		group holds and not of the group itself. A press on something outside the group one stands
		in climbs the whole way, which is how pressing beside a group steps back out of it.
	*/
	public static outermostOf(objectId: number, standingIn?: number): number {
		let outermost = objectId;

		for (let guard = 0; guard < 100; ++guard) {
			if (outermost === standingIn) break;

			const parent = RenderGeometrySection.parentOf(outermost);
			if (!parent || parent === standingIn) break;

			outermost = parent;
		}

		return outermost;
	}

	/*
		The objects a change to these ones really changes. A group holds no drawing of its own and
		no mapping of its own - it is an organisation of the objects under it, a multi-selection
		someone gave a name to - so a change to a group is a change to each of the objects it
		holds. Anything that is not a group answers with itself.
	*/
	public static theOnesThatDraw(objectIds: number[]): number[] {
		return RenderGeometrySection.descendingInto(objectIds, false);
	}

	/*
		The objects that have a say in what may be done to these ones: a group is asked along with
		everything it holds, so a group holding one object whose place is held fast refuses to be
		moved. A diagram answers on its own. The office holds the place and the size of a
		diagram's members fast as bookkeeping of its own, so that the layout it builds stays the
		one thing that places them, which is not the user saying the diagram may not be moved.
	*/
	public static theOnesThatHaveASay(objectIds: number[]): number[] {
		return objectIds.concat(
			RenderGeometrySection.descendingInto(objectIds, true),
		);
	}

	/// The descent itself. It walks to the objects that draw, and stops at a diagram when asked.
	private static descendingInto(
		objectIds: number[],
		stoppingAtADiagram: boolean,
	): number[] {
		const data = RenderGeometrySection.currentPart();
		if (!data) return objectIds;

		const holds = new Map<number, number[]>();
		for (const id of data.order) {
			const parent = data.objects.get(id)?.parent;
			if (!parent) continue;

			const held = holds.get(parent);
			if (held) held.push(id);
			else holds.set(parent, [id]);
		}

		const drawn: number[] = [];
		const takeIn = (objectId: number, depth: number) => {
			const stops =
				stoppingAtADiagram && data.objects.get(objectId)?.isDiagram === true;
			const held = depth < 100 && !stops ? holds.get(objectId) : undefined;
			if (!held) {
				drawn.push(objectId);
				return;
			}

			for (const one of held) takeIn(one, depth + 1);
		};

		for (const objectId of objectIds) takeIn(objectId, 0);

		return drawn;
	}

	/*
		Everything a change to these objects touches: the objects themselves and all that lies
		under them, the groups in between included. A group carries a mapping and a box of its
		own even though it draws nothing, and a change laid on here has to reach them too, or a
		second change worked out on the group's square before the engine has answered the first
		would be worked out on a stale one.
	*/
	public static theOnesAChangeTouches(objectIds: number[]): number[] {
		const touched: number[] = [];
		const takeIn = (objectId: number, depth: number) => {
			touched.push(objectId);
			if (depth >= 100) return;

			for (const held of RenderGeometrySection.heldBy(objectId))
				takeIn(held, depth + 1);
		};

		for (const objectId of objectIds) takeIn(objectId, 0);

		return touched;
	}

	/// The objects that object holds, in the order the page draws them. Empty for anything that
	/// is not a group.
	public static heldBy(objectId: number): number[] {
		const data = RenderGeometrySection.currentPart();
		if (!data) return [];

		return data.order.filter(
			(id: number) => data.objects.get(id)?.parent === objectId,
		);
	}

	/// Whether the object holds other objects, which is what makes it a group.
	public static holdsObjects(objectId: number): boolean {
		return RenderGeometrySection.heldBy(objectId).length > 0;
	}

	/// The object of the page with the given id, or nothing when the page holds no such object.
	public static objectOf(objectId: number): cool.SlideObject | undefined {
		return RenderGeometrySection.currentPart()?.objects.get(objectId);
	}

	/*
		The object of the page under a document position, in twips, by its unique id, or nothing
		where the point meets no object. The point is asked as it comes, so an answer stands for
		that position and no other.
	*/
	public static objectIdAt(x: number, y: number): number | undefined {
		return RenderGeometrySection.ofPage()?.hitObjectInDocument(x, y)?.id;
	}

	/*
		Whether the objects answer for the mouse pointer here, instead of the engine being asked
		what lies under the mouse. They do whenever the document is drawn from vector primitives
		and one page is on screen, which is when the section is there to ask and knows which page
		the mouse is over. The file based view stacks every page, and the engine answers there.
	*/
	public static answersPointer(): boolean {
		return (
			RenderManager.isVectorRendering() &&
			!app.file.fileBasedView &&
			app.sectionContainer?.doesSectionExist(
				app.CSections.RenderGeometry.name,
			) === true
		);
	}

	/*
		Tells the engine whether it should report the mouse pointer at all. While the objects here
		answer for it, the engine works one out on every hover and would send it for nothing.
	*/
	public static requestPointerFromServer(wanted: boolean): void {
		app.socket.sendMessage(
			'reportmousepointer wanted=' + (wanted ? 'true' : 'false'),
		);
	}

	/// Asks for a redraw, so the rectangles come or go with the debug tool.
	public static update(): void {
		if (app.sectionContainer) app.sectionContainer.requestReDraw();
	}
}
