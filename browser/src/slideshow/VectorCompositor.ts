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
 * A shape effect redraws the slide from the primitive tree on every frame,
 * each target in the state its element holds. The videos of a slide draw
 * over it once its transition has ended.
 */

/* global app RenderManager */

declare var SlideShow: any;

class VectorCompositor extends SlideCompositor {
	private offscreenCanvas: OffscreenCanvas = null;
	private offscreenContext: OffscreenCanvasRenderingContext2D = null;
	// The 2d render context over the offscreen canvas.
	private renderContext: RenderContext2d = null;
	private videos = new SlideVideos();
	private canvasWidth = 0;
	private canvasHeight = 0;

	private disposed = false;
	// The listener registered on the render manager, kept to remove it.
	private onDataChanged: (() => void) | null = null;

	// The slide the initial fetchAndRun is waiting on, and the callback to
	// fire once its primitive tree is in the cache.
	private pendingSlideNumber: number = null;
	private pendingReady: VoidFunction = null;

	// The layers of the slide on the GPU, in the WebGL context of the slideshow canvas, for
	// the frames drawn straight into it.
	private layers: VectorSlideLayers = null;

	constructor(slideShowPresenter: SlideShowPresenter) {
		super(slideShowPresenter);
	}

	protected _addHooks(): void {
		// The shared cache fires this when a tree or an embedded bitmap
		// arrives. Use it to release the initial fetchAndRun once its
		// slide is ready.
		this.onDataChanged = () => this._onDataChanged();
		RenderManager.onVectorChanged(this.onDataChanged);
	}

	public removeHooks(): void {
		if (this.onDataChanged) RenderManager.offVectorChanged(this.onDataChanged);
		this.onDataChanged = null;
	}

	public onUpdatePresentationInfo(): void {
		// The videos drew into the old canvas. Entering the slide makes them anew.
		this.videos.deleteResources();
		this.videos.clear();

		[this.canvasWidth, this.canvasHeight] = this._computeCanvasSize();
		this.offscreenCanvas = new OffscreenCanvas(
			this.canvasWidth,
			this.canvasHeight,
		);
		this.offscreenContext = this.offscreenCanvas.getContext('2d');
		this.renderContext = new RenderContext2d(this.offscreenCanvas);
		this.layers?.invalidate();
		// A compositor whose slideshow was closed draws the next one.
		this.disposed = false;

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
		// A frame of an effect and the slide itself compose the same way.
		return this._composeSlide(slideIndex);
	}

	// --- fetching ---------------------------------------------------

	private _fetchAllSlides(): void {
		this._metaPresentation.getMetaSlides().forEach((_metaSlide, hash) => {
			const info = this._metaPresentation.getSlideInfo(hash);
			if (info) {
				RenderManager.requestPartById(info.part, cool.VectorMode.Slides);
			}
		});
	}

	/// The id of the page that shows the slide. The slideshow can run from
	/// any view, and the slide info names each slide by its id.
	private _partIdForSlide(slideNumber: number): cool.VectorPartGuid | null {
		const hash = this._metaPresentation.getSlideHash(slideNumber);
		return this._partIdForSlideHash(hash);
	}

	private _partIdForSlideHash(slideHash: string): cool.VectorPartGuid | null {
		const info = slideHash
			? this._metaPresentation.getSlideInfo(slideHash)
			: null;
		return info ? info.part : null;
	}

	private _runWhenReady(slideNumber: number, ready: VoidFunction): void {
		const partId = this._partIdForSlide(slideNumber);
		if (
			partId !== null &&
			RenderManager.isPartDrawableById(partId, cool.VectorMode.Slides)
		) {
			ready();
			return;
		}
		this.pendingSlideNumber = slideNumber;
		this.pendingReady = ready;
	}

	private _onDataChanged(): void {
		// A tree or an embedded bitmap that arrives can change what a layer
		// of the slide shows.
		this.layers?.invalidate();
		if (this.disposed) return;
		this._refreshShownSlide();
		if (this.pendingReady === null) return;

		const partId = this._partIdForSlide(this.pendingSlideNumber);
		if (
			partId === null ||
			!RenderManager.isPartDrawableById(partId, cool.VectorMode.Slides)
		)
			return;

		const ready = this.pendingReady;
		this.pendingReady = null;
		this.pendingSlideNumber = null;
		ready();
	}

	// An element without a base takes it from the tree that arrived. Every
	// slide is checked, since a transition enters a slide before it is the
	// one drawn last.
	private _refreshShownSlide(): void {
		this._metaPresentation.getMetaSlides().forEach((_metaSlide, slideHash) => {
			if (!this._drawableData(slideHash)) return;
			this.getAnimatedElementMap(slideHash)?.forEach((element) =>
				element.refreshContent(),
			);
		});
		this._slideShowPresenter._slideRenderer?.requestRender();
	}

	// --- composing --------------------------------------------------

	private _composeSlide(slideNumber: number): ImageBitmap {
		if (this.disposed || !this.offscreenContext) return null;

		const slideHash = this._metaPresentation.getSlideHash(slideNumber);
		const data = this._drawableData(slideHash);
		if (!data) return null;

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
		const states = this._drawStates(slideHash, data);
		// A slideshow shows the document in its own colors.
		RenderManager.renderInto(
			context as unknown as CanvasRenderingContext2D,
			data,
			{ authoredColors: true, states },
		);
		context.restore();

		// The videos are placed on the canvas in pixels, each in the state
		// of its shape.
		this.videos.render(slideHash, (shapeId) =>
			this._videoLook(data, states, shapeId),
		);

		return this.offscreenCanvas.transferToImageBitmap();
	}

	// How the video of the shape is drawn: in the state of the shape.
	// Undefined where no animation moves the shape.
	private _videoLook(
		data: cool.VectorPrimitivesData,
		states: Map<number, cool.ObjectRenderState> | undefined,
		shapeId: number,
	): AnimatedContentLook | undefined {
		if (!states) return undefined;
		const state = VectorManager.stateOf(data, shapeId, states);
		if (!state) return undefined;
		return {
			state,
			slideWidth: data.slideWidth,
			slideHeight: data.slideHeight,
		};
	}

	// The object id of each animation target of the slide, with the positions
	// of the paragraphs of its text that an animation moves on their own. A
	// video counts as a target, so its frame draws in its place in the paint
	// order, unless a group above it is a target and its run holds the video.
	private _animatedTargets(
		slideHash: string,
		data: cool.VectorPrimitivesData,
	): Map<number, number[]> {
		const targets = new Map<number, number[]>();
		const elements = this.getAnimatedElementMap(slideHash);
		if (elements) {
			for (const target of elements.keys()) {
				const { id, paragraph } = VectorCompositor._splitTarget(target);
				if (!data.objects.has(id)) continue;
				if (!targets.has(id)) targets.set(id, []);
				if (paragraph !== undefined) targets.get(id).push(paragraph);
			}
		}
		targets.forEach((paragraphs) => paragraphs.sort((a, b) => a - b));

		for (const video of this._metaPresentation.getSlideInfo(slideHash)
			?.videos ?? []) {
			const id = video.id;
			if (!data.objects.has(id) || targets.has(id)) continue;
			let above = data.objects.get(id)?.parent;
			while (above !== undefined && !targets.has(above))
				above = data.objects.get(above)?.parent;
			if (above === undefined) targets.set(id, []);
		}
		return targets;
	}

	// The frame each video shows now, by the object id of its shape. A video
	// without a frame yet leaves its placeholder showing.
	private _videoFrames(
		slideHash: string,
		data: cool.VectorPrimitivesData,
	): Map<number, ContentFrame> {
		const frames = new Map<number, ContentFrame>();
		for (const video of this._metaPresentation.getSlideInfo(slideHash)
			?.videos ?? []) {
			if (!data.objects.has(video.id)) continue;
			const frame = this.videos.get(slideHash, video)?.currentFrame();
			if (frame) frames.set(video.id, frame);
		}
		return frames;
	}

	public drawsFrames(): boolean {
		return !this.disposed;
	}

	public drawFrameInto(
		context: RenderContextGl,
		slideIndex: number,
		texture?: WebGLTexture | null,
	): boolean {
		if (this.disposed || context.isDisposed()) return false;

		const slideHash = this._metaPresentation.getSlideHash(slideIndex);
		const data = this._drawableData(slideHash);
		if (!data) return false;

		if (this.layers && this.layers.context !== context) {
			this.layers.dispose();
			this.layers = null;
		}
		if (!this.layers) this.layers = new VectorSlideLayers(context);
		if (
			!this.layers.holds(slideHash, data, this.canvasWidth, this.canvasHeight)
		)
			this.layers.build(
				slideHash,
				data,
				this._animatedTargets(slideHash, data),
				this.canvasWidth,
				this.canvasHeight,
			);
		return this.layers.draw(
			this._drawStates(slideHash, data),
			texture,
			this._videoFrames(slideHash, data),
		);
	}

	/// The cached tree of the slide, or null until the slide and its master,
	/// which holds the background and footers, have arrived.
	private _drawableData(slideHash: string): cool.VectorPrimitivesData | null {
		const partId = this._partIdForSlideHash(slideHash);
		if (partId === null) return null;
		if (!RenderManager.isPartDrawableById(partId, cool.VectorMode.Slides))
			return null;
		const data = RenderManager.requestPartById(partId, cool.VectorMode.Slides);
		if (!data || data.slideWidth <= 0 || data.slideHeight <= 0) return null;
		return data;
	}

	// The state of every animated target by object id. A paragraph target
	// is the shape id, an underscore and the paragraph position. A target
	// without a base rectangle is drawn as it comes.
	private _drawStates(
		slideHash: string,
		data: cool.VectorPrimitivesData,
	): Map<number, cool.ObjectRenderState> | undefined {
		const elements = this.getAnimatedElementMap(slideHash);
		if (!elements || elements.size === 0) return undefined;

		const states = new Map<number, cool.ObjectRenderState>();
		elements.forEach((element, target) => {
			if (!element.isValid()) return;
			const { id: objectId, paragraph } = VectorCompositor._splitTarget(target);
			if (!data.objects.has(objectId)) return;

			let state = states.get(objectId);
			if (!state) {
				state = {};
				states.set(objectId, state);
			}
			const drawState = element.getDrawState();
			if (paragraph === undefined) {
				Object.assign(state, drawState);
			} else {
				if (!state.paragraphs) state.paragraphs = new Map();
				state.paragraphs.set(paragraph, drawState);
			}
		});
		return states;
	}

	// A shape target is its object id, a paragraph target the id, an
	// underscore and the paragraph position.
	private static _splitTarget(target: string): {
		id: number;
		paragraph: number | undefined;
	} {
		const separator = target.indexOf('_');
		if (separator < 0) return { id: Number(target), paragraph: undefined };
		return {
			id: Number(target.substring(0, separator)),
			paragraph: Number(target.substring(separator + 1)),
		};
	}

	// --- animation and video ----------------------------------------

	public createAnimatedElement(
		sId: string,
		slideHash: string,
		title: string,
		slideWidth: number,
		slideHeight: number,
		bIsTextElement: boolean,
	): AnimatedElement {
		return new AnimatedElementVector(
			this,
			sId,
			slideHash,
			title,
			slideWidth,
			slideHeight,
			bIsTextElement,
		);
	}

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

	/// The box of the shape or the bounds of the paragraph, in twips. Null
	/// without a cached tree or such a target.
	public getLayerBounds(
		slideHash: string,
		targetElement: string,
	): BoundingBoxType {
		const target = this._findTarget(slideHash, targetElement);
		if (!target) return null;

		if (target.paragraph) {
			const bounds = target.paragraph.bounds;
			if (!bounds) return null;
			return new DOMRect(
				bounds[0],
				bounds[1],
				bounds[2] - bounds[0],
				bounds[3] - bounds[1],
			);
		}

		const object = target.object;
		if (object.x === undefined || object.width === undefined) return null;
		return new DOMRect(object.x, object.y, object.width, object.height);
	}

	/// The target's colors as #rrggbb: the solid fill and line of the shape,
	/// and the color of the first text run, of its own paragraph for a
	/// paragraph target. Absent where there is none or it is automatic. Null
	/// without a cached tree or such a target.
	public getTargetColors(
		slideHash: string,
		targetElement: string,
	): { fillColor?: string; lineColor?: string; fontColor?: string } | null {
		const target = this._findTarget(slideHash, targetElement);
		if (!target) return null;

		const text = target.paragraph
			? [target.paragraph]
			: cool.TextHierarchyParagraphPrimitive.collect(target.object.primitives);
		return {
			fillColor: target.object.properties?.fillColor,
			lineColor: target.object.properties?.lineColor,
			fontColor: VectorCompositor._firstFontColor(target.data, text),
		};
	}

	// The object the target names, its paragraph for a paragraph target,
	// and the cached part. Null without a cached tree or such an object.
	private _findTarget(
		slideHash: string,
		targetElement: string,
	): {
		data: cool.VectorPrimitivesData;
		object: cool.SlideObject;
		paragraph?: cool.TextHierarchyParagraphPrimitive;
	} | null {
		const partId = this._partIdForSlideHash(slideHash);
		if (partId === null) return null;
		const data = RenderManager.requestPartById(partId, cool.VectorMode.Slides);
		if (!data) return null;

		const { id, paragraph } = VectorCompositor._splitTarget(targetElement);
		const object = data.objects.get(id);
		if (!object) return null;
		if (paragraph === undefined) return { data, object };

		const paragraphNode = cool.TextHierarchyParagraphPrimitive.collect(
			object.primitives,
		)[paragraph];
		return paragraphNode ? { data, object, paragraph: paragraphNode } : null;
	}

	// The color of the first text run, automatic resolved against the
	// surface under it, or undefined without text.
	private static _firstFontColor(
		data: cool.VectorPrimitivesData,
		primitives: cool.Primitive[] | undefined,
	): string | undefined {
		if (!primitives) return undefined;
		for (const primitive of primitives) {
			if (
				primitive.type === cool.TextSimplePortionPrimitive.type ||
				primitive.type === cool.TextDecoratedPortionPrimitive.type
			)
				return RenderManager.textColorOf(
					data,
					primitive as cool.TextSimplePortionPrimitive,
				);
			const found = VectorCompositor._firstFontColor(data, primitive.children);
			if (found) return found;
		}
		return undefined;
	}

	public getLayerRendererContext(): RenderContext {
		return null;
	}

	public getVideoRenderer(
		slideHash: string,
		videoInfo: VideoInfo,
	): AnimatedContentRenderer {
		return this.videos.get(slideHash, videoInfo);
	}

	public pauseVideos(slideHash: string): void {
		this.videos.pause(slideHash);
	}

	// --- transitions and lifecycle ----------------------------------

	public notifyTransitionStart(): void {
		// The primitive trees are already cached, so there is nothing to
		// hold back while a transition runs.
	}

	public notifyTransitionEnd(slideHash: string): void {
		if (this.disposed || !this.renderContext) return;
		this.videos.enterSlide(
			slideHash,
			this._metaPresentation.getSlideInfo(slideHash),
			this.renderContext,
		);
	}

	public deleteResources(): void {
		this.disposed = true;
		this.videos.deleteResources();
		this.videos.clear();
		this.offscreenCanvas = null;
		this.offscreenContext = null;
		this.renderContext = null;
		this.layers?.dispose();
		this.layers = null;
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
