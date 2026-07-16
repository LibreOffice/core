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

/*
 * VectorCompositor composes slides from vector primitives. It renders a
 * slide's primitive tree into an offscreen canvas and returns the
 * composed slide as an ImageBitmap.
 *
 * The primitive trees and their embedded bitmaps come from the shared
 * cache behind RenderManager. This compositor only reads that cache.
 * Fetching and caching of the trees is owned elsewhere.
 *
 * This first version renders and shows slides and lets transitions work.
 * Shape animations and video are not handled yet: the animation and video
 * methods return nothing.
 */

/* global app RenderManager */

declare var SlideShow: any;

class VectorCompositor extends SlideCompositor {
	private offscreenCanvas: OffscreenCanvas = null;
	private offscreenContext: OffscreenCanvasRenderingContext2D = null;
	private canvasWidth = 0;
	private canvasHeight = 0;

	private disposed = false;

	// The slide the initial fetchAndRun is waiting on, and the callback to
	// fire once its primitive tree is in the cache.
	private pendingSlideNumber: number = null;
	private pendingReady: VoidFunction = null;

	constructor(
		slideShowPresenter: SlideShowPresenter,
		metaPresentation: MetaPresentation,
	) {
		super(slideShowPresenter, metaPresentation);
	}

	protected _addHooks(): void {
		// The shared cache fires this when a tree or an embedded bitmap
		// arrives. Use it to release the initial fetchAndRun once its
		// slide is ready.
		RenderManager.onVectorChanged(() => this._onDataChanged());
	}

	public removeHooks(): void {
		// RenderManager.onVectorChanged keeps the callback for the life of
		// the page, so the disposed flag guards against acting after the
		// slideshow has closed.
	}

	public onUpdatePresentationInfo(): void {
		[this.canvasWidth, this.canvasHeight] = this._computeCanvasSize();
		this.offscreenCanvas = new OffscreenCanvas(
			this.canvasWidth,
			this.canvasHeight,
		);
		this.offscreenContext = this.offscreenCanvas.getContext('2d');

		// The slideshow shows the whole deck, so request every slide's
		// tree at the start.
		this._fetchAllSlides();
	}

	public getCanvasSize(): [number, number] {
		return [this.canvasWidth, this.canvasHeight];
	}

	public fetchAndRun(slideNumber: number, callback: VoidFunction): void {
		super.fetchAndRun(slideNumber, callback);
		this._runWhenReady(slideNumber, () => {
			const done = this._onGotSlideCallback;
			this._onGotSlideCallback = null;
			if (done) done.call(this);
		});
	}

	public getSlide(slideNumber: number): ImageBitmap {
		return this._composeSlide(slideNumber);
	}

	public getAnimatedSlide(slideIndex: number): ImageBitmap {
		// No per-shape animation yet, so this is the static slide.
		return this._composeSlide(slideIndex);
	}

	// --- fetching ---------------------------------------------------

	private _fetchAllSlides(): void {
		this._metaPresentation.getMetaSlides().forEach((_metaSlide, hash) => {
			const info = this._metaPresentation.getSlideInfo(hash);
			if (info) RenderManager.requestPart(info.index);
		});
	}

	private _partForSlide(slideNumber: number): number {
		const hash = this._metaPresentation.getSlideHash(slideNumber);
		const info = hash ? this._metaPresentation.getSlideInfo(hash) : null;
		return info ? info.index : null;
	}

	private _runWhenReady(slideNumber: number, ready: VoidFunction): void {
		const part = this._partForSlide(slideNumber);
		if (part !== null && RenderManager.requestPart(part)) {
			ready();
			return;
		}
		this.pendingSlideNumber = slideNumber;
		this.pendingReady = ready;
	}

	private _onDataChanged(): void {
		if (this.disposed) return;
		if (this.pendingReady === null) return;

		const part = this._partForSlide(this.pendingSlideNumber);
		if (part === null || !RenderManager.requestPart(part)) return;

		const ready = this.pendingReady;
		this.pendingReady = null;
		this.pendingSlideNumber = null;
		ready();
	}

	// --- composing --------------------------------------------------

	private _composeSlide(slideNumber: number): ImageBitmap {
		if (this.disposed || !this.offscreenContext) return null;

		const part = this._partForSlide(slideNumber);
		if (part === null) return null;

		const data = RenderManager.requestPart(part);
		if (!data || data.slideWidth <= 0 || data.slideHeight <= 0) return null;

		const context = this.offscreenContext;
		context.clearRect(0, 0, this.canvasWidth, this.canvasHeight);
		context.fillStyle = '#FFFFFF';
		context.fillRect(0, 0, this.canvasWidth, this.canvasHeight);

		context.save();
		// Slide twips to canvas pixels. The canvas already carries the
		// slide aspect ratio, so a single scale fits the slide to it.
		context.scale(
			this.canvasWidth / data.slideWidth,
			this.canvasHeight / data.slideHeight,
		);
		RenderManager.renderInto(
			context as unknown as CanvasRenderingContext2D,
			data,
		);
		context.restore();

		return this.offscreenCanvas.transferToImageBitmap();
	}

	// --- animation and video: not handled yet -----------------------

	public getAnimatedLayerInfo(
		_slideHash: string,
		_targetElement: string,
	): AnimatedShapeInfo {
		return null;
	}

	public getLayerImage(
		_slideHash: string,
		_targetElement: string,
	): ImageBitmap {
		return null;
	}

	public getLayerBounds(
		_slideHash: string,
		_targetElement: string,
	): BoundingBoxType {
		return null;
	}

	public getLayerRendererContext(): RenderContext {
		return null;
	}

	public getVideoRenderer(
		_slideHash: string,
		_videoInfo: VideoInfo,
	): VideoRenderer {
		return null;
	}

	public pauseVideos(_slideHash: string): void {
		// Video is not handled yet.
	}

	// --- transitions and lifecycle ----------------------------------

	public notifyTransitionStart(): void {
		// The primitive trees are already cached, so there is nothing to
		// hold back while a transition runs.
	}

	public notifyTransitionEnd(_slideHash: string): void {
		// No per-slide video to start once a transition ends yet.
	}

	public deleteResources(): void {
		this.disposed = true;
		this.offscreenCanvas = null;
		this.offscreenContext = null;
		this.pendingReady = null;
		this.pendingSlideNumber = null;
	}

	// --- canvas sizing ----------------------------------------------

	// Primitives render at any scale, so the canvas takes the display
	// resolution directly, fitted to the slide aspect ratio.
	private _computeCanvasSize(): [number, number] {
		const devicePixelRatio = window.devicePixelRatio || 1;
		return this.computeLayerSize(
			window.screen.width * devicePixelRatio,
			window.screen.height * devicePixelRatio,
		);
	}
}

SlideShow.VectorCompositor = VectorCompositor;
