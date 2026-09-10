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
	debug panel's "Render Geometry" tool is on.
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
			RenderManager.onVectorChanged(() => {
				RenderGeometrySection._lastHit = undefined;
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
	private static currentPart(): cool.VectorPrimitivesData | undefined {
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
		// so they follow the debug panel's "Render Geometry" tool.
		if (!app.map._debug.renderGeometryOn) return;

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
			hit.hit.kind + (flags.length ? ' (' + flags.join(', ') + ')' : ''),
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
		const section = app.sectionContainer?.getSectionWithName(
			app.CSections.RenderGeometry.name,
		) as RenderGeometrySection;

		if (!section) return 'default';

		const hit = section.hitObjectInDocument(x, y);
		if (!hit) return 'default';

		return hit.hit.kind === 'text' ? 'text' : 'move';
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
