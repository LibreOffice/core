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
	MouseControl class is for handling mouse events if no other section (bound to tiles)
	handles it. This section is responsible for sending events to core side.
	This section is bound to tiles section. Tiles section represents the document view.
	Since this MouseControl class will handle mouse events that need to be sent to the core side,
	binding this class to tiles is normal. This class shouldn't cover all the canvas but only the document area.
*/

class MouseControl extends CanvasSectionObject {
	zIndex: number = app.CSections.MouseControl.zIndex;
	drawingOrder: number = app.CSections.MouseControl.drawingOrder;
	processingOrder: number = app.CSections.MouseControl.processingOrder;
	boundToSection: string = app.CSections.Tiles.name;

	mouseMoveTimer: any | null = null;
	clickTimer: any | null = null;
	currentPosition: cool.SimplePoint = new cool.SimplePoint(0, 0);
	clickCount: number = 0;
	positionOnMouseDown: cool.SimplePoint | null = null;
	localPositionOnMouseDown: cool.SimplePoint | null = null;
	mouseDownSent: boolean = false;

	inSwipeAction: boolean = false;
	swipeVelocity: number[] = [0, 0];
	swipeTimeStamp: number = 0;
	amplitude: number[] = [0, 0];
	touchstart: number = 0;
	previousViewedRectangle: cool.SimpleRectangle | null = null; // To check if we hit the borders of document.

	pinchStartCenter: any;
	zoom: any;
	origCenter: any;
	pinchLength: number = 0;

	constructor(name: string) {
		super(name);
	}

	static readModifier(e: MouseEvent) {
		let modifier = 0;
		const shift = e.shiftKey ? app.UNOModifier.SHIFT : 0;
		const ctrl = e.ctrlKey ? app.UNOModifier.CTRL : 0;
		const alt = e.altKey ? app.UNOModifier.ALT : 0;
		const cmd = e.metaKey ? app.UNOModifier.CTRLMAC : 0;
		modifier = shift | ctrl | alt | cmd;

		return modifier;
	}

	/*
		We need to map view coordinates to document coordinates.
		When user clicks at a position on the screen, we have its document coordinate according to
		traditional document view: It starts from top-left and ends at bottom-right. Now the new ViewLayouts
		can paint multiple parts at the same time and they can paint objects in different locations than the objects' original positions.
		This means that the point user clicks/taps may not overlap with the document through simple math.
		During this transition phase (to view layouts), we need both point.pX and point.vX properties. Later, we may use only vX, vY etc.

		This function should be used only for real user interactions. If you want to simulate a click at a certain position, call _postMouseEvent directly.
	*/
	private postCoreMouseEvent(
		eventType: string,
		point: cool.SimplePoint,
		clickCount: number,
		buttons: number,
		modifier: number,
	) {
		let viewToDocumentPos = point.clone();
		Util.ensureValue(app.activeDocument);

		// Convert to pure canvas html element coordinate.
		viewToDocumentPos.pX +=
			-app.activeDocument.activeLayout.viewedRectangle.pX1 +
			app.sectionContainer.getDocumentAnchor()[0];
		viewToDocumentPos.pY +=
			-app.activeDocument.activeLayout.viewedRectangle.pY1 +
			app.sectionContainer.getDocumentAnchor()[1];

		viewToDocumentPos =
			app.activeDocument.activeLayout.canvasToDocumentPoint(viewToDocumentPos);

		if (Number.isNaN(viewToDocumentPos.x) || Number.isNaN(viewToDocumentPos.y))
			return;
		else {
			app.map._docLayer._postMouseEvent(
				eventType,
				viewToDocumentPos.x,
				viewToDocumentPos.y,
				clickCount,
				buttons,
				modifier,
			);
		}
	}

	public onContextMenu(point: cool.SimplePoint, e: MouseEvent): void {
		// We need this to prevent native context menu.
		e.preventDefault();

		// We will remove below ones after we remove map HTML element.
		e.stopPropagation();
		e.stopImmediatePropagation();

		const buttons = app.LOButtons.right;
		const modifier = MouseControl.readModifier(e);

		if (modifier === 0) {
			this.postCoreMouseEvent(
				'buttondown',
				this.currentPosition,
				1,
				buttons,
				modifier,
			);
		}
	}

	// Gets the mouse position either on browser page or within the
	// canvas in css pixels.
	private _getMousePosition(browserPage: boolean): cool.PointLike {
		Util.ensureValue(app.activeDocument);
		const pagePosition = this.currentPosition.clone();
		let docTLx = app.activeDocument.activeLayout.viewedRectangle.pX1;
		let docTLy = app.activeDocument.activeLayout.viewedRectangle.pY1;
		if (app.map.getDocType() === 'spreadsheet') {
			if (app.isXOrdinateInFrozenPane(pagePosition.pX)) {
				docTLx = 0;
			}

			if (app.isYOrdinateInFrozenPane(pagePosition.pY)) {
				docTLy = 0;
			}
		}
		pagePosition.pX -= docTLx - this.containerObject.getDocumentAnchor()[0];
		pagePosition.pY -= docTLy - this.containerObject.getDocumentAnchor()[1];
		if (browserPage) {
			const boundingClientRectangle =
				this.context.canvas.getBoundingClientRect();
			return {
				x: pagePosition.cX + boundingClientRectangle.x,
				y: pagePosition.cY + boundingClientRectangle.y,
			};
		}
		return {
			x: pagePosition.cX,
			y: pagePosition.cY,
		};
	}

	// Gets the mouse position on browser page in CSS pixels.
	public getMousePagePosition() {
		return this._getMousePosition(true);
	}

	// Gets the mouse position on canvas in CSS pixels.
	public getMouseCanvasPosition() {
		return this._getMousePosition(false);
	}

	// This is useful when a section handles the event but wants to set the document mouse position.
	public setMousePosition(point: cool.SimplePoint) {
		this.currentPosition = point.clone();
	}

	private refreshPosition(point: cool.SimplePoint) {
		Util.ensureValue(app.activeDocument);
		let topLeftX = app.activeDocument.activeLayout.viewedRectangle.pX1;
		let topLeftY = app.activeDocument.activeLayout.viewedRectangle.pY1;

		if (app.map.getDocType() === 'spreadsheet') {
			if (app.isXOrdinateInFrozenPane(point.pX)) topLeftX = 0;

			if (app.isYOrdinateInFrozenPane(point.pY)) topLeftY = 0;
		}

		this.currentPosition.pX = point.pX + topLeftX;
		this.currentPosition.pY = point.pY + topLeftY;
	}

	private setCursorType() {
		// If the core has set a specific pointer (e.g. 'pointer' for hyperlinks),
		// let it take precedence over our client-side cursor.
		const corePointer = app.map._docLayer._coreMousePointer;
		if (corePointer && corePointer !== 'default') return;

		// If we have blinking cursor visible
		// we need to change cursor from default style
		if (app.file.textCursor.visible) this.context.canvas.style.cursor = 'text';
		else if (app.map._docLayer._docType === 'spreadsheet') {
			const textCursor =
				app.file.textCursor.visible &&
				app.calc.cellCursorRectangle &&
				app.calc.cellCursorRectangle.pContainsPoint(
					this.currentPosition.pToArray(),
				);

			if (textCursor) {
				const change =
					this.context.canvas.style.cursor !== 'text' ||
					this.context.canvas.classList.contains('spreadsheet-cursor');
				if (change) {
					this.context.canvas.classList.remove('spreadsheet-cursor');
					this.context.canvas.style.cursor = 'text';
				}
			} else {
				const change =
					this.context.canvas.style.cursor !== '' ||
					!this.context.canvas.classList.contains('spreadsheet-cursor');
				if (change) {
					this.context.canvas.style.cursor = '';
					this.context.canvas.classList.add('spreadsheet-cursor');
				}
			}
		} else if (app.map._docLayer._docType === 'presentation') {
			this.context.canvas.style.cursor = '';
		}
	}

	private cancelSwipe() {
		this.inSwipeAction = false;

		if (this.containerObject.getAnimatingSectionName() === this.name)
			this.containerObject.stopAnimating();
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		if (this.inSwipeAction) {
			const elapsed = Date.now() - this.swipeTimeStamp;
			const delta = [
				this.amplitude[0] * Math.exp(-elapsed / 650),
				this.amplitude[1] * Math.exp(-elapsed / 650),
			];

			Util.ensureValue(app.activeDocument);

			if (Math.abs(delta[0]) > 0.2 || Math.abs(delta[1]) > 0.2) {
				app.activeDocument.activeLayout.scrollTo(
					app.activeDocument.activeLayout.viewedRectangle.pX1 + delta[0],
					app.activeDocument.activeLayout.viewedRectangle.pY1 + delta[1],
				);
				app.sectionContainer.requestReDraw();

				if (this.previousViewedRectangle) {
					if (
						app.activeDocument.activeLayout.viewedRectangle.equals(
							this.previousViewedRectangle.toArray(),
						)
					)
						this.cancelSwipe();
				}

				this.previousViewedRectangle =
					app.activeDocument.activeLayout.viewedRectangle.clone();
			} else this.cancelSwipe();
		}
	}

	// Comment from original author:
	/*
		Code and maths for the ergonomic scrolling is inspired by formulas at
		https://ariya.io/2013/11/javascript-kinetic-scrolling-part-2
		Some constants are changed based on the testing/experimenting/trial-error
	*/
	private swipe(e: any): void {
		this.previousViewedRectangle = null;

		const velocityX = app.map._docLayer.isCalcRTL()
			? -e.velocityX
			: e.velocityX;
		const pointVelocity = [velocityX, e.velocityY];

		if (this.inSwipeAction) {
			this.swipeVelocity[0] += pointVelocity[0];
			this.swipeVelocity[1] += pointVelocity[1];
		} else {
			this.swipeVelocity = pointVelocity;
			this.inSwipeAction = true;
		}

		this.amplitude = [this.swipeVelocity[0] * 0.1, this.swipeVelocity[1] * 0.1];
		this.swipeTimeStamp = Date.now();

		const animatingSection = this.containerObject.getAnimatingSectionName();

		if (!animatingSection) this.startAnimating({ defer: true });

		app.idleHandler.notifyActive();
	}

	public onMouseMove(
		point: cool.SimplePoint,
		dragDistance: Array<number>,
		e: MouseEvent,
	): void {
		this.setCursorType();

		this.refreshPosition(point);

		if (this.clickTimer) return;

		clearTimeout(this.mouseMoveTimer);

		const count = 1;
		const modifier = MouseControl.readModifier(e);

		if (!this.containerObject.isDraggingSomething()) {
			this.mouseMoveTimer = setTimeout(() => {
				this.postCoreMouseEvent(
					'move',
					this.currentPosition,
					count,
					0,
					modifier,
				);
			}, 100);
		} else if (e.type === 'touchmove' && this.positionOnMouseDown) {
			// For non-touch events, we can select text etc, so we send the mouse button events to core while dragging.
			// Users can scroll the view using keyboard or mouse wheel, or the scroll bars in those devices.
			// On touch devices, dragging (touchmove) is used to scroll the view.
			// We don't send the mouse button down and up events to core while dragging (touchmove). Instead, we scroll the view.
			const diff = this.currentPosition.clone();
			diff.x -= this.positionOnMouseDown.x;
			diff.y -= this.positionOnMouseDown.y;

			Util.ensureValue(app.activeDocument);
			const viewedRectangle =
				app.activeDocument.activeLayout.viewedRectangle.clone();

			// Use scrollTo, or repeating events break the scrolling.
			app.activeDocument.activeLayout.scrollTo(
				viewedRectangle.pX1 - diff.pX,
				viewedRectangle.pY1 - diff.pY,
			);
		} else {
			if (!this.mouseDownSent && this.positionOnMouseDown) {
				this.postCoreMouseEvent(
					'buttondown',
					this.positionOnMouseDown,
					count,
					app.LOButtons.left,
					modifier,
				);
				this.mouseDownSent = true;
			}

			if (!this.containerObject.isMouseInside()) {
				point.pX += this.position[0];
				point.pY += this.position[1];
				app.map.fire('handleautoscroll', {
					pos: { x: point.cX, y: point.cY },
					map: app.map,
				});
			} else app.map.fire('scrollvelocity', { vx: 0, vy: 0 });

			this.postCoreMouseEvent(
				'move',
				this.currentPosition,
				count,
				app.LOButtons.left,
				modifier,
			);

			this.showShapeDragPreview(dragDistance);
		}

		app.idleHandler.notifyActive();
	}

	// Shows unselected shapes drag preview
	private showShapeDragPreview(dragDistance: number[]): void {
		const handles = GraphicSelection.handlesSection;
		if (!handles?.sectionProperties?.svg) return;
		if (!GraphicSelection.extraInfo?.isDraggable) return;

		handles.sectionProperties.svg.style.left =
			String((handles.myTopLeft[0] + dragDistance[0]) / app.dpiScale) + 'px';
		handles.sectionProperties.svg.style.top =
			String((handles.myTopLeft[1] + dragDistance[1]) / app.dpiScale) + 'px';
		handles.sectionProperties.svg.style.opacity = '0.5';
		handles.showSVG();
	}

	private hideShapeDragPreview(): void {
		const handles = GraphicSelection.handlesSection;
		if (!handles?.sectionProperties?.svg) return;

		handles.sectionProperties.svg.style.opacity = '1';
		handles.hideSVG();
	}

	onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		this.refreshPosition(point);
		this.positionOnMouseDown = this.currentPosition.clone();

		if (e.type === 'touchstart') {
			// For swipe action.
			this.localPositionOnMouseDown = point.clone();
			this.touchstart = Date.now();
		}
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		this.refreshPosition(point);

		this.hideShapeDragPreview();

		if (this.mouseDownSent) {
			this.postCoreMouseEvent(
				'buttonup',
				this.currentPosition,
				1,
				app.LOButtons.left,
				MouseControl.readModifier(e),
			);

			app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
		} else if (e.type === 'touchend' && this.localPositionOnMouseDown) {
			// For swipe action.
			const diff = new cool.SimplePoint(
				this.localPositionOnMouseDown.x - point.x,
				this.localPositionOnMouseDown.y - point.y,
			);
			const timeDiff = Date.now() - this.touchstart;

			if (timeDiff < 200 && (Math.abs(diff.cX) > 5 || Math.abs(diff.cY) > 5))
				this.swipe({ velocityX: diff.pX, velocityY: diff.pY });
			else if (this.inSwipeAction) this.cancelSwipe();

			this.localPositionOnMouseDown = null;
		}

		this.positionOnMouseDown = null;
		this.mouseDownSent = false;

		if (this.containerObject.isDraggingSomething()) app.map.focus();
	}

	onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		if (app.map._docLayer._docType === 'spreadsheet') {
			this.context.canvas.classList.add('spreadsheet-cursor');
		}
		this.context.canvas.style.cursor = '';
	}

	onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		// Normally, we don't change the cursor style on mouse leave.
		// That is responsibility of the new target section.
		// But this is a class name and we need to remove it.
		if (app.map._docLayer._docType === 'spreadsheet') {
			this.context.canvas.classList.remove('spreadsheet-cursor');
		}
	}

	public getMobileKeyboardVisibility(): boolean {
		if (!app.map._docLayer) return false;
		else if (app.map._docLayer._docType === 'text') return true;
		else if (app.map._docLayer._docType === 'spreadsheet') {
			const acceptInput =
				app.calc.cellCursorVisible &&
				app.calc.cellCursorRectangle &&
				app.calc.cellCursorRectangle.containsPoint(
					this.currentPosition.toArray(),
				);
			return acceptInput as boolean;
		} else return false;
	}

	private sendClick(clickInfo: any, count: number) {
		this.postCoreMouseEvent(
			'buttondown',
			clickInfo.sendingPosition,
			count,
			clickInfo.buttons,
			clickInfo.modifier,
		);
		this.postCoreMouseEvent(
			'buttonup',
			clickInfo.sendingPosition,
			count,
			clickInfo.buttons,
			clickInfo.modifier,
		);
	}

	onClick(point: cool.SimplePoint, e: MouseEvent): void {
		app.map.fire('closepopups');
		app.map.fire('editorgotfocus');

		this.refreshPosition(point);
		this.clickCount++;

		if (!(<any>window).mode.isDesktop()) app.map.fire('closemobilewizard');

		// Right click is not supported. And click event doesn't have "buttons" property set. Safe to set it here to default.
		let buttons = app.LOButtons.left;

		let modifier = MouseControl.readModifier(e);
		const sendingPosition = this.currentPosition.clone();

		// Turn ctrl-left-click into right-click for browsers on macOS
		if (window.L.Browser.mac) {
			if (modifier == app.UNOModifier.CTRL && buttons == app.LOButtons.left) {
				modifier = 0;
				buttons = app.LOButtons.right;
			}
		}

		const clickInfo = {
			sendingPosition: sendingPosition,
			buttons: buttons,
			modifier: modifier,
		};

		if (this.clickTimer) app.timerRegistry.clearTimeout(this.clickTimer);
		else {
			// Old code always sends the first click, so do we.
			this.sendClick(clickInfo, 1);

			// For future: Here, we are checking the window size to determine the view mode, we can also check the event type (touch/click).
			app.map.focus(
				(<any>window).mode.isDesktop()
					? undefined
					: this.getMobileKeyboardVisibility(),
			);
		}

		this.clickTimer = app.timerRegistry.setTimeout(
			'clicktimer',
			() => {
				if (this.clickCount === 3 && app.map._docLayer.isCalc()) {
					// Also send a double click for a triple click in Calc.
					this.sendClick(clickInfo, 2);
					this.sendClick(clickInfo, 3);
				} else if (this.clickCount > 1) {
					this.sendClick(clickInfo, this.clickCount);
				}

				this.clickTimer = null;
				this.clickCount = 0;
			},
			250,
		);
	}

	onMultiTouchStart(e: TouchEvent): void {
		if (this.inSwipeAction) this.containerObject.stopAnimating();

		if (e.touches.length !== 2) return;

		const centerX = Math.round(
			(e.touches[0].clientX + e.touches[1].clientX) * 0.5,
		);
		const centerY = Math.round(
			(e.touches[0].clientY + e.touches[1].clientY) * 0.5,
		);

		if (isNaN(centerX) || isNaN(centerY)) return;

		this.pinchLength = Math.sqrt(
			Math.pow(e.touches[0].clientX - e.touches[1].clientX, 2) +
				Math.pow(e.touches[0].clientY - e.touches[1].clientY, 2),
		);

		this.pinchStartCenter = { x: centerX, y: centerY };
		const _pinchStartLatLng = app.map.mouseEventToLatLng({
			clientX: centerX,
			clientY: centerY,
		});
		app.map._docLayer.preZoomAnimation(_pinchStartLatLng);
	}

	onMultiTouchMove(
		point: cool.SimplePoint,
		dragDistance: number,
		e: TouchEvent,
	): void {
		const centerX = Math.round(
			(e.touches[0].clientX + e.touches[1].clientX) * 0.5,
		);
		const centerY = Math.round(
			(e.touches[0].clientY + e.touches[1].clientY) * 0.5,
		);

		if (!this.pinchStartCenter || isNaN(centerX) || isNaN(centerY)) return;

		// we need to invert the offset or the map is moved in the opposite direction
		var offset = {
			x: centerX - this.pinchStartCenter.x,
			y: centerY - this.pinchStartCenter.y,
		};
		var center = {
			x: this.pinchStartCenter.x - offset.x,
			y: this.pinchStartCenter.y - offset.y,
		};

		const newPinchLength = Math.sqrt(
			Math.pow(e.touches[0].clientX - e.touches[1].clientX, 2) +
				Math.pow(e.touches[0].clientY - e.touches[1].clientY, 2),
		);
		const diff = newPinchLength - this.pinchLength;
		this.zoom = app.map.getZoom() + diff * 0.01;
		this.zoom = app.map._limitZoom(this.zoom);

		this.origCenter = app.map.mouseEventToLatLng({
			clientX: center.x,
			clientY: center.y,
		});

		if (app.map._docLayer.zoomStep)
			app.map._docLayer.zoomStep(this.zoom, this.origCenter);

		app.idleHandler.notifyActive();
	}

	onMultiTouchEnd(e: TouchEvent): void {
		var oldZoom = app.map.getZoom();
		var zoomDelta = this.zoom - oldZoom;
		var finalZoom = app.map._limitZoom(
			zoomDelta > 0 ? Math.ceil(this.zoom) : Math.floor(this.zoom),
		);

		this.pinchStartCenter = undefined;

		if (app.map._docLayer.zoomStepEnd) {
			app.map._docLayer.zoomStepEnd(
				finalZoom,
				this.origCenter,
				function (newMapCenter: any) {
					// mapUpdater
					app.map.setView(newMapCenter, finalZoom);
				},
				// showMarkers
				function () {
					app.map._docLayer.postZoomAnimation();
				},
			);
		}
	}

	onDrop(position: cool.SimplePoint, e: DragEvent): void {
		this.refreshPosition(position);

		const modifier = MouseControl.readModifier(e);

		// Move the cursor, so that the insert position is as close to the drop coordinates as possible.
		this.postCoreMouseEvent(
			'buttondown',
			this.currentPosition,
			1,
			app.LOButtons.left,
			modifier,
		);
		this.postCoreMouseEvent(
			'buttonup',
			this.currentPosition,
			1,
			app.LOButtons.left,
			modifier,
		);

		if (app.map._clip && e.dataTransfer) {
			// Always capture the html content separate as we may lose it when we
			// pass the clipboard data to a different context (async calls, f.e.).
			const htmlText = e.dataTransfer.getData('text/html');
			app.map._clip.dataTransferToDocument(
				e.dataTransfer,
				/* preferInternal = */ false,
				htmlText,
			);
		}
	}
}
