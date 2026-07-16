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
 * SlideCompositor is responsible for slide image generation, which later will be shown on the screen
 */

declare var SlideShow: any;

abstract class SlideCompositor {
	_slideShowPresenter: SlideShowPresenter = null;
	protected _metaPresentation: MetaPresentation = null;
	_initialSlideNumber: number = 0;
	_onGotSlideCallback: VoidFunction = null;

	constructor(
		slideShowPresenter: SlideShowPresenter,
		metaPresentation: MetaPresentation,
	) {
		this._slideShowPresenter = slideShowPresenter;
		this._metaPresentation = metaPresentation;
		this._addHooks();
	}

	public computeLayerSize(width: number, height: number): [number, number] {
		// compute the slide size in pixel with respect to the current resolution
		const slideWidth = this._metaPresentation.getDocWidth();
		const slideHeight = this._metaPresentation.getDocHeight();
		const slideRatio = slideWidth / slideHeight;
		const resolutionRatio = width / height;
		if (slideRatio > resolutionRatio) {
			height = Math.trunc((width * slideHeight) / slideWidth);
		} else if (slideRatio < resolutionRatio) {
			width = Math.ceil((height * slideWidth) / slideHeight);
		}
		return [width, height];
	}

	protected abstract _addHooks(): void;

	public abstract removeHooks(): void;

	public abstract onUpdatePresentationInfo(): void;

	public fetchAndRun(slideNumber: number, callback: VoidFunction) {
		this._initialSlideNumber = slideNumber;
		this._onGotSlideCallback = callback;
	}

	public abstract getCanvasSize(): [number, number]; // [width, height]

	public abstract getSlide(slideNumber: number): ImageBitmap;

	public abstract getLayerImage(
		slideHash: string,
		targetElement: string,
	): ImageBitmap;

	public abstract getLayerBounds(
		slideHash: string,
		targetElement: string,
	): BoundingBoxType;

	public abstract getAnimatedSlide(slideIndex: number): ImageBitmap;

	public abstract getAnimatedLayerInfo(
		slideHash: string,
		targetElement: string,
	): AnimatedShapeInfo;

	public abstract getLayerRendererContext(): RenderContext;

	public abstract getVideoRenderer(
		slideHash: string,
		videoInfo: VideoInfo,
	): VideoRenderer;

	public abstract deleteResources(): void;

	public abstract pauseVideos(slideHash: string): void;

	public abstract notifyTransitionStart(): void;

	public abstract notifyTransitionEnd(slideHash: string): void;
}

SlideShow.SlideCompositor = SlideCompositor;
