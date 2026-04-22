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
/* See CanvasSectionContainer.ts for explanations. */

// We will keep below definitions until we use tsconfig.json.

namespace cool {

export class ScrollSection extends CanvasSectionObject {
	// The fraction of the line-height per 16.6ms to accelerate
	static readonly scrollAnimationAcceleration: number = 0.1;

	// The number of lines a scroll-wheel tick should travel. Note that
	// this is approximate, as it doesn't include deceleration once the target is reached.
	static readonly scrollWheelDelta: number = 3;

	// Any scroll events within this number of milliseconds after direct (e.g. touchpad)
	// scrolling will be treated as direct scroll events.
	static readonly scrollDirectTimeoutMs: number = 100;

	processingOrder: number = app.CSections.Scroll.processingOrder
	drawingOrder: number = app.CSections.Scroll.drawingOrder;
	zIndex: number = app.CSections.Scroll.zIndex;
	windowSection: boolean = true; // This section covers the entire canvas.

	map: any;
	autoScrollTimer: any;
	pendingScrollEvent: any = null;
	stepByStepScrolling: boolean = false; // quick scroll will move "page up/down" not "jump to"

	isRTL: () => boolean;

	constructor (isRTL?: () => boolean) {
		super(app.CSections.Scroll.name);

		this.map = window.L.Map.THIS;

		this.isRTL = isRTL ?? (() => false);

		this.map.on('scrollby', this.onScrollBy, this);
		this.map.on('scrollvelocity', this.onScrollVelocity, this);
		this.map.on('handleautoscroll', this.onHandleAutoScroll, this);
	}

	public onInitialize (): void {
		this.sectionProperties.previousDragDistance = null;

		this.sectionProperties.scrollBarThickness = 6 * app.roundedDpiScale;

		this.sectionProperties.drawScrollBarRailway = true;
		this.sectionProperties.scrollBarRailwayThickness = 6 * app.roundedDpiScale;
		this.sectionProperties.scrollBarRailwayAlpha = this.map._docLayer._docType === 'spreadsheet' ? 1.0 : 0.5;
		this.sectionProperties.scrollBarRailwayColor = '#EFEFEF';

		this.sectionProperties.drawVerticalScrollBar = ((<any>window).mode.isDesktop() ? true: false);
		this.sectionProperties.drawHorizontalScrollBar = ((<any>window).mode.isDesktop() ? true: false);

		this.sectionProperties.clickScrollVertical = false; // true when user presses on the scroll bar drawing.
		this.sectionProperties.clickScrollHorizontal = false;

		this.sectionProperties.mouseIsOnVerticalScrollBar = false;
		this.sectionProperties.mouseIsOnHorizontalScrollBar = false;

		this.sectionProperties.minimumScrollSize = 80 * app.roundedDpiScale;

		this.sectionProperties.circleSliderRadius = 24 * app.roundedDpiScale; // Radius of the mobile vertical circular slider.
		this.sectionProperties.arrowCornerLength = 10 * app.roundedDpiScale; // Corner length of the arrows inside circular slider.

		// Opacity.
		this.sectionProperties.alphaWhenVisible = 0.5; // Scroll bar is visible but not being used.
		this.sectionProperties.alphaWhenBeingUsed = 0.8; // Scroll bar is being used.
		this.sectionProperties.currentAlpha = 1.0; // This variable will be updated while animating. When not animating, this will be equal to one of the above variables.

		// Durations.
		this.sectionProperties.idleDuration = 2000; // In milliseconds. Scroll bar will be visible for this period of time after being used.
		this.sectionProperties.fadeOutStartingTime = 1800; // After this period, scroll bar starts to disappear. This duration is included in "idleDuration".
		this.sectionProperties.fadeOutDuration = this.sectionProperties.idleDuration - this.sectionProperties.fadeOutStartingTime;

		this.sectionProperties.animatingScroll = false;

		this.sectionProperties.animateWheelScroll = (<any>window).mode.isDesktop();
		this.sectionProperties.lastElapsedTime = 0;
		this.sectionProperties.scrollAnimationDelta = [0, 0];
		this.sectionProperties.scrollAnimationAcc = [0, 0];
		this.sectionProperties.scrollAnimationVelocity = [0, 0];
		this.sectionProperties.scrollAnimationDirection = [0, 0];
		this.sectionProperties.scrollAnimationDisableTimeout = null;
		this.sectionProperties.scrollWheelDelta = [0, 0];	// Used for non-animated scrolling

		this.sectionProperties.pointerSyncWithVerticalScrollBar = true;
		this.sectionProperties.pointerSyncWithHorizontalScrollBar = true;
		this.sectionProperties.pointerReCaptureSpacer = null; // Clicked point of the scroll bar.

		// Step by step scrolling interval in ms
		this.sectionProperties.stepDuration = 50;
		this.sectionProperties.quickScrollHorizontalTimer = null;

		// Chrome's mouse-wheel events are extremely inconsistent between platforms and they don't
		// use the deltaMode property, so we need to apply heuristics to determine when a wheel event
		// comes from a mouse-wheel or a touchpad.
		this.sectionProperties.scrollQuirks = true;

		this.sectionProperties.alwaysDrawVerticalScrollBar =
			(this.map._docLayer._docType === 'spreadsheet' &&
				!(<any>window).mode.isDesktop()) ||
			(app.map.getDocType() === 'text' && (<any>window).mode.isDesktop());
	}

	public completePendingScroll(): void {
		if (this.pendingScrollEvent) {
			this.onScrollTo(this.pendingScrollEvent);
			this.pendingScrollEvent = null;
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollTo (e: any): void {
		if (!this.containerObject.drawingAllowed()) {
			// Only remember the last scroll-to position.
			this.pendingScrollEvent = e;
			return;
		}
		// Triggered by the document (e.g. search result out of the viewing area).
		app.activeDocument.activeLayout.scrollTo(e.x, e.y);
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollBy (e: any): void {
		app.activeDocument.activeLayout.scroll(e.x, e.y);
	}

	public cancelAutoScroll(): void {
		if (this.autoScrollTimer !== null) {
			app.timerRegistry.clearInterval(this.autoScrollTimer);
			this.autoScrollTimer = null;
		}
		this.map.isAutoScrolling = false;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onScrollVelocity (e: any): void {
		if (e.vx === 0 && e.vy === 0) {
			this.cancelAutoScroll();
		} else {
			this.cancelAutoScroll();
			this.map.isAutoScrolling = true;
			this.autoScrollTimer = app.timerRegistry.setInterval('autoscroll', window.L.bind(function() {
				this.onScrollBy({x: e.vx, y: e.vy});
			}, this), 100);
		}
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	public onHandleAutoScroll (e: any): void {
		var vx = 0;
		var vy = 0;

		if (e.pos.y > e.map._size.y - 50) {
			vy = 50;
		} else if (e.pos.y < 50 && e.map._getTopLeftPoint().y > 50) {
			vy = -50;
		}

		const mousePosX: number = this.isRTL() ? e.map._size.x - e.pos.x : e.pos.x;
		const mapLeft: number = this.isRTL() ? e.map._size.x - e.map._getTopLeftPoint().x : e.map._getTopLeftPoint().x;
		if (mousePosX > e.map._size.x - 50) {
			vx = 50;
		} else if (mousePosX < 50 && mapLeft > 50) {
			vx = -50;
		}

		this.onScrollVelocity({ vx: vx, vy: vy, pos: e.pos });
	}

	public onUpdateScrollOffset (): void {
		if (this.map._docLayer._docType === 'spreadsheet') {
			this.map._docLayer.refreshViewData();
			this.map._docLayer._restrictDocumentSize();
		}
	}

	private DrawVerticalScrollBarMobile(): void {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;

		this.context.globalAlpha = this.sectionProperties.clickScrollVertical ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;

		this.context.strokeStyle = '#7E8182';
		this.context.fillStyle = 'white';

		var circleStartY = scrollProps.startY + this.sectionProperties.circleSliderRadius;
		var circleStartX = this.isRTL()
			? this.sectionProperties.circleSliderRadius * 0.5
			: this.size[0] - this.sectionProperties.circleSliderRadius * 0.5;

		this.context.beginPath();
		this.context.arc(circleStartX, circleStartY, this.sectionProperties.circleSliderRadius, 0, Math.PI * 2, true);
		this.context.fill();
		this.context.stroke();

		this.context.fillStyle = '#7E8182';
		this.context.beginPath();
		var x: number = circleStartX - this.sectionProperties.arrowCornerLength * 0.5;
		var y: number = circleStartY - 5 * app.roundedDpiScale;
		this.context.moveTo(x, y);
		x += this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y -= Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y += Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		this.context.fill();

		x = circleStartX - this.sectionProperties.arrowCornerLength * 0.5;
		y = circleStartY + 5 * app.roundedDpiScale;
		this.context.moveTo(x, y);
		x += this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y += Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		x -= this.sectionProperties.arrowCornerLength * 0.5;
		y -= Math.sin(Math.PI / 3) * this.sectionProperties.arrowCornerLength;
		this.context.lineTo(x, y);
		this.context.fill();

		this.context.globalAlpha = 1.0;
	}

	private drawVerticalScrollBar(): void {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;
		const isDarkBackground = this.map.uiManager.isBackgroundDark();
		const docType = app.map._docLayer._docType;

		var startX = this.isRTL() ? scrollProps.edgeOffset : this.size[0] - this.sectionProperties.scrollBarThickness - scrollProps.edgeOffset;

		if (isDarkBackground && (docType === 'text' || docType === 'drawing')) {
			this.sectionProperties.scrollBarRailwayColor = 'transparent';
		}

		if (this.sectionProperties.drawScrollBarRailway) {
			this.context.globalAlpha = this.sectionProperties.scrollBarRailwayAlpha;
			this.context.fillStyle = this.sectionProperties.scrollBarRailwayColor;
			this.context.fillRect(
				this.myTopLeft[0] + this.size[0] - scrollProps.edgeOffset - this.sectionProperties.scrollBarRailwayThickness,
				scrollProps.yOffset,
				this.sectionProperties.scrollBarRailwayThickness,
				scrollProps.verticalScrollLength
			);
		}

		this.context.globalAlpha = this.sectionProperties.clickScrollVertical ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;

		this.context.fillStyle = '#7E8182';


		this.context.fillRect(startX, scrollProps.startY, this.sectionProperties.scrollBarThickness, scrollProps.verticalScrollSize - this.sectionProperties.scrollBarThickness);

		this.context.globalAlpha = 1.0;

		if (this.containerObject.testing) {
			var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-vertical-scrollbar');
			if (!element) {
				element = document.createElement('div');
				element.id = 'test-div-vertical-scrollbar';
				document.body.appendChild(element);
			}
			element.textContent = String(scrollProps.startY);
			element.style.display = 'none';
			element.style.position = 'fixed';
			element.style.zIndex = '-1';
		}
	}

	private drawHorizontalScrollBar(): void {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;

		var startY = this.size[1] - this.sectionProperties.scrollBarThickness - scrollProps.edgeOffset;

		const sizeX = scrollProps.horizontalScrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number = app.sectionContainer.getWidth();
		const startX = this.isRTL() ? docWidth - scrollProps.startX - sizeX : scrollProps.startX;

		if (this.sectionProperties.drawScrollBarRailway) {
			this.context.globalAlpha = this.sectionProperties.scrollBarRailwayAlpha;
			this.context.fillStyle = this.sectionProperties.scrollBarRailwayColor;
			this.context.fillRect(
				scrollProps.xOffset,
				this.myTopLeft[1] + this.size[1] - scrollProps.edgeOffset - this.sectionProperties.scrollBarRailwayThickness,
				scrollProps.horizontalScrollLength,
				this.sectionProperties.scrollBarRailwayThickness
			);
		}

		this.context.globalAlpha = this.sectionProperties.clickScrollHorizontal ? this.sectionProperties.alphaWhenBeingUsed: this.sectionProperties.alphaWhenVisible;

		this.context.fillStyle = '#7E8182';

		this.context.fillRect(startX, startY, sizeX, this.sectionProperties.scrollBarThickness);

		this.context.globalAlpha = 1.0;

		if (this.containerObject.testing) {
			var element: HTMLDivElement = <HTMLDivElement>document.getElementById('test-div-horizontal-scrollbar');
			if (!element) {
				element = document.createElement('div');
				element.id = 'test-div-horizontal-scrollbar';
				document.body.appendChild(element);
			}
			element.textContent = String(scrollProps.startX);
			element.style.display = 'none';
			element.style.position = 'fixed';
			element.style.zIndex = '-1';
		}

	}

	private calculateCurrentAlpha(elapsedTime: number): void {
		if (elapsedTime >= this.sectionProperties.fadeOutStartingTime) {
			this.sectionProperties.currentAlpha = Math.max((1 - ((elapsedTime - this.sectionProperties.fadeOutStartingTime) / this.sectionProperties.fadeOutDuration)) * this.sectionProperties.alphaWhenVisible, 0.1);
		}
		else {
			this.sectionProperties.currentAlpha = this.sectionProperties.alphaWhenVisible;
		}
	}

	private doMove() {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;

		this.map.panBy(new cool.Point(scrollProps.moveBy[0] / app.dpiScale, scrollProps.moveBy[1] / app.dpiScale));
		scrollProps.moveBy = null;
		this.onUpdateScrollOffset();

		if (app && app.file.fileBasedView === true)
			app.map._docLayer._checkSelectedPart();

		app.activeDocument.activeLayout.refreshScrollProperties();
	}

	public onDraw(frameCount: number, elapsedTime: number): void {
		if (app.activeDocument.activeLayout.scrollProperties.moveBy !== null)
			this.doMove();
		else
			app.activeDocument.activeLayout.refreshScrollProperties();

		if (this.isAnimating && frameCount >= 0)
			this.calculateCurrentAlpha(elapsedTime);

		if ((this.sectionProperties.drawVerticalScrollBar || this.sectionProperties.alwaysDrawVerticalScrollBar)) {
			if ((<any>window).mode.isSmallScreenDevice())
				this.DrawVerticalScrollBarMobile();
			else
				this.drawVerticalScrollBar();
		}

		if (this.sectionProperties.drawHorizontalScrollBar) {
			this.drawHorizontalScrollBar();
		}
	}

	public onAnimate(frameCount: number, elapsedTime: number): void {
		const timeDelta = (elapsedTime - this.sectionProperties.lastElapsedTime) / (1000/60);
		if (this.sectionProperties.animatingScroll) {
			const lineHeight = this.containerObject.getScrollLineHeight();
			// Smoothness will be affected by Firefox bug #1967935
			// Note that we should really use geometric series to calculate acceleration when frames
			// are skipped, but given we expect consistent performance, this shouldn't make a
			// noticeable difference and isn't worth the added complication.
			const accel = lineHeight * ScrollSection.scrollAnimationAcceleration * timeDelta * app.dpiScale;

			// Calculate horizontal and vertical scroll deltas for this animation step
			const deltas = [0, 0];
			for (let i = 0; i < 2; ++i) {
				const sign = this.sectionProperties.scrollAnimationDirection[i];

				// Note for future implementers: if we wanted to accelerate the scroll distance over time,
				// we ought to multiply the scrollAnimationDelta here by some factor that increases with
				// elapsedTime.
				this.sectionProperties.scrollAnimationAcc[i] += this.sectionProperties.scrollAnimationDelta[i];
				this.sectionProperties.scrollAnimationDelta[i] = 0;

				this.sectionProperties.scrollAnimationVelocity[i] += this.sectionProperties.scrollAnimationAcc[i] != 0 ? accel : -accel;
				this.sectionProperties.scrollAnimationVelocity[i] = Math.max(0, this.sectionProperties.scrollAnimationVelocity[i]);

				deltas[i] = this.sectionProperties.scrollAnimationVelocity[i] * sign;

				if (Math.abs(deltas[i]) >= Math.abs(this.sectionProperties.scrollAnimationAcc[i]))
					this.sectionProperties.scrollAnimationAcc[i] = 0;
				else
					this.sectionProperties.scrollAnimationAcc[i] -= deltas[i];
			}

			// Perform scrolling, if necessary
			if (deltas[0] !== 0)
				this.scrollHorizontalWithOffset(deltas[0]);
			if (deltas[1] !== 0)
				this.scrollVerticalWithOffset(deltas[1]);
		} else {
			if (this.sectionProperties.scrollWheelDelta[0] !== 0) {
				const delta = this.sectionProperties.scrollWheelDelta[0];
				this.sectionProperties.scrollWheelDelta[0] = 0;
				this.scrollHorizontalWithOffset(delta);
			}
			if (this.sectionProperties.scrollWheelDelta[1] !== 0) {
				const delta = this.sectionProperties.scrollWheelDelta[1];
				this.sectionProperties.scrollWheelDelta[1] = 0;
				this.scrollVerticalWithOffset(delta);
			}
		}

		this.sectionProperties.lastElapsedTime = elapsedTime;

		const animatingScrollbar = elapsedTime && (elapsedTime < this.sectionProperties.fadeOutDuration);
		const animatingScroll = this.sectionProperties.animatingScroll
			&& (timeDelta <= 0 || this.sectionProperties.scrollAnimationVelocity.reduce((a: number, x: number) => a + x, 0) !== 0);
		if (!animatingScrollbar && !animatingScroll) this.containerObject.stopAnimating();
	}

	public onAnimationEnded(frameCount: number, elapsedTime: number): void {
		this.sectionProperties.animatingScroll = false;
		this.sectionProperties.scrollAnimationAcc = [0, 0];
		this.sectionProperties.scrollAnimationVelocity = [0, 0];
		this.sectionProperties.scrollAnimationDirection = [0, 0];
	}

	private increaseScrollBarThickness () : void {
		this.sectionProperties.scrollBarThickness = this.sectionProperties.scrollBarRailwayThickness = 8 * app.roundedDpiScale;
		this.containerObject.requestReDraw();
	}

	private decreaseScrollBarThickness () : void {
		this.sectionProperties.scrollBarThickness = this.sectionProperties.scrollBarRailwayThickness = 6 * app.roundedDpiScale;
		this.containerObject.requestReDraw();
	}

	private hideHorizontalScrollBar(): void {
		this.sectionProperties.drawHorizontalScrollBar = false;

		if (this.sectionProperties.mouseIsOnHorizontalScrollBar) {
			this.sectionProperties.mouseIsOnHorizontalScrollBar = false;
		}

		this.decreaseScrollBarThickness();
	}

	private hideVerticalScrollBar (): void {
		if (this.sectionProperties.mouseIsOnVerticalScrollBar) {
			this.sectionProperties.mouseIsOnVerticalScrollBar = false;
		}

		this.decreaseScrollBarThickness();

		if (!(<any>window).mode.isDesktop() || app.map._docLayer._docType !== 'spreadsheet') { // On desktop, we don't want to hide the vertical scroll bar.
			this.sectionProperties.drawVerticalScrollBar = false;
		}
	}

	private showHorizontalScrollBar (): void {
		this.sectionProperties.drawHorizontalScrollBar = true;

		// Prevent Instant Mouse hover
		setTimeout(() => {
			if (this.sectionProperties.mouseIsOnHorizontalScrollBar) {
				this.increaseScrollBarThickness();
			}
		}, 100);

		if (!this.containerObject.isDraggingSomething() && !(<any>window).mode.isDesktop())
			this.containerObject.requestReDraw();
	}

	private showVerticalScrollBar (): void {
		this.sectionProperties.drawVerticalScrollBar = true;

		// Prevent Instant Mouse hover
		setTimeout(() => {
			if (this.sectionProperties.mouseIsOnVerticalScrollBar) {
				this.increaseScrollBarThickness();
			}
		}, 100);

		if (!this.containerObject.isDraggingSomething() && !(<any>window).mode.isDesktop())
			this.containerObject.requestReDraw();
	}

	private setCursorForScrollBar(): void {
		this.context.canvas.style.cursor = 'pointer';
		if (this.context.canvas.classList.contains('spreadsheet-cursor'))
			this.context.canvas.classList.remove('spreadsheet-cursor');
	}

	private isMouseOnScrollBar (point: cool.SimplePoint): void {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;
		const documentAnchor: CanvasSectionObject = app.sectionContainer.getSectionWithName(app.CSections.Tiles.name);

		const mirrorX = this.isRTL();

		let temp = point.pY >= this.size[1] - scrollProps.usableThickness;
		temp = temp && ((!mirrorX && point.pX <= this.size[0] - scrollProps.horizontalScrollRightOffset && point.pX >= scrollProps.xOffset)
					|| (mirrorX && point.pX >= scrollProps.horizontalScrollRightOffset && point.pX >= scrollProps.xOffset));

		this.sectionProperties.mouseIsOnHorizontalScrollBar = temp;

		if (app.activeDocument.activeLayout.canScrollHorizontal(documentAnchor)) {
			if ((<any>window).mode.isDesktop() || this.sectionProperties.mouseIsOnHorizontalScrollBar)
				this.showHorizontalScrollBar();
			else if (this.sectionProperties.drawHorizontalScrollBar === true)
				this.hideHorizontalScrollBar();
		}
		else if (this.sectionProperties.drawHorizontalScrollBar === true) this.hideHorizontalScrollBar();

		temp = point.pX >= this.size[0] - scrollProps.usableThickness;
		temp = temp && (!mirrorX && point.pX >= this.size[0] - scrollProps.usableThickness) || (mirrorX && point.pX <= scrollProps.usableThickness);
		this.sectionProperties.mouseIsOnVerticalScrollBar = temp;

		if (app.activeDocument.activeLayout.canScrollVertical(documentAnchor)) {
			if ((<any>window).mode.isDesktop() || this.sectionProperties.mouseIsOnVerticalScrollBar)
				this.showVerticalScrollBar();
			else
				this.hideVerticalScrollBar();
		}
		else this.hideVerticalScrollBar();

		if (this.sectionProperties.mouseIsOnHorizontalScrollBar || this.sectionProperties.mouseIsOnVerticalScrollBar) {
			this.setCursorForScrollBar();
		}
	}

	public onMouseLeave (): void {
		this.hideVerticalScrollBar();
		this.hideHorizontalScrollBar();
	}

	public scrollVerticalWithOffset (offset: number): boolean {
		if (!app.activeDocument.activeLayout.canScrollVertical(app.sectionContainer.getSectionWithName(app.CSections.Tiles.name)))
			return;

		app.activeDocument.activeLayout.scroll(0, offset);

		if (app.file.fileBasedView) this.map._docLayer._checkSelectedPart();

		return true;
	}

	public scrollHorizontalWithOffset (offset: number): boolean {
		if (!app.activeDocument.activeLayout.canScrollHorizontal(app.sectionContainer.getSectionWithName(app.CSections.Tiles.name)))
			return;

		app.activeDocument.activeLayout.scroll(offset,0 );

		return true;
	}

	private isMouseInsideDocumentAnchor (point: cool.SimplePoint): boolean {
		var docSection = this.containerObject.getDocumentAnchorSection();
		return this.containerObject.doesSectionIncludePoint(docSection, point.pToArray());
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private isMousePointerSyncedWithVerticalScrollBar (scrollProps: any, position: cool.SimplePoint): boolean {
		// Keep this desktop-only for now.
		if (!(<any>window).mode.isDesktop())
			return true;

		var spacer = 0;
		if (!this.sectionProperties.pointerSyncWithVerticalScrollBar) {
			spacer = this.sectionProperties.pointerReCaptureSpacer;
		}

		var pointerIsSyncWithScrollBar = false;
		if (this.sectionProperties.pointerSyncWithVerticalScrollBar) {
			pointerIsSyncWithScrollBar = scrollProps.startY < position.pX && scrollProps.startY + scrollProps.scrollSize - this.sectionProperties.scrollBarThickness > position.pY;
			pointerIsSyncWithScrollBar = pointerIsSyncWithScrollBar || (this.isMouseInsideDocumentAnchor(position) && spacer === 0);
		}
		else {
			// See if the scroll bar is on top or bottom.
			var docAncSectionY = this.containerObject.getDocumentAnchorSection().myTopLeft[1];
			if (scrollProps.startY < 30 * window.app.roundedDpiScale + docAncSectionY) {
				pointerIsSyncWithScrollBar = scrollProps.startY + spacer < position.pY;
			}
			else {
				pointerIsSyncWithScrollBar = scrollProps.startY + spacer > position.pY;
			}
		}

		this.sectionProperties.pointerSyncWithVerticalScrollBar = pointerIsSyncWithScrollBar;
		return pointerIsSyncWithScrollBar;
	}

	// eslint-disable-next-line @typescript-eslint/explicit-module-boundary-types
	private isMousePointerSyncedWithHorizontalScrollBar (scrollProps: any, position: cool.SimplePoint): boolean {
		// Keep this desktop-only for now.
		if (!(<any>window).mode.isDesktop())
			return true;

		var spacer = 0;
		if (!this.sectionProperties.pointerSyncWithHorizontalScrollBar) {
			spacer = this.sectionProperties.pointerReCaptureSpacer;
		}

		const sizeX = scrollProps.scrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number =  app.sectionContainer.getWidth();
		const startX = this.isRTL() ? docWidth - scrollProps.startX - sizeX : scrollProps.startX;
		const endX = startX + sizeX;

		var pointerIsSyncWithScrollBar = false;
		if (this.sectionProperties.pointerSyncWithHorizontalScrollBar) {
			pointerIsSyncWithScrollBar = position.pX > startX && position.pX < endX;
			pointerIsSyncWithScrollBar = pointerIsSyncWithScrollBar || (this.isMouseInsideDocumentAnchor(position) && spacer === 0);
		}
		else {
			// See if the scroll bar is on left or right.
			var docAncSectionX = this.containerObject.getDocumentAnchorSection().myTopLeft[0];
			if (startX < 30 * window.app.roundedDpiScale + docAncSectionX) {
				pointerIsSyncWithScrollBar = startX + spacer < position.pX;
			}
			else {
				pointerIsSyncWithScrollBar = startX + spacer > position.pX;
			}
		}

		this.sectionProperties.pointerSyncWithHorizontalScrollBar = pointerIsSyncWithScrollBar;
		return pointerIsSyncWithScrollBar;
	}

	public onMouseMove (position: cool.SimplePoint, dragDistance: Array<number>, e: MouseEvent): void {
		const scrollProps: ScrollProperties = (app.activeDocument as DocumentBase).activeLayout.scrollProperties;

		this.clearQuickScrollTimeout();

		if (this.sectionProperties.clickScrollVertical && this.containerObject.isDraggingSomething()) {
			if (!this.sectionProperties.previousDragDistance) {
				this.sectionProperties.previousDragDistance = [0, 0];
			}

			this.showVerticalScrollBar();

			var diffY: number = dragDistance[1] - this.sectionProperties.previousDragDistance[1];

			if (this.isMousePointerSyncedWithVerticalScrollBar(scrollProps, position))
				this.scrollVerticalWithOffset(diffY * scrollProps.verticalScrollRatio);

			this.sectionProperties.previousDragDistance[1] = dragDistance[1];

			this.stopPropagating(); // Don't propagate to other sections.
		}
		else if (this.sectionProperties.clickScrollHorizontal && this.containerObject.isDraggingSomething()) {
			if (!this.sectionProperties.previousDragDistance) {
				this.sectionProperties.previousDragDistance = [0, 0];
			}

			this.showHorizontalScrollBar();

			var diffX: number = dragDistance[0] - this.sectionProperties.previousDragDistance[0];
			var actualDistance = scrollProps.horizontalScrollRatio * diffX;

			if (this.isMousePointerSyncedWithHorizontalScrollBar(scrollProps, position))
				this.scrollHorizontalWithOffset(actualDistance);

			this.sectionProperties.previousDragDistance[0] = dragDistance[0];
			this.stopPropagating(); // Don't propagate to other sections.
		}
		else {
			const draggingOnDocument = this.containerObject.isDraggingSomething();

			this.isMouseOnScrollBar(position);
			const onScrollBar = this.sectionProperties.mouseIsOnVerticalScrollBar || this.sectionProperties.mouseIsOnHorizontalScrollBar;

			// To stop propagation, user shouldn't be dragging something and mouse should be on a scroll bar.
			if (!draggingOnDocument && onScrollBar)
				this.stopPropagating(); // Don't propagate to other sections.
		}
	}

	/*
		When user presses the button while the mouse pointer is on the railway of the scroll bar but not on the scroll bar directly,
		we quickly scroll the document to that position.
	*/
	private quickScrollVertical (point: cool.SimplePoint, originalSign?: number): void {
		// Desktop only for now.
		if (!(<any>window).mode.isDesktop())
			return;

		window.L.DomUtil.addClass(document.documentElement, 'prevent-select');
		const scrollProps: ScrollProperties = app.activeDocument.activeLayout.scrollProperties;

		if (this.stepByStepScrolling) {
			var sign = (point.pY - (scrollProps.startY + scrollProps.verticalScrollSize)) > 0
				? 1 : ((point.pY - scrollProps.startY) < 0 ? -1 : 0);
			var offset = scrollProps.verticalScrollStep * sign;

			if (this.sectionProperties.quickScrollVerticalTimer)
				clearTimeout(this.sectionProperties.quickScrollVerticalTimer);
			if (this.sectionProperties.clickScrollVertical)
				this.sectionProperties.quickScrollVerticalTimer = setTimeout(() => {
					if (!originalSign || originalSign === sign) {
						this.quickScrollVertical(point, sign);
					}
				}, this.sectionProperties.stepDuration);
		} else {
			offset = (point.pY - (scrollProps.startY + scrollProps.verticalScrollSize * 0.5)) * scrollProps.verticalScrollRatio
		}

		this.scrollVerticalWithOffset(offset);
	}

	/*
		When user presses the button while the mouse pointer is on the railway of the scroll bar but not on the scroll bar directly,
		we quickly scroll the document to that position.
	*/
	private quickScrollHorizontal (point: cool.SimplePoint, originalSign?: number): void {
		// Desktop only for now.
		if (!(<any>window).mode.isDesktop())
			return;

		window.L.DomUtil.addClass(document.documentElement, 'prevent-select');
		const scrollProps: ScrollProperties = app.activeDocument.activeLayout.scrollProperties;
		const sizeX = scrollProps.horizontalScrollSize - this.sectionProperties.scrollBarThickness;
		const docWidth: number = app.sectionContainer.getWidth();
		const startX = this.isRTL() ? docWidth - scrollProps.startX - sizeX : scrollProps.startX;

		if (this.stepByStepScrolling) {
			var sign = (point.pX - (startX + sizeX)) > 0
				? 1 : ((point.pX - startX) < 0 ? -1 : 0);
			var offset = scrollProps.horizontalScrollStep * sign;

			if (this.sectionProperties.quickScrollHorizontalTimer)
				clearTimeout(this.sectionProperties.quickScrollHorizontalTimer);
			if (this.sectionProperties.clickScrollHorizontal)
				this.sectionProperties.quickScrollHorizontalTimer = setTimeout(() => {
					if (!originalSign || originalSign === sign) {
						this.quickScrollHorizontal(point, sign);
					}
				}, this.sectionProperties.stepDuration);
		} else {
			offset = (point.pX - (scrollProps.startX + scrollProps.horizontalScrollSize * 0.5)) * scrollProps.horizontalScrollRatio;
		}

		this.scrollHorizontalWithOffset(offset);
	}

	private getLocalYOnVerticalScrollBar (point: cool.SimplePoint): number {
		return point.pY - app.activeDocument.activeLayout.scrollProperties.startY;
	}

	private getLocalXOnHorizontalScrollBar (point: cool.SimplePoint): number {
		return point.pX - app.activeDocument.activeLayout.scrollProperties.startX;
	}

	private clearQuickScrollTimeout() {
		if (this.sectionProperties.quickScrollVerticalTimer) {
			clearTimeout(this.sectionProperties.quickScrollVerticalTimer);
			this.sectionProperties.quickScrollVerticalTimer = null;
		}
		if (this.sectionProperties.quickScrollHorizontalTimer) {
			clearTimeout(this.sectionProperties.quickScrollHorizontalTimer);
			this.sectionProperties.quickScrollHorizontalTimer = null;
		}
	}

	public onMouseDown (point: cool.SimplePoint, e: MouseEvent): void {
		const layout = (app.activeDocument as DocumentBase).activeLayout;
		const scrollProps: ScrollProperties = layout.scrollProperties;

		this.clearQuickScrollTimeout();
		this.onMouseMove(point, null, e);
		this.isMouseOnScrollBar(point);

		const mirrorX = this.isRTL();
		const documentAnchor = app.sectionContainer.getSectionWithName(app.CSections.Tiles.name);

		// For CompareChanges view, viewedRectangle.pY1 can be negative while scrolling is possible.
		const initialVerticalCheck = layout.type === 'ViewLayoutCompareChanges' ? true : layout.viewedRectangle.pY1 >= 0;

		if (initialVerticalCheck) {
			if ((!mirrorX && point.pX >= this.size[0] - scrollProps.usableThickness)
				|| (mirrorX && point.pY <= scrollProps.usableThickness)) {
				if (point.pY > scrollProps.yOffset) {
					this.sectionProperties.clickScrollVertical = true;
					this.map.scrollingIsHandled = true;
					this.quickScrollVertical(point);
					this.sectionProperties.pointerReCaptureSpacer = this.getLocalYOnVerticalScrollBar(point);
					e.stopPropagation(); // Don't propagate to map.
					this.stopPropagating(); // Don't propagate to bound sections.
				}
				else {
					this.sectionProperties.clickScrollVertical = false;
				}
			}
			else {
				this.sectionProperties.clickScrollVertical = false;
			}
		}

		// Same reasoning as vertical: CompareChanges can have negative pX1.
		const canScrollH = layout.type === 'ViewLayoutCompareChanges'
			? layout.canScrollHorizontal(documentAnchor)
			: layout.viewedRectangle.pX1 >= 0;

		if (canScrollH) {
			if (point.pY >= this.size[1] - scrollProps.usableThickness) {
				if ((!mirrorX && point.pX >= scrollProps.xOffset && point.pX <= this.size[0] - scrollProps.horizontalScrollRightOffset)
					|| (mirrorX && point.pX >= scrollProps.xOffset && point.pX >= scrollProps.horizontalScrollRightOffset)) {
					this.sectionProperties.clickScrollHorizontal = true;
					this.map.scrollingIsHandled = true;
					this.quickScrollHorizontal(point);
					this.sectionProperties.pointerReCaptureSpacer = this.getLocalXOnHorizontalScrollBar(point);
					e.stopPropagation(); // Don't propagate to map.
					this.stopPropagating(); // Don't propagate to bound sections.
				}
				else {
					this.sectionProperties.clickScrollHorizontal = false;
				}
			}
			else {
				this.sectionProperties.clickScrollHorizontal = false;
			}
		}
	}

	public onMouseUp (point: cool.SimplePoint, e: MouseEvent): void {
		window.L.DomUtil.removeClass(document.documentElement, 'prevent-select');
		this.map.scrollingIsHandled = false;
		this.clearQuickScrollTimeout();

		if (this.sectionProperties.clickScrollVertical) {
			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
			this.sectionProperties.clickScrollVertical = false;
			this.sectionProperties.pointerSyncWithVerticalScrollBar = true; // Default.
		}
		else if (this.sectionProperties.clickScrollHorizontal) {
			e.stopPropagation(); // Don't propagate to map.
			this.stopPropagating(); // Don't propagate to bound sections.
			this.sectionProperties.clickScrollHorizontal = false;
			this.sectionProperties.pointerSyncWithHorizontalScrollBar = true; // Default.
		}

		this.sectionProperties.previousDragDistance = null;
		this.onMouseMove(point, null, e);
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.isAnimating && this.sectionProperties.animatingWheelScrollVertical)
			this.containerObject.stopAnimating();
	}

	private animateScroll(delta: [number, number]): void {
		const lineHeight = this.containerObject.getScrollLineHeight();

		for (let i = 0; i < 2; ++i) {
			if (Math.abs(delta[i]) === 0) continue;

			const sign = delta[i] > 0 ? 1 : -1;
			if (sign !== this.sectionProperties.scrollAnimationDirection[i]) {
				// Stop animation on scroll change direction
				this.sectionProperties.scrollAnimationVelocity[i] = 0;
				this.sectionProperties.scrollAnimationAcc[i] = 0;
				this.sectionProperties.scrollAnimationDirection[i] = sign;
			}

			this.sectionProperties.scrollAnimationDelta[i] =
				lineHeight * ScrollSection.scrollWheelDelta * sign * app.dpiScale;
		}

		if (!this.sectionProperties.animatingScroll) {
			this.sectionProperties.animatingScroll = true;
			this.sectionProperties.lastElapsedTime = 0;
			// We're about to start a duration-less animation, so we need to
			// ensure the animation is reset.
			if (!this.startAnimating({ 'defer': true })) this.resetAnimation();
		}
	}

	public onMouseWheel (point: cool.SimplePoint, delta: Array<number>, e: WheelEvent): void {
		if (e.ctrlKey) {
			e.preventDefault();
			e.stopImmediatePropagation();
			this.stopPropagating();
			app.map.scrollHandler._onWheelScroll(e);
			return;
		}

		this.map.fire('closepopups'); // close all popups when scrolling

		let hscroll = 0, vscroll = 0;
		if (Math.abs(delta[1]) > Math.abs(delta[0])) {
			if (e.shiftKey)
				hscroll = delta[1];
			else
				vscroll = delta[1];
		} else
			hscroll = delta[0];

		let shouldAnimate = this.sectionProperties.animateWheelScroll
			&& !this.sectionProperties.scrollAnimationDisableTimeout;

		// We don't want to animate in the case of touchpad events. There is no
		// completely browser/OS-agnostic way of determining if a wheel event was
		// generated by a touchpad or a mouse-wheel.
		if (shouldAnimate) {
			// Firefox sends line scroll events for the mouse-wheel and pixel events
			// for the touch-pad. If we receive a non-pixel mousewheel scroll, we know
			// that we can rely on this and disable other heuristics that may cause
			// false-positives.
			if (e.deltaMode !== WheelEvent.DOM_DELTA_PIXEL) {
				this.sectionProperties.scrollQuirks = false;
				// Some touchpads with bad drivers generate mousewheel events. In those cases, the
				// line height will be much smaller and we can treat them as a janky touchpad. Not
				// doing so otherwise makes scrolling difficult to control.
				if (Math.abs((e as any).wheelDeltaY) <= 32 && Math.abs((e as any).wheelDeltaX) <= 32)
					shouldAnimate = false;
			} else if (!this.sectionProperties.scrollQuirks)
				shouldAnimate = false;

			if (e.deltaX !== 0 && e.deltaY !== 0) {
				// It's not a mouse-wheel if both components are non-zero. I suppose it's
				// theoretically possible to scroll in both directions at once with a wheel,
				// but very difficult.
				shouldAnimate = false;
			} else if (this.sectionProperties.scrollQuirks) {
				const nowIsAccurate = performance.now() % 1 !== 0;
				const hasFractionalComponent = (e.deltaX % 1 !== 0) || (e.deltaY % 1 !== 0);
				const deltaMaybeDiscrete = Math.abs((e as any).wheelDelta) % 60 === 0;

				if (hasFractionalComponent && (!nowIsAccurate || !deltaMaybeDiscrete)) {
					// Firefox touchpad deltas always seem to have a fractional
					// component on Linux, but this is also true of wheel events for
					// Chrome on Mac.
					// To distinguish Firefox from Chrome, we can use the fact that
					// Firefox performance.now() is rounded to a whole number and that
					// the wheelDelta on Mac will be discrete.
					shouldAnimate = false;
				} else if (nowIsAccurate && !deltaMaybeDiscrete) {
					// In Chrome, performance.now can (and usually does) have a
					// fractional component. We can use this to single it out, then
					// check if the delta is discrete. This would indicate the event
					// was generated by a mouse-wheel.
					shouldAnimate = false;
				}
			}
		}

		hscroll *= app.dpiScale;
		vscroll *= app.dpiScale;

		if (shouldAnimate)
			this.animateScroll([hscroll, vscroll]);
		else {
			this.sectionProperties.animatingScroll = false;

			if (this.sectionProperties.scrollAnimationDisableTimeout)
				clearTimeout(this.sectionProperties.scrollAnimationDisableTimeout);
			this.sectionProperties.scrollAnimationDisableTimeout =
				setTimeout(() => { this.sectionProperties.scrollAnimationDisableTimeout = null; },
					ScrollSection.scrollDirectTimeoutMs);

			this.sectionProperties.scrollWheelDelta[0] += hscroll;
			this.sectionProperties.scrollWheelDelta[1] += vscroll;

			if (!this.isAnimating) this.startAnimating({});
		}
	}
}

}
