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
	Creating an object by dragging out its box, as the office does: an object chosen to be added
	is not put in the middle of the slide, it puts one into this mode. A press sets one corner of
	the box and the drag sets its size; letting go asks the engine for the object in that
	rectangle, and the mode is over. Escape, a command or a press anywhere else end the mode
	without an object; what a click without a drag does is the kind's own.

	Before the press lines across the view mark where the drag would begin, snapped as the drag
	snaps, with its place beside them; during the drag the frame of the object is drawn with its
	size beside it. As in the office, Shift makes the box as wide as high, Alt drags it from its
	middle, and Ctrl turns the snap over.

	This is what every kind made by a box shares; a kind of object says the rest in a class of its
	own.
*/
abstract class ClassicCreateContext implements InteractionMode {
	/// The command that makes the object, without its .uno: prefix, as BasicShapes.rectangle.
	protected shape: string;
	/// The part the mode was entered on; it ends when another one is shown.
	private part: number;
	/// Where the mouse stands snapped, in twips, while no drag runs; null off the page.
	private hover: cool.Point | null = null;
	/// The corner the drag began at, snapped, in twips; null while no drag runs.
	private from: cool.Point | null = null;
	/// Where the press was, in twips, as it came, not snapped.
	private pressedAt: cool.Point | null = null;
	/// The rectangle of the drag, in twips, as the shape would get it.
	private box: cool.Range2D | null = null;
	/// The rectangle sent to the engine, drawn until the engine's answer brings the object.
	protected sent: cool.Range2D | null = null;
	private sentWait: ReturnType<typeof setTimeout> | null = null;
	/// What is found to snap to, drawn as lines.
	private snap: SelectionSnap = new SelectionSnap();
	/// A press anywhere outside the page ends the mode.
	private pressedOutside: (event: PointerEvent) => void;
	/// Where the mouse was last, in twips, as it came, and whether the button was down.
	private lastAt: cool.Point | null = null;
	private lastPressed: boolean = false;

	/// The least size a drag gives, in CSS pixels; a smaller one is a click.
	private static readonly leastSize: number = 3;

	/// How long the frame sent stays drawn when no answer brings the object, in milliseconds.
	protected static readonly longestWait: number = 5000;

	/// Whether the answers of the engine are followed, which they are once.
	private static listening: boolean = false;

	constructor(shape: string) {
		this.shape = shape;
		this.part = app.map._docLayer._selectedPart;
		this.pressedOutside = (event: PointerEvent) => {
			if (event.target !== document.getElementById('document-canvas'))
				Interaction.leaveMode();
		};
		document.addEventListener('pointerdown', this.pressedOutside, true);
	}

	/// Whether that command can be created by a drag here, as one of the kinds there is a class
	/// for.
	public static canCreate(shape: string): boolean {
		return (
			(ClassicShapeCreateContext.makes(shape) ||
				ClassicTextCreateContext.makes(shape) ||
				ClassicLineCreateContext.makes(shape) ||
				ClassicConnectorCreateContext.makes(shape)) &&
			RenderGeometrySection.answersPointer() &&
			app.map.isEditMode()
		);
	}

	/// The command of a text box, which is made by a drag too, or by a click.
	public static readonly text: string = 'Text';

	/// Enters the mode for that command, with the class of its kind.
	public static start(shape: string): void {
		ClassicCreateContext.listen();
		const mode = ClassicTextCreateContext.makes(shape)
			? new ClassicTextCreateContext(shape)
			: ClassicLineCreateContext.makes(shape)
				? new ClassicLineCreateContext(shape)
				: ClassicConnectorCreateContext.makes(shape)
					? new ClassicConnectorCreateContext(shape)
					: new ClassicShapeCreateContext(shape);
		Interaction.enterMode(mode);
		mode.started();
	}

	/// Follows the answers of the engine, so the frame sent goes once the object is drawn.
	private static listen(): void {
		if (ClassicCreateContext.listening) return;
		ClassicCreateContext.listening = true;
		RenderManager.onVectorChanged(() => ClassicCreateContext.answered());
	}

	// What a kind of object says of itself.

	/// The mode was entered; nothing more for most kinds.
	public started(): void {
		// Most kinds need nothing from the engine before the drag.
	}

	/// The pointer over the page while the mode is on.
	public abstract pointer(): string;

	/// A click without a drag at that point, snapped, and where it was as it came. Answers
	/// whether it was taken; one that was not ends the mode, as for most kinds.
	protected clicked(_at: cool.Point, _pressedAt: cool.Point): boolean {
		return false;
	}

	/// Whether the box is as wide as high without Shift.
	protected asWideAsHighByItself(): boolean {
		return false;
	}

	/// The arguments the command takes besides the rectangle.
	protected moreArguments(): { [name: string]: { type: string; value: any } } {
		return {};
	}

	/// The rectangle a drag from that corner to that point gives, with Shift and Alt.
	protected dragTo(
		from: cool.Point,
		to: cool.Point,
		modifier: number,
	): cool.Range2D | null {
		return this.boxTo(from, to, modifier);
	}

	/// Whether that rectangle is large enough for an object; a smaller one is a click.
	protected largeEnough(box: cool.Range2D, least: number): boolean {
		return box.width >= least && box.height >= least;
	}

	/// The arguments that place the object, the rectangle in twips.
	protected placeArguments(box: cool.Range2D): {
		[name: string]: { type: string; value: any };
	} {
		return {
			X: ClassicCreateContext.twipsArgument(box.minX),
			Y: ClassicCreateContext.twipsArgument(box.minY),
			Width: ClassicCreateContext.twipsArgument(box.width),
			Height: ClassicCreateContext.twipsArgument(box.height),
		};
	}

	/// What is written beside the mouse while it drags, of that size of the drag.
	protected readingOf(size: cool.Point): string {
		return (
			'(' +
			ClassicCreateContext.inTheUnit(size.x) +
			', ' +
			ClassicCreateContext.inTheUnit(size.y) +
			')'
		);
	}

	/// A length in twips as an argument of a command.
	protected static twipsArgument(twips: number): {
		type: string;
		value: string;
	} {
		return { type: 'long', value: String(Math.round(twips)) };
	}

	/// Draws the object as it looks in that rectangle, half see-through while dragged and whole
	/// once sent. Answers whether it could; where not the frame stands in for it.
	protected drawTheObject(
		_context: CanvasRenderingContext2D,
		_box: cool.Range2D,
		_whole: boolean,
	): boolean {
		return false;
	}

	/// The command for the object in that rectangle went out; nothing more for most kinds.
	protected wasSent(_box: cool.Range2D): void {
		// Most kinds wait for the engine's report of the object.
	}

	/// The frame waiting for the engine, of the mode last on.
	private static waiting: ClassicCreateContext | null = null;

	private static answered(): void {
		const waiting = ClassicCreateContext.waiting;
		if (!waiting) return;
		waiting.forgetTheSent();
	}

	protected forgetTheSent(): void {
		if (this.sentWait !== null) app.timerRegistry.clearTimeout(this.sentWait);
		this.sentWait = null;
		this.sent = null;
		if (ClassicCreateContext.waiting === this)
			ClassicCreateContext.waiting = null;
		InteractionModeSection.follow();
	}

	// the context under the mode, where the selection stands
	private under(): InteractionContext {
		return Interaction.contextAt(
			Interaction.placeOf(GraphicSelection.selectedObjectIDs),
		);
	}

	// a length in CSS pixels, in twips
	protected static twips(cssPixels: number): number {
		return cssPixels * app.dpiScale * app.pixelsToTwips;
	}

	/// Whether the last snap was to the grid, where every point snaps.
	private snappedToTheGrid: boolean = false;

	/// Where the mouse at that point places the drag; for most kinds the point snapped.
	protected placeOf(at: cool.Point, modifier: number): cool.Point {
		return this.snapped(at, modifier);
	}

	/// What a kind draws besides the object and the readings; nothing for most kinds.
	protected drawMore(_context: CanvasRenderingContext2D): void {
		// Most kinds draw nothing more.
	}

	/// That point of the page snapped as the drag snaps, to the grid or to the other objects.
	protected snapped(at: cool.Point, modifier: number): cool.Point {
		const point = new cool.SimplePoint(at.x, at.y);
		const turnedOver =
			(modifier & (app.UNOModifier.CTRL | app.UNOModifier.CTRLMAC)) !== 0;
		const gridOn =
			app.map.stateChangeHandler.getItemValue('.uno:GridUse') === 'true';
		this.snappedToTheGrid = gridOn !== turnedOver;
		this.snap.look([0, 0], [point.pX, point.pY], [0, 0], turnedOver);
		if (!this.snap.found()) return at;
		const corner = this.snap.corner(point.pX, point.pY);
		const snapped = cool.SimplePoint.fromCorePixels(corner);
		return new cool.Point(snapped.x, snapped.y);
	}

	/// The rectangle from the corner the drag began at to that point, with Shift and Alt.
	private boxTo(
		from: cool.Point,
		to: cool.Point,
		modifier: number,
	): cool.Range2D | null {
		let width = to.x - from.x;
		let height = to.y - from.y;
		// as in the office, Shift makes a shape as wide as high, and lets one that is made so
		// be dragged out freely
		const shift = (modifier & app.UNOModifier.SHIFT) !== 0;
		if (this.asWideAsHighByItself() !== shift) {
			const side = Math.max(Math.abs(width), Math.abs(height));
			width = width < 0 ? -side : side;
			height = height < 0 ? -side : side;
		}
		// with Alt the drag began at the middle of the shape
		const fromMiddle = (modifier & app.UNOModifier.ALT) !== 0;
		const x1 = fromMiddle ? from.x - width : from.x;
		const y1 = fromMiddle ? from.y - height : from.y;
		return cool.Range2D.fromPoints([
			new cool.Point(x1, y1),
			new cool.Point(from.x + width, from.y + height),
		]);
	}

	// the size of the drag as it goes, which is negative towards the upper left
	private signedSize: cool.Point | null = null;

	mouseAt(at: cool.Point, modifier: number, pressed: boolean): void {
		if (this.leftThePart()) return;
		this.lastAt = at;
		this.lastPressed = pressed;
		const point = this.placeOf(at, modifier);
		if (!pressed || !this.from) {
			this.hover = point;
			return;
		}
		this.box = this.dragTo(this.from, point, modifier);
		this.signedSize = new cool.Point(
			point.x - this.from.x,
			point.y - this.from.y,
		);
	}

	mouseDown(at: cool.Point, modifier: number): void {
		if (this.leftThePart()) return;
		this.from = this.placeOf(at, modifier);
		this.pressedAt = at;
		this.hover = null;
		this.box = null;
		this.signedSize = null;
	}

	mouseUp(at: cool.Point, modifier: number, dragged: boolean): void {
		if (!this.from) return;
		this.mouseAt(at, modifier, true);
		const box = this.box;
		const least = ClassicCreateContext.twips(ClassicCreateContext.leastSize);
		// a click, or a drag too small for an object, is the kind's own, or ends the mode
		if (!dragged || !box || !this.largeEnough(box, least)) {
			if (!this.clicked(this.from, this.pressedAt ?? this.from))
				Interaction.leaveMode();
			return;
		}
		this.send(box);
	}

	dragCancelled(): void {
		Interaction.leaveMode();
	}

	mouseLeft(): void {
		this.hover = null;
		this.snap.forget();
	}

	/*
		Asks the engine for the object in that rectangle and ends the mode. The mode is left first,
		so the command that goes out does not end it again. The frame stays drawn until the
		engine's answer brings the object.
	*/
	protected send(box: cool.Range2D): void {
		this.sent = box;
		ClassicCreateContext.waiting = this;
		this.sentWait = app.timerRegistry.setTimeout(
			'createdshapeanswer',
			() => this.forgetTheSent(),
			ClassicCreateContext.longestWait,
		);
		Interaction.leaveMode();
		app.map.sendUnoCommand('.uno:' + this.shape, {
			...this.placeArguments(box),
			...this.moreArguments(),
		});
		this.wasSent(box);
	}

	/// Ends the mode where another part is shown than the one it was entered on.
	private leftThePart(): boolean {
		if (app.map._docLayer._selectedPart === this.part) return false;
		Interaction.leaveMode();
		return true;
	}

	stillDraws(): boolean {
		return this.sent !== null;
	}

	ended(): void {
		document.removeEventListener('pointerdown', this.pressedOutside, true);
		this.lastAt = null;
		this.from = null;
		this.box = null;
		this.hover = null;
		this.snap.forget();
	}

	/*
		The length in the unit the sidebar measures positions and sizes in: inches where the
		ruler says so or the language is American English, centimetres otherwise.
	*/
	protected static inTheUnit(twips: number): string {
		const ruler = app.map.stateChangeHandler.getItemValue(
			'.uno:LeftRightParaMargin',
		);
		const unit =
			ruler && typeof ruler === 'object' && ruler.unit
				? ruler.unit
				: (window.coolParams?.get('lang') || navigator.language) === 'en-US'
					? 'inch'
					: 'cm';
		if (unit === 'inch') return (twips / 1440).toFixed(2) + '"';
		if (unit === 'mm') return (twips / 56.7).toFixed(1) + ' mm';
		return (twips / 567).toFixed(2) + ' cm';
	}

	/*
		Writes a reading beside that point of the canvas, as the turn angle is: below and to the
		right of it, or to the left and above where the drag goes that way, so it stays outside
		the frame.
	*/
	private static writeAt(
		context: CanvasRenderingContext2D,
		text: string,
		at: cool.SimplePoint,
		toTheLeft: boolean,
		above: boolean,
	): void {
		const scale = app.dpiScale;
		const x = at.vX + (toTheLeft ? -8 : 16) * scale;
		const y = at.vY + (above ? -8 : 24) * scale;
		context.font = Math.round(11 * scale) + 'px sans-serif';
		context.textAlign = toTheLeft ? 'right' : 'left';
		context.textBaseline = above ? 'bottom' : 'middle';
		context.strokeStyle = '#FFFFFF';
		context.lineWidth = 3;
		context.strokeText(text, x, y);
		context.fillStyle = HandleLook.outlineColour;
		context.fillText(text, x, y);
	}

	draw(section: CanvasSectionObject): void {
		const context = section.context;
		context.save();
		context.setTransform(1, 0, 0, 1, 0, 0);

		// the object as it looks where its kind can tell, the frame of it otherwise
		const frame = this.box ?? this.sent;
		if (frame && !this.drawTheObject(context, frame, !this.box)) {
			const from = new cool.SimplePoint(frame.minX, frame.minY);
			const to = new cool.SimplePoint(frame.maxX, frame.maxY);
			context.strokeStyle = HandleLook.outlineOfADrag;
			context.lineWidth = 1;
			context.strokeRect(
				Math.round(from.vX) + 0.5,
				Math.round(from.vY) + 0.5,
				Math.round(to.vX - from.vX),
				Math.round(to.vY - from.vY),
			);
		}

		if (this.box && this.from && this.signedSize) {
			// the size beside the corner the mouse holds
			const corner = new cool.SimplePoint(
				this.from.x + this.signedSize.x,
				this.from.y + this.signedSize.y,
			);
			ClassicCreateContext.writeAt(
				context,
				this.readingOf(this.signedSize),
				corner,
				this.signedSize.x < 0,
				this.signedSize.y < 0,
			);
		} else if (this.hover && !this.sent) {
			// lines across the whole view through where the drag would begin, with its place above
			// and to the left; they have a light edge, so they show on any page and object
			const at = new cool.SimplePoint(this.hover.x, this.hover.y);
			const x = Math.round(at.vX) + 0.5;
			const y = Math.round(at.vY) + 0.5;
			const canvas = context.canvas;
			// the line of an axis that met an edge of another object takes the colour of the lines
			// a snap shows; on the grid every point snaps, so that says nothing there
			if (!HelperLineStyles.smartGuidesStyle) HelperLineStyles.initiate();
			const snappedAt = this.snappedToTheGrid ? [null, null] : this.snap.at();
			const across = (
				fromX: number,
				fromY: number,
				toX: number,
				toY: number,
			) => {
				context.beginPath();
				context.moveTo(fromX, fromY);
				context.lineTo(toX, toY);
			};
			const line = (snapped: boolean, draw: () => void) => {
				draw();
				context.strokeStyle = '#FFFFFF';
				context.lineWidth = 3;
				context.stroke();
				draw();
				context.strokeStyle = snapped
					? HelperLineStyles.smartGuidesStyle
					: HandleLook.outlineOfADrag;
				context.lineWidth = 1;
				context.stroke();
			};
			line(snappedAt[1] !== null, () => across(0, y, canvas.width, y));
			line(snappedAt[0] !== null, () => across(x, 0, x, canvas.height));
			ClassicCreateContext.writeAt(
				context,
				'(' +
					ClassicCreateContext.inTheUnit(this.hover.x) +
					', ' +
					ClassicCreateContext.inTheUnit(this.hover.y) +
					')',
				at,
				true,
				true,
			);
		}
		this.drawMore(context);
		context.restore();
	}

	// What the mode answers of the actions of a context: it takes every press and key itself,
	// and the rest is answered as where the selection stands.

	press(_hit: number | undefined, _modifier: number): boolean {
		return true;
	}
	takesHoldFor(_hit: number, _modifier: number): number | undefined {
		return undefined;
	}
	doublePress(_hit: number | undefined): boolean {
		return true;
	}
	altPress(_hits: number[], _backwards: boolean): boolean {
		return true;
	}
	arrowKey(_towards: cool.Point, _event: KeyboardEvent): boolean {
		return true;
	}
	/*
		Shift, Ctrl and Alt change the drag and the snap at once, also while the mouse stands
		still. Escape ends the mode; any other key ends it too and goes on as it would.
	*/
	keyEvent(event: KeyboardEvent, modifier: number): boolean {
		if (['Shift', 'Control', 'Alt', 'Meta'].indexOf(event.key) >= 0) {
			if (this.lastAt) {
				this.mouseAt(this.lastAt, modifier, this.lastPressed);
				app.sectionContainer?.requestReDraw();
			}
			return true;
		}
		if (event.type !== 'keydown') return true;
		if (event.key === 'Escape') return this.escape();
		Interaction.leaveMode();
		return false;
	}

	escape(): boolean {
		Interaction.leaveMode();
		return true;
	}
	enterKey(): boolean {
		return true;
	}
	leaveKey(): boolean {
		return true;
	}
	commandStates(): { [command: string]: boolean } {
		return this.under().commandStates();
	}
	showsGroupBands(): boolean {
		return this.under().showsGroupBands();
	}
	theOnesThatHaveASay(objectIds: number[]): number[] {
		return this.under().theOnesThatHaveASay(objectIds);
	}
	theOnesAChangeNames(objectIds: number[]): number[] {
		return this.under().theOnesAChangeNames(objectIds);
	}
	heldToItsWay(objectIds: number[], by: cool.Point): cool.Point {
		return this.under().heldToItsWay(objectIds, by);
	}
	ungroup(): boolean {
		return true;
	}
	tabKey(_forwards: boolean): boolean {
		return true;
	}
	frameSelects(
		_frame: cool.Range2D,
		_extending: boolean,
	): number[] | undefined {
		return undefined;
	}
	toAnEnd(_first: boolean): boolean {
		return true;
	}
	groupEntered(): void {
		this.under().groupEntered();
	}
	enteredBy(
		selection: number[],
		hits?: number[],
		backwards?: boolean,
	): number[] {
		return this.under().enteredBy(selection, hits, backwards);
	}
	whatIsLeft(selected: number[], groupsAbove: number[]): number[] {
		return this.under().whatIsLeft(selected, groupsAbove);
	}
}

/// How a shape looks in its default size, as the engine made it: the box it was made in, in
/// twips, what it draws, and whether it is as wide as high by itself.
interface ShapeLook {
	box: cool.Range2D;
	primitives: cool.Primitive[];
	asWideAsHigh: boolean;
	// the entry a new shape of it has in the part, but its id
	entry: cool.SlideObject;
}

/// A run of object ids the engine reserved for this view: the first one and how many of them are
/// still free. A count of 0 is an empty set.
interface IdSet {
	first: number;
	count: number;
}

/*
	Object ids the engine reserved for this view, which no other object takes. The client gives
	each shape it creates one of them, so the shape has its real id from the start and nothing has
	to be told again once the engine reports it. We hold two sets, the active one the ids are taken
	from and a reserve. Both are asked for when the document goes into edit mode; when the active
	one is used up, the reserve becomes the active one and a new reserve is asked for.
*/
class ReservedObjectIds {
	/// How many ids the engine is asked for in one set.
	public static setSize: number = 50;

	/// The set the ids are taken from.
	private static active: IdSet = { first: 0, count: 0 };

	/// The set that takes over once the active one is used up.
	private static reserve: IdSet = { first: 0, count: 0 };

	/// How many sets were asked for and have not come yet.
	private static asked: number = 0;

	/// Whether the answers are followed, which they are once.
	private static following: boolean = false;

	/// Fills both sets once the document can be edited.
	public static editModeChanged(edit: boolean): void {
		if (edit) ReservedObjectIds.keepEnough();
	}

	/// Asks for as many sets as are empty and not asked for yet.
	public static keepEnough(): void {
		ReservedObjectIds.follow();
		const empty =
			(ReservedObjectIds.active.count === 0 ? 1 : 0) +
			(ReservedObjectIds.reserve.count === 0 ? 1 : 0);
		for (; ReservedObjectIds.asked < empty; ++ReservedObjectIds.asked)
			app.socket.sendMessage(
				'commandvalues command=.uno:ReserveObjectIds?Count=' +
					ReservedObjectIds.setSize,
			);
	}

	/// Takes the next id of the active set, where there is one.
	public static take(): number | undefined {
		ReservedObjectIds.moveTheReserveUp();
		const set = ReservedObjectIds.active;
		const id = set.count > 0 ? set.first : undefined;
		if (id !== undefined) {
			++set.first;
			--set.count;
		}
		ReservedObjectIds.moveTheReserveUp();
		ReservedObjectIds.keepEnough();
		return id;
	}

	// a used up active set is followed by the reserve, so a new set asked for becomes the reserve
	private static moveTheReserveUp(): void {
		if (ReservedObjectIds.active.count > 0) return;
		ReservedObjectIds.active = ReservedObjectIds.reserve;
		ReservedObjectIds.reserve = { first: 0, count: 0 };
	}

	// an answer fills the active set where it is empty, else the reserve
	private static follow(): void {
		if (ReservedObjectIds.following) return;
		ReservedObjectIds.following = true;
		app.map.on('commandvalues', (event: any) => {
			if (event.commandName !== '.uno:ReserveObjectIds') return;
			ReservedObjectIds.asked = Math.max(0, ReservedObjectIds.asked - 1);
			const set: IdSet = {
				first: Number(event.commandValues?.first) || 0,
				count: Number(event.commandValues?.count) || 0,
			};
			if (set.first <= 0 || set.count <= 0) return;
			if (ReservedObjectIds.active.count === 0) ReservedObjectIds.active = set;
			else if (ReservedObjectIds.reserve.count === 0)
				ReservedObjectIds.reserve = set;
		});
	}
}

/*
	The looks of shapes kept in the browser's storage, so they hold over documents and restarts.
	They are kept under the key of the attributes they were made with, which the engine gives and
	which names its build too, for the few keys used last; looks of a key not used for long, or
	kept in another format, are dropped. Where the storage cannot be used, nothing is kept.
*/
class ShapeLookStore {
	/// The name the looks have in the storage.
	private static readonly storageName: string = 'cool-shape-looks';

	/// The format the looks are kept in; looks of another one are dropped.
	private static readonly format: number = 1;

	/// How many keys keep their looks, the ones used last.
	private static readonly keys: number = 4;

	/// When the looks are written next, where they are to be.
	private static writing: ReturnType<typeof setTimeout> | null = null;

	// what the storage holds: the looks by key, the key used last first
	private static read(): { key: string; looks: any }[] {
		try {
			const kept = JSON.parse(
				window.localStorage.getItem(ShapeLookStore.storageName) || 'null',
			);
			if (kept?.format === ShapeLookStore.format && Array.isArray(kept.keys))
				return kept.keys;
		} catch (error) {
			// a storage that cannot be read or holds no looks keeps none
		}
		return [];
	}

	/// The looks kept under that key.
	public static load(key: string): Map<string, ShapeLook> {
		const looks: Map<string, ShapeLook> = new Map();
		const kept =
			ShapeLookStore.read().find((entry) => entry.key === key)?.looks ?? {};
		for (const shape in kept) {
			const look = kept[shape];
			const box = look?.box;
			if (!Array.isArray(box) || box.length !== 4) continue;
			if (!Array.isArray(look.primitives) || !look.entry) continue;
			looks.set(shape, {
				box: new cool.Range2D(box[0], box[1], box[2], box[3]),
				primitives: look.primitives,
				asWideAsHigh: look.asWideAsHigh === true,
				entry: look.entry,
			});
		}
		return looks;
	}

	/// Keeps those looks under that key, a moment later, so a run of looks is written once.
	public static save(key: string, looks: Map<string, ShapeLook>): void {
		if (ShapeLookStore.writing !== null) clearTimeout(ShapeLookStore.writing);
		ShapeLookStore.writing = setTimeout(() => {
			ShapeLookStore.writing = null;
			const written: any = {};
			looks.forEach((look, shape) => {
				written[shape] = {
					box: [look.box.minX, look.box.minY, look.box.maxX, look.box.maxY],
					primitives: look.primitives,
					asWideAsHigh: look.asWideAsHigh,
					entry: look.entry,
				};
			});
			const keys = ShapeLookStore.read().filter((entry) => entry.key !== key);
			keys.unshift({ key: key, looks: written });
			try {
				window.localStorage.setItem(
					ShapeLookStore.storageName,
					JSON.stringify({
						format: ShapeLookStore.format,
						keys: keys.slice(0, ShapeLookStore.keys),
					}),
				);
			} catch (error) {
				// a full or blocked storage keeps what it had
			}
		}, 1000);
	}

	/// Forgets all looks kept.
	public static clear(): void {
		if (ShapeLookStore.writing !== null) clearTimeout(ShapeLookStore.writing);
		ShapeLookStore.writing = null;
		try {
			window.localStorage.removeItem(ShapeLookStore.storageName);
		} catch (error) {
			// a blocked storage holds nothing to forget
		}
	}
}

/*
	Creating a shape of the shapes grid by a drag. The library of looks knows how the shape looks
	from before or from a look ahead, or the engine is asked at once; the drag shows it, and the
	frame until it is known. Letting go creates the shape under an id reserved for this view and
	takes it as created at once, selected; the engine's report of it replaces it. Some shapes are
	as wide as high by themselves, and Shift lets those go free, as in the office. A click makes no
	shape.
*/
class ClassicShapeCreateContext extends ClassicCreateContext {
	/// The groups of the shapes grid that make a shape by a drag.
	private static readonly shapeGroups: string[] = [
		'BasicShapes',
		'SymbolShapes',
		'ArrowShapes',
		'StarShapes',
		'CalloutShapes',
		'FlowChartShapes',
	];

	/// Whether that command is one of the shapes grid.
	public static makes(shape: string): boolean {
		return (
			shape.indexOf('.') > 0 &&
			ClassicShapeCreateContext.shapeGroups.indexOf(shape.split('.')[0]) >= 0
		);
	}

	/// The looks of the shapes known so far in this session, by command; a shape looks the same
	/// whenever it is made with the same attributes, so the drag shows it at once where it is
	/// known.
	private static library: Map<string, ShapeLook> = new Map();

	/// The key of the attributes new shapes take, which the engine tells with every look; the
	/// looks in the library are all of this key.
	private static styleKey: string = '';

	/// The shapes created by a drag under the key of the library, or whose looks came from the
	/// storage; only their looks are stored, not those only asked for ahead.
	private static used: Set<string> = new Set();

	/// The shapes whose look was asked for ahead, so each is asked for once.
	private static askedAhead: Set<string> = new Set();

	/// Forgets every look the library holds, so each shape is asked for again.
	public static forgetTheLooks(): void {
		ClassicShapeCreateContext.library.clear();
		ClassicShapeCreateContext.used.clear();
		ClassicShapeCreateContext.askedAhead.clear();
	}

	/// The look of that command where the library knows it; where not, it is asked for, once.
	public static knownLook(shape: string): ShapeLook | undefined {
		ClassicShapeCreateContext.followTheGhosts();
		const look = ClassicShapeCreateContext.library.get(shape);
		if (!look && !ClassicShapeCreateContext.askedAhead.has(shape))
			ClassicShapeCreateContext.ask(shape);
		return look;
	}

	/// One of that command was made, so its look is one to keep, also over a restart.
	public static made(shape: string): void {
		ClassicShapeCreateContext.used.add(shape);
		ClassicShapeCreateContext.storeTheUsed();
	}

	/// Stores the looks of the shapes used, where the library keeps looks.
	private static storeTheUsed(): void {
		if (
			!ClassicShapeCreateContext.keeping ||
			ClassicShapeCreateContext.styleKey === ''
		)
			return;
		const looks: Map<string, ShapeLook> = new Map();
		ClassicShapeCreateContext.library.forEach((look, shape) => {
			if (ClassicShapeCreateContext.used.has(shape)) looks.set(shape, look);
		});
		ShapeLookStore.save(ClassicShapeCreateContext.styleKey, looks);
	}

	/// Whether the library keeps the looks; without it every drag asks for its shape.
	private static keeping: boolean = true;

	/// Keeps the looks of shapes from now on, or forgets them, the stored ones too, and keeps none.
	public static keepTheLooks(keep: boolean): void {
		ClassicShapeCreateContext.keeping = keep;
		if (keep) return;
		ClassicShapeCreateContext.forgetTheLooks();
		ShapeLookStore.clear();
	}

	/// Asks the engine for the key of the attributes new shapes take where it is not known yet,
	/// so the looks kept for it are there before the first drag.
	public static learnTheKey(): void {
		ClassicShapeCreateContext.followTheGhosts();
		if (ClassicShapeCreateContext.styleKey === '')
			app.socket.sendMessage('commandvalues command=.uno:ShapeStyleKey');
	}

	/// Whether the answers about shapes are followed, which they are once.
	private static followingTheGhosts: boolean = false;

	/// Takes every answer about a shape into the library, and the one about the shape of the mode
	/// that is on into that mode.
	private static followTheGhosts(): void {
		if (ClassicShapeCreateContext.followingTheGhosts) return;
		ClassicShapeCreateContext.followingTheGhosts = true;
		// the engine tells a changed key by itself as well
		app.map.on('commandstatechanged', (event: any) => {
			if (event.commandName === '.uno:ShapeStyleKey')
				ClassicShapeCreateContext.takeTheKey(event.state);
		});
		app.map.on('commandvalues', (event: any) => {
			if (event.commandName === '.uno:ShapeStyleKey') {
				ClassicShapeCreateContext.takeTheKey(event.commandValues?.styleKey);
				return;
			}
			if (event.commandName !== '.uno:ShapeGhost') return;
			const values = event.commandValues;
			ClassicShapeCreateContext.takeTheKey(values?.styleKey);
			const look = ClassicShapeCreateContext.lookOf(values);
			if (look && ClassicShapeCreateContext.keeping) {
				ClassicShapeCreateContext.library.set(values.shape, look);
				if (ClassicShapeCreateContext.used.has(values.shape))
					ClassicShapeCreateContext.storeTheUsed();
			}
			const mode = Interaction.modeOn();
			if (mode instanceof ClassicShapeCreateContext) mode.takeTheGhost(values);
		});
	}

	/*
		Where the attributes of new shapes changed, as by a change of the default drawing style,
		the looks kept are not right any more: we take those kept for the new key instead, and the
		shape of a mode that is on is asked for again.
	*/
	private static takeTheKey(key: string | undefined): void {
		if (!key || key === ClassicShapeCreateContext.styleKey) return;
		ClassicShapeCreateContext.styleKey = key;
		ClassicShapeCreateContext.forgetTheLooks();
		if (ClassicShapeCreateContext.keeping) {
			ClassicShapeCreateContext.library = ShapeLookStore.load(key);
			ClassicShapeCreateContext.library.forEach((_look, shape) =>
				ClassicShapeCreateContext.used.add(shape),
			);
		}
		const mode = Interaction.modeOn();
		if (mode instanceof ClassicShapeCreateContext)
			ClassicShapeCreateContext.ask(mode.shape);
	}

	/// Asks the engine how that shape looks.
	private static ask(shape: string): void {
		ClassicShapeCreateContext.askedAhead.add(shape);
		app.socket.sendMessage(
			'commandvalues command=.uno:ShapeGhost?Shape=' +
				encodeURIComponent(shape),
		);
	}

	/// The look an answer about a shape tells, where it tells one.
	private static lookOf(values: any): ShapeLook | null {
		const entry: cool.SlideObject | undefined = Array.isArray(values?.objects)
			? values.objects[0]
			: undefined;
		const primitives = entry?.primitives;
		// a dimension line or a connector has no box; it is laid out from its measurement, or
		// its run is worked out from its routing numbers
		const hasABox = values?.width > 0 && values?.height > 0;
		const laidOut = !!entry?.measurement || !!entry?.routing;
		if (!((hasABox || laidOut) && Array.isArray(primitives))) return null;
		const x = values.x ?? 0;
		const y = values.y ?? 0;
		return {
			box: new cool.Range2D(x, y, x + values.width, y + values.height),
			primitives: primitives,
			asWideAsHigh: values.asWideAsHigh === true,
			entry: { ...(entry as cool.SlideObject), id: undefined },
		};
	}

	/*
		Asks the engine how a shape looks, ahead of its use, where the library does not know it
		yet: when the mouse rests on it in the shapes grid.
	*/
	public static lookAhead(shape: string): void {
		if (
			!ClassicShapeCreateContext.makes(shape) ||
			!RenderGeometrySection.answersPointer() ||
			!ClassicShapeCreateContext.keeping ||
			ClassicShapeCreateContext.library.has(shape) ||
			ClassicShapeCreateContext.askedAhead.has(shape)
		)
			return;
		ClassicShapeCreateContext.followTheGhosts();
		ReservedObjectIds.keepEnough();
		ClassicShapeCreateContext.ask(shape);
	}

	/*
		Shows the shape at once where the library knows how it looks, and asks the engine whether
		the attributes of new shapes are still those of the library; else it asks for the look.
	*/
	public started(): void {
		ClassicShapeCreateContext.followTheGhosts();
		ReservedObjectIds.keepEnough();
		const look = ClassicShapeCreateContext.library.get(this.shape);
		if (!look) {
			ClassicShapeCreateContext.ask(this.shape);
			return;
		}
		this.asWideAsHigh = look.asWideAsHigh;
		this.ghost = look;
		app.socket.sendMessage('commandvalues command=.uno:ShapeStyleKey');
	}

	/// The office shows the plain cross while a custom shape is created.
	public pointer(): string {
		return 'crosshair';
	}

	/// How the shape looks in its default size, as the engine made it, and the size it was
	/// made in, in twips; null until the engine or the library told.
	private ghost: ShapeLook | null = null;

	/// The id given to the shape sent, one reserved for this view; undefined where none was at
	/// hand, and the shape then takes one of its own.
	private givenId: number | undefined = undefined;

	/// Whether the shape is always as wide as high, as the engine makes some.
	private asWideAsHigh: boolean = false;

	protected asWideAsHighByItself(): boolean {
		return this.asWideAsHigh;
	}

	/// Takes the engine's answer about the shape, where it is about this one.
	private takeTheGhost(values: any): void {
		if (!values || values.shape !== this.shape) return;
		const look = ClassicShapeCreateContext.lookOf(values);
		if (look) {
			this.asWideAsHigh = look.asWideAsHigh;
			this.ghost = look;
		}
		app.sectionContainer?.requestReDraw();
	}

	/// The mapping of the box the shape was made in onto that rectangle.
	private mappingOnto(box: cool.Range2D): cool.Matrix2D {
		const made = this.ghost?.box ?? box;
		const across = box.width / made.width;
		const down = box.height / made.height;
		return new cool.Matrix2D(
			across,
			0,
			0,
			down,
			box.minX - made.minX * across,
			box.minY - made.minY * down,
		);
	}

	/*
		The shape as created in that rectangle, as the engine is to report it: its entry with what
		it draws, its box and its mapping taken onto the rectangle. Its own handles and path are
		left to the engine's report, which brings them where they are.
	*/
	private createdIn(box: cool.Range2D, id: number): cool.SlideObject | null {
		const entry = this.ghost?.entry;
		if (!entry) return null;
		const mapping = this.mappingOnto(box);
		const transform = cool.Matrix2D.fromArray(entry.transform);
		const at = (x: number, y: number) =>
			new cool.Point(
				mapping.a * x + mapping.c * y + mapping.e,
				mapping.b * x + mapping.d * y + mapping.f,
			);
		const from = at(entry.x ?? 0, entry.y ?? 0);
		const to = at(
			(entry.x ?? 0) + (entry.width ?? 0),
			(entry.y ?? 0) + (entry.height ?? 0),
		);
		return {
			...entry,
			id: id,
			x: from.x,
			y: from.y,
			width: to.x - from.x,
			height: to.y - from.y,
			transform: transform ? transform.then(mapping).toArray() : undefined,
			primitives: [
				{
					type: cool.TransformPrimitive.type,
					matrix: mapping.toArray(),
					children: entry.primitives ?? [],
				} as cool.TransformPrimitive,
			],
			handles: undefined,
			path: undefined,
		};
	}

	/*
		Draws the shape as it looks, mapped from the box it was made in onto that rectangle: half
		see-through while it is dragged, as a drag shows what it moves, and whole once it is sent,
		as the shape the engine is making. Answers whether it could.
	*/
	protected drawTheObject(
		context: CanvasRenderingContext2D,
		box: cool.Range2D,
		whole: boolean,
	): boolean {
		const data = RenderGeometrySection.currentPart();
		if (!this.ghost || !data) return false;

		context.save();
		// the point at the very start of the document says where the part begins on the canvas,
		// and a twip is that many pixels wide
		const origin = new cool.SimplePoint(0, 0);
		context.translate(origin.vX, origin.vY);
		context.scale(app.twipsToPixels, app.twipsToPixels);
		if (!whole) context.globalAlpha = 0.5;
		const mapping = this.mappingOnto(box);
		RenderManager.renderPrimitives(context, data, [
			{
				type: cool.TransformPrimitive.type,
				matrix: mapping.toArray(),
				children: this.ghost.primitives,
			} as cool.TransformPrimitive,
		]);
		context.restore();
		return true;
	}

	/// The id the shape is to take, the next reserved for this view, where one is at hand.
	protected moreArguments(): { [name: string]: { type: string; value: any } } {
		this.givenId = ReservedObjectIds.take();
		return this.givenId === undefined
			? {}
			: { Id: { type: 'unsigned long', value: String(this.givenId) } };
	}

	/*
		Where the shape was given an id and its look is known, the shape is taken as created at
		once, under that id, which is its real one: it is in the part and selected, and the
		engine's report of it replaces it.
	*/
	protected wasSent(box: cool.Range2D): void {
		ClassicShapeCreateContext.made(this.shape);

		const id = this.givenId;
		const created = id === undefined ? null : this.createdIn(box, id);
		const part = RenderGeometrySection.currentPart();
		if (
			created &&
			created.id !== undefined &&
			part &&
			RenderManager.addProvisionalObject(
				part.partId,
				created,
				ClassicCreateContext.longestWait,
			)
		) {
			this.forgetTheSent();
			GraphicSelection.selectAndShow([created.id]);
		}
	}
}

/*
	Creating a text box, as the office's text tool does: a drag makes one of the dragged width,
	which grows downwards with its text, and a click makes one at that point that grows in width
	as well. Either goes into its text at once. An empty text box draws nothing, so the drag shows
	its frame.
*/
class ClassicTextCreateContext extends ClassicCreateContext {
	/// Whether that command makes a text box.
	public static makes(shape: string): boolean {
		return shape === ClassicCreateContext.text;
	}

	/// The room a clicked text box starts with, in CSS pixels, before its text widens it.
	private static readonly room: number = 6;

	/// Whether the text box sent grows in width with its text.
	private grows: boolean = false;

	/// The office shows its text pointer while a text box is made.
	public pointer(): string {
		return (
			'url(' + app.LOUtil.getURL('images/cursors/draw_text.svg') + ') 8 8, text'
		);
	}

	// the kinds of object that hold no text of their own: a picture, an embedded object
	private static readonly holdNoText: number[] = [22, 23];

	/*
		A click on an object that holds text edits that text, with the cursor where it was clicked,
		as the office's text tool does: the mode ends and the click goes to the engine there. A
		click anywhere else makes a text box there.
	*/
	protected clicked(at: cool.Point, pressedAt: cool.Point): boolean {
		const hit = RenderGeometrySection.objectIdAt(pressedAt.x, pressedAt.y);
		const kind =
			hit === undefined
				? undefined
				: RenderGeometrySection.objectOf(hit)?.objectKind;
		if (
			hit !== undefined &&
			(kind === undefined ||
				ClassicTextCreateContext.holdNoText.indexOf(kind) < 0)
		) {
			Interaction.leaveMode();
			const layer = app.map._docLayer;
			layer._postMouseEvent('buttondown', pressedAt.x, pressedAt.y, 1, 1, 0);
			layer._postMouseEvent('buttonup', pressedAt.x, pressedAt.y, 1, 1, 0);
			return true;
		}

		const room = ClassicCreateContext.twips(ClassicTextCreateContext.room);
		this.grows = true;
		this.send(new cool.Range2D(at.x, at.y, at.x + room, at.y + room));
		return true;
	}

	protected moreArguments(): { [name: string]: { type: string; value: any } } {
		return { Grow: { type: 'boolean', value: this.grows } };
	}
}

/*
	Creating a line by dragging it out from its start to its end, as the office does: Shift keeps
	it at a multiple of 45 degrees, or lets the line at 45 degrees go free, Alt drags it out from
	its middle, and Ctrl turns the snap over.
	A click makes no line. While dragged and until the engine reports it, the line is drawn plain,
	without its arrows; letting go creates it under an id reserved for this view and takes it as
	created at once, selected.
*/
class ClassicLineCreateContext extends ClassicCreateContext {
	/// The commands that make a line, from its start to its end.
	private static readonly lines: string[] = [
		'Line',
		'Line_Diagonal',
		'MeasureLine',
		'LineArrowStart',
		'LineArrowEnd',
		'LineArrows',
		'LineArrowCircle',
		'LineCircleArrow',
		'LineArrowSquare',
		'LineSquareArrow',
	];

	/// Whether that command makes a line.
	public static makes(shape: string): boolean {
		return ClassicLineCreateContext.lines.indexOf(shape) >= 0;
	}

	/// The start and the end of the line as dragged, in twips; null while no drag runs.
	protected start: cool.Point | null = null;
	protected end: cool.Point | null = null;

	/// The id given to the line sent, one reserved for this view; undefined where none was at
	/// hand, and the line then takes one of its own.
	private givenId: number | undefined = undefined;

	/// The look of a dimension line is asked for at once, where it is not known yet.
	public started(): void {
		ReservedObjectIds.keepEnough();
		if (this.shape === ClassicLineCreateContext.measureLine)
			ClassicShapeCreateContext.knownLook(this.shape);
	}

	/// The office shows the plain cross while a line is created.
	public pointer(): string {
		return 'crosshair';
	}

	/// The line from where the drag began to that point, and the rectangle around it.
	protected dragTo(
		from: cool.Point,
		to: cool.Point,
		modifier: number,
	): cool.Range2D | null {
		let x = to.x - from.x;
		let y = to.y - from.y;
		// as in the office, Shift turns the line to the nearest multiple of 45 degrees; the line at
		// 45 degrees keeps to those by itself, and Shift lets it go free
		const shift = (modifier & app.UNOModifier.SHIFT) !== 0;
		if (shift !== (this.shape === 'Line_Diagonal')) {
			const step = Math.PI / 4;
			const angle = Math.round(Math.atan2(y, x) / step) * step;
			const length = Math.hypot(x, y);
			x = Math.cos(angle) * length;
			y = Math.sin(angle) * length;
		}
		this.end = new cool.Point(from.x + x, from.y + y);
		// with Alt the drag began at the middle of the line
		this.start =
			(modifier & app.UNOModifier.ALT) !== 0
				? new cool.Point(from.x - x, from.y - y)
				: from;
		return cool.Range2D.fromPoints([this.start, this.end]);
	}

	/// A line is long enough where its length is, whatever its direction.
	protected largeEnough(_box: cool.Range2D, least: number): boolean {
		return (
			!!this.start &&
			!!this.end &&
			Math.hypot(this.end.x - this.start.x, this.end.y - this.start.y) >= least
		);
	}

	protected placeArguments(_box: cool.Range2D): {
		[name: string]: { type: string; value: any };
	} {
		const start = this.start ?? new cool.Point(0, 0);
		const end = this.end ?? start;
		return {
			StartX: ClassicCreateContext.twipsArgument(start.x),
			StartY: ClassicCreateContext.twipsArgument(start.y),
			EndX: ClassicCreateContext.twipsArgument(end.x),
			EndY: ClassicCreateContext.twipsArgument(end.y),
		};
	}

	/// The id the line is to take, the next reserved for this view, where one is at hand.
	protected moreArguments(): { [name: string]: { type: string; value: any } } {
		this.givenId = ReservedObjectIds.take();
		return this.givenId === undefined
			? {}
			: { Id: { type: 'unsigned long', value: String(this.givenId) } };
	}

	/// The length of the line and its angle, counted upwards from the right as the office does.
	protected readingOf(_size: cool.Point): string {
		if (!this.start || !this.end) return '';
		// a dimension line shows the value it will write
		const value = this.measured()?.value;
		if (value) return value;
		const x = this.end.x - this.start.x;
		const y = this.end.y - this.start.y;
		const degrees = (-Math.atan2(y, x) * 180) / Math.PI;
		return (
			ClassicCreateContext.inTheUnit(Math.hypot(x, y)) +
			', ' +
			((degrees + 360) % 360).toFixed(1) +
			ClassicLineCreateContext.degreeSign
		);
	}

	/// The sign after an angle in degrees.
	private static readonly degreeSign: string = String.fromCharCode(0xb0);

	/// The command of the dimension line, whose look the engine tells.
	private static readonly measureLine: string = 'MeasureLine';

	/// The dimension line as dragged, laid out from the look of the default one: its lines, the
	/// box of its text and the value it writes. Nothing for another kind, or before the look is
	/// known.
	private measured(): {
		outline: cool.ObjectPathPolygon[];
		value: string | null;
	} | null {
		if (this.shape !== ClassicLineCreateContext.measureLine) return null;
		if (!this.start || !this.end) return null;
		const entry = ClassicShapeCreateContext.knownLook(this.shape)?.entry;
		const square = cool.Matrix2D.fromArray(entry?.transform);
		if (!entry?.measurement || !square) return null;
		return MeasurementGhost.between(
			entry.measurement,
			square,
			this.start,
			this.end,
		);
	}

	/// What the stand-in draws as hairlines: the lines of a dimension line where its look is
	/// known; nothing for the plain line.
	protected standInPolygons(): cool.ObjectPathPolygon[] | null {
		return this.measured()?.outline ?? null;
	}

	/// Draws the line plain from its start to its end, half see-through while it is dragged; a
	/// dimension line as laid out where its look is known.
	protected drawTheObject(
		context: CanvasRenderingContext2D,
		_box: cool.Range2D,
		whole: boolean,
	): boolean {
		if (!this.start || !this.end) return false;
		context.save();
		context.globalAlpha = whole ? 1 : 0.5;
		const measured = this.measured();
		if (measured) {
			SelectionSection.strokeOutline(
				context,
				measured.outline,
				HandleLook.outlineColour,
			);
		} else {
			const from = new cool.SimplePoint(this.start.x, this.start.y);
			const to = new cool.SimplePoint(this.end.x, this.end.y);
			context.strokeStyle = HandleLook.outlineColour;
			context.lineWidth = Math.max(1, app.dpiScale);
			context.beginPath();
			context.moveTo(from.vX, from.vY);
			context.lineTo(to.vX, to.vY);
			context.stroke();
		}
		context.restore();
		return true;
	}

	/*
		Where the line was given an id, it is taken as created at once, under that id, which is its
		real one: it is in the part and selected, and the engine's report of it replaces it with
		its arrows.
	*/
	protected wasSent(box: cool.Range2D): void {
		// a kind with a look keeps it, also over a restart; a plain line has none to keep
		ClassicShapeCreateContext.made(this.shape);

		const id = this.givenId;
		const part = RenderGeometrySection.currentPart();
		if (id === undefined || !part || !this.start || !this.end) return;
		// the stand-in is the line, or the lines of the dimension line, as hairlines
		const polygons: cool.ObjectPathPolygon[] = this.standInPolygons() ?? [
			{ points: [this.start, this.end] },
		];
		const created: cool.SlideObject = {
			id: id,
			x: box.minX,
			y: box.minY,
			width: box.width,
			height: box.height,
			primitives: polygons.map(
				(polygon) =>
					({
						type: cool.PolygonHairlinePrimitive.type,
						color: HandleLook.outlineColour,
						path:
							(polygon.points ?? [])
								.map(
									(point, at) => (at ? 'L ' : 'M ') + point.x + ' ' + point.y,
								)
								.join(' ') + (polygon.closed ? ' Z' : ''),
					}) as cool.PolygonHairlinePrimitive,
			),
		};
		if (
			RenderManager.addProvisionalObject(
				part.partId,
				created,
				ClassicCreateContext.longestWait,
			)
		) {
			this.forgetTheSent();
			GraphicSelection.selectAndShow([id]);
		}
	}
}

/// Where an end of a connector ties itself: the object, and the place on it as the engine counts
/// it.
type ConnectorEndTie = ReturnType<typeof SelectionSection.tieOn>;

/*
	Creating a connector by dragging it out from its start to its end, as the office does: the
	object under the mouse shows the places an end can tie itself to, already before the press,
	and the start and the end snap to the one within reach, which is lit. An end let go of over
	none of them stays free. The connector is drawn as a line between its ends until the engine
	reports it, tied as the client found its ends.
*/
class ClassicConnectorCreateContext extends ClassicLineCreateContext {
	/// The commands that make a connector.
	private static readonly connectors: string[] = [
		'Connector',
		'ConnectorArrowStart',
		'ConnectorArrowEnd',
		'ConnectorArrows',
		'ConnectorCircleStart',
		'ConnectorCircleEnd',
		'ConnectorCircles',
		'ConnectorLine',
		'ConnectorLineArrowStart',
		'ConnectorLineArrowEnd',
		'ConnectorLineArrows',
		'ConnectorLineCircleStart',
		'ConnectorLineCircleEnd',
		'ConnectorLineCircles',
		'ConnectorCurve',
		'ConnectorCurveArrowStart',
		'ConnectorCurveArrowEnd',
		'ConnectorCurveArrows',
		'ConnectorCurveCircleStart',
		'ConnectorCurveCircleEnd',
		'ConnectorCurveCircles',
		'ConnectorLines',
		'ConnectorLinesArrowStart',
		'ConnectorLinesArrowEnd',
		'ConnectorLinesArrows',
		'ConnectorLinesCircleStart',
		'ConnectorLinesCircleEnd',
		'ConnectorLinesCircles',
	];

	/// Whether that command makes a connector.
	public static makes(shape: string): boolean {
		return ClassicConnectorCreateContext.connectors.indexOf(shape) >= 0;
	}

	/// The object under the mouse, whose places to tie to are shown; null over none.
	private aimed: number | null = null;

	/// Where the mouse would tie an end now, and where the start and the end are tied.
	private here: ConnectorEndTie = null;
	private startTie: ConnectorEndTie = null;
	private endTie: ConnectorEndTie = null;

	/// The look of the connector is asked for at once, where it is not known yet: it holds the
	/// numbers its run is worked out from.
	public started(): void {
		super.started();
		ClassicShapeCreateContext.knownLook(this.shape);
	}

	/*
		The run the connector will take, worked out the way the engine works it out, from the
		numbers of the default connector of its command and what its ends tie to: the place on
		an object, or the point a free end lies at. Nothing before those numbers are known.
	*/
	private run(): cool.ObjectPathPoint[] | undefined {
		const routing = ClassicShapeCreateContext.knownLook(this.shape)?.entry
			?.routing;
		const data = RenderGeometrySection.currentPart();
		if (!routing || !data || !this.start || !this.end) return undefined;

		const tieOf = (tie: ConnectorEndTie, at: cool.Point): cool.ConnectorTie =>
			tie ? tie.tie : { x: at.x, y: at.y };
		const nodeOf = (tie: ConnectorEndTie) =>
			tie ? ConnectorRouter.nodeOf(data.objects.get(tie.objectId)) : undefined;
		return ConnectorRouter.route(
			{
				...routing,
				trackUserDefined: false,
				lineDeltas: routing.lineDeltas.map(() => 0),
				tieAtStart: tieOf(this.startTie, this.start),
				tieAtEnd: tieOf(this.endTie, this.end),
			},
			nodeOf(this.startTie),
			nodeOf(this.endTie),
		);
	}

	/// The connector along the run it will take, where that is known, else as a line.
	protected drawTheObject(
		context: CanvasRenderingContext2D,
		box: cool.Range2D,
		whole: boolean,
	): boolean {
		const run = this.run();
		if (!run) return super.drawTheObject(context, box, whole);
		context.save();
		context.globalAlpha = whole ? 1 : 0.5;
		SelectionSection.strokeOutline(
			context,
			[{ closed: false, points: run }],
			HandleLook.outlineColour,
		);
		context.restore();
		return true;
	}

	/// The stand-in follows the run as well, where it is known.
	protected standInPolygons(): cool.ObjectPathPolygon[] | null {
		const run = this.run();
		return run ? [{ closed: false, points: run }] : super.standInPolygons();
	}

	/// An end lands on the place within reach on the object under the mouse, else as a line's.
	protected placeOf(at: cool.Point, modifier: number): cool.Point {
		this.aimed = SelectionSection.tieTargetAt(at);
		this.here =
			this.aimed === null ? null : SelectionSection.tieOn(this.aimed, at);
		return this.here?.point ?? (this.here ? at : this.snapped(at, modifier));
	}

	mouseDown(at: cool.Point, modifier: number): void {
		super.mouseDown(at, modifier);
		this.startTie = this.here;
		this.endTie = null;
	}

	/// A connector goes from where the drag began to the mouse; Shift and Alt change nothing.
	protected dragTo(
		from: cool.Point,
		to: cool.Point,
		_modifier: number,
	): cool.Range2D | null {
		this.start = from;
		this.end = to;
		this.endTie = this.here;
		return cool.Range2D.fromPoints([from, to]);
	}

	/// The ties go along as four numbers: the object at the start and its glue point, then the
	/// same for the end. An object of 0 leaves that end free.
	protected moreArguments(): { [name: string]: { type: string; value: any } } {
		const counted = (tie: ConnectorEndTie): string =>
			tie
				? tie.objectId + ' ' + SelectionSection.gluePointIndexOf(tie.tie)
				: '0 -1';
		return {
			...super.moreArguments(),
			Ties: {
				type: 'string',
				value: counted(this.startTie) + ' ' + counted(this.endTie),
			},
		};
	}

	/// A connector shows its length only.
	protected readingOf(_size: cool.Point): string {
		if (!this.start || !this.end) return '';
		return ClassicCreateContext.inTheUnit(
			Math.hypot(this.end.x - this.start.x, this.end.y - this.start.y),
		);
	}

	/// The places to tie to on the object under the mouse, and the one the start is tied to.
	protected drawMore(context: CanvasRenderingContext2D): void {
		if (this.aimed !== null)
			SelectionSection.paintTargets(
				context,
				this.aimed,
				this.here?.point ?? null,
			);
		if (this.startTie && this.startTie.objectId !== this.aimed)
			SelectionSection.paintTargets(
				context,
				this.startTie.objectId,
				this.startTie.point ?? null,
			);
	}
}
