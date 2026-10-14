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
	The Impress and Draw edit view while the client draws the document from the objects. It is a
	layout of its own, swapped in for the tiled one when the rendering is switched, so the tiled
	view and the other applications keep theirs as they are.

	The slide stands on the lawn, the "Wiese" of the office: a number of page widths across and of
	page heights down, the page in the middle of it. The office lays three widths and two heights
	(drviewsa.cxx: aPageOrg = (w, h/2), aSize = (3w, 2h)), and that is where this starts; the two
	numbers are kept below so they can be changed. The document's origin stays at the top left of
	the page, so the lawn begins before it.

	The view is held in document terms: the zoom, which is the application's, and the point of the
	document at the middle of the frame. Everything else is worked out from those two every time
	the view is built - the rectangle of the document in view, the scroll from the lawn's top left,
	the place of the scroll bars. Every zoom places the view through one function, about a point of
	the document that is kept either at the middle of the frame or at the place on the frame it was
	given with, so the point under the mouse stays under the mouse. A zoom that comes with no point
	at all, as the fit zoom on load, on a resize and from the menu does, puts the view back onto the
	middle of the page.
*/
class ViewLayoutVector extends ViewLayoutBase {
	public readonly type: string = 'ViewLayoutVector';

	/// How many page widths the lawn is across, and how many page heights it is down. The page
	/// stands in the middle of it.
	public static lawnWidthInPages: number = 3;
	public static lawnHeightInPages: number = 2;

	/// The size of the page in twips, as the document reports it.
	private page: cool.SimplePoint = new cool.SimplePoint(0, 0);

	/// The point of the document at the middle of the frame, in twips. None while the view
	/// stands on the middle of the page.
	private middle: cool.SimplePoint | null = null;

	/// The place on the frame, in core pixels from its top left, that the pivot of the zoom
	/// being applied is to stay at. None for a zoom about the middle.
	private pivotOnFrame: cool.Point | null = null;

	/// True while a zoom with no pivot is applied, which puts the view back onto the page.
	private fitting: boolean = false;

	/*
		True once the user chose the view, by scrolling or by zooming, and while that holds a
		resize keeps the zoom. The presentation fits its zoom to the frame on every resize, which
		is right for a view that stands on the fitted page and wrong for one the user zoomed into.
	*/
	private theUserChoseTheView: boolean = false;

	/// True while the frame is being resized, so a fit zoom that comes of it can be told apart.
	private resizing: boolean = false;

	/// Whether the client draws from the objects for this document, which is when this layout is
	/// the one to use.
	public static isWanted(): boolean {
		const type = app.map._docLayer?._docType;
		return (
			RenderManager.isVectorRendering() &&
			!app.file.fileBasedView &&
			(type === 'presentation' || type === 'drawing')
		);
	}

	/// The layout a presentation or a drawing is edited in: this one while the client draws from
	/// the objects, the tiled one otherwise.
	public static forAPresentation(): ViewLayoutBase {
		return ViewLayoutVector.isWanted()
			? new ViewLayoutVector()
			: new ViewLayoutImpress();
	}

	/*
		Swaps the presentation's layout for the one the rendering now asks for, after the
		rendering was switched. What was in view stays in view: the point at the middle of the
		frame is handed to the new layout, which shows it at its middle.
	*/
	public static switchTheRendering(): void {
		const document = app.activeDocument;
		const before = document?.activeLayout;
		if (!document || !before || app.file.fileBasedView) return;
		if (
			before.type !== 'ViewLayoutImpress' &&
			before.type !== 'ViewLayoutVector'
		)
			return;

		const fresh = ViewLayoutVector.forAPresentation();
		if (fresh.type === before.type) {
			fresh.dispose();
			return;
		}

		const middle = before.zoomAnchorPoint();
		fresh.documentAnchorPosition = before.documentAnchorPosition;
		fresh.viewSize = document.fileSize.clone();
		document.swapLayout(fresh);
		fresh.showAtTheMiddle(middle);
	}

	/*
		The page and the lawn round it, in twips of the document: where the lawn begins, which is
		before the page's origin, and how large it is. It is so many pages across and down, and
		grows to take in every object of the slide shown with a margin round it, so an object
		that lies further out, from a file or from another user, can still be reached.
	*/
	private lawn(): cool.Range2D {
		const lawn = this.lawnOfThePage();

		const objects = RenderGeometrySection.boxOfAllObjects();
		if (!objects) return lawn;

		const margin = Math.max(this.page.x, this.page.y) / 10;
		return lawn.union(objects.expand(margin, margin));
	}

	/// The lawn as the page gives it, so many pages across and down round it, before it grows
	/// to take in the objects.
	private lawnOfThePage(): cool.Range2D {
		const across = ViewLayoutVector.lawnWidthInPages;
		const down = ViewLayoutVector.lawnHeightInPages;
		const left = (-this.page.x * (across - 1)) / 2;
		const top = (-this.page.y * (down - 1)) / 2;
		return new cool.Range2D(
			left,
			top,
			left + this.page.x * across,
			top + this.page.y * down,
		);
	}

	/// The lawn as the page gives it, in twips, or nothing where the view is not drawn from the
	/// objects.
	public static lawnOfThePageInView(): cool.Range2D | undefined {
		const layout = app.activeDocument?.activeLayout;
		return layout?.type === 'ViewLayoutVector'
			? (layout as ViewLayoutVector).lawnOfThePage()
			: undefined;
	}

	/// Works the lawn out again from the objects as they stand, and the view with it where it
	/// changed.
	public static takeInTheObjects(): void {
		const layout = app.activeDocument?.activeLayout;
		if (layout?.type !== 'ViewLayoutVector') return;

		const vector = layout as ViewLayoutVector;
		const lawn = vector.lawn();
		if (lawn.width === vector._viewSize.x && lawn.height === vector._viewSize.y)
			return;

		vector._viewSize = new cool.SimplePoint(lawn.width, lawn.height);
		vector.updateViewData();
	}

	private middleOfThePage(): cool.SimplePoint {
		return new cool.SimplePoint(this.page.x / 2, this.page.y / 2);
	}

	public override get viewSize(): cool.SimplePoint {
		return this._viewSize;
	}

	/// The size given is the page's. What can be scrolled is the lawn round it.
	public override set viewSize(size: cool.SimplePoint) {
		this.page = size.clone();
		const lawn = this.lawn();
		this._viewSize = new cool.SimplePoint(lawn.width, lawn.height);
	}

	protected override getBaseViewSize(): cool.SimplePoint {
		return this._viewSize;
	}

	public override laysRoomBeforeTheDocument(): boolean {
		return true;
	}

	public override zoomAnchorPoint(): cool.SimplePoint {
		return (this.middle ?? this.middleOfThePage()).clone();
	}

	/// Shows that point of the document at the middle of the frame.
	public override showAtTheMiddle(point: cool.SimplePoint): void {
		this.middle = point.clone();
		this.theUserChoseTheView = true;
		this.updateViewData();
	}

	/*
		Builds the view from the zoom and the point at the middle of the frame. Where the frame is
		smaller than the lawn on an axis, the view is kept inside the lawn; where it is larger, the
		lawn stands in the middle of the frame. The point at the middle is then where the view put
		it, so a view held at an edge stays there through the next zoom.
	*/
	protected override refreshVisibleAreaRectangle(): void {
		const anchor = this.getDocumentAnchorSection();
		if (!anchor) return;

		const scale = app.twipsToPixels;
		if (!scale || this.page.x <= 0 || this.page.y <= 0) {
			this.viewedRectangle = cool.SimpleRectangle.fromCorePixels([
				0,
				0,
				anchor.size[0],
				anchor.size[1],
			]);
			return;
		}

		const lawn = this.lawn();
		const width = anchor.size[0] / scale;
		const height = anchor.size[1] / scale;
		const wanted = this.middle ?? this.middleOfThePage();

		const placed = (
			at: number,
			size: number,
			from: number,
			length: number,
		): number => {
			if (size >= length) return from - (size - length) / 2;
			return Math.min(Math.max(at - size / 2, from), from + length - size);
		};
		const left = placed(wanted.x, width, lawn.minX, lawn.width);
		const top = placed(wanted.y, height, lawn.minY, lawn.height);

		if (this.middle)
			this.middle = new cool.SimplePoint(left + width / 2, top + height / 2);

		this.scrollProperties.viewX = Math.max(
			0,
			Math.round((left - lawn.minX) * scale),
		);
		this.scrollProperties.viewY = Math.max(
			0,
			Math.round((top - lawn.minY) * scale),
		);
		this.refreshScrollProperties();
		if (this._viewSize.pX > 0)
			this.scrollProperties.startX = Math.round(
				(this.scrollProperties.viewX / this._viewSize.pX) *
					this.scrollProperties.horizontalScrollLength,
			);
		if (this._viewSize.pY > 0)
			this.scrollProperties.startY = Math.round(
				(this.scrollProperties.viewY / this._viewSize.pY) *
					this.scrollProperties.verticalScrollLength,
			);

		this.viewedRectangle = cool.SimpleRectangle.fromCorePixels([
			Math.round(left * scale),
			Math.round(top * scale),
			anchor.size[0],
			anchor.size[1],
		]);
	}

	/// The part of the document in view, in core pixels of the document.
	public override getViewportCorePixelBounds(): cool.Bounds {
		const rect = this._viewedRectangle;
		return new cool.Bounds(
			new cool.Point(rect.pX1, rect.pY1),
			new cool.Point(rect.pX1 + rect.pWidth, rect.pY1 + rect.pHeight),
		);
	}

	/// Moves the view by that many core pixels. A move that the lawn's edges allow is what
	/// happens, and the answer says whether the view moved at all.
	public override scroll(
		pX: number,
		pY: number,
		userIsScrolling: boolean = false,
	): boolean {
		if (userIsScrolling) this.unselectCommentOnScroll();

		const scale = app.twipsToPixels;
		if (!scale) return false;

		const before = this._viewedRectangle.clone();
		const from = this.zoomAnchorPoint();
		this.middle = new cool.SimplePoint(
			from.x + pX / scale,
			from.y + pY / scale,
		);
		this.theUserChoseTheView = true;
		this.updateViewData();

		return (
			before.pX1 !== this._viewedRectangle.pX1 ||
			before.pY1 !== this._viewedRectangle.pY1
		);
	}

	/*
		Places the view for a zoom about that point: the point stays at the place on the frame
		the zoom was given with, or at the middle of the frame. A zoom with no point puts the view
		onto the middle of the page.
	*/
	public override setViewRectangleFromPointAndScale(
		point: cool.SimplePoint,
		scale: number,
	): void {
		if (!scale) return;

		if (this.fitting) {
			this.middle = null;
		} else if (this.pivotOnFrame) {
			const anchor = this.getDocumentAnchorSection();
			const frameMiddleX = anchor ? anchor.size[0] / 2 : 0;
			const frameMiddleY = anchor ? anchor.size[1] / 2 : 0;
			this.middle = new cool.SimplePoint(
				point.x + (frameMiddleX - this.pivotOnFrame.x) / scale,
				point.y + (frameMiddleY - this.pivotOnFrame.y) / scale,
			);
		} else {
			this.middle = point.clone();
		}

		this.refreshVisibleAreaRectangle();
	}

	public override applyZoom(
		targetZoom: number,
		anchorTwips?: cool.SimplePoint,
		anchorOnCanvas?: cool.SimplePoint,
	): void {
		// A fit that comes of a resize leaves a view the user chose as it is, zoom and all. The
		// resize builds the view again for the new frame itself.
		if (!anchorTwips && this.resizing && this.theUserChoseTheView) return;

		this.theUserChoseTheView = anchorTwips !== undefined;
		this.fitting = !anchorTwips;
		this.pivotOnFrame =
			anchorTwips && anchorOnCanvas
				? new cool.Point(
						anchorOnCanvas.pX - this._documentAnchorPosition[0],
						anchorOnCanvas.pY - this._documentAnchorPosition[1],
					)
				: null;

		super.applyZoom(targetZoom, anchorTwips, anchorOnCanvas);

		this.fitting = false;
		this.pivotOnFrame = null;
		this.updateViewData();
	}

	/*
		The frame changed size. A view the user chose keeps what stands at its top left there, as
		a window resized by its lower right corner does elsewhere; a view on the fitted page is
		fitted again and stays on the middle of the page.
	*/
	public override onResize(): void {
		const topLeft = new cool.SimplePoint(
			this._viewedRectangle.x1,
			this._viewedRectangle.y1,
		);
		const keepsTheTopLeft =
			this.theUserChoseTheView && this._viewedRectangle.width > 0;

		this.resizing = true;
		try {
			app.map._docLayer._syncTileContainerSize();
		} finally {
			this.resizing = false;
		}

		app.layoutingService.appendLayoutingTask(() => {
			const anchor = this.getDocumentAnchorSection();
			const scale = app.twipsToPixels;
			if (keepsTheTopLeft && anchor && scale)
				this.middle = new cool.SimplePoint(
					topLeft.x + anchor.size[0] / scale / 2,
					topLeft.y + anchor.size[1] / scale / 2,
				);
			this.updateViewData();
		});
	}
}
