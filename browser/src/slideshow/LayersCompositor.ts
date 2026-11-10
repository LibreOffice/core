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
 * LayersCompositor generates slide from layers
 */

declare var SlideShow: any;

class LayersCompositor extends SlideCompositor {
	private layerDrawing: LayerDrawing; // setup in constructor
	// For each WebGL context drawn into, its quads and the slides split at their master page
	// videos, at most two, the slide being left and the slide coming in.
	private slidePartsByContext = new Map<
		RenderContextGl,
		{ quads: SlideQuadsGl; slides: SlidePartsGl[] }
	>();

	constructor(slideShowPresenter: SlideShowPresenter) {
		super(slideShowPresenter);
	}

	protected _addHooks() {
		this.layerDrawing = new SlideShow.LayerDrawing(app.map, this);
		this.layerDrawing.addHooks();
	}

	public removeHooks() {
		this.layerDrawing.removeHooks();
	}

	public fetchAndRun(slideNumber: number, callback: VoidFunction) {
		super.fetchAndRun(slideNumber, callback);
		this.layerDrawing.requestSlide(this._initialSlideNumber, () => {
			const oldCallback = this._onGotSlideCallback;
			this._onGotSlideCallback = null;
			oldCallback();
		});
	}

	public getSlideInfo(slideHash: string): SlideInfo {
		return this._metaPresentation.getSlideInfo(slideHash);
	}

	public getDocWidth(): number {
		return this._metaPresentation.getDocWidth();
	}

	public getDocHeight(): number {
		return this._metaPresentation.getDocHeight();
	}

	public setDocWidth(slideWidth: number): void {
		this._metaPresentation.setDocWidth(slideWidth);
	}

	public setDocHeight(slideHeight: number): void {
		this._metaPresentation.setDocHeight(slideHeight);
	}

	public onUpdatePresentationInfo() {
		this.layerDrawing.onUpdatePresentationInfo();
		// TODO: optimize
		this.layerDrawing.invalidateAll();
	}

	public getSlideHash(slideIndex: number) {
		return this._metaPresentation.getSlideHash(slideIndex);
	}

	public createAnimatedElement(
		sId: string,
		slideHash: string,
		title: string,
		slideWidth: number,
		slideHeight: number,
		bIsTextElement: boolean,
	): AnimatedElement {
		return bIsTextElement
			? new AnimatedTextElement(sId, slideHash, title, slideWidth, slideHeight)
			: new AnimatedElementBitmap(
					sId,
					slideHash,
					title,
					slideWidth,
					slideHeight,
				);
	}

	public getSlideSizePixel() {
		return [
			app.twipsToPixels * this._metaPresentation.getDocWidth(),
			app.twipsToPixels * this._metaPresentation.getDocHeight(),
		];
	}

	public computeLayerResolution(
		width: number,
		height: number,
	): [number, number] {
		width *= 1.2;
		height *= 1.2;
		let resolutionWidth = 960;
		let resolutionHeight = 540;

		if (width > 3840 || height > 2160) {
			resolutionWidth = 3840;
			resolutionHeight = 2160;
		} else if (width > 2560 || height > 1440) {
			resolutionWidth = 2560;
			resolutionHeight = 1440;
		} else if (width > 1920 || height > 1080) {
			resolutionWidth = 1920;
			resolutionHeight = 1080;
		} else if (width > 1280 || height > 720) {
			resolutionWidth = 1280;
			resolutionHeight = 720;
		}

		// On a phone or tablet the system kills the browser content process
		// when it passes its memory ceiling. A transition holds every layer
		// of two slides as both a bitmap and a GPU texture, and a layer is
		// 33 MB at 3840x2160 but 8 MB at 1920x1080.
		if (window.L.Browser.mobile) {
			resolutionWidth = Math.min(resolutionWidth, 1920);
			resolutionHeight = Math.min(resolutionHeight, 1080);
		}

		return [resolutionWidth, resolutionHeight];
	}

	// return [width, height]
	public getCanvasSize(): [number, number] {
		return this.layerDrawing.getCanvasSize();
	}

	public getSlide(slideNumber: number): ImageBitmap {
		return this.layerDrawing.getSlide(slideNumber);
	}

	public getLayerRendererContext(): RenderContext {
		return this.layerDrawing.getLayerRendererContext();
	}

	public getVideoRenderer(
		slideHash: string,
		videoInfo: VideoInfo,
	): AnimatedContentRenderer {
		return this.layerDrawing.getVideoRenderer(slideHash, videoInfo);
	}

	public getAnimatedSlide(slideIndex: number): ImageBitmap {
		return this.layerDrawing.getAnimatedSlide(slideIndex);
	}

	public getAnimatedLayerInfo(
		slideHash: string,
		targetElement: string,
	): AnimatedShapeInfo {
		return this.layerDrawing.getAnimatedLayerInfo(slideHash, targetElement);
	}

	public getLayerImage(slideHash: string, targetElement: string): ImageBitmap {
		return this.layerDrawing.getLayerImage(slideHash, targetElement);
	}

	public getLayerBounds(
		slideHash: string,
		targetElement: string,
	): BoundingBoxType {
		return this.layerDrawing.getLayerBounds(slideHash, targetElement);
	}

	public isSlideShowPlaying() {
		return this._slideShowPresenter._checkAlreadyPresenting();
	}

	public deleteResources() {
		this.slidePartsByContext.forEach((entry) => {
			for (const slide of entry.slides) slide.dispose();
			entry.quads.dispose();
		});
		this.slidePartsByContext.clear();
		this.layerDrawing.deleteResources();
	}

	/// A slide whose only moving parts are the videos of its master page is drawn from its still
	/// parts and the current video frames. Anything else that moves changes the stills, so the
	/// slide is then composed whole and the stills are taken anew afterwards.
	public drawFrameInto(
		context: RenderContextGl,
		slideIndex: number,
		texture?: WebGLTexture | null,
	): boolean {
		if (context.isDisposed() || !this.layerDrawing.hasMasterVideos(slideIndex))
			return false;
		const slideRenderer = this._slideShowPresenter._slideRenderer;
		if (
			!slideRenderer.isOnlyMoving((videoId) =>
				this.layerDrawing.isMasterVideo(videoId),
			)
		) {
			this.deleteSlideParts();
			return false;
		}

		const width = context.canvas.width;
		const height = context.canvas.height;
		if (width <= 0 || height <= 0) return false;
		const slideParts = this.getSlideParts(context, slideIndex);
		if (!slideParts.quads.begin(width, height, texture)) return false;
		slideParts.slide.draw(slideParts.quads);
		slideParts.quads.end();
		return true;
	}

	private getSlideParts(
		context: RenderContextGl,
		slideIndex: number,
	): { quads: SlideQuadsGl; slide: SlidePartsGl } {
		this.slidePartsByContext.forEach((entry, owner) => {
			if (!owner.isDisposed()) return;
			for (const slide of entry.slides) slide.dispose();
			entry.quads.dispose();
			this.slidePartsByContext.delete(owner);
		});
		let entry = this.slidePartsByContext.get(context);
		if (!entry) {
			entry = { quads: new SlideQuadsGl(context), slides: [] };
			this.slidePartsByContext.set(context, entry);
		}

		// An animation that starts changes the stills, also one that ends at once, as when an
		// effect is undone. Both counts only grow, so their sum changes with either.
		const slideHash = this.getSlideHash(slideIndex);
		const version =
			this.layerDrawing.getLayersVersion(slideHash) +
			this._slideShowPresenter._slideRenderer.animationStarts;
		let slide = entry.slides.find((held) => held.slideHash === slideHash);
		if (slide && slide.version !== version) {
			slide.dispose();
			entry.slides = entry.slides.filter((held) => held !== slide);
			slide = null;
		}
		if (!slide) {
			slide = new SlidePartsGl(
				context,
				slideHash,
				version,
				this.layerDrawing.getSlideParts(slideIndex),
			);
			entry.slides.push(slide);
			if (entry.slides.length > 2) entry.slides.shift().dispose();
		}
		return { quads: entry.quads, slide };
	}

	private deleteSlideParts() {
		this.slidePartsByContext.forEach((entry) => {
			for (const slide of entry.slides) slide.dispose();
			entry.slides = [];
		});
	}

	public pauseVideos(slideHash: string) {
		this.layerDrawing.pauseVideos(slideHash);
	}

	public hasMasterVideos(slideIndex: number): boolean {
		return this.layerDrawing.hasMasterVideos(slideIndex);
	}

	public getMasterVideoFrames(slideIndex: number): number {
		return this.layerDrawing.getMasterVideoFrames(slideIndex);
	}

	public notifyTransitionStart(slideHash: string) {
		this.layerDrawing.notifyTransitionStart(slideHash);
	}

	public notifyTransitionEnd(slideHash: string) {
		this.layerDrawing.notifyTransitionEnd(slideHash);
	}
}

SlideShow.LayersCompositor = LayersCompositor;
