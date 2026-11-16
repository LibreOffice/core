// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
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
 * over it once its transition has ended. The videos of its master page
 * play from the start of the transition onto it.
 */

/* global app RenderManager */

declare var SlideShow: any;

/// What a frame of a morph draws of one of its two slides: the layers, the state of each
/// animation target and piece of text, the video frames, the animation targets that each object
/// directly on the slide holds, and the objects drawn without their text.
interface MorphSide {
	layers: VectorSlideLayers;
	states: Map<number, cool.ObjectRenderState>;
	videos: Map<number, ContentFrame>;
	ownersByObject: Map<number, number[]>;
	split: ReadonlySet<number>;
}

/// The pieces that the text of a slide is taken apart into for a morph: words or characters.
type MorphPieceMode = 'words' | 'characters';

/// The pieces of the text of a slide, as the morph pairs them and as the layers draw them.
interface SlideTextPieces {
	entries: MorphSlidePiece[];
	layerPieces: SlidePiece[];
}

/// The two slides of a morph: the hash, the data and the layers of each, and the pieces of their
/// text when the morph takes the text apart.
interface MorphSlides {
	leavingHash: string;
	enteringHash: string;
	leavingData: cool.VectorPrimitivesData;
	enteringData: cool.VectorPrimitivesData;
	leavingLayers: VectorSlideLayers;
	enteringLayers: VectorSlideLayers;
	text: MorphTextPieces | undefined;
}

class VectorCompositor extends SlideCompositor {
	private offscreenCanvas: OffscreenCanvas = null;
	private offscreenContext: OffscreenCanvasRenderingContext2D = null;
	// The 2d render context over the offscreen canvas.
	private renderContext: RenderContext2d = null;
	private videos = new SlideVideos();
	private masterVideos = new MasterVideos();
	private canvasWidth = 0;
	private canvasHeight = 0;

	private disposed = false;
	// The listener registered on the render manager, kept to remove it.
	private onDataChanged: (() => void) | null = null;

	// The slide the initial fetchAndRun is waiting on, and the callback to
	// fire once its primitive tree is in the cache.
	private pendingSlideNumber: number = null;
	private pendingReady: VoidFunction = null;

	// The layers of the most recently drawn slides on the GPU, the most recent first, for each
	// WebGL context that frames are drawn straight into: the one of the slideshow canvas, and the
	// ones of the canvases that show the frames in other windows. A context holds at most
	// layerSetsPerContext of them.
	private layers = new Map<RenderContextGl, VectorSlideLayers[]>();
	// The slide a transition leaves and the one it enters. Each set holds full-canvas textures,
	// so the count stays small.
	private static readonly layerSetsPerContext = 2;
	// Stops the idle callback that paints the layers of the next slide ahead. Null while none
	// is waiting.
	private cancelPaintingAhead: (() => void) | null = null;
	// The slide that is shown once its transition has ended. Null while a transition runs.
	private shownSlideHash: string | null = null;
	// The pairs of objects of the morph that runs, null outside a morph.
	private morph: SlideMorph | null = null;
	// The pieces of the text of each tree, by the kind of piece.
	private textPieces = new WeakMap<
		cool.VectorPrimitivesData,
		Map<MorphPieceMode, SlideTextPieces>
	>();

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
		this.masterVideos.deleteResources();

		[this.canvasWidth, this.canvasHeight] = this._computeCanvasSize();
		this.offscreenCanvas = new OffscreenCanvas(
			this.canvasWidth,
			this.canvasHeight,
		);
		this.offscreenContext = this.offscreenCanvas.getContext('2d');
		this.renderContext = new RenderContext2d(this.offscreenCanvas);
		this._stopPaintingAhead();
		this.shownSlideHash = null;
		this._invalidateLayers();
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
		// The slide starts once the running code has returned, also when its primitives are
		// cached, so every listener of the presentation info is set up before the first slide
		// starts.
		queueMicrotask(() => {
			if (this.disposed) return;
			this._runWhenReady(slideNumber, () => {
				const done = this._onGotSlideCallback;
				this._onGotSlideCallback = null;
				if (done) done.call(this);
			});
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
		this._invalidateLayers();
		if (this.disposed) return;
		if (this.shownSlideHash) this._paintNextSlideAhead(this.shownSlideHash);
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
		// A slideshow shows the document in its own colors. Each video is drawn right after
		// its shape, in the state of the shape, so the shapes after it cover it.
		RenderManager.renderInto(
			context as unknown as CanvasRenderingContext2D,
			data,
			{
				authoredColors: true,
				states,
				drawOver: this._videosOverShapes(slideHash, data, states),
			},
		);
		context.restore();

		return this.offscreenCanvas.transferToImageBitmap();
	}

	// What draws the video of each shape that holds one, by shape id. The shapes of the master
	// page are included. A video is placed on the canvas in pixels, so the context goes back to
	// pixels while it is drawn.
	private _videosOverShapes(
		slideHash: string,
		data: cool.VectorPrimitivesData,
		states: Map<number, cool.ObjectRenderState> | undefined,
	): Map<number, (context: CanvasRenderingContext2D) => void> {
		const overShapes = new Map<
			number,
			(context: CanvasRenderingContext2D) => void
		>();
		for (const shapeId of this.videos.shapeIds(slideHash)) {
			overShapes.set(shapeId, (context) => {
				context.save();
				context.setTransform(1, 0, 0, 1, 0, 0);
				this.videos.renderVideo(
					slideHash,
					shapeId,
					this._videoLook(data, states, shapeId),
				);
				context.restore();
			});
		}
		for (const videoRenderer of this._masterVideoRenderers(slideHash)) {
			overShapes.set(videoRenderer.infoId, (context) => {
				context.save();
				context.setTransform(1, 0, 0, 1, 0, 0);
				videoRenderer.render();
				context.restore();
			});
		}
		return overShapes;
	}

	private _masterVideoRenderers(
		slideHash: string,
	): Array<AnimatedContentRenderer> {
		const masterHash =
			this._metaPresentation.getSlideInfo(slideHash)?.masterPage;
		return masterHash ? this.masterVideos.get(masterHash) : [];
	}

	// How the video of the shape is drawn: in the state of the shape.
	// Undefined where no animation moves the shape.
	private _videoLook(
		data: cool.VectorPrimitivesData,
		states: Map<number, cool.ObjectRenderState> | undefined,
		shapeId: number,
	): AnimatedContentLook | undefined {
		if (!states) return undefined;
		const state = ImpressVectorManager.stateOf(data, shapeId, states);
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

		// A slide that a morph leaves or enters draws each object directly on it on its own, so
		// the morph can move it.
		if (this._morphsOf(slideHash).length)
			for (const id of SlideMorph.topObjects(data))
				if (!targets.has(id)) targets.set(id, []);
		return targets;
	}

	// The morphs into the slide and out of it, the one into it first.
	private _morphsOf(slideHash: string): TransitionFilterInfo[] {
		const morphs: TransitionFilterInfo[] = [];
		const slideInfo = this._metaPresentation.getSlideInfo(slideHash);
		if (!slideInfo) return morphs;
		const into = TransitionFilterInfo.fromSlideInfo(slideInfo);
		if (into?.isMorph()) morphs.push(into);
		const nextInfo = slideInfo.next
			? this._metaPresentation.getSlideInfo(slideInfo.next)
			: null;
		const out = nextInfo ? TransitionFilterInfo.fromSlideInfo(nextInfo) : null;
		if (out?.isMorph()) morphs.push(out);
		return morphs;
	}

	// The pieces that a morph takes the text of the slide apart into. A morph by object moves the
	// words of the paired objects, so the text keeps its size while its object is resized. Null
	// outside a morph.
	private static _pieceModeOf(
		morph: TransitionFilterInfo | null | undefined,
	): MorphPieceMode | null {
		switch (morph?.transitionSubtype) {
			case TransitionSubType.MORPH:
			case TransitionSubType.MORPHBYWORD:
				return 'words';
			case TransitionSubType.MORPHBYCHARACTER:
				return 'characters';
			default:
				return null;
		}
	}

	// The pieces of the slide's layers outside a morph: those of the morph into the slide, so
	// the layers painted ahead for the next slide are the ones that the morph into it draws,
	// else those of the morph out of it.
	private _defaultPieceMode(slideHash: string): MorphPieceMode | null {
		for (const morph of this._morphsOf(slideHash)) {
			const mode = VectorCompositor._pieceModeOf(morph);
			if (mode) return mode;
		}
		return null;
	}

	// The kind number of a table, whose cells each hold a text of their own.
	private static readonly tableKind = 35;

	// The pieces of the text of each object directly on the slide. A table keeps its text whole,
	// and so does an object that an animation moves a paragraph of. targets are the animation
	// targets of the slide with the paragraphs that move on their own.
	private _slidePieces(
		data: cool.VectorPrimitivesData,
		mode: MorphPieceMode,
		targets: Map<number, number[]>,
	): SlideTextPieces {
		let byMode = this.textPieces.get(data);
		if (!byMode) {
			byMode = new Map();
			this.textPieces.set(data, byMode);
		}
		const known = byMode.get(mode);
		if (known) return known;

		const entries: MorphSlidePiece[] = [];
		for (const id of SlideMorph.topObjects(data)) {
			const object = data.objects.get(id);
			if (
				!object ||
				object.emptyPlaceholder ||
				object.objectKind === VectorCompositor.tableKind ||
				targets.get(id)?.length
			)
				continue;
			const text = MorphMatcher.textPiecesOf(object, mode === 'characters');
			for (const piece of text?.pieces ?? [])
				entries.push({ key: -(entries.length + 1), owner: id, piece });
		}
		const pieces: SlideTextPieces = {
			entries,
			layerPieces: entries.map((entry) => ({
				key: entry.key,
				owner: entry.owner,
				paragraph: entry.piece.paragraph,
				frame: entry.piece.clip,
			})),
		};
		byMode.set(mode, pieces);
		return pieces;
	}

	// The frame each video shows now, by the object id of its shape. The
	// shapes of the master page are included. A video without a frame yet
	// leaves its placeholder showing.
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
		for (const videoRenderer of this._masterVideoRenderers(slideHash)) {
			const frame = videoRenderer.currentFrame();
			if (frame) frames.set(videoRenderer.infoId, frame);
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

		this._dropDisposedLayers();
		const layers = this._slideLayers(context, slideHash, data);
		if (!layers) return false;
		return layers.draw(
			this._drawStates(slideHash, data),
			texture,
			this._videoFrames(slideHash, data),
		);
	}

	/// Build the layers of the two slides of a morph and paint them, so the first frame of the
	/// morph draws at once. False when either slide cannot be drawn object by object.
	public prepareMorph(
		context: RenderContextGl,
		leavingIndex: number,
		enteringIndex: number,
	): boolean {
		const morph = this._morphOf(context, leavingIndex, enteringIndex);
		if (!morph) return false;
		while (morph.leavingLayers.paintNext());
		while (morph.enteringLayers.paintNext());
		return true;
	}

	// The layers of the two slides of a morph, with the pieces of the morph, and the pairs of
	// objects of the morph in this.morph. Null when either slide cannot be drawn.
	private _morphOf(
		context: RenderContextGl,
		leavingIndex: number,
		enteringIndex: number,
	): MorphSlides | null {
		if (this.disposed || context.isDisposed()) return null;
		const leavingHash = this._metaPresentation.getSlideHash(leavingIndex);
		const enteringHash = this._metaPresentation.getSlideHash(enteringIndex);
		const leavingData = this._drawableData(leavingHash);
		const enteringData = this._drawableData(enteringHash);
		if (!leavingData || !enteringData) return null;

		// Both slides are drawn with the pieces of this morph.
		const filterInfo = TransitionFilterInfo.fromSlideInfo(
			this._metaPresentation.getSlideInfo(enteringHash),
		);
		const mode = VectorCompositor._pieceModeOf(filterInfo);
		this._dropDisposedLayers();
		const leavingLayers = this._slideLayers(
			context,
			leavingHash,
			leavingData,
			mode,
		);
		const enteringLayers = this._slideLayers(
			context,
			enteringHash,
			enteringData,
			mode,
		);
		if (!leavingLayers || !enteringLayers) return null;
		const text: MorphTextPieces | undefined = mode
			? {
					leaving: this._slidePieces(
						leavingData,
						mode,
						this._animatedTargets(leavingHash, leavingData),
					).entries,
					entering: this._slidePieces(
						enteringData,
						mode,
						this._animatedTargets(enteringHash, enteringData),
					).entries,
					splitsText: filterInfo?.transitionSubtype !== TransitionSubType.MORPH,
				}
			: undefined;
		if (!this.morph?.isBetween(leavingData, enteringData, text)) {
			this.morph = new SlideMorph(
				leavingData,
				enteringData,
				this._hiddenObjects(leavingHash, leavingData),
				this._hiddenObjects(enteringHash, enteringData),
				text,
			);
			const megabytes = (layers: VectorSlideLayers) =>
				(layers.textureMemory / (1024 * 1024)).toFixed(1);
			app.console.debug(
				`VectorCompositor: a morph of ${text?.leaving.length ?? 0} and ` +
					`${text?.entering.length ?? 0} pieces of text, layers of ` +
					`${megabytes(leavingLayers)} and ${megabytes(enteringLayers)} MB`,
			);
		}
		return {
			leavingHash,
			enteringHash,
			leavingData,
			enteringData,
			leavingLayers,
			enteringLayers,
			text,
		};
	}

	public drawMorphFrame(
		context: RenderContextGl,
		leavingIndex: number,
		enteringIndex: number,
		time: number,
	): boolean {
		const morph = this._morphOf(context, leavingIndex, enteringIndex);
		if (!morph || !this.morph) return false;
		const {
			leavingHash,
			enteringHash,
			leavingData,
			enteringData,
			leavingLayers,
			enteringLayers,
			text,
		} = morph;

		const [leavingMoves, enteringMoves] = this.morph.states(time);
		const leaving = this._morphSide(
			leavingLayers,
			leavingHash,
			leavingData,
			leavingMoves,
			text?.leaving,
			this.morph.leavingSplit,
		);
		const entering = this._morphSide(
			enteringLayers,
			enteringHash,
			enteringData,
			enteringMoves,
			text?.entering,
			this.morph.enteringSplit,
		);

		// Both backgrounds are drawn under the objects of both slides, so the background coming
		// in does not cover an object of the slide going out.
		const rest = new Set<number | undefined>([undefined]);
		if (!leaving.layers.begin()) return false;
		leaving.layers.drawLayers(undefined, leaving.videos, rest);
		leaving.layers.end();
		if (!entering.layers.begin(null, false)) return false;
		entering.layers.drawLayers(undefined, entering.videos, rest, {
			alpha: time,
		});
		entering.layers.end();

		// Each run of steps from one slide is drawn in one pass.
		const steps = this.morph.paintOrder;
		for (let index = 0; index < steps.length;) {
			const isLeaving = steps[index].leaving;
			const side = isLeaving ? leaving : entering;
			const owners = new Set<number>();
			for (
				;
				index < steps.length && steps[index].leaving === isLeaving;
				++index
			)
				for (const owner of side.ownersByObject.get(steps[index].id) ?? [])
					owners.add(owner);
			if (!side.layers.begin(null, false)) return false;
			side.layers.drawLayers(
				side.states,
				side.videos,
				owners,
				undefined,
				side.split,
			);
			side.layers.end();
		}
		return true;
	}

	// The objects that the animations of the slide hold hidden: the ones that an exit effect
	// took away on the slide a morph leaves, and the ones that wait for an entrance effect on
	// the slide it enters.
	private _hiddenObjects(
		slideHash: string,
		data: cool.VectorPrimitivesData,
	): Set<number> {
		const hidden = new Set<number>();
		this._drawStates(slideHash, data)?.forEach((state, id) => {
			if (state.hidden) hidden.add(id);
		});
		return hidden;
	}

	// What a frame of a morph draws of one slide. Each animation target is drawn in the state
	// its animation gives it, carried along by the move of the object directly on the slide
	// that holds it. A piece of text is drawn where its move takes it, in the place and the
	// opacity that the animation gives its object.
	private _morphSide(
		layers: VectorSlideLayers,
		slideHash: string,
		data: cool.VectorPrimitivesData,
		moves: Map<number, cool.ObjectRenderState>,
		pieces: MorphSlidePiece[] | undefined,
		split: ReadonlySet<number>,
	): MorphSide {
		const animations = this._drawStates(slideHash, data);
		const states = new Map<number, cool.ObjectRenderState>();
		const ownersByObject = new Map<number, number[]>();
		for (const owner of this._animatedTargets(slideHash, data).keys()) {
			const top = VectorCompositor._topObjectOf(data, owner);
			if (top === undefined) continue;
			const owners = ownersByObject.get(top);
			if (owners) owners.push(owner);
			else ownersByObject.set(top, [owner]);
			states.set(
				owner,
				VectorCompositor._overMove(moves.get(top), animations?.get(owner)),
			);
		}
		for (const entry of pieces ?? []) {
			const move = moves.get(entry.key);
			if (!move) continue;
			const animation = animations?.get(entry.owner);
			states.set(
				entry.key,
				VectorCompositor._overMove(
					move,
					animation && {
						hidden: animation.hidden,
						transform: animation.transform,
						alpha: animation.alpha,
						clip: animation.clip,
					},
				),
			);
		}
		return {
			layers,
			states,
			videos: this._videoFrames(slideHash, data),
			ownersByObject,
			split,
		};
	}

	// The object directly on the slide that holds the object, itself or through the groups
	// above it. Undefined for an object that no such object holds.
	private static _topObjectOf(
		data: cool.VectorPrimitivesData,
		objectId: number,
	): number | undefined {
		for (
			let at: number | undefined = objectId, guard = 0;
			at !== undefined && guard < 100;
			++guard
		) {
			const object = data.objects.get(at);
			if (!object) return undefined;
			if (object.parent === 0) return at;
			at = object.parent;
		}
		return undefined;
	}

	// The state of an animation target moved by a morph: its animation's transform first and
	// the move after it, with the opacities multiplied.
	private static _overMove(
		move: cool.ObjectRenderState | undefined,
		animation: cool.ObjectRenderState | undefined,
	): cool.ObjectRenderState {
		if (!move) return animation ?? {};
		if (!animation) return move;
		const state: cool.ObjectRenderState = VectorCompositor.overOwner(
			move,
			animation,
		);
		if (animation.paragraphs) {
			state.paragraphs = new Map();
			for (const [paragraph, own] of animation.paragraphs)
				state.paragraphs.set(paragraph, VectorCompositor.overOwner(move, own));
		}
		return state;
	}

	// The layers of a context that is gone are freed with it.
	private _dropDisposedLayers(): void {
		this.layers.forEach((sets, owner) => {
			if (!owner.isDisposed()) return;
			for (const layers of sets) layers.dispose();
			this.layers.delete(owner);
		});
	}

	private _invalidateLayers(): void {
		this.layers.forEach((sets) =>
			sets.forEach((layers) => layers.invalidate()),
		);
	}

	// The layers of the slide in the context, made anew where the context holds none for it.
	// A full context reuses the set of its least recently drawn slide. During a transition the
	// slide it leaves and the one it enters are both drawn from layers painted before. Null while
	// the canvas of the context has no size.
	private _slideLayers(
		context: RenderContextGl,
		slideHash: string,
		data: cool.VectorPrimitivesData,
		pieceMode: MorphPieceMode | null = this._defaultPieceMode(slideHash),
	): VectorSlideLayers | null {
		// The layers are drawn at the size of the canvas of the context, so the text on a smaller
		// canvas is drawn for its own size.
		const width = context.canvas.width;
		const height = context.canvas.height;
		if (width <= 0 || height <= 0) return null;
		const master =
			data.masterPartId !== undefined
				? (RenderManager.requestPartById(
						data.masterPartId,
						cool.VectorMode.MasterPages,
					) ?? null)
				: null;

		const targets = this._animatedTargets(slideHash, data);
		const pieces = pieceMode
			? this._slidePieces(data, pieceMode, targets).layerPieces
			: null;

		let sets = this.layers.get(context);
		if (!sets) {
			sets = [];
			this.layers.set(context, sets);
		}
		const index = sets.findIndex((layers) =>
			layers.holds(slideHash, data, master, width, height, pieces),
		);
		let layers: VectorSlideLayers;
		if (index >= 0) {
			layers = sets.splice(index, 1)[0];
		} else {
			layers =
				sets.length < VectorCompositor.layerSetsPerContext
					? new VectorSlideLayers(context)
					: sets.pop();
			const masterVideoIds = new Set<number>(
				(
					this._metaPresentation.getSlideInfo(slideHash)?.masterVideos ?? []
				).map((video) => video.id),
			);
			layers.build(
				slideHash,
				data,
				targets,
				width,
				height,
				master,
				masterVideoIds,
				pieces,
			);
		}
		sets.unshift(layers);
		return layers;
	}

	// Paint the layers of the slide after slideHash ahead of time, one layer per idle callback,
	// in every context that frames are drawn into. A full-canvas layer takes about as long as a
	// frame, so the frames between two callbacks go to the video.
	private _paintNextSlideAhead(slideHash: string): void {
		this._stopPaintingAhead();
		const nextHash = this._metaPresentation.getSlideInfo(slideHash)?.next;
		if (!nextHash) return;
		const step = (): void => {
			this.cancelPaintingAhead = null;
			if (this.disposed) return;
			const data = this._drawableData(nextHash);
			if (!data) return;
			this._dropDisposedLayers();
			let isLayerLeft = false;
			this.layers.forEach((_sets, context) => {
				if (this._slideLayers(context, nextHash, data)?.paintNext())
					isLayerLeft = true;
			});
			if (isLayerLeft) this._whenIdle(step);
		};
		this._whenIdle(step);
	}

	// Run callback once the browser is idle. The timeout runs it also while the frames of a
	// video keep the browser busy. A browser without idle callbacks runs it after a short delay.
	private _whenIdle(callback: () => void): void {
		if (typeof window.requestIdleCallback === 'function') {
			const id = window.requestIdleCallback(callback, { timeout: 1000 });
			this.cancelPaintingAhead = () => window.cancelIdleCallback(id);
		} else {
			const id = setTimeout(callback, 50);
			this.cancelPaintingAhead = () => clearTimeout(id);
		}
	}

	private _stopPaintingAhead(): void {
		this.cancelPaintingAhead?.();
		this.cancelPaintingAhead = null;
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
		// A paragraph is drawn in its own state on top of its shape's, so a shape that an effect
		// moved shows its paragraphs where it is.
		for (const state of states.values()) {
			if (!state.paragraphs) continue;
			for (const [paragraph, own] of state.paragraphs)
				state.paragraphs.set(paragraph, VectorCompositor.overOwner(state, own));
		}
		return states;
	}

	/// A paragraph's state on top of its shape's. Transforms chain, opacities multiply, and
	/// an unset clip, colors or text look falls back to the shape's. Hidden when either is.
	public static overOwner(
		owner: cool.DrawState,
		own: cool.DrawState,
	): cool.DrawState {
		if (owner.hidden || own.hidden) return { hidden: true };
		const state: cool.DrawState = {};
		const ownMatrix = cool.Matrix2D.fromArray(own.transform);
		const ownerMatrix = cool.Matrix2D.fromArray(owner.transform);
		const matrix =
			ownMatrix && ownerMatrix
				? ownMatrix.then(ownerMatrix)
				: (ownMatrix ?? ownerMatrix);
		if (matrix) state.transform = matrix.toArray();
		if (own.alpha !== undefined || owner.alpha !== undefined)
			state.alpha = (own.alpha ?? 1) * (owner.alpha ?? 1);
		const clip = own.clip ?? owner.clip;
		if (clip) state.clip = clip;
		const colorOverride = own.colorOverride ?? owner.colorOverride;
		if (colorOverride) state.colorOverride = colorOverride;
		const textOverride = own.textOverride ?? owner.textOverride;
		if (textOverride) state.textOverride = textOverride;
		return state;
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

	public hasMasterVideos(slideIndex: number): boolean {
		const slideHash = this._metaPresentation.getSlideHash(slideIndex);
		return this._masterVideoRenderers(slideHash).length > 0;
	}

	public getMasterVideoFrames(slideIndex: number): number {
		const slideHash = this._metaPresentation.getSlideHash(slideIndex);
		const masterHash =
			this._metaPresentation.getSlideInfo(slideHash)?.masterPage;
		return masterHash ? this.masterVideos.presentedFrames(masterHash) : 0;
	}

	// The primitive trees are already cached before a transition starts. The background video of
	// the slide coming in plays from the start of the transition.
	public notifyTransitionStart(slideHash: string): void {
		// The context holds the layers of the slide the transition leaves and of the one it
		// enters, so the painting ahead waits until the transition has ended.
		this._stopPaintingAhead();
		this.shownSlideHash = null;
		if (this.disposed || !this.renderContext) return;
		this.masterVideos.start(
			this._metaPresentation.getSlideInfo(slideHash),
			this.renderContext,
		);
	}

	public notifyTransitionEnd(slideHash: string): void {
		if (this.disposed || !this.renderContext) return;
		const slideInfo = this._metaPresentation.getSlideInfo(slideHash);
		this.masterVideos.start(slideInfo, this.renderContext);
		if (slideInfo) this.masterVideos.keepOnly(slideInfo.masterPage);
		else this.masterVideos.deleteResources();
		this.videos.enterSlide(slideHash, slideInfo, this.renderContext);
		this.shownSlideHash = slideHash;
		this.morph = null;
		this._paintNextSlideAhead(slideHash);
	}

	public deleteResources(): void {
		this.disposed = true;
		this.videos.deleteResources();
		this.videos.clear();
		this.masterVideos.deleteResources();
		this.offscreenCanvas = null;
		this.offscreenContext = null;
		this.renderContext = null;
		this._stopPaintingAhead();
		this.shownSlideHash = null;
		this.layers.forEach((sets) => sets.forEach((layers) => layers.dispose()));
		this.layers.clear();
		this.morph = null;
		this.pendingReady = null;
		this.pendingSlideNumber = null;
	}

	// --- canvas sizing ----------------------------------------------

	// Primitives render at any scale, so the resolution is the requested size in device pixels.
	public computeLayerResolution(
		width: number,
		height: number,
	): [number, number] {
		const devicePixelRatio = window.devicePixelRatio || 1;
		return [width * devicePixelRatio, height * devicePixelRatio];
	}

	// The canvas takes the display resolution, fitted to the slide aspect ratio.
	private _computeCanvasSize(): [number, number] {
		const [width, height] = this.computeLayerResolution(
			window.screen.width,
			window.screen.height,
		);
		return this.computeLayerSize(width, height);
	}
}

SlideShow.VectorCompositor = VectorCompositor;
