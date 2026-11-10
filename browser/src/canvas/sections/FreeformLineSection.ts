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

/// A line sent to the engine as an object, the ticket of its message, and the wait for the
/// answer that brings the object.
interface SentFreeformLine {
	places: cool.Point[];
	ticket: cool.InputTicket;
	wait: ReturnType<typeof setTimeout> | null;
}

/*
	The freeform line tool of a view that draws from objects. While the tool is chosen, the
	section lies over the whole page and takes the mouse: a press starts a line, the line follows
	the mouse while the button is held, and letting go of the button ends it. The engine is then
	asked to add the line as a polyline, which it selects, and the tool is put away.

	The line is drawn here while it is being drawn, and after that until the engine's answer
	brings the object. The section is there only for as long as one of these is the case.
*/
class FreeformLineSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.FreeformLine.processingOrder;
	drawingOrder: number = app.CSections.FreeformLine.drawingOrder;
	zIndex: number = app.CSections.FreeformLine.zIndex;
	documentObject: boolean = true;

	/// The command of the client that chooses the tool, whose state is whether it is chosen.
	public static readonly command: string = 'freeformline';

	/// The least distance from one place of the line to the next, in CSS pixels.
	private static readonly leastStep: number = 2;

	/// How far the line sent to the engine may stray from the line drawn, in CSS pixels.
	private static readonly tolerance: number = 1;

	/// How long the line sent stays drawn when no answer of the engine brings it, in
	/// milliseconds. The engine adds nothing for a layer that is locked, for one.
	private static readonly longestWait: number = 5000;

	/// Whether the next press draws a line.
	private chosen: boolean = false;

	/// The line being drawn while the button is held, or null.
	private stroke: cool.FreeformStroke | null = null;

	/// Where the mouse last was during the drawing, on the section. The view can scroll under a
	/// mouse that stands still, and the line goes on to where the mouse then is on the page.
	private lastAt: cool.SimplePoint | null = null;

	/// The lines sent to the engine that it has not answered yet, the oldest first.
	private sent: SentFreeformLine[] = [];

	/// Whether the class follows the answers of the engine, which it does once.
	private static listening: boolean = false;

	/// The state of the command last told to the toolbars.
	private static toldAs: boolean = false;

	constructor() {
		super(app.CSections.FreeformLine.name);
		this.size = app.sectionContainer.getDocumentAnchorSection().size;
	}

	/// The section, where it is there.
	private static section(): FreeformLineSection | null {
		const section = app.sectionContainer?.getSectionWithName(
			app.CSections.FreeformLine.name,
		);
		return section instanceof FreeformLineSection ? section : null;
	}

	/// Whether the tool is chosen, so the next press on the page draws a line.
	public static isChosen(): boolean {
		return FreeformLineSection.section()?.chosen === true;
	}

	/// Chooses the tool, or puts it away where it is chosen. Only a view that draws from objects
	/// and may edit the document has the tool.
	public static choose(): void {
		if (!RenderGeometrySection.answersPointer() || !app.map.isEditMode())
			return;

		const existing = FreeformLineSection.section();
		if (existing?.chosen) {
			existing.putAway();
			return;
		}

		FreeformLineSection.listen();
		const section = existing ?? new FreeformLineSection();
		if (!existing) app.sectionContainer.addSection(section);

		section.chosen = true;
		section.interactable = true;
		FreeformLineSection.tell();
	}

	/// Puts the tool away, with the line being drawn. Answers whether the tool was chosen.
	public static drop(): boolean {
		const section = FreeformLineSection.section();
		if (!section?.chosen) return false;

		section.putAway();
		return true;
	}

	private putAway(): void {
		// The press that draws the line ends here, so its release and its click reach no section.
		if (this.stroke) this.containerObject.cancelPress();
		this.stopDrawing();

		this.chosen = false;
		this.interactable = false;
		this.context.canvas.style.cursor = '';
		FreeformLineSection.tell();
		this.leaveWhenDone();
	}

	/// Tells the toolbars whether the tool is chosen, where that changed.
	private static tell(): void {
		const chosen = FreeformLineSection.isChosen();
		if (chosen === FreeformLineSection.toldAs) return;

		FreeformLineSection.toldAs = chosen;
		app.map.fire('commandstatechanged', {
			commandName: FreeformLineSection.command,
			state: chosen ? 'true' : 'false',
		});
	}

	/// Leaves the page where the tool is put away and no line is drawn.
	private leaveWhenDone(): void {
		if (this.chosen || this.stroke || this.sent.length) {
			this.containerObject.requestReDraw();
			return;
		}

		if (FreeformLineSection.section() !== this) return;

		app.sectionContainer.removeSection(this.name);
		app.sectionContainer.requestReDraw();
	}

	/// Follows the answers of the engine, so a line sent goes once its object is drawn.
	private static listen(): void {
		if (FreeformLineSection.listening) return;
		FreeformLineSection.listening = true;

		RenderManager.onVectorChanged(
			(_answered?: number[], inputSequence?: number) => {
				const section = FreeformLineSection.section();
				if (!section || inputSequence === undefined) return;

				const answered = section.sent.filter((line: SentFreeformLine) =>
					line.ticket.appliedBy(inputSequence),
				);
				if (answered.length) section.forget(answered);
			},
		);
	}

	/// Stops drawing those lines sent, and waiting for their answers.
	private forget(lines: SentFreeformLine[]): void {
		for (const line of lines)
			if (line.wait !== null) app.timerRegistry.clearTimeout(line.wait);

		this.sent = this.sent.filter(
			(line: SentFreeformLine) => lines.indexOf(line) < 0,
		);
		this.leaveWhenDone();
	}

	/// The section covers the page wherever the document is shown.
	isHit(point: number[]): boolean {
		const page = this.containerObject.getDocumentAnchorSection();
		return (
			point[0] >= page.myTopLeft[0] &&
			point[0] <= page.myTopLeft[0] + page.size[0] &&
			point[1] >= page.myTopLeft[1] &&
			point[1] <= page.myTopLeft[1] + page.size[1]
		);
	}

	// The section is added while the document is on screen, so it places itself right away.
	onInitialize(): void {
		this.onNewDocumentTopLeft();
	}

	onNewDocumentTopLeft(): void {
		Util.ensureValue(app.activeDocument);
		const viewed = app.activeDocument.activeLayout.viewedRectangle;
		this.setPosition(viewed.pX1, viewed.pY1);

		if (this.stroke && this.lastAt) this.drawTo(this.lastAt);
	}

	/// The place on the page, in twips, of a point on the section.
	private inTheDocument(point: cool.SimplePoint): cool.Point {
		const inDocument = point.clone();
		inDocument.pX += this.position[0];
		inDocument.pY += this.position[1];
		return new cool.Point(inDocument.x, inDocument.y);
	}

	/// Takes the line on to that point of the section.
	private drawTo(point: cool.SimplePoint): void {
		if (!this.stroke) return;

		this.lastAt = point.clone();
		if (this.stroke.add(this.inTheDocument(point)))
			this.containerObject.requestReDraw();
	}

	private stopDrawing(): void {
		this.stroke = null;
		this.lastAt = null;
		// The drawing was scrolling the view while it reached past the edge.
		app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
	}

	onMouseDown(point: cool.SimplePoint, _e: MouseEvent): void {
		if (!this.chosen) return;

		const pixel = app.dpiScale * app.pixelsToTwips;
		this.stroke = new cool.FreeformStroke(
			FreeformLineSection.leastStep * pixel,
		);
		this.drawTo(point);
	}

	onMouseMove(
		point: cool.SimplePoint,
		_dragDistance: number[],
		_e: MouseEvent,
	): void {
		if (!this.stroke) {
			if (this.chosen) this.context.canvas.style.cursor = 'crosshair';
			return;
		}

		this.drawTo(point);

		// The view scrolls on by itself while the mouse stands near an edge of it, or outside.
		const onCanvas = point.clone();
		onCanvas.pX += this.myTopLeft[0];
		onCanvas.pY += this.myTopLeft[1];
		app.map.fire('handleautoscroll', {
			pos: { x: onCanvas.cX, y: onCanvas.cY },
			map: app.map,
		});
	}

	/*
		Letting go of the button ends the line. A press the container did not take as a drag is a
		click, which draws no line and leaves the tool chosen, so the click that follows it comes
		here too.
	*/
	onMouseUp(point: cool.SimplePoint, _e: MouseEvent): void {
		if (!this.stroke) return;

		this.drawTo(point);
		const stroke = this.stroke;
		this.stopDrawing();

		if (this.containerObject.isDraggingSomething() && stroke.drawsALine())
			this.send(stroke);
		this.containerObject.requestReDraw();
	}

	onDragCancel(): void {
		this.stopDrawing();
		this.containerObject.requestReDraw();
	}

	/*
		Asks the engine to add the line as a polyline and puts the tool away. The line stays drawn
		until the answer that brings the object, which is told by the sequence number of the
		message. What waits to go out to the engine goes first, so the line is added after it.
	*/
	private send(stroke: cool.FreeformStroke): void {
		const pixel = app.dpiScale * app.pixelsToTwips;
		const places = cool.FreeformStroke.simplified(
			stroke.placesSoFar(),
			FreeformLineSection.tolerance * pixel,
		);

		ChangeOutbox.sendEverythingAhead();
		const ticket = new cool.InputTicket();
		const sequence = cool.InputSequence.next([ticket], false);
		app.socket.sendMessage(
			cool.FreeformStroke.message(places, false) + ' sequence=' + sequence,
		);

		const line: SentFreeformLine = { places, ticket, wait: null };
		line.wait = app.timerRegistry.setTimeout(
			'freeformlineanswer',
			() => {
				line.wait = null;
				this.forget([line]);
			},
			FreeformLineSection.longestWait,
		);
		this.sent.push(line);

		this.putAway();
	}

	/// The lines sent that wait for the engine, and the line being drawn.
	private linesToDraw(): ReadonlyArray<cool.Point>[] {
		const lines: ReadonlyArray<cool.Point>[] = this.sent.map(
			(line: SentFreeformLine) => line.places,
		);
		if (this.stroke) lines.push(this.stroke.placesSoFar());
		return lines.filter((line: ReadonlyArray<cool.Point>) => line.length >= 2);
	}

	onDraw(): void {
		const lines = this.linesToDraw();
		if (!lines.length) return;

		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		// The point at the very start of the document says where the drawing of the part begins
		// on the canvas, and a twip is that many pixels wide.
		const origin = new cool.SimplePoint(0, 0);
		this.context.translate(origin.vX, origin.vY);
		this.context.scale(app.twipsToPixels, app.twipsToPixels);
		this.context.strokeStyle = HandleLook.outlineOfADrag;
		this.context.lineWidth = app.dpiScale * app.pixelsToTwips;
		this.context.lineJoin = 'round';
		this.context.lineCap = 'round';

		this.context.beginPath();
		for (const places of lines) {
			this.context.moveTo(places[0].x, places[0].y);
			for (let at = 1; at < places.length; ++at)
				this.context.lineTo(places[at].x, places[at].y);
		}
		this.context.stroke();

		this.context.restore();
	}
}
