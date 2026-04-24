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
	private metaPresentation: MetaPresentation;

	constructor(
		slideShowPresenter: SlideShowPresenter,
		metaPres: MetaPresentation,
	) {
		super(slideShowPresenter);
		this.metaPresentation = metaPres;
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
		return this.metaPresentation.getSlideInfo(slideHash);
	}

	public getDocWidth(): number {
		return this.metaPresentation.getDocWidth();
	}

	public getDocHeight(): number {
		return this.metaPresentation.getDocHeight();
	}

	public setDocWidth(slideWidth: number): void {
		this.metaPresentation.setDocWidth(slideWidth);
	}

	public setDocHeight(slideHeight: number): void {
		this.metaPresentation.setDocHeight(slideHeight);
	}

	public onUpdatePresentationInfo() {
		this.layerDrawing.onUpdatePresentationInfo();
		// TODO: optimize
		this.layerDrawing.invalidateAll();
	}

	public getSlideHash(slideIndex: number) {
		return this.metaPresentation.getSlideHash(slideIndex);
	}

	public getAnimatedElement(
		slideHash: string,
		animatedElementHash: string,
	): AnimatedElement {
		const metaSlide = this.metaPresentation.getMetaSlide(slideHash);
		if (!metaSlide) {
			window.app.console.log(
				'LayersCompositor.getAnimatedElement: failed to retrieve meta slide for hash: ' +
					slideHash,
			);
			return;
		}
		if (metaSlide.animationsHandler) {
			const animElemMap = metaSlide.animationsHandler.getAnimatedElementMap();
			return animElemMap.get(animatedElementHash);
		}
	}

	public getSlideSizePixel() {
		return [
			app.twipsToPixels * this.metaPresentation.getDocWidth(),
			app.twipsToPixels * this.metaPresentation.getDocHeight(),
		];
	}

	public computeLayerResolution(width: number, height: number) {
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
		return [resolutionWidth, resolutionHeight];
	}

	public computeLayerSize(width: number, height: number) {
		// compute the slide size in pixel with respect to the current resolution
		const slideWidth = this.metaPresentation.getDocWidth();
		const slideHeight = this.metaPresentation.getDocHeight();
		const slideRatio = slideWidth / slideHeight;
		const resolutionRatio = width / height;
		if (slideRatio > resolutionRatio) {
			height = Math.trunc((width * slideHeight) / slideWidth);
		} else if (slideRatio < resolutionRatio) {
			width = Math.ceil((height * slideWidth) / slideHeight);
		}
		return [width, height];
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
	): VideoRenderer {
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
		this.layerDrawing.deleteResources();
	}

	public pauseVideos(slideHash: string) {
		this.layerDrawing.pauseVideos(slideHash);
	}

	public notifyTransitionStart() {
		this.layerDrawing.notifyTransitionStart();
	}

	public notifyTransitionEnd(slideHash: string) {
		this.layerDrawing.notifyTransitionEnd(slideHash);
	}
}

SlideShow.LayersCompositor = LayersCompositor;
