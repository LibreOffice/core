// @ts-strict-ignore
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

// Below class and CanvasSectionObject class are for managing the canvas layout.
/*
	Potential values are separated with '|'
	All pixels are in core pixels.
 	Supports multi touch with 2 fingers.
 	This class uses only native events. There is: No timer for anything & no longpress for desktop & no doubleclick for touch screen & no wheel for touchscreen etc.

	Propagated events:  All events are propagated between "bound sections".
						If 2 sections overlap but not bound, only top section gets the event.

	lowestPropagatedBoundSection:
			This property applies to bound sections.
			Bound sections are like layers. They overlap entirely.
			Every bound section should share same zIndex unless there is a very good reason for different zIndex values.
			To make a section "bound" to another, one can use "boundToSection" property (section name - string).
			When 2 or more sections are bound, the top section will get the event first.
			If a section handles the event and calls stopPropagating function, the bound sections after that section won't get the event.
			Example scenario:
				Event: drag
				Top section handles event.
				Middle section handles the event and then calls "stopPropagating" function. From this point, top section still handles the event until the end of the event.
				Bottom section doesn't get the event.

			If stopPropagating function is called, it is valid until the end of the event. So for events like dragging, calling
			the function only once is enough. When the event is lasted, lowestPropagatedBoundSection variable is set to null automatically.

	New section's options:
	name: 'tiles' | 'row headers' | 'column headers' | 'markers & cursor' | 'shapes' | 'scroll' etc.
	anchor: 'top left' | 'top right' | 'bottom left' | 'bottom right' (Examples: For row & column headers, anchor will be 'top left'; If we would want to draw something sticked to bottom, it would be 'bottom left' or 'bottom right').
		One can also anchor sections to other sections' edges.
		Order is important, first variables are related to top or bottom, next ones are related to left or right.
		Examples:
		1- [["column header", "bottom", "some section name", "top", "top"], "left"]
			^ If "column header" section exists, its bottom will be section's top
			If "some section name" exists, its top will be section's top.
			If none of them exists, canvas's top will be used as anchor.
			Canvas's left will be used as horizontal anchor.
		2- [["column header", "bottom", "ruler", "bottom", "top"], ["row header", "right", "left"]]
		It is also possible to position a section's right edge anchor to another section's left edge by using a special option '-left'.

	position: [0, 0] | [10, 50] | [x, y] // Related to anchor. Example 'bottom right': P(0, 0) is bottom right etc. myTopLeft is updated according to position and anchor.

	documentObject:
		* This means that the section is a document object. So its anchor can only be "top left", it doesn't need to be set.
		* Container's documentAnchorSectionName should be set for enabling document objects.
		* For this type of sections, only "size" and "position" are meaningful.
		* Position is the object's position inside the document.
		* So a document object is not a UI element, other sections are UI elements and their positions and sizes can be managed by the section container.
		* Document objects' positions and sizes are not managed by the section container.
		* When "onDraw" function of a document-object section is called, the canvas pen is positioned to the coordinate of the document object. So the section can draw from point (0, 0)
		* Before "onDraw" function is called, section's "isVisible" property is set. If the object is visible inside the viewed area (even partially), the property value will be true, if not, it will be false.

	windowSection:	Bound sections overlap entirely, their sizes and locations are the same.
					Events are propagated "only" between bound sections.
					When an event occurs, the target section is found and local coordinates are calculated.
					Since the same coordinates are valid for the bound sections beneath the target section, we can easily propagate the event to those bound sections.
					If 2 sections intersect but not bound, the top section gets the event and event is not propagated to the intersecting section.
					This type of propagation reduces the computational load of the CanvasSectionContainer.

					Now, the document is placed inside TilesSection and we need to scroll / zoom the document.
					If we bind the ScrollSection to TilesSection, we can easily scroll the document using event propagation.
					But now we have CommentListSection next to TilesSection. We also want to be able to scroll the document when mouse is above CommentListSection.
					This request needs a top handler to be met.
					Top handlers (sections) will handle events like scroll & zoom universally.
					Users will be able to scroll the document while the mouse pointer is above CommentListSection.
					So the solution to problems like scrolling is using "windowSection"s.

					A window section is always the first to handle an event and its size is equal to canvas element's size.
					An event is propagated for the target section after window section handles it.
					A window section can stop propagation of the event.
					"Window section"s drawing order and zIndex can be set. So it will be drawn accordingly.

					If:
						We don't want to handle the event with the window section while mouse is above "section X".
							Check canvasSectionContainer.targetSection property. It shows the actual target of the event.
								Example:	A document object is being moved and window section shouldn't scroll the document.
											Window section will check the containerObject.targetSection (string) and see it is a document object.
						We are handling keyboard events with a window section so there is no target section?
							Check the containerObject.activeSection property, if it is not included in your list, don't handle the keyboard event with the window section.
							One should set and keep up to date the containerObject.activeSection (string) property accordingly.

	size: [100, 100] | [10, 20] | [maxX, maxY] // This doesn't restrict the drawable area, that is up to implementation. Size is not important for expanded directions. Expandable size is assigned after calculations.
	zIndex: Elements with highest zIndex will be drawn on top.
	expand: '' | 'right' | 'left' | 'top' | 'bottom' | 'left right top bottom' (any combination)
	interactable: true | false // If false, only general events will be fired (onDraw, newDocumentTopLeft, onResize). Example: editing mode, background drawings etc.
	drawingOrder: Sections with the same zIndex value are drawn according to their drawing order values.
		Section with the highest drawing order is drawn on top (for specific zIndex).
		So, in terms of being drawn on top, priority is: zIndex > drawingOrder.
	processingOrder:

	Processing order feature is tricky, let's say you want something like this:

	--------------------
	|     top bar      |
	--------------------
	--------- ----------
	| left  | | tiles  |
	| bar   | | area   |
	--------- ----------

	Top bar's height will be static most probably. It needs to be drawn first, so it can be expanded to right.
	If top bar is processed after tiles area, since "tiles area" will most probably be expanded to all directions, it will leave no space for top bar.
	So, tiles area will be processed last.
	For above situation, processing orders would be (with the same zIndex):
	* top bar -> 1
	* left bar -> 2
	* tiles area -> 3

	And "expand" properties would be like below:
		top bar: 'right' from position (0, 0)
		left bar: 'top bottom' from position (0, 200) -> So it will go up and find where top bar ends, then go down and find where canvas ends.
		tiles area: 'top bottom left right' from position like (300, 300) -> So it won't overlap with resulting positions of others (or it can stuck inside one of them).

	Expandable sections' dimensions are calculated according to other sections with the same zIndex.

	Below events trigger a redraw:
	* Adding a new section.
	* Click.
	* Double click.
	* Renewing all sections (optional redraw).
	* Requesting a redraw.

	Every section has a "section" property inside "sectionProperties".

	Event handling:
		Mouse event combinations:
			mouse down + mouse up + click
			mouse down + mouse up + click + mouse down + mouse up + click + double click
			mouse move (if mouse is down, "draggingSomething" = true)
			mouse down + mouse move (dragging) + mouse up
			mouse wheel
			mouse enter
			mouse leave

		Touch event combinations:
			mouse down + mouse up + click
			mouse down + long press + mouse up
			mouse down + mouse move (it means dragging, "draggingSomething" = true) + mouse up // There is no "mouse move" event without dragging (for touch events)
			mouse down + multi touch start + multi touch move + multi touch end
*/

class CanvasSectionContainer {
	/*
		All events will be cached by this class and propagated to sections.
		This class should work also mouse & touch enabled (at the same time) devices. Users should be able to use both.
	*/

	private sections: Array<CanvasSectionObject> = new Array(0);
	private sectionsByName: Map<string, CanvasSectionObject> = new Map();
	private canvas: HTMLCanvasElement;
	private context: CanvasRenderingContext2D;
	private width: number;
	private height: number;
	private needsResize: boolean = false;
	private positionOnMouseDown: Array<number> = null;
	private positionOnMouseUp: Array<number> = null;
	private positionOnClick: Array<number> = null;
	private positionOnDoubleClick: Array<number> = null;
	private mousePosition: Array<number> = null;
	private dragDistance: Array<number> = null;
	private draggingSomething: boolean = false; // This will be managed by container, used by sections.
	private sectionOnMouseDown: string = null; // (Will contain section name) When dragging, user can leave section borders, dragging will continue. Target section will be informed.
	private sectionUnderMouse: string = null; // For mouse enter & leave events.
	private draggingTolerance: number = 5; // This is for only desktop, mobile browsers seem to distinguish dragging and clicking nicely.
	private multiTouch: boolean = false;
	private touchCenter: Array<number> = null;
	private longPressTimer: any = null;
	private clearColor: string = '#f8f9fa';
	private documentBackgroundColor = '#ffffff'; // This is the background color of the document
	private useCSSForBackgroundColor = true;
	private touchEventInProgress: boolean = false; // This prevents multiple calling of mouse down and up events.
	private activePointerId: number | null = null;
	public testing: boolean = false; // If this set to true, container will create a div element for every section. So, cypress tests can find where to click etc.
	public lowestPropagatedBoundSection: string = null; // Event propagating to bound sections. The first section which stops propagating and the sections those are on top of that section, get the event.
	public targetSection: string = null;
	public activeSection: string = null;
	private scrollLineHeight: number = 30; // This will be overridden.
	private mouseIsInside: boolean = false;
	private inZoomAnimation: boolean = false;
	private postZoomReplay: boolean = false;
	private zoomChanged: boolean = false;
	private documentAnchorSectionName: string = null; // This section's top left point declares the point where document starts.
	private documentAnchor: Array<number> = null; // This is the point where document starts inside canvas element. Initial value shouldn't be [0, 0].
	// Above 2 properties can be used with documentBounds.
	private drawRequest: number = null;
	private drawingPaused: number = 0;
	private drawingEnabled: boolean = true;
	private deferredDrawCallback: () => void = null;
	private sectionsDirty: boolean = false;
	private framesRendered: number = 0; // Total frame count for debugging

	// For window sections.
	private windowSectionList: Array<CanvasSectionObject> = [];

	// Below variables are related to animation feature.
	private animatingSectionName: string = null; // The section that called startAnimating function. This variable is null when animations are not running.
	private lastFrameStamp: number = null;
	private continueAnimating: boolean = null;
	private frameCount: number = null; // Frame count of the current animation.
	private duration: number = null; // Duration for the animation.
	private elapsedTime: number = null; // Time that passed since the animation started.

	private dpiMediaQuery: MediaQueryList = null;

	constructor (canvasDOMElement: HTMLCanvasElement, disableDrawing?: boolean) {
		this.canvas = canvasDOMElement;
		this.context = canvasDOMElement.getContext('2d', { alpha: false });
		this.context.setTransform(1,0,0,1,0,0);
		document.addEventListener('mousemove', this.onMouseMove.bind(this));
		this.canvas.onmousedown = this.onMouseDown.bind(this);
		document.addEventListener('mouseup', this.onMouseUp.bind(this));
		this.canvas.onclick = this.onClick.bind(this);
		this.canvas.ondblclick = this.onDoubleClick.bind(this);
		this.canvas.oncontextmenu = this.onContextMenu.bind(this);
		this.canvas.onwheel = this.onMouseWheel.bind(this);
		this.canvas.onmouseleave = this.onMouseLeave.bind(this);
		this.canvas.onmouseenter = this.onMouseEnter.bind(this);
		this.canvas.addEventListener('pointerdown', (e: PointerEvent) => {
			if (e.button === 0 && e.isPrimary) this.activePointerId = e.pointerId;
		});
		this.canvas.addEventListener('pointerup', () => { this.activePointerId = null; });
		this.canvas.addEventListener('pointercancel', () => { this.activePointerId = null; });
		this.canvas.ontouchstart = this.onTouchStart.bind(this);
		this.canvas.ontouchmove = this.onTouchMove.bind(this);
		this.canvas.ontouchend = this.onTouchEnd.bind(this);
		this.canvas.ontouchcancel = this.onTouchCancel.bind(this);
		this.canvas.ondrop = this.onDrop.bind(this);
		this.canvas.ondragover = this.onDragOver.bind(this);
		window.addEventListener('blur', this.onWindowBlur.bind(this));

		// Some explanation first.
		// When the user uses the mouse wheel for scrolling, different browsers use different technics for calculating the deltaY and deltaX values.
		// For example FireFox uses "deltaMode=1" which corresponds to "lines". So it creates the event with the number of lines to scroll.
		// Chrome uses "deltaMode=0" which corresponds to "pixels". So it creates the event with the number of pixels to scroll.
		// When "deltaMode=1" is used, we need to know the height of the line, so we will convert it to pixels.
		// For that purpose, we'll create a temporary div element, get the font size and delete the temporary element.
		const tempElement = document.createElement('div');
		tempElement.style.fontSize = 'initial'; // IE doesn't support this property, but it uses "deltaMode=0" (so we don't need to get the line height).
		tempElement.style.display = 'none';
		document.body.appendChild(tempElement);
		this.scrollLineHeight = parseInt(window.getComputedStyle(tempElement).fontSize);
		document.body.removeChild(tempElement); // Remove the temporary element.

		this.clearColor = window.getComputedStyle(document.documentElement).getPropertyValue('--color-canvas');
		// Set document background color to the app background color for now until we get the real color from the kit
		// through a LOK_CALLBACK_DOCUMENT_BACKGROUND_COLOR
		this.documentBackgroundColor = window.getComputedStyle(document.documentElement).getPropertyValue('--color-background-document');

		// If CanvasTileLayer.js:this._layer.isCalc()
		if (disableDrawing) {
			// On Calc cells have no color, so let's set canvas color with document bg
			this.clearColor = this.documentBackgroundColor;

			this.disableDrawing();
		}

		this.setupDPIChangeListener();
	}

	private setupDPIChangeListener(): void {
		if (!window.matchMedia) return;

		const updateDPI = (): void => {
			if (window.devicePixelRatio !== app.dpiScale) {
				this.onResize(0, 0);
			}
			// Re-register since the query is for a specific dppx value
			this.setupDPIChangeListener();
		};
		if (this.dpiMediaQuery) {
			this.dpiMediaQuery.removeEventListener('change', updateDPI);
		}
		this.dpiMediaQuery = window.matchMedia(
			'(resolution: ' + window.devicePixelRatio + 'dppx)'
		);
		this.dpiMediaQuery.addEventListener('change', updateDPI, { once: true });
	}

	private clearCanvas() {
		this.context.fillStyle = this.clearColor;
		this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
		this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
	}

	public getContext () {
		return this.context;
	}

	public setDocumentAnchorSection(sectionName: string) {
		var section: CanvasSectionObject = this.getSectionWithName(sectionName);
		if (section) {
			this.documentAnchorSectionName = sectionName;
		}
		else {
			this.documentAnchorSectionName = null;
			this.documentAnchor = null;

			if (app.activeDocument?.activeLayout)
				app.activeDocument.activeLayout.documentAnchorPosition = [0, 0];
		}
	}

	public getDocumentAnchorSection (): CanvasSectionObject {
		return this.getSectionWithName(this.documentAnchorSectionName);
	}

	public getViewSize (): Array<number> {
		return [this.width, this.height];
	}

	public setBackgroundColorMode(useCSSVars: boolean = true) {
		this.useCSSForBackgroundColor = useCSSVars;
	}

	public setClearColor (color: string) {
		if (!this.useCSSForBackgroundColor)
			this.clearColor = color;
	}

	public getClearColor () {
		return this.clearColor;
	}

	public setDocumentBackgroundColor (color: string) {
		this.documentBackgroundColor = color;
	}

	public getDocumentBackgroundColor () {
		return this.documentBackgroundColor;
	}

	public getCanvasStyle(): CSSStyleDeclaration {
		return this.canvas.style;
	}

	public setInZoomAnimation (inZoomAnimation: boolean) {
		this.inZoomAnimation = inZoomAnimation;
	}

	public isInZoomAnimation (): boolean {
		return this.inZoomAnimation;
	}

	public setPostZoomReplay (postZoomReplay: boolean) {
		this.postZoomReplay = postZoomReplay;
	}

	public isPostZoomReplay (): boolean {
		return this.postZoomReplay;
	}

	public setZoomChanged (zoomChanged: boolean) {
		this.zoomChanged = zoomChanged;
	}

	public isZoomChanged (): boolean {
		return this.zoomChanged;
	}

	public drawingAllowed (): boolean {
		return this.drawingEnabled && this.drawingPaused <= 0;
	}

	// This is used for making sure rendering does not happen for multiple runs of
	// Socket._emitSlurpedEvents(). Currently this is used in Calc to disable rendering
	// from docload till we get tiles of the correct view area to render.
	// After calling this, only enableDrawing() can undo this call.
	private disableDrawing () {
		if (this.drawRequest) {
			cancelAnimationFrame(this.drawRequest);
			this.drawRequest = null;
		}
		this.drawingEnabled = false;
	}

	public enableDrawing () {
		if (this.drawingEnabled)
			return;

		this.drawingEnabled = true;
		if (this.drawingPaused === 0) {
			// Trigger a forced repaint as drawing is not paused currently.
			this.paintOnResumeOrEnable();
		}
	}

	public pauseDrawing () {
		if (this.drawRequest) {
			cancelAnimationFrame(this.drawRequest);
			this.drawRequest = null;
		}
		this.drawingPaused++;
	}

	// set topLevel if we are sure that we are the top of call nesting
	// eg. in a browser event handler. Avoids JS exceptions poisoning
	// the count, since we have no RAII helpers here.
	public resumeDrawing(topLevel?: boolean) {
		var wasNonZero: boolean = this.drawingPaused !== 0;
		if (topLevel)
		   this.drawingPaused = 0;
		else if (this.drawingPaused > 0)  // ensure non-negative value.
		   this.drawingPaused--;

		if (this.drawingEnabled && wasNonZero && this.drawingPaused === 0) {
			this.paintOnResumeOrEnable();
		} else {
			// This is unusual, make it visible in the logs
			let log = 'Skipped paint on resume.';
			log += '\nDrawing Enabled: ' + (this.drawingEnabled ? 'YES' : 'NO');
			log += '\nDrawing Paused: ' + (this.drawingPaused > 0 ? 'YES' : 'NO');
			app.console.debug(log);
		}
	}

	// Drawing requests will call this callback instead of queueing a redraw. Set the
	// callback to null to resume the standard drawing chain.
	public deferDrawing (callback: () => void) {
		this.deferredDrawCallback = callback;
	}

	private paintOnResumeOrEnable() {
		if (this.sectionsDirty) {
			this.updateBoundSectionLists();
			this.reNewAllSections(false);
			this.sectionsDirty = false;
		}

		var scrollSection = <any> this.getSectionWithName(app.CSections.Scroll.name);
		if (scrollSection)
			scrollSection.completePendingScroll(); // No painting, only dirtying.

		this.requestReDraw();
	}

	private clearMousePositions () {
		this.positionOnClick = this.positionOnDoubleClick = this.positionOnMouseDown = this.positionOnMouseUp = this.dragDistance = this.sectionOnMouseDown = null;
		this.touchCenter = null;
		this.draggingSomething = false;
		this.touchEventInProgress = false;
		this.lowestPropagatedBoundSection = null;
		this.targetSection = null;
	}

	private convertPositionToSectionLocale (section: CanvasSectionObject, point: Array<number>): Array<number> {
		return [point[0] - section.myTopLeft[0], point[1] - section.myTopLeft[1]];
	}

	private convertPositionToCanvasLocale (e: any): Array<number> {
		var rect: any = this.canvas.getBoundingClientRect();
		var x: number, y: number;

		if (e.touches !== undefined && e.touches.length > 0) {
			x = e.touches[0].clientX - rect.left;
			y = e.touches[0].clientY - rect.top;
		}
		else if (e.changedTouches !== undefined && e.changedTouches.length > 0) {
			x = e.changedTouches[0].clientX - rect.left;
			y = e.changedTouches[0].clientY - rect.top;
		}
		else {
			x = e.clientX - rect.left;
			y = e.clientY - rect.top;
		}
		return [Math.round(x * app.dpiScale), Math.round(y * app.dpiScale)];
	}

	private convertPointToCanvasLocale (point: Array<number>): Array<number> {
		var rect: any = this.canvas.getBoundingClientRect();
		var x: number, y: number;

		x = point[0] - rect.left;
		y = point[1] - rect.top;

		return [Math.round(x * app.dpiScale), Math.round(y * app.dpiScale)];
	}

	public getSectionWithName (name: string): CanvasSectionObject {
		if (name) {
			var section: CanvasSectionObject = this.sectionsByName.get(name);
			if (section) {
				return section;
			}
			return null;
		}
		else {
			return null;
		}
	}

	public getDocumentAnchor(): Array<number> {
		return [this.documentAnchor[0], this.documentAnchor[1]];
	}

	public getCanvasBoundingClientRect(): DOMRect {
		return this.canvas.getBoundingClientRect();
	}

	public getWidth(): number {
		return this.width;
	}

	public getHeight(): number {
		return this.height;
	}

	public isDraggingSomething(): boolean {
		return this.draggingSomething;
	}

	// Capture/release can throw if the pointer is gone or was never captured;
	// we ignore that and carry on.
	public capturePointerForDrag(): void {
		if (this.activePointerId !== null) {
			try { this.canvas.setPointerCapture(this.activePointerId); } catch (_) { /* ignore */ }
		}
	}

	public releasePointerForDrag(): void {
		if (this.activePointerId !== null) {
			try { this.canvas.releasePointerCapture(this.activePointerId); } catch (_) { /* ignore */ }
		}
	}

	public getDragDistance(): number[] {
		return [...this.dragDistance];
	}

	public getPositionOnMouseUp(): number[] {
		return [...this.positionOnMouseUp];
	}

	public isDocumentObjectVisible (section: CanvasSectionObject): boolean {
		return section.isAlwaysVisible ||
			app.isRectangleVisibleInTheDisplayedArea(
			[
				Math.round(section.position[0] * app.pixelsToTwips),
				Math.round(section.position[1] * app.pixelsToTwips),
				Math.round(section.size[0] * app.pixelsToTwips),
				Math.round(section.size[1] * app.pixelsToTwips)
			]
		);
	}

	// For window sections, there is a "targetSection" property in CanvasSectionContainer.
	// Because a window section is above all sections and cover entire canvas, it may need to act according to the actual target of the event.
	// In this case, "targetSection" property gives the window section the first target of the event.
	// This (below) function gives the window section if a section will sooner or later get the event.
	// But this function cannot know if the event will be stopped by a prior section before the event reaches the section specified with the "sectionName" variable.
	// This function doesn't neither check the "interactable" property of the section in question ("sectionName"). Though that check can be added here, as an optional one.
	public targetBoundSectionListContains (sectionName: string): boolean {
		if (!this.targetSection)
			return false;
		else {
			var section: CanvasSectionObject = this.getSectionWithName(this.targetSection);
			if (section && section.boundsList) {
				for (var i: number = 0; i < section.boundsList.length; i++) {
					if (section.boundsList[i].name === sectionName)
						return true;
				}
				return false;
			}
			else
				return false;
		}
	}

	private updateBoundSectionList(section: CanvasSectionObject, sectionList: Array<CanvasSectionObject> = null): Array<CanvasSectionObject> {
		if (sectionList === null)
			sectionList = new Array(0);

		sectionList.push(section);

		var tempSectionList: Array<CanvasSectionObject> = new Array(0);

		if (section.boundToSection) {
			var tempSection = this.getSectionWithName(section.boundToSection);
			if (tempSection && tempSection.isLocated) {
				if (!sectionList.includes(tempSection))
					tempSectionList.push(tempSection);
			}
		}

		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].isLocated && this.sections[i].boundToSection === section.name) {
				if (!sectionList.includes(this.sections[i]))
					tempSectionList.push(this.sections[i]);
			}
		}

		for (var i: number = 0; i < tempSectionList.length; i++) {
			this.updateBoundSectionList(tempSectionList[i], sectionList);
		}

		return sectionList;
	}

	private orderBoundsList(section: CanvasSectionObject) {
		// According to zIndex & drawingOrder.
		for (var i: number = 0; i < section.boundsList.length - 1; i++) {
			for (var j = i + 1; j < section.boundsList.length; j++) {
				if (section.boundsList[i].zIndex > section.boundsList[j].zIndex
					|| (section.boundsList[i].zIndex === section.boundsList[j].zIndex && section.boundsList[i].drawingOrder > section.boundsList[j].drawingOrder)) {
					var temp = section.boundsList[i];
					section.boundsList[i] = section.boundsList[j];
					section.boundsList[j] = temp;
				}
			}
		}

		// Remove the sections those are above this section. Events will not be propagated to them.
		for (var i: number = section.boundsList.length - 1; i > -1; i--) {
			if (section.boundsList[i].name !== section.name) {
				section.boundsList.splice(i, 1);
			}
			else {
				break;
			}
		}
	}

	// Should only be called by CanvasSectionObject member functions!
	// HACK: Typescript does not have the concept of friend classes hence this has to be public.
	public updateBoundSectionLists() {
		for (var i: number = 0; i < this.sections.length; i++) {
			this.sections[i].boundsList = null;
			this.sections[i].boundsList = this.updateBoundSectionList(this.sections[i]);
			this.orderBoundsList(this.sections[i]);
		}
	}

	private flushLayoutingTasks() {
		const layoutingService = app.layoutingService;
		if (layoutingService.hasTasksPending())
			layoutingService.cancelFrame();

		while (layoutingService.runTheTopTask());

		// Trigger drain callbacks since we bypassed the normal async flow
		layoutingService.triggerDrainCallbacks();
	}

	private isCanvasSizeValidAfterDisplayChange(): boolean {
		/*
			In some situations, we receive resize events that cause the canvas to size to 0x0 and we don't receive a subsequent
			resize event when the size is restored. As a work-around, if we have a zero-sized canvas,
			double-check the document container size to see if we've missed a resize event
		*/
		if (this.width === 0 || this.height === 0) {
			app.console.warn('Canvas width or height is zero.');
			const documentContainer = document.getElementById('document-container');
			if (documentContainer && documentContainer.clientWidth !== 0 || documentContainer.clientHeight !== 0) {
				if (app.map._docLayer) {
					app.map._docLayer._syncTileContainerSize(true);
					app.activeDocument.activeLayout.sendClientVisibleArea();
					this.requestReDraw();
					return false;
				}
			}
		}

		return true;
	}

	private resizeCanvas() {
		if (!this.needsResize)
			return;

		this.needsResize = false;
		this.canvas.width = this.width;
		this.canvas.height = this.height;

		// CSS pixels can be fractional, but need to round to the same real pixels
		var cssWidth: number = this.width / app.dpiScale; // NB. beware
		var cssHeight: number = this.height / app.dpiScale;
		this.canvas.style.width = cssWidth.toFixed(4) + 'px';
		this.canvas.style.height = cssHeight.toFixed(4) + 'px';

		// Avoid black default background.
		this.clearCanvas();
	}

	private redrawCallback(timestamp: number) {
		app.enterRAF();

		this.drawRequest = null;

		if (!this.isCanvasSizeValidAfterDisplayChange()) {
			app.exitRAF();
			return;
		}

		this.flushLayoutingTasks();
		this.resizeCanvas();
		this.drawSections();
		this.canvas.style.visibility = 'unset';

		// need to check if we should continue animation
		this.animate(timestamp);

		app.exitRAF();
	}

	public requestReDraw() {
		if (!this.drawingAllowed()) return;
		if (this.deferredDrawCallback) {
			this.deferredDrawCallback();
			return;
		}
		if (this.drawRequest === null)
			this.drawRequest = requestAnimationFrame(this.redrawCallback.bind(this));
	}

	private propagateCursorPositionChanged() {
		// There is no target section for cursor position change, this event comes from core side.
		// Sections with interactable = false will not be targeted.
		// Window sections can still stop the propagation of the event. Others cannot.

		var propagate: boolean = true;

		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onCursorPositionChanged(app.file.textCursor.rectangle.clone() as cool.SimpleRectangle);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = this.sections.length - 1; i > -1; i--) {
				if (this.sections[i].interactable)
					this.sections[i].onCursorPositionChanged(app.file.textCursor.rectangle.clone() as cool.SimpleRectangle);
			}
		}
	}

	private propagateCellAddressChanged() {
		// There is no target section for cell address change, this event comes from core side.
		// Sections with interactable = false will not be targeted.
		// Window sections can still stop the propagation of the event. Others cannot.

		var propagate: boolean = true;

		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			windowSection.onCellAddressChanged();

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = this.sections.length - 1; i > -1; i--) {
				this.sections[i].onCellAddressChanged();
			}
		}
	}

	private propagateOnClick(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onClick(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onClick((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnDoubleClick(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onDoubleClick(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onDoubleClick(position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]) : null, e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMouseLeave(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		if (!windowPosition) { // This event is valid only if the windowPosition is null for window sections. Otherwise mouse cannot leave from a section that is covering entire canvas element.
			for (var j: number = 0; j < this.windowSectionList.length; j++) {
				var windowSection = this.windowSectionList[j];
				if (windowSection.interactable)
					windowSection.onMouseLeave(windowPosition, e);
				// This event's propagation shouldn't be cancelled.
			}
		}

		for (var i: number = section.boundsList.length - 1; i > -1; i--) {
			if (section.boundsList[i].interactable)
				section.boundsList[i].onMouseLeave((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

			if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
				break; // Stop propagation.
		}
	}

	private propagateOnMouseEnter(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		// This event is handled in the mouseEnter event of the canvas itself (for window sections).

		for (var i: number = section.boundsList.length - 1; i > -1; i--) {
			if (section.boundsList[i].interactable)
				section.boundsList[i].onMouseEnter((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

			if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
				break; // Stop propagation.
		}
	}

	private propagateOnMouseMove(section: CanvasSectionObject, position: Array<number>, dragDistance: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMouseMove(windowPosition, dragDistance, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMouseMove((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), dragDistance, e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}

		this.lowestPropagatedBoundSection = null; // onMouseMove event doesn't clear the mouse positions, so we need to clear the property here.
	}

	private propagateOnMouseDown(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMouseDown(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMouseDown((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMouseUp(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMouseUp(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMouseUp((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnContextMenu(section: CanvasSectionObject, position: Array<number>, e: MouseEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		const windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onContextMenu(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onContextMenu(cool.SimplePoint.fromCorePixels([position[0], position[1]]), e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMouseWheel(section: CanvasSectionObject, position: Array<number>, delta: Array<number>, e: WheelEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMouseWheel(windowPosition, delta, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMouseWheel((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), delta, e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMultiTouchStart(section: CanvasSectionObject, e: TouchEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMultiTouchStart(e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMultiTouchStart(e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMultiTouchMove(section: CanvasSectionObject, position: Array<number>, distance: number, e: TouchEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		var windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMultiTouchMove(windowPosition, distance, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMultiTouchMove((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), distance, e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnMultiTouchEnd(section: CanvasSectionObject, e: TouchEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onMultiTouchEnd(e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onMultiTouchEnd(e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	private propagateOnDrop(section: CanvasSectionObject, position: number[], e: DragEvent) {
		this.targetSection = section.name;

		var propagate: boolean = true;
		const windowPosition: cool.SimplePoint = position ? cool.SimplePoint.fromCorePixels([position[0] + section.myTopLeft[0], position[1] + section.myTopLeft[1]]): null;
		for (var j: number = 0; j < this.windowSectionList.length; j++) {
			var windowSection = this.windowSectionList[j];
			if (windowSection.interactable)
				windowSection.onDrop(windowPosition, e);

			if (this.lowestPropagatedBoundSection === windowSection.name)
				propagate = false; // Window sections can not stop the propagation of the event for other window sections.
		}

		if (propagate) {
			for (var i: number = section.boundsList.length - 1; i > -1; i--) {
				if (section.boundsList[i].interactable)
					section.boundsList[i].onDrop((position ? cool.SimplePoint.fromCorePixels([position[0], position[1]]): null), e);

				if (section.boundsList[i].name === this.lowestPropagatedBoundSection)
					break; // Stop propagation.
			}
		}
	}

	/*
		Cursor position is sent from the core side.
		The event is handled in CanvasTileLayer file.
		CanvasTileLayer then informs CanvasSectionContainer.
		This is for Writer. Other apps may need other functions. To be checked..
	*/
	public onCursorPositionChanged() {
		if (app.map._docLayer._docType === 'text') {
			// Global state holder should already have the latest information: app.file.textCursor.rectangle.
			this.propagateCursorPositionChanged();
		}
	}

	private getMyTopLeftForDocumentObject(section: CanvasSectionObject): number[] {
		if (this.documentAnchor === null) return;

		if (app.map._docLayer._docType === 'spreadsheet') {
			return [
				this.documentAnchor[0] +
					section.position[0] -
					(app.isXOrdinateInFrozenPane(section.position[0])
						? 0
						: app.activeDocument.activeLayout.viewedRectangle.pX1),
				this.documentAnchor[1] +
					section.position[1] -
					(app.isYOrdinateInFrozenPane(section.position[1])
						? 0
						: app.activeDocument.activeLayout.viewedRectangle.pY1),
			];
		}
		else
			return [section.documentPosition.vX, section.documentPosition.vY];
	}

	// Called when document position is changed.
	public onNewDocumentTopLeft() {
		for (var i: number = 0; i < this.sections.length; i++) {
			var section: CanvasSectionObject = this.sections[i];

			if (section.documentObject === true) {
				section.myTopLeft = this.getMyTopLeftForDocumentObject(section);

				const isVisible = this.isDocumentObjectVisible(section);
				if (isVisible !== section.isVisible) {
					section.isVisible = isVisible;
					section.onDocumentObjectVisibilityChange();
				}

				if (this.testing) this.createUpdateSingleDivElement(section);
			}

			this.sections[i].onNewDocumentTopLeft();
		}
	}

	/*
		Cell position is sent from the core side.
		The event is handled in CanvasTileLayer file.
		This is for Calc.
	*/
	public onCellAddressChanged() {
		if (app.map._docLayer._docType === 'spreadsheet') {
			// We should have set the variable to latest state already.
			this.propagateCellAddressChanged();
		}
	}

	public onClick (e: MouseEvent) {
		if (this.touchEventInProgress) {
			Object.defineProperty(e, 'pointerType', {
				value: 'touch',
				writable: false
			});
		}

		if (!this.draggingSomething) { // Prevent click event after dragging.
			if (this.positionOnMouseDown !== null && this.positionOnMouseUp !== null) {
				this.positionOnClick = this.convertPositionToCanvasLocale(e);
				var s1 = this.findSectionContainingPoint(this.positionOnMouseDown);
				var s2 = this.findSectionContainingPoint(this.positionOnMouseUp);
				if (s1 && s2 && s1 == s2) { // Allow click event if only mouse was above same section while clicking.
					var section: CanvasSectionObject = this.findSectionContainingPoint(this.positionOnClick);
					if (section) { // "interactable" property is checked while propagating the event.
						this.propagateOnClick(section, this.convertPositionToSectionLocale(section, this.positionOnClick), e);
					}
				}
			}
			this.requestReDraw();
		}
		this.clearMousePositions();
	}

	public onDoubleClick (e: MouseEvent) {
		this.positionOnDoubleClick = this.convertPositionToCanvasLocale(e);

		var section: CanvasSectionObject = this.findSectionContainingPoint(this.positionOnDoubleClick);
		if (section) {
			this.propagateOnDoubleClick(section, this.convertPositionToSectionLocale(section, this.positionOnDoubleClick), e);
		}
		this.clearMousePositions();
		this.requestReDraw();
	}

	private isLongPressActive() {
		return !!this.longPressTimer;
	}

	private startLongPress(e: TouchEvent) {
		this.stopLongPress();
		this.longPressTimer = setTimeout(function() {
			(e as any).clientX = e.touches[0].clientX;
			(e as any).clientY = e.touches[0].clientY;
			this.onMouseMove(e);
		}.bind(this), 550);
	}

	private stopLongPress() {
		if (this.longPressTimer) {
			clearTimeout(this.longPressTimer);
			this.longPressTimer = null;
		}
	}

	private onMouseMove (e: MouseEvent) {
		// Early exit. If mouse is outside and "draggingSomething = false", then there is no reason to check further.
		// Add an exception for leaflet, check after removing it.
		if (!this.mouseIsInside && !this.draggingSomething && !this.isLongPressActive())
			return;

		if (!this.isLongPressActive()) {
			if (!this.touchEventInProgress) {
				this.mousePosition = this.convertPositionToCanvasLocale(e);
				if (this.positionOnMouseDown !== null && !this.draggingSomething) {
					var dragDistance = [this.mousePosition[0] - this.positionOnMouseDown[0], this.mousePosition[1] - this.positionOnMouseDown[1]];
					if (Math.abs(dragDistance[0]) >= this.draggingTolerance || Math.abs(dragDistance[1]) >= this.draggingTolerance) {
						this.draggingSomething = true;
					}
				}

				var section: CanvasSectionObject;

				if (this.draggingSomething) {
					this.dragDistance = [this.mousePosition[0] - this.positionOnMouseDown[0], this.mousePosition[1] - this.positionOnMouseDown[1]];
					section = this.getSectionWithName(this.sectionOnMouseDown);
				}
				else {
					section = this.findSectionContainingPoint(this.mousePosition);
				}

				if (section && section.boundsList) {
					if (section.name !== this.sectionUnderMouse) {
						if (this.sectionUnderMouse !== null) {
							var previousSection: CanvasSectionObject = this.getSectionWithName(this.sectionUnderMouse);
							if (previousSection)
								this.propagateOnMouseLeave(previousSection, this.convertPositionToSectionLocale(previousSection, this.mousePosition), e);
						}
						this.sectionUnderMouse = section.name;
						this.propagateOnMouseEnter(section, this.convertPositionToSectionLocale(section, this.mousePosition), e);
					}
					this.propagateOnMouseMove(section, this.convertPositionToSectionLocale(section, this.mousePosition), this.dragDistance, e);
				}
				else if (this.sectionUnderMouse !== null) {
					var previousSection: CanvasSectionObject = this.getSectionWithName(this.sectionUnderMouse);
					if (previousSection)
						this.propagateOnMouseLeave(previousSection, this.convertPositionToSectionLocale(previousSection, this.mousePosition), e);
					this.sectionUnderMouse = null;
				}
			}
		}
		else {
			this.mousePosition = this.convertPositionToCanvasLocale(e);
			var section: CanvasSectionObject = this.findSectionContainingPoint(this.mousePosition);
			if (section) {
				this.stopLongPress();
				this.propagateOnContextMenu(section, this.convertPositionToSectionLocale(section, this.mousePosition), e);
			}
		}
	}

	public onMouseDown (e: MouseEvent) { // Ignore this event, just rely on this.draggingSomething variable.
		if (e.button === 0 && !this.touchEventInProgress && this.mouseIsInside ) { // So, we only handle left button (and only when mouse is inside).
			this.clearMousePositions();
			this.positionOnMouseDown = this.convertPositionToCanvasLocale(e);

			var section: CanvasSectionObject = this.findSectionContainingPoint(this.positionOnMouseDown);
			if (section) {
				this.sectionOnMouseDown = section.name;
				this.propagateOnMouseDown(section, this.convertPositionToSectionLocale(section, this.positionOnMouseDown), e);
			}
		}
	}

	private onMouseUp (e: MouseEvent) { // Should be ignored unless this.draggingSomething = true.
		// Early exit. If mouse down position is not inside the canvas area, we have nothing to check further.
		if (!this.positionOnMouseDown) {
			this.clearMousePositions();
			return;
		}

		if (e.button === 0 && !this.touchEventInProgress) {
			this.positionOnMouseUp = this.convertPositionToCanvasLocale(e);
			var section: CanvasSectionObject;
			if (!this.draggingSomething) {
				section = this.findSectionContainingPoint(this.positionOnMouseUp);
			}
			else {
				section = this.getSectionWithName(this.sectionOnMouseDown);
			}
			if (section) {
				this.propagateOnMouseUp(section, this.convertPositionToSectionLocale(section, this.positionOnMouseUp), e);
			}
		}

		if (!this.mouseIsInside) { // Normally, onclick event clears the positions. In this case, onClick won't be fired. So we clear the positions.
			this.clearMousePositions();
		}
	}

	public onContextMenu (e: MouseEvent) {
		var mousePosition = this.convertPositionToCanvasLocale(e);
		var section: CanvasSectionObject = this.findSectionContainingPoint(mousePosition);
		if (section) {
			this.propagateOnContextMenu(section, this.convertPositionToSectionLocale(section, mousePosition), e);
		}
		if (this.isLongPressActive()) {
			// LongPress triggers context menu.
			// We should stop propagating here because we are using different context menu handlers for touch and mouse events.
			// By stopping this event here, we can have real context menus (for mice) and other handlers (for longpress) at the same time (see Control.RowHeader.js).
			e.preventDefault();
			e.stopPropagation();
			return false;
		}
	}

	public extendAnimationDuration(extendMs: number) {
		if (this.getAnimatingSectionName()) { // Is animating.
			this.duration += extendMs;
		}
	}

	public getAnimationDuration(): number {
		return this.duration;
	}

	public getScrollLineHeight() {
		return this.scrollLineHeight;
	}

	public onMouseWheel (e: WheelEvent) {
		var point = this.convertPositionToCanvasLocale(e);
		var delta: Array<number>;

		if (e.deltaMode === 1)
			delta = [e.deltaX * this.scrollLineHeight, e.deltaY * this.scrollLineHeight];
		else
			delta = [e.deltaX, e.deltaY];

		var section: CanvasSectionObject = this.findSectionContainingPoint(point);
		if (section)
			this.propagateOnMouseWheel(section, this.convertPositionToSectionLocale(section, point), delta, e);

		app.idleHandler.notifyActive();
	}

	public onMouseLeave (e: MouseEvent) {
		// While dragging something, we don't clear the event information even if the mouse is outside of the canvas area.
		// We catch the mouse move and mouse up events even when the mouse pointer is outside the canvas area.
		// This feature is enabled to create a better dragging experience.
		if (!this.draggingSomething) {
			if (this.sectionUnderMouse !== null) {
				var section: CanvasSectionObject = this.getSectionWithName(this.sectionUnderMouse);
				if (section)
					this.propagateOnMouseLeave(section, null, e);
				this.sectionUnderMouse = null;
			}
			this.clearMousePositions();
			this.mousePosition = null;
		}
		this.mouseIsInside = false;
	}

	public onMouseEnter (e: MouseEvent) {
		this.mouseIsInside = true;

		for (var i: number = 0; i < this.windowSectionList.length; i++) {
			var windowSection = this.windowSectionList[i];
			if (windowSection.interactable)
				windowSection.onMouseEnter(null, e);
		}
	}

	// When the browser window/tab loses focus during a drag, we never receive the mouseup event.
	// This leaves draggingSomething=true and sections (e.g. MouseControl) never send 'buttonup' to the core.
	// This may cause a stuck selection in some cases.
	private onWindowBlur () {
		if (!this.draggingSomething)
			return;

		// Propagate a synthetic mouseUp to the section that received the original
		// mouseDown so it can clean up (e.g. MouseControl sends 'buttonup' to core).
		if (this.sectionOnMouseDown) {
			var section: CanvasSectionObject = this.getSectionWithName(this.sectionOnMouseDown);
			if (section) {
				var position = this.positionOnMouseUp || this.mousePosition || this.positionOnMouseDown;
				// Create a synthetic MouseEvent so section handlers can safely access
				// event properties (e.g. stopPropagation, modifiers).
				var syntheticEvent = new MouseEvent('mouseup', { button: 0, buttons: 0 });
				this.propagateOnMouseUp(section, this.convertPositionToSectionLocale(section, position), syntheticEvent);
			}
		}

		this.clearMousePositions();
		this.mousePosition = null;
		this.mouseIsInside = false;
	}

	public onTouchStart (e: TouchEvent) { // Should be ignored unless this.draggingSomething = true.
		if (e.touches.length === 1) {
			this.clearMousePositions();
			this.startLongPress(e);
			this.positionOnMouseDown = this.convertPositionToCanvasLocale(e);

			var section: CanvasSectionObject = this.findSectionContainingPoint(this.positionOnMouseDown);
			if (section) {
				this.sectionOnMouseDown = section.name;
				this.propagateOnMouseDown(section, this.convertPositionToSectionLocale(section, this.positionOnMouseDown), (<MouseEvent><any>e));
			}
		}
		else if (!this.multiTouch) {
			this.stopLongPress();
			this.multiTouch = true;
			var section: CanvasSectionObject = this.getSectionWithName(this.sectionOnMouseDown);
			if (section)
				this.propagateOnMultiTouchStart(section, e);
		}
	}

	public onTouchMove (e: TouchEvent) {
		// Sometimes onTouchStart is fired for another element. In this case, we return.
		if (this.positionOnMouseDown === null)
			return;

		this.stopLongPress();
		if (!this.multiTouch) {
			this.mousePosition = this.convertPositionToCanvasLocale(e);

			this.dragDistance = [this.mousePosition[0] - this.positionOnMouseDown[0], this.mousePosition[1] - this.positionOnMouseDown[1]];

			if (this.dragDistance[0] ** 2 + this.dragDistance[1] ** 2 > 0.1)
				this.draggingSomething = true;

			var section: CanvasSectionObject = this.getSectionWithName(this.sectionOnMouseDown);
			if (section) {
				this.propagateOnMouseMove(section, this.convertPositionToSectionLocale(section, this.mousePosition), this.dragDistance, <MouseEvent><any>e);
			}
		}
		else if (e.touches.length === 2) {
			var section: CanvasSectionObject = this.getSectionWithName(this.sectionOnMouseDown);
			if (section) {
				var diffX = Math.abs(e.touches[0].clientX - e.touches[1].clientX);
				var diffY = Math.abs(e.touches[0].clientY - e.touches[1].clientY);
				// Let's keep "touchCenter" variable "static" for now. When we want to allow move & drag at the same time, we should make it dynamic again.
				if (!this.touchCenter) {
					this.touchCenter = [(e.touches[0].clientX + e.touches[1].clientX) * 0.5, (e.touches[0].clientY + e.touches[1].clientY) * 0.5];
					this.touchCenter = this.convertPointToCanvasLocale(this.touchCenter);
				}
				var distance = Math.sqrt(Math.pow(diffX, 2) + Math.pow(diffY, 2));
				this.propagateOnMultiTouchMove(section, this.convertPositionToSectionLocale(section, this.touchCenter), distance, e);
			}
		}

		app.idleHandler.notifyActive();
	}

	public onTouchEnd (e: TouchEvent) { // Should be ignored unless this.draggingSomething = true.
		this.stopLongPress();
		if (!this.multiTouch) {
			this.positionOnMouseUp = this.convertPositionToCanvasLocale(e);
			var section: CanvasSectionObject;
			if (!this.draggingSomething) {
				section = this.findSectionContainingPoint(this.positionOnMouseUp);
			}
			else {
				section = this.getSectionWithName(this.sectionOnMouseDown);
			}
			if (section) {
				this.propagateOnMouseUp(section, this.convertPositionToSectionLocale(section, this.positionOnMouseUp), <MouseEvent><any>e);
			}
		}
		else if (e.touches.length === 0) {
			this.multiTouch = false;
			var section: CanvasSectionObject = this.getSectionWithName(this.sectionOnMouseDown);
			if (section) {
				this.propagateOnMultiTouchEnd(section, e);
			}
		}
		this.touchEventInProgress = true;
	}

	public onTouchCancel (e: TouchEvent) {
		this.clearMousePositions();
		this.stopLongPress();
	}

	private onDragOver(e: DragEvent) {
		// This is necessary to prevent window from taking the dropped object.
		e.preventDefault();
	}

	private onDrop(e: DragEvent) {
		e.preventDefault();

		const point = this.convertPositionToCanvasLocale(e);
		const target = this.findSectionContainingPoint(point);

		if (target)
			this.propagateOnDrop(target, this.convertPositionToSectionLocale(target, point), e);

		this.clearMousePositions();
	}

	public onResize (newWidth: number, newHeight: number) {
		var container: HTMLElement = <HTMLElement> this.canvas.parentNode;
		var cRect: ClientRect =	container.getBoundingClientRect();
		if (!newWidth)
			newWidth = cRect.right - cRect.left;

		if (!newHeight)
			newHeight = cRect.bottom - cRect.top;

		app.dpiScale = window.devicePixelRatio;
		app.roundedDpiScale = Math.round(window.devicePixelRatio);

		newWidth = Math.floor(newWidth * app.dpiScale);
		newHeight = Math.floor(newHeight * app.dpiScale);

		if (this.width === newWidth && this.height === newHeight && this.documentAnchor) {
			this.reNewAllSections(false);
			return;
		}

		this.clearMousePositions();
		this.width = newWidth;
		this.height = newHeight;
		this.needsResize = true;

		this.reNewAllSections(true);
	}

	private findSectionContainingPoint (point: Array<number>, interactable = true): any {
		for (var i: number = this.sections.length - 1; i > -1; i--) { // Search from top to bottom. Top section will be sent as target section.
			if (
				this.sections[i].isLocated && !this.sections[i].windowSection &&
				this.sections[i].showSection &&
				(!this.sections[i].documentObject || this.sections[i].isVisible) &&
				this.doesSectionIncludePoint(this.sections[i], point)
				&& (interactable ? this.sections[i].interactable : true)
			)
				return this.sections[i];
		}

		return null;
	}

	public doesSectionIncludePoint (section: any, point: number[]): boolean { // No ray casting here, it is a rectangle.
		// use isHit from section, that does check against bounds of local range (position, size)
		return section.isHit(point);
	}

	private doSectionsIntersectOnYAxis (section1: any, section2: any): boolean {
		var y11 = section1.myTopLeft[1];
		var y12 = section1.myTopLeft[1] + section1.size[1];

		var y21 = section2.myTopLeft[1];
		var y22 = section2.myTopLeft[1] + section2.size[1];

		if (((y11 >= y21 && y11 <= y22) || (y12 >= y21 && y12 <= y22)) || ((y21 >= y11 && y21 <= y12) || (y22 >= y11 && y22 <= y12)))
			return true;
		else
			return false;
	}

	private doSectionsIntersectOnXAxis (section1: any, section2: any): boolean {
		var x11 = section1.myTopLeft[0];
		var x12 = section1.myTopLeft[0] + section1.size[0];

		var x21 = section2.myTopLeft[0];
		var x22 = section2.myTopLeft[0] + section2.size[0];

		if (((x11 >= x21 && x11 < x22) || (x12 >= x21 && x12 <= x22)) || ((x21 >= x11 && x21 <= x12) || (x22 >= x11 && x22 <= x12)))
			return true;
		else
			return false;
	}

	// Find the left most point from a position with same zIndex.
	private hitLeft (section: CanvasSectionObject): number {
		var maxX = -Infinity;
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].isLocated && this.sections[i].zIndex === section.zIndex && this.sections[i].name !== section.name) {
				var currentLeft = this.sections[i].myTopLeft[0] + this.sections[i].size[0];
				if (currentLeft > maxX && currentLeft < section.myTopLeft[0]) {
					if (this.doSectionsIntersectOnYAxis(this.sections[i], section)) {
						maxX = currentLeft;
					}
				}
			}
		}
		if (maxX === -Infinity)
			return 0; // There is nothing on the left of this section.
		else
			return maxX + app.roundedDpiScale; // Don't overlap with the section on the left.
	}

	// Find the right most point from a position with same zIndex.
	private hitRight (section: CanvasSectionObject): number {
		var minX = Infinity;
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].isLocated && this.sections[i].zIndex === section.zIndex && this.sections[i].name !== section.name) {
				var currentRight = this.sections[i].myTopLeft[0];
				if (currentRight < minX && currentRight > section.myTopLeft[0]) {
					if (this.doSectionsIntersectOnYAxis(this.sections[i], section)) {
						minX = currentRight;
					}
				}
			}
		}

		if (minX === Infinity)
			return this.width; // There is nothing on the right of this section.
		else
			return minX - app.roundedDpiScale; // Don't overlap with the section on the right.
	}

	// Find the top most point from a position with same zIndex.
	private hitTop (section: CanvasSectionObject): number {
		var maxY = -Infinity;
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].isLocated && this.sections[i].zIndex === section.zIndex && this.sections[i].name !== section.name) {
				var currentTop =  this.sections[i].myTopLeft[1] + this.sections[i].size[1];
				if (currentTop > maxY && currentTop < section.myTopLeft[1]) {
					if (this.doSectionsIntersectOnXAxis(this.sections[i], section)) {
						maxY = currentTop;
					}
				}
			}
		}
		if (maxY === -Infinity)
			return 0; // There is nothing on the left of this section.
		else
			return maxY + app.roundedDpiScale; // Don't overlap with the section on the top.
	}

	// Find the bottom most point from a position with same zIndex.
	private hitBottom (section: CanvasSectionObject): number {
		var minY = Infinity;
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].isLocated && this.sections[i].zIndex === section.zIndex && this.sections[i].name !== section.name) {
				var currentBottom =  this.sections[i].myTopLeft[1];
				if (currentBottom < minY && currentBottom > section.myTopLeft[1]) {
					if (this.doSectionsIntersectOnXAxis(this.sections[i], section)) {
						minY = currentBottom;
					}
				}
			}
		}
		if (minY === Infinity)
			return this.height; // There is nothing on the left of this section.
		else
			return minY - app.roundedDpiScale; // Don't overlap with the section on the bottom.
	}

	public createUpdateSingleDivElement (section: CanvasSectionObject) {
		var bcr: ClientRect = this.canvas.getBoundingClientRect();
		var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-' + section.name);

		if ((!section.documentObject || section.isVisible) && section.isSectionShown() && section.myTopLeft) {
			if (!element) {
				element = document.createElement('div');
				element.id = 'test-div-' + section.name;
				document.body.appendChild(element);
			}
			element.style.position = 'fixed';
			element.style.zIndex = '-1';
			element.style.left = String(bcr.left + Math.round(section.myTopLeft[0] / app.dpiScale)) + 'px';
			element.style.top = String(bcr.top + Math.round(section.myTopLeft[1] / app.dpiScale)) + 'px';
			element.style.width = String(Math.round(section.size[0] / app.dpiScale)) + 'px';
			element.style.height = String(Math.round(section.size[1] / app.dpiScale)) + 'px';
			if (section.name === 'tiles') {
				// For tiles section add document coordinates of top and left too.
				element.innerText = JSON.stringify({
					top: Math.round(app.activeDocument.activeLayout.viewedRectangle.pY1),
					left: Math.round(app.activeDocument.activeLayout.viewedRectangle.pX1),
					width: Math.round(section.size[0]),
					height: Math.round(section.size[1])
				});
			}
		}
		else if (element)
			element.remove(); // Remove test-div if section is not visible.
	}

	private createUpdateDivElements () {
		for (var i: number = 0; i < this.sections.length; i++) {
			this.createUpdateSingleDivElement(this.sections[i]);
		}
	}

	public reNewAllSections(redraw: boolean = true) {
		this.orderSections();
		this.locateSections();

		for (var i: number = 0; i < this.sections.length; i++) {
			this.sections[i].onResize();
		}
		this.applyDrawingOrders();
		if (this.testing)
			this.createUpdateDivElements();
		if (redraw && this.drawingAllowed())
			this.requestReDraw();
		else
			this.resizeCanvas(); // Ensure canvas backing store is correctly sized even when drawing is disabled, to prevent blurriness on HiDPI displays
	}

	private roundPositionAndSize(section: CanvasSectionObject) {
		section.myTopLeft[0] = Math.round(section.myTopLeft[0]);
		section.myTopLeft[1] = Math.round(section.myTopLeft[1]);
		section.size[0] = Math.round(section.size[0]);
		section.size[1] = Math.round(section.size[1]);
	}

	private calculateSectionInitialPosition(section: CanvasSectionObject, index: number): number {
		if (typeof section.anchor[index] === 'string' || section.anchor[index].length === 1) {
			const anchor: string | string[] = typeof section.anchor[index] === 'string' ? section.anchor[index]: section.anchor[index][0];
			if (index === 0)
				return anchor === 'top' ? section.position[1]: (this.height - (section.position[1] + section.size[1]));
			else
				return anchor === 'left' ? section.position[0]: (this.width - (section.position[0] + section.size[0]));
		}
		else {
			// If we are here, it means section's edge(s) will be snapped to another section's edges.
			// Count should always be an odd number. Because last variable will be used as a fallback to canvas's edges (top, bottom, right or left).
			// See anchor explanation on top of this file.
			// Correct example: ["header", "bottom", "top"] => Look for section "header", if found, use its bottom, if not found, use canvas's top.
			if (section.anchor[index].length % 2 === 0) { // eslint-disable-line no-lonely-if
				console.error('Section: ' + section.name + '. Wrong anchor definition.');
				return 0;
			}
			else {
				var count: number = section.anchor[index].length;
				var targetSection: CanvasSectionObject = null;
				var targetEdge: string = null;
				for (var i: number = 0; i < count - 1; i++) {
					targetSection = this.getSectionWithName(section.anchor[index][i]);
					if (targetSection) {
						targetEdge = section.anchor[index][i + 1];
						break;
					}
				}

				if (targetSection) {
					// So, we have target section, we will use its position. Is it located?
					if (!targetSection.isLocated) {
						console.error('Section: ' + section.name + '. Target section for anchor should be located before this section.'
							+ ' It means that target section\'s (if zIndex is the same) processing order should be less or its zIndex should be less than this section.');
						return 0;
					}
					else {
						if (targetEdge === 'top') { // eslint-disable-line no-lonely-if
							return targetSection.myTopLeft[1] - app.roundedDpiScale;
						}
						else if (targetEdge === 'bottom') {
							return targetSection.myTopLeft[1] + targetSection.size[1] + app.roundedDpiScale;
						}
						else if (targetEdge === 'left') {
							return targetSection.myTopLeft[0] - app.roundedDpiScale;
						}
						else if (targetEdge === 'right') {
							return targetSection.myTopLeft[0] + targetSection.size[0] + app.roundedDpiScale;
						}
						else if (targetEdge === '-left') {
							if (section.expand[0] === 'left' && section.origSizeHint) {
								section.size[0] = section.origSizeHint[0];
							}
							return targetSection.myTopLeft[0] - section.size[0];
						}
					}
				}
				else {
					// No target section is found. Use fallback.
					const anchor: string | string[] = section.anchor[index][count - 1];
					if (index === 0) {
						return anchor === 'top' ? section.position[1]: (this.height - (section.position[1] + section.size[1]));
					}
					else {
						return anchor === 'left' ? section.position[0]: (this.width - (section.position[0] + section.size[0]));
					}
				}
			}
		}
	}

	private expandSection(section: CanvasSectionObject) {
		if (section.expand.includes('left')) {
			var initialX = section.myTopLeft[0];
			section.myTopLeft[0] = this.hitLeft(section);
			section.size[0] = initialX - section.myTopLeft[0];
		}

		if (section.expand.includes('right')) {
			section.size[0] = this.hitRight(section) - section.myTopLeft[0];
		}

		if (section.expand.includes('top')) {
			var initialY = section.myTopLeft[1];
			section.myTopLeft[1] = this.hitTop(section);
			section.size[1] = initialY - section.myTopLeft[1];
		}

		if (section.expand.includes('bottom')) {
			section.size[1] = this.hitBottom(section) - section.myTopLeft[1];
		}
	}

	private locateSections () {

		for (var i: number = 0; i < this.sections.length; i++) {
			const section = this.sections[i];
			// Reset some values.
			section.isLocated = false;
			section.myTopLeft = null;

			// Preserve the original size hint
			if (typeof section.origSizeHint === 'undefined') {
				section.origSizeHint = [...section.size];
			}
		}

		this.documentAnchor = null;
		this.windowSectionList = [];

		for (var i: number = 0; i < this.sections.length; i++) {
			const section: CanvasSectionObject = this.sections[i];

			if (section.documentObject === true) { // "Document anchor" section should be processed before "document object" sections.
				if (section.size && section.position) {
					section.isLocated = true;
					section.myTopLeft = [section.documentPosition.vX, section.documentPosition.vY];
				}
			}
			else if (section.boundToSection) { // Don't set boundToSection property for "window sections".
				var parentSection = this.getSectionWithName(section.boundToSection);
				if (parentSection) {
					section.myTopLeft = [0, 0];
					section.size = [0, 0];
					section.size[0] = parentSection.size[0];
					section.size[1] = parentSection.size[1];

					section.myTopLeft[0] = parentSection.myTopLeft[0];
					section.myTopLeft[1] = parentSection.myTopLeft[1];

					this.roundPositionAndSize(section);
					section.isLocated = true;
				}
			}
			else if (section.windowSection) {
				section.myTopLeft = [0, 0];
				section.size = [this.width, this.height];
				section.isLocated = true;
				this.windowSectionList.push(section);
			}
			else { // A regular UI element.
				section.myTopLeft = [this.calculateSectionInitialPosition(section, 1), this.calculateSectionInitialPosition(section, 0)];

				if (section.expand.length !== 0)
					this.expandSection(section);

				this.roundPositionAndSize(section);
				section.isLocated = true;
			}

			if (section.name === this.documentAnchorSectionName) {
				this.documentAnchor = [section.myTopLeft[0], section.myTopLeft[1]];

				if (app.activeDocument?.activeLayout)
					app.activeDocument.activeLayout.documentAnchorPosition = this.documentAnchor.slice();
			}
		}
	}

	private orderSections () {
		// According to zIndex & processing order.
		function compareSections(a: CanvasSectionObject, b: CanvasSectionObject) {
			var zIndexDiff = a.zIndex - b.zIndex;
			if (zIndexDiff != 0)
				return zIndexDiff;
			return a.processingOrder - b.processingOrder;
		}
		this.sections.sort(compareSections);
	}

	public applyDrawingOrders () {
		// According to zIndex, drawing order & processing order.
		// Section with the highest drawing order will be drawn on top (inside same zIndex).
		function compareSections(a: CanvasSectionObject, b: CanvasSectionObject) {
			var zIndexDiff = a.zIndex - b.zIndex;
			if (zIndexDiff != 0)
				return zIndexDiff;
			var drawingOrderDiff = a.drawingOrder - b.drawingOrder;
			if (drawingOrderDiff != 0)
				return drawingOrderDiff;
			return a.processingOrder - b.processingOrder;
		}
		this.sections.sort(compareSections);
	}

	private drawSectionBorders () {
		this.context.lineWidth = 2 * app.dpiScale;
		this.context.strokeStyle = 'blue';
		for (var i: number = 0; i < this.sections.length; i++) {
			var section = this.sections[i];

			if (section.isLocated && section.showSection && (!section.documentObject || section.isVisible)) {
				var xStart = section.myTopLeft[0];
				var xEnd = xStart + section.size[0];

				var yStart = section.myTopLeft[1];
				var yEnd = yStart + section.size[1];

				this.context.beginPath();
				this.context.moveTo(xStart, yStart);
				this.context.lineTo(xEnd, yStart);
				this.context.stroke();
				this.context.beginPath();
				this.context.moveTo(xEnd, yStart);
				this.context.lineTo(xEnd, yEnd);
				this.context.stroke();
				this.context.beginPath();
				this.context.moveTo(xEnd, yEnd);
				this.context.lineTo(xStart, yEnd);
				this.context.stroke();
				this.context.beginPath();
				this.context.moveTo(xStart, yEnd);
				this.context.lineTo(xStart, yStart);
				this.context.stroke();
			}
		}
	}

	public setPenPosition (section: CanvasSectionObject) {
		this.context.setTransform(1, 0, 0, 1, 0, 0);
		this.context.translate(section.myTopLeft[0], section.myTopLeft[1]);
	}

	private shouldDrawSection (section: CanvasSectionObject) {
	    return section.isLocated && section.showSection && (!section.documentObject || section.isVisible);
	}

	private drawSections (frameCount: number = null, elapsedTime: number = null) {

		// Un-comment to debug duplicate rendering problems:
		// const stack = new Error().stack;
		// console.log("Draw sections:\n", stack);

		if (app.map && app.map._debug)
			app.map._debug.setOverlayMessage('top-frames', 'Frames: ' + this.framesRendered++);

		this.context.setTransform(1, 0, 0, 1, 0, 0);

		if (!this.zoomChanged) {
			this.clearCanvas();
		}

		this.context.font = String(20 * app.dpiScale) + 'px Verdana';
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.shouldDrawSection(this.sections[i])) {
				this.context.translate(this.sections[i].myTopLeft[0], this.sections[i].myTopLeft[1]);
				if (this.sections[i].backgroundColor) {
					this.context.globalAlpha = this.sections[i].backgroundOpacity;
					this.context.fillStyle = this.sections[i].backgroundColor;
					this.context.fillRect(0, 0, this.sections[i].size[0], this.sections[i].size[1]);
					this.context.globalAlpha = 1;
				}

				this.sections[i].onDraw(frameCount, elapsedTime);
				if (this.sections[i].borderColor) { // If section's border is set, draw its borders after section's "onDraw" function is called.
					var offset = this.sections[i].getLineOffset();
					this.context.lineWidth = this.sections[i].getLineWidth();
					this.context.strokeStyle = this.sections[i].borderColor;
					this.context.strokeRect(offset, offset, this.sections[i].size[0], this.sections[i].size[1]);
				}

				this.context.translate(-this.sections[i].myTopLeft[0], -this.sections[i].myTopLeft[1]);
			}
		}

		//this.drawSectionBorders();
	}

	public isMouseInside(): boolean {
		return this.mouseIsInside;
	}

	public doesSectionExist (name: string): boolean {
		if (name && typeof name === 'string') {
			return this.sectionsByName.has(name);
		}
		else {
			return false;
		}
	}

	private checkNewSectionName (options: any) {
		if (options.name !== undefined && typeof options.name === 'string' && options.name.trim() !== '') {
			if (this.doesSectionExist(options.name)) {
				console.error('There is a section with the same name: ' + options.name + '. Use doesSectionExist for existancy checks.');
				return false;
			}
			else if (['top', 'left', 'bottom', 'right'].includes(options.name.trim())) {
				console.error('"top", "left", "bottom" and "right" words are reserved. Choose another name for the section.');
				return false;
			}
			else {
				return true;
			}
		}
		else {
			console.error('Sections should have a "name" property.');
			return false;
		}
	}

	private checkSectionProperties (options: any) {
		if (
			options.anchor === undefined
			|| options.position === undefined
			|| options.size === undefined
			|| options.expand === undefined
			|| options.processingOrder === undefined
			|| options.drawingOrder === undefined
			|| options.zIndex === undefined
			|| options.interactable === undefined
		) {
			console.error('Section has missing properties. See "checkSectionProperties" function.');
			return false;
		}
		else {
			if (options.showSection === undefined)
				options.showSection = true;

			if (options.windowSection === undefined)
				options.windowSection = false;

			return true;
		}
	}

	private newSectionChecks (options: any): boolean {
		if (!this.checkNewSectionName(options))
			return false;
		else if (!this.checkSectionProperties(options))
			return false;
		else
			return true;
	}

	private addSectionFunctions(section: CanvasSectionObject) {
		// This is to be removed to a better place.
		section.isCalcRTL = function(): boolean {
			const docLayer = this.sectionProperties.docLayer;
			if (docLayer && docLayer.isCalcRTL())
				return true;

			return false;
		}.bind(section);
	}

	public addSection (newSection: CanvasSectionObject) {
		if (this.newSectionChecks(newSection)) {
			this.pushSection(newSection);
			return true;
		}
		else {
			return false;
		}
	}

	private pushSection (newSection: CanvasSectionObject) {
		// Every section can draw from Point(0, 0), their drawings will be translated to myTopLeft position.
		newSection.context = this.context;
		newSection.containerObject = this;
		newSection.sectionProperties.section = newSection;
		this.sections.push(newSection);
		this.sectionsByName.set(newSection.name, newSection);
		this.addSectionFunctions(newSection);
		newSection.onInitialize();
		if (this.drawingAllowed()) {
			this.updateBoundSectionLists();
			this.reNewAllSections();
		} else
			this.sectionsDirty = true;
	}

	public removeSection (name: string) {
		var found: boolean = false;
		for (var i: number = 0; i < this.sections.length; i++) {
			if (this.sections[i].name === name) {
				var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-' + this.sections[i].name);
				if (element) // Remove test div if exists.
					document.body.removeChild(element);
				this.sections[i].onRemove();
				this.sectionsByName.delete(name);
				this.sections[i] = null;
				this.sections.splice(i, 1);
				found = true;
				break;
			}
		}

		if (found) {
			if (!this.drawingAllowed())
			    this.sectionsDirty = true;
			else {
			    this.updateBoundSectionLists();
			    this.reNewAllSections();
			}
			return true;
		}
		else {
			return false;
		}
	}

	private setAnimatingSectionName (sectionName: string) {
		this.animatingSectionName = sectionName;
	}

	public getAnimatingSectionName (): string {
		return this.animatingSectionName;
	}

	private animate (timeStamp: number) {
		app.enterRAF();

		if (this.lastFrameStamp > 0)
			this.elapsedTime += Math.max(0, timeStamp - this.lastFrameStamp);

		this.lastFrameStamp = timeStamp;

		if (this.duration && this.elapsedTime >= this.duration) { // This is not the only place that can set "continueAnimating" to "false".
			this.continueAnimating = false;
		}

		var section: CanvasSectionObject = this.getSectionWithName(this.getAnimatingSectionName());
		if (this.continueAnimating) {
			if (section) section.onAnimate(this.frameCount, this.elapsedTime);
			this.frameCount++;
			this.requestReDraw();
		}
		else {
			if (section) {
				section.isAnimating = false;
				section.onAnimationEnded(this.frameCount, this.elapsedTime);
			}

			this.setAnimatingSectionName(null);
			this.frameCount = this.elapsedTime = null;
		}

		app.exitRAF();
	}

	// Resets animation duration. Not to be called directly. Instead, use (inside section class) this.resetAnimation()
	public resetAnimation (sectionName: string) {
		if (sectionName === this.getAnimatingSectionName()) {
			this.lastFrameStamp = 0;
			this.elapsedTime = 0;
			this.frameCount = 0;
		}
	}

	public stopAnimating () {
		// Though this function is available for every section, generally, only the section that started animation should use this.
		this.continueAnimating = false;
	}

	// Don't call this directly. Instead, call (inside the section class) section.startAnimating(options).
	public startAnimating (sectionName: string, options: any): boolean {
		/*
			Most of the time, we need to draw entire canvas when animating.
			Because if there is another section under the animated one, that section needs to be renewed too.
			Also, sections may need to be redrawn because of the updated view (while animating).
			This animation feature re-draws all sections with requestAnimationFrame.
			Developer can set options to ensure the animation stops at certain point.
			If your section's "onDraw" function is given the variables "frameCount, elapsedTime", then you can assume that CanvasSectionContainer is in animation mode.
			You can also check getAnimatingSectionName function of container class (if it is null or not) to see if animating is on.

			Sections other than the one which started the animation, can't know when the animation will stop.

			The section which started the animation => "(inside section class) this.containerObject.getAnimatingSectionName()".

			For now, only one section can start animations at a time.

			options (possible values are separated by the '|' char):
				// Developer can set the duration for the animation, in milliseconds. There are also other ways to stop the animation.
				duration: 2000 | null // 2 seconds | null.
				// The animation can start after a requestAnimationFrame
				defer: boolean
		*/

		if (!this.getAnimatingSectionName()) {
			this.setAnimatingSectionName(sectionName);
			this.getSectionWithName(sectionName).isAnimating = true;
			this.lastFrameStamp = 0;
			this.continueAnimating = true;
			this.duration = options.duration ? options.duration: null;
			this.elapsedTime = 0;
			this.frameCount = 0;

			if (options.defer)
				requestAnimationFrame(this.animate.bind(this));
			else
				this.animate(document.timeline.currentTime as number);
			return true;
		}
		else {
			return false;
		}
	}
}
