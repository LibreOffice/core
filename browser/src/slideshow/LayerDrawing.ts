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
 * LayerDrawing is handling the slideShow action
 */

/* global app  _ $ SlideBitmapManager */

declare var SlideShow: any;
// These are defined in browser/js/global.js
declare var ThisIsTheAndroidApp: any;
declare var ThisIsTheiOSApp: any;
declare var makeHttpUrl: any;
declare var coolParams: any;

type LayerContentType =
	| ImageInfo
	| PlaceholderInfo
	| AnimatedShapeInfo
	| TextFieldInfo;

type TextFieldsType = 'SlideNumber' | 'Footer' | 'DateTime';

type TextFields = {
	SlideNumber: string;
	DateTime: string;
	Footer: string;
};

interface TextFieldInfo {
	type: TextFieldsType;
	hash: string;
	content: ImageInfo;
}

interface ImageInfo {
	type: 'png' | 'zstd';
	checksum: string;
	data?: any;
}

interface AnimatedShapeInfo {
	hash: string;
	initVisible: boolean;
	type: 'bitmap' | 'svg';
	content: ImageInfo | SVGElement;
	bounds: BoundingBoxType;
	fillColor?: string;
	lineColor?: string;
	fontColor?: string;
}

interface PlaceholderInfo {
	type: TextFieldsType;
	hash: string;
}

interface LayerInfo {
	group: 'Background' | 'MasterPage' | 'DrawPage' | 'TextFields';
	slideHash: string;
	index?: number;
	type?: 'bitmap' | 'placeholder' | 'animated';
	content: LayerContentType;
	isField?: boolean;
}

interface LayerEntry {
	type: 'bitmap' | 'placeholder' | 'animated';
	content: LayerContentType;
	isField?: boolean;
}

class LayerDrawing {
	private map: any = null;
	private helper: LayersCompositor;

	private slideCache: SlideCache = new SlideCache();
	private compressedSlideCache: Map<string, CompressedSlideLayer[]> = new Map();
	private compressedPrefetchSlideHash: string = null;
	private requestedSlideHash: string = null;
	private prefetchedSlideHash: string = null;
	private nextRequestedSlideHash: string = null;
	private nextPrefetchedSlideHash: string = null;
	private slideRequestTimeout: any = null;
	private resolutionWidth: number = 960;
	private resolutionHeight: number = 540;
	private canvasWidth: number = 0;
	private canvasHeight: number = 0;
	private backgroundChecksums: Map<string, string> = new Map();
	private cachedBackgrounds: Map<string, ImageInfo> = new Map();
	private cachedMasterPages: Map<string, Array<LayerEntry>> = new Map();
	private cachedDrawPages: Map<string, Array<LayerEntry>> = new Map();
	private cachedTextFields: Map<string, TextFieldInfo> = new Map();
	private slideTextFieldsMap: Map<string, Map<string, string>> = new Map();
	private offscreenCanvas: OffscreenCanvas = null;
	private onSlideRenderingCompleteCallback: VoidFunction = null;
	private layerRenderer: LayerRenderer;
	private videoRenderers: Map<string, Array<VideoRenderer>> = new Map();
	private isTransitionActive: boolean = false;
	private queuedLayers: any[] = [];
	private isDecompressing: boolean = false;
	private decompressionQueue: any[] = [];
	private completionQueue: any[] = [];

	constructor(mapObj: any, helper: LayersCompositor) {
		this.map = mapObj;
		this.helper = helper;
	}

	addHooks() {
		this.map.on('slidelayer', this.onSlideLayerMsg, this);
		this.map.on('sliderenderingcomplete', this.onSlideRenderingComplete, this);
		this.map.on('compressedslide', this.onCompressSlide, this);
	}

	removeHooks() {
		this.map.off('slidelayer', this.onSlideLayerMsg, this);
		this.map.off('sliderenderingcomplete', this.onSlideRenderingComplete, this);
		this.map.off('compressedslide', this.onCompressSlide, this);
	}

	public onCompressSlide(e: any) {
		app.console.log(
			`CompressedCache: Storing compressed slide in LayerDrawing cache - slideHash: ${e.slideHash}, layers: ${e.layers.length}`,
		);
		this.compressedSlideCache.set(e.slideHash, e.layers);
	}

	public isDisposed() {
		return this.layerRenderer && this.layerRenderer.isDisposed();
	}
	public deleteResources() {
		this.requestedSlideHash = null;
		this.prefetchedSlideHash = null;
		this.nextRequestedSlideHash = null;
		this.nextPrefetchedSlideHash = null;
		this.deleteVideosResources();
		this.layerRenderer.dispose();
		this.compressedSlideCache.clear();
	}

	private getSlideInfo(slideHash: string): SlideInfo {
		return this.helper.getSlideInfo(slideHash);
	}

	private getDocWidth(): number {
		return this.helper.getDocWidth();
	}

	private getDocHeight(): number {
		return this.helper.getDocHeight();
	}

	private setDocWidth(slideWidth: number): void {
		this.helper.setDocWidth(slideWidth);
	}

	private setDocHeight(slideHeight: number): void {
		this.helper.setDocHeight(slideHeight);
	}

	public getSlide(slideNumber: number): ImageBitmap {
		const startSlideHash = this.helper.getSlideHash(slideNumber);
		return this.slideCache.get(startSlideHash);
	}

	public getLayerBounds(
		slideHash: string,
		targetElement: string,
	): BoundingBoxType {
		const layers = this.cachedDrawPages.get(slideHash);
		if (!layers) return null;
		for (const i in layers) {
			const animatedInfo = layers[i].content as AnimatedShapeInfo;
			if (
				animatedInfo &&
				animatedInfo.hash === targetElement &&
				animatedInfo.content
			)
				return animatedInfo.bounds;
		}
		return null;
	}

	public getAnimatedSlide(slideIndex: number): ImageBitmap {
		app.console.debug(
			'LayerDrawing.getAnimatedSlide: slide index: ' + slideIndex,
		);
		const slideHash = this.helper.getSlideHash(slideIndex);
		this.composeLayers(slideHash);
		return this.offscreenCanvas.transferToImageBitmap();
	}

	public composeLayers(slideHash: string): void {
		if (this.isDisposed()) return;

		this.drawBackground(slideHash);
		this.drawMasterPage(slideHash);
		this.drawTextField(slideHash);
		this.drawDrawPage(slideHash);
		this.drawVideos(slideHash);
	}

	private handleVideos(slideHash: string) {
		const slideInfo = this.getSlideInfo(slideHash);
		const videosInfo = slideInfo.videos;
		if (!videosInfo || videosInfo.length === 0) return;
		this.videoRenderers.set(slideHash, []);

		if (
			!this.layerRenderer.getRenderContext().is2dGl() &&
			!VideoRendererGl.videoProgramInitialized
		) {
			VideoRendererGl.createProgram(this.layerRenderer.getRenderContext());
		}

		for (let i = 0; i < videosInfo.length; ++i) {
			const videoInfo = videosInfo[i];
			this.handleVideo(i, slideHash, videoInfo);
		}
	}

	private handleVideo(index: number, slideHash: string, videoInfo: VideoInfo) {
		const slideShowPresenter = app.map.slideShowPresenter;
		const slideRenderer = slideShowPresenter._slideRenderer;
		const metaPres = slideShowPresenter._metaPresentation;

		const videoId = slideHash + index;
		const videoRenderer = makeVideoRenderer(
			videoId,
			this.layerRenderer.getRenderContext(),
			slideRenderer,
		);
		videoRenderer.prepareVideo(
			videoInfo,
			metaPres.docWidth,
			metaPres.docHeight,
		);

		this.videoRenderers.get(slideHash).push(videoRenderer);
	}

	private drawVideos(slideHash: string) {
		const videoRenderers = this.videoRenderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) {
			videoRenderer.render();
		}
	}

	public loadVideos(slideHash: string) {
		const videoRenderers = this.videoRenderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) {
			videoRenderer.loadVideo();
		}
	}

	public pauseVideos(slideHash: string) {
		const videoRenderers = this.videoRenderers.get(slideHash);
		if (!videoRenderers) return;

		for (const videoRenderer of videoRenderers) {
			videoRenderer.pauseVideo();
		}
	}

	public getVideoRenderer(
		slideHash: string,
		videoInfo: VideoInfo,
	): VideoRenderer {
		const videoRenderers = this.videoRenderers.get(slideHash);

		for (const videoRenderer of videoRenderers) {
			if (videoRenderer.videoInfoId === videoInfo.id) {
				return videoRenderer;
			}
		}
	}

	private deleteVideosResources() {
		this.videoRenderers.forEach((videoRenderers) => {
			for (const videoRenderer of videoRenderers) {
				videoRenderer.deleteResources();
			}
		});
		VideoRendererGl.deleteProgram(this.layerRenderer.getRenderContext());
	}

	public getAnimatedLayerInfo(
		slideHash: string,
		targetElement: string,
	): AnimatedShapeInfo {
		app.console.debug(
			`LayerDrawing.getAnimatedLayerInfo(${slideHash}, ${targetElement})`,
		);
		const layers = this.cachedDrawPages.get(slideHash);
		if (!layers) return null;
		for (const i in layers) {
			const animatedInfo = layers[i].content as AnimatedShapeInfo;
			if (animatedInfo && animatedInfo.hash === targetElement)
				return animatedInfo;
		}
		return null;
	}

	public getLayerImage(slideHash: string, targetElement: string): ImageBitmap {
		const layers = this.cachedDrawPages.get(slideHash);
		if (!layers) return null;
		for (const i in layers) {
			const animatedInfo = layers[i].content as AnimatedShapeInfo;
			if (
				animatedInfo &&
				animatedInfo.hash === targetElement &&
				animatedInfo.content
			)
				return (animatedInfo.content as ImageInfo).data;
		}
		return null;
	}

	public invalidateAll(): void {
		this.slideCache.invalidateAll();
		this.slideTextFieldsMap.clear();
		this.cachedTextFields.clear();
		this.cachedBackgrounds.clear();
		this.cachedMasterPages.clear();
		this.cachedDrawPages.clear();
		this.videoRenderers.clear();
		this.compressedSlideCache.clear();
	}

	public getCanvasSize(): [number, number] {
		return [this.canvasWidth, this.canvasHeight];
	}

	public onUpdatePresentationInfo() {
		this.computeInitialResolution();
		this.initializeCanvas();
	}

	public requestSlide(slideNumber: number, callback: VoidFunction) {
		this.onSlideRenderingCompleteCallback = callback;

		const startSlideHash = this.helper.getSlideHash(slideNumber);
		this.requestSlideImpl(startSlideHash);
	}

	private initializeCanvas() {
		this.computeCanvasSize(this.resolutionWidth, this.resolutionHeight);
		this.createRenderingCanvas();
	}

	private createRenderingCanvas() {
		this.offscreenCanvas = new OffscreenCanvas(
			this.canvasWidth,
			this.canvasHeight,
		);

		try {
			this.layerRenderer = new SlideShow.LayerRendererGl(this.offscreenCanvas);
		} catch (error) {
			app.console.log('LayerDrawing: WebGl offscreen rendering not supported');
			this.layerRenderer = new SlideShow.LayerRenderer2d(this.offscreenCanvas);
		}
	}

	private requestSlideImpl(
		slideHash: string,
		prefetch: boolean = false,
		compressedLayers: boolean = false,
	) {
		if (this.isDisposed()) return;

		app.console.debug(
			'LayerDrawing.requestSlideImpl: slide hash: ' +
				slideHash +
				', prefetching: ' +
				prefetch,
		);
		const slideInfo = this.getSlideInfo(slideHash);
		if (!slideInfo) {
			window.app.console.log(
				'LayerDrawing.requestSlideImpl: No info for requested slide: hash: ' +
					slideHash,
			);
			return;
		}
		if (
			slideHash === this.requestedSlideHash ||
			slideHash === this.prefetchedSlideHash ||
			slideHash === this.nextRequestedSlideHash ||
			slideHash === this.nextPrefetchedSlideHash
		) {
			// A non-prefetch request for a slide that is already being
			// prefetched: upgrade to a regular request so the callback
			// fires when the data arrives.
			if (!prefetch && slideHash === this.prefetchedSlideHash) {
				this.requestedSlideHash = this.prefetchedSlideHash;
				this.prefetchedSlideHash = null;
			}
			app.console.debug(
				'LayerDrawing.requestSlideImpl: no need to fetch slide again',
			);
			return;
		}

		if (
			this.requestedSlideHash ||
			this.prefetchedSlideHash ||
			this.slideRequestTimeout
		) {
			if (!prefetch || !this.slideRequestTimeout) {
				if (!prefetch) {
					// maybe user has switched to a new slide
					clearTimeout(this.slideRequestTimeout);
					this.nextRequestedSlideHash = slideHash;
					this.nextPrefetchedSlideHash = null;
				} else {
					// prefetching and nothing already queued
					this.nextPrefetchedSlideHash = slideHash;
				}
				this.slideRequestTimeout = setTimeout(() => {
					if (!this.helper.isSlideShowPlaying()) return;
					this.slideRequestTimeout = null;
					this.nextRequestedSlideHash = null;
					this.nextPrefetchedSlideHash = null;

					this.requestSlideImpl(slideHash, prefetch);
				}, 500);
			}
			return;
		}

		if (prefetch) {
			this.prefetchedSlideHash = slideHash;
			this.requestedSlideHash = null;
		} else {
			this.requestedSlideHash = slideHash;
			this.prefetchedSlideHash = null;
		}

		if (this.slideCache.has(slideHash)) {
			this.onSlideRenderingComplete({ success: true });
			return;
		}

		if (this.compressedSlideCache.has(slideHash)) {
			app.console.log(
				'CompressedCache: fetching slides from CompressedCache, SlideHash :',
				slideHash,
			);
			SlideBitmapManager.renderCachedCompressedSlide(
				this.compressedSlideCache.get(slideHash),
			);
			return;
		}

		this.requestSlideFromServer(slideInfo, prefetch, compressedLayers);
	}

	private requestSlideFromServer(
		slideInfo: SlideInfo,
		prefetch: boolean = false,
		compressedLayers: boolean = false,
	) {
		if (
			slideInfo.slideWidth > this.getDocWidth() ||
			slideInfo.slideHeight > this.getDocHeight()
		) {
			this.setDocWidth(slideInfo.slideWidth);
			this.setDocHeight(slideInfo.slideHeight);
			this.onUpdatePresentationInfo();
		}
		const slideHash = slideInfo.hash;
		const backgroundRendered = this.drawBackground(slideHash);
		const masterPageRendered = this.drawMasterPage(slideHash);
		if (backgroundRendered && masterPageRendered) {
			if (this.drawDrawPage(slideHash)) {
				this.onSlideRenderingComplete({ success: true });
				return;
			}
		}

		if (!prefetch)
			this.map.fire('handleslideshowprogressbar', { isVisible: true });

		app.socket.sendMessage(
			`getslide hash=${slideInfo.hash} part=${slideInfo.index} width=${this.canvasWidth} height=${this.canvasHeight} ` +
				`renderBackground=${backgroundRendered ? 0 : 1} renderMasterPage=${masterPageRendered ? 0 : 1} devicePixelRatio=${window.devicePixelRatio} compressedLayers=${compressedLayers ? 1 : 0} uniqueID=${slideInfo.uniqueID}`,
		);
	}

	onSlideLayerMsg(e: any) {
		if (this.isDisposed()) return;

		if (this.isTransitionActive) {
			this.queuedLayers.push(e);
			return;
		}
		if (this.isDecompressing) {
			this.decompressionQueue.push(e);
			return;
		}

		const info = e.message;
		if (!info) {
			window.app.console.log(
				'LayerDrawing.onSlideLayerMsg: no json data available.',
			);
			return;
		}
		if (!this.getSlideInfo(info.slideHash)) {
			window.app.console.log(
				'LayerDrawing.onSlideLayerMsg: no slide info available for ' +
					info.slideHash +
					'.',
			);
			return;
		}
		if (!info.content) {
			window.app.console.log(
				'LayerDrawing.onSlideLayerMsg: no layer content available.',
			);
			return;
		}

		if (e.imgBytes) {
			const promise = SlideBitmapManager.decompressSlideLayer(info, e.imgBytes);
			if (promise) {
				this.isDecompressing = true;
				promise
					.then((img: ImageBitmap) => {
						this.handleMsg(info, img);
					})
					.finally(() => {
						this.isDecompressing = false;
						while (this.decompressionQueue.length) {
							this.onSlideLayerMsg(this.decompressionQueue.shift());
							if (this.isDecompressing) return;
						}
						while (this.completionQueue.length)
							this.onSlideRenderingComplete(this.completionQueue.shift());
					});
			}
			return;
		} else this.handleMsg(info, e.image);
	}

	handleMsg(info: LayerInfo, img: any) {
		switch (info.group) {
			case 'Background':
				this.handleBackgroundMsg(info, img);
				break;
			case 'MasterPage':
				this.handleMasterPageLayerMsg(info, img);
				break;
			case 'DrawPage':
				this.handleDrawPageLayerMsg(info, img);
				break;
			case 'TextFields':
				this.handleTextFieldMsg(info, img);
		}
	}

	handleTextFieldMsg(info: LayerInfo, img: any) {
		const textFieldInfo = info.content as TextFieldInfo;
		const imageInfo = textFieldInfo.content;
		if (!this.checkAndAttachImageData(imageInfo, img)) return;

		let textFields = this.slideTextFieldsMap.get(info.slideHash);
		if (!textFields) {
			textFields = new Map<string, string>();
			this.slideTextFieldsMap.set(info.slideHash, textFields);
		}
		textFields.set(textFieldInfo.hash, imageInfo.checksum);

		this.cachedTextFields.set(imageInfo.checksum, textFieldInfo);
	}

	private handleBackgroundMsg(info: LayerInfo, img: any) {
		const slideInfo = this.getSlideInfo(info.slideHash);
		if (!slideInfo.background) {
			return;
		}
		if (info.type === 'bitmap') {
			const imageInfo = info.content as ImageInfo;
			if (!this.checkAndAttachImageData(imageInfo, img)) return;

			const pageHash = slideInfo.background.isCustom
				? info.slideHash
				: slideInfo.masterPage;
			this.backgroundChecksums.set(pageHash, imageInfo.checksum);
			this.cachedBackgrounds.set(imageInfo.checksum, imageInfo);

			this.clearCanvas();
			this.drawBitmap(imageInfo);
		}
	}

	private handleMasterPageLayerMsg(info: LayerInfo, img: any) {
		const slideInfo = this.getSlideInfo(info.slideHash);
		if (!slideInfo.masterPageObjectsVisibility) {
			return;
		}

		if (info.index === 0 || !this.cachedMasterPages.get(slideInfo.masterPage))
			this.cachedMasterPages.set(slideInfo.masterPage, new Array<LayerEntry>());

		const layers = this.cachedMasterPages.get(slideInfo.masterPage);
		if (layers.length !== info.index) {
			window.app.console.log(
				'LayerDrawing.handleMasterPageLayerMsg: missed any layers ?',
			);
		}
		const layerEntry: LayerEntry = {
			type: info.type,
			content: info.content,
			isField: info.isField,
		};
		if (info.type === 'bitmap') {
			if (!this.checkAndAttachImageData(layerEntry.content as ImageInfo, img))
				return;
		}
		layers.push(layerEntry);

		this.drawMasterPageLayer(layerEntry, info.slideHash);
	}

	private handleDrawPageLayerMsg(info: LayerInfo, img: any) {
		if (info.index === 0 || !this.cachedDrawPages.get(info.slideHash)) {
			this.cachedDrawPages.set(info.slideHash, new Array<LayerEntry>());
		}
		const layers = this.cachedDrawPages.get(info.slideHash);
		if (layers.length !== info.index) {
			window.app.console.log(
				'LayerDrawing.handleDrawPageLayerMsg: missed any layers ?',
			);
		}
		const layerEntry: LayerEntry = {
			type: info.type,
			content: info.content,
		};
		if (info.type === 'bitmap') {
			if (!this.checkAndAttachImageData(layerEntry.content as ImageInfo, img))
				return;
		} else if (info.type === 'animated') {
			const content = layerEntry.content as AnimatedShapeInfo;
			if (content.type === 'bitmap') {
				if (!this.checkAndAttachImageData(content.content as ImageInfo, img))
					return;
				const animatedElement = this.helper.getAnimatedElement(
					info.slideHash,
					content.hash,
				);
				if (animatedElement) {
					animatedElement.updateAnimationInfo(content);
				}
			}
		}
		layers.push(layerEntry);

		this.drawDrawPageLayer(info.slideHash, layerEntry);
	}

	private clearCanvas() {
		this.layerRenderer.clearCanvas();
	}
	private drawBackground(slideHash: string) {
		this.clearCanvas();

		const slideInfo = this.getSlideInfo(slideHash);

		if (this.layerRenderer.fillColor(slideInfo)) return true;

		const pageHash = slideInfo.background.isCustom
			? slideHash
			: slideInfo.masterPage;
		const checksum = this.backgroundChecksums.get(pageHash);
		if (!checksum) return false;

		const imageInfo = this.cachedBackgrounds.get(checksum);
		if (!imageInfo) {
			window.app.console.log(
				'LayerDrawing: no cached background for slide: ' + slideHash,
			);
			return false;
		}

		this.drawBitmap(imageInfo);
		return true;
	}

	private drawMasterPage(slideHash: string) {
		const slideInfo = this.getSlideInfo(slideHash);
		if (!slideInfo.masterPageObjectsVisibility) return true;

		const layers = this.cachedMasterPages.get(slideInfo.masterPage);
		if (!layers || layers.length === 0) {
			window.app.console.log(
				'LayerDrawing: No layer cached for master page: ' +
					slideInfo.masterPage,
			);
			return false;
		}

		for (const layer of layers) {
			this.drawMasterPageLayer(layer, slideHash);
		}

		return true;
	}

	private drawMasterPageLayer(layer: LayerEntry, slideHash: string) {
		if (layer.type === 'bitmap') {
			this.drawBitmap(layer.content as ImageInfo);
		} else if (layer.type === 'placeholder') {
			const placeholder = layer.content as PlaceholderInfo;
			const slideTextFields = this.slideTextFieldsMap.get(slideHash);
			const checksum = slideTextFields
				? slideTextFields.get(placeholder.hash)
				: null;
			if (!checksum) {
				window.app.console.log(
					'LayerDrawing: No content found for text field placeholder, type: ' +
						placeholder.type,
				);
				return;
			}
			const imageInfo = this.cachedTextFields.get(checksum).content;
			this.drawBitmap(imageInfo);
		}
	}

	private drawDrawPage(slideHash: string) {
		const slideInfo = this.getSlideInfo(slideHash);
		if (slideInfo.empty) {
			return true;
		}

		const layers = this.cachedDrawPages.get(slideHash);
		if (!layers || layers.length === 0) {
			window.app.console.log(
				'LayerDrawing: No layer cached for draw page: ' + slideHash,
			);
			return false;
		}

		for (const layer of layers) {
			this.drawDrawPageLayer(slideHash, layer);
		}
		return true;
	}

	private drawTextField(slideHash: string) {
		const slideInfo = this.getSlideInfo(slideHash);
		if (slideInfo.empty) {
			return true;
		}

		const fields = this.slideTextFieldsMap.get(slideHash);
		if (!fields) {
			window.app.console.log(
				'LayerDrawing: No layer cached text field for draw page: ' + slideHash,
			);
			return false;
		}

		for (const field of fields) {
			const imageInfo = this.cachedTextFields.get(field[1]).content;
			this.drawBitmap(imageInfo);
		}
		return true;
	}

	private drawDrawPageLayer(slideHash: string, layer: LayerEntry) {
		if (layer.type === 'bitmap') {
			this.drawBitmap(layer.content as ImageInfo);
		} else if (layer.type === 'animated') {
			const content = layer.content as AnimatedShapeInfo;
			if (content.type === 'bitmap') {
				const animatedElement = this.helper.getAnimatedElement(
					slideHash,
					content.hash,
				);
				if (animatedElement) {
					app.console.debug(
						'LayerDrawing.drawDrawPageLayer: retrieved animatedElement',
					);
					if (animatedElement.isValid()) {
						animatedElement.renderLayer(this.layerRenderer);
						return;
					}
				}
				this.drawBitmap(content.content as ImageInfo);
			}
		}
	}

	private drawBitmap(imageInfo: ImageInfo | ImageBitmap) {
		this.layerRenderer.drawBitmap(imageInfo);
	}

	onSlideRenderingComplete(e: any) {
		if (this.isDisposed()) return;
		if (this.isDecompressing) {
			this.completionQueue.push(e);
			return;
		}

		if (e.message) {
			const promise = SlideBitmapManager.waitForSlideDecompression(e.message);
			if (promise) {
				promise.then(() => {
					app.map.fire('sliderenderingcomplete', {
						success: e.success,
						compressedLayers: e.message.compressedLayers,
					});
				});
				return;
			}
		}

		this.map.fire('handleslideshowprogressbar', { isVisible: false });

		const slideHash = this.requestedSlideHash || this.prefetchedSlideHash;
		const slideInfo = this.getSlideInfo(slideHash);
		const index = slideInfo ? slideInfo.index : undefined;

		if (!e.success) {
			this.requestedSlideHash = null;
			this.prefetchedSlideHash = null;
			app.console.debug(
				'LayerDrawing.onSlideRenderingComplete: rendering failed for slide: ' +
					index,
			);
			return;
		}

		if (this.prefetchedSlideHash) {
			var currSlideHash = this.prefetchedSlideHash;
			this.prefetchedSlideHash = null;
			if (e.compressedLayers) {
				this.compressedPrefetchSlideHash = currSlideHash;
				this.prefetchNextCompressedSlide();
			}
			return;
		}

		if (e.compressedLayers) {
			this.prefetchNextCompressedSlide();
			return;
		}

		const reqSlideInfo = this.getSlideInfo(this.requestedSlideHash);

		this.cacheAndNotify();

		// fetch next slide and draw it on offscreen canvas
		if (reqSlideInfo?.next && !this.slideCache.has(reqSlideInfo.next)) {
			this.requestSlideImpl(reqSlideInfo.next, true);
		}
	}

	private cacheAndNotify() {
		if (!this.offscreenCanvas) {
			window.app.console.log(
				'LayerDrawing.onSlideRenderingComplete: no offscreen canvas available.',
			);
			return;
		}
		if (!this.slideCache.has(this.requestedSlideHash)) {
			const renderedSlide = this.offscreenCanvas.transferToImageBitmap();
			this.slideCache.set(this.requestedSlideHash, renderedSlide);
		}
		this.compressedSlideCache.delete(this.requestedSlideHash);
		this.requestedSlideHash = null;

		const oldCallback = this.onSlideRenderingCompleteCallback;
		this.onSlideRenderingCompleteCallback = null;
		if (oldCallback)
			// if we already closed presentation it is missing
			oldCallback.call(this.helper);
	}

	private prefetchNextCompressedSlide(): void {
		if (this.compressedPrefetchSlideHash) {
			const slideInfo = this.getSlideInfo(this.compressedPrefetchSlideHash);
			this.compressedPrefetchSlideHash = null;

			if (slideInfo?.next) {
				this.prefetchCompressedSlide(slideInfo.next);
			}
		}
	}

	private prefetchCompressedSlide(slideHash: string) {
		const slideInfo = this.getSlideInfo(slideHash);
		if (!slideInfo) {
			app.console.warn(`SlideInfo not found for hash: ${slideHash}`);
			return;
		}

		if (this.isSlideAlreadyCached(slideHash)) {
			this.prefetchNextSlide(slideInfo);
			return;
		}
		this.compressedPrefetchSlideHash = slideHash;
		this.requestSlideFromServer(slideInfo, true, true);
		return;
	}

	private isSlideAlreadyCached(slideHash: string): boolean {
		return (
			this.slideCache.has(slideHash) || this.compressedSlideCache.has(slideHash)
		);
	}

	private prefetchNextSlide(slideInfo: SlideInfo): void {
		if (slideInfo.next) {
			this.prefetchCompressedSlide(slideInfo.next);
		}
	}

	private checkAndAttachImageData(imageInfo: ImageInfo, img: any): boolean {
		if (!img || (imageInfo.type === 'png' && !img.src)) {
			window.app.console.log(
				'LayerDrawing.checkAndAttachImageData: no bitmap available.',
			);
			return false;
		}
		imageInfo.data = img;
		return true;
	}

	private computeInitialResolution() {
		// window.screen dimensions are in CSS pixels; multiply by DPR so the
		// tier selection matches the display's physical pixels and the canvas
		// backing store isn't upscaled by CSS on HiDPI screens.
		const dpr = window.devicePixelRatio || 1;
		const viewWidth = window.screen.width * dpr;
		const viewHeight = window.screen.height * dpr;
		this.computeResolution(viewWidth, viewHeight);
	}

	private computeResolution(viewWidth: number, viewHeight: number) {
		[this.resolutionWidth, this.resolutionHeight] =
			this.helper.computeLayerResolution(viewWidth, viewHeight);
	}

	private computeCanvasSize(resWidth: number, resHeight: number) {
		[this.canvasWidth, this.canvasHeight] = this.helper.computeLayerSize(
			resWidth,
			resHeight,
		);
	}

	public getLayerRendererContext(): RenderContext {
		return this.layerRenderer.getRenderContext();
	}

	public notifyTransitionStart() {
		this.isTransitionActive = true;
		this.queuedLayers = [];
	}

	public notifyTransitionEnd(slideHash: string) {
		this.isTransitionActive = false;
		while (this.queuedLayers.length > 0) {
			const layer = this.queuedLayers.shift();
			this.onSlideLayerMsg(layer);
		}

		this.handleVideos(slideHash);
		if (this.videoRenderers.has(slideHash)) {
			this.loadVideos(slideHash);
		}
	}
}

SlideShow.LayerDrawing = LayerDrawing;
