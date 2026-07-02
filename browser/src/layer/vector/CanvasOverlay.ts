// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

// OverlayTransform is used by CanvasOverlay to apply transformations
// to points/bounds before drawing is done.
// The reason why we cannot use canvasRenderingContext2D.transform() is it
// does not support coordinate values bigger than 2^24 - 1 and if we use it in this
// regime the renders will be incorrect. At least in Calc it is possible to have pixel
// coordinates greater than this limit at higher zooms near the bottom of the sheet.
class OverlayTransform {
	private translationAmount: cool.Point;
	private scaleAmount: cool.Point;

	constructor() {
		this.translationAmount = new cool.Point(0, 0);
		this.scaleAmount = new cool.Point(1, 1);
	}

	translate(x: number, y: number) {
		this.translationAmount.x = x;
		this.translationAmount.y = y;
	}

	scale(sx: number, sy: number) {
		this.scaleAmount.x = sx;
		this.scaleAmount.y = sy;
	}

	reset() {
		this.translationAmount.x = 0;
		this.translationAmount.y = 0;
		this.scaleAmount.x = 1;
		this.scaleAmount.y = 1;
	}

	applyToPoint(point: cool.Point): cool.Point {
		// 'scale first then translation' model.
		return new cool.Point(
			point.x * this.scaleAmount.x - this.translationAmount.x,
			point.y * this.scaleAmount.y - this.translationAmount.y);
	}

	applyToBounds(bounds: cool.Bounds): cool.Bounds {
		return new cool.Bounds(
			this.applyToPoint(bounds.min),
			this.applyToPoint(bounds.max)
		);
	}
}

// This allows the overlay section to use multiple transformations to be applied
// one after the other on a point or bounds.
class TransformationsList {
	private list: OverlayTransform[];

	constructor() {
		this.list = [];
	}

	public add(tx: OverlayTransform) {
		this.list.push(tx);
	}

	public addNew(translate: cool.Point, scale: cool.Point) {
		const tx = new OverlayTransform();
		tx.translate(translate.x, translate.y);
		tx.scale(scale.x, scale.y);
		this.add(tx);
	}

	public reset() {
		this.list = [];
	}

	applyToPoint(point: cool.Point): cool.Point {
		let tPoint = point.clone();
		this.list.forEach((tx) => {
			tPoint = tx.applyToPoint(tPoint);
		});

		return tPoint;
	}

	applyToBounds(bounds: cool.Bounds): cool.Bounds {
		let tBounds = bounds.clone();
		this.list.forEach((tx) => {
			tBounds = tx.applyToBounds(tBounds);
		});

		return tBounds;
	}
}

// CanvasOverlay handles CPath rendering and mouse events handling via overlay-section of the main canvas.
// where overlays like cell-cursors, cell-selections, edit-cursors are instances of CPath or its subclasses.
class CanvasOverlay extends CanvasSectionObject {
	processingOrder: number = app.CSections.Overlays.processingOrder;
	drawingOrder: number = app.CSections.Overlays.drawingOrder;
	zIndex: number = app.CSections.Overlays.zIndex;
	anchor: string[] = ['top', 'left'];
	boundToSection: string = 'tiles';

	private map: any;
	private ctx: CanvasRenderingContext2D;
	private paths: Map<number, any>;
	private bounds: cool.Bounds;
	private tsManager: any;
	private transformList: TransformationsList;

	constructor(mapObject: any, canvasContext: CanvasRenderingContext2D) {
		super(app.CSections.Overlays.name);
		this.map = mapObject;
		this.ctx = canvasContext;
		this.tsManager = this.map.getTileSectionMgr();
		this.sectionProperties.docLayer = this.map._docLayer;
		this.sectionProperties.tsManager = this.tsManager;
		this.paths = new Map<number, CPath>();
		this.transformList = new TransformationsList();
		this.updateCanvasBounds();
	}

	onResize(): void {
		this.paths.forEach(function (path: CPath) {
			path.onResize();
		});
		this.onDraw();
	}

	onDraw(): void {
		// No need to "erase" previous drawings because tiles are draw first via its onDraw.
		this.draw();
	}

	onMouseMove(position: cool.SimplePoint): void {
		var mousePos = new cool.Point(position.pX, position.pY);
		var overlaySectionBounds = this.bounds.clone();
		var splitPos = this.tsManager.getSplitPos();
		if (this.isCalcRTL()) {
			// Mirror the mouse position in overlay section coordinates.
			mousePos.x = overlaySectionBounds.max.x - overlaySectionBounds.min.x - mousePos.x;
		}

		// overlay section coordinates -> document coordinates
		if (mousePos.x > splitPos.x)
			mousePos.x += overlaySectionBounds.min.x;
		if (mousePos.y > splitPos.y)
			mousePos.y += overlaySectionBounds.min.y;

		this.paths.forEach(function (path:CPath) {
			var pathBounds = path.getBounds();

			if (!pathBounds.isValid())
				return;

			var mouseOverPath = pathBounds.contains(mousePos);
			if (mouseOverPath && !path.isUnderMouse()) {
				path.onMouseEnter(mousePos);
				path.setUnderMouse(true);
			} else if (!mouseOverPath && path.isUnderMouse()) {
				path.onMouseLeave(mousePos);
				path.setUnderMouse(false);
			}
		});
	}

	setPenOnOverlay(): void {
		this.containerObject.setPenPosition(this);
	}

	initPath(path: CPath) {
		var pathId: number = path.getId();
		this.paths.set(pathId, path);
		path.setRenderer(this);
		this.setPenOnOverlay();
		path.updatePathAllPanes();
	}

	initPathGroup(pathGroup: CPathGroup) {
		pathGroup.forEach(function (path: CPath) {
			this.initPath(path);
		}.bind(this));
	}

	removePath(path: CPath) {
		// This does not get called via onDraw, so ask section container to redraw everything.
		path.setDeleted();
		this.paths.delete(path.getId());
		this.containerObject.requestReDraw();
	}

	removePathGroup(pathGroup: CPathGroup) {
		pathGroup.forEach(function (path: CPath) {
			this.removePath(path);
		}.bind(this));
	}

	updatePath(path: CPath, oldBounds: cool.Bounds) {
		this.redraw(path, oldBounds);
	}

	updateStyle(path: CPath, oldBounds: cool.Bounds) {
		this.redraw(path, oldBounds);
	}

	paintRegion(paintArea: cool.Bounds) {
		this.draw(paintArea);
	}

	getSplitPanesContext(): any {
		return this.map.getSplitPanesContext();
	}

	private isPathVisible(path: CPath): boolean {
		var pathBounds = path.getBounds();
		if (!pathBounds.isValid())
			return false;
		return this.intersectsVisible(pathBounds);
	}

	private intersectsVisible(queryBounds: cool.Bounds): boolean {
		this.updateCanvasBounds();
		var spc = this.getSplitPanesContext();
		return spc ? spc.intersectsVisible(queryBounds) : this.bounds.intersects(queryBounds);
	}

	private static renderOrderComparator(a: CPath, b: CPath): number {
		if (a.viewId === -1 && b.viewId === -1) {
			// Both are 'own' / 'self' paths.

			// Both paths are part of the same group, use their zindex to break the tie.
			if (a.groupType === b.groupType)
				return a.zIndex - b.zIndex;

			return a.groupType - b.groupType;

		} else if (a.viewId === -1) {
			// a is an 'own' path and b is not => draw a on top of b.
			return 1;

		} else if (b.viewId === -1) {
			// b is an 'own' path and a is not => draw b on top of a.
			return -1;

		}

		// Both a and b belong to other views.

		if (a.viewId === b.viewId) {
			// Both belong to the same view.

			// Both paths are part of the same group, use their zindex to break the tie.
			if (a.groupType === b.groupType)
				return a.zIndex - b.zIndex;

			return a.groupType - b.groupType;

		}

		// a and b belong to different views.
		return a.viewId - b.viewId;
	}

	private draw(paintArea?: cool.Bounds) {
		if (this.tsManager && this.tsManager.waitForTiles()) {
			// don't paint anything till tiles arrive for new zoom.
			return;
		}

		var orderedPaths = Array<CPath>();
		this.paths.forEach((path: CPath) => {
			orderedPaths.push(path);
		});

		// Sort them w.r.t. rendering order.
		orderedPaths.sort(CanvasOverlay.renderOrderComparator);

		orderedPaths.forEach((path: CPath) => {
			if (this.isPathVisible(path))
				path.updatePathAllPanes(paintArea);
		}, this);
	}

	private redraw(path: CPath, oldBounds: cool.Bounds) {
		if (this.tsManager && this.tsManager.waitForTiles()) {
			// don't paint anything till tiles arrive for new zoom.
			return;
		}

		if (!this.isPathVisible(path) && (!oldBounds.isValid() || !this.intersectsVisible(oldBounds)))
			return;
		// This does not get called via onDraw(ie, tiles aren't painted), so ask tileSection to "erase" by painting over.
		// Repainting the whole canvas is not necessary but finding the minimum area to paint over
		// is potentially expensive to compute (think of overlapped path objects).
		// TODO: We could repaint the area on the canvas occupied by all the visible path-objects
		// and paint tiles just for that, but need a more general version of _tilesSection.onDraw() and callees.
		this.containerObject.requestReDraw();
	}

	private updateCanvasBounds() {
		if (app.activeDocument.activeLayout.type === 'ViewLayoutCalc') {
			const r = app.activeDocument.activeLayout.viewedRectangle;
			this.bounds = new cool.Bounds(new cool.Point(r.pX1, r.pY1), new cool.Point(r.pX1 + r.pWidth, r.pY1 + r.pHeight));
			return;
		}
		var viewBounds: any = this.map.getPixelBoundsCore();
		this.bounds = new cool.Bounds(new cool.Point(viewBounds.min.x, viewBounds.min.y), new cool.Point(viewBounds.max.x, viewBounds.max.y));
	}

	getBounds(): cool.Bounds {
		this.updateCanvasBounds();
		return this.bounds;
	}

	// Applies canvas translation so that polygons/circles can be drawn using core-pixel coordinates.
	private ctStart(clipArea?: cool.Bounds, paneBounds?: cool.Bounds, fixed?: boolean, freezePane?: { freezeX: boolean, freezeY: boolean }) {
		this.updateCanvasBounds();
		this.transformList.reset();
		this.ctx.save();

		if (!paneBounds)
			paneBounds = this.bounds.clone();

		const transform = new OverlayTransform();

		const { freezeX, freezeY } = freezePane ?? { freezeX: false, freezeY: false };

		if (this.tsManager._inZoomAnim && !fixed) {
			// zoom-animation is in progress : so draw overlay on main canvas
			// at the current frame's zoom level.

			var splitPos = this.tsManager.getSplitPos();
			var scale = this.tsManager._zoomFrameScale;

			const docPos = this.tsManager._getZoomDocPos(
				this.tsManager._newCenter,
				this.tsManager._layer._pinchStartCenter,
				paneBounds,
				{ freezeX, freezeY },
				splitPos,
				scale,
				false /* findFreePaneCenter? */
			);

			const clipTopLeft = new cool.Point(docPos.topLeft.x, docPos.topLeft.y);

			// Original pane size.
			var paneSize = paneBounds.getSize();
			var clipSize = paneSize.clone();
			if (!freezeX) {
				// Pane's "free" size will shrink(expand) as we zoom in(out)
				// respectively because fixed pane size expand(shrink).
				clipSize.x = (paneSize.x - splitPos.x * (scale - 1)) / scale;

				docPos.topLeft.x -= splitPos.x;
			}
			if (!freezeY) {
				// See comment regarding pane width above.
				clipSize.y = (paneSize.y - splitPos.y * (scale - 1)) / scale;

				docPos.topLeft.y -= splitPos.y;
			}
			// Force clip area to the zoom frame area of the pane specified.
			clipArea = new cool.Bounds(
				clipTopLeft,
				clipTopLeft.add(clipSize));

			transform.scale(scale, scale);
			transform.translate(scale * docPos.topLeft.x, scale * docPos.topLeft.y);

		} else if (this.tsManager._inZoomAnim && fixed) {

			var scale = this.tsManager._zoomFrameScale;
			transform.scale(scale, scale);

			if (clipArea) {
				clipArea = new cool.Bounds(
					clipArea.min.divideBy(scale),
					clipArea.max.divideBy(scale)
				);
			}

		} else {
			transform.translate(
				paneBounds.min.x ? this.bounds.min.x : 0,
				paneBounds.min.y ? this.bounds.min.y : 0);
		}

		this.transformList.add(transform);
		if (this.isCalcRTL()) {
			const sectionWidth = this.size[0];
			// Apply horizontal flip transformation.
			this.transformList.addNew(new cool.Point(-sectionWidth, 0), new cool.Point(-1, 1));
		}

		if (clipArea) {
			this.ctx.beginPath();
			clipArea = this.transformList.applyToBounds(clipArea);
			var clipSize = clipArea.getSize();
			this.ctx.rect(clipArea.min.x, clipArea.min.y, clipSize.x, clipSize.y);
			this.ctx.clip();
		}
	}

	// Undo the canvas translation done by ctStart().
	private ctEnd() {
		this.ctx.restore();
	}

	updatePoly(path: CPath, closed: boolean = false, clipArea?: cool.Bounds, paneBounds?: cool.Bounds, freezePane?: { freezeX: boolean, freezeY: boolean }) {
		var i: number;
		var j: number;
		var len2: number;
		var part: cool.Point;
		var parts = path.getParts();
		var len: number = parts.length;

		if (!len)
			return;

		this.ctStart(clipArea, paneBounds, path.fixed, freezePane);
		this.ctx.beginPath();

		for (i = 0; i < len; i++) {
			for (j = 0, len2 = parts[i].length; j < len2; j++) {
				part = this.transformList.applyToPoint(parts[i][j]);
				this.ctx[j ? 'lineTo' : 'moveTo'](part.x, part.y);
			}
			if (closed) {
				this.ctx.closePath();
			}
		}

		this.fillStroke(path);

		this.ctEnd();
	}

	updateCircle(path: CPath, clipArea?: cool.Bounds, paneBounds?: cool.Bounds, freezePane?: { freezeX: boolean, freezeY: boolean }) {
		if (path.empty())
			return;

		this.ctStart(clipArea, paneBounds, path.fixed, freezePane);

		var point = this.transformList.applyToPoint(path.point);
		var r: number = path.radius;
		var s: number = (path.radiusY || r) / r;

		if (s !== 1) {
			this.ctx.save();
			this.ctx.scale(1, s);
		}

		this.ctx.beginPath();
		this.ctx.arc(point.x, point.y / s, r, 0, Math.PI * 2, false);

		if (s !== 1) {
			this.ctx.restore();
		}

		this.fillStroke(path);

		this.ctEnd();
	}

	private fillStroke(path: CPath) {
		if (path.fill) {
			this.ctx.globalAlpha = path.fillOpacity;
			this.ctx.fillStyle = path.fillColor || path.color;

			this.ctx.fill(path.fillRule || 'evenodd');
		}

		if (path.stroke && path.weight !== 0) {
			this.ctx.globalAlpha = path.opacity;

			this.ctx.lineWidth = path.weight;
			this.ctx.strokeStyle = path.color;
			this.ctx.lineCap = path.lineCap;
			this.ctx.lineJoin = path.lineJoin;
			this.ctx.stroke();
		}

	}

	bringToFront(path: CPath) {
		// TODO: Implement this.
	}

	bringToBack(path: CPath) {
		// TODO: Implement this.
	}

	getMap(): any {
		return this.map;
	}
}
