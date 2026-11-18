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
	The mouse of an interaction mode: while a mode is on, the section lies over the whole page and
	hands every press, movement and release on it to the mode, and draws what the mode shows. It
	knows nothing of what the mode does. It stays a while after the mode is over where the mode
	still draws something that waits for the engine, taking no mouse then.
*/
class InteractionModeSection extends WholePageSection {
	processingOrder: number = app.CSections.InteractionMode.processingOrder;
	drawingOrder: number = app.CSections.InteractionMode.drawingOrder;
	zIndex: number = app.CSections.InteractionMode.zIndex;

	/// The mode last on, which may still draw after it is over.
	private static lastMode: InteractionMode | null = null;

	/// The press and the release of the right button over the canvas, which the container does
	/// not hand on.
	private rightPressed: (event: PointerEvent) => void;
	private rightReleased: (event: PointerEvent) => void;

	constructor() {
		super(app.CSections.InteractionMode.name);
		this.rightPressed = (event: PointerEvent) => this.onRight(event, true);
		this.rightReleased = (event: PointerEvent) => this.onRight(event, false);
	}

	onInitialize(): void {
		super.onInitialize();
		this.context.canvas.addEventListener('pointerdown', this.rightPressed);
		this.context.canvas.addEventListener('pointerup', this.rightReleased);
	}

	onRemove(): void {
		this.context.canvas.removeEventListener('pointerdown', this.rightPressed);
		this.context.canvas.removeEventListener('pointerup', this.rightReleased);
	}

	/// The place on the page, in twips, of a mouse event over the canvas.
	private inTheDocumentAt(event: MouseEvent): cool.Point {
		const rect = this.context.canvas.getBoundingClientRect();
		return this.inTheDocument(
			cool.SimplePoint.fromCorePixels([
				(event.clientX - rect.left) * app.dpiScale - this.myTopLeft[0],
				(event.clientY - rect.top) * app.dpiScale - this.myTopLeft[1],
			]),
		);
	}

	private onRight(event: PointerEvent, down: boolean): void {
		const mode = Interaction.modeOn();
		if (event.button !== 2 || !mode || !mode.takesTheRightButton()) return;
		const at = this.inTheDocumentAt(event);
		const modifier = MouseControl.readModifier(event);
		if (down) mode.rightDown(at, modifier);
		else mode.rightUp(at, modifier);
		this.containerObject.requestReDraw();
	}

	// the menu opens on the press or on the release, as the system has it
	onContextMenu(_point: cool.SimplePoint, e: MouseEvent): void {
		if (!Interaction.modeOn()?.takesTheRightButton()) return;
		e.preventDefault();
		e.stopPropagation();
		this.stopPropagating();
	}

	/// How long the double click that follows a release is waited for, in milliseconds.
	private static readonly doubleClickWait: number = 500;

	/*
		Passes over the double click the browser reports after the release that just ended a mode,
		so it reaches nothing under the mode, which would start a text edit on the object made, or
		take the selection away from it. It is caught at the window, before the canvas has it; one
		that does not come soon is no longer waited for.
	*/
	public static passOverTheDoubleClick(): void {
		const passOver = (event: MouseEvent) => {
			if (event.target !== document.getElementById('document-canvas')) return;
			event.stopPropagation();
			event.preventDefault();
			stop();
		};
		const stop = () => {
			app.timerRegistry.clearTimeout(wait);
			window.removeEventListener('dblclick', passOver, true);
		};
		const wait = app.timerRegistry.setTimeout(
			'passoverdoubleclick',
			stop,
			InteractionModeSection.doubleClickWait,
		);
		window.addEventListener('dblclick', passOver, true);
	}

	/// The section, where it is there.
	private static section(): InteractionModeSection | null {
		const section = app.sectionContainer?.getSectionWithName(
			app.CSections.InteractionMode.name,
		);
		return section instanceof InteractionModeSection ? section : null;
	}

	/// Puts the section over the page while a mode is on or still draws, and takes it away after.
	public static follow(): void {
		const mode = Interaction.modeOn();
		if (mode) InteractionModeSection.lastMode = mode;
		const drawing = mode ?? InteractionModeSection.lastMode;
		const needed = !!mode || !!drawing?.stillDraws();

		let section = InteractionModeSection.section();
		if (!needed) {
			InteractionModeSection.lastMode = null;
			if (section) {
				section.context.canvas.style.cursor = '';
				app.sectionContainer.removeSection(section.name);
			}
			app.sectionContainer?.requestReDraw();
			return;
		}

		if (!section) {
			section = new InteractionModeSection();
			app.sectionContainer.addSection(section);
		}
		section.interactable = !!mode;
		if (!mode) section.context.canvas.style.cursor = '';
		app.sectionContainer.requestReDraw();
	}

	private showThePointer(mode: InteractionMode): void {
		this.context.canvas.style.cursor = mode.pointer();
	}

	onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		const mode = Interaction.modeOn();
		if (!mode) return;
		mode.mouseDown(this.inTheDocument(point), MouseControl.readModifier(e));
		this.containerObject.requestReDraw();
	}

	onMouseMove(
		point: cool.SimplePoint,
		_dragDistance: number[],
		e: MouseEvent,
	): void {
		const mode = Interaction.modeOn();
		if (!mode) return;

		const pressed = (e.buttons & 1) === 1;
		mode.mouseAt(
			this.inTheDocument(point),
			MouseControl.readModifier(e),
			pressed,
		);
		this.showThePointer(mode);
		this.containerObject.requestReDraw();

		// the view scrolls on by itself while a drag stands near an edge of it, or outside
		if (!pressed) return;
		const onCanvas = point.clone();
		onCanvas.pX += this.myTopLeft[0];
		onCanvas.pY += this.myTopLeft[1];
		app.map.fire('handleautoscroll', {
			pos: { x: onCanvas.cX, y: onCanvas.cY },
			map: app.map,
		});
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		const mode = Interaction.modeOn();
		app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
		if (!mode) return;
		mode.mouseUp(
			this.inTheDocument(point),
			MouseControl.readModifier(e),
			this.containerObject.isDraggingSomething(),
		);
		this.containerObject.requestReDraw();
	}

	onMouseLeave(): void {
		Interaction.modeOn()?.mouseLeft();
		this.containerObject.requestReDraw();
	}

	onDragCancel(): void {
		app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
		Interaction.modeOn()?.dragCancelled();
		this.containerObject.requestReDraw();
	}

	onDraw(): void {
		const mode = Interaction.modeOn() ?? InteractionModeSection.lastMode;
		if (!mode) return;

		mode.draw(this);
	}
}
