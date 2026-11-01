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

	constructor(slideShowPresenter: SlideShowPresenter) {
		this._slideShowPresenter = slideShowPresenter;
		this._addHooks();
	}

	// The presentation to draw. It is built after the compositor.
	public setMetaPresentation(metaPresentation: MetaPresentation) {
		this._metaPresentation = metaPresentation;
	}

	public getAnimatedElement(
		slideHash: string,
		animatedElementHash: string,
	): AnimatedElement {
		return this.getAnimatedElementMap(slideHash)?.get(animatedElementHash);
	}

	/// The animated elements of the slide by target, or undefined without
	/// animations.
	public getAnimatedElementMap(
		slideHash: string,
	): Map<string, AnimatedElement> | undefined {
		const metaSlide = this._metaPresentation.getMetaSlide(slideHash);
		if (!metaSlide) {
			window.app.console.log(
				'SlideCompositor.getAnimatedElementMap: failed to retrieve meta slide for hash: ' +
					slideHash,
			);
			return undefined;
		}
		return metaSlide.animationsHandler?.getAnimatedElementMap();
	}

	/// The animated element of the kind this compositor draws, for the target sId.
	public abstract createAnimatedElement(
		sId: string,
		slideHash: string,
		title: string,
		slideWidth: number,
		slideHeight: number,
		bIsTextElement: boolean,
	): AnimatedElement;

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

	/// Draw the current frame of the slide straight into the WebGL context of the slideshow
	/// canvas, filling it, or into texture when one is given. False when the compositor does
	/// not draw the frame.
	public drawFrameInto(
		_context: RenderContextGl,
		_slideIndex: number,
		_texture?: WebGLTexture | null,
	): boolean {
		return false;
	}

	public abstract getAnimatedLayerInfo(
		slideHash: string,
		targetElement: string,
	): AnimatedShapeInfo;

	public abstract getLayerRendererContext(): RenderContext;

	public abstract getVideoRenderer(
		slideHash: string,
		videoInfo: VideoInfo,
	): AnimatedContentRenderer;

	public abstract deleteResources(): void;

	public abstract pauseVideos(slideHash: string): void;

	public abstract notifyTransitionStart(): void;

	public abstract notifyTransitionEnd(slideHash: string): void;
}

SlideShow.SlideCompositor = SlideCompositor;
