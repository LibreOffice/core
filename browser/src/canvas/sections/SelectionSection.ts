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
	What is selected, drawn and worked on as one thing: a selected object, a selection of
	several of them, and later the editor of a path. It draws its own handles, says which of
	them lies under the mouse, carries the keyboard that travels them and runs the drag of the
	one that was taken hold of.

	It holds the handles as a list rather than giving each of them a piece of canvas of its own,
	so that a path with hundreds of points costs one of these and not hundreds.
*/
abstract class SelectionSection extends CanvasSectionObject {
	processingOrder: number =
		app.CSections.DefaultForDocumentObjects.processingOrder;
	drawingOrder: number =
		app.CSections.DefaultForDocumentObjects.drawingOrder + 1;
	zIndex: number = app.CSections.DefaultForDocumentObjects.zIndex;
	documentObject: boolean = true;
	/*
		The section covers the handles where the objects stand, but it draws a good deal beyond
		that: the ghost of a drag wherever the drag has taken it, the connectors laid out anew, the
		crop preview. A drag can scroll the view so far that the place the objects stand on leaves
		it, and a document object out of view is not drawn, so the section counts as visible always.
	*/
	isAlwaysVisible: boolean = true;

	/// The keyboard on the handles of this selection.
	public readonly travel: HandleTravel = new HandleTravel(
		() => this.handles(),
		(handle: SelectionHandle, towards: cool.Point, event: KeyboardEvent) =>
			this.moveHandleBy(handle, towards, event),
		(handle: SelectionHandle, event: KeyboardEvent) =>
			this.pressTheHandle(handle, event),
	);

	/*
		Space, Return or 0 on a handle the keyboard is on. On the handle above the selection, 0
		brings the turn or the slant back to nought, and Space and Return switch between turning
		and shearing, where both are offered, as a click on it does. Other handles take no such
		key.
	*/
	private pressTheHandle(
		handle: SelectionHandle,
		event: KeyboardEvent,
	): boolean {
		if (handle.kind !== SelectionSection.turningKind) return false;

		if (event.key === '0') {
			const group = this.groupOf(handle);
			if (group instanceof TurningHandle) group.backToNought(handle);
			return true;
		}

		if (!this.shearingIsOffered()) return false;

		this.switchTheHandleMode();
		return true;
	}

	/// The handles as they were last worked out from the objects of the selection.
	private known: SelectionHandle[] = [];

	/*
		Where a key last left the handle it works on, and which of the eight that was. A change
		that turns the object over lays its square down the other way round, so the handle of
		that name is drawn on the far side and the one now standing where the key left it is the
		one facing it. Held so that the keyboard can follow the place rather than the name.
	*/
	private keyLeftTheHandle: {
		at: cool.Point;
		named: string;
	} | null = null;

	/// The object an end of a connector is being dragged onto, or null while none is aimed at.
	private aimingAt: number | null = null;

	/*
		The connectors whose run was worked out here after a change was laid on the objects they
		hold on to, and which the engine has not answered for yet. The page leaves them out and
		this section draws their run meanwhile, so they stand where the objects now are rather
		than where they were.
	*/
	private connectorsAwaitingAnswer: Set<number> = new Set();

	/*
		The window change of a crop that was sent and is not answered yet, with the picture it
		was sent for, or null while none is open. It keeps the picture drawn as the crop will
		leave it for as long as the engine takes to answer.
	*/
	private cropAwaitingAnswer: {
		objectId: number;
		matrix: cool.Matrix2D;
	} | null = null;

	/// The groups of handles this selection has, in the order that says how they are walked,
	/// drawn and reached by a press. Made again whenever the handles are worked out afresh.
	private groupsHeld: HandleGroup[] | null = null;

	/// The path of a single selected object, with the points and the weights that shape it, or
	/// null where what is selected has no path.
	private editor: PolyPolygonEditor | null = null;

	/// The handle the mouse is over, marked so, or null while it is over none.
	private hovered: string | null = null;

	/*
		The objects the handle above the selection shears, as a key of their ids in any order, or
		null while it turns. It is kept apart from the section, which is made anew, and from the
		order of the ids, which an answer from the engine may change, so the shearing mode stays
		for as long as the same objects are selected, and any other selection turns.
	*/
	private static shearing: string | null = null;

	/// The key of the selected objects, the same for the same objects in any order.
	private keyOfTheSelection(): string {
		return SelectionSection.keyOf(this.selectedObjects());
	}

	private static keyOf(objectIds: number[]): string {
		return objectIds
			.slice()
			.sort((a: number, b: number) => a - b)
			.join(',');
	}

	/// Those objects are selected here: where they are not the ones that were sheared, the
	/// handle turns again, as it does for every new selection.
	public static selectedHere(objectIds: number[]): void {
		if (SelectionSection.shearing !== SelectionSection.keyOf(objectIds))
			SelectionSection.shearing = null;
	}

	/// The mode the handle above the selection is in. It shears only where every selected object
	/// may be sheared; otherwise turning is the only mode.
	public handleMode(): 'turn' | 'shear' {
		return SelectionSection.shearing === this.keyOfTheSelection() &&
			this.shearingIsOffered()
			? 'shear'
			: 'turn';
	}

	/// Whether shearing is offered for this selection: where every object in it may be sheared.
	public shearingIsOffered(): boolean {
		return ObjectHandles.canBeSheared(this.selectedObjects());
	}

	/// Switches the handle above the selection to the other mode, where shearing is offered.
	public switchTheHandleMode(): void {
		if (!this.shearingIsOffered()) return;

		SelectionSection.shearing =
			this.handleMode() === 'shear' ? null : this.keyOfTheSelection();
		this.containerObject.requestReDraw();
	}

	/// The kind of the handle beside the one above the selection that switches it to the other
	/// mode.
	public static readonly otherModeKind: string = 'othermode';

	/*
		Whether the handle that switches to the other mode is shown beside the one above the
		selection: while the mouse is on that one and shearing is offered.
	*/
	private otherModeShown: boolean = false;

	public showsTheOtherMode(): boolean {
		return this.otherModeShown;
	}

	/*
		The bar behind the handle above the selection and the handle of the other mode while that
		one is shown, in canvas pixels: the two points its line runs between and how far the bar
		reaches out from that line. Its ends are half circles around those points, as wide as the
		bar. The points are the middles of the two handles, and beyond the one that shows the
		slanted symbol of shearing the line goes on a little, since that symbol is wider than the
		circle and needs more room to the end. Nothing while the handle of the other mode is not
		shown.
	*/
	private barToTheOtherMode(): {
		from: cool.Point;
		to: cool.Point;
		reach: number;
	} | null {
		if (!this.otherModeShown) return null;

		const shown = this.handles();
		const from = shown.find(
			(handle: SelectionHandle) => handle.kind === SelectionSection.turningKind,
		);
		const to = shown.find(
			(handle: SelectionHandle) =>
				handle.kind === SelectionSection.otherModeKind,
		);
		if (!from || !to) return null;

		const start = HandleLook.drawnAt(from);
		const end = HandleLook.drawnAt(to);
		const a = new cool.Point(start.vX, start.vY);
		const b = new cool.Point(end.vX, end.vY);
		const length = b.subtract(a).length();
		if (!length) return null;

		const size = ShapeHandlesSection.handleSize();
		const along = b.subtract(a).multiplyBy((0.5 * size) / length);
		const shearingFirst = this.handleMode() === 'shear';
		return {
			from: shearingFirst ? a.subtract(along) : a,
			to: shearingFirst ? b : b.add(along),
			reach: 1.25 * size,
		};
	}

	/*
		The middle of the bar to the handle of the other mode, the angle it lies at and its half
		length, in canvas pixels; the half height is how far it reaches out.
	*/
	private static layOfTheBar(bar: {
		from: cool.Point;
		to: cool.Point;
		reach: number;
	}): { middle: cool.Point; turn: number; halfLength: number } {
		const way = bar.to.subtract(bar.from);
		return {
			middle: bar.from.add(way.multiplyBy(0.5)),
			turn: Math.atan2(way.y, way.x),
			halfLength: way.length() / 2 + bar.reach,
		};
	}

	/// Whether that point on the canvas lies on the bar to the handle of the other mode: no
	/// further from the line between the two middles than the bar reaches out.
	private onTheBarToTheOtherMode(mouse: cool.Point): boolean {
		const bar = this.barToTheOtherMode();
		if (!bar) return false;

		const way = bar.to.subtract(bar.from);
		const length = way.length();
		if (!length) return false;

		const along = Math.max(
			0,
			Math.min(length, mouse.subtract(bar.from).dot(way) / length),
		);
		const nearest = bar.from.add(way.multiplyBy(along / length));
		return mouse.distanceTo(nearest) <= bar.reach;
	}

	/*
		Draws the bar to the handle of the other mode, behind the handles, where it is shown: a
		rectangle along the upper side with half circles for ends, filled and edged as a handle
		under the mouse is, as both handles on it are drawn.
	*/
	private drawTheBarToTheOtherMode(): void {
		const bar = this.barToTheOtherMode();
		if (!bar) return;

		const lay = SelectionSection.layOfTheBar(bar);
		const width = lay.halfLength;
		const height = bar.reach;
		const corner = height;

		this.context.save();
		this.context.translate(lay.middle.x, lay.middle.y);
		this.context.rotate(lay.turn);
		this.context.beginPath();
		this.context.moveTo(-width + corner, -height);
		this.context.arcTo(width, -height, width, height, corner);
		this.context.arcTo(width, height, -width, height, corner);
		this.context.arcTo(-width, height, -width, -height, corner);
		this.context.arcTo(-width, -height, width, -height, corner);
		this.context.closePath();
		this.context.fillStyle = HandleLook.colourOfTheKind.turning;
		this.context.strokeStyle = SelectionSection.underTheMouse(
			HandleLook.colourOfTheKind.turning,
		);
		this.context.lineWidth = 2 * app.dpiScale;
		this.context.fill();
		this.context.stroke();
		this.context.restore();
	}

	/// The wait before the handle of the other mode comes up, while it runs.
	private otherModeWait: any = null;

	/// How long the mouse rests on the handle above the selection before the handle of the other
	/// mode comes up, in milliseconds (vectorOtherModeDelay).
	private static otherModeDelay(): number {
		const delay = window.prefs?.getNumber('vectorOtherModeDelay', 2000) ?? 2000;
		return delay >= 0 ? delay : 2000;
	}

	/// Puts the handle of the other mode away, and calls off a wait for it.
	private dropTheOtherMode(): void {
		if (this.otherModeWait !== null) {
			app.timerRegistry.clearTimeout(this.otherModeWait);
			this.otherModeWait = null;
		}
		if (this.otherModeShown) {
			this.otherModeShown = false;
			this.containerObject.requestReDraw();
		}
	}

	/*
		Follows the mouse for the switch to the other mode. Once the mouse has rested on the handle
		above the selection for a while, the handle of the other mode comes up beside it, with a
		bar behind both, and stays while the mouse is on the bar, given as the point on the
		canvas. A press on the handle, which begins a drag, comes before that, so a drag starts
		without the bar. Reaching that handle switches the mode, and it goes again with the bar,
		so the mouse stands beside the handle and nothing switches again until it comes back onto
		it. Answers whether the mode was switched.
	*/
	private followTheOtherMode(
		over: SelectionHandle | undefined,
		mouse?: cool.Point,
	): boolean {
		if (over?.kind === SelectionSection.otherModeKind) {
			this.dropTheOtherMode();
			this.switchTheHandleMode();
			return true;
		}

		const onTheHandle =
			over?.kind === SelectionSection.turningKind &&
			!over.deactivated &&
			!this.interaction &&
			this.shearingIsOffered();

		if (this.otherModeShown) {
			const onTheBar =
				!over &&
				!!mouse &&
				!this.interaction &&
				this.onTheBarToTheOtherMode(mouse);
			if (!onTheHandle && !onTheBar) this.dropTheOtherMode();
			return false;
		}

		if (!onTheHandle) {
			this.dropTheOtherMode();
			return false;
		}

		if (this.otherModeWait === null)
			this.otherModeWait = app.timerRegistry.setTimeout(
				'otherMode',
				() => {
					this.otherModeWait = null;
					if (this.interaction) return;
					this.otherModeShown = true;
					this.containerObject.requestReDraw();
				},
				SelectionSection.otherModeDelay(),
			);
		return false;
	}

	/// What a drag is doing to the selection, null while nothing is being dragged.
	private interaction: SelectionInteraction | null = null;

	/// What a drag of this selection can line up with on the page, and the lines that mark it.
	public readonly snap: SelectionSnap = new SelectionSnap();

	/// A press waiting to be handed to the engine, kept so that a second press can drop it.
	private pendingClick: any = null;

	/// Whether the press that is being answered landed on a handle. Such a press is never a
	/// press on the page, however short it turns out to be.
	private pressedAHandle: boolean = false;

	/// The handle a press took hold of, kept so that a press that turns out to be a click can
	/// put the keyboard on it.
	private pressedHandle: SelectionHandle | null = null;

	/// The group that took the press, which says what a click on that place means.
	private pressedGroup: HandleGroup | null = null;

	/*
		The kind that names the handle which turns the selection. The client makes that handle
		itself: the engine has no handle of the kind, and what it is told at the end of a turn is
		a transformation of the objects.
	*/
	public static readonly turningKind: string = 'rotate';

	/*
		The color that marks the handle the mouse is over: the handle's own color with its
		light turned up, or turned down where it is light already, so that the mark stands out
		from it whatever color that is. Nothing else about the handle changes.
	*/
	public static underTheMouse(color: string): string {
		const of = (at: number) => parseInt(color.substr(at, 2), 16) / 255;
		const red = of(1);
		const green = of(3);
		const blue = of(5);

		const light = Math.max(red, green, blue);
		const shade = light > 0.6 ? 0.55 : 1.8;

		const back = (one: number) =>
			Math.round(Math.min(1, one * shade) * 255)
				.toString(16)
				.padStart(2, '0');

		return '#' + back(red) + back(green) + back(blue);
	}

	/// How far above the selection the handle that turns it sits, in core pixels.
	private static readonly turningDistance: number = 30;

	/*
		The groups of handles of this selection, in order. The last one is the object itself,
		which holds no handle and takes a press that reached none.
	*/
	protected groups(): HandleGroup[] {
		if (!this.groupsHeld)
			this.groupsHeld = [
				new FramingHandles(this),
				new CropHandles(this),
				new TurningHandle(this),
				new ShapingHandles(this),
				new TiePointHandles(this),
				new PathHandles(this),
				this.theConnector() === undefined
					? new TheObjectItself(this)
					: new TheConnectorItself(this),
			];

		return this.groupsHeld;
	}

	/// The group a handle belongs to, or nothing for a handle of no group.
	private groupOf(handle: SelectionHandle): HandleGroup | undefined {
		return this.groups().find(
			(group: HandleGroup) => group.name === handle.group,
		);
	}

	/// Works the handles out again from the objects as they stand, and covers them.
	public refresh(): void {
		/*
			A connector is held by a group of its own, and whether what is selected is one is
			known once the object has arrived, so the groups are made again here. The path of the
			object goes the same way: the group that holds its points asks for it.
		*/
		this.groupsHeld = null;

		const objects = this.selectedObjects();
		/*
			The points of a path are edited only where the objects may be made larger or smaller:
			moving one changes the shape of the object, which is the thing that is held against
			being changed. Where they may not, the editor is not made at all, so there are no
			points to walk to and nothing to take hold of.
		*/
		this.editor =
			objects.length === 1 &&
			RenderGeometrySection.objectOf(objects[0])?.path &&
			ObjectHandles.canBeResized(objects)
				? new PolyPolygonEditor(objects[0])
				: null;

		const worked = this.handlesOfTheGroups();

		/*
			The objects are what the handles are worked out from, and a selection can be reported
			before the objects it stands on have arrived. Holding on to the handles we had says
			something slightly old for a moment, where dropping them says the selection is gone -
			and the update that brings the objects puts them right.
		*/
		if (!worked.length && this.known.length) return;

		this.known = worked;

		// The place a key left is followed once, when the objects it changed come back, and is
		// then forgotten: whatever changes them after that, an undo among other things, is not
		// that key's doing and says nothing about where the keyboard should be.
		this.followTheHandleByPlace();
		this.keyLeftTheHandle = null;

		this.coverTheHandles();
	}

	/*
		The middle of what is selected, from the two handles that frame it across the diagonal.
		Nothing where the client did not work the framing handles out.
	*/
	public middle(): cool.SimplePoint | undefined {
		return SelectionSection.middleOf(this.handles());
	}

	/// The middle of the selection as those handles frame it.
	public static middleOf(
		shown: SelectionHandle[],
	): cool.SimplePoint | undefined {
		const upperLeft = shown.find((one: SelectionHandle) => one.kind === '1');
		const lowerRight = shown.find((one: SelectionHandle) => one.kind === '8');
		if (!upperLeft || !lowerRight) return undefined;

		return new cool.SimplePoint(
			(upperLeft.point.x + lowerRight.point.x) * 0.5,
			(upperLeft.point.y + lowerRight.point.y) * 0.5,
		);
	}

	/// The handles as they are drawn now: where a drag would leave them while one runs, where
	/// the objects put them otherwise.
	public handles(): SelectionHandle[] {
		const shown = this.interaction?.handles(this.known) ?? this.known;

		/*
			Group by group, in the order the groups stand in, and each of them is given the chance
			to lay its own handles out for the state a drag leads to: the one that turns the
			selection keeps its distance from the upper side however the selection is scaled, and
			the points to tie to follow the object under the mouse rather than the drag.
		*/
		return this.groups().flatMap((group: HandleGroup) => {
			const placed = group.place(shown);
			if (placed)
				return placed.map((handle: SelectionHandle) => ({
					...handle,
					group: group.name,
				}));

			return shown.filter(
				(handle: SelectionHandle) => handle.group === group.name,
			);
		});
	}

	/// The handles as the objects have them, which is what an interaction works from.
	public knownHandles(): SelectionHandle[] {
		return this.known;
	}

	/// The path of what is selected, for an interaction that works on one of its points.
	public pathEditor(): PolyPolygonEditor | null {
		return this.editor;
	}

	/// Asks for the page to be drawn again, after something moved under an interaction.
	public redraw(): void {
		this.tellWhoStandsIn();

		// The buttons beside the bands and the player of a video are things of their own beside
		// the canvas, so they are moved rather than drawn along with the rest.
		GraphicSelection.followTheGroupButtons();
		GraphicSelection.followThePlayer();

		this.containerObject.requestReDraw();
	}

	/// Takes in every handle with the box it is drawn as, so that a press on one arrives here.
	private coverTheHandles(): void {
		const shown = this.handles();
		if (!shown.length) {
			this.size = [0, 0];
			return;
		}

		const box = ShapeHandlesSection.handleSize() * app.pixelsToTwips;
		const covered = cool.Range2D.fromPoints(
			shown.map((handle: SelectionHandle) => HandleLook.drawnAt(handle)),
		).expand(box, box);
		const area = new cool.SimpleRectangle(
			covered.minX,
			covered.minY,
			covered.width,
			covered.height,
		);

		// The size goes first: placing the section is what works out whether it is in sight, and
		// that answer is about the whole box, not about the place its corner is at.
		this.size = [area.pWidth, area.pHeight];
		this.setPosition(area.pX1, area.pY1);
	}

	/*
		The handle whose box holds that point, which is where the mouse is on the canvas. The
		handles are searched from the last one to the first, so that the one lying on top of the
		others is the one taken hold of - two of them can sit at the same place, the corner
		radius of a rectangle right next to the upper left corner among them.

		Only a handle that has a name can be taken hold of. A nameless one is shown and no more,
		as the points a connector could tie itself to are, and one of those sits exactly where the
		end of a tied connector sits.
	*/
	private handleAt(x: number, y: number): SelectionHandle | undefined {
		const half = 0.5 * ShapeHandlesSection.handleSize();
		const shown = this.handles();

		for (let at = shown.length - 1; at >= 0; --at) {
			if (!shown[at].name || shown[at].deactivated) continue;

			const point = HandleLook.drawnAt(shown[at]);
			if (Math.abs(point.vX - x) <= half && Math.abs(point.vY - y) <= half)
				return shown[at];
		}

		return undefined;
	}

	/*
		The four corners of the selection, in the order they go round it, in view pixels. Empty
		where the client did not work the framing handles out.
	*/
	private cornersInView(): cool.Point[] {
		const framing = this.framingCorners();

		return framing.length === 4 ? framing : this.boxInView();
	}

	/// The four corners the eight stand on, in the order they go round, in view pixels. Empty for
	/// a selection that is framed by no handles.
	private framingCorners(): cool.Point[] {
		return ['1', '3', '8', '6']
			.map((kind: string) =>
				this.handles().find((handle: SelectionHandle) => handle.kind === kind),
			)
			.filter((handle): handle is SelectionHandle => handle !== undefined)
			.map((handle: SelectionHandle) => {
				const point = new cool.SimplePoint(handle.point.x, handle.point.y);
				return new cool.Point(point.vX, point.vY);
			});
	}

	/*
		The four corners of the box the selected objects take up, for a selection that is framed
		by no handles: a connector is offered its own points alone, and the box is then what says
		where the selection is and what a press on it lands on.
	*/
	private boxInView(): cool.Point[] {
		let box: cool.Range2D | null = null;
		for (const objectId of this.selectedObjects()) {
			const one = RenderGeometrySection.boxOf(objectId);
			if (!one) return [];

			box = box ? box.union(one) : one;
		}

		if (!box) return [];

		const upperLeft = new cool.SimplePoint(box.minX, box.minY);
		const lowerRight = new cool.SimplePoint(box.maxX, box.maxY);

		return [
			new cool.Point(upperLeft.vX, upperLeft.vY),
			new cool.Point(lowerRight.vX, upperLeft.vY),
			new cool.Point(lowerRight.vX, lowerRight.vY),
			new cool.Point(upperLeft.vX, lowerRight.vY),
		];
	}

	/*
		Whether that point lies within the selection. The four corners go round it in order, so
		a point inside is on the same side of every one of the four edges. It holds for a
		selection that is turned as well as for one that is upright.
	*/
	public within(x: number, y: number): boolean {
		const corners = this.cornersInView();
		if (corners.length !== 4) return false;

		const pressed = new cool.Point(x, y);
		let left = false;
		let right = false;

		for (let at = 0; at < 4; ++at) {
			const one = corners[at];
			const next = corners[(at + 1) % 4];
			const side = next.subtract(one).cross(pressed.subtract(one));

			if (side > 0) right = true;
			if (side < 0) left = true;
		}

		return !(left && right);
	}

	/*
		A press lands here where it is on a handle, and where it is on the selection itself,
		which is what carrying it begins with. Everywhere else it goes to whoever draws the page.

		What counts as on the selection is the box it takes up, except for a connector: a
		connector is a line across a box that is mostly empty, and a press in that emptiness
		belongs to whatever is drawn there, so only the line itself counts.
	*/
	isHit(point: number[]): boolean {
		if (this.handleAt(point[0], point[1]) !== undefined) return true;

		// The bar to the handle of the other mode belongs to the section while it is shown, so
		// the mouse stays here on the way along it.
		if (this.onTheBarToTheOtherMode(new cool.Point(point[0], point[1])))
			return true;

		return this.groups().some((group: HandleGroup) =>
			group.takesAPress(point[0], point[1]),
		);
	}

	/// The point on the page, in twips, that a place on the canvas stands for.
	public inDocumentFromCanvas(x: number, y: number): cool.SimplePoint {
		return cool.SimplePoint.fromCorePixels([
			x - this.myTopLeft[0] + this.position[0],
			y - this.myTopLeft[1] + this.position[1],
		]);
	}

	/*
		While an end of a connector is dragged, the object under the mouse is the one the end
		would tie itself to, and it offers the places where it could land. They come and go as
		the mouse enters an object and leaves it again.
	*/
	private aimAtWhatIsUnder(at: cool.SimplePoint): void {
		const connector = this.theConnector();
		const held = this.interaction?.handleHeld();
		const onAnEnd =
			connector !== undefined &&
			held !== null &&
			held !== undefined &&
			['9.0.0', '9.0.1'].includes(held.name);

		const under = onAnEnd
			? (RenderGeometrySection.objectIdAt(at.x, at.y) ?? null)
			: null;
		const aimed = under === connector ? null : under;

		if (aimed === this.aimingAt) return;

		this.aimingAt = aimed;
		this.coverTheHandles();
		this.redraw();
	}

	/// The names of a connector's two ends: it counts them as the first two points of its first
	/// polygon.
	private static readonly connectorEnds: string[] = ['9.0.0', '9.0.1'];

	/*
		Where an end of the connector being dragged to that place would tie itself, worked out
		the way the engine works it out when the end is let go: on the object aimed at, one of
		its own points within a handle's reach comes first, then the middle of one of its sides,
		then the middle of its box, which makes the engine pick the best side itself. An end over
		the object but near none of those stays free, so a shape behind the drag does not take
		an end that was only passing over it. Nothing while no object is aimed at.
	*/
	public whereTheEndWouldTie(at: cool.Point): {
		objectId: number;
		tie: cool.ConnectorTie;
		point?: cool.Point;
	} | null {
		const aimed = this.aimingAt;
		if (aimed === null) return null;

		const object = RenderGeometrySection.objectOf(aimed);
		if (!object) return null;

		// The engine's reach for a tie is a handle's size either way of the point.
		const reach = ShapeHandlesSection.handleSize() * app.pixelsToTwips;
		const within = (point: cool.Point) =>
			Math.abs(point.x - at.x) <= reach && Math.abs(point.y - at.y) <= reach;
		const nearest = (
			handles: SelectionHandle[],
		): { handle: SelectionHandle; index: number } | undefined => {
			let best: { handle: SelectionHandle; index: number } | undefined;
			let bestDistance = Number.POSITIVE_INFINITY;
			handles.forEach((handle: SelectionHandle, index: number) => {
				if (!within(handle.point)) return;
				const distance =
					Math.abs(handle.point.x - at.x) + Math.abs(handle.point.y - at.y);
				if (distance < bestDistance) {
					bestDistance = distance;
					best = { handle, index };
				}
			});
			return best;
		};

		const own = nearest(GluePointHandles.ownPointsOf(aimed));
		if (own) {
			const id = object.gluePoints?.[own.index]?.id;
			return {
				objectId: aimed,
				tie: { point: id ?? 0 },
				point: own.handle.point,
			};
		}

		const side = nearest(GluePointHandles.defaultPointsOf(aimed));
		if (side)
			return {
				objectId: aimed,
				tie: { autoVertex: true, point: side.index },
				point: side.handle.point,
			};

		if (object.x !== undefined && object.y !== undefined) {
			const middle = new cool.Point(
				object.x + (object.width ?? 0) / 2,
				object.y + (object.height ?? 0) / 2,
			);
			if (within(middle)) return { objectId: aimed, tie: { bestVertex: true } };
		}

		return null;
	}

	/*
		Which way a handle that moves a line of the selected connector goes, worked out from the
		connector's run as it stands now. Nothing for an end of the connector, for anything that
		is not a connector, and for a run that was set from outside.
	*/
	public howTheLineHandleMoves(
		handle: SelectionHandle,
	): { movesAcross: boolean } | undefined {
		const connector = this.theConnector();
		if (
			connector === undefined ||
			handle.kind !== '9' ||
			SelectionSection.connectorEnds.includes(handle.name)
		)
			return undefined;

		const data = RenderGeometrySection.currentPart();
		const object = data?.objects.get(connector);
		const number = Number(handle.name.split('.')[2]);
		if (!data || !object || !(number >= 2)) return undefined;

		return ConnectorRouter.lineHandle(data, object, number);
	}

	/*
		The run a drag of a line handle of a connector shows of itself. Where the client routes
		the connector, the run is laid out anew with that line moved across itself by what the
		drag moved, which is what the engine does with such a drag. Otherwise it is the current
		run with the line the handle sits on moved, the line being the one whose middle the
		handle stood on. Nothing for anything else.
	*/
	public runWhileALineIsDragged(
		handle: SelectionHandle,
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const connector = this.theConnector();
		if (connector === undefined) return null;
		if (
			handle.kind !== '9' ||
			SelectionSection.connectorEnds.includes(handle.name)
		)
			return null;

		const data = RenderGeometrySection.currentPart();
		const routedFrom = data?.objects.get(connector);
		const number = Number(handle.name.split('.')[2]);
		const startedAt = this.known.find(
			(one: SelectionHandle) => one.name === handle.name,
		);
		if (data && routedFrom && startedAt && number >= 2) {
			const routed = ConnectorRouter.routeWithALineMoved(
				data,
				routedFrom,
				number,
				new cool.Point(at.x - startedAt.point.x, at.y - startedAt.point.y),
			);
			if (routed) return [{ closed: false, points: routed }];
		}

		const object = RenderGeometrySection.objectOf(connector);
		const run = object ? ConnectorRouter.trackOf(object) : undefined;
		if (!run || run.length < 2) return null;

		const stood = this.known.find(
			(one: SelectionHandle) => one.name === handle.name,
		);
		if (!stood) return null;

		let line = -1;
		let nearest = Number.POSITIVE_INFINITY;
		for (let n = 0; n + 1 < run.length; ++n) {
			const middle = new cool.Point(
				(run[n].x + run[n + 1].x) / 2,
				(run[n].y + run[n + 1].y) / 2,
			);
			const away = middle.distanceTo(stood.point);
			if (away < nearest) {
				nearest = away;
				line = n;
			}
		}
		if (line < 0) return null;

		const horizontal =
			Math.abs(run[line + 1].y - run[line].y) <=
			Math.abs(run[line + 1].x - run[line].x);
		const moved = run.map((point: cool.ObjectPathPoint) => ({ ...point }));
		const by = at.subtract(stood.point);
		for (const n of [line, line + 1]) {
			if (horizontal) moved[n].y += by.y;
			else moved[n].x += by.x;
		}

		return [{ closed: false, points: moved }];
	}

	/*
		The run a drag of one end of a connector shows of itself: the connector laid out anew
		between the end that stays and the one the drag holds, tied where it would tie or free
		where it would lie, as a thin line. Where the connector has no routing numbers the line
		runs straight from the one end to the other. Nothing for anything else.
	*/
	public runWhileAnEndIsDragged(
		handle: SelectionHandle,
		at: cool.Point,
	): cool.ObjectPathPolygon[] | null {
		const connector = this.theConnector();
		if (connector === undefined) return null;

		const dragged = SelectionSection.connectorEnds.indexOf(handle.name);
		if (dragged < 0) return null;

		const data = RenderGeometrySection.currentPart();
		const object = data?.objects.get(connector);
		if (data && object?.routing) {
			const would = this.whereTheEndWouldTie(at);
			const run = ConnectorRouter.routeWithAnEnd(
				data,
				object,
				dragged === 0,
				would ? would.tie : { x: at.x, y: at.y },
				would?.objectId,
			);
			if (run) return [{ closed: false, points: run }];
		}

		const stays = this.known.find(
			(one: SelectionHandle) =>
				one.name === SelectionSection.connectorEnds[1 - dragged],
		);
		if (!stays) return null;

		return [
			{
				closed: false,
				points: [
					{ x: stays.point.x, y: stays.point.y },
					{ x: at.x, y: at.y },
				],
			},
		];
	}

	/// The object an end of a connector is being dragged onto, or nothing while none is aimed at.
	public aimedAtObject(): number | null {
		return this.aimingAt;
	}

	/// Where the running drag has taken what it holds, or nothing while nothing is dragged.
	public leadingPointOfTheDrag(): cool.SimplePoint | null {
		return this.interaction?.leadingPoint() ?? null;
	}

	/// The connector this selection stands on, where it stands on one and it is a connector.
	public theConnector(): number | undefined {
		const objects = this.selectedObjects();
		if (objects.length !== 1) return undefined;

		return ObjectHandles.isAConnector(objects[0]) ? objects[0] : undefined;
	}

	/*
		Draws the dashed line around what is selected, through the four corners the framing
		handles sit on. They follow whatever is being done to the selection, so the line turns
		with a turn and grows with a scale.
	*/
	private drawTheFrame(): void {
		// The line follows the eight. A selection they do not frame - a connector, which is
		// offered its own points alone - is marked by those points and nothing else.
		const corners = this.framingCorners();
		if (corners.length !== 4) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.context.strokeStyle = app.map.uiManager.isBackgroundDark()
			? 'white'
			: 'black';
		this.context.setLineDash([3, 3]);
		this.context.beginPath();

		corners.forEach((corner: cool.Point, at: number) => {
			if (at === 0) this.context.moveTo(corner.x, corner.y);
			else this.context.lineTo(corner.x, corner.y);
		});

		this.context.closePath();
		this.context.stroke();
		this.context.setLineDash([]);
		this.context.restore();
	}

	/*
		The objects the bands go round, from the outermost one inwards: every group the selection
		sits inside, and every selected object that holds others. Each of them takes a band, so how
		deep in a hierarchy of groups the selection sits is there to be seen, and which of several
		selected objects are groups is seen at one look. A diagram is one of them, which is why a
		band stays round a diagram when one of the shapes it holds is selected on its own.

		A selection of several objects takes none round itself: it is put together for the moment
		and taken apart again, while a group stays.
	*/
	private surroundObjects(): number[] {
		const objects = this.selectedObjects();
		if (!objects.length) return [];

		const holding: number[] = [];
		let walking = RenderGeometrySection.parentOf(objects[0]);

		for (let guard = 0; guard < 100 && walking; ++guard) {
			holding.unshift(walking);
			walking = RenderGeometrySection.parentOf(walking);
		}

		// The groups a diagram holds are its own business and stay hidden: the band of the
		// diagram is the last one.
		const diagram = holding.findIndex(
			(objectId: number) => RenderGeometrySection.objectOf(objectId)?.isDiagram,
		);
		if (diagram >= 0) return holding.slice(0, diagram + 1);

		for (const objectId of objects)
			if (RenderGeometrySection.holdsObjects(objectId)) holding.push(objectId);

		return holding;
	}

	/// What kind of band that object takes.
	private surroundKind(objectId: number): SurroundKind {
		return RenderGeometrySection.objectOf(objectId)?.isDiagram
			? 'diagram'
			: 'group';
	}

	/*
		The four places the band round that group or diagram goes through, in twips, in the order
		they go round it: the corners of the upright box round its frame. For the one that is
		selected that frame is the selection's, the dashed one the handles stand on; for a group
		holding what is selected it is the group's own mapping. So the band is upright and holds
		the frame, however that is turned.
	*/
	private surroundPlaces(objectId: number): cool.SimplePoint[] {
		const objects = this.selectedObjects();
		const frame =
			objects.length === 1 && objects[0] === objectId
				? this.squareNow()
				: RenderGeometrySection.transformOf(objectId);

		// Without a mapping of its own the band goes round the boxes of what it holds.
		const box = frame
			? cool.Range2D.fromPoints([
					frame.apply(0, 0),
					frame.apply(1, 0),
					frame.apply(1, 1),
					frame.apply(0, 1),
				])
			: this.boxAround(RenderGeometrySection.theOnesThatDraw([objectId]));
		if (!box) return [];

		return [
			new cool.SimplePoint(box.minX, box.minY),
			new cool.SimplePoint(box.maxX, box.minY),
			new cool.SimplePoint(box.maxX, box.maxY),
			new cool.SimplePoint(box.minX, box.maxY),
		];
	}

	/*
		The groups and diagrams whose bands take a column of buttons beside them. Where the
		selection sits inside a group or a diagram it is the outermost of those, the one the page
		holds, whatever the depth. Otherwise it is every selected group and diagram, which then
		stand side by side on the page.
	*/
	public bandsWithButtons(): number[] {
		const objects = this.selectedObjects();
		if (!objects.length) return [];

		let outermost: number | undefined;
		for (
			let walking = RenderGeometrySection.parentOf(objects[0]), guard = 0;
			walking && guard < 100;
			walking = RenderGeometrySection.parentOf(walking), ++guard
		)
			outermost = walking;
		if (outermost !== undefined) return [outermost];

		return objects.filter((objectId: number) =>
			RenderGeometrySection.holdsObjects(objectId),
		);
	}

	/*
		The box the band round that group or diagram goes round, in core pixels, or nothing where
		it has none. Whatever stands beside that band is placed against this box, so that it sits
		beside what is drawn and not beside something else.
	*/
	public bandBox(objectId: number): cool.Range2D | undefined {
		const places = this.surroundPlaces(objectId);
		if (places.length !== 4) return undefined;

		return cool.Range2D.fromPoints(
			places.map((place) => new cool.Point(place.pX, place.pY)),
		);
	}

	/*
		The four places pushed outwards by that much, in view pixels. Each one moves to the place
		that lies that far from both of the edges meeting there, so the band round them keeps an
		even width along a selection that is turned or sheared.
	*/
	private static grownOutwards(
		corners: cool.Point[],
		by: number,
	): cool.Point[] {
		const middle = corners
			.reduce(
				(sum: cool.Point, corner: cool.Point) => sum.add(corner),
				new cool.Point(0, 0),
			)
			.divideBy(corners.length);

		// The unit normal of the edge that begins at each corner, facing away from the middle.
		const normals = corners.map((corner: cool.Point, at: number) => {
			const next = corners[(at + 1) % corners.length];
			const along = next.subtract(corner);
			const length = along.length() || 1;
			const normal = new cool.Point(along.y / length, -along.x / length);
			const outwards = corner.add(next).multiplyBy(0.5).subtract(middle);

			return normal.dot(outwards) < 0 ? normal.multiplyBy(-1) : normal;
		});

		return corners.map((corner: cool.Point, at: number) => {
			const before = normals[(at + corners.length - 1) % corners.length];
			const here = normals[at];

			// The two edges meeting at a corner stand at some angle to each other, and the
			// sharper that angle the further out the corner has to go to keep its distance from
			// both. A very sharp one is held back, so a thin sliver of a selection cannot throw
			// its corners far across the page.
			const together = before.dot(here);
			const share = by / Math.max(1 + together, 0.2);

			return corner.add(before.add(here).multiplyBy(share));
		});
	}

	/*
		Lays a rounded path through those places into the path being built: along each edge, and
		round each corner with an arc of that radius. The radius is held down to half of the
		shortest edge, so a small band still comes out as a closed path rather than as an arc that
		has nowhere to fit.
	*/
	private static layRounded(
		context: CanvasRenderingContext2D,
		corners: cool.Point[],
		radius: number,
	): void {
		const middleOf = (one: cool.Point, other: cool.Point): cool.Point =>
			one.add(other).multiplyBy(0.5);

		const shortest = Math.min(
			...corners.map((corner: cool.Point, at: number) =>
				corners[(at + 1) % corners.length].distanceTo(corner),
			),
		);
		const rounding = Math.max(0, Math.min(radius, shortest * 0.5));

		const from = middleOf(corners[corners.length - 1], corners[0]);
		context.moveTo(from.x, from.y);

		for (let at = 0; at < corners.length; ++at) {
			const corner = corners[at];
			const to = middleOf(corner, corners[(at + 1) % corners.length]);

			context.arcTo(corner.x, corner.y, to.x, to.y, rounding);
			context.lineTo(to.x, to.y);
		}

		context.closePath();
	}

	/*
		Draws the band round one group or diagram: two rounded paths, the inner one through the
		places the band goes round and the outer one that much further out, with the space between
		them filled.

		Everything it is drawn with comes from the one color of its kind: the fill runs from the
		darker of that color through it to the lighter, and both bounds are drawn in the darker.
	*/
	private drawOneSurround(objectId: number): void {
		const kind = this.surroundKind(objectId);
		if (kind === 'group' && !HandleLook.bandRoundAGroup) return;

		const places = this.surroundPlaces(objectId);
		if (places.length !== 4) return;

		const around = places.map(
			(place: cool.SimplePoint) => new cool.Point(place.vX, place.vY),
		);
		const thickness = HandleLook.widthOfASurround();
		const inner = SelectionSection.grownOutwards(
			around,
			HandleLook.distanceBeforeASurround(),
		);
		const outer = SelectionSection.grownOutwards(
			around,
			HandleLook.reachOfASurround(),
		);

		const color = HandleLook.colourOfTheSurround[kind];
		const lighter = HandleLook.blended(color, HandleLook.surroundBlend);
		const darker = HandleLook.blended(color, -HandleLook.surroundBlend);

		const gradient = this.context.createLinearGradient(
			outer[0].x,
			outer[0].y,
			outer[2].x,
			outer[2].y,
		);
		gradient.addColorStop(0, darker);
		gradient.addColorStop(0.5, color);
		gradient.addColorStop(1, lighter);

		this.context.beginPath();
		SelectionSection.layRounded(this.context, outer, thickness);
		SelectionSection.layRounded(this.context, inner, thickness * 0.5);
		this.context.fillStyle = gradient;
		this.context.fill('evenodd');

		this.context.strokeStyle = darker;
		this.context.lineWidth = app.dpiScale;

		this.context.beginPath();
		SelectionSection.layRounded(this.context, outer, thickness);
		this.context.stroke();

		this.context.beginPath();
		SelectionSection.layRounded(this.context, inner, thickness * 0.5);
		this.context.stroke();
	}

	/*
		Draws a band for every group the selection sits in, the outermost first. While a drag
		runs there is none, as there are no buttons beside it then: the band stands where the
		objects stand once the drag is over.
	*/
	private drawTheSurrounds(): void {
		if (this.isDragged()) return;

		const holding = this.surroundObjects();
		if (!holding.length) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		for (const objectId of holding) this.drawOneSurround(objectId);

		this.context.restore();
	}

	/*
		Draws the path the running interaction would leave behind, as a thin line. It is what a
		drag of a point shows of itself: moving a point is not a transformation of the object, so
		the object cannot be drawn through one, but the line it would follow can.
	*/
	private drawTheOutline(): void {
		const polygons = this.interaction?.outline();
		if (!polygons?.length) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		// The point at the very start of the document says where the drawing of the part begins
		// on the canvas, and a twip is that many pixels wide.
		const origin = new cool.SimplePoint(0, 0);
		this.context.translate(origin.vX, origin.vY);
		this.context.scale(app.twipsToPixels, app.twipsToPixels);
		this.context.strokeStyle = HandleLook.outlineOfADrag;
		this.context.lineWidth = app.pixelsToTwips;
		this.context.beginPath();

		for (const polygon of polygons) {
			const points = polygon.points ?? [];
			if (!points.length) continue;

			this.context.moveTo(points[0].x, points[0].y);

			for (let at = 1; at <= points.length; ++at) {
				const from = points[at - 1];
				const to = points[at % points.length];
				if (at === points.length && !polygon.closed) break;

				if (from.aheadX !== undefined || to.behindX !== undefined)
					this.context.bezierCurveTo(
						from.aheadX ?? from.x,
						from.aheadY ?? from.y,
						to.behindX ?? to.x,
						to.behindY ?? to.y,
						to.x,
						to.y,
					);
				else this.context.lineTo(to.x, to.y);
			}

			if (polygon.closed) this.context.closePath();
		}

		this.context.stroke();
		this.context.restore();
	}

	onDraw(): void {
		this.drawTheInteraction();
		this.drawTheConnectorsAwaitingAnswer();
		this.drawTheOutline();

		const shown = this.handles();
		for (const group of this.groups()) group.paint(this, shown);

		this.snap.draw(this);
		this.drawTheSurrounds();
		this.drawTheFrame();

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		this.drawTheBarToTheOtherMode();

		const size = ShapeHandlesSection.handleSize();
		for (const handle of this.handles()) {
			const group = this.groupOf(handle);
			if (!group) continue;

			const point = HandleLook.drawnAt(handle);
			// The handle of the other mode is drawn as one under the mouse, as large as the
			// handle it stands beside.
			const hovered =
				(!handle.deactivated &&
					handle.name !== '' &&
					handle.name === this.hovered) ||
				handle.lit === true ||
				handle.kind === SelectionSection.otherModeKind;

			/*
				The handle the keyboard is on is drawn a third larger, at the larger half of its
				blink. The one under the mouse is drawn a little larger as well, by a third of
				that, which is enough to be seen beside the color it takes and little enough
				that the two cannot be taken for one another.
			*/
			const grown =
				!handle.deactivated && this.travel.showsAsActive(handle.name)
					? size / 3
					: hovered
						? size / 8
						: 0;

			this.context.beginPath();
			this.context.strokeStyle = handle.deactivated
				? HandleLook.deactivatedOutlineColour
				: HandleLook.outlineColour;

			/*
				A handle that does nothing keeps its shape and its size, so that the selection
				looks the same whatever may be done to it, and is drawn pale to say that taking
				hold of it would do nothing.
			*/
			const color = group.shape(this.context, handle, point, grown);
			this.context.fillStyle = handle.deactivated
				? HandleLook.deactivatedColour
				: color;

			this.context.closePath();
			this.context.fill();
			this.context.stroke();

			/*
				The handle under the mouse is marked by drawing its own edge again, two pixels
				wide and in the one color kept for it. It keeps its shape, so a square stays a
				square and a circle a circle, whatever the handle is.
			*/
			if (hovered) {
				this.context.strokeStyle = SelectionSection.underTheMouse(
					String(this.context.fillStyle),
				);
				this.context.lineWidth = 2 * app.dpiScale;
				this.context.stroke();
				this.context.lineWidth = 1;
			}
		}

		this.context.restore();

		this.drawTheMarks();
		this.drawTheReading();
	}

	/*
		Draws the marks of a turn or a shear on the circle the handle runs on: a small triangle
		pointing to the middle, with what it stands for written inside it. They lie over
		everything else, since the handle is taken to them.
	*/
	private drawTheMarks(): void {
		const marks = this.interaction?.marks() ?? [];
		if (!marks.length) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.context.lineWidth = 1;
		const font = Math.round(11 * app.dpiScale) + 'px sans-serif';
		this.context.font = font;
		this.context.textAlign = 'center';
		this.context.textBaseline = 'middle';

		// The mark the mouse is on is marked as a handle under the mouse is: a quarter larger,
		// grown from its middle, with its edge drawn again in the color kept for it.
		const mouse = this.lastMoved?.at;

		for (const mark of marks) {
			const hit = !!mouse && SelectionInteraction.isOnTheMark(mark, mouse);
			const middle = mark.corners
				.reduce((sum: cool.Point, corner: cool.Point) => sum.add(corner))
				.divideBy(mark.corners.length);
			const corners = mark.corners.map((corner: cool.Point) => {
				const grown = hit
					? middle.add(corner.subtract(middle).multiplyBy(1.25))
					: corner;
				return new cool.SimplePoint(grown.x, grown.y);
			});

			this.context.beginPath();
			corners.forEach((corner: cool.SimplePoint, at: number) =>
				at
					? this.context.lineTo(corner.vX, corner.vY)
					: this.context.moveTo(corner.vX, corner.vY),
			);
			this.context.closePath();
			this.context.fillStyle = HandleLook.colourOfTheKind.turning;
			this.context.strokeStyle = HandleLook.outlineColour;
			this.context.fill();
			this.context.stroke();

			if (hit) {
				this.context.strokeStyle = SelectionSection.underTheMouse(
					String(this.context.fillStyle),
				);
				this.context.lineWidth = 2 * app.dpiScale;
				this.context.stroke();
				this.context.lineWidth = 1;
			}

			// The label stands upright in the middle of the triangle, in bold on its white fill.
			const written = new cool.SimplePoint(middle.x, middle.y);
			this.context.font = 'bold ' + font;
			this.context.fillStyle = HandleLook.outlineColour;
			this.context.fillText(mark.label, written.vX, written.vY);
			this.context.font = font;
		}

		this.context.restore();
	}

	/*
		Writes an angle beside where it is changed, below and to the right, where the pointer
		leaves it free: what the running interaction reads beside the mouse, and while nothing
		runs, the turn or the slant beside the handle above the selection when the keyboard is
		on it. It has a light edge, so it reads on any page. Each of the two can be switched off in
		the client's preferences: vectorAngleAtMouse and vectorAngleAtHandle.
	*/
	private drawTheReading(): void {
		let reading: string | null = null;
		let x = 0;
		let y = 0;

		if (this.interaction) {
			const mouse = this.lastMoved?.at;
			if (!mouse || !window.prefs?.getBoolean('vectorAngleAtMouse', true))
				return;
			reading = this.interaction.reading();
			const at = new cool.SimplePoint(mouse.x, mouse.y);
			x = at.vX + 16 * app.dpiScale;
			y = at.vY + 24 * app.dpiScale;
		} else {
			const handle = this.travel.activeHandle() as SelectionHandle | undefined;
			if (
				handle?.kind !== SelectionSection.turningKind ||
				handle.deactivated ||
				!window.prefs?.getBoolean('vectorAngleAtHandle', true)
			)
				return;
			reading = this.readingOfTheHandleAbove(handle);
			const at = HandleLook.drawnAt(handle);
			const size = ShapeHandlesSection.handleSize();
			x = at.vX + size;
			y = at.vY + size;
		}
		if (!reading) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.context.font = Math.round(11 * app.dpiScale) + 'px sans-serif';
		this.context.textAlign = 'left';
		this.context.textBaseline = 'middle';
		this.context.strokeStyle = '#FFFFFF';
		this.context.lineWidth = 3;
		this.context.strokeText(reading, x, y);
		this.context.fillStyle = HandleLook.outlineColour;
		this.context.fillText(reading, x, y);
		this.context.restore();
	}

	/// The angle of the selection in the mode of the handle above it: the turn as the engine
	/// counts it, or the slant.
	private readingOfTheHandleAbove(handle: SelectionHandle): string {
		return this.handleMode() === 'shear'
			? ShearingInteraction.readingOf(
					ShearingInteraction.slantOf(this, handle.turnedBy ?? 0),
				)
			: TurningInteraction.readingOf(this.turnOfTheSelection());
	}

	/// Whether the selection is one object that draws itself, not several and not a group.
	public isOneObject(): boolean {
		const objects = this.selectedObjects();
		const drawing = RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(objects),
		);
		return (
			objects.length === 1 && drawing.length === 1 && drawing[0] === objects[0]
		);
	}

	/// The turn of a mapping: the angle of its across axis, as the canvas counts angles.
	private static turnOfTheMapping(mapping: cool.Matrix2D): number {
		return Math.atan2(mapping.b, mapping.a);
	}

	/*
		The slant of a mapping: the angle its axis down from the upper side stands at against the
		right angle to that side, in the slant's own coordinates. On a mapping turned over that
		axis points up, and the slant is measured against its own way.
	*/
	private static slantOfTheMapping(mapping: cool.Matrix2D): number {
		const turn = SelectionSection.turnOfTheMapping(mapping);
		const across = mapping.c * Math.cos(turn) + mapping.d * Math.sin(turn);
		const down = -mapping.c * Math.sin(turn) + mapping.d * Math.cos(turn);
		return down >= 0 ? Math.atan2(-across, down) : Math.atan2(across, -down);
	}

	/*
		The turns or the slants of the objects that draw for this selection, members of groups
		included: whether any of them has one, and the one all of them share, or nothing where
		they differ, in radians. The engine keeps angles in hundredths of a degree, so a little is
		allowed for.
	*/
	public anglesOf(kind: 'turn' | 'slant'): {
		any: boolean;
		shared: number | null;
	} {
		const angles = RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(this.selectedObjects()),
		)
			.map((objectId: number) => RenderGeometrySection.transformOf(objectId))
			.filter((mapping) => !!mapping)
			.map((mapping) =>
				kind === 'turn'
					? SelectionSection.turnOfTheMapping(mapping as cool.Matrix2D)
					: SelectionSection.slantOfTheMapping(mapping as cool.Matrix2D),
			);
		if (!angles.length) return { any: false, shared: null };

		const apart = (one: number, other: number) =>
			Math.abs(Math.sin((one - other) / 2));
		const any = angles.some((angle: number) => apart(angle, 0) > 0.0005);
		const shared = angles.every(
			(angle: number) => apart(angle, angles[0]) < 0.001,
		);
		return { any, shared: shared ? angles[0] : null };
	}

	/*
		The mapping that stands for several objects or a group where they all have the same turn
		and the same slant: the first one's. Nothing where they differ.
	*/
	public mappingTheyShare(): cool.Matrix2D | null {
		if (this.anglesOf('turn').shared === null) return null;
		if (this.anglesOf('slant').shared === null) return null;

		const first = RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(this.selectedObjects()),
		)[0];
		return first === undefined
			? null
			: (RenderGeometrySection.transformOf(first) ?? null);
	}

	/*
		Takes the turns away by the 0 key, the 0 of the row of digits or of the number pad: every
		turn where some object is turned, otherwise every slant where some object is slanted. A 0
		that finds neither is left alone and goes on as any other key. Answers whether the key was
		used.
	*/
	public resetByKey(event: KeyboardEvent): boolean {
		if (event.code !== 'Digit0' && event.code !== 'Numpad0') return false;
		if (event.shiftKey || event.ctrlKey || event.altKey || event.metaKey)
			return false;

		if (this.anglesOf('turn').any && this.takeTheAnglesAway('turn'))
			return true;
		return this.anglesOf('slant').any && this.takeTheAnglesAway('slant');
	}

	/*
		Takes the turn or the slant away from every object that draws for the selection, members
		of groups included, each by its own angle around its own middle. The engine is sent every
		object's new mapping as one change, and each object's own change is laid on it here at
		once, so a further key reads the objects as they will be. The frame is made anew from the
		objects: a single object's from its mapping, that of several objects or a group as their
		upright box until the engine has answered. Answers whether any object was changed.
	*/
	private takeTheAnglesAway(kind: 'turn' | 'slant'): boolean {
		const reached = new Map<number, cool.Matrix2D>();
		const changes = new Map<number, cool.Matrix2D>();
		for (const objectId of RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(this.selectedObjects()),
		)) {
			const mapping = RenderGeometrySection.transformOf(objectId);
			if (!mapping) continue;

			// An object that may not be turned keeps its turn, one that may not be sheared its
			// slant.
			const object = RenderGeometrySection.objectOf(objectId);
			if (kind === 'turn' ? object?.cannotBeTurned : object?.cannotBeSheared)
				continue;

			const middle = mapping.apply(0.5, 0.5);
			const change =
				kind === 'slant'
					? SelectionSection.slantTakenAway(mapping, middle)
					: cool.Matrix2D.IDENTITY.rotateAround(
							middle.x,
							middle.y,
							-SelectionSection.turnOfTheMapping(mapping),
						);
			changes.set(objectId, change);
			reached.set(objectId, mapping.then(change));
		}

		if (!reached.size) return false;

		ShearingInteraction.forgetTheKeys();
		TurningInteraction.forgetTheKeys();
		SelectionSection.sendMappings(reached, kind === 'turn' ? 'turn' : 'shear');

		// A tied connector is laid out anew by the engine between what it is tied to.
		for (const [objectId, change] of changes)
			if (!ObjectHandles.isATiedConnector(objectId))
				RenderGeometrySection.changeObject(objectId, change);

		if (this.isOneObject()) this.square = null;
		else {
			this.frameFollowsTheObjects = true;
			this.square = null;
		}
		this.known = this.handlesOfTheGroups();
		this.redraw();
		return true;
	}

	/*
		The change that takes the slant of that mapping away around that point: in the frame along
		its upper side, the upper side stays and the axis down from it is stood at a right angle
		to it, keeping its length.
	*/
	private static slantTakenAway(
		mapping: cool.Matrix2D,
		around: cool.Point,
	): cool.Matrix2D {
		const turn = SelectionSection.turnOfTheMapping(mapping);
		const cos = Math.cos(turn);
		const sin = Math.sin(turn);
		const frame = new cool.Matrix2D(cos, sin, -sin, cos, around.x, around.y);
		const back = frame.invert();
		const across = mapping.c * cos + mapping.d * sin;
		const down = -mapping.c * sin + mapping.d * cos;
		if (!back || !down) return cool.Matrix2D.IDENTITY;

		const length = Math.hypot(across, down);
		const upright = new cool.Matrix2D(
			1,
			0,
			-across / down,
			(Math.sign(down) * length) / down,
			0,
			0,
		);
		return back.then(upright).then(frame);
	}

	/*
		The angle the selection stands at, as the canvas counts angles: the one the turn keys left
		it at, where they did; for several objects or a group the turn they all share, where they
		do; otherwise the one its square stands at, which for a single object also knows whether
		it was turned over.
	*/
	public turnOfTheSelection(): number {
		const byKeys = TurningInteraction.keysStillHold(this);
		if (byKeys) return (-byKeys.stands * Math.PI) / 180;

		const shared = this.isOneObject() ? null : this.anglesOf('turn').shared;
		return shared ?? this.squareNow()?.rotation() ?? 0;
	}

	/*
		The change the running interaction stands for, and whether it moves only the window of a
		picture. Nothing while nothing runs.
	*/
	public runningChange(): { matrix: cool.Matrix2D; cropOnly: boolean } | null {
		const matrix = this.interaction?.transformation();
		if (!matrix || !this.interaction) {
			if (this.cropAwaitingAnswer)
				return { matrix: this.cropAwaitingAnswer.matrix, cropOnly: true };
			return null;
		}

		return { matrix, cropOnly: this.interaction.isACrop() };
	}

	/// Takes in that the engine answered for that object, or for anything given no object, so a
	/// crop that waited for it is over.
	public theEngineAnswered(objectId?: number): void {
		let changed = false;
		if (
			this.cropAwaitingAnswer &&
			(objectId === undefined || objectId === this.cropAwaitingAnswer.objectId)
		) {
			this.cropAwaitingAnswer = null;
			changed = true;
		}
		if (objectId === undefined && this.connectorsAwaitingAnswer.size) {
			this.connectorsAwaitingAnswer.clear();
			changed = true;
		} else if (
			objectId !== undefined &&
			this.connectorsAwaitingAnswer.delete(objectId)
		)
			changed = true;

		if (changed) this.tellWhoStandsIn();
	}

	/*
		Tells the drawing of the page which objects this section draws in its place: the picture
		while a crop preview shows it as the crop leaves it, and none otherwise. The page is drawn
		before this section, so it is told whenever the preview comes or goes rather than asked
		while drawing.
	*/
	public tellWhoStandsIn(): void {
		const standsIn =
			this.runningChange()?.cropOnly === true && GraphicSelection.cropping;
		const objectId = this.selectedObjects()[0];
		const drawnHere = [...this.connectorsAwaitingAnswer];
		if (standsIn && objectId !== undefined) drawnHere.push(objectId);
		RenderManager.drawnElsewhere(drawnHere);
	}

	/// The objects a change here moves: the selected ones, the groups above them and what they
	/// draw, so a connector tied to any of them follows.
	private theOnesAChangeMoves(): Set<number> {
		const selected = this.selectedObjects();
		return new Set([
			...selected,
			...RenderGeometrySection.theOnesThatDraw(selected),
			...RenderGeometrySection.theOnesAChangeTouches(selected),
		]);
	}

	/*
		Draws the run of every connector tied to what a change moves, routed here from where the
		objects stand with the mapping laid over them, as a hairline in the drag's color: the
		drag of an object shows its connectors going along, and a change already laid on shows
		them where they will be until the engine answers for them.
	*/
	/// The connectors a change here lays out anew: the tied ones among what it moves, and the
	/// ones tied to anything it moves.
	private theConnectorsAChangeRoutes(
		data: cool.VectorPrimitivesData,
		moved: Set<number>,
	): number[] {
		const routed = new Set(ConnectorRouter.tiedTo(data, [...moved]));
		for (const objectId of moved)
			if (ObjectHandles.isATiedConnector(objectId)) routed.add(objectId);
		return [...routed];
	}

	private drawTheTiedConnectors(
		data: cool.VectorPrimitivesData,
		matrix: cool.Matrix2D | null,
		only?: Set<number>,
	): void {
		const moved = this.theOnesAChangeMoves();
		for (const connectorId of this.theConnectorsAChangeRoutes(data, moved)) {
			if (only && !only.has(connectorId)) continue;
			const connector = data.objects.get(connectorId);
			if (!connector) continue;
			const run = ConnectorRouter.routeWithMoved(
				data,
				connector,
				moved,
				matrix,
			);
			if (run)
				ConnectorRouter.strokeRun(this.context, run, HandleLook.outlineOfADrag);
		}
	}

	/// Draws the connectors whose change was laid on here and not answered yet, where the laid
	/// objects now put them.
	private drawTheConnectorsAwaitingAnswer(): void {
		if (!this.connectorsAwaitingAnswer.size) return;
		const data = RenderGeometrySection.currentPart();
		if (!data) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		const origin = new cool.SimplePoint(0, 0);
		this.context.translate(origin.vX, origin.vY);
		this.context.scale(app.twipsToPixels, app.twipsToPixels);
		this.drawTheTiedConnectors(data, null, this.connectorsAwaitingAnswer);
		this.context.restore();
	}

	/*
		Draws what a running interaction would leave behind: the objects of the selection, as
		they draw themselves, with the transformation it stands for over them. Half see-through,
		so that what lies under them stays readable.
	*/
	private drawTheInteraction(): void {
		const matrix = this.interaction?.ghostTransformation();
		if (!matrix) return;

		const data = RenderGeometrySection.currentPart();
		if (!data) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		// The point at the very start of the document says where the drawing of the part begins
		// on the canvas, and a twip is that many pixels wide.
		const origin = new cool.SimplePoint(0, 0);
		this.context.translate(origin.vX, origin.vY);
		this.context.scale(app.twipsToPixels, app.twipsToPixels);
		this.context.globalAlpha = 0.5;

		// A tied connector is not moved as a whole. It is routed below, as a hairline.
		RenderManager.renderObjectsWith(
			this.context,
			data,
			RenderGeometrySection.theOnesThatDraw(
				RenderGeometrySection.standingFor(this.selectedObjects()),
			).filter((objectId: number) => !ObjectHandles.isATiedConnector(objectId)),
			matrix,
		);

		// The connectors tied to what moves go along, as hairlines.
		this.context.globalAlpha = 1;
		this.drawTheTiedConnectors(data, matrix);

		this.context.restore();
	}

	/// Which objects the selection holds, so that an interaction can draw them.
	public abstract selectedObjects(): number[];

	/*
		The pointer over a handle: the direction that handle scales in, the hand that takes hold
		of one that shapes the object, the hand that turns the selection, and the sign that says
		no over an object that cannot be resized.
	*/
	/// The hand that takes hold of the object itself, or of a handle that shapes it.
	private static grabPointer(): string {
		return (
			'url(' + app.LOUtil.getURL('images/cursors/grab.svg') + ') 12 12, grab'
		);
	}

	private pointerOver(handle: SelectionHandle): string {
		if (handle.kind === SelectionSection.turningKind) return 'pointer';

		if (handle.kind === CropHandles.middleKind) return 'move';

		if (handle.group === 'crop')
			return (
				'url(' + app.LOUtil.getURL('images/cursors/crop.svg') + ') 8 8, auto'
			);

		if (handle.group !== 'framing') return SelectionSection.grabPointer();

		if (!ObjectHandles.canBeResized(this.selectedObjects()))
			return 'not-allowed';

		const byKind: Record<string, string> = {
			'1': 'nwse-resize',
			'2': 'ns-resize',
			'3': 'nesw-resize',
			'4': 'ew-resize',
			'5': 'ew-resize',
			'6': 'nesw-resize',
			'7': 'ns-resize',
			'8': 'nwse-resize',
		};

		return byKind[handle.kind] ?? 'default';
	}

	/*
		The five places the handles that frame a single object stand at, worked out from the
		mapping that object is drawn by. They are the same places the handles hold, only whole:
		a handle lies on a whole twip, and the fraction rounding takes off tilts the two axes the
		object is scaled along, which leaves a shear behind that the object never had. Nothing
		for a selection of several, which is framed by a box and not by a mapping.
	*/
	private framingPlacesOfTheObject():
		| { [where: string]: cool.SimplePoint }
		| undefined {
		const objects = this.selectedObjects();
		if (objects.length !== 1) return undefined;

		const transform = RenderGeometrySection.transformOf(objects[0]);
		if (!transform) return undefined;

		const at = (x: number, y: number): cool.SimplePoint => {
			const place = transform.apply(x, y);

			return new cool.SimplePoint(place.x, place.y);
		};

		return {
			middle: at(0.5, 0.5),
			above: at(0.5, 0),
			below: at(0.5, 1),
			left: at(0, 0.5),
			right: at(1, 0.5),
		};
	}

	/*
		The shape of the selection as the maths of a drag wants it: the middle it turns around,
		how wide and how high it is in core pixels, and the angle it stands at, counted in
		radians against the clock. All four come from the handles that frame it, so nothing else
		has to be asked for them.
	*/
	public shapeNow(): any {
		const handleOfKind = (kind: string): cool.SimplePoint | undefined => {
			const handle = this.known.find(
				(one: SelectionHandle) => one.kind === kind,
			);
			return handle
				? new cool.SimplePoint(handle.point.x, handle.point.y)
				: undefined;
		};

		// The handles as the objects have them, not as a drag shows them: this says what the
		// selection is now, which is what a drag starts from.
		const whole = this.framingPlacesOfTheObject();
		const middle = whole?.middle ?? SelectionSection.middleOf(this.known);
		const above = whole?.above ?? handleOfKind('2');
		const below = whole?.below ?? handleOfKind('7');
		const left = whole?.left ?? handleOfKind('4');
		const right = whole?.right ?? handleOfKind('5');
		if (!middle || !above || !below || !left || !right) return undefined;

		/*
			An object that stands straight still answers with a hundredth of a degree of turn
			where the places come from the handles, so the turn is taken as none when it is that
			small.
		*/
		const turned =
			Math.atan2(middle.y - above.y, above.x - middle.x) - Math.PI * 0.5;
		const straight = Math.abs(turned) < 0.0005;

		return {
			center: middle.clone(),
			width: left.pDistanceTo(right.pToArray()),
			height: above.pDistanceTo(below.pToArray()),
			angleRadian: straight ? 0 : turned,
		};
	}

	/// Where the mouse is in the document, in core pixels, from a point this section was given.
	private inTheDocument(point: cool.SimplePoint): cool.SimplePoint {
		const inDocument = point.clone();

		inDocument.pX += this.position[0];
		inDocument.pY += this.position[1];

		return inDocument;
	}

	/// Looks for something on the page that the handle could line up with, and marks it.
	public lookForASnap(handle: SelectionHandle, event?: MouseEvent): void {
		const half = 0.5 * ShapeHandlesSection.handleSize();
		const at = new cool.SimplePoint(handle.point.x, handle.point.y);

		// A point is asked about, so there is no size to it and no distance left to travel.
		this.snap.look(
			[0, 0],
			[at.pX - half, at.pY - half],
			[0, 0],
			event?.altKey === true,
		);
		this.redraw();
	}

	/// Where the last look landed, in core pixels, null on an axis that found nothing.
	public snappedTo(): (number | null)[] {
		return this.snap.at();
	}

	/*
		Looks for something on the page that the whole selection could line up with, from where
		the move has taken it, and marks it. The distance is in core pixels.
	*/
	public lookForASnapOfTheWhole(
		across: number,
		down: number,
		event?: MouseEvent,
	): void {
		this.snap.look(
			this.selectionSize(),
			this.selectionCorner(),
			[across, down],
			event?.altKey === true,
		);
	}

	/// Where the upper left corner of the selection lands, in core pixels: where the move puts
	/// it, or where it lined up with something on the page.
	public snappedCorner(x: number, y: number): number[] {
		return this.snap.corner(x, y);
	}

	/// The box those objects all fit in, in twips, or nothing where none of them carries one.
	private boxAround(objectIds: number[]): cool.Range2D | undefined {
		let box: cool.Range2D | undefined;
		for (const objectId of objectIds) {
			const one = RenderGeometrySection.boxOf(objectId);
			if (one) box = box ? box.union(one) : one;
		}

		return box;
	}

	/*
		The box the selection takes up, in core pixels. While the document is drawn from objects
		it is worked out from the boxes they carry, which is the same box the handles are worked
		out from; otherwise it is the one the engine sends with the selection.
	*/
	private boxOfTheSelection(): cool.Range2D | undefined {
		const box = this.boxAround(
			RenderGeometrySection.theOnesThatDraw(this.selectedObjects()),
		);
		if (!box) return undefined;

		const scale = app.twipsToPixels;

		return new cool.Range2D(
			box.minX * scale,
			box.minY * scale,
			box.maxX * scale,
			box.maxY * scale,
		);
	}

	/*
		Whether a change leaves every object it touches somewhere on this side of reason: no
		place and no side longer than a hundred metres. A change worked out on a state that has
		gone stale can ask for far more than that, and a mapping like that is worth neither
		drawing nor sending - the engine cannot even work out what to invalidate for it.
	*/
	public static withinReason(
		objectIds: number[],
		matrix: cool.Matrix2D,
	): boolean {
		const most = 100 * 1440 * 39.37;

		return RenderGeometrySection.theOnesThatDraw(objectIds).every(
			(objectId: number) => {
				const mapping = RenderGeometrySection.transformOf(objectId);
				if (!mapping) return true;

				return mapping
					.then(matrix)
					.toArray()
					.every(
						(value: number) => Number.isFinite(value) && Math.abs(value) < most,
					);
			},
		);
	}

	/// The upper left corner of the selection, in core pixels: the box round what is drawn of
	/// the selected objects, or where this section stands while no object is known.
	public selectionCorner(): number[] {
		const box = this.boxOfTheSelection();
		return box ? [box.minX, box.minY] : this.position;
	}

	/// How wide and how high the selection is, in core pixels: the box round what is drawn of
	/// the selected objects, or the size of this section while no object is known.
	public selectionSize(): number[] {
		const box = this.boxOfTheSelection();
		return box ? [box.width, box.height] : this.size;
	}

	/*
		How far a key turns the selection, in degrees: one of them, fifteen with Shift, which is
		the step the office holds a turn to, and a tenth with Alt for the finest of it.
	*/
	private static turningStep(event: KeyboardEvent): number {
		if (event.shiftKey) return 15;
		if (event.altKey) return 0.1;
		return 1;
	}

	/*
		Hands the engine what a drag or a key did to the points of a path: the object it belongs
		to and one item for every point or weight that moved. The points that did not move are
		not named at all, so a path of many thousand points still travels as the few numbers that
		changed, and the ones left alone come back exactly as they went out.
	*/
	public static sendPathChanges(objectId: number, changes: string[]): void {
		if (!changes.length) return;

		app.socket.sendMessage(
			'setobjectpoints id=' +
				String(objectId) +
				' changes=' +
				changes.join(';'),
		);
	}

	/*
		Moves the objects one step the way an arrow key says, worked out here and sent to the
		engine as the one change it is, the way a drag is. The step is the key's: a millimetre,
		ten with Shift, a pixel as seen with Alt. Objects that may not be moved stay where they
		are.
	*/
	public moveByKey(towards: cool.Point, event: KeyboardEvent): void {
		if (!ObjectHandles.canBeMoved(this.selectedObjects())) return;

		const step = HandleTravel.stepFor(event);
		const by = this.heldInsideTheLawn(
			new cool.Point(towards.x * step, towards.y * step),
		);
		if (!by.x && !by.y) return;

		const matrix = cool.Matrix2D.IDENTITY.translate(by.x, by.y);

		SelectionSection.sendTransform(this.selectedObjects(), matrix, 'move');
		this.layTheChangeOn(matrix);

		// The view follows every step, a key held down included.
		GraphicSelection.showTheObjects(this.selectedObjects());
	}

	/*
		How far the selection may be moved of that much, in twips, so that its box does not go
		past the lawn the page gives, and no object goes where it cannot be reached. The lawn the
		view shows grows round objects lying further out, so it is not what is held to, or a
		move would push it on and on. A move back towards the page is never held: a selection that
		lies outside already may come in, and is not pulled in by the hold.
	*/
	public heldInsideTheLawn(by: cool.Point): cool.Point {
		const lawn = ViewLayoutVector.lawnOfThePageInView();
		const box = this.boxAround(
			RenderGeometrySection.theOnesThatDraw(this.selectedObjects()),
		);
		if (!lawn || !box) return by;

		// On an axis where the selection is larger than the lawn it moves freely.
		const held = (
			move: number,
			least: number,
			most: number,
			larger: boolean,
		): number =>
			larger
				? move
				: Math.min(Math.max(move, Math.min(least, 0)), Math.max(most, 0));

		return new cool.Point(
			held(
				by.x,
				lawn.minX - box.minX,
				lawn.maxX - box.maxX,
				box.width > lawn.width,
			),
			held(
				by.y,
				lawn.minY - box.minY,
				lawn.maxY - box.maxY,
				box.height > lawn.height,
			),
		);
	}

	/// Lays a change a key made over the objects, as the end of a drag does with its own.
	public layTheChangeOn(matrix: cool.Matrix2D | null): void {
		this.leaveTheChangeOnTheObjects(matrix);
	}

	/*
		The selection's own square: the mapping of the unit square onto what is selected, made
		from the objects when it is first asked for and carried through every change made here
		since. It remembers what the objects cannot: a group's or a text frame's mapping comes
		back from the engine upright, since neither can hold a mirror, and the square stays
		turned over as it was made, so the handle at the edge being moved keeps its name. It is
		dropped when the selection changes, and when the engine answers something for the
		selection that was not predicted here, since then it may no longer fit what is drawn.
	*/
	private square: cool.Matrix2D | null = null;

	public squareNow(): cool.Matrix2D | undefined {
		// After a change that turned or sheared several objects or a group, and until the engine
		// has answered for all it touched, the frame is their upright box, made afresh each time.
		if (this.frameFollowsTheObjects) {
			if (this.isWaitingForTheEngine()) {
				const box = this.uprightBoxOfTheObjects();
				return box ? cool.Matrix2D.fromRange(box) : undefined;
			}
			this.frameFollowsTheObjects = false;
			this.square = null;
		}
		if (this.square) return this.square;

		const objects = this.selectedObjects();
		if (!objects.length) return undefined;

		// One object or one group is framed by its own mapping from the engine, several objects
		// by the upright box round them.
		if (objects.length === 1) {
			this.square = RenderGeometrySection.transformOf(objects[0]);
			return this.square ?? undefined;
		}

		const box = this.uprightBoxOfTheObjects();
		this.square = box ? cool.Matrix2D.fromRange(box) : null;
		return this.square ?? undefined;
	}

	/// The upright box round the objects that draw for the selection, in twips, or nothing where
	/// it has no size.
	private uprightBoxOfTheObjects(): cool.Range2D | undefined {
		const box = this.boxAround(
			RenderGeometrySection.theOnesThatDraw(this.selectedObjects()),
		);
		return box && box.width && box.height ? box : undefined;
	}

	public forgetTheSquare(): void {
		this.square = null;
		this.frameFollowsTheObjects = false;
	}

	/*
		Whether the frame of several objects or a group is the upright box round them, after a
		change that turned or sheared them, until the engine has answered for it.
	*/
	private frameFollowsTheObjects: boolean = false;

	/// Whether a change laid on here still waits for the engine's answer for anything it touched:
	/// the objects that draw and the groups above them.
	private isWaitingForTheEngine(): boolean {
		return RenderGeometrySection.theOnesAChangeTouches(
			this.selectedObjects(),
		).some((objectId: number) =>
			RenderGeometrySection.awaitsAnAnswer(objectId),
		);
	}

	/*
		Whether that change leaves the frame upright rather than taking it along: where it turns
		or shears several objects or a group, rather than only moving, scaling or mirroring them
		along the page's own axes.
	*/
	private keepsAnUprightFrame(matrix: cool.Matrix2D | null): boolean {
		if (!matrix || this.isOneObject()) return false;
		return Math.abs(matrix.b) > 1e-9 || Math.abs(matrix.c) > 1e-9;
	}

	/*
		Lays what a drag did over the objects the client holds, so that everything reads them as
		the drag left them. The engine's own version of an object replaces this one when it
		arrives, so a change it made differently puts itself right then.
	*/
	private leaveTheChangeOnTheObjects(matrix: cool.Matrix2D | null): void {
		if (!matrix) return;
		if (!SelectionSection.withinReason(this.selectedObjects(), matrix)) return;

		/*
			The square goes through the change along with the objects. Several objects or a group
			keep an upright frame, though: after a change that turns or shears them it is the
			upright box round the objects, as their band is, until the engine has answered and the
			square is made from what it says.
		*/
		const keepsUpright = this.keepsAnUprightFrame(matrix);
		const square = this.squareNow();
		if (keepsUpright) {
			this.frameFollowsTheObjects = true;
			this.square = null;
		} else if (square) this.square = square.then(matrix);

		/*
			The change goes to the objects that draw, and to the groups above them as well: a
			group's mapping and box are read for the next change before the engine has answered
			this one, so they are kept current here just as the members are.
		*/
		const data = RenderGeometrySection.currentPart();
		for (const objectId of RenderGeometrySection.theOnesAChangeTouches(
			this.selectedObjects(),
		))
			if (!ObjectHandles.isATiedConnector(objectId))
				RenderGeometrySection.changeObject(objectId, matrix);

		/*
			A tied connector cannot take the change as a mapping, whether it was moved itself or
			hangs on something that was: its run is laid out anew. Such connectors are drawn here
			from the laid objects until the engine answers for them, and the page leaves them out
			meanwhile.
		*/
		if (data) {
			const moved = this.theOnesAChangeMoves();
			for (const connectorId of this.theConnectorsAChangeRoutes(data, moved))
				this.connectorsAwaitingAnswer.add(connectorId);
			this.tellWhoStandsIn();
		}

		// The handles are laid on that upright frame at once.
		if (keepsUpright) this.known = this.handlesOfTheGroups();

		app.sectionContainer?.requestReDraw();
	}

	/// What a running drag does to the objects, or nothing while nothing is being dragged.
	public dragMatrix(): cool.Matrix2D | null {
		return this.interaction?.transformation() ?? null;
	}

	/// Where a drag of that handle may take it, which its own group says. The end of a connector
	/// snaps onto the point it would tie itself to, so that it is let go exactly there.
	public holdTheDrag(handle: SelectionHandle, at: cool.Point): cool.Point {
		if (SelectionSection.connectorEnds.includes(handle.name)) {
			const would = this.whereTheEndWouldTie(at);
			if (would?.point) return would.point.clone();
		}

		return this.groupOf(handle)?.holdInside(handle, at) ?? at;
	}

	/// Hands the engine what a drag of that handle did, which its own group knows how to say.
	public handOverTheDrag(handle: SelectionHandle, to: cool.Point): void {
		this.groupOf(handle)?.handOver(handle, to);
	}

	/*
		Hands the engine what a drag did to the objects as a whole: for every object the mapping
		it is to be drawn by, which is the mapping it was drawn by here while the drag ran. The
		last word says what the drag was, which is what names the step it can be undone with.

		The mapping travels whole rather than as the change alone, because the client is the one
		that holds what the object is drawn by: the engine would have to read that again and the
		two could then disagree.

		Where mappings are given for objects, the change is laid over those instead of over what
		the objects are drawn by now.
	*/
	public static sendTransform(
		objectIds: number[],
		matrix: cool.Matrix2D,
		what: string,
		from?: Map<number, cool.Matrix2D>,
	): void {
		if (!SelectionSection.withinReason(objectIds, matrix)) return;

		// A change worked out from what the objects are drawn by now ends a slant keys gave, and
		// any change ends a turn they gave, which a turn by a key takes up again after it.
		if (!from) ShearingInteraction.forgetTheKeys();
		TurningInteraction.forgetTheKeys();

		/*
			A group is named to the engine as the objects it holds. It has no mapping of its own
			to be given - the drawing layer keeps only a rectangle for it, and hands a turn on to
			what is inside it - so naming the group would lose the turn. Naming the objects
			cannot: each of them has a mapping and takes it whole.
		*/
		const reached = new Map<number, cool.Matrix2D>();
		for (const objectId of RenderGeometrySection.theOnesThatDraw(
			RenderGeometrySection.standingFor(objectIds),
		)) {
			const mapping =
				from?.get(objectId) ?? RenderGeometrySection.transformOf(objectId);

			// The change the drag stands for, laid over what the object is drawn by now.
			if (mapping) reached.set(objectId, mapping.then(matrix));
		}

		SelectionSection.sendMappings(reached, what);
	}

	/*
		Hands the engine the mapping each of those objects is to be drawn by, as one change named
		by the last word.

		A mapping that turns the object over travels whole, turn over and all. The object lays that
		out itself, and it does it in its own frame: it builds its rectangle unturned at the origin,
		mirrors it there, and only then shears, turns and moves it. Asking for the turn over apart
		from the mapping would mean naming a line for it, and a line that does not lie along one of
		the page's own two is not one the drawing layer can follow.
	*/
	public static sendMappings(
		reached: Map<number, cool.Matrix2D>,
		what: string,
	): void {
		const mappings: string[] = [];
		for (const [objectId, mapping] of reached)
			mappings.push(
				String(objectId) +
					',' +
					mapping
						.toArray()
						.map((value: number) => value.toFixed(4))
						.join(','),
			);

		if (!mappings.length) return;

		app.socket.sendMessage(
			'setobjecttransform to=' + mappings.join(';') + ' as=' + what,
		);
	}

	/*
		Moves a handle as a key asks, which is the business of the group the handle belongs to:
		one of the eight scales the selection, the one above it turns it, a point of a path moves
		that point. Answers where the handle lands, and nothing where it goes nowhere of its own.
	*/
	private moveHandleBy(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.SimplePoint | null {
		/*
			The group sends the change and lays it on the objects itself, so the handles are
			worked out afresh before this has the place the key is taking them to. Nothing is
			followed while that happens: the place the step before left says nothing about where
			the handles stand now, and following it sends the keyboard to the handle facing the
			one being moved.
		*/
		this.keyLeftTheHandle = null;

		const to = this.groupOf(handle)?.moveByKey(handle, towards, event) ?? null;

		// Only one of the eight can end up facing the other way, and only it is followed by
		// place. Anything else stays with the keyboard by name.
		this.keyLeftTheHandle =
			to && handle.group === 'framing' ? { at: to, named: handle.name } : null;

		return to ? new cool.SimplePoint(to.x, to.y) : null;
	}

	/*
		Keeps the keyboard on the handle that stands where the key left it, rather than on the
		one that keeps its name. They are the same handle until a change turns the object over,
		and then the object's square is laid down the other way and the two part company: going
		on in the same direction would otherwise push back the side that was just crossed.

		It works the same whether the handles were worked out from the change laid on here or
		from the one the engine answered with, which are not the same thing and need not be: the
		place is what the keyboard follows, and both of them put a handle there.
	*/
	private followTheHandleByPlace(): void {
		const left = this.keyLeftTheHandle;
		if (!left) return;

		// The keyboard being somewhere else means it was sent there, by a press or by walking
		// on, and then the place a key left behind says nothing any more.
		const active = this.travel.whichIsActive();
		if (active !== left.named) {
			this.keyLeftTheHandle = null;
			return;
		}

		/*
			Only the handle itself and the ones it reflects onto are candidates. A change that
			turns the object over is the one thing that can put another handle where this one
			was, and it can only ever put one of those there: the handle in the middle of the
			upper side can become the one in the middle of the lower side and nothing else.
		*/
		const couldBe = ObjectHandles.facing(
			this.known.find((one: SelectionHandle) => one.name === active)?.kind ??
				'',
		);

		let nearest: SelectionHandle | null = null;
		let howFar = Number.POSITIVE_INFINITY;
		for (const one of this.known) {
			if (one.group !== 'framing' || !couldBe.includes(one.kind)) continue;

			const away = one.point.distanceTo(left.at);
			if (away < howFar) {
				howFar = away;
				nearest = one;
			}
		}

		if (!nearest || nearest.name === active) return;

		this.travel.goTo(nearest.name);
		this.keyLeftTheHandle = { at: left.at, named: nearest.name };
	}

	/*
		Carrying the selection through a press that landed before this section was there: on an
		object nobody had selected, which the press selects. The section that took that press
		drives it, since the mouse belongs to that one until it is let go.
	*/
	public beginCarrying(at: cool.SimplePoint): void {
		this.interaction = new MovingInteraction(this, at);
	}

	/// Carries it on to that point of the document, as a drag of this section's own does, the
	/// view scrolling along at its edge.
	public carryTo(to: cool.SimplePoint): void {
		this.dragTo(to, new cool.Point(to.vX / app.dpiScale, to.vY / app.dpiScale));
	}

	public finishCarrying(to: cool.SimplePoint): void {
		this.interaction?.finish(to);
		this.endTheInteraction(true);
	}

	public cancelCarrying(): void {
		this.interaction?.cancel();
		this.endTheInteraction(false);
	}

	/*
		Where the mouse last was during a drag, and the event that took it there. A change of
		Shift or Alt while the mouse stands still is taken up from these: the drag is moved to
		the same place again with the keys as they are now.
	*/
	private lastMoved: { at: cool.SimplePoint; event?: MouseEvent } | null = null;

	private readonly keysChanged = (e: KeyboardEvent): void => {
		if (!this.interaction || !this.lastMoved) return;
		if (!['Shift', 'Alt', 'Control'].includes(e.key)) return;

		// The event that lets a key go can still carry that key as held, so the key it names
		// is taken as let go whatever its flag says.
		const letGo = e.type === 'keyup' ? e.key : null;

		this.interaction.move(this.lastMoved.at, {
			shiftKey: letGo !== 'Shift' && e.shiftKey,
			altKey: letGo !== 'Alt' && e.altKey,
			ctrlKey: letGo !== 'Control' && e.ctrlKey,
		} as MouseEvent);
	};

	onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		const at = this.inTheDocument(point);
		const onCanvas = [
			this.myTopLeft[0] + point.pX,
			this.myTopLeft[1] + point.pY,
		];
		const handle = this.handleAt(onCanvas[0], onCanvas[1]);

		this.pressedAHandle = handle !== undefined;
		this.pressedHandle = handle ?? null;

		/*
			The group that owns the handle begins the drag of it. A press that reached no handle
			goes to the last group that answers for the place it landed on, which is the object
			itself where it lands on the object.
		*/
		if (handle) {
			this.pressedGroup = this.groupOf(handle) ?? null;
			this.interaction = this.pressedGroup?.interactionFor(handle, at) ?? null;

			// A drag leads the handle, so it is told where inside the handle the press landed
			// and can set off from where the handle stands rather than from the mouse.
			this.interaction?.tookHoldAt(at, handle);
		} else {
			this.pressedGroup =
				this.groups().find((group: HandleGroup) =>
					group.takesAPress(onCanvas[0], onCanvas[1]),
				) ?? null;
			this.interaction = this.pressedGroup?.interactionForThePlace(at) ?? null;
		}

		if (this.interaction) {
			this.lastMoved = null;

			// The handle of the other mode goes with the press, and one waiting to come up.
			this.dropTheOtherMode();

			document.addEventListener('keydown', this.keysChanged, true);
			document.addEventListener('keyup', this.keysChanged, true);
		}

		this.stopPropagating();
		e.stopPropagation();
	}

	onMouseMove(point: cool.SimplePoint, dragDistance: number[], e: MouseEvent) {
		if (!this.containerObject.isDraggingSomething() || !this.interaction) {
			const onCanvas = [
				this.myTopLeft[0] + point.pX,
				this.myTopLeft[1] + point.pY,
			];
			let over = this.handleAt(onCanvas[0], onCanvas[1]);
			if (
				this.followTheOtherMode(over, new cool.Point(onCanvas[0], onCanvas[1]))
			)
				over = undefined;

			// Off a handle but still on the object, the pointer is the object's again, or the
			// one it left on a handle would stay: over text the text pointer, the hand where the
			// selection may be carried, the plain pointer where it may not.
			const onTheObject = this.groups().some((group: HandleGroup) =>
				group.takesAPress(onCanvas[0], onCanvas[1]),
			);
			const inTheDocument = this.inTheDocument(point);
			const overText =
				RenderGeometrySection.pointerAt(inTheDocument.x, inTheDocument.y) ===
				'text';
			this.context.canvas.style.cursor = over
				? this.pointerOver(over)
				: !onTheObject
					? 'default'
					: overText
						? 'text'
						: ObjectHandles.canBeMoved(this.selectedObjects())
							? SelectionSection.grabPointer()
							: 'default';

			// The handle under the mouse is marked, so that it is plain which one a press would
			// take hold of where several lie close together.
			// On the bar to the handle of the other mode the one it stands beside stays marked.
			const hovered =
				over?.name ??
				(this.otherModeShown ? SelectionSection.turningKind : null);
			if (hovered !== this.hovered) {
				this.hovered = hovered;
				this.containerObject.requestReDraw();
			}
			return;
		}

		this.stopPropagating();
		e.stopPropagation();

		const onCanvas = point.clone();
		onCanvas.pX += this.myTopLeft[0];
		onCanvas.pY += this.myTopLeft[1];
		this.dragTo(
			this.inTheDocument(point),
			new cool.Point(onCanvas.cX, onCanvas.cY),
			e,
		);
	}

	/*
		Takes the running drag to that point of the document, where the mouse stands at that
		point of the canvas, in CSS pixels, with the keys of that event held.
	*/
	private dragTo(
		at: cool.SimplePoint,
		onCanvas: cool.Point,
		e?: MouseEvent,
	): void {
		if (!this.interaction) return;

		this.lastMoved = { at, event: e };
		this.interaction.move(at, e);
		this.aimAtWhatIsUnder(at);

		/*
			Carrying the objects brings the page along: while the mouse stands near an edge of the
			view, or outside it, the view scrolls on by itself, and it stops of its own accord as
			soon as the mouse is away from the edge again.
		*/
		if (this.interaction.scrollsWithTheMouse()) {
			app.map.fire('handleautoscroll', { pos: onCanvas, map: app.map });
			return;
		}

		/*
			A drag led by a handle brings the view along another way: it is scrolled by the
			shortest way that shows that handle again, once for each move the mouse makes. A view
			that scrolled on by itself would move the page under a mouse standing still, and the
			handle, which follows the mouse on the page, would run away with it.
		*/
		const leading = this.interaction.leadingPoint();
		if (leading) GraphicSelection.scrollPointIntoView(leading);
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (!this.containerObject.isDraggingSomething() || !this.interaction) {
			/*
				A press on a handle that was let go where it started puts the keyboard on that
				handle. It is the short way onto one handle among hundreds - a point of a polygon
				among them - without walking there from the first.
			*/
			if (
				this.pressedHandle?.kind === SelectionSection.turningKind &&
				this.shearingIsOffered()
			) {
				// On the handle above the selection a click switches between turning and
				// shearing instead, where both are offered, and the handle of the other mode
				// comes up beside it again after the wait.
				this.switchTheHandleMode();
				this.interaction = null;
				this.followTheOtherMode(this.pressedHandle);
			} else if (this.pressedHandle?.name)
				this.travel.goTo(this.pressedHandle.name);

			// The press hid the buttons beside the bands as a drag does; a click brings them back.
			this.interaction = null;
			GraphicSelection.followTheGroupButtons();
			return;
		}

		this.stopPropagating();
		e.stopPropagation();

		this.interaction.finish(this.inTheDocument(point), e);
		this.endTheInteraction(true);
	}

	/*
		A press that was not a drag. It is handed to the engine as a press of its own, a moment
		later, so that a press that turns out to be the first half of a double one can still be
		called off: the engine would otherwise have selected something else before the text edit
		of the double press could begin.
	*/
	onClick(point: cool.SimplePoint, e: MouseEvent): void {
		this.stopPropagating();
		e.stopPropagation();

		/*
			A press that took hold of a handle is answered by what that handle does, and by
			nothing else. A short press on one counts as a click all the same, and handing that
			on would tell the engine that the page was pressed where the handle lies - which for
			the handle that turns the selection is beside the object, so the selection would go.
		*/
		if (this.pressedAHandle) return;

		// The group that took the press says whether the engine hears of it: a press on the line
		// of a connector is the connector itself, and it stays selected.
		if (this.pressedGroup && !this.pressedGroup.passesTheClickOn()) return;

		// A click on a selected video wakes its player, so its controls take the presses from
		// now on.
		GraphicSelection.mediaPlayer?.wake();

		const at = this.inTheDocument(point);
		const modifier = MouseControl.readModifier(e);

		this.forgetThePendingClick();

		/*
			A click with Shift or Ctrl adds an object to the selection or takes one away, and that
			is the context's: taking away the last one in a group selects the group, for one. A
			plain click goes on to the engine, which places the text cursor and the like.
		*/
		if (
			(modifier === app.UNOModifier.SHIFT ||
				modifier === app.UNOModifier.CTRL) &&
			RenderGeometrySection.answersPointer() &&
			Interaction.context().press(
				RenderGeometrySection.objectIdAt(at.x, at.y),
				modifier,
			)
		)
			return;

		// Alt goes through the objects under the press, which is the context's as well.
		const alt = app.UNOModifier.ALT;
		if (
			(modifier === alt || modifier === (alt | app.UNOModifier.SHIFT)) &&
			RenderGeometrySection.answersPointer() &&
			Interaction.context().altPress(
				RenderGeometrySection.objectIdsAt(at.x, at.y),
				modifier !== alt,
			)
		)
			return;

		this.pendingClick = app.timerRegistry.setTimeout(
			'selectionClick',
			() => {
				app.map._docLayer._postMouseEvent(
					'buttondown',
					at.x,
					at.y,
					1,
					1,
					modifier,
				);
				app.map._docLayer._postMouseEvent(
					'buttonup',
					at.x,
					at.y,
					1,
					1,
					modifier,
				);
				this.pendingClick = null;
			},
			250,
		);
	}

	/*
		A press of two. The context takes it where it means something it does itself, entering a
		group; otherwise it goes to the engine, which starts the text edit of the object underneath.
	*/
	onDoubleClick(point: cool.SimplePoint, e: MouseEvent): void {
		const at = this.inTheDocument(point);

		this.forgetThePendingClick();

		// With Alt each press is a step through the objects under the mouse, taken on its own.
		if (e.altKey) {
			this.stopPropagating();
			e.stopPropagation();
			return;
		}

		if (
			Interaction.context().doublePress(
				RenderGeometrySection.objectIdAt(at.x, at.y),
			)
		) {
			this.stopPropagating();
			e.stopPropagation();
			return;
		}

		app.map._docLayer._postMouseEvent('buttondown', at.x, at.y, 2, 1, 0);
		app.map._docLayer._postMouseEvent('buttonup', at.x, at.y, 2, 1, 0);

		this.stopPropagating();
		e.stopPropagation();
	}

	/*
		A press of the right button, which asks for the menu of what is selected. The section
		covers the objects while the client draws the selection, so the press would otherwise
		stop here and no menu would open.
	*/
	onContextMenu(point: cool.SimplePoint, e: MouseEvent): void {
		MouseControl.contextMenuFrom(this, point, e);
	}

	/// Drops a press that is waiting to be handed over, because a second one took its place.
	private forgetThePendingClick(): void {
		if (this.pendingClick === null) return;

		app.timerRegistry.clearTimeout(this.pendingClick);
		this.pendingClick = null;
	}

	/// The mouse has left what is selected, so no handle is under it any more.
	onMouseLeave(): void {
		this.followTheOtherMode(undefined);
		if (this.hovered === null) return;

		this.hovered = null;
		this.containerObject.requestReDraw();
	}

	onDragCancel(): void {
		this.interaction?.cancel();
		this.endTheInteraction(false);
	}

	/// Whether an interaction of the mouse is running on the selection.
	public isDragged(): boolean {
		return this.interaction !== null;
	}

	/// The handles the groups work out from the objects as they stand. Every handle is marked
	/// with the group it came from, so that whatever is done to it afterwards is done by the group
	/// that knows it.
	private handlesOfTheGroups(): SelectionHandle[] {
		return this.groups().flatMap((group: HandleGroup) =>
			group
				.handles()
				.map((handle: SelectionHandle) => ({ ...handle, group: group.name })),
		);
	}

	/*
		Keeps what an interaction that was carried out did, until the engine answers with its own
		version: the handles stay where it left them, and the objects are drawn as it showed them.
		For a crop that is the window alone - the picture stays, and the engine answers with both -
		so the crop's preview stands in for the picture as it will be. The window is named against
		the picture as the engine last gave it, so a crop that follows before the answer names its
		whole window again rather than a step on from this one.
	*/
	public keepWhatItDid(interaction: SelectionInteraction): void {
		// A crop is named against the picture as the engine gave it, and so are the handles it
		// moves: the ones worked out from that picture, not the ones a crop before it moved.
		// Several objects or a group turned or sheared get their upright frame, not the drag's.
		const ghost = interaction.ghostTransformation();
		if (!this.keepsAnUprightFrame(ghost))
			this.known = interaction.handles(
				interaction.isACrop() ? this.handlesOfTheGroups() : this.known,
			);
		this.leaveTheChangeOnTheObjects(ghost);

		const window = interaction.transformation();
		const objectId = this.selectedObjects()[0];
		if (interaction.isACrop() && window && objectId !== undefined) {
			this.cropAwaitingAnswer = { objectId, matrix: window };
			this.tellWhoStandsIn();
			this.containerObject.requestReDraw();
		}
	}

	/*
		Lets the interaction go and takes the lines that marked a snap away with it.

		Where the drag was carried out, the handles stay where it left them. The engine answers a
		moment later and the objects move then; falling back to where they still are would show
		the handles jumping back and forward again. A drag that was called off leaves them where
		the objects have them, which is where they belong.
	*/
	private endTheInteraction(carriedOut: boolean): void {
		/*
			What the drag did stays on the objects until the engine answers with its own version of
			them. The client worked the change out and has already sent it, so waiting for it to
			come back would mean showing the objects as they were for as long as that takes, and
			every handle would jump back with them.
		*/
		if (carriedOut && this.interaction) this.keepWhatItDid(this.interaction);

		// A slant keys gave goes on from where the drag left the objects.
		if (carriedOut) ShearingInteraction.forgetTheKeys();

		this.interaction = null;
		this.tellWhoStandsIn();
		GraphicSelection.followTheGroupButtons();
		this.lastMoved = null;
		document.removeEventListener('keydown', this.keysChanged, true);
		document.removeEventListener('keyup', this.keysChanged, true);
		this.snap.forget();

		// Nothing is aimed at once the drag is over, so the places to tie to go with it.
		this.aimingAt = null;

		app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
		this.containerObject.requestReDraw();
	}
}
