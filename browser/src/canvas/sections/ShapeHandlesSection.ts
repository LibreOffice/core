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

/*
	This class is for the modifier handles of shape objects.
	Shape is rendered on the core side. Only the handles are drawn here and modification commands are sent to the core side.
*/

class HelperLineStyles {
	static initiated: false;
	static gridSolidStyle: string = null;
	static gridDashedStyle: string = null;
	static smartGuidesStyle: string = null;
	static darkModeListenerAdded = false;

	private static addListener() {
		app.map.on('darkmodechanged', function() {
			this.gridSolidStyle = null;
			this.gridDashedStyle = null;
			this.smartGuidesStyle = null;
		}.bind(this));

		this.darkModeListenerAdded = true;
	}

	public static initiate() {
		if (!this.darkModeListenerAdded) this.addListener();

		const tempElement = document.createElement('div');

		tempElement.style.display = 'none';
		document.body.appendChild(tempElement);

		tempElement.style.color = 'var(--color-grid-helper-line-solid)';
		let computedStyle = window.getComputedStyle(tempElement);
		this.gridSolidStyle = computedStyle.color;

		tempElement.style.color = 'var(--color-grid-helper-line-dashed)';
		computedStyle = window.getComputedStyle(tempElement);
		this.gridDashedStyle = computedStyle.color;

		tempElement.style.color = 'var(--color-smart-guides-helper-line)';
		computedStyle = window.getComputedStyle(tempElement);
		this.smartGuidesStyle = computedStyle.color;

		tempElement.remove();
	}
}

class ShapeHandlesSection extends CanvasSectionObject {
	processingOrder: number = app.CSections.ShapeHandlesSection.processingOrder;
	drawingOrder: number = app.CSections.ShapeHandlesSection.drawingOrder;
	zIndex: number = app.CSections.ShapeHandlesSection.zIndex;
	documentObject: boolean = true;
	showSection: boolean = false;

	/// How wide and how high a handle is drawn, in core pixels.
	public static handleSize(): number {
		return HandleLook.widthOf('framing');
	}

	/// How wide a point that a connector can be tied to is drawn, in core pixels.
	public static gluePointSize(): number {
		return HandleLook.widthOf('tiePoint');
	}

	/// What every handle of the selection is, in the order they are drawn.
	public handleInfos(): any[] {
		return this.sectionProperties.handles.map((handle: any) => handle.info);
	}

	constructor (info: any) {
		super(app.CSections.ShapeHandlesSection.name);

		this.sectionProperties.info = null;
		this.sectionProperties.handles = [];
		this.sectionProperties.subSections = [];
		this.sectionProperties.activeHandleIndex = null;
		this.sectionProperties.handleWidth = ShapeHandlesSection.handleSize();
		this.sectionProperties.handleHeight = ShapeHandlesSection.handleSize();
		this.sectionProperties.anchorWidth = 20 * app.dpiScale;
		this.sectionProperties.anchorHeight = 20 * app.dpiScale;
		this.sectionProperties.rotationHandleWidth = 15 * app.dpiScale;
		this.sectionProperties.rotationHandleHeight = 15 * app.dpiScale;
		this.sectionProperties.gluePointRadius = ShapeHandlesSection.gluePointSize();
		this.sectionProperties.subSectionPrefix = 'shape-handle-';
		this.sectionProperties.svg = null; // This is for preview of modifications.
		this.sectionProperties.hasVideo = false; // Don't hide svg when there is video content.
		this.sectionProperties.shapeRectangleProperties = null; // Not null when there are scaling handles.
		this.sectionProperties.inPlaceResizeRectangle = null;
		this.sectionProperties.lastDragDistance = [0, 0];
		// What the drag could line up with on the page, and the lines that mark it.
		this.sectionProperties.snap = new SelectionSnap();
		this.sectionProperties.mathObjectBorderColor = 'red'; // Border color for Math objects.
		this.sectionProperties.lastTapTime = 0;
		this.sectionProperties.clickTimer = null;
		this.sectionProperties.viewedRectangleOnMouseDown = new cool.SimpleRectangle(0, 0, 0, 0);
		this.sectionProperties.initialPosition = this.position.slice();
		this.sectionProperties.positionOnMouseDown = new cool.SimplePoint(0, 0);

		// Drag preview polyline sent by core while a handle is dragged.
		// Array of polygons, each an array of [x, y] in core pixels.
		this.sectionProperties.shapeDragPreviewPolygons = null;
		this.sectionProperties.shapeDragPreviewRequestTime = null;
		this.sectionProperties.queuedShapeDragPreview = null;

		// These are for snapping the objects to the same level with others' boundaries.




		this.refreshInfo(info);

		if (HelperLineStyles.gridDashedStyle === null) HelperLineStyles.initiate();
	}

	public refreshInfo(info: any) {
		this.sectionProperties.info = info;
		this.convertToTileTwipsIfNeeded();
		this.getHandles();
		this.updateSize();

		if (GraphicSelection?.extraInfo?.isMathObject === true) {
			// Math objects don't need handles. They are not resizable or rotateable.
			// In this case, we want to draw a rectangle around it. So the user can be sure that they selected the object.

			// For drawing the rectangle, use CanvasSectionContainer's awesome border drawing feature.
			this.borderColor = this.sectionProperties.mathObjectBorderColor;
		} else {
			this.addSubSections();
			this.sectionProperties.shapeRectangleProperties = this.getShapeRectangleProperties();
			this.calculateInitialAnglesOfShapeHandlers();
			this.borderColor = null;
		}
	}

	private convertToTileTwipsIfNeeded() {
		if (app.map._docLayer._docType !== 'spreadsheet' || !app.map._docLayer.sheetGeometry)
			return;

		const kinds = this.sectionProperties.info?.handles?.kinds;
		if (!kinds) return;

		if (this.sectionProperties.info.handlesInTileTwips) return;
		this.sectionProperties.info.handlesInTileTwips = true;

		// Core can send handle X coordinates as negative print twips in RTL
		// (its internal sign convention), so normalize to positive first.
		// Handles arrive coincident with the selection rectangle sent next to
		// them, so they take the same translation the rectangle got.
		const offset = GraphicSelection.tileTwipsOffset;
		const normalize = (entry: any): void => {
			entry.point.x = Math.abs(parseInt(entry.point.x)) + offset[0];
			entry.point.y = parseInt(entry.point.y) + offset[1];
		};

		// Core groups handles into rectangle / poly / custom / anchor / others
		// (see SdrMarkView in svdmrkv.cxx). Convert positions for every group,
		// not just rectangle, otherwise custom-shape adjustment handles (kind
		// 22) and others render in print twips while the shape is in tile
		// twips, leaving them visually detached from the shape.
		for (const group of Object.values(kinds) as Record<string, any[]>[]) {
			if (!group) continue;
			for (const handles of Object.values(group)) {
				if (!Array.isArray(handles)) continue;
				for (const handle of handles) {
					handle.point.x = Math.abs(parseInt(handle.point.x)) + offset[0];
					handle.point.y = parseInt(handle.point.y) + offset[1];
				}
			}
		}

		if (Array.isArray(kinds.custom?.['22'])) {
			for (const entry of kinds.custom['22']) normalize(entry);
		}
	}

	public calculateInitialAnglesOfShapeHandlers(shapeRecProps?: any) {
		if (this.sectionProperties.info?.handles?.kinds?.rectangle) {
			if (!shapeRecProps)
				shapeRecProps = this.sectionProperties.shapeRectangleProperties;

			const halfDiagonal = Math.pow(Math.pow(shapeRecProps.width * 0.5, 2) + Math.pow(shapeRecProps.height * 0.5, 2), 0.5);
			// In Calc RTL, convertToTileTwipsIfNeeded's abs flips the handle
			// doc-X: core's internal topLeft (min internal X) becomes the
			// shape's doc-max-X (= visual top-left on the mirrored canvas).
			// So kind '1' is at (+w/2, +h/2) relative to center, not (-w/2, +h/2).
			// Flip the X factor for every non-vertical handle.
			const xSign = app.map._docLayer.isCalcRTL() ? 1 : -1;
			for (let i = 0; i < this.sectionProperties.subSections.length; i++) {
				const subSection = this.sectionProperties.subSections[i];

				if (subSection.sectionProperties.ownInfo.kind === '1') {
					subSection.sectionProperties.distanceToCenter = halfDiagonal;
					subSection.sectionProperties.initialAngle = Math.atan2(shapeRecProps.height * 0.5, xSign * shapeRecProps.width * 0.5);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '2') {
					subSection.sectionProperties.distanceToCenter = shapeRecProps.height * 0.5;
					subSection.sectionProperties.initialAngle = Math.atan2(shapeRecProps.height * 0.5, 0);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '3') {
					subSection.sectionProperties.distanceToCenter = halfDiagonal;
					subSection.sectionProperties.initialAngle = Math.atan2(shapeRecProps.height * 0.5, -xSign * shapeRecProps.width * 0.5);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '4') {
					subSection.sectionProperties.distanceToCenter = shapeRecProps.width * 0.5;
					subSection.sectionProperties.initialAngle = Math.atan2(0, xSign * shapeRecProps.width * 0.5);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '5') {
					subSection.sectionProperties.distanceToCenter = shapeRecProps.width * 0.5;
					subSection.sectionProperties.initialAngle = Math.atan2(0, -xSign * shapeRecProps.width * 0.5);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '6') {
					subSection.sectionProperties.distanceToCenter = halfDiagonal;
					subSection.sectionProperties.initialAngle = Math.atan2(-shapeRecProps.height * 0.5, xSign * shapeRecProps.width * 0.5);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '7') {
					subSection.sectionProperties.distanceToCenter = shapeRecProps.height * 0.5;
					subSection.sectionProperties.initialAngle = Math.atan2(-shapeRecProps.height * 0.5, 0);
				}
				else if (subSection.sectionProperties.ownInfo.kind === '8') {
					subSection.sectionProperties.distanceToCenter = halfDiagonal;
					subSection.sectionProperties.initialAngle = Math.atan2(-shapeRecProps.height * 0.5, -xSign * shapeRecProps.width * 0.5);
				}
			}
		}
	}

	getShapeWidth(twips = true) {
		let middleLeft = this.sectionProperties.info.handles.kinds.rectangle['4'][0];
		middleLeft = new cool.SimplePoint(parseInt(middleLeft.point.x), parseInt(middleLeft.point.y));

		let middleRight = this.sectionProperties.info.handles.kinds.rectangle['5'][0];
		middleRight = new cool.SimplePoint(parseInt(middleRight.point.x), parseInt(middleRight.point.y));

		if (twips)
			return Math.abs(middleLeft.distanceTo(middleRight.toArray()));
		else
			return Math.abs(middleLeft.pDistanceTo(middleRight.pToArray()));
	}

	getShapeHeight(twips = true) {
		let topMiddle = this.sectionProperties.info.handles.kinds.rectangle['2'][0];
		topMiddle = new cool.SimplePoint(parseInt(topMiddle.point.x), parseInt(topMiddle.point.y));

		let bottomMiddle = this.sectionProperties.info.handles.kinds.rectangle['7'][0];
		bottomMiddle = new cool.SimplePoint(parseInt(bottomMiddle.point.x), parseInt(bottomMiddle.point.y));

		if (twips)
			return Math.abs(topMiddle.distanceTo(bottomMiddle.toArray()));
		else
			return Math.abs(topMiddle.pDistanceTo(bottomMiddle.pToArray()))
	}

	/*
		This is also sent from the core side.
	*/
	getShapeAngleRadians() {
		let topMiddle = this.sectionProperties.info.handles.kinds.rectangle['2'][0];
		topMiddle = new cool.SimplePoint(parseInt(topMiddle.point.x), parseInt(topMiddle.point.y));

		const center = this.getShapeCenter();

		const radians = Math.atan2((center.y - topMiddle.y), (topMiddle.x - center.x));
		return radians - Math.PI * 0.5;
	}

	private getShapeCenter() {
		let topLeft = this.sectionProperties.info.handles.kinds.rectangle['1'][0];
		topLeft = new cool.SimplePoint(parseInt(topLeft.point.x), parseInt(topLeft.point.y));

		let bottomRight = this.sectionProperties.info.handles.kinds.rectangle['8'][0];
		bottomRight = new cool.SimplePoint(parseInt(bottomRight.point.x), parseInt(bottomRight.point.y));

		const center = new cool.SimplePoint((topLeft.x + bottomRight.x) * 0.5, (topLeft.y + bottomRight.y) * 0.5);

		return center;
	}

	/*
		Selection rectangle is different from the object's inner rectangle.
		Handlers are positioned based on object's inner rectangle (~borders). So we need to get the object's inner rectangle and its rotation angle.
	*/
	private getShapeRectangleProperties() {
		if (!this.sectionProperties.info.handles?.kinds?.rectangle)
			return null;

		return {
			angleRadian: this.getShapeAngleRadians(),
			center: this.getShapeCenter(),
			height: this.getShapeHeight(false),
			width: this.getShapeWidth(false)
		};
	}

	private getScalingHandles(halfWidth: number, halfHeight: number) {
		if (this.sectionProperties.info?.handles?.kinds?.rectangle) {
			const topLeft = this.sectionProperties.info.handles.kinds.rectangle['1'][0];
			const topMiddle = this.sectionProperties.info.handles.kinds.rectangle['2'][0];
			const topRight = this.sectionProperties.info.handles.kinds.rectangle['3'][0];
			const middleLeft = this.sectionProperties.info.handles.kinds.rectangle['4'][0];
			const middleRight = this.sectionProperties.info.handles.kinds.rectangle['5'][0];
			const bottomLeft = this.sectionProperties.info.handles.kinds.rectangle['6'][0];
			const bottomMiddle = this.sectionProperties.info.handles.kinds.rectangle['7'][0];
			const bottomRight = this.sectionProperties.info.handles.kinds.rectangle['8'][0];

			this.sectionProperties.handles.push({ info: topLeft, point: new cool.SimplePoint(topLeft.point.x - halfWidth, topLeft.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: topMiddle, point: new cool.SimplePoint(topMiddle.point.x - halfWidth, topMiddle.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: topRight, point: new cool.SimplePoint(topRight.point.x - halfWidth, topRight.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: middleLeft, point: new cool.SimplePoint(middleLeft.point.x - halfWidth, middleLeft.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: middleRight, point: new cool.SimplePoint(middleRight.point.x - halfWidth, middleRight.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: bottomLeft, point: new cool.SimplePoint(bottomLeft.point.x - halfWidth, bottomLeft.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: bottomMiddle, point: new cool.SimplePoint(bottomMiddle.point.x - halfWidth, bottomMiddle.point.y - halfHeight) });
			this.sectionProperties.handles.push({ info: bottomRight, point: new cool.SimplePoint(bottomRight.point.x - halfWidth, bottomRight.point.y - halfHeight) });
		}
	}

	private getAnchorHandle(halfWidth: number, halfHeight: number) {
		if (this.sectionProperties.info?.handles?.kinds?.anchor) {
			const anchor = this.sectionProperties.info.handles.kinds.anchor['16'][0];
			this.sectionProperties.handles.push({ info: anchor, point: new cool.SimplePoint(anchor.point.x - halfWidth, anchor.point.y - halfHeight) });
		}
	}

	public getRotationInfo(): any {
		if (!this.sectionProperties.info?.handles?.kinds?.rectangle)
			return;

		let coreAngle = GraphicSelection.selectionAngle;
		if (this.sectionProperties.svg)
			coreAngle = this.sectionProperties.svg.innerHTML.includes('class="Group"') ? 0: coreAngle;

		if (coreAngle !== undefined && coreAngle !== null) {
			coreAngle = coreAngle / 100;
			coreAngle = (coreAngle * Math.PI) / 180;

			while (coreAngle > Math.PI * 2) coreAngle -= Math.PI * 2;
			while (coreAngle < 0) coreAngle += Math.PI * 2;
		}
		else
			coreAngle = null;

		const result = {
			kind: 'ShapeRotationHandle',
			coreAngle: coreAngle
		};

		return result;
	}

	getRotationHandlePosition(rotationInfo: any) {
		const centerX = parseInt(this.sectionProperties.info.handles.kinds.rectangle['2'][0].point.x) * app.twipsToPixels - this.position[0];
		const centerY = parseInt(this.sectionProperties.info.handles.kinds.rectangle['2'][0].point.y) * app.twipsToPixels - this.position[1];
		const diff = 30 * app.dpiScale;

		let y = centerY - diff * Math.sin(rotationInfo.coreAngle + Math.PI * 0.5);
		let x = centerX + diff * Math.cos(rotationInfo.coreAngle + Math.PI * 0.5);

		x -= this.sectionProperties.rotationHandleWidth * 0.5;
		y -= this.sectionProperties.rotationHandleHeight * 0.5;

		return new cool.SimplePoint((this.position[0] + x) * app.pixelsToTwips, (this.position[1] + y) * app.pixelsToTwips);
	}

	private getRotationHandle() {
		if (
			this.sectionProperties.info?.handles?.kinds?.rectangle &&
			GraphicSelection.extraInfo?.isRotatable !== false &&
			!this.sectionProperties.hasVideo
		) {
			const rotationInfo = this.getRotationInfo(); // Rotation section will read the information from this (parent) class.
			const rotationHandlePosition: cool.SimplePoint = this.getRotationHandlePosition(rotationInfo);
			rotationInfo.initialPosition = rotationHandlePosition.clone();

			// Core side doesn't send a position information for rotation handle. We add this.
			this.sectionProperties.handles.push({ info: rotationInfo, point: rotationHandlePosition });
		}
	}

	private getCustomHandles(halfWidth: number, halfHeight: number) {
		if (this.sectionProperties.info?.handles?.kinds?.custom) {
			const customHandleList = this.sectionProperties.info.handles.kinds.custom['22'];

			if (customHandleList && customHandleList.length > 0) {
				for (let i = 0; i < customHandleList.length; i++) {
					const customHandler = customHandleList[i];
					this.sectionProperties.handles.push({ info: customHandler, point: new cool.SimplePoint(customHandler.point.x - halfWidth, customHandler.point.y - halfHeight) });
				}
			}
		}
	}

	private getPolyHandles(halfWidth: number, halfHeight: number) {
		if (this.sectionProperties.info?.handles?.kinds?.poly) {
			if (Array.isArray(this.sectionProperties.info.handles.kinds.poly['9'])) {
				const polyArray = this.sectionProperties.info.handles.kinds.poly['9'];

				for (let i = 0; i < polyArray.length; i++) {
					const poly = polyArray[i];
					this.sectionProperties.handles.push({ info: poly, point: new cool.SimplePoint(poly.point.x - halfWidth, poly.point.y - halfHeight) });
				}
			}
		}
	}

	private getGluePoints() {
		if (this.sectionProperties.info?.GluePoints?.shapes) {
			if (Array.isArray(this.sectionProperties.info.GluePoints.shapes)) {
				const shapeArray = this.sectionProperties.info.GluePoints.shapes;

				for (let i = 0; i < shapeArray.length; i++) {
					const shape = shapeArray[i];
					shape.kind = 'GluePoint';
					const glueArray = shape.gluepoints;

					for (let j = 0; j < glueArray.length; j++) {
						const info = Object.assign({}, shape);
						info.id = String(i) + String(j);
						this.sectionProperties.handles.push({ info: info, point: new cool.SimplePoint(glueArray[j].point.x, glueArray[j].point.y) });
					}
				}
			}
		}
	}

	private getDiagram(halfWidth: number, halfHeight: number) {
		if (this.isDiagram() && this.sectionProperties.info?.handles?.kinds?.rectangle) {
			// get a scaled oversize measurement in X and Y
			const scaleFactor = 3;
			halfWidth *= scaleFactor;
			halfHeight *= scaleFactor;

			// get object size as base for calculations
			const topLeft = this.sectionProperties.info.handles.kinds.rectangle['1'][0];

			// create a shape with overhang of halfWidth/Height on all sides,
			const width : number = this.getShapeWidth(true) + 2 * halfWidth;
			const height : number = this.getShapeHeight(true) + 2 * halfHeight;
			const info = {
				kind: 'DiagramHandle',
				size: new cool.SimplePoint(width, height),
				halfWidth: halfWidth,
				halfHeight: halfHeight
			};
			this.sectionProperties.handles.push({
				info: info,
				point: new cool.SimplePoint(
					topLeft.point.x - info.halfWidth,
					topLeft.point.y - info.halfHeight) });
		}
	}

	// Get the handle positions and other information from the info that core side sent us.
	private getHandles() {
		this.sectionProperties.handles = [];

		const halfWidth = app.pixelsToTwips * (this.sectionProperties.handleWidth * 0.5);
		const halfHeight = app.pixelsToTwips * (this.sectionProperties.handleHeight * 0.5);

		this.getDiagram(halfWidth, halfHeight);
		this.getScalingHandles(halfWidth, halfHeight);
		this.getAnchorHandle(halfWidth, halfHeight);
		this.getRotationHandle();
		this.getCustomHandles(halfWidth, halfHeight);
		this.getPolyHandles(halfWidth, halfHeight);
		this.getGluePoints();
	}

	// Update this section's size according to handle coordinates.
	updateSize() {
		this.size = [0, 0];

		if (GraphicSelection.hasActiveSelection())
			this.size = [GraphicSelection.rectangle.pWidth, GraphicSelection.rectangle.pHeight];

		this.boundingRectangle = cool.SimpleRectangle.fromCorePixels([...this.position, ...this.size]);

		if (this.sectionProperties.svg)
			this.adjustSVGProperties();
	}

	isSVGVisible() {
		if (this.sectionProperties.svg)
			return this.sectionProperties.svg.style.display === '';
		else
			return false;
	}

	isDiagram() {
		if (GraphicSelection?.extraInfo?.isDiagram === true)
			return true;
		else
			return false;
	}

	removeSVG() {
		if (this.sectionProperties.svg)
			this.sectionProperties.svg.remove();
	}

	static addVideoSupportHandlers(videos: any) {
		if (!videos)
			return;

		// slide show may have more than one video and it does not require any selection
		for (var i = 0; i < videos.length; i++) {
			var video = videos[i];
			var sources = video.getElementsByTagName('source');

			video.addEventListener('playing', function() {
				window.setTimeout(function() {
					if (video.webkitDecodedFrameCount === 0 && video.webkitAudioDecodedByteCount === 0) {
						ShapeHandlesSection.showUnsupportedMediaWarning();
					}
				}, 1000);
			});

			video.addEventListener('error', function() {
				ShapeHandlesSection.showUnsupportedMediaWarning();
			});

			if (sources.length) {
				sources[0].addEventListener('error', function(error: string) {
					ShapeHandlesSection.showUnsupportedMediaWarning();
				});
			}
		}
	}

	static showUnsupportedMediaWarning() {
		var videoWarning = _('Document contains unsupported media');
		window.L.Map.THIS.uiManager.showSnackbar(videoWarning);
	}

	addEmbeddedVideo(svgString: any) {
		this.sectionProperties.hasVideo = true;
		this.setSVG(svgString);
		this.sectionProperties.svg.style.opacity = 0.5;
		this.sectionProperties.svg.remove();
		document.getElementById('canvas-container').appendChild(this.sectionProperties.svg);
		this.sectionProperties.svg.style.zIndex = 11; // Update z-index or video buttons are unreachable.

		if (!app.map.isEditMode()) {
			this.sectionProperties.svg.style.opacity = 1;
			this.sectionProperties.svg.style.pointerEvents = '';
			this.hideSubSections();
		}

		if (!this.sectionProperties.svg.innerHTML.includes('foreignObject')) {
			console.error('Failed to parse svg for embedded video');
			return;
		}

		var videoContainer = this.sectionProperties.svg;
		var videos = this.sectionProperties.svg.getElementsByTagName('video');

		// fix URL, it's important to have correct WOPISrc, we need to decode "&" before other params
		// like ServerId and Tag so load balancer will not use it as a part of WOPISrc
		// this has to be done here (after parseSVG), because it other case we will fail to get
		// the svg object
		var source = this.sectionProperties.svg.getElementsByTagName('source');
		source[0].src = decodeURIComponent(source[0].src);

		ShapeHandlesSection.addVideoSupportHandlers(videos);
	}

	removeTagFromHTML(data: string, startString: string, endString: string): string {
		let startIndex = data.indexOf(startString);
		let endIndex = data.indexOf(endString, startIndex + 1);

		while (startIndex !== -1 && endIndex !== -1) {
			const toRemove = data.substring(startIndex, endIndex + endString.length);
			data = data.replace(toRemove, '');

			startIndex = data.indexOf(startString);
			endIndex = data.indexOf(endString, startIndex + 1);
		}

		return data;
	}

	getTagFromHTML(data: string, startString: string, endString: string): string {
		const startIndex = data.indexOf(startString);
		const endIndex = data.indexOf(endString, startIndex + 1);

		if (startIndex !== -1 && endIndex !== -1) {
			const pickedData = data.substring(startIndex, endIndex + endString.length);
			return pickedData;
		}
		else {
			return '';
		}
	}

	setSVG(data: string) {
		this.removeSVG();

		data = this.removeTagFromHTML(data, ' style="', '"');

		this.sectionProperties.svg = document.createElement('svg');
		this.sectionProperties.svg.style.pointerEvents = 'none';
		document.getElementById('canvas-container').appendChild(this.sectionProperties.svg);

		// eslint-disable-next-line no-restricted-syntax -- sanitized SVG markup
		this.sectionProperties.svg.innerHTML = app.LOUtil.sanitize(data, 'svg');
		this.sectionProperties.svg.style.position = 'absolute';
		this.sectionProperties.svg.children[0].style.width = this.sectionProperties.svg.children[0].style.height = 'auto';
		this.sectionProperties.svg.children[0].style.transformOrigin = 'center';
		this.sectionProperties.svg.children[0].setAttribute('preserveAspectRatio', 'none');

		this.adjustSVGProperties();
	}

	showSVG() {
		// A view that draws from objects shows what a drag would do by drawing the objects
		// themselves, so the picture the engine renders stays away.
		if (RenderManager.isVectorRendering()) return;

		if (this.sectionProperties.svg)
			this.sectionProperties.svg.style.display = '';
	}

	hideSVG() {
		if (this.sectionProperties.svg && !this.sectionProperties.hasVideo)
			this.sectionProperties.svg.style.display = 'none';
	}

	onSectionShowStatusChange() {
		for (let i = 0; i < this.sectionProperties.subSections.length; i++)
			this.sectionProperties.subSections[i].setShowSection(this.showSection);

		// Don't call showSVG() here: the SVG overlay may contain stale
		// content from a previous render.  A new shapeselectioncontent
		// message will arrive and setSVG() will set the up-to-date SVG.
		// The overlay is only made visible on demand (e.g. during drag).
		if (!this.showSection)
			this.hideSVG();
	}

	checkAnchorSubSection(handle: any): any {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + handle.info.id);

		if (!newSubSection) {
			newSubSection = new ShapeHandleAnchorSubSection(
				this,
				this.sectionProperties.subSectionPrefix + handle.info.id,
				[this.sectionProperties.anchorWidth / app.dpiScale, this.sectionProperties.anchorHeight / app.dpiScale],
				handle.point.clone(),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			return null;
		}
	}

	checkScalingSubSection(handle: any): any {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + handle.info.id);

		if (!newSubSection) {
			newSubSection = new ShapeHandleScalingSubSection(
				this,
				this.sectionProperties.subSectionPrefix + handle.info.id,
				[this.sectionProperties.handleWidth, this.sectionProperties.handleHeight],
				handle.point.clone(),
				handle.info,
				GraphicSelection.extraInfo.isCropMode
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			if (GraphicSelection.extraInfo.isCropMode && !newSubSection.sectionProperties.cropModeEnabled) {
				newSubSection.sectionProperties.cropModeEnabled = GraphicSelection.extraInfo.isCropMode;
			}
			return null;
		}
	}

	checkRotationSubSection(handle: any) {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + 'rotation');

		if (!newSubSection) {
			newSubSection = new app.definitions.shapeHandleRotationSubSection(
				this,
				this.sectionProperties.subSectionPrefix + 'rotation',
				[this.sectionProperties.rotationHandleWidth, this.sectionProperties.rotationHandleHeight],
				handle.point.clone(),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			return null;
		}
	}

	checkDiagramSubSection(handle: any) {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + 'diagram');

		if (!newSubSection) {
			newSubSection = new app.definitions.shapeHandleDiagramSubSection(
				this,
				this.sectionProperties.subSectionPrefix + 'diagram',
				[handle.info.size.pX, handle.info.size.pY],
				new cool.SimplePoint(
					handle.point.x,
					handle.point.y),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(
				handle.point.pX,
				handle.point.pY);
			newSubSection.setSize(
				handle.info.size.pX,
				handle.info.size.pY);

			// we also need to update the edit button by resetting it's
			// last remembered zoom value
			if (GraphicSelection.diagramButton)
				GraphicSelection.diagramButton.forceNextReposition();

			return null;
		}
	}

	checkCustomSubSection(handle: any): any {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + handle.info.id);

		if (!newSubSection) {
			newSubSection = new app.definitions.shapeHandleCustomSubSection(
				this,
				this.sectionProperties.subSectionPrefix + handle.info.id,
				[this.sectionProperties.handleWidth, this.sectionProperties.handleHeight],
				handle.point.clone(),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			return null;
		}
	}

	checkPolySubSection(handle: any): any {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + handle.info.id);

		if (!newSubSection) {
			newSubSection = new app.definitions.shapeHandlePolySubSection(
				this,
				this.sectionProperties.subSectionPrefix + handle.info.id,
				[this.sectionProperties.handleWidth, this.sectionProperties.handleHeight],
				handle.point.clone(),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			return null;
		}
	}

	checkGluePointSubSection(handle: any): any {
		let newSubSection = app.sectionContainer.getSectionWithName(this.sectionProperties.subSectionPrefix + handle.info.id);

		if (!newSubSection) {
			newSubSection = new app.definitions.shapeHandleGluePointSubSection(
				this,
				this.sectionProperties.subSectionPrefix + handle.info.id,
				[this.sectionProperties.gluePointRadius, this.sectionProperties.gluePointRadius],
				handle.point.clone(),
				handle.info
			);
			return newSubSection;
		}
		else {
			newSubSection.sectionProperties.ownInfo = handle.info;
			newSubSection.setPosition(handle.point.pX, handle.point.pY);
			return null;
		}
	}

	/*
		Whether a handle of that kind gets a section of its own. A view that draws from objects
		draws the handles that frame and shape the selection in one section of its own, which
		knows them as a list, and draws the band round a group or a diagram there as well, so only
		the anchor is left here. Cropping an image is done with the whole set of handles here, so
		while it runs they are all made as they always were.
	*/
	private handleGetsItsOwnSection(kind: string): boolean {
		if (!RenderManager.isVectorRendering()) return true;
		if (GraphicSelection.extraInfo?.isCropMode) return true;

		return kind === '16';
	}

	addSubSections() {
		for (let i = 0; i < this.sectionProperties.handles.length; i++) {
			if (!this.handleGetsItsOwnSection(this.sectionProperties.handles[i].info.kind))
				continue;

			let newSubSection: any = null;
			if (this.sectionProperties.handles[i].info.kind === '16')
				newSubSection = this.checkAnchorSubSection(this.sectionProperties.handles[i]);
			else if (['1', '2', '3', '4', '5', '6', '7', '8'].includes(this.sectionProperties.handles[i].info.kind))
				newSubSection = this.checkScalingSubSection(this.sectionProperties.handles[i]);
			else if (this.sectionProperties.handles[i].info.kind === 'ShapeRotationHandle')
				newSubSection = this.checkRotationSubSection(this.sectionProperties.handles[i]);
			else if (this.sectionProperties.handles[i].info.kind === 'DiagramHandle')
				newSubSection = this.checkDiagramSubSection(this.sectionProperties.handles[i]);
			else if (['11', '22'].includes(this.sectionProperties.handles[i].info.kind))
				newSubSection = this.checkCustomSubSection(this.sectionProperties.handles[i]);
			else if (this.sectionProperties.handles[i].info.kind === '9')
				newSubSection = this.checkPolySubSection(this.sectionProperties.handles[i]);
			else if (this.sectionProperties.handles[i].info.kind === 'GluePoint')
				newSubSection = this.checkGluePointSubSection(this.sectionProperties.handles[i]);

			if (newSubSection) {
				this.containerObject.addSection(newSubSection as any);
				this.sectionProperties.subSections.push(newSubSection);
			}
		}
	}

	onMouseEnter() {
		this.context.canvas.style.cursor =
			GraphicSelection.extraInfo?.isDraggable === false ? 'not-allowed' : 'move';
	}

	/// Where the upper left corner of the selection lands, in core pixels, on the axis given.
	adjustSnapTransformCoordinate(x: number, y: number) {
		const corner = this.sectionProperties.snap.corner(x, y);
		return x !== null ? corner[0] : corner[1];
	}

	sendTransformCommand(point: cool.SimplePoint) {
		const corner = this.sectionProperties.snap.corner(
			this.sectionProperties.lastDragDistance[0] + this.position[0],
			this.sectionProperties.lastDragDistance[1] + this.position[1],
		);

		let x = corner[0];
		const y = corner[1];

		if (GraphicSelection.inPlaceActive) {
			GraphicSelection.sendInPlaceArea(
				cool.SimpleRectangle.fromCorePixels([
					x,
					y,
					this.size[0],
					this.size[1],
				]),
				true,
			);
			app.map._docLayer.requestNewFiledBasedViewTiles();
			this.sectionProperties.lastDragDistance = [0, 0];
			return;
		}

		const docLayer = app.map._docLayer;

		if (docLayer.isCalcRTL())
			x = -this.size[0] + this.sectionProperties.lastDragDistance[0] - this.position[0];

		let xTwips = Math.round(x * app.pixelsToTwips);
		let yTwips = Math.round(y * app.pixelsToTwips);

		const verticalOffset = docLayer.getFiledBasedViewVerticalOffset();
		if (verticalOffset) {
			// Transform from canvas twips to core twips.
			yTwips -= verticalOffset;
		}

		if (docLayer._docType === 'spreadsheet' && GraphicSelection.inPlaceArea) {
			// what is inside an open object is shown where the object is
			const sign = xTwips < 0 ? -1 : 1;
			xTwips = sign * (Math.abs(xTwips) - GraphicSelection.inPlaceTileOffset[0]);
			yTwips -= GraphicSelection.inPlaceTileOffset[1];
		}
		else if (docLayer._docType === 'spreadsheet' && docLayer.sheetGeometry) {
			// we keep the shape in tile twips, core reads these as print twips
			const sign = xTwips < 0 ? -1 : 1;
			const printTwips = docLayer.sheetGeometry.getPrintTwipsPointFromTile(
				new cool.Point(Math.abs(xTwips), yTwips));
			xTwips = sign * Math.round(printTwips.x);
			yTwips = Math.round(printTwips.y);
		}

		const parameters = {
			'TransformPosX': {
				'type': 'long',
				'value': xTwips
			},
			'TransformPosY': {
				'type': 'long',
				'value': yTwips
			}
		};

		app.map.sendUnoCommand('.uno:TransformDialog', parameters);

		docLayer.requestNewFiledBasedViewTiles();

		this.sectionProperties.lastDragDistance = [0, 0];
	}

	/*
		Ask core for the geometry the shape would get if the dragged handle
		was dropped at the given position. Core answers with a
		shapedragpreview message. At most one request is in flight, newer
		positions replace the queued one until the answer arrives.
	*/
	public requestShapeDragPreview(handleInfo: any, point: cool.SimplePoint) {
		const requestTime = this.sectionProperties.shapeDragPreviewRequestTime;
		if (requestTime !== null && Date.now() - requestTime < 250) {
			this.sectionProperties.queuedShapeDragPreview = {
				handleInfo: handleInfo,
				point: point,
			};
			return;
		}

		this.sendShapeDragPreviewRequest(handleInfo, point);
	}

	/*
		How a handle is named to the engine: by what it is, where the client worked the handles out
		for itself, and otherwise by the place the engine gave it in its own list.
	*/
	public static handleParameters(handleInfo: any): any {
		if (handleInfo?.name)
			return { HandleName: { type: 'string', value: String(handleInfo.name) } };

		return { HandleNum: { type: 'long', value: handleInfo?.id } };
	}

	private sendShapeDragPreviewRequest(handleInfo: any, point: cool.SimplePoint) {
		this.sectionProperties.shapeDragPreviewRequestTime = Date.now();
		this.sectionProperties.queuedShapeDragPreview = null;

		const parameters = {
			...ShapeHandlesSection.handleParameters(handleInfo),
			NewPosX: { type: 'long', value: Math.round(point.x) },
			NewPosY: { type: 'long', value: Math.round(point.y) },
			Preview: { type: 'boolean', value: true }
		};

		app.map.sendUnoCommand('.uno:MoveShapeHandle', parameters);
	}

	public onShapeDragPreview(preview: any) {
		this.sectionProperties.shapeDragPreviewRequestTime = null;

		if (!this.containerObject.isDraggingSomething()) {
			// The drag has ended in the meantime, this preview is stale.
			this.clearShapeDragPreview();
			return;
		}

		const polygons: number[][][] = [];
		if (Array.isArray(preview?.polygons)) {
			for (const polygonText of preview.polygons) {
				const points: number[][] = [];
				for (const pointText of String(polygonText).split(' ')) {
					const coordinates = pointText.split(',');
					if (coordinates.length !== 2) continue;
					points.push([
						parseInt(coordinates[0]) * app.twipsToPixels,
						parseInt(coordinates[1]) * app.twipsToPixels
					]);
				}
				if (points.length > 1) polygons.push(points);
			}
		}

		this.sectionProperties.shapeDragPreviewPolygons = polygons.length > 0 ? polygons : null;
		this.containerObject.requestReDraw();

		const queued = this.sectionProperties.queuedShapeDragPreview;
		if (queued) this.sendShapeDragPreviewRequest(queued.handleInfo, queued.point);
	}

	/*
		The drag is off, so what was shown of it goes: the copy of the shape that followed the
		mouse, the outline the engine answered with, and the lines it was snapping to. The shape
		itself never moved, the engine breaks its own drag on the same key.
	*/
	/// Forgets what the last look for a snap found, so the lines that marked it are gone.
	public forgetTheSnap(): void {
		this.sectionProperties.snap.forget();
	}

	/// Where the last look for a snap landed, in core pixels, null on an axis that found none.
	public snappedAt(): (number | null)[] {
		return this.sectionProperties.snap.at();
	}

	onDragCancel(): void {
		this.sectionProperties.lastDragDistance = [0, 0];
		this.forgetTheSnap();

		this.hideSVG();
		this.clearShapeDragPreview();
	}

	public clearShapeDragPreview() {
		this.sectionProperties.shapeDragPreviewPolygons = null;
		this.sectionProperties.shapeDragPreviewRequestTime = null;
		this.sectionProperties.queuedShapeDragPreview = null;
		this.containerObject.requestReDraw();
	}

	private drawShapeDragPreview() {
		const polygons = this.sectionProperties.shapeDragPreviewPolygons;
		if (!polygons) return;

		this.context.save();
		this.context.strokeStyle = app.map.uiManager.isBackgroundDark() ? 'white' : 'black';
		this.context.lineWidth = app.dpiScale;
		this.context.beginPath();

		for (let i = 0; i < polygons.length; i++) {
			const points = polygons[i];
			this.context.moveTo(points[0][0] - this.position[0], points[0][1] - this.position[1]);
			for (let j = 1; j < points.length; j++)
				this.context.lineTo(points[j][0] - this.position[0], points[j][1] - this.position[1]);
		}

		this.context.stroke();
		this.context.restore();
	}

	public onMouseDown(point: cool.SimplePoint, e: MouseEvent): void {
		Util.ensureValue(app.activeDocument);

		this.sectionProperties.viewedRectangleOnMouseDown = app.activeDocument.activeLayout.viewedRectangle.clone();
		this.sectionProperties.initialPosition = this.position.slice();
		this.sectionProperties.positionOnMouseDown = point.clone();
		this.sectionProperties.positionOnMouseDown.pX += this.position[0];
		this.sectionProperties.positionOnMouseDown.pY += this.position[1];
	}

	onMouseUp(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.sectionProperties.svg)
			this.sectionProperties.svg.style.opacity = 1;

		this.hideSVG();

		if (this.containerObject.isDraggingSomething()) {
			app.map.fire('scrollvelocity', { vx: 0, vy: 0 });

			// A view that draws from objects carries the selection in the section that draws the
			// handles, which works the move out itself. Cropping an image still runs here.
			if (
				RenderManager.isVectorRendering() &&
				GraphicSelection.extraInfo?.isCropMode !== true
			)
				return;

			point.x += app.activeDocument.activeLayout.viewedRectangle.x1 - this.sectionProperties.viewedRectangleOnMouseDown.x1;
			point.y += app.activeDocument.activeLayout.viewedRectangle.y1 - this.sectionProperties.viewedRectangleOnMouseDown.y1;
			this.sendTransformCommand(point);
		}
	}

	/// Looks for what the drag could line up with, and marks it.
	public checkHelperLinesAndSnapPoints(size: number[], position: number[], dragDistance: number[]) {
		this.sectionProperties.snap.look(size, position, dragDistance);
		this.containerObject.requestReDraw();
	}

	private constrainDragToSheetArea(dragDistance: number[]) {
		if (app.map.getDocType() === 'spreadsheet') {
			// Cap the drag distance to the document boundaries
			const calcGridSection = app.sectionContainer.getSectionWithName(
				app.CSections.CalcGrid.name,
			);
			const rowHeaderSection = app.sectionContainer.getSectionWithName(
				app.CSections.RowHeader.name,
			) as cool.RowHeader;
			const columnHeaderSection = app.sectionContainer.getSectionWithName(
				app.CSections.ColumnHeader.name,
			) as cool.ColumnHeader;

			if (calcGridSection && rowHeaderSection && columnHeaderSection) {
				dragDistance[0] = Math.max(
					calcGridSection.myTopLeft[0] -
						this.myTopLeft[0] -
						columnHeaderSection.getHeaderInfo().getDocVisStart(),
					dragDistance[0],
				);
				dragDistance[1] = Math.max(
					calcGridSection.myTopLeft[1] -
						this.myTopLeft[1] -
						rowHeaderSection.getHeaderInfo().getDocVisStart(),
					dragDistance[1],
				);
				// Clip svg at the header edges
				if (this.sectionProperties.svg) {
					const cropLeft =
						Math.max(
							calcGridSection.myTopLeft[0] -
								this.myTopLeft[0] -
								dragDistance[0],
							0,
						) / app.dpiScale;
					const cropTop =
						Math.max(
							calcGridSection.myTopLeft[1] -
								this.myTopLeft[1] -
								dragDistance[1],
							0,
						) / app.dpiScale;
					this.sectionProperties.svg.style.clipPath = `inset(${cropTop}px 0px 0px ${cropLeft}px)`;
				}
			}
		}
	}

	onMouseMove(position: cool.SimplePoint, dragDistance: number[]) {
		let canDrag = (GraphicSelection.inPlaceActive || !app.file.textCursor.visible) && GraphicSelection.extraInfo?.isDraggable !== false;

		if (canDrag && app.map.getDocType() === 'presentation') {
			// Tables get selected when multiple cells are selected. In this case, we check if DeleteRows is disabled.
			// Because in non-edit mode, deleteRows is disabled. So we can drag the table.
			const deleteRowsState = app.map.stateChangeHandler.getItemValue('.uno:DeleteRows');
			canDrag = deleteRowsState ? deleteRowsState === 'disabled': true;
		}

		// A view that draws from objects carries the selection in the section that draws the
		// handles, so a drag that lands here does nothing at all. Cropping an image still runs
		// here, and so does everything while the document is drawn from tiles.
		if (
			RenderManager.isVectorRendering() &&
			GraphicSelection.extraInfo?.isCropMode !== true
		)
			return;

		if (this.containerObject.isDraggingSomething() && canDrag) {
			this.constrainDragToSheetArea(dragDistance);

			if (!app.activeDocument.activeLayout.viewedRectangle.equals(this.sectionProperties.viewedRectangleOnMouseDown.toArray())) {
				const diff = new cool.SimplePoint(
					app.activeDocument.activeLayout.viewedRectangle.x1 - this.sectionProperties.viewedRectangleOnMouseDown.x1,
					app.activeDocument.activeLayout.viewedRectangle.y1 - this.sectionProperties.viewedRectangleOnMouseDown.y1
				);

				this.setPosition(this.sectionProperties.initialPosition[0] + diff.pX, this.sectionProperties.initialPosition[1] + diff.pY);
			}

			if (this.sectionProperties.svg) {
				const addition = app.map._docLayer.isCalcRTL() ? -this.size[0] : 0;
				this.sectionProperties.svg.style.left = String((this.myTopLeft[0] + dragDistance[0] + addition) / app.dpiScale) + 'px';
				this.sectionProperties.svg.style.top = String((this.myTopLeft[1] + dragDistance[1]) / app.dpiScale) + 'px';
				this.sectionProperties.svg.style.opacity = 0.5;
			}
			this.sectionProperties.lastDragDistance = [dragDistance[0], dragDistance[1]];
			this.checkHelperLinesAndSnapPoints(this.size, this.position, dragDistance);

			this.showSVG();

			if (!this.containerObject.isMouseInside()) {
				position.pX += this.myTopLeft[0];
				position.pY += this.myTopLeft[1];
				app.map.fire('handleautoscroll', { pos: { x: position.cX, y: position.cY }, map: app.map });
			}
			else app.map.fire('scrollvelocity', { vx: 0, vy: 0 });
		}
	}

	getViewBox(svg: any): number[] {
		let viewBox: any = svg.getAttribute('viewBox');

		if (viewBox) {
			viewBox = viewBox.split(' ');
			for (let i = 0; i < viewBox.length; i++) viewBox[i] = parseInt(viewBox[i]);
		}
		else
			viewBox = null;

		return viewBox;
	}

	adjustSVGProperties() {
		if (this.sectionProperties.svg && this.sectionProperties.svg.style.display === '' && GraphicSelection.hasActiveSelection()) {
			const widthText = GraphicSelection.rectangle.cWidth + 'px';
			const heightText = GraphicSelection.rectangle.cHeight + 'px';

			const viewBox: number[] = this.getViewBox(this.sectionProperties.svg.children[0]);
			const isImage = this.sectionProperties.svg.querySelectorAll('.Graphic').length > 0;

			const clientRect = (this.sectionProperties.svg.children[0] as SVGElement).getBoundingClientRect();

			if (viewBox && !isImage && clientRect.width > 0 && clientRect.height > 0) {
				this.sectionProperties.svg.children[0].style.width = widthText;
				this.sectionProperties.svg.children[0].style.height = heightText;
			}
			else {
				this.sectionProperties.svg.style.width = widthText;
				this.sectionProperties.svg.style.height = heightText;

				if (isImage) {
					this.sectionProperties.svg.children[0].style.width = widthText;
					this.sectionProperties.svg.children[0].style.height = heightText;
				}
			}

			// New view layouts are not tested for Calc yet. Transition to new structure is not complete.
			this.updateSVGPosition();
		}
		this.hideSVG();
	}

	// Shared positioning logic used by both adjustSVGProperties (initial
	// placement) and onNewDocumentTopLeft (viewport change update).
	private updateSVGPosition(): void {
		if (!this.sectionProperties.svg || !GraphicSelection.hasActiveSelection()) return;

		const left = GraphicSelection.rectangle.v1X;
		const top = GraphicSelection.rectangle.v1Y;

		this.sectionProperties.svg.style.left = Math.round(left / app.dpiScale) + 'px';
		this.sectionProperties.svg.style.top = Math.round(top / app.dpiScale) + 'px';
		this.sectionProperties.svgPosition = [left, top];
	}

	onNewDocumentTopLeft(): void {
		if (this.sectionProperties.svgPosition) {
			this.updateSVGPosition();
		}
	}

	private drawSelectionFrame() {
		this.context.save();
		this.context.setTransform(1, 0, 0, 1, 0, 0);

		this.context.beginPath();
		this.context.strokeStyle = app.map.uiManager.isBackgroundDark() ? 'white' : 'black';
		this.context.setLineDash([3, 3]);

		if (this.containerObject.isDraggingSomething() && this.containerObject.targetSection === this.name) {
			const rectangle = this.boundingRectangle.clone();
			rectangle.pX1 += app.calc.isRTL() ? -this.sectionProperties.lastDragDistance[0] : this.sectionProperties.lastDragDistance[0];
			rectangle.pY1 += this.sectionProperties.lastDragDistance[1];
			this.drawViewRectangle(rectangle);
		}
		else if (this.containerObject.isDraggingSomething() && this.sectionProperties.inPlaceResizeRectangle)
			this.drawViewRectangle(this.sectionProperties.inPlaceResizeRectangle);
		else
			this.drawViewRectangle(this.boundingRectangle);

		this.context.setLineDash([]);
		this.context.closePath();

		this.context.restore();
	}

	public onDraw() {
		// A view that draws from objects draws the line around the selection in the section that
		// draws its handles, where it follows everything that is done to the selection.
		if (!RenderManager.isVectorRendering()) this.drawSelectionFrame();
		this.drawShapeDragPreview();
		if (!this.showSection || !this.isVisible)
			this.hideSVG();
		else this.sectionProperties.snap.draw(this);
	}

	/*
		Shows what a scaling drag would do to the shape, by taking the picture of it to the size
		and the place the drag leads to. Does nothing while there is no picture to take.
	*/
	public scalePreviewTo(shapeRecProps: any): void {
		const svg = this.sectionProperties.svg;
		if (!svg) return;

		const shape = this.sectionProperties.shapeRectangleProperties;
		let scaleX = shapeRecProps.width / shape.width;
		let scaleY = shapeRecProps.height / shape.height;

		if (GraphicSelection.extraInfo?.typeString === 'ole2') {
			scaleX = Math.abs(scaleX);
			scaleY = Math.abs(scaleY);
		}

		const diffX = (shapeRecProps.center.pX - shape.center.pX) / app.dpiScale;
		const diffY = (shapeRecProps.center.pY - shape.center.pY) / app.dpiScale;

		svg.children[0].style.transform = 'translate(' + Math.round(diffX) + 'px, ' + Math.round(diffY) + 'px)' + 'rotate(' + -shapeRecProps.angleRadian + 'rad) scale(' + scaleX + ', ' + scaleY + ') rotate(' + shapeRecProps.angleRadian + 'rad)';

		this.showSVG();
	}

	removeSubSections(): void {
		this.removeSVG();
		for (let i = 0; i < this.sectionProperties.subSections.length; i++) {
			this.containerObject.removeSection(this.sectionProperties.subSections[i].name);
		}
	}

	onContextMenu(point: cool.SimplePoint, e: MouseEvent): void {
		MouseControl.contextMenuFrom(this, point, e);
	}

	private hideSubSections() {
		for (let i = 0; i < this.sectionProperties.subSections.length; i++)
			this.sectionProperties.subSections[i].setShowSection(false);
	}

	onClick(point: cool.SimplePoint, e: MouseEvent): void {
		point.pX += this.position[0];
		point.pY += this.position[1];
		var modifier = MouseControl.readModifier(e);

		// Capture position for the timer callback.
		const clickX = point.x;
		const clickY = point.y;

		// Cancel any pending click timer from a previous click.
		if (this.sectionProperties.clickTimer) {
			app.timerRegistry.clearTimeout(this.sectionProperties.clickTimer);
			this.sectionProperties.clickTimer = null;
		}

		// Delay the click so that a double-click can cancel it.
		this.sectionProperties.clickTimer = app.timerRegistry.setTimeout(
			'shapeClickTimer',
			() => {
				app.map._docLayer._postMouseEvent(
					'buttondown',
					clickX,
					clickY,
					1,
					1,
					modifier,
				);
				app.map._docLayer._postMouseEvent(
					'buttonup',
					clickX,
					clickY,
					1,
					1,
					modifier,
				);
				this.sectionProperties.clickTimer = null;
			},
			250,
		);

		// There is no native "double-click" event for touch devices. But we need to support double-tap.
		if ((e as any).pointerType === 'touch') {
			if (this.sectionProperties.lastTapTime && (Date.now() - this.sectionProperties.lastTapTime) < 250) {
				point.pX -= this.position[0]; // Take the addition back. It will be added in onDoubleClick.
				point.pY -= this.position[1];
				this.onDoubleClick(point, e);
			}

			this.sectionProperties.lastTapTime = Date.now();
		}

		if (this.sectionProperties.hasVideo && this.sectionProperties.svg) {
			const videoObject = this.sectionProperties.svg.querySelector('video');

			if (videoObject) {
				this.sectionProperties.svg.style.opacity = 1;
				this.sectionProperties.svg.style.pointerEvents = '';
				this.hideSubSections();
			}
		}
	}

	onDoubleClick(point: cool.SimplePoint, e: MouseEvent): void {
		// Cancel the pending click - a double-click replaces it.
		if (this.sectionProperties.clickTimer) {
			app.timerRegistry.clearTimeout(this.sectionProperties.clickTimer);
			this.sectionProperties.clickTimer = null;
		}

		if (app.map._docLayer.isCalcRTL()) {
			point.pX = (-point.pX + this.position[0]);
		} else {
			point.pX += this.position[0];
		}
		point.pY += this.position[1];
		app.map._docLayer._postMouseEvent('buttondown', point.x, point.y, 2, 1, 0);
		app.map._docLayer._postMouseEvent('buttonup', point.x, point.y, 2, 1, 0);
	}
}

app.definitions.shapeHandlesSection = ShapeHandlesSection;
