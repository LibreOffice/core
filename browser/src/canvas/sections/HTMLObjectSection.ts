// @ts-strict-ignore
/* global Proxy _ */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

class HTMLObjectSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.HTMLObject.processingOrder;
	drawingOrder: number = app.CSections.HTMLObject.drawingOrder;
	zIndex: number = app.CSections.HTMLObject.zIndex;
	documentObject: boolean = true;
	htmlPosition: Array<number> = [0, 0];
	pendingUpdate: TaskId | null = null;

	constructor (sectionName: string, objectWidth: number, objectHeight: number, documentPosition: cool.SimplePoint, extraClass: string = "", showSection: boolean = true) {
        super(sectionName);

		this.size = [objectWidth * app.dpiScale, objectHeight * app.dpiScale];
		this.position = [documentPosition.pX, documentPosition.pY];
		this.sectionProperties.objectWidth = objectWidth;
		this.sectionProperties.objectHeight = objectHeight;
		this.sectionProperties.objectDiv = document.createElement('div');
		this.sectionProperties.objectDiv.className = 'html-object-section';

		if (objectWidth === null) this.sectionProperties.objectDiv.style.width = 'auto';
		else this.sectionProperties.objectDiv.style.width = objectWidth + 'px';

		if (objectHeight === null) this.sectionProperties.objectDiv.style.height = 'auto';
		else this.sectionProperties.objectDiv.style.height = objectHeight + 'px';

		if (extraClass)
			this.sectionProperties.objectDiv.className += ' ' + extraClass;

		// canvas-container and canvas overlap entirely. We can append the html object to canvas-container.
		document.getElementById('canvas-container').appendChild(this.sectionProperties.objectDiv);

		if (!showSection) {
			this.sectionProperties.objectDiv.style.display = 'none';
			this.showSection = false;
		}

		// This one is important for canvas section container. This property lets the events pass through the object and find canvas.
		this.sectionProperties.objectDiv.style.pointerEvents = 'none';
	}

	onInitialize(): void {
		this.setPosition(this.position[0], this.position[1]);
		this.adjustHTMLObjectPosition();
	}

	public onSectionShowStatusChange(): void {
		if (this.showSection)
			this.sectionProperties.objectDiv.style.display = '';
		else
			this.sectionProperties.objectDiv.style.display = 'none';
	}

	adjustHTMLObjectPosition() {
		const leftNumber = Math.round(this.myTopLeft[0] / app.dpiScale);
		const topNumber = Math.round(this.myTopLeft[1] / app.dpiScale);

		// setup model data now and schedule DOM update during animation frame
		this.htmlPosition = [leftNumber, topNumber];

		if (this.pendingUpdate) app.layoutingService.cancelLayoutingTask(this.pendingUpdate);

		this.pendingUpdate = app.layoutingService.appendLayoutingTask(() => {
			this.pendingUpdate = null;

			const left = this.htmlPosition[0] + 'px';
			const top = this.htmlPosition[1] + 'px';

			if (this.sectionProperties.objectDiv.style.left !== left)
				this.sectionProperties.objectDiv.style.left = left;

			if (this.sectionProperties.objectDiv.style.top !== top)
				this.sectionProperties.objectDiv.style.top = top;
		});
	}

	onDraw(frameCount?: number, elapsedTime?: number): void {
		this.adjustHTMLObjectPosition();
	}

	public getHTMLObject() {
		return this.sectionProperties.objectDiv;
	}

	public onNewDocumentTopLeft(): void {
		this.adjustHTMLObjectPosition();

		if (this.isVisible && this.isSectionShown()) {
			if (this.sectionProperties.objectDiv.style.display !== '')
				this.sectionProperties.objectDiv.style.display = '';
		}
		else
			this.sectionProperties.objectDiv.style.display = 'none';
	}

	public getPosition(): cool.SimplePoint {
		return this.documentPosition.clone();
	}

	public onRemove(): void {
		this.sectionProperties.objectDiv.remove();
	}
}
