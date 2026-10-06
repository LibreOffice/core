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
	Scaling the selection by one of the eight handles that frame it. The shape it leads to is
	worked out from the shape it started with and where the mouse is, and everything it shows -
	the objects and every handle - goes through the one transformation that leads from the one
	shape to the other.
*/
class ScalingInteraction extends SelectionInteraction {
	/// The handle that was taken hold of.
	protected handle: SelectionHandle;

	/// The shape the selection had when it began: a middle, a width, a height and an angle.
	private shapeAtStart: any;

	/// The shape it would leave behind, null until the mouse has moved.
	private reached: any = null;

	/// Whether the drag keeps the shape of the object, which a held Shift asks for.
	private keepsRatio: boolean = false;

	/// Where the drag has taken the handle, in twips, null until the mouse has moved.
	protected at: cool.SimplePoint | null = null;

	/*
		Whether the drag goes exactly where it is told. One led by the mouse lines up with other
		things on the page as it goes and keeps the shape of the object while Shift is held; a
		step from the keyboard does neither, it goes as far as the key says along the one axis
		the key names, and Shift is what makes the step a large one.
	*/
	protected exact: boolean = false;

	/// Whether the drag stops a hair short of the opposite side rather than crossing it and
	/// turning the object over. A crop does: a window cannot be turned inside out.
	protected keepsItsFacing: boolean = false;

	constructor(
		selection: SelectionSection,
		handle: SelectionHandle,
		shapeAtStart: any,
	) {
		super(selection);

		this.handle = handle;
		this.shapeAtStart = shapeAtStart;
	}

	/*
		The handles that keep a length of their own, laid out again on the object as the drag
		leaves it rather than carried along with it. The corner radius of a rectangle is such a
		handle: the radius stays the size it is while the object grows.
	*/
	private keptAsTheyAre(
		known: SelectionHandle[],
		mapped: SelectionHandle[],
	): SelectionHandle[] {
		const transform = this.onTheSquare();
		const matrix = this.transformation();
		if (!transform || !matrix) return mapped;

		const becomes = transform.then(matrix);

		return mapped.map((one: SelectionHandle, at: number) => {
			const place = ObjectHandles.keptAlongItsRail(
				one.rails,
				known[at]?.point ?? one.point,
				transform,
				becomes,
			);

			return place ? { ...one, point: place } : one;
		});
	}

	/*
		Whether the drag keeps the shape of the object for those keys: the rule a scale has,
		where a picture keeps its ratio unless Shift says otherwise. A crop asks the other way
		round, since it is not a change of the picture but of the window onto it.
	*/
	protected ratioKept(event: MouseEvent): boolean {
		return HandleScaling.keepsRatio(event, false);
	}

	/// Makes the drag one that goes exactly where it is told.
	public exactlyAsTold(): ScalingInteraction {
		this.exact = true;

		return this;
	}

	/// Whether the objects keep the way they face through this drag: a crop does, and so does a
	/// selection holding an object that may not be turned over.
	private facingKept(): boolean {
		return (
			this.keepsItsFacing ||
			!ObjectHandles.canBeMirrored(this.selection.selectedObjects())
		);
	}

	/// The shape the mouse at that point leads to. Its width or its height counts backwards where
	/// the drag took a side past the one opposite it.
	private shapeFor(to: cool.SimplePoint): any {
		return HandleScaling.shapeAfterDrag(
			to.clone(),
			this.shapeAtStart,
			this.handle.kind,
			this.keepsRatio,
		);
	}

	public move(to: cool.SimplePoint, event: MouseEvent): void {
		if (!ObjectHandles.canBeResized(this.selection.selectedObjects())) return;
		if (!this.shapeAtStart) return;

		/*
			Whether the drag keeps the ratio. A group or a selection of several with a turned
			object in it keeps the ratio by default and Shift lets it go, the other way round from
			a plain shape, where Shift asks for it. One with an object in it that cannot shear
			keeps it whatever the keys say. A single object is never sheared by a scale on its own
			square and goes by the keys alone. A key step has no Shift for this, so it goes by the
			default.
		*/
		const selected = this.selection.selectedObjects();
		const byDefault = ObjectHandles.keepsTheRatioByDefault(selected);
		const always = ObjectHandles.keepsTheRatioAlways(selected);
		const asked = this.exact
			? byDefault
			: byDefault
				? event?.shiftKey !== true
				: this.ratioKept(event);
		this.keepsRatio = always || asked;

		// The drag leads the handle, so it goes to where the handle stands for this mouse point
		// and not to the mouse point itself.
		const led = this.led(to);

		this.at = led.clone();
		this.reached = this.shapeFor(led);

		if (!this.exact) {
			// What may snap to another object of the page is the handle being dragged, taken
			// where the drag has it now.
			const dragged = this.draggedHandle();
			if (dragged) this.selection.lookForASnap(dragged, event);

			// The snap is applied while the drag runs, so that what is drawn and where the
			// handles stand are the one thing, and letting go changes nothing again.
			this.applyTheSnap(led, event);
		}

		this.selection.redraw();
	}

	/*
		Applies the snap: the drag is moved from where the mouse is onto whatever it lined up with
		on the page. One way can snap while the other does not, so each is taken from the line
		where it found one and from the mouse where it did not. A drag that keeps the shape of the
		object snaps to nothing, since it has only one number to give.
	*/
	private applyTheSnap(to: cool.SimplePoint, _event: MouseEvent): void {
		if (this.keepsRatio) return;

		const snapped = this.selection.snappedTo();
		if (!snapped || (snapped[0] === null && snapped[1] === null)) return;

		const at = cool.SimplePoint.fromCorePixels([
			snapped[0] ?? to.pX,
			snapped[1] ?? to.pY,
		]);

		this.at = at.clone();
		this.reached = this.shapeFor(at);
	}

	/*
		The drag ends by handing the engine the change itself, which is the one the objects were
		drawn with while it ran. Naming the handle instead would leave the engine to work the
		change out again from where that handle was let go, and for a turned object the two do not
		meet: a scale along one side of a turned object shears it, and the way an object holds its
		shear is not the way it is drawn.
	*/
	public finish(to: cool.SimplePoint, event: MouseEvent): void {
		if (!this.shapeAtStart) return;

		// The snap has been applied all through the drag, so letting go only hands over what is
		// drawn.
		this.move(to, event);

		const matrix = this.transformation();
		if (matrix)
			SelectionSection.sendTransform(
				this.selection.selectedObjects(),
				matrix,
				'scale',
			);
	}

	/// The handle being dragged, where the drag has it now.
	private draggedHandle(): SelectionHandle | undefined {
		return this.handles(this.selection.knownHandles()).find(
			(one: SelectionHandle) => one.name === this.handle.name,
		);
	}

	public leadingPoint(): cool.SimplePoint | null {
		const dragged = this.draggedHandle();

		return dragged
			? new cool.SimplePoint(dragged.point.x, dragged.point.y)
			: null;
	}

	/*
		The mapping that takes a square onto what is being scaled: the object's own mapping where
		one object is selected, and the box that frames them where several are. A box that frames
		several objects stands upright, so its mapping only says where it is and how large it is.
	*/
	private onTheSquare(): cool.Matrix2D | null {
		const square = this.selection.squareNow();
		if (square) return square;

		const objects = this.selection.selectedObjects();
		if (objects.length === 1)
			return RenderGeometrySection.transformOf(objects[0]);

		const handles = this.selection.knownHandles();
		const upperLeft = handles.find((one: SelectionHandle) => one.kind === '1');
		const lowerRight = handles.find((one: SelectionHandle) => one.kind === '8');
		if (!upperLeft || !lowerRight) return null;

		const width = lowerRight.point.x - upperLeft.point.x;
		const height = lowerRight.point.y - upperLeft.point.y;
		if (!width || !height) return null;

		return new cool.Matrix2D(
			width,
			0,
			0,
			height,
			upperLeft.point.x,
			upperLeft.point.y,
		);
	}

	/*
		What the drag does to a single object, worked out on the object's own square: the handle
		that was taken hold of is at a corner of that square and the one facing it stays where it
		is, so the drag says how much larger the object becomes along each of its own two axes.
		Turning, shearing and mirroring are all in the mapping already, so none of them has to be
		worked out again, and nothing passes through a whole twip or a whole pixel on the way,
		which is what let a shear collect over a run of key presses.
	*/
	private onTheSquareTransformation(
		transform: cool.Matrix2D,
	): cool.Matrix2D | null {
		const mine = ObjectHandles.placeOfKind(this.handle.kind);
		if (!mine || !this.at) return null;

		const back = transform.invert();
		if (!back) return null;

		const asked = back.apply(this.at.x, this.at.y);

		// The place that stays where it is, which is the one facing the handle across the middle
		// of the square.
		const stays = new cool.Point(1 - mine.x, 1 - mine.y);

		/*
			How much larger the object becomes along one of its own axes: how far the mouse is
			from the place that stays, against how far the handle was. A hair rather than nothing,
			so that what the object is given can be undone, and the way it faces is kept while the
			hair is taken. The size is how long the axis is on the page.
		*/
		const along = (
			handleAt: number,
			staysAt: number,
			askedAt: number,
			size: number,
		): number => {
			if (handleAt === 0.5) return 1;

			const reached = (askedAt - staysAt) / (handleAt - staysAt);
			const hair = size ? 10 / size : 0.001;

			// No one step scales an object a thousandfold. A step worked out on a stale state
			// that was a hair high could ask for it, and the engine cannot even work out what
			// to invalidate for a mapping like that.
			const most = 1000;

			if (this.facingKept()) return Math.min(Math.max(reached, hair), most);

			return (
				Math.sign(reached || 1) *
				Math.min(Math.max(Math.abs(reached), hair), most)
			);
		};

		let across = along(mine.x, stays.x, asked.x, transform.lengthOfXAxis());
		let down = along(mine.y, stays.y, asked.y, transform.lengthOfYAxis());

		// Keeping the shape of the object means the same step on both axes, which is the larger
		// of the two the drag asks for. Each of them keeps the way it faces.
		if (this.keepsRatio) {
			const both = Math.max(
				mine.x === 0.5 ? 0 : Math.abs(across),
				mine.y === 0.5 ? 0 : Math.abs(down),
			);
			across = Math.sign(across) * both;
			down = Math.sign(down) * both;
		}

		// Back onto the square, scaled there about the place that stays, and onto the page again.
		const onTheSquare = cool.Matrix2D.IDENTITY.translate(-stays.x, -stays.y)
			.scale(across, down)
			.translate(stays.x, stays.y);

		return back.then(onTheSquare).then(transform);
	}

	public transformation(): cool.Matrix2D | null {
		const transform = this.onTheSquare();
		if (transform) return this.onTheSquareTransformation(transform);

		const from = this.shapeAtStart;
		const to = this.reached;
		if (!from || !to || !from.width || !from.height) return null;

		/*
			A drag that takes a side past the one opposite it says so in the change, as a length
			that counts backwards. What that comes to is the object's own business: one drawn
			along an outline of its own turns the outline over, a custom shape marks itself as
			facing the other way, and one drawn from a rectangle does what the office does with
			it, which is the same road this takes.
		*/
		const across = to.width / from.width;
		const down = to.height / from.height;

		// A hair rather than nothing, so that what the object is given can be undone, and the
		// way it faces is kept while the hair is taken.
		const least = app.twipsToPixels * 10;
		const wide =
			Math.sign(across || 1) * Math.max(Math.abs(across), least / from.width);
		const high =
			Math.sign(down || 1) * Math.max(Math.abs(down), least / from.height);

		// From the middle it started at, upright, scaled along its own two directions, turned
		// back and on to the middle it reached.
		return cool.Matrix2D.IDENTITY.translate(-from.center.x, -from.center.y)
			.rotateAround(0, 0, from.angleRadian)
			.scale(wide, high)
			.rotateAround(0, 0, -from.angleRadian)
			.translate(to.center.x, to.center.y);
	}

	/*
		Where the handles stand while the selection is scaled: every one of them through the
		transformation the drag stands for, which is the state the objects would be in when it
		ends. The eight that frame the selection land exactly where they belong, since they are
		the corners and the sides of that state, and the drawing of the objects goes through the
		very same transformation, so the two can never disagree. A handle that does not travel
		with the object that way is laid out afterwards by the rule of its own kind.
	*/
	public handles(known: SelectionHandle[]): SelectionHandle[] {
		const matrix = this.transformation();
		if (!matrix) return known;

		const mapped = known.map((handle: SelectionHandle) => ({
			...handle,
			point: matrix.apply(handle.point.x, handle.point.y).round(),
		}));

		// A scale can turn the selection over, and the axes turn with it, so the way each of the
		// eight faces is worked out again from where they stand now.
		return ObjectHandles.facedAwayFromTheirPlaces(
			this.keptAsTheyAre(known, mapped),
		);
	}
}
