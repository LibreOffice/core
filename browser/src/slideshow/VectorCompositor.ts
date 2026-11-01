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
		// A slideshow shows the document in its own colors.
		RenderManager.renderInto(
			context as unknown as CanvasRenderingContext2D,
			data,
			{ authoredColors: true, states: this._drawStates(slideHash, data) },
		);
		context.restore();

		// The videos are placed on the canvas in pixels.
		this.videos.render(slideHash);

		return this.offscreenCanvas.transferToImageBitmap();
	}

	// The object id of each animation target of the slide, with the positions
	// of the paragraphs of its text that an animation moves on their own.
	private _animatedTargets(
		slideHash: string,
		data: cool.VectorPrimitivesData,
	): Map<number, number[]> {
		const targets = new Map<number, number[]>();
		const elements = this.getAnimatedElementMap(slideHash);
		if (!elements) return targets;

		for (const target of elements.keys()) {
			const { id, paragraph } = VectorCompositor._splitTarget(target);
			if (!data.objects.has(id)) continue;
			if (!targets.has(id)) targets.set(id, []);
			if (paragraph !== undefined) targets.get(id).push(paragraph);
		}
		targets.forEach((paragraphs) => paragraphs.sort((a, b) => a - b));
		return targets;
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
		// The videos draw into the 2D canvas of the composed frame.
		if (this._metaPresentation.getSlideInfo(slideHash)?.videos?.length)
			return false;
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
		return this.layers.draw(this._drawStates(slideHash, data), texture);
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
			const drawState = VectorCompositor._drawState(element);
			if (paragraph === undefined) {
				Object.assign(state, drawState);
			} else {
				if (!state.paragraphs) state.paragraphs = new Map();
				state.paragraphs.set(paragraph, drawState);
			}
		});
		return states;
	}

	// How the target is drawn now: left out while hidden, else under the
	// element's transform, opacity and colors, clipped to what the last
	// filter shows.
	private static _drawState(element: AnimatedElement): cool.DrawState {
		if (!element.getVisibility()) return { hidden: true };

		const state: cool.DrawState = {};
		const transform = element.getTransformMatrix();
		if (transform && !transform.isIdentity)
			state.transform = [
				transform.a,
				transform.b,
				transform.c,
				transform.d,
				transform.e,
				transform.f,
			];
		let opacity = element.getOpacity() ?? 1;

		// The filter's part is in the base box, before the transform.
		const filterFrame = element.getTransitionFilterFrame();
		if (filterFrame) {
			const mask = new ShapeTransitionMask(filterFrame.info);
			if (mask.hasArea()) {
				const clip = mask.clipAt(filterFrame.time, element.getBaseBBox());
				if (clip) state.clip = clip;
			} else {
				opacity *= mask.fadeAt(filterFrame.time);
			}
		}
		if (opacity < 1) state.alpha = opacity;
		const colorOverride = VectorCompositor._colorOverride(element);
		if (colorOverride) state.colorOverride = colorOverride;
		return state;
	}

	// The colors a color animation has moved, or undefined. A shape element
	// carries fill and line, a text element its font color in the fill
	// slot. Both are named once either moved, so only a color that differs
	// from its base stands in.
	private static _colorOverride(
		element: AnimatedElement,
	): cool.ColorOverride | undefined {
		const colorMap = element.getColorMap();
		const fillMoved =
			colorMap.toFillColor &&
			!(
				colorMap.fromFillColor &&
				colorMap.toFillColor.equal(colorMap.fromFillColor)
			);
		const lineMoved =
			colorMap.toLineColor &&
			!(
				colorMap.fromLineColor &&
				colorMap.toLineColor.equal(colorMap.fromLineColor)
			);
		if (!fillMoved && !lineMoved) return undefined;

		// Interpolated colors are unclamped, the canvas needs them clamped.
		if (element.isTextElement())
			return { text: colorMap.toFillColor.toString(true) };

		const override: cool.ColorOverride = {};
		if (fillMoved) override.fill = colorMap.toFillColor.toString(true);
		if (lineMoved) override.stroke = colorMap.toLineColor.toString(true);
		return override;
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
