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
	One group of handles of a selection, and everything that is its own: which handles it has and
	where they lie, how they are drawn, whether a press reaches one, what a drag of one does and
	what the engine is told when the drag ends.

	A selection holds a list of these in a fixed order, and that one list says three things. The
	keyboard walks it from the first group to the last. The drawing goes the same way, so the
	finest handles lie on top. A press goes the other way, so the finest is reached first and the
	first group that answers takes the press.
*/
abstract class HandleGroup {
	protected selection: SelectionSection;

	constructor(selection: SelectionSection) {
		this.selection = selection;
	}

	/// What the group is called, which is what marks a handle as belonging to it.
	public abstract readonly name: string;

	/// The handles it has, worked out from the objects of the selection.
	public handles(): SelectionHandle[] {
		return [];
	}

	/*
		Where its handles stand while a drag runs, from all of them as the drag leaves them.
		Nothing where they simply travel with the drag, which is what most of them do.
	*/
	public place(_shown: SelectionHandle[]): SelectionHandle[] | null {
		return null;
	}

	/// Anything it draws besides the handles themselves, drawn under them.
	public paint(_section: SelectionSection, _shown: SelectionHandle[]): void {
		// Most groups draw their handles and nothing else.
	}

	/*
		Lays the shape of one of its handles on the canvas and answers the color it is filled
		with. The selection fills it, draws its edge and marks it when the mouse is over it, so
		every handle is marked the same way whatever it looks like.
	*/
	public abstract shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string;

	/// Whether a press that is on none of the handles lands on this group.
	public takesAPress(_x: number, _y: number): boolean {
		return false;
	}

	/// The drag a press on one of its handles begins, or nothing where it begins none.
	public interactionFor(
		_handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		return null;
	}

	/// The drag a press that reached no handle begins, for a group that answers for a place.
	public interactionForThePlace(
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		return null;
	}

	/*
		Whether a press this group took is handed to the engine as a click of its own when it
		turns out not to be a drag. The engine hit tests it and selects what it finds, which is
		how a click through the box of a selection reaches an object behind it.
	*/
	public passesTheClickOn(): boolean {
		return true;
	}

	/*
		What an arrow key does to one of its handles, which is what a drag of it would do. Answers
		where the handle lands, so the view can follow it, and nothing where it goes nowhere.
	*/
	public moveByKey(
		_handle: SelectionHandle,
		_towards: cool.Point,
		_event: KeyboardEvent,
	): cool.Point | null {
		return null;
	}

	/*
		Hands the engine what a drag of one of its handles did, once the drag has ended. What that
		is differs from group to group: a handle the engine knows is named to it, a point of a
		path is named as a point of that path, a point to tie a connector to is moved where it
		was let go.
	*/
	public handOver(_handle: SelectionHandle, _to: cool.Point): void {
		// A group whose handles do not move tells the engine nothing.
	}

	/// Where one of its handles may go, from where the mouse has taken it. Most of them go where
	/// the mouse goes.
	public holdInside(_handle: SelectionHandle, at: cool.Point): cool.Point {
		return at;
	}

	/*
		Hands the engine a handle that was moved, naming the object and the handle. The object
		reads the move its own way - a corner radius, the angle a piece of a circle begins at,
		the offset of a measurement - and nothing about a selection is needed for it.
	*/
	protected sendTheHandle(handle: SelectionHandle, to: cool.Point): void {
		const objectId = this.selection.selectedObjects()[0];
		if (objectId === undefined) return;

		// The name says the kind, the polygon and the point. The number the object gives the
		// handle travels beside them, for the handles that are told apart by nothing else.
		const named = handle.name.split('.');

		app.socket.sendMessage(
			'setobjecthandle id=' +
				String(objectId) +
				' kind=' +
				String(Number(handle.kind)) +
				' polygon=' +
				String(Number(named[1] ?? 0)) +
				' point=' +
				String(Number(named[2] ?? 0)) +
				' at=' +
				String(handle.at ?? -1) +
				' x=' +
				String(Math.round(to.x)) +
				' y=' +
				String(Math.round(to.y)),
		);
	}

	/*
		Moves the handle a key step and hands the engine where it landed, the way the end of a
		drag of it would. Answers where it landed.
	*/
	protected stepAndHandOver(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point {
		const to = HandleGroup.steppedBy(handle, towards, event);
		this.handOver(handle, to);

		return to;
	}

	/// Where a key takes the handle it is on, in twips.
	protected static steppedBy(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point {
		const step = HandleTravel.stepFor(event);

		return new cool.Point(
			handle.point.x + towards.x * step,
			handle.point.y + towards.y * step,
		).round();
	}
}

/*
	The eight that frame the selection, from the upper left one to the lower right one. They are
	what a scale takes hold of, and what everything else about the selection is measured from: its
	middle, the side the handle that turns it stands above, and the corners a press is tested
	against.
*/
class FramingHandles extends HandleGroup {
	public readonly name: string = 'framing';

	public handles(): SelectionHandle[] {
		return (
			ObjectHandles.framingOf(
				this.selection.selectedObjects(),
				this.selection.squareNow(),
			) ?? []
		);
	}

	/*
		A key on one of the eight scales the selection as a drag of that handle would, and hands
		the engine what a drag hands it: the mapping every object is to be drawn by. The drag
		works that mapping out, so the key borrows the drag rather than saying it again.
	*/
	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		const shape = this.selection.shapeNow();
		if (!shape) return null;

		const to = HandleGroup.steppedBy(handle, towards, event);

		// A key moves by exactly what it says: the drag it makes lines up with nothing on the
		// page and keeps no ratio, since Shift on a key is the large step and not the ratio.
		const scaling = new ScalingInteraction(
			this.selection,
			handle,
			shape,
		).exactlyAsTold();

		scaling.finish(
			new cool.SimplePoint(to.x, to.y),
			event as unknown as MouseEvent,
		);

		const change = scaling.transformation();
		this.selection.layTheChangeOn(change);

		/*
			The handle keeps its name through a change that takes a side past the one opposite
			it. Each of the eight sits at its own corner of the object's square, so the one that
			is named the upper left is drawn wherever the mapping puts that corner, and a mapping
			that turns the object over puts it on the other side by itself. The key that follows
			goes on stretching the same way.

			What is answered is where the handle really landed, and not where the key asked it
			to go. The two part company where the object is held to a hair rather than made
			thinner still: the handle then stops a hair short of the asked place, and the edge
			facing it, which never moved, can lie nearer to that place than the handle does.
			The place is taken through the change itself, which is a change of the page: no
			object's mapping is asked, so a group, whose own mapping the change is never laid
			on, answers as truly as one object does.
		*/
		if (!change) return to;

		return change.apply(handle.point.x, handle.point.y).round();
	}

	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		return HandleLook.lay(context, 'framing', at, grown, handle.turnedBy);
	}

	public interactionFor(
		handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		if (!ObjectHandles.canBeResized(this.selection.selectedObjects()))
			return null;

		const shape = this.selection.shapeNow();

		return shape ? new ScalingInteraction(this.selection, handle, shape) : null;
	}
}

/*
	The marks a picture is cropped by: an L at each corner and a bar along each side, standing on
	the picture's own square where the eight stand, but on the edge itself rather than a little
	outside it. They are laid in the picture's own axes, so they turn and shear with it and lie
	along its edges whatever it stands like. The eight are what scales the picture; these are
	what crops it.
*/
class CropHandles extends HandleGroup {
	public readonly name: string = 'crop';

	/// The picture the selection is, or nothing where it is not one picture.
	private thePicture(): cool.SlideObject | undefined {
		const objects = this.selection.selectedObjects();
		if (objects.length !== 1) return undefined;

		const object = RenderGeometrySection.objectOf(objects[0]);
		return object?.graphicWidth !== undefined ? object : undefined;
	}

	/// Whether the marks are shown at all: the crop switch is on and one picture is selected.
	private shown(): boolean {
		return (
			GraphicSelection.cropping &&
			this.thePicture() !== undefined &&
			ObjectHandles.canBeResized(this.selection.selectedObjects())
		);
	}

	public handles(): SelectionHandle[] {
		if (!this.shown()) return [];

		const square = this.selection.squareNow();
		const eight =
			ObjectHandles.framingOf(this.selection.selectedObjects(), square) ?? [];
		if (!square) return [];

		const [across, down] = CropHandles.axesOf(square);

		/*
			A mark runs from its place into the picture, so its box is put where the mark is,
			half a mark in along each axis it runs along; the mark itself is still laid from the
			place, which is why the drawing takes the same half back.
		*/
		const marks: SelectionHandle[] = eight.map((one: SelectionHandle) => {
			const inward = CropHandles.inwardOf(one.kind);

			return {
				name: 'crop' + one.kind,
				kind: 'crop' + one.kind,
				pointer: one.pointer,
				point: one.point.clone(),
				turnedBy: one.turnedBy,
				awayFromItsPlace: across
					.multiplyBy(inward.x)
					.add(down.multiplyBy(inward.y)),
				awayBy: HandleLook.widthOf('crop') * 0.5,
			};
		});

		// The handle in the middle takes the window, the visible part, over the graphic.
		marks.push({
			name: CropHandles.middleKind,
			kind: CropHandles.middleKind,
			pointer: 'move',
			point: square.apply(0.5, 0.5),
			turnedBy: Math.atan2(across.y, across.x),
		});

		return marks;
	}

	/// The kind of the handle in the middle of the picture.
	public static readonly middleKind: string = 'cropMove';

	/*
		Tells the engine to crop the picture to the window that square maps the unit square to,
		by two opposite corners of the window on the page, in twips. The window is named whole,
		whatever part of it moved.
	*/
	public static cropTo(objectId: number, window: cool.Matrix2D): void {
		const upperLeft = window.apply(0, 0);
		const lowerRight = window.apply(1, 1);

		app.socket.sendMessage(
			'cropobject id=' +
				String(objectId) +
				' x1=' +
				String(Math.round(upperLeft.x)) +
				' y1=' +
				String(Math.round(upperLeft.y)) +
				' x2=' +
				String(Math.round(lowerRight.x)) +
				' y2=' +
				String(Math.round(lowerRight.y)),
		);
	}

	/// The two axes of that square, each a step of length one.
	private static axesOf(square: cool.Matrix2D): cool.Point[] {
		const unit = (axis: cool.Point): cool.Point =>
			axis.divideBy(axis.length() || 1);

		return [
			unit(new cool.Point(square.a, square.b)),
			unit(new cool.Point(square.c, square.d)),
		];
	}

	/// Which way is into the picture from the place of that one of the eight, along each axis;
	/// nought along an axis the place lies in the middle of.
	private static inwardOf(kind: string): cool.Point {
		const place = ObjectHandles.placeOfKind(kind) ?? new cool.Point(0.5, 0.5);

		return new cool.Point(
			place.x < 0.5 ? 1 : place.x > 0.5 ? -1 : 0,
			place.y < 0.5 ? 1 : place.y > 0.5 ? -1 : 0,
		);
	}

	public interactionFor(
		handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		if (handle.kind === CropHandles.middleKind)
			return new CropPanInteraction(this.selection, handle);

		const shape = this.selection.shapeNow();

		return shape ? new CropInteraction(this.selection, handle, shape) : null;
	}

	/*
		A key crops as a drag of the mark to the stepped place would. The engine is told at once,
		and what the step did is kept here as a drag's end keeps it, so the next step of a key
		held down goes on from where this one left the handle.
	*/
	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		const to = HandleGroup.steppedBy(handle, towards, event);
		const place = new cool.SimplePoint(to.x, to.y);

		let crop: SelectionInteraction;
		if (handle.kind === CropHandles.middleKind) {
			const pan = new CropPanInteraction(this.selection, handle);
			pan.finish(place);
			crop = pan;
		} else {
			const shape = this.selection.shapeNow();
			if (!shape) return null;

			const mark = new CropInteraction(this.selection, handle, shape);
			mark.exactlyAsTold().finish(place, event as unknown as MouseEvent);
			crop = mark;
		}

		this.selection.keepWhatItDid(crop);
		return to;
	}

	/*
		Where the whole graphic stands on the picture's square, as the corner it begins at and
		the corner it ends at: the visible window is the graphic less the crop, so the whole
		graphic runs from minus the crop before the window to one plus the crop after it. The
		crop belongs to the graphic before it is turned over, so on a picture that is drawn
		turned over the two sides of that axis change places. Nothing where the picture shows
		no graphic or is cropped to nothing.
	*/
	public static graphicExtentOf(
		picture: cool.SlideObject,
	): cool.Range2D | undefined {
		if (!picture.graphicWidth || !picture.graphicHeight) return undefined;

		const mirror = picture.graphicMirror ?? 0;
		const left = (mirror & 1 ? picture.cropRight : picture.cropLeft) ?? 0;
		const right = (mirror & 1 ? picture.cropLeft : picture.cropRight) ?? 0;
		const top = (mirror & 2 ? picture.cropBottom : picture.cropTop) ?? 0;
		const bottom = (mirror & 2 ? picture.cropTop : picture.cropBottom) ?? 0;

		const visibleWidth = picture.graphicWidth - left - right;
		const visibleHeight = picture.graphicHeight - top - bottom;
		if (visibleWidth <= 0 || visibleHeight <= 0) return undefined;

		return new cool.Range2D(
			-left / visibleWidth,
			-top / visibleHeight,
			1 + right / visibleWidth,
			1 + bottom / visibleHeight,
		);
	}

	/// The four corners of the unit square through that mapping, as an SVG path, in twips.
	private static quadOf(mapping: cool.Matrix2D): string {
		return (
			[
				[0, 0],
				[1, 0],
				[1, 1],
				[0, 1],
			]
				.map(([x, y]: number[], at: number) => {
					const corner = mapping.apply(x, y);
					return (at === 0 ? 'M' : 'L') + corner.x + ' ' + corner.y;
				})
				.join(' ') + ' Z'
		);
	}

	/*
		Draws the whole graphic faintly round the cropped picture, as the office does while one
		crops: what the crop took away is shown at a fifth, with a line round the whole, so one
		sees what a crop could bring back. The visible window is left out of the drawing, since
		the picture itself is there. The graphic is drawn from the image the picture's own
		primitives are drawn from, by its checksum, so nothing is fetched for it.
	*/
	public paint(section: SelectionSection, _shown: SelectionHandle[]): void {
		if (!this.shown()) return;

		const picture = this.thePicture();
		const square = section.squareNow();
		const data = RenderGeometrySection.currentPart();
		if (!picture || !square || !data) return;
		if (picture.graphicChecksum === undefined) return;

		const extent = CropHandles.graphicExtentOf(picture);
		if (!extent) return;

		// An uncropped picture has nothing to show round it, until a crop moves its window.
		const running = section.runningChange();
		if (
			!extent.minX &&
			!extent.minY &&
			extent.maxX === 1 &&
			extent.maxY === 1 &&
			!running?.cropOnly
		)
			return;

		let whole = cool.Matrix2D.fromRange(extent).then(square);
		let window = square;

		// While a drag runs the picture is shown as it would be: a scale takes the whole graphic
		// and the window with it, a crop moves the window over a graphic that stays.
		if (running) {
			window = window.then(running.matrix);
			if (!running.cropOnly) whole = whole.then(running.matrix);
		}

		const preview = {
			type: cool.UnifiedTransparencePrimitive.type,
			transparence: 0.8,
			children: [
				{
					type: cool.MaskPrimitive.type,
					clip: CropHandles.quadOf(whole) + ' ' + CropHandles.quadOf(window),
					children: [
						{
							type: cool.GraphicPrimitive.type,
							checksum: picture.graphicChecksum,
							matrix: whole.toArray(),
							mirror: picture.graphicMirror,
						},
						{
							type: cool.PolygonHairlinePrimitive.type,
							path: CropHandles.quadOf(whole),
							color: HandleLook.outlineOfADrag,
						},
					],
				},
			],
		} as unknown as cool.Primitive;

		const context = section.context;
		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);

		// The point at the very start of the document says where the drawing of the part begins
		// on the canvas, and a twip is that many pixels wide.
		const origin = new cool.SimplePoint(0, 0);
		context.translate(origin.vX, origin.vY);
		context.scale(app.twipsToPixels, app.twipsToPixels);

		RenderManager.renderPrimitives(context, data, [preview]);

		/*
			While a crop runs, the picture is drawn here in place of the page's own drawing of it,
			as the crop leaves it: the picture as it is inside the new window, the whole graphic
			where the new window reaches beyond the old one, and what the new window leaves
			outside of the old one half see-through, so that what is taken away fades and what
			lies under it stays as it is.
		*/
		if (running?.cropOnly && picture.id !== undefined) {
			const everything = CropHandles.quadBeyondEverything();
			const identity = cool.Matrix2D.IDENTITY;

			context.save();
			context.clip(new Path2D(CropHandles.quadOf(window)));
			RenderManager.renderObjectsWith(context, data, [picture.id], identity);
			context.restore();

			RenderManager.renderPrimitives(context, data, [
				{
					type: cool.MaskPrimitive.type,
					clip: CropHandles.quadOf(window),
					children: [
						{
							// Outside the old window: the second mask takes everything but it.
							type: cool.MaskPrimitive.type,
							clip: everything + ' ' + CropHandles.quadOf(square),
							children: [
								{
									type: cool.GraphicPrimitive.type,
									checksum: picture.graphicChecksum,
									matrix: whole.toArray(),
									mirror: picture.graphicMirror,
								},
							],
						},
					],
				} as unknown as cool.Primitive,
			]);

			context.save();
			context.clip(new Path2D(CropHandles.quadOf(square)));
			context.clip(
				new Path2D(everything + ' ' + CropHandles.quadOf(window)),
				'evenodd',
			);
			context.globalAlpha = 0.5;
			RenderManager.renderObjectsWith(context, data, [picture.id], identity);
			context.restore();
		}

		context.restore();
	}

	/// A quad far larger than any page, for a mask that is to take everything but its other path.
	private static quadBeyondEverything(): string {
		const far = 10000000;
		return (
			'M' +
			-far +
			' ' +
			-far +
			' L' +
			far +
			' ' +
			-far +
			' L' +
			far +
			' ' +
			far +
			' L' +
			-far +
			' ' +
			far +
			' Z'
		);
	}

	/*
		Lays the mark of that handle into the path: at a corner an L whose arms run along the two
		edges meeting there, into the picture; along a side a bar lying on the edge, inside it. The
		mark is laid in the picture's own axes, taken from the selection's square, so it turns and
		shears with the picture.
	*/
	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		const color = HandleLook.blended(
			HandleLook.colourOfTheKind.framing,
			-HandleLook.cropMarkShade,
		);
		const place = ObjectHandles.placeOfKind(handle.kind.substring(4));
		const square = this.selection.squareNow();
		if (!square) return color;

		const [across, down] = CropHandles.axesOf(square);

		/*
			The handle in the middle is as wide as the mark on the top side is long, and as high
			as the mark on the left side is long, laid in the picture's axes and standing on its
			point. Three lines across it, kept off its sides, say that it is taken hold of to
			move; the edge is drawn closed here, so the lines are drawn as lines alone.
		*/
		if (handle.kind === CropHandles.middleKind) {
			const long = HandleLook.widthOf('crop') + 2 * grown;
			context.save();
			context.setTransform(across.x, across.y, down.x, down.y, at.vX, at.vY);
			context.moveTo(-long * 0.5, -long * 0.5);
			context.lineTo(long * 0.5, -long * 0.5);
			context.lineTo(long * 0.5, long * 0.5);
			context.lineTo(-long * 0.5, long * 0.5);
			context.closePath();
			for (const y of [-long * 0.25, 0, long * 0.25]) {
				context.moveTo(-long * 0.3, y);
				context.lineTo(long * 0.3, y);
			}
			context.restore();
			return color;
		}

		if (!place) return color;
		const inward = CropHandles.inwardOf(handle.kind.substring(4));
		const inwardAcross = inward.x;
		const inwardDown = inward.y;

		const long = HandleLook.widthOf('crop') + 2 * grown;
		const thick = long * 0.5;

		// The point handed in is where the box is, half a mark in. The mark is laid from the
		// place on the edge, that half back out.
		const back = HandleLook.widthOf('crop') * 0.5;

		context.save();

		if (inwardAcross && inwardDown) {
			// A corner: x runs along the top or bottom edge into the picture, y along the side.
			context.setTransform(
				across.x * inwardAcross,
				across.y * inwardAcross,
				down.x * inwardDown,
				down.y * inwardDown,
				at.vX,
				at.vY,
			);
			context.moveTo(-back, -back);
			context.lineTo(long - back, -back);
			context.lineTo(long - back, thick - back);
			context.lineTo(thick - back, thick - back);
			context.lineTo(thick - back, long - back);
			context.lineTo(-back, long - back);
		} else {
			// A side: x runs along the edge, y into the picture.
			const along = inwardDown ? across : down;
			const into = inwardDown
				? down.multiplyBy(inwardDown)
				: across.multiplyBy(inwardAcross);
			context.setTransform(along.x, along.y, into.x, into.y, at.vX, at.vY);
			context.moveTo(-long * 0.5, -back);
			context.lineTo(long * 0.5, -back);
			context.lineTo(long * 0.5, thick - back);
			context.lineTo(-long * 0.5, thick - back);
		}

		context.restore();

		return color;
	}
}

/*
	The one handle that turns the selection, above the middle of its upper side and away from it
	by the same distance whatever the zoom. The client makes it: the engine has no handle of the
	kind, and what it is told at the end of a turn is a transformation.
*/
class TurningHandle extends HandleGroup {
	public readonly name: string = 'turning';

	/// How far above the selection it sits, in core pixels.
	public static readonly distance: number = 30;

	public handles(): SelectionHandle[] {
		return this.placedAbove(
			ObjectHandles.framingOf(
				this.selection.selectedObjects(),
				this.selection.squareNow(),
			) ?? [],
		);
	}

	/*
		It is laid out again from the handles as a drag leaves them, so that it keeps its distance
		from the upper side however the selection is scaled or turned.
	*/
	public place(shown: SelectionHandle[]): SelectionHandle[] | null {
		return this.placedAbove(shown);
	}

	/*
		The handle above the upper side of what those handles frame, or none where they do not
		frame anything. It stands out from the middle of that side at a right angle to it, so a
		slanted selection does not slant the way out to it. Where the objects say they cannot be
		turned it is there but dead, so the selection looks the same whatever may be done to it.
	*/
	private placedAbove(shown: SelectionHandle[]): SelectionHandle[] {
		const middle = SelectionSection.middleOf(shown);
		const above = shown.find((one: SelectionHandle) => one.kind === '2');
		if (!middle || !above) return [];

		const dead = !ObjectHandles.canBeTurned(this.selection.selectedObjects());

		const away = TurningHandle.distance * app.dpiScale;
		const out = new cool.Point(
			above.point.x - middle.x,
			above.point.y - middle.y,
		);
		const length = out.length();
		if (!length) return [];

		const step = cool.SimplePoint.fromCorePixels([away, away]);

		// The upper side's own top, which faces the way out, stands at a right angle to it.
		const turnedBy = TurningHandle.turnOf(shown, out);
		const up = new cool.Point(Math.sin(turnedBy), -Math.cos(turnedBy));

		const point = new cool.Point(
			above.point.x + up.x * step.x,
			above.point.y + up.y * step.y,
		);
		const handles: SelectionHandle[] = [
			{
				name: SelectionSection.turningKind,
				kind: SelectionSection.turningKind,
				pointer: '0',
				point: point.round(),
				turnedBy: turnedBy,
				deactivated: dead,
			},
		];

		/*
			While the mouse is on it, the handle of the other mode stands beside it along the
			upper side: to the right of the turning handle, to the left of the shearing one. Both
			are drawn as large as a handle under the mouse, with a handle between them, and a bar
			lies behind the two.
		*/
		if (this.selection.showsTheOtherMode() && !dead)
			handles.push({
				name: SelectionSection.otherModeKind,
				kind: SelectionSection.otherModeKind,
				pointer: '0',
				point: TurningHandle.placeOfTheOtherMode(
					point,
					turnedBy,
					this.selection.handleMode() === 'shear' ? -1 : 1,
				),
				turnedBy: turnedBy,
			});

		return handles;
	}

	/// Where the handle of the other mode stands beside the handle above the selection at that
	/// point, in twips, turned by that angle: on its right for a side of 1, on its left for -1.
	public static placeOfTheOtherMode(
		point: cool.Point,
		turnedBy: number,
		side: number,
	): cool.Point {
		const apart = 2.25 * ShapeHandlesSection.handleSize();
		const across = cool.SimplePoint.fromCorePixels([apart, apart]);
		return new cool.Point(
			point.x + side * Math.cos(turnedBy) * across.x,
			point.y + side * Math.sin(turnedBy) * across.y,
		).round();
	}

	/*
		The angle the handle is drawn turned by: that of the selection's upper side, from its left
		corner to its right one, which a slant does not change, so a slanted selection turns the
		symbol only by its own turn. On a mirrored selection that side runs the other way, and
		the symbol is turned over again, so its top faces away from the selection as the handle
		does. Where the corners are not known it is the way up to the handle that says it.
	*/
	private static turnOf(shown: SelectionHandle[], out: cool.Point): number {
		const left = shown.find((one: SelectionHandle) => one.kind === '1');
		const right = shown.find((one: SelectionHandle) => one.kind === '3');
		if (!left || !right) return Math.atan2(out.y, out.x) + Math.PI / 2;

		const across = right.point.subtract(left.point);
		const turn = Math.atan2(across.y, across.x);

		// The symbol's top points that way at this turn; it has to point the way the handle is.
		const top = new cool.Point(Math.sin(turn), -Math.cos(turn));
		return top.dot(out) < 0 ? turn + Math.PI : turn;
	}

	/// The handle shows the mode it is in, and the one beside it the other mode: the circle for
	/// turning, the slanted rectangle for shearing.
	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		const shearing = this.selection.handleMode() === 'shear';
		const other = handle.kind === SelectionSection.otherModeKind;
		return shearing !== other
			? HandleLook.layShearing(context, at, grown, handle.turnedBy)
			: HandleLook.lay(context, 'turning', at, grown);
	}

	/// A drag turns the selection in the turning mode and shears it in the shearing mode, around
	/// its middle either way.
	public interactionFor(
		handle: SelectionHandle,
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		const middle = this.selection.middle();
		if (!middle) return null;

		return this.selection.handleMode() === 'shear'
			? new ShearingInteraction(
					this.selection,
					middle,
					at,
					handle.turnedBy ?? 0,
				)
			: new TurningInteraction(this.selection, middle, at);
	}

	/*
		How far a key turns the selection, in degrees against the clock for a way of 1 and with it
		for -1: on to the next step of the angle the selection stands at, counted from the page, so
		a few presses bring a turned object back to standing upright. The step is the one a turn
		by the mouse snaps to for the modifier held (EditSteps.turnSnap), and with no modifier the
		smallest one (EditSteps.keyTurnStep).
	*/
	private keyTurn(event: KeyboardEvent, way: number): number {
		const step = EditSteps.turnSnap(event) ?? EditSteps.keyTurnStep();
		const stands = this.standsNow();
		return EditSteps.nextStep(stands, step, way) - stands;
	}

	/// The angle the selection stands at, in degrees against the clock as the engine counts it.
	private standsNow(): number {
		return (-this.selection.turnOfTheSelection() * 180) / Math.PI;
	}

	/*
		Turns the selection by a key by that many degrees against the clock, around the middle
		the turn keys turned it around before where they did, and keeps the angle and the middle
		for the next key.
	*/
	private turnByKey(degrees: number): cool.Matrix2D | null {
		const byKeys = TurningInteraction.keysStillHold(this.selection);
		const middle = byKeys?.middle ?? this.selection.middle();
		if (!middle) return null;

		const stands = this.standsNow();
		const turned = TurningInteraction.turnObjects(
			this.selection,
			middle,
			degrees,
		);
		TurningInteraction.keysLeftItAt(this.selection, stands + degrees, middle);
		return turned;
	}

	/*
		The keys that point left and up turn or slant against the clock, the others with it. A
		slant goes on to the next step of the slant there is, counted from nought, of the size a
		turn takes for the same modifier.
	*/
	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		const middle = this.selection.middle();
		if (!middle) return null;

		const against = towards.x < 0 || towards.y < 0;
		const changed =
			this.selection.handleMode() === 'shear'
				? ShearingInteraction.shearObjects(
						this.selection,
						middle,
						handle.turnedBy ?? 0,
						(stands: number) =>
							EditSteps.nextStep(
								stands,
								EditSteps.turnSnap(event) ?? EditSteps.keyTurnStep(),
								against ? -1 : 1,
							),
					)
				: this.turnByKey(this.keyTurn(event, against ? 1 : -1));

		// What the key did is laid on the objects, as the end of a drag lays its own.
		this.selection.layTheChangeOn(changed);

		return null;
	}

	/*
		Brings the selection back to nought in the mode the handle is in: standing upright in the
		turning mode, the shorter way round, and without a slant in the shearing mode.
	*/
	public backToNought(handle: SelectionHandle): void {
		const middle = this.selection.middle();
		if (!middle) return;

		let changed: cool.Matrix2D | null;
		if (this.selection.handleMode() === 'shear') {
			changed = ShearingInteraction.shearObjects(
				this.selection,
				middle,
				handle.turnedBy ?? 0,
				() => 0,
			);
		} else {
			if (handle.deactivated) return;

			// The angle the selection stands at is turned away the shorter way.
			const stands = this.standsNow() % 360;
			const back =
				stands > 180 ? 360 - stands : stands < -180 ? -360 - stands : -stands;
			changed = this.turnByKey(back);
		}

		this.selection.layTheChangeOn(changed);
	}
}

/*
	A group whose handles are each dragged on their own, to where the mouse takes them, and moved
	by a key the same way. Where a handle ends up is handed to the engine.
*/
abstract class HandlesMovedOneByOne extends HandleGroup {
	public interactionFor(
		handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		return new ShapingInteraction(this.selection, handle);
	}

	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		return this.stepAndHandOver(handle, towards, event);
	}
}

/*
	The handles that shape one object rather than frame it: the corner radius of a rectangle, the
	points a custom shape is shaped by, the points along the way of a connector. What such a
	handle does to the object is the object's own business, so the engine is told which handle
	moved and where it went.
*/
class ShapingHandles extends HandlesMovedOneByOne {
	public readonly name: string = 'shaping';

	/// The kind the engine gives a point a custom shape is shaped by.
	private static readonly shapingPointKind: string = '22';

	/*
		A point a custom shape is shaped by is moved by naming the shape and the point: the shape
		reads it as the value it shapes itself from, and nothing about a selection or a handle of
		a view is needed for that. The others - the corner radius of a rectangle, the points along
		a connector's way - are still named as handles, since what they mean lives in rules of
		their own that nothing here says yet.
	*/
	public handOver(handle: SelectionHandle, to: cool.Point): void {
		if (handle.kind !== ShapingHandles.shapingPointKind) {
			this.sendTheHandle(handle, to);
			return;
		}

		const at = Number(handle.name.split('.')[2]);
		if (!(at >= 0)) return;

		app.socket.sendMessage(
			'setobjectcontrolpoint id=' +
				String(this.selection.selectedObjects()[0]) +
				' at=' +
				String(at) +
				' x=' +
				String(Math.round(to.x)) +
				' y=' +
				String(Math.round(to.y)) +
				// The tail of a callout carries the shape along, which a drag says by having no
				// modifier held down. Nothing here holds one yet.
				' withtheshape=1',
		);
	}

	public handles(): SelectionHandle[] {
		return ObjectHandles.shapingOf(this.selection.selectedObjects());
	}

	/*
		A point a custom shape is shaped by runs on rails the shape gives it: across the shape,
		down it, both, or around a point of it, and between the places the shape names. The
		places are given on the square of the object, so the place the mouse asks for is taken
		onto that square, held there, and taken back onto the page. A shape that is turned,
		sheared, mirrored or stretched more one way than the other is held right that way, and
		the places stay right while the object is dragged, since they move with it.
	*/
	public holdInside(handle: SelectionHandle, at: cool.Point): cool.Point {
		// A handle that moves a line of a connector moves across that line only.
		const line = this.selection.howTheLineHandleMoves(handle);
		if (line)
			return line.movesAcross
				? new cool.Point(at.x, handle.point.y)
				: new cool.Point(handle.point.x, at.y);

		const rails = handle.rails;
		if (!rails) return at;

		const transform = RenderGeometrySection.transformOf(
			this.selection.selectedObjects()[0],
		);
		if (!transform) return at;

		const onTheSquare = transform.invert();
		if (!onTheSquare) return at;

		const asked = onTheSquare.apply(at.x, at.y);
		const was = onTheSquare.apply(handle.point.x, handle.point.y);

		const held = rails.movesAround
			? ShapingHandles.heldAround(rails, asked, was)
			: ShapingHandles.heldOnTheAxes(rails, asked, was);

		return transform.apply(held.x, held.y).round();
	}

	/*
		Where on the square of the object a handle that moves on the axes may go. An axis it does
		not move on keeps the place it had. On an axis it does move on, each place the shape names
		blocks the side it lies on, which is the side away from where the handle stands: the shape
		says which end of its own value a place comes from, and that end can be either side of the
		shape.
	*/
	private static heldOnTheAxes(
		rails: cool.ObjectHandleRails,
		asked: cool.Point,
		was: cool.Point,
	): cool.Point {
		return new cool.Point(
			rails.movesAcross
				? ShapingHandles.between(asked.x, was.x, [
						rails.leastAcross?.x,
						rails.mostAcross?.x,
					])
				: was.x,
			rails.movesDown
				? ShapingHandles.between(asked.y, was.y, [
						rails.leastDown?.y,
						rails.mostDown?.y,
					])
				: was.y,
		);
	}

	/*
		Where on the square of the object a handle that turns about a point may go: anywhere
		around that point, no nearer and no further than the shape allows. A shape that names no
		point of its own turns about its middle.
	*/
	private static heldAround(
		rails: cool.ObjectHandleRails,
		asked: cool.Point,
		was: cool.Point,
	): cool.Point {
		const around = rails.around
			? new cool.Point(rails.around.x, rails.around.y)
			: new cool.Point(0.5, 0.5);
		const awayFrom = (place?: cool.ObjectPlace): number | undefined =>
			place ? new cool.Point(place.x, place.y).distanceTo(around) : undefined;

		const away = asked.distanceTo(around);
		const kept = ShapingHandles.between(away, was.distanceTo(around), [
			awayFrom(rails.nearest),
			awayFrom(rails.furthest),
		]);
		if (!away || kept === away) return asked;

		return around.add(asked.subtract(around).divideBy(away).multiplyBy(kept));
	}

	/*
		A value held between the places the shape names. Where it names both, the value stays
		between them. Where it names one, that one blocks the side it lies on, which is told from
		the place the handle stands at now: the handle stands inside its own rails, so the side
		the place lies on is the side that is closed.
	*/
	private static between(
		value: number,
		stands: number,
		places: (number | undefined)[],
	): number {
		const given = places.filter(
			(place): place is number => place !== undefined,
		);
		if (given.length === 2)
			return Math.min(Math.max(value, Math.min(...given)), Math.max(...given));

		let held = value;
		for (const place of given)
			held = place <= stands ? Math.max(held, place) : Math.min(held, place);

		return held;
	}

	public shape(
		context: CanvasRenderingContext2D,
		_handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		return HandleLook.lay(context, 'shaping', at, grown);
	}

	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		const to = this.holdInside(
			handle,
			HandleGroup.steppedBy(handle, towards, event),
		);
		this.handOver(handle, to);

		return to;
	}
}

/*
	The points someone added to an object for a connector to tie itself to, and, while an end of a
	connector is dragged onto an object, every place that end could land. They are walked to and
	marked; moving one is not built yet.
*/
class TiePointHandles extends HandleGroup {
	public readonly name: string = 'tiePoints';

	/// The points the selected object was given, which travel with it as it is dragged.
	public handles(): SelectionHandle[] {
		const objects = this.selection.selectedObjects();

		return objects.length === 1 ? GluePointHandles.ownPointsOf(objects[0]) : [];
	}

	/*
		The object's own points as the drag leaves them, and beside them every place an end of a
		connector could land on the object under the mouse. Those belong to that object, so they
		are worked out where it stands rather than carried along by the drag.
	*/
	public place(shown: SelectionHandle[]): SelectionHandle[] | null {
		const own = this.ownAsTheDragLeavesThem(shown);

		const aimedAt = this.selection.aimedAtObject();
		if (aimedAt === null) return own;

		// The point the dragged end would tie itself to is lit, so the drag says where it lands.
		const led = this.selection.leadingPointOfTheDrag();
		const would = led
			? this.selection.whereTheEndWouldTie(new cool.Point(led.x, led.y))
			: null;
		const lit = (handle: SelectionHandle): SelectionHandle =>
			would?.point?.equals(handle.point) ? { ...handle, lit: true } : handle;

		return [
			...own,
			...GluePointHandles.ownPointsOf(aimedAt).map(lit),
			...GluePointHandles.defaultPointsOf(aimedAt).map(lit),
		];
	}

	/*
		The object's own points while a drag of the object runs.

		Such a point is not held on the object's own square but on the box around it, and that is
		what has to be worked out again: the drawing layer reads it from the middle of that box,
		or from one of its sides where the point names one. A point held as a share of the size
		keeps its share of the box, and one that keeps a distance of its own keeps that distance
		in twips. Either way the point ends up inside the box.

		Carrying the point through the change instead would hold it to the shape, and for a
		turned object the two part company, because turning the box is not the same as turning
		the shape inside it.
	*/
	private ownAsTheDragLeavesThem(shown: SelectionHandle[]): SelectionHandle[] {
		const own = shown.filter(
			(handle: SelectionHandle) => handle.group === this.name,
		);

		const matrix = this.selection.dragMatrix();
		const objects = this.selection.selectedObjects();
		if (!matrix || objects.length !== 1) return own;

		const transform = RenderGeometrySection.transformOf(objects[0]);
		const held = RenderGeometrySection.objectOf(objects[0])?.gluePoints ?? [];
		if (!transform || !held.length) return own;

		/*
			A change that leaves the object the size it was - a move, or a turn - carries the
			points along with it and nothing more: the drawing layer takes them off the box for
			as long as such a change runs, moves them with the object and measures them against
			the box again afterwards. It is only a change of size that they follow the box
			through, and that is what is worked out below.
		*/
		const grewAcross = matrix.lengthOfXAxis();
		const grewDown = matrix.lengthOfYAxis();
		if (Math.abs(grewAcross - 1) < 0.0001 && Math.abs(grewDown - 1) < 0.0001)
			return own;

		const was = TiePointHandles.theBoxAround(transform);
		const becomes = TiePointHandles.theBoxAround(transform.then(matrix));
		const standing = GluePointHandles.ownPointsOf(objects[0]);

		return own.map((handle: SelectionHandle, at: number) => {
			const point = held[at];
			// A point the shape makes from its geometry goes with the shape's mapping.
			if (!point || !standing[at] || point.fromTheShape) return handle;

			return {
				...handle,
				point: TiePointHandles.laidOnTheBox(
					point,
					standing[at].point,
					was,
					becomes,
				),
			};
		});
	}

	/// The box around what the mapping draws, which is the box the drawing layer holds a point
	/// to tie a connector to against.
	private static theBoxAround(transform: cool.Matrix2D): cool.Range2D {
		return cool.Range2D.fromPoints(
			[
				[0, 0],
				[1, 0],
				[1, 1],
				[0, 1],
			].map(([x, y]: number[]) => transform.apply(x, y)),
		);
	}

	/*
		Where a point stands once the box around the object has become the other one: the side it
		is measured from moves with the box, and what it keeps from that side is either its share
		of the size or its own distance. A point that would end up outside the box is brought
		back to the edge, as the drawing layer brings it back.
	*/
	private static laidOnTheBox(
		point: cool.ObjectGluePoint,
		standing: cool.Point,
		was: cool.Range2D,
		becomes: cool.Range2D,
	): cool.Point {
		const acrossFrom = (box: cool.Range2D): number =>
			point.fromAcross === 'left'
				? box.minX
				: point.fromAcross === 'right'
					? box.maxX
					: box.centerX;
		const downFrom = (box: cool.Range2D): number =>
			point.fromDown === 'top'
				? box.minY
				: point.fromDown === 'bottom'
					? box.maxY
					: box.centerY;

		// A point held as a share of the size grows with the box. One that keeps a distance of
		// its own is left as far from its side as it was.
		const grewAcross =
			point.keepsItsDistance || !was.width ? 1 : becomes.width / was.width;
		const grewDown =
			point.keepsItsDistance || !was.height ? 1 : becomes.height / was.height;

		const x = acrossFrom(becomes) + (standing.x - acrossFrom(was)) * grewAcross;
		const y = downFrom(becomes) + (standing.y - downFrom(was)) * grewDown;

		return becomes.clamp(new cool.Point(x, y)).round();
	}

	/*
		A point to tie a connector to stays on the object it belongs to: the drawing layer reads
		such a point off the box around the object and holds it inside that box, so a drag that
		would take it outside leaves it on the edge.
	*/
	public holdInside(_handle: SelectionHandle, at: cool.Point): cool.Point {
		const transform = RenderGeometrySection.transformOf(
			this.selection.selectedObjects()[0],
		);
		if (!transform) return at;

		const onTheObject = transform.invert()?.apply(at.x, at.y);
		if (!onTheObject) return at;

		const held = (value: number) => Math.min(Math.max(value, 0), 1);
		return transform.apply(held(onTheObject.x), held(onTheObject.y)).round();
	}

	/*
		A point the object was given is moved where it is put, by the number it has in the object's
		own list. The four an object falls back on are not its own and are shown only, so they
		have no name and nothing takes hold of one.
	*/
	public interactionFor(
		handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		return handle.name ? new ShapingInteraction(this.selection, handle) : null;
	}

	public handOver(handle: SelectionHandle, to: cool.Point): void {
		const at = Number(handle.name.split('.')[1]);
		if (!(at >= 0)) return;

		const objectId = this.selection.selectedObjects()[0];
		const transform = RenderGeometrySection.transformOf(objectId);
		if (!transform) return;

		// The object is told where the point sits on it, which is how it was given to us.
		const onTheObject = transform.invert()?.apply(to.x, to.y);
		if (!onTheObject) return;

		app.socket.sendMessage(
			'setobjectgluepoint id=' +
				String(objectId) +
				' at=' +
				String(at) +
				' x=' +
				onTheObject.x.toFixed(4) +
				' y=' +
				onTheObject.y.toFixed(4),
		);
	}

	public moveByKey(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point | null {
		if (!handle.name) return null;

		const to = HandleGroup.steppedBy(handle, towards, event);
		this.handOver(handle, to);

		return to;
	}

	/// What the object holds about the point that handle stands for, or nothing for one of the
	/// four an object falls back on, which it holds nothing about.
	private held(handle: SelectionHandle): cool.ObjectGluePoint | undefined {
		if (handle.kind !== 'GluePoint') return undefined;

		const at = Number(handle.name.split('.')[1]);
		const objects = this.selection.selectedObjects();
		if (!(at >= 0) || objects.length !== 1) return undefined;

		return RenderGeometrySection.objectOf(objects[0])?.gluePoints?.[at];
	}

	/*
		The side a way out points to while a drag turns the object. A way out is one of the four
		sides of the page, never a way of the object's own, and turning an object moves it to the
		side it comes nearest to: the model adds the turn to it and takes the side that is within
		a quarter turn of the result. So the drag shows what the turn will leave behind, and a
		turn of less than an eighth leaves it where it is.
	*/
	private turnedBy(name: string): string {
		const matrix = this.selection.dragMatrix();
		const side = TiePointHandles.sides.findIndex(
			(one: { name: string }) => one.name === name,
		);
		if (!matrix || side < 0) return name;

		const quarters =
			Math.round(
				(TiePointHandles.sides[side].turn + matrix.rotation()) /
					(Math.PI * 0.5),
			) & 3;

		return TiePointHandles.sides[quarters].name;
	}

	/// The sides, in the order they go round the circle, with the way each of them points.
	private static readonly sides: { name: string; turn: number }[] = [
		{ name: 'right', turn: 0 },
		{ name: 'bottom', turn: Math.PI * 0.5 },
		{ name: 'left', turn: Math.PI },
		{ name: 'top', turn: Math.PI * 1.5 },
	];

	/*
		A point to tie a connector to is a circle, and every way a connector may leave the object
		there puts a nose on that circle: the quarter of it facing that way is replaced by two
		lines out to a tip. A point with no way named is a plain circle, and a connector leaving
		it may go whichever way it likes.
	*/
	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		const held = this.held(handle);
		const radius = HandleLook.widthOf('tiePoint') * 0.5 + grown;

		// The two that face each other are drawn as what they are, a nose on each side.
		const wayOut = held?.wayOut;
		const ways = (
			wayOut === 'horizontal'
				? ['left', 'right']
				: wayOut === 'vertical'
					? ['top', 'bottom']
					: wayOut
						? [wayOut]
						: []
		).map((name: string) => this.turnedBy(name));

		// A nose sits on a whole quarter of the circle. A point never carries more than two ways
		// out, and those two face each other, so half the circle is left between them.
		const half = Math.PI / 4;
		const tip = radius * Math.SQRT2;

		const onTheCircle = (turn: number) => [
			at.vX + radius * Math.cos(turn),
			at.vY + radius * Math.sin(turn),
		];

		let from = TiePointHandles.sides[0].turn - half;
		const [startX, startY] = onTheCircle(from);
		context.moveTo(startX, startY);

		for (const side of TiePointHandles.sides) {
			if (!ways.includes(side.name)) {
				// The circle runs on through this side, so it is drawn with the gaps beside it.
				context.arc(at.vX, at.vY, radius, from, side.turn + half);
			} else {
				context.arc(at.vX, at.vY, radius, from, side.turn - half);
				context.lineTo(
					at.vX + tip * Math.cos(side.turn),
					at.vY + tip * Math.sin(side.turn),
				);
				const [backX, backY] = onTheCircle(side.turn + half);
				context.lineTo(backX, backY);
			}

			from = side.turn + half;
		}

		context.arc(
			at.vX,
			at.vY,
			radius,
			from,
			TiePointHandles.sides[0].turn - half + Math.PI * 2,
		);

		/*
			A dimmed red for a point the object holds as a share of its own size, that red lighter
			for one it holds as a distance of its own, and darker again for the four an object
			falls back on, which it was given none of.
		*/
		if (handle.kind !== 'GluePoint') return HandleLook.tiePointFallenBackOn;

		return held?.keepsItsDistance
			? HandleLook.tiePointKeepingItsDistance
			: HandleLook.colourOfTheKind.tiePoint;
	}

	/*
		The side a point is measured from, where it is not the middle: a short line lying along
		the circle, just outside it, on that side. A point measured from the middle shows none,
		which is what a point is given unless someone says otherwise.
	*/
	public paint(section: SelectionSection, shown: SelectionHandle[]): void {
		const marked = shown.filter(
			(handle: SelectionHandle) => handle.group === this.name,
		);
		if (!marked.length) return;

		const radius = ShapeHandlesSection.gluePointSize() * 0.5;
		// A pixel of air between the circle and the line, so the two read apart.
		const away = radius + 2 * app.dpiScale;

		section.context.save();
		section.context.setTransform(1, 0, 0, 1, 0, 0);
		section.context.strokeStyle = 'black';
		section.context.lineWidth = app.dpiScale;
		section.context.beginPath();

		for (const handle of marked) {
			const held = this.held(handle);
			if (!held) continue;

			const at = new cool.SimplePoint(handle.point.x, handle.point.y);

			for (const side of TiePointHandles.sides) {
				if (held.fromAcross !== side.name && held.fromDown !== side.name)
					continue;

				// The line lies along the circle, so it runs across the way the side faces.
				const middleX = at.vX + away * Math.cos(side.turn);
				const middleY = at.vY + away * Math.sin(side.turn);
				const alongX = -Math.sin(side.turn) * radius * 0.5;
				const alongY = Math.cos(side.turn) * radius * 0.5;

				section.context.moveTo(middleX - alongX, middleY - alongY);
				section.context.lineTo(middleX + alongX, middleY + alongY);
			}
		}

		section.context.stroke();
		section.context.restore();
	}
}

/*
	The points of the path an object is drawn along, with the weights that bend the curves between
	them. They are the finest handles there are, so they come last: they are drawn over everything
	and a press reaches them first.
*/
class PathHandles extends HandlesMovedOneByOne {
	public readonly name: string = 'path';

	public handOver(handle: SelectionHandle, to: cool.Point): void {
		const change = this.selection.pathEditor()?.changeFor(handle.name, to);
		if (!change) return;

		SelectionSection.sendPathChanges(this.selection.selectedObjects()[0], [
			change,
		]);
	}

	public handles(): SelectionHandle[] {
		return this.selection.pathEditor()?.handles() ?? [];
	}

	public paint(section: SelectionSection, shown: SelectionHandle[]): void {
		this.selection.pathEditor()?.paint(section, shown);
	}

	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		/*
			A point of the path is smaller than the eight and drawn over them, so that it can be
			taken hold of where the two meet. A weight that bends the curve at a point is smaller
			again, since there are two of them for every point of a curve.
		*/
		return HandleLook.lay(
			context,
			handle.kind === '9' ? 'pathPoint' : 'weight',
			at,
			grown,
		);
	}
}

/*
	The object itself rather than any handle of it: the group that takes a press anywhere on the
	selection and carries it. It holds no handle and stands last in the list, so it is reached
	only where no group before it answered.
*/
class TheObjectItself extends HandleGroup {
	public readonly name: string = 'object';

	public takesAPress(x: number, y: number): boolean {
		return this.selection.within(x, y);
	}

	public shape(): string {
		// It has no handle to draw.
		return '';
	}

	public interactionForThePlace(
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		if (!ObjectHandles.canBeMoved(this.selection.selectedObjects()))
			return null;

		return new MovingInteraction(this.selection, at);
	}
}

/*
	A connector itself, which is held differently: it is a line across a box that is mostly empty,
	so a press counts as on it only where its own line is drawn, and a press in that emptiness
	belongs to whatever is drawn there. One that is tied to an object is not carried at all - it
	runs from where it is tied, so carrying it would leave it where it was.
*/
class TheConnectorItself extends TheObjectItself {
	/*
		A press reaches this group only where the connector's own line is drawn, so the connector
		itself is what was pressed and it stays selected. Handing the press on would let the
		engine hit test it and find whatever else is drawn there.
	*/
	public passesTheClickOn(): boolean {
		return false;
	}

	public takesAPress(x: number, y: number): boolean {
		const connector = this.selection.theConnector();
		if (connector === undefined) return false;

		const at = this.selection.inDocumentFromCanvas(x, y);

		return RenderGeometrySection.objectHitAt(connector, at.x, at.y);
	}

	public interactionForThePlace(
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		const connector = this.selection.theConnector();
		if (connector !== undefined && ObjectHandles.isATiedConnector(connector))
			return null;

		return super.interactionForThePlace(at);
	}
}
