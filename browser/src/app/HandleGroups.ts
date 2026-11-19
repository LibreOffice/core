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

/*
	Whether the points of one kind are shown, and whether the next press on the object adds one.
	The points are shown for one selection, named by the key of its objects. A new selection
	starts with them hidden. Each of the two states is also the state of a command.
*/
class ShownPoints {
	/// The key of the selection whose points are shown, or null when they are hidden.
	private shownFor: string | null = null;

	/// Whether the next press on the object adds a point. It is always false when no points are
	/// shown.
	private beingAdded: boolean = false;

	/// The states last told to the toolbars.
	private shownToldAs: boolean = false;
	private addingToldAs: boolean = false;

	/// The command whose state is whether the points are shown, and the command whose state is
	/// whether the next press adds a point.
	public readonly showCommand: string;
	public readonly addCommand: string;

	constructor(showCommand: string, addCommand: string) {
		this.showCommand = showCommand;
		this.addCommand = addCommand;
	}

	/// Whether the points are shown for the selection of that key.
	public areShownFor(key: string): boolean {
		return this.shownFor === key;
	}

	/// Whether the next press on the object of that selection adds a point.
	public areAddedFor(key: string): boolean {
		return this.beingAdded && this.areShownFor(key);
	}

	/// Shows the points for the selection of that key, or hides them for null.
	public showFor(key: string | null): void {
		this.shownFor = key;
		if (key === null) this.beingAdded = false;
	}

	/// Makes the next press on the object add a point.
	public startAdding(): void {
		this.beingAdded = true;
	}

	/// Makes the next press on the object add no point. Returns whether a point was being added.
	public stopAdding(): boolean {
		if (!this.beingAdded) return false;

		this.beingAdded = false;
		return true;
	}

	/// Tells the toolbars whether the points are shown and whether a point is being added. Each
	/// state is told only when it changed.
	public tell(shown: boolean, adding: boolean): void {
		if (shown !== this.shownToldAs) {
			this.shownToldAs = shown;
			app.map.fire('commandstatechanged', {
				commandName: this.showCommand,
				state: shown ? 'true' : 'false',
			});
		}

		if (adding !== this.addingToldAs) {
			this.addingToldAs = adding;
			app.map.fire('commandstatechanged', {
				commandName: this.addCommand,
				state: adding ? 'true' : 'false',
			});
		}
	}
}

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

	/// The element that the menu of a point opens from, or null before any menu opened. Only one
	/// such menu is open at a time, so they all share this element.
	private static menuAnchor: HTMLElement | null = null;

	/*
		The callback of a menu of a handle. When an entry is chosen, chosen gets its id and answers
		whether the menu stays open. Otherwise the menu and its submenus close. A button of a grid
		in the menu reports a click with its command, which names the entry it stands for.
	*/
	private static menuCallback(
		chosen?: (id: string) => boolean | void,
	): JSDialogMenuCallback {
		return (
			_objectType: string,
			eventType: string,
			_object: unknown,
			data: unknown,
			entry: JSBuilder | MenuDefinition,
		): boolean => {
			let id: string;
			if (eventType === 'selected') id = (entry as MenuDefinition)?.id ?? '';
			else if (eventType === 'click' && typeof data === 'string') id = data;
			else return false;

			// An entry of a submenu closes the submenu as well as the menu.
			if (chosen?.(id) !== true) JSDialog.CloseAllDropdowns();
			return true;
		};
	}

	/*
		Opens a menu with these entries, the given number of core pixels below the handle. When
		an entry is chosen, chosen gets its id and answers whether the menu stays open, and the
		menu and its submenus close where it does not. A menu that only shows text has no entry to
		choose and needs no chosen.
	*/
	protected openAMenuBelow(
		handle: SelectionHandle,
		below: number,
		menuId: string,
		entries: MenuDefinition[],
		chosen?: (id: string) => boolean | void,
	): void {
		const container = document.getElementById('canvas-container');
		if (!container) return;

		if (!HandleGroup.menuAnchor?.isConnected) {
			HandleGroup.menuAnchor = document.createElement('div');
			HandleGroup.menuAnchor.style.position = 'absolute';
			HandleGroup.menuAnchor.style.zIndex = '1500';
			container.append(HandleGroup.menuAnchor);
		}

		// The canvas fills the container, so a place on the canvas is a place in the container.
		const drawnAt = HandleLook.drawnAt(handle);
		const anchor = HandleGroup.menuAnchor;
		anchor.style.left = drawnAt.vX / app.dpiScale + 'px';
		anchor.style.top = (drawnAt.vY + below) / app.dpiScale + 'px';

		JSDialog.OpenDropdown(
			menuId,
			anchor,
			entries,
			HandleGroup.menuCallback(chosen),
			'',
			false,
			false,
			true,
		);
	}

	/// Shows new entries in a menu that openAMenuBelow opened and that is still open, in their
	/// place. The entries keep their order, and chosen is as for openAMenuBelow.
	protected static updateTheMenu(
		menuId: string,
		entries: MenuDefinition[],
		chosen: (id: string) => boolean | void,
	): void {
		JSDialog.UpdateDropdown(
			menuId,
			entries,
			HandleGroup.menuCallback(chosen),
			false,
			true,
		);
	}

	/// Follows the objects of the selection after they were worked out again, as they arrive from
	/// the engine among other times.
	public followTheObjects(): void {
		// Most groups show nothing beyond their handles, which are worked out anew anyway.
	}

	/// What the group is called, which is what marks a handle as belonging to it.
	public abstract readonly name: string;

	/// The handles it has, worked out from the objects of the selection.
	public handles(): SelectionHandle[] {
		return [];
	}

	/// What it draws thin of the objects while no drag runs, in twips: an object as handles
	/// moved by keys, or let go of, would leave it before the engine has answered. Nothing for
	/// most groups.
	public outline(): cool.ObjectPathPolygon[] | null {
		return null;
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

	/*
		Whether its handles are shown now: drawn, reached by a press and walked by the keyboard.
		A hidden group still has its handles worked out, so the frame of the selection and its
		middle are measured from them as always.
	*/
	public isShown(): boolean {
		return true;
	}

	/// Whether one of its handles blinks while the keyboard is on it. Most of them do.
	public blinksWhileActive(): boolean {
		return true;
	}

	/// How far from its middle a press takes hold of one of its handles, from how far it does for
	/// a handle of the usual size, in core pixels. Most groups take that distance.
	public reachOf(usual: number): number {
		return usual;
	}

	/// What a press on one of its handles does when it is let go where it started, besides
	/// putting the keyboard on that handle. Nothing for most groups.
	public letGoWhereItWasPressed(
		_handle: SelectionHandle,
		_event?: MouseEvent,
	): void {
		// Most groups do nothing more.
	}

	/// Whether that handle of it is dragged, or only clicked, as a button is. Dragged for most
	/// groups.
	public dragsTheHandle(_handle: SelectionHandle): boolean {
		return true;
	}

	/// Whether it answers a click that took no handle itself, at that place and with those
	/// modifiers, before the selection does what such a click does. No for most groups.
	public answersTheClick(_at: cool.SimplePoint, _modifier: number): boolean {
		return false;
	}

	/// Whether its handles belong inside the band of a group the selection stands in, so the band
	/// grows round them. No for most groups, whose handles stand at what is selected.
	public extendsTheBand(): boolean {
		return false;
	}

	/// What Return or Space does on one of its handles while the keyboard is on it. Answers
	/// whether it did something; nothing for most groups.
	public pressByKey(_handle: SelectionHandle, _event: KeyboardEvent): boolean {
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

	/// Whether that handle keeps its place on the page while the objects are moved or scaled,
	/// rather than going along with them. None does for most groups.
	public keepsItsPlace(_handle: SelectionHandle): boolean {
		return false;
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
		was let go. With ortho set, it was moved while Shift was held.
	*/
	public handOver(
		_handle: SelectionHandle,
		_to: cool.Point,
		_ortho: boolean = false,
	): void {
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
	protected sendTheHandle(
		handle: SelectionHandle,
		to: cool.Point,
		ortho: boolean = false,
	): void {
		const objectId = this.selection.selectedObjects()[0];
		if (objectId === undefined) return;

		if (this.sendTheConnectorEnd(objectId, handle, to)) return;

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
				String(Math.round(to.y)) +
				// The engine moves the handle as the office does while Shift is held.
				(ortho ? ' ortho=1' : ''),
		);
	}

	/*
		Hands the engine an end of a connector that was moved, with what it ties to there, which
		the client finds by the rule the engine has: the object a drag aims at, or for a key step
		the object under the end that is no connector. Answers whether the handle was an end of a
		connector.
	*/
	private sendTheConnectorEnd(
		objectId: number,
		handle: SelectionHandle,
		to: cool.Point,
	): boolean {
		const start = handle.name === '9.0.0';
		if (
			!ObjectHandles.isAConnector(objectId) ||
			(!start && handle.name !== '9.0.1')
		)
			return false;

		const aimed =
			this.selection.aimedAtObject() ??
			SelectionSection.tieTargetAt(to, objectId);
		const tie = aimed === null ? null : SelectionSection.tieOn(aimed, to);

		app.socket.sendMessage(
			'setconnectorend id=' +
				String(objectId) +
				' start=' +
				(start ? '1' : '0') +
				' x=' +
				String(Math.round(to.x)) +
				' y=' +
				String(Math.round(to.y)) +
				' tiedto=' +
				String(tie ? tie.objectId : 0) +
				' gluepoint=' +
				String(tie ? SelectionSection.gluePointIndexOf(tie.tie) : -1),
		);
		return true;
	}

	/*
		Moves the handle a key step and hands the engine where it landed, the way the end of a
		drag of it would, held where a drag of it is held. Answers where it landed. A handle goes
		on from where keys have taken it so far, so that a key held down goes on step by step
		before the engine has answered.
	*/
	protected stepAndHandOver(
		handle: SelectionHandle,
		towards: cool.Point,
		event: KeyboardEvent,
	): cool.Point {
		const to = this.holdInside(
			handle,
			HandleGroup.steppedBy(
				HeldSteps.from(this.selection, handle),
				towards,
				event,
			),
		);
		this.handOver(handle, to);

		return to;
	}

	/// How far from a line of a path a press takes hold of it, in twips: half a handle.
	public static lineReach(): number {
		return 0.5 * ShapeHandlesSection.handleSize() * app.pixelsToTwips;
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

	/// The eight handles are hidden while the points to tie a connector to, the points of the
	/// path or the crop marks are shown.
	public isShown(): boolean {
		return FramingHandles.areShownFor(this.selection);
	}

	/// Whether the handles of the object itself are shown in that selection: they are not while
	/// the points to tie a connector to, the points of the path or the crop marks are.
	public static areShownFor(selection: SelectionSection): boolean {
		return (
			!selection.tiePointsAreShown() &&
			!selection.pathPointsAreShown() &&
			!CropHandles.areShownFor(selection)
		);
	}

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
		this.selection.layTheChangeOn(change, scaling.ticketOfTheChange());

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
	public static thePictureOf(
		selection: SelectionSection,
	): cool.SlideObject | undefined {
		const objects = selection.selectedObjects();
		if (objects.length !== 1) return undefined;

		const object = RenderGeometrySection.objectOf(objects[0]);
		return object?.graphicWidth !== undefined ? object : undefined;
	}

	/*
		Takes the crop away from the picture of that selection while its crop marks are shown, so
		the whole graphic shows again in the size it has now; the object grows round it. Answers
		whether there was a crop to take away.
	*/
	public static takeTheCropAway(selection: SelectionSection): boolean {
		const picture = CropHandles.thePictureOf(selection);
		const objectId = selection.selectedObjects()[0];
		const square =
			objectId === undefined
				? null
				: RenderGeometrySection.transformOf(objectId);
		const extent = picture ? CropHandles.graphicExtentOf(picture) : undefined;
		if (!CropHandles.areShownFor(selection) || !square || !extent) return false;
		if (!extent.minX && !extent.minY && extent.maxX === 1 && extent.maxY === 1)
			return false;

		CropHandles.cropTo(objectId, cool.Matrix2D.fromRange(extent).then(square));
		return true;
	}

	/// The 0 key on a crop mark takes the crop away, as it does with no handle in use.
	public pressByKey(_handle: SelectionHandle, event: KeyboardEvent): boolean {
		return event.key === '0' && CropHandles.takeTheCropAway(this.selection);
	}

	/// Whether the marks are shown at all: the crop switch is on and one picture is selected.
	public static areShownFor(selection: SelectionSection): boolean {
		return (
			GraphicSelection.cropping &&
			CropHandles.thePictureOf(selection) !== undefined &&
			ObjectHandles.canBeResized(selection.selectedObjects())
		);
	}

	private shown(): boolean {
		return CropHandles.areShownFor(this.selection);
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

		const picture = CropHandles.thePictureOf(this.selection);
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
			HandleLook.colorOfTheKind.framing,
			-HandleLook.cropMarkShade,
		);
		const place = ObjectHandles.placeOfKind(handle.kind.substring(4));
		const square = this.selection.squareNow();
		if (!square) return color;

		const [across, down] = CropHandles.axesOf(square);

		/*
			The handle in the middle shows the drag marker, the one a border marker of a table
			shows under the mouse, so whatever is dragged to move looks the same. It grows under
			the mouse as the other handles do, is laid in the picture's axes and lays no shape.
		*/
		if (handle.kind === CropHandles.middleKind) {
			HandleLook.drawMarker(
				context,
				'table-row-resize-marker',
				at,
				TableHandles.borderSize(),
				grown,
				{ axes: [across, down], underMouse: 'drag', always: true },
			);
			return 'transparent';
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

	/// The handle is hidden while the points to tie a connector to, the points of the path or the
	/// crop marks are shown, and for a selection that holds a form control.
	public isShown(): boolean {
		return (
			FramingHandles.areShownFor(this.selection) &&
			!ObjectHandles.holdsAFormControl(this.selection.selectedObjects())
		);
	}

	/// How far above the selection it sits, in core pixels.
	public static readonly distance: number = 30;

	/// How far beyond the dimension line of a measurement it sits, in core pixels: further than
	/// above a frame, so that it stays clear of the text standing on that line.
	public static readonly distanceFromMeasureLine: number = 45;

	public handles(): SelectionHandle[] {
		const objectIds = this.selection.selectedObjects();
		const square = this.selection.squareNow();

		// An object shaped by its own handles alone shows no eight, and the handle stands beyond
		// its line instead.
		if (
			objectIds.length === 1 &&
			ObjectHandles.hasHandlesOfItsOwnOnly(objectIds[0])
		) {
			// without a length it stands straight above the spot and does nothing
			const spot = ObjectHandles.spotWithoutLength(objectIds[0]);
			if (spot)
				return this.placedOut(
					spot,
					new cool.Point(0, -1),
					TurningHandle.distance,
					true,
				);

			if (!square) return [];
			return RenderGeometrySection.objectOf(objectIds[0])?.measurement
				? this.placedBeyondTheLine(objectIds[0], square)
				: this.placedBesideTheLine(objectIds[0], square);
		}

		return this.placedAbove(ObjectHandles.framingOf(objectIds, square) ?? []);
	}

	/*
		The point a turn goes about: the middle of the selection as the eight frame it, and where
		no eight are shown, as for an object shaped by its own handles alone, the middle of the
		selection's square.
	*/
	private middle(): cool.SimplePoint | undefined {
		const framed = this.selection.middle();
		if (framed) return framed;

		const middle = this.selection.squareNow()?.apply(0.5, 0.5);
		return middle ? new cool.SimplePoint(middle.x, middle.y) : undefined;
	}

	/*
		The handle of an object that is shaped by its own handles alone, a measurement: out from
		the middle of its dimension line, at a right angle to it and away from the measured line,
		far enough to stay clear of the text on that line. It is worked out from the mapping
		itself, since the square of such an object may be too thin for the eight to tell its sides
		apart.
	*/
	private placedBeyondTheLine(
		objectId: number,
		square: cool.Matrix2D,
	): SelectionHandle[] {
		const measurement = RenderGeometrySection.objectOf(objectId)?.measurement;
		const distance = measurement?.lineDistance ?? 0;
		const across = measurement?.below ? distance : -distance;
		const middle = square.apply(0.5, across);
		const below = square.apply(0.5, across + 1).subtract(middle);
		const length = below.length();
		if (!length) return [];

		// Away from the measured line: up where the dimension line stands above it.
		const outwards = across > 0 ? 1 : -1;
		const out = new cool.Point(
			(outwards * below.x) / length,
			(outwards * below.y) / length,
		);

		return this.placedOut(middle, out, TurningHandle.distanceFromMeasureLine);
	}

	/*
		the handle of a line: out from its middle at a right angle, on the left of the way from
		its first point to its second, so it is above a line drawn from left to right
	*/
	private placedBesideTheLine(
		objectId: number,
		square: cool.Matrix2D,
	): SelectionHandle[] {
		const way = ObjectHandles.wayOfTheLine(objectId, square)?.way;
		if (!way) return [];
		const length = way.length();
		if (!length) return [];

		const out = new cool.Point(way.y / length, -way.x / length);
		return this.placedOut(square.apply(0.5, 0.5), out, TurningHandle.distance);
	}

	/// the handle that far out from the middle, in core pixels, the way out points, turned to
	/// stand along it
	private placedOut(
		middle: cool.Point,
		out: cool.Point,
		distance: number,
		dead: boolean = false,
	): SelectionHandle[] {
		const away = distance * app.dpiScale;
		const step = cool.SimplePoint.fromCorePixels([away, away]);
		const point = new cool.Point(
			middle.x + out.x * step.x,
			middle.y + out.y * step.y,
		);

		return this.handlesAt(point, Math.atan2(out.x, -out.y), dead);
	}

	/*
		It is laid out again from the handles as a drag leaves them, so that it keeps its distance
		from the upper side however the selection is scaled or turned. Where no eight are shown,
		it stays where the drag takes it.
	*/
	public place(shown: SelectionHandle[]): SelectionHandle[] | null {
		if (!shown.some((one: SelectionHandle) => one.kind === '2')) return null;
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

		return this.handlesAt(point, turnedBy);
	}

	/// The handle at that point, turned by that angle, and the handle of the other mode beside it
	/// while that one is shown. Where the objects say they cannot be turned, or it would do
	/// nothing, there is none, as there is nothing to do with it.
	private handlesAt(
		point: cool.Point,
		turnedBy: number,
		deadAnyway: boolean = false,
	): SelectionHandle[] {
		const dead =
			deadAnyway ||
			!ObjectHandles.canBeTurned(this.selection.selectedObjects());
		if (dead) return [];
		const handles: SelectionHandle[] = [
			{
				name: SelectionSection.turningKind,
				kind: SelectionSection.turningKind,
				pointer: '0',
				point: point.round(),
				turnedBy: turnedBy,
			},
		];

		/*
			While the mouse is on it, the handle of the other mode stands beside it along the
			upper side: to the right of the turning handle, to the left of the shearing one. Both
			are drawn as large as a handle under the mouse, with a handle between them, and a bar
			lies behind the two.
		*/
		if (this.selection.showsTheOtherMode())
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
		const middle = this.middle();
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
	private turnByKey(degrees: number): SentChange | null {
		const byKeys = TurningInteraction.keysStillHold(this.selection);
		const middle = byKeys?.middle ?? this.middle();
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
		const middle = this.middle();
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
		this.selection.layTheChangeOn(
			changed?.change ?? null,
			changed?.ticket ?? null,
		);

		return null;
	}

	/*
		Brings the selection back to nought in the mode the handle is in: standing upright in the
		turning mode, the shorter way round, and without a slant in the shearing mode.
	*/
	public backToNought(handle: SelectionHandle): void {
		const middle = this.middle();
		if (!middle) return;

		let changed: SentChange | null;
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

		this.selection.layTheChangeOn(
			changed?.change ?? null,
			changed?.ticket ?? null,
		);
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
	where keys, or a drag that was let go, have taken handles the engine moves itself and has
	not answered for yet, by the object and the name of the handle. nothing is laid on the object
	here for such a handle, so the handle stands where it was taken until the answer for the
	object brings it, and a key held down goes on from there step by step
*/
class HeldSteps {
	private static stepped: Map<string, { at: cool.Point; ortho: boolean }> =
		new Map();

	/// Whether the answers of the engine are listened to, which is once.
	private static followingTheAnswers: boolean = false;

	private static keyOf(objectId: number | undefined, name: string): string {
		return String(objectId) + ':' + name;
	}

	/// where the handle of that name of the one object selected was taken, if it was
	public static of(
		selection: SelectionSection,
		name: string,
	): { at: cool.Point; ortho: boolean } | undefined {
		return HeldSteps.stepped.get(
			HeldSteps.keyOf(selection.selectedObjects()[0], name),
		);
	}

	/// notes where the handle was taken, until the engine answers for its object
	public static hold(
		selection: SelectionSection,
		handle: SelectionHandle,
		to: cool.Point,
		ortho: boolean = false,
	): void {
		if (!HeldSteps.followingTheAnswers) {
			HeldSteps.followingTheAnswers = true;
			RenderManager.onVectorChanged((answered?: number[]) => {
				if (answered === undefined) {
					HeldSteps.stepped.clear();
					return;
				}
				for (const objectId of answered)
					for (const key of Array.from(HeldSteps.stepped.keys()))
						if (key.startsWith(String(objectId) + ':'))
							HeldSteps.stepped.delete(key);
			});
		}

		HeldSteps.stepped.set(
			HeldSteps.keyOf(selection.selectedObjects()[0], handle.name),
			{ at: to, ortho: ortho },
		);

		// The handles are worked out again at once, so the handle stands where it was taken in
		// the same frame as the ghost of the measurement.
		selection.refresh();
		selection.redraw();
	}

	/// the handle standing where it was taken, or where it stands where it was not
	public static from(
		selection: SelectionSection,
		handle: SelectionHandle,
	): SelectionHandle {
		const stepped = HeldSteps.of(selection, handle.name);
		return stepped ? { ...handle, point: stepped.at } : handle;
	}

	/// the handles with each one that was taken standing where it was taken
	public static laidOn(
		selection: SelectionSection,
		handles: SelectionHandle[],
	): SelectionHandle[] {
		return handles.map((one: SelectionHandle) =>
			HeldSteps.from(selection, one),
		);
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

	/// The points the object is shaped by are hidden while the eight are, as the points to tie a
	/// connector to, the points of the path or the crop marks take their place.
	public isShown(): boolean {
		return FramingHandles.areShownFor(this.selection);
	}

	/// The kind the engine gives a point a custom shape is shaped by.
	private static readonly shapingPointKind: string = '22';

	/*
		A point a custom shape is shaped by is moved by naming the shape and the point: the shape
		reads it as the value it shapes itself from, and nothing about a selection or a handle of
		a view is needed for that. The others - the corner radius of a rectangle, the points along
		a connector's way - are still named as handles, since what they mean lives in rules of
		their own that nothing here says yet.
	*/
	public handOver(
		handle: SelectionHandle,
		to: cool.Point,
		ortho: boolean = false,
	): void {
		if (handle.kind !== ShapingHandles.shapingPointKind) {
			this.sendTheHandle(handle, to, ortho);
			HeldSteps.hold(this.selection, handle, to, ortho);
			return;
		}

		const at = Number(handle.name.split('.')[2]);
		if (!(at >= 0)) return;

		HeldSteps.hold(this.selection, handle, to, ortho);

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

	/// the end of the tail of a caption stays where it is, the tail is laid out anew to it
	public keepsItsPlace(handle: SelectionHandle): boolean {
		const objectIds = this.selection.selectedObjects();
		return (
			handle.kind === '9' &&
			objectIds.length === 1 &&
			RenderGeometrySection.objectOf(objectIds[0])?.caption !== undefined
		);
	}

	public handles(): SelectionHandle[] {
		const known = ObjectHandles.shapingOf(this.selection.selectedObjects());

		// Where a handle of a measurement was taken, all its handles go where that leaves them.
		for (const handle of known) {
			const stepped = HeldSteps.of(this.selection, handle.name);
			if (stepped) {
				const measured = MeasurementGhost.handlesDuring(
					this.selection,
					handle,
					stepped.at,
					known,
					stepped.ortho,
				);
				if (measured) return measured;
			}
		}

		// any other handle that was taken goes alone
		return HeldSteps.laidOn(this.selection, known);
	}

	/// The measurement, the line or the connector as the handles keys took, or a drag let go of,
	/// will leave it.
	public outline(): cool.ObjectPathPolygon[] | null {
		for (const handle of ObjectHandles.shapingOf(
			this.selection.selectedObjects(),
		)) {
			const stepped = HeldSteps.of(this.selection, handle.name);
			if (stepped)
				return (
					MeasurementGhost.outline(
						this.selection,
						handle,
						stepped.at,
						stepped.ortho,
					) ??
					LineGhost.outline(this.selection, handle, stepped.at) ??
					CaptionGhost.outline(this.selection, handle, stepped.at) ??
					// a connector as a drag of that handle shows it
					this.selection.runWhileAnEndIsDragged(handle, stepped.at) ??
					this.selection.runWhileALineIsDragged(handle, stepped.at)
				);
		}

		return null;
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
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		return HandleLook.lay(
			context,
			'shaping',
			at,
			grown + (handle.larger ? HandleLook.largerBy('shaping') : 0),
		);
	}
}

/// The way out of a point to tie a connector to and what the point is anchored to, as its menu
/// shows them: the anchor is "none", or one of the nine names of the align attribute of ODF.
interface TiePointMenuState {
	wayOut: cool.ObjectGluePoint['wayOut'];
	anchor: string;
}

/*
	The points someone added to an object for a connector to tie itself to, and, while an end of a
	connector is dragged onto an object, every place that end could land. The points of the
	selected object are shown only while they are asked for, in place of the eight that frame it
	and the handle that turns it.
*/
class TiePointHandles extends HandleGroup {
	public readonly name: string = 'tiePoints';

	public isShown(): boolean {
		return (
			this.selection.tiePointsAreShown() ||
			this.selection.aimedAtObject() !== null
		);
	}

	/// A point is small and sits on the edge of the object, so a press takes it from twice as far.
	public reachOf(usual: number): number {
		return 2 * usual;
	}

	/// A click on a point opens the menu of that point beside it.
	public letGoWhereItWasPressed(handle: SelectionHandle): void {
		if (handle.kind !== 'GluePoint') return;

		this.openTheMenuOf(handle);
	}

	/// The id of the menu a click on a point opens.
	private static readonly menuId: string = 'tie-point-menu';

	/*
		The nine places a point can be anchored to, row by row from the upper left corner, by the
		names of the align attribute of ODF, with the text that names each one in the language of
		the user interface.
	*/
	private static anchors(): { name: string; text: string }[] {
		return [
			{ name: 'top-left', text: _('Top Left') },
			{ name: 'top', text: _('Top') },
			{ name: 'top-right', text: _('Top Right') },
			{ name: 'left', text: _('Left') },
			{ name: 'center', text: _('Center') },
			{ name: 'right', text: _('Right') },
			{ name: 'bottom-left', text: _('Bottom Left') },
			{ name: 'bottom', text: _('Bottom') },
			{ name: 'bottom-right', text: _('Bottom Right') },
		];
	}

	/*
		What a point is anchored to: "none" for a point that keeps its share of the size, else the
		place it keeps its distance from. The render geometry leaves out a side that is the middle,
		so a point with neither side is anchored to the center.
	*/
	private static anchorOf(point: cool.ObjectGluePoint): string {
		if (point.keepsItsDistance !== true) return 'none';

		return (
			[point.fromDown, point.fromAcross].filter(Boolean).join('-') || 'center'
		);
	}

	/// The name of the icon that shows that anchor: the percent sign for none, else a box with a
	/// dot on that place.
	private static iconOfTheAnchor(anchor: string): string {
		return anchor === 'none'
			? 'gluepercent'
			: 'glueanchor' + anchor.replace('-', '');
	}

	/*
		The nine places to anchor a point to, as buttons in a grid of three by three that stand
		where the place is on the box. The one the point is anchored to is selected. A button is
		chosen by its command, which is the id of the entry it stands for.

		A button with a value to compare is one that is on or off, and it tells a screen reader
		whether it is pressed. No command state holds the anchor, so the value never matches and
		selected alone turns the button on.
	*/
	private static anchorGrid(anchor: string): GridWidgetJSON {
		return {
			id: TiePointHandles.menuId + '-anchor-grid',
			type: 'grid',
			cols: 3,
			rows: 3,
			children: TiePointHandles.anchors().map(
				(one: { name: string; text: string }, at: number) =>
					({
						id: TiePointHandles.menuId + '-anchor-' + one.name,
						type: 'toolitem',
						command: 'anchor-' + one.name,
						text: one.text,
						icon: 'lc_' + TiePointHandles.iconOfTheAnchor(one.name) + '.svg',
						noLabel: true,
						selected: one.name === anchor,
						stateValue: one.name,
						left: at % 3,
						top: Math.floor(at / 3),
						tabIndex: 0,
						index: Math.floor(at / 3) + ':' + (at % 3),
					}) as any as WidgetJSON,
			),
		} as any as GridWidgetJSON;
	}

	/// The way out a point names and what it is anchored to, which the menu of the point shows.
	private static stateOf(point: cool.ObjectGluePoint): TiePointMenuState {
		return { wayOut: point.wayOut, anchor: TiePointHandles.anchorOf(point) };
	}

	/// The glue id last sent for a new point, by the id of its object.
	private static lastGlueSent: Map<number, number> = new Map();

	/*
		The menu of a point that is open, or null while none is. It names the point by its object
		and its glue id, and holds the entries the menu was opened with. It holds what
		the marks of the menu show, which is the point as the engine reported it last. And it holds
		what the choices made in the menu lead to: the engine answers a choice after a moment, so a
		choice made before that answer follows on from the one before it. Waiting counts the
		choices sent that no change of the point has answered yet. The groups are made again
		whenever the objects arrive, so this is kept with the class.
	*/
	private static openMenu: {
		objectId: number;
		glue: number;
		entries: MenuDefinition[];
		shown: TiePointMenuState;
		asked: TiePointMenuState;
		waiting: number;
	} | null = null;

	/// The line above the entries of the menu of a point.
	private static header(): MenuDefinition {
		return { id: 'gluepoint', type: 'header', text: _('Gluepoint') };
	}

	/// The Anchor submenu for a point with that anchor: None, then the grid of the nine places.
	private static anchorItems(anchor: string): MenuDefinition[] {
		return [
			{
				id: 'anchor-none',
				type: 'action',
				text: _('None'),
				img: TiePointHandles.iconOfTheAnchor('none'),
				checked: anchor === 'none',
			},
			{
				id: 'anchor-grid',
				type: 'json',
				content: TiePointHandles.anchorGrid(anchor),
			},
		];
	}

	/*
		The entries of the menu of a point that can be edited: the way a connector comes into the
		object there, what the point is anchored to, and taking the point away. The ways the point
		names are checked, both of them for a point that names two sides facing each other, and
		none for a point that names no way.

		The anchor is None for a point that keeps its share of the size of the object. Otherwise
		it is one of nine places on the box of the object, laid out as a grid: a corner, the
		middle of a side, or the middle of the box. The point keeps its distance from that place
		when the object changes size. The Anchor entry itself shows a box, and the submenu shows
		which place it is.
	*/
	private static entriesOf(point: cool.ObjectGluePoint): MenuDefinition[] {
		const ways = TiePointHandles.waysOf(point.wayOut);
		const side = (id: string, text: string, img: string): MenuDefinition => ({
			id,
			type: 'action',
			text,
			img,
			checked: ways.includes(id),
		});

		return [
			TiePointHandles.header(),
			side('left', _('Left'), 'glueescapedirectionleft'),
			side('right', _('Right'), 'glueescapedirectionright'),
			side('top', _('Up'), 'glueescapedirectiontop'),
			side('bottom', _('Down'), 'glueescapedirectionbottom'),
			{ id: 'separator', type: 'separator' },
			{
				id: 'anchor',
				type: 'menu',
				text: _('Anchor'),
				img: TiePointHandles.iconOfTheAnchor('center'),
				items: TiePointHandles.anchorItems(TiePointHandles.anchorOf(point)),
			},
			{ id: 'separator', type: 'separator' },
			{
				id: 'delete',
				type: 'action',
				text: _('Delete'),
				img: 'delete',
			},
		];
	}

	/*
		Opens the menu of a point, below the point. The menu stays open while ways and anchors are
		chosen in it, so several of them can be set one after the other, and its marks follow the
		point as the engine answers. Delete closes it, as do Escape and a press outside it.

		A point the shape makes from its own geometry keeps all of these and stays, so its menu
		holds a line that says it cannot be edited.
	*/
	private openTheMenuOf(handle: SelectionHandle): void {
		const point = this.held(handle);
		const objects = this.selection.selectedObjects();
		if (!point || objects.length !== 1) return;

		if (point.fromTheShape === true) {
			TiePointHandles.openMenu = null;
			this.openAMenuBelow(
				handle,
				0.5 * ShapeHandlesSection.gluePointSize(),
				TiePointHandles.menuId,
				[
					TiePointHandles.header(),
					{
						id: 'fromtheshape',
						type: 'json',
						content: {
							id: TiePointHandles.menuId + '-from-the-shape',
							type: 'fixedtext',
							text: _('Default gluepoints cannot be edited.'),
						},
					},
				],
			);
			return;
		}

		if (point.id === undefined) return;
		const entries = TiePointHandles.entriesOf(point);
		TiePointHandles.openMenu = {
			objectId: objects[0],
			glue: point.id,
			entries,
			shown: TiePointHandles.stateOf(point),
			asked: TiePointHandles.stateOf(point),
			waiting: 0,
		};

		this.openAMenuBelow(
			handle,
			0.5 * ShapeHandlesSection.gluePointSize(),
			TiePointHandles.menuId,
			entries,
			(chosen: string): boolean => this.choose(chosen),
		);
	}

	/*
		Sends what was chosen in the open menu of a point, and answers whether the menu stays
		open. A way switches that side on or off from the way the choices so far
		lead to, and an anchor is sent unless the choices so far lead to it already, since it would
		change nothing and leave a step to undo.
	*/
	private choose(chosen: string): boolean {
		const open = TiePointHandles.openMenu;
		if (!open) return false;
		if (chosen === 'delete') {
			TiePointHandles.openMenu = null;
			this.deleteTheGluePoint(open.glue);
			return false;
		}

		const anchorPrefix = 'anchor-';
		if (chosen.startsWith(anchorPrefix)) {
			const to = chosen.substring(anchorPrefix.length);
			if (to !== open.asked.anchor) {
				this.sendAboutThePoint(
					'setobjectgluepointanchor',
					open.glue,
					'anchor=' + to,
				);
				open.asked.anchor = to;
				open.waiting++;
			}
			return true;
		}

		if (TiePointHandles.sides.some((one) => one.name === chosen)) {
			const way = TiePointHandles.switchedWay(
				TiePointHandles.waysOf(open.asked.wayOut),
				chosen,
			);
			this.sendAboutThePoint(
				'setobjectgluepointwayout',
				open.glue,
				'way=' + way,
			);
			// The render geometry leaves out the way of a point that names none.
			open.asked.wayOut =
				way === 'smart' ? undefined : (way as cool.ObjectGluePoint['wayOut']);
			open.waiting++;
			return true;
		}

		return false;
	}

	/*
		Shows the point in its open menu as the engine reports it now. The menu shows the way out
		in its own entries and the anchor in the submenu, so each of the two is shown again only
		where it changed, and a submenu that is open stays open. The entries the menu was opened
		with take the new ones in their place, so the submenu shows the new anchor when it opens
		later. A point that is gone closes the menu.

		Once no choice waits for its answer, the choices that follow start from the point as the
		engine reports it, which takes in a change that came from elsewhere, an undo or another
		user.
	*/
	public followTheObjects(): void {
		const open = TiePointHandles.openMenu;
		if (!open) return;
		if (!JSDialog.IsDropdownOpen(TiePointHandles.menuId)) {
			TiePointHandles.openMenu = null;
			return;
		}

		// The object may not have arrived yet, and the update that brings it comes back here.
		const object = RenderGeometrySection.objectOf(open.objectId);
		if (!object) return;

		// The point is found by its glue id, so a point another user adds or takes away before it
		// leaves the menu on the same point.
		const at =
			object.gluePoints?.findIndex(
				(one: cool.ObjectGluePoint) => one.id === open.glue,
			) ?? -1;
		const point = object.gluePoints?.[at];
		if (!point || point.fromTheShape === true) {
			TiePointHandles.openMenu = null;
			JSDialog.CloseAllDropdowns();
			return;
		}

		const now = TiePointHandles.stateOf(point);
		const wayChanged = now.wayOut !== open.shown.wayOut;
		const anchorChanged = now.anchor !== open.shown.anchor;
		if (!wayChanged && !anchorChanged) return;

		open.shown = now;
		open.waiting = Math.max(0, open.waiting - 1);
		if (open.waiting === 0) open.asked = { ...now };
		open.entries.splice(
			0,
			open.entries.length,
			...TiePointHandles.entriesOf(point),
		);

		const chosen = (id: string): boolean => this.choose(id);

		if (wayChanged)
			TiePointHandles.updateTheMenu(
				TiePointHandles.menuId,
				open.entries,
				chosen,
			);

		// A submenu is named by its menu and the place of its entry there.
		const anchorAt = open.entries.findIndex(
			(entry: MenuDefinition) => entry.id === 'anchor',
		);
		const submenu = TiePointHandles.menuId + '-' + anchorAt;
		if (anchorChanged && JSDialog.IsDropdownOpen(submenu))
			TiePointHandles.updateTheMenu(
				submenu,
				open.entries[anchorAt].items ?? [],
				chosen,
			);
	}

	/// Sends a change of the point of that glue id of the selected object, with the parameters
	/// that say what the change is, or none for a change that needs none.
	private sendAboutThePoint(
		command: string,
		glue: number,
		change: string,
	): void {
		app.socket.sendMessage(
			command +
				' id=' +
				String(this.selection.selectedObjects()[0]) +
				' glue=' +
				String(glue) +
				(change ? ' ' + change : ''),
		);
	}

	/*
		Takes away the point that handle stands for. Answers whether the handle stands for a
		point. A point the shape makes from its own geometry stays. The points after the one
		taken away move up one place, so the keyboard leaves the points.
	*/
	public deleteThePoint(handle: SelectionHandle): boolean {
		const point = this.held(handle);
		if (!point) return false;
		if (point.fromTheShape !== true && point.id !== undefined)
			this.deleteTheGluePoint(point.id);
		return true;
	}

	/// Takes away the point of that glue id of the selected object. The points after it move up
	/// one place, so the keyboard leaves the points.
	private deleteTheGluePoint(glue: number): void {
		this.selection.travel.leave();
		this.sendAboutThePoint('deleteobjectgluepoint', glue, '');
	}

	/// The sides a way out names: one side, the two that face each other, or none.
	private static waysOf(wayOut: cool.ObjectGluePoint['wayOut']): string[] {
		return wayOut === 'horizontal'
			? ['left', 'right']
			: wayOut === 'vertical'
				? ['top', 'bottom']
				: wayOut
					? [wayOut]
					: [];
	}

	/*
		The way out once that side is switched on or off among those ways, by the name the engine
		takes. A point holds one side, or two that face each other, or none. A side switched on
		joins the side facing it into a pair, and replaces any other side the point held.
	*/
	private static switchedWay(ways: string[], side: string): string {
		if (ways.includes(side)) {
			const left = ways.filter((one: string) => one !== side);
			return left.length ? left[0] : 'smart';
		}

		const across = side === 'left' || side === 'right';
		const facing = across
			? side === 'left'
				? 'right'
				: 'left'
			: side === 'top'
				? 'bottom'
				: 'top';
		if (ways.includes(facing)) return across ? 'horizontal' : 'vertical';
		return side;
	}

	/*
		Gives the selected object a new point where the press landed, when it landed on the
		object. Answers whether it did. The place goes to the engine on the object, as a share of
		its width and of its height.
	*/
	public addAt(at: cool.SimplePoint): boolean {
		const objects = this.selection.selectedObjects();
		if (objects.length !== 1) return false;

		const transform = RenderGeometrySection.transformOf(objects[0]);
		const onTheObject = transform?.invert()?.apply(at.x, at.y);
		if (!onTheObject) return false;

		const inside = (value: number) => value >= 0 && value <= 1;
		if (!inside(onTheObject.x) || !inside(onTheObject.y)) return false;

		// The new point takes the glue id after the highest one the object has or was last sent
		// from here, so two points added before the engine answers get two ids.
		const ids = (
			RenderGeometrySection.objectOf(objects[0])?.gluePoints ?? []
		).map((point: cool.ObjectGluePoint) => point.id ?? 0);
		const glue =
			Math.max(0, TiePointHandles.lastGlueSent.get(objects[0]) ?? 0, ...ids) +
			1;
		TiePointHandles.lastGlueSent.set(objects[0], glue);

		app.socket.sendMessage(
			'insertobjectgluepoint id=' +
				String(objects[0]) +
				' glue=' +
				String(glue) +
				' x=' +
				onTheObject.x.toFixed(4) +
				' y=' +
				onTheObject.y.toFixed(4),
		);
		return true;
	}

	/// the connectors tied to a point keys took, or a drag let go of, as they will run
	public outline(): cool.ObjectPathPolygon[] | null {
		const objects = this.selection.selectedObjects();
		if (objects.length !== 1) return null;

		const runs = GluePointHandles.ownPointsOf(objects[0]).flatMap(
			(handle: SelectionHandle) => {
				const stepped = HeldSteps.of(this.selection, handle.name);
				return stepped
					? (this.selection.runsWhileAGluePointIsDragged(handle, stepped.at) ??
							[])
					: [];
			},
		);
		return runs.length ? runs : null;
	}

	/// The points the selected object was given, which travel with it as it is dragged.
	public handles(): SelectionHandle[] {
		const objects = this.selection.selectedObjects();
		if (objects.length !== 1) return [];

		const own = HeldSteps.laidOn(
			this.selection,
			GluePointHandles.ownPointsOf(objects[0]),
		);
		return this.selection.tiePointsAreShown()
			? own.concat(TiePointHandles.fallbacksOf(objects[0]))
			: own;
	}

	/*
		The four points the object falls back on, in the middle of its sides, shown grey while the
		points are shown: they tell that the points are shown also where the object has none of
		its own. No one can move or delete them, so a press takes them without doing anything and
		the keyboard walks past them.
	*/
	private static fallbacksOf(objectId: number): SelectionHandle[] {
		return GluePointHandles.defaultPointsOf(objectId).map(
			(handle: SelectionHandle, at: number) => ({
				...handle,
				name: 'DefaultGluePoint.' + String(at),
				deactivated: true,
			}),
		);
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
		A point the object was given is moved where it is put, by its glue id. A point the shape
		makes from its own geometry is laid out again with the shape, so it stays where the shape
		puts it. The four an object falls back on are not its own and are shown only, so they have
		no name and nothing takes hold of one.
	*/
	public interactionFor(
		handle: SelectionHandle,
		_at: cool.SimplePoint,
	): SelectionInteraction | null {
		return this.canBeMoved(handle)
			? new ShapingInteraction(this.selection, handle)
			: null;
	}

	/// A point the shape makes is only clicked, for its menu.
	public dragsTheHandle(handle: SelectionHandle): boolean {
		return this.canBeMoved(handle);
	}

	/// Whether that handle stands for a point the object holds and may be moved by a drag or a key.
	private canBeMoved(handle: SelectionHandle): boolean {
		return handle.name !== '' && this.held(handle)?.fromTheShape !== true;
	}

	public handOver(handle: SelectionHandle, to: cool.Point): void {
		const glue = this.held(handle)?.id;
		if (glue === undefined) return;

		const objectId = this.selection.selectedObjects()[0];
		const transform = RenderGeometrySection.transformOf(objectId);
		if (!transform) return;

		// The object is told where the point sits on it, which is how it was given to us.
		const onTheObject = transform.invert()?.apply(to.x, to.y);
		if (!onTheObject) return;

		HeldSteps.hold(this.selection, handle, to);
		app.socket.sendMessage(
			'setobjectgluepoint id=' +
				String(objectId) +
				' glue=' +
				String(glue) +
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
		if (!this.canBeMoved(handle)) return null;

		return this.stepAndHandOver(handle, towards, event);
	}

	/// What the object holds about the point that handle stands for, which the handle names by
	/// its glue id, or nothing for one of the four an object falls back on, which it holds
	/// nothing about.
	private held(handle: SelectionHandle): cool.ObjectGluePoint | undefined {
		if (handle.kind !== 'GluePoint') return undefined;

		const glue = Number(handle.name.split('.')[1]);
		const objects = this.selection.selectedObjects();
		if (!(glue >= 0) || objects.length !== 1) return undefined;

		return RenderGeometrySection.objectOf(objects[0])?.gluePoints?.find(
			(point: cool.ObjectGluePoint) => point.id === glue,
		);
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
		const ways = TiePointHandles.waysOf(held?.wayOut).map((name: string) =>
			this.turnedBy(name),
		);

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
			: HandleLook.colorOfTheKind.tiePoint;
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

	/// The points are shown while they are edited.
	public isShown(): boolean {
		return this.selection.pathPointsAreShown();
	}

	/*
		While a point is being added, a press counts when it is inside the box of the object or on
		its line. The box of a straight path has no height or no width, and a thick line reaches
		outside its box, so the box alone is not enough. Else, while the points are shown, a press
		on the line of a polygon counts, which moves that polygon.
	*/
	public takesAPress(x: number, y: number): boolean {
		const at = this.selection.inDocumentFromCanvas(x, y);
		if (!this.selection.isAddingAPathPoint())
			return this.isShown() && this.polygonAt(at) !== null;

		if (this.selection.within(x, y)) return true;

		return RenderGeometrySection.objectHitAt(
			this.selection.selectedObjects()[0],
			at.x,
			at.y,
		);
	}

	/// The polygon of the path whose line runs at that place in the document, or nothing.
	private polygonAt(at: cool.SimplePoint): number | null {
		return (
			this.selection
				.pathEditor()
				?.polygonAt(new cool.Point(at.x, at.y), HandleGroup.lineReach()) ?? null
		);
	}

	/// A press on the line of a polygon drags that polygon, its points and weights together.
	public interactionForThePlace(
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		const editing = this.selection.pathEditor();
		const polygon = this.polygonAt(at);
		if (!editing || polygon === null || this.selection.isAddingAPathPoint())
			return null;

		return new PolygonDrag(
			this.selection,
			{
				editing: editing,
				polygon: polygon,
				inPath: (handleName: string) => handleName,
				handOver: (by: cool.Point) => editing.movePolygon(polygon, by),
			},
			at,
		);
	}

	/// A closed path can always be opened. An open path can be closed when one of its polygons
	/// has at least three points.
	public canBeClosedOrOpened(): boolean {
		return this.selection.pathEditor()?.canBeClosedOrOpened() ?? false;
	}

	/// Points and weights are drawn smaller than the eight handles, so a press reaches them from
	/// twice the usual distance.
	public reachOf(usual: number): number {
		return 2 * usual;
	}

	/// A click on a point opens the menu of the point below it. A click on a weight does not.
	public letGoWhereItWasPressed(handle: SelectionHandle): void {
		const editing = this.selection.pathEditor();
		const menu =
			handle.kind === '9' && editing
				? PathHandles.pointMenu(editing, handle.name, true)
				: null;
		if (menu)
			this.openAMenuBelow(
				handle,
				0.5 * PathEditing.pointSize(),
				PathHandles.menuId,
				menu.entries,
				menu.chosen,
			);
	}

	/// The id of the menu a click on a point opens.
	private static readonly menuId: string = 'path-point-menu';

	/*
		The menu of a point of the path that editor edits, by the name the editor gives the
		point, and what a choice in it does. The entries delete the point, make the edge after it
		a curve or a line, set how the curve runs through the point, and split the path there.
		An entry is disabled when it does not apply to this point:
		- Delete, where canBeDeleted says no.
		- The last point of an open polygon has no edge after it.
		- The edge is a curve when a weight bends it at either end.
		- Smooth and symmetric need a point with both weights. The current choice is checked and
		  disabled.
		- A path of one polygon can be split at any point of a closed polygon, and at any point
		  of an open polygon except its two ends.
		Nothing where the name names no point of the path.
	*/
	public static pointMenu(
		editing: PathEditing,
		name: string,
		canBeDeleted: boolean,
	): { entries: MenuDefinition[]; chosen: (id: string) => void } | null {
		const facts = editing.factsAt(name);
		if (!facts) return null;

		const way = (
			id: string,
			text: string,
			img: string,
			value: number,
			enabled: boolean,
		): MenuDefinition => ({
			id,
			type: 'action',
			text,
			img,
			checked: facts.continuity === value,
			isEnabled: () => enabled && facts.continuity !== value,
		});

		const entries: MenuDefinition[] = [
			{ id: 'point', type: 'header', text: _('Point') },
			{
				id: 'delete',
				type: 'action',
				text: _('Delete'),
				img: 'bezierdelete',
				isEnabled: () => canBeDeleted,
			},
			{ id: 'separator', type: 'separator' },
			{
				id: facts.curved ? 'line' : 'curve',
				type: 'action',
				text: facts.curved ? _('To Line') : _('To Curve'),
				img: 'bezierconvert',
				isEnabled: () => facts.hasAnEdgeOut,
			},
			{ id: 'separator', type: 'separator' },
			way(
				'corner',
				_('Corner Point'),
				'bezieredge',
				0,
				facts.weightIn || facts.weightOut,
			),
			way(
				'smooth',
				_('Smooth Transition'),
				'beziersmooth',
				1,
				facts.weightIn && facts.weightOut,
			),
			way(
				'symmetric',
				_('Symmetric Transition'),
				'beziersymmetric',
				2,
				facts.weightIn && facts.weightOut,
			),
			{ id: 'separator', type: 'separator' },
			{
				id: 'split',
				type: 'action',
				text: _('Split Path'),
				img: 'beziercutline',
				isEnabled: () => facts.canBeSplit,
			},
		];

		return {
			entries: entries,
			chosen: (chosen: string): void => {
				if (chosen === 'delete') editing.deletePoint(name);
				else if (chosen === 'curve' || chosen === 'line')
					editing.setSegmentCurved(name, chosen === 'curve');
				else if (chosen === 'corner') editing.setContinuity(name, 0);
				else if (chosen === 'smooth') editing.setContinuity(name, 1);
				else if (chosen === 'symmetric') editing.setContinuity(name, 2);
				else if (chosen === 'split') editing.split(name);
			},
		};
	}

	/*
		Deletes the point that the handle stands for. On a weight it does nothing. It returns
		true for every handle of the path, so the key is used up and the object is not deleted.
	*/
	public deleteThePoint(handle: SelectionHandle): boolean {
		this.selection.pathEditor()?.deletePoint(handle.name);
		return true;
	}

	/// Adds a point to the path of the selected object, on the edge nearest to the press. The
	/// press is in twips on the page.
	public addAt(at: cool.SimplePoint): void {
		this.selection.pathEditor()?.insertPointAt(new cool.Point(at.x, at.y));
	}

	/// Closes the path of the selected object, or opens it again where it is closed.
	public switchClosed(): void {
		this.selection.pathEditor()?.switchClosed();
	}

	/// Turns every curved edge of the path that runs straight into a line. With pointsOnALine,
	/// it also deletes the points that lie on a straight line between their neighbours.
	public simplify(pointsOnALine: boolean): void {
		this.selection.pathEditor()?.simplify(pointsOnALine);
	}

	public handOver(handle: SelectionHandle, to: cool.Point): void {
		const editing = this.selection.pathEditor();
		if (!editing) return;

		editing.moveTo(handle.name, to);
		HeldSteps.hold(this.selection, handle, to);
	}

	/// The point that was taken stands where it was taken, with its weights.
	public handles(): SelectionHandle[] {
		const editor = this.selection.pathEditor();
		const known = editor?.handles() ?? [];

		for (const handle of known) {
			const stepped = HeldSteps.of(this.selection, handle.name);
			if (stepped)
				return editor?.handlesDuring(handle.name, stepped.at, known) ?? known;
		}

		return known;
	}

	/// The pieces of the path at the point keys took, or a drag let go of, as they will be.
	public outline(): cool.ObjectPathPolygon[] | null {
		const editor = this.selection.pathEditor();
		for (const handle of editor?.handles() ?? []) {
			const stepped = HeldSteps.of(this.selection, handle.name);
			if (stepped) return editor?.outlineFor(handle.name, stepped.at) ?? null;
		}

		// a change of the path that has not come back shows the whole path as it will be
		return editor?.changedOutline() ?? null;
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
			A point of the path is smaller than the eight. A weight that bends the curve at a
			point is smaller again, since there are two of them for every point of a curve.
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
	The changes of a motion path go out whole: a change made by its editor, as a point added or
	deleted, hands on the whole path as it leaves it, where it lies on the slide, and nothing
	else; a drag of a point or a weight sets the path itself once it is let go.
*/
class MotionPathSink implements PathSink {
	// The path goes out whole, as offsets from the middle of what moves, and that middle can
	// lie between two twips.
	public readonly wholeTwips: boolean = false;
	private onPath: ((path: cool.ObjectPathPolygon[]) => void) | null;

	constructor(
		onPath: ((path: cool.ObjectPathPolygon[]) => void) | null = null,
	) {
		this.onPath = onPath;
	}

	public pathChangedHere(path: cool.ObjectPathPolygon[]): void {
		this.onPath?.(path);
	}
	public movePoints(_moves: PathPointMove[]): void {
		// The whole path goes out when the drag is let go.
	}
	public deletePoints(_places: PathPlace[]): void {
		// The whole path went out already.
	}
	public deleteObject(): void {
		// The motion path keeps its last two points.
	}
	public addPoint(_addition: PathPointAddition): void {
		// The whole path went out already.
	}
	public changeWeights(_change: PathWeightsChange): void {
		// The whole path went out already.
	}
	public split(_place: PathPlace, _after: cool.ObjectPathPolygon[]): void {
		// The whole path went out already.
	}
	public shownPathChanged(): void {
		app.sectionContainer?.requestReDraw();
	}
	public setClosed(_closed: boolean, _at?: PathPlace): void {
		// The whole path went out already.
	}
	public simplify(_simplification: PathSimplification): void {
		// The whole path went out already.
	}
}

/*
	The points and the weights of the motion paths of the one object selected, which a drag
	moves: each path is edited as the path of an object is, the weights of a curve going along
	with a point and the other weight of a smooth point swinging with a dragged one, and once the
	drag is let go the engine is told the new path, by its place among the object's paths and with
	the path it started from. The point a path starts at has a handle as every other point, as a
	drawn path can start away from its object. A press on the line of a path, off its handles,
	takes hold of that polygon of the path, and a drag moves it whole. A click on a point opens
	the menu of that point, as in point editing, and a click on the line the menu of the path.
*/
class MotionPathHandles extends HandlesMovedOneByOne {
	public readonly name: string = 'motionPath';

	/// The kinds of the handles of the group, a point and a weight, which tell them from the
	/// handles of the object's own path.
	public static readonly kind: string = 'motionPoint';
	public static readonly weightKind: string = 'motionWeight';

	/// Whether that handle belongs to the group.
	public static isOne(handle: SelectionHandle | null | undefined): boolean {
		return (
			handle?.kind === MotionPathHandles.kind ||
			handle?.kind === MotionPathHandles.weightKind
		);
	}

	/// The object whose paths are shown, where exactly one object with paths is selected and the
	/// handles of the object itself are shown, not its points, its glue points or crop marks.
	private object(): cool.SlideObject | undefined {
		const selected = this.selection.selectedObjects();
		if (selected.length !== 1 || !FramingHandles.areShownFor(this.selection))
			return undefined;
		const object = RenderGeometrySection.objectOf(selected[0]);
		return object?.motionPaths?.length ? object : undefined;
	}

	/// The editor of the path at that place among the object's paths, which reads the path where
	/// it lies on the slide.
	private static editorOf(
		object: cool.SlideObject,
		path: number,
		onPath: ((placed: cool.ObjectPathPolygon[]) => void) | null = null,
	): PathEditing {
		return new PathEditing(
			'motion ' + String(object.id) + ' ' + String(path),
			() => cool.MotionPaths.placedOf(object)[path]?.polygons,
			new MotionPathSink(onPath),
		);
	}

	/// The editor of the path at that place among the object's paths, whose every change sets
	/// the path.
	private editorThatSets(object: cool.SlideObject, path: number): PathEditing {
		return MotionPathHandles.editorOf(
			object,
			path,
			(placed: cool.ObjectPathPolygon[]) => {
				const moved = MotionPathHandles.withPathPlaced(object, path, placed);
				if (moved) this.setPath(object, path, moved);
			},
		);
	}

	/*
		The object with the path at that place among its paths as given, laid on the slide in
		twips. The path is held as offsets from the middle of what moves, which is taken off
		again. The object is a copy; nothing where it has no such path.
	*/
	public static withPathPlaced(
		object: cool.SlideObject,
		path: number,
		placed: cool.ObjectPathPolygon[],
	): cool.SlideObject | null {
		const paths = object.motionPaths;
		const given = paths?.[path]?.path[0]?.points[0];
		const laid =
			cool.MotionPaths.placedOf(object)[path]?.polygons[0]?.points[0];
		if (!paths || !given || !laid) return null;

		const back = new cool.Point(given.x - laid.x, given.y - laid.y);
		const relative = placed.map((polygon: cool.ObjectPathPolygon) => ({
			...polygon,
			points: polygon.points.map((point: cool.ObjectPathPoint) =>
				cool.MotionPaths.shifted(point, back),
			),
		}));
		return {
			...object,
			motionPaths: paths.map((motionPath: cool.ObjectMotionPath, at: number) =>
				at === path ? { ...motionPath, path: relative } : motionPath,
			),
		};
	}

	/// The place of the path and the name the editor of that path gives the handle, by the name
	/// of a handle of the group.
	public static placeOf(name: string): { path: number; inPath: string } | null {
		const parts = name.split(' ');
		if (parts.length !== 3 || parts[0] !== 'motion' || !(+parts[1] >= 0))
			return null;
		return { path: +parts[1], inPath: parts[2] };
	}

	public handles(): SelectionHandle[] {
		const object = this.object();
		if (!object) return [];

		return (object.motionPaths ?? []).flatMap((_motionPath, path: number) =>
			MotionPathHandles.editorOf(object, path)
				.handles()
				.map((handle: SelectionHandle) => ({
					...handle,
					name: ['motion', path, handle.name].join(' '),
					kind:
						handle.kind === '9'
							? MotionPathHandles.kind
							: MotionPathHandles.weightKind,
				})),
		);
	}

	/// The lines from the points to their weights, as the editor of each path draws them.
	public paint(section: SelectionSection, shown: SelectionHandle[]): void {
		const object = this.object();
		if (!object) return;

		(object.motionPaths ?? []).forEach((_motionPath, path: number) => {
			const own = shown.flatMap((handle: SelectionHandle) => {
				const place = MotionPathHandles.placeOf(handle.name);
				return place?.path === path
					? [
							{
								...handle,
								name: place.inPath,
								kind: handle.kind === MotionPathHandles.kind ? '9' : '10',
							},
						]
					: [];
			});
			MotionPathHandles.editorOf(object, path).paint(
				section,
				own,
				cool.MotionPaths.colourOf(path),
			);
		});
	}

	/// The points and weights are drawn as those of a path, so a press reaches them from twice the
	/// usual distance as well.
	public reachOf(usual: number): number {
		return 2 * usual;
	}

	/// A point or a weight is filled in the colour its path is shown in.
	public shape(
		context: CanvasRenderingContext2D,
		handle: SelectionHandle,
		at: cool.SimplePoint,
		grown: number,
	): string {
		HandleLook.lay(
			context,
			handle.kind === MotionPathHandles.kind ? 'pathPoint' : 'weight',
			at,
			grown,
		);
		const path = MotionPathHandles.placeOf(handle.name)?.path ?? 0;
		return cool.MotionPaths.colourOf(path);
	}

	/*
		The object with the point or the weight of that handle moved to that place on the page, in
		twips, as a drag leaves it. The object is a copy; nothing where the handle names no point of
		its paths.
	*/
	public static withPointAt(
		object: cool.SlideObject,
		name: string,
		to: cool.Point,
	): cool.SlideObject | null {
		const place = MotionPathHandles.placeOf(name);
		if (!place) return null;

		const moved = MotionPathHandles.editorOf(object, place.path).pathFor(
			place.inPath,
			to,
		);
		return moved
			? MotionPathHandles.withPathPlaced(object, place.path, moved)
			: null;
	}

	/// A path as the engine takes it, point by point in whole twips, with how the curve runs
	/// through each point, which the engine then makes the rounded numbers hold exactly.
	private static written(path: cool.ObjectPathPolygon[]): string {
		const round = (value: number | undefined) =>
			value === undefined ? undefined : Math.round(value);
		return JSON.stringify({
			path: path.map((polygon: cool.ObjectPathPolygon) => ({
				closed: !!polygon.closed,
				points: polygon.points.map((point: cool.ObjectPathPoint) => ({
					x: Math.round(point.x),
					y: Math.round(point.y),
					behindX: round(point.behindX),
					behindY: round(point.behindY),
					aheadX: round(point.aheadX),
					aheadY: round(point.aheadY),
					continuity: point.continuity,
				})),
			})),
		});
	}

	/// Tells the engine the path with the point or weight where it was let go.
	public handOver(handle: SelectionHandle, to: cool.Point): void {
		const object = this.object();
		const place = MotionPathHandles.placeOf(handle.name);
		const moved = object
			? MotionPathHandles.withPointAt(object, handle.name, to)
			: null;
		if (object && place && moved) this.setPath(object, place.path, moved);
	}

	/*
		Tells the engine the path at that place among the object's paths as the object moved
		has it, together with the path it started from, and shows the new path at once; the
		engine's report of the object replaces it, or brings the paths as they are where they
		changed meanwhile.
	*/
	private setPath(
		object: cool.SlideObject,
		path: number,
		moved: cool.SlideObject,
	): void {
		const was = object.motionPaths?.[path];
		const now = moved.motionPaths?.[path];
		if (!was || !now || object.id === undefined) return;

		// a change that leaves the path as it was, as Simplify on curves that run curved, sets
		// nothing, so it leaves no step to undo
		const written = MotionPathHandles.written(now.path);
		const wasWritten = MotionPathHandles.written(was.path);
		if (written === wasWritten) return;

		app.map.sendUnoCommand('.uno:SetMotionPath', {
			ObjectId: { type: 'string', value: String(object.id) },
			Index: { type: 'long', value: String(path) },
			Path: { type: 'string', value: written },
			Was: { type: 'string', value: wasWritten },
		});
		object.motionPaths = moved.motionPaths;
		this.selection.redraw();
	}

	/*
		The object with that polygon of the path at that place among its paths moved by that
		offset, in twips, its points and their weights together. The object is a copy; nothing
		where it has no such polygon.
	*/
	public static withPolygonMovedBy(
		object: cool.SlideObject,
		path: number,
		polygon: number,
		by: cool.Point,
	): cool.SlideObject | null {
		const paths = object.motionPaths;
		if (!paths?.[path]?.path[polygon]) return null;

		return {
			...object,
			motionPaths: paths.map((motionPath: cool.ObjectMotionPath, at: number) =>
				at !== path
					? motionPath
					: {
							...motionPath,
							path: motionPath.path.map(
								(one: cool.ObjectPathPolygon, which: number) =>
									which !== polygon
										? one
										: {
												...one,
												points: one.points.map((point: cool.ObjectPathPoint) =>
													cool.MotionPaths.shifted(point, by),
												),
											},
							),
						},
			),
		};
	}

	/*
		The path among the object's paths and the polygon of it whose line runs at that place in
		the document; nothing where none does, while text is edited, and on text: a path that runs
		over the text of its object leaves a press there to the text, as the pointer says.
	*/
	private lineAt(
		at: cool.SimplePoint,
	): { path: number; polygon: number } | null {
		const object = this.object();
		if (!object || app.file.textCursor.visible) return null;

		const pointer = RenderGeometrySection.pointerAt(at.x, at.y);
		if (pointer === 'text' || pointer === 'pointer') return null;

		const place = new cool.Point(at.x, at.y);
		const paths = object.motionPaths ?? [];
		for (let path = 0; path < paths.length; ++path) {
			const polygon = MotionPathHandles.editorOf(object, path).polygonAt(
				place,
				HandleGroup.lineReach(),
			);
			if (polygon !== null) return { path: path, polygon: polygon };
		}
		return null;
	}

	/// The id of the menu a click on a point of a motion path opens.
	private static readonly pointMenuId: string = 'motion-path-point-menu';

	/// The id of the menu a click on the line of a motion path opens.
	private static readonly pathMenuId: string = 'motion-path-menu';

	/*
		A click on a point opens the menu of the point below it, as in point editing; a click on
		a weight does not. A point is not deleted where its polygon would be left with fewer than
		two points, as a motion path keeps every part it has.
	*/
	public letGoWhereItWasPressed(handle: SelectionHandle): void {
		const object = this.object();
		const place = MotionPathHandles.placeOf(handle.name);
		if (!object || !place || handle.kind !== MotionPathHandles.kind) return;

		const editing = this.editorThatSets(object, place.path);
		const polygon = editing.pointOf(place.inPath)?.of;
		const menu = PathHandles.pointMenu(
			editing,
			place.inPath,
			(polygon?.points.length ?? 0) > 2,
		);
		if (menu)
			this.openAMenuBelow(
				handle,
				0.5 * PathEditing.pointSize(),
				MotionPathHandles.pointMenuId,
				menu.entries,
				menu.chosen,
			);
	}

	/*
		A click on the line of a path opens the menu of that path below the place clicked: a
		point added there, the path closed or opened, and its curves simplified.
	*/
	public answersTheClick(at: cool.SimplePoint, _modifier: number): boolean {
		const object = this.object();
		const line = this.lineAt(at);
		if (!object || !line) return false;

		const editing = this.editorThatSets(object, line.path);
		const place = new cool.Point(at.x, at.y);
		const closed = PathEditing.isClosed(editing.path());
		const entries: MenuDefinition[] = [
			{ id: 'path', type: 'header', text: _('Motion Path') },
			{ id: 'add', type: 'action', text: _('Add Point'), img: 'bezierinsert' },
			{ id: 'separator', type: 'separator' },
			{
				id: 'close',
				type: 'action',
				text: closed ? _('Open Path') : _('Close Path'),
				img: 'bezierclose',
				isEnabled: () => editing.canBeClosedOrOpened(),
			},
			{
				id: 'curves',
				type: 'action',
				text: _('Simplify Curves'),
				img: 'beziereliminatepoints',
			},
			{
				id: 'all',
				type: 'action',
				text: _('Simplify All'),
				img: 'beziersimplifyall',
			},
		];

		this.openAMenuBelow(
			{ name: '', kind: '', point: place } as SelectionHandle,
			0,
			MotionPathHandles.pathMenuId,
			entries,
			(chosen: string): void => {
				if (chosen === 'add') editing.insertPointAt(place);
				else if (chosen === 'close') editing.switchClosed();
				else if (chosen === 'curves') editing.simplify(false);
				else if (chosen === 'all') editing.simplify(true);
			},
		);
		return true;
	}

	/// A press on the line of a path, off its handles, takes hold of that polygon of the path.
	public takesAPress(x: number, y: number): boolean {
		return this.lineAt(this.selection.inDocumentFromCanvas(x, y)) !== null;
	}

	/// A drag of the line of a path moves that polygon of it, and the path is set once let go.
	public interactionForThePlace(
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		const object = this.object();
		const line = this.lineAt(at);
		if (!object || !line) return null;

		return new PolygonDrag(
			this.selection,
			{
				editing: MotionPathHandles.editorOf(object, line.path),
				polygon: line.polygon,
				inPath: (handleName: string) => {
					const place = MotionPathHandles.placeOf(handleName);
					return place?.path === line.path ? place.inPath : null;
				},
				handOver: (by: cool.Point) => {
					const moved = MotionPathHandles.withPolygonMovedBy(
						object,
						line.path,
						line.polygon,
						by,
					);
					if (moved) this.setPath(object, line.path, moved);
				},
			},
			at,
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

	/// A press inside the box of the selection, or on the line of a selected path, which reaches
	/// a little outside its box.
	public takesAPress(x: number, y: number): boolean {
		if (this.selection.within(x, y)) return true;

		const at = this.selection.inDocumentFromCanvas(x, y);
		return (
			(this.selection
				.pathEditor()
				?.polygonAt(new cool.Point(at.x, at.y), HandleGroup.lineReach()) ??
				null) !== null
		);
	}

	public shape(): string {
		// It has no handle to draw.
		return '';
	}

	/// The object does not move while the points of its path are shown, so a press that misses a
	/// point does not change the shape.
	public interactionForThePlace(
		at: cool.SimplePoint,
	): SelectionInteraction | null {
		if (this.selection.pathPointsAreShown()) return null;

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
