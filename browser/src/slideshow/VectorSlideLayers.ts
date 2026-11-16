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
 * Draws the frames of a vector slide in the WebGL context of the slideshow canvas. The slide is
 * split in paint order into layers: runs of untargeted objects, runs of one animation target,
 * and a layer per paragraph that an animation moves on its own. Each layer is drawn once into a
 * texture over the rectangle it covers, and again only when its stand-in colors or text look
 * change. A frame draws the textures as quads in the state the animation gives them. The current
 * frame of a video is uploaded on every frame and drawn over the layer that holds its shape. The
 * page, which draws its master with it, is split after each video of the master, so the master
 * objects above a video are drawn over its frame.
 *
 * Canvas positions are in pixels of the slideshow canvas, with the slide filling it.
 */

/// A run of the paint order: the positions from start up to, not including, end. owner is the
/// object id of the animation target the objects belong to, itself or through a group above it.
/// Absent for a run that no animation targets.
interface PaintRun {
	start: number;
	end: number;
	owner?: number;
	// The positions in the paint order of the master that the page draws, from start up to, not
	// including, end. Absent where the run does not hold the page or the page draws all of it.
	master?: { start: number; end: number };
}

interface SlideLayer {
	run: PaintRun;
	// The paragraph the layer holds, as its position in the owner's text. Absent for a run.
	paragraph?: number;
	// The rectangle the texture covers, in canvas pixels.
	pixels: number[];
	// The rectangle the layer covers with its text at its own size.
	ownPixels: number[];
	texture: WebGLTexture | null;
	// The stand-in colors and text look the texture is drawn with, as JSON.
	look: string;
	// True for a layer of the owner drawn apart from its text: the owner without any text, or a
	// piece of the text.
	split?: boolean;
	// The bytes that the texture holds, 4 for each pixel.
	bytes: number;
	// The key of the piece of text the layer holds. Absent for any other layer.
	piece?: number;
	// The pieces of one paragraph, which are cut out of one drawing of it. Absent for a layer
	// that holds no piece.
	cut?: PieceCut;
}

/// A piece of a paragraph to draw as a layer of its own: the part of the paragraph of the owner
/// object that lies inside frame, which maps the unit square onto it in slide twips. key is the
/// id that the piece's state is given by, which is the id of no object.
interface SlidePiece {
	key: number;
	owner: number;
	paragraph: number;
	frame: cool.Matrix2D;
}

/// The layers of the pieces of one paragraph and the canvas rectangle that holds all of them.
/// painted is true once the paragraph is drawn and the pieces are cut out of it.
interface PieceCut {
	owner: number;
	paragraph: number;
	area: number[];
	layers: SlideLayer[];
	corners: cool.Point[][];
	painted: boolean;
}

// The margin around a layer's rectangle, in pixels, for the antialiased edge.
const layerMargin = 2;
// The share of a paragraph's height its layer reaches above and below it, for accents,
// descenders and italic overhang.
const paragraphOverhang = 0.3;
// The number of segments of a full circle in a clip piece.
const circleSegments = 64;

class VectorSlideLayers {
	public readonly context: RenderContextGl;
	private readonly quads: SlideQuadsGl;
	private readonly stencilProgram: WebGLProgram;
	private readonly stencilArray: WebGLVertexArrayObject | null;
	private readonly stencilBuffer: WebGLBuffer | null;

	private layers: SlideLayer[] = [];
	// The layers whose textures are not painted yet, in paint order.
	private unpainted: SlideLayer[] = [];
	// The pieces of text the layers hold, null where there are none, and the objects they are
	// the text of.
	private pieces: SlidePiece[] | null = null;
	private splitOwners = new Set<number>();
	// The bytes that the textures of these layers and of their videos hold, and the bytes of
	// the textures of every set of layers there is.
	private textureBytes = 0;
	private static allTextureBytes = 0;
	// The bytes of the texture of each video, by the object id of its shape.
	private videoBytes = new Map<number, number>();
	// The slide and tree the layers are drawn from, null while there are none.
	private slideHash: string | null = null;
	private data: cool.VectorPrimitivesData | null = null;
	// The tree of the master of the slide, null where there is none.
	private master: cool.VectorPrimitivesData | null = null;
	private canvasWidth = 0;
	private canvasHeight = 0;
	// Canvas pixels per twip.
	private scaleX = 1;
	private scaleY = 1;

	constructor(context: RenderContextGl) {
		this.context = context;
		const gl = context.getGl();

		this.quads = new SlideQuadsGl(context);
		this.stencilProgram = context.createProgram(
			context.createVertexShader(
				SlideshowShaders['VectorSlideLayersStencil.vert.glsl'],
			),
			context.createFragmentShader(
				SlideshowShaders['VectorSlideLayersStencil.frag.glsl'],
			),
		);

		this.stencilArray = gl.createVertexArray();
		gl.bindVertexArray(this.stencilArray);
		this.stencilBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.stencilBuffer);
		const pixelLocation = gl.getAttribLocation(this.stencilProgram, 'a_pixel');
		gl.enableVertexAttribArray(pixelLocation);
		gl.vertexAttribPointer(pixelLocation, 2, gl.FLOAT, false, 0, 0);
		gl.bindVertexArray(null);
	}

	/// Split the paint order into runs of consecutive objects with the same owner, or none.
	/// targets holds the object ids of the animation targets.
	public static paintRuns(
		data: cool.VectorPrimitivesData,
		targets: Set<number>,
	): PaintRun[] {
		const runs: PaintRun[] = [];
		for (let position = 0; position < data.order.length; ++position) {
			const owner = VectorSlideLayers.ownerOf(
				data,
				data.order[position],
				targets,
			);
			const last = runs[runs.length - 1];
			if (last && last.owner === owner) last.end = position + 1;
			else runs.push({ start: position, end: position + 1, owner });
		}
		return runs;
	}

	/// Split the run that starts with the page after each master object in videoIds, so the
	/// frame of the video is drawn over the master objects below it and under those above it.
	/// Each piece but the last holds only the page. The last piece also holds the rest of the run.
	public static splitAtMasterVideos(
		runs: PaintRun[],
		master: cool.VectorPrimitivesData | null,
		videoIds: Set<number>,
	): PaintRun[] {
		const first = runs[0];
		if (
			!master ||
			videoIds.size === 0 ||
			!first ||
			first.start !== 0 ||
			first.owner !== undefined
		)
			return runs;

		const pieces: PaintRun[] = [];
		let from = 0;
		for (let position = 0; position < master.order.length; ++position) {
			if (!videoIds.has(master.order[position])) continue;
			pieces.push({
				start: 0,
				end: 1,
				owner: undefined,
				master: { start: from, end: position + 1 },
			});
			from = position + 1;
		}
		if (pieces.length === 0) return runs;
		pieces.push({
			...first,
			master: { start: from, end: master.order.length },
		});
		return pieces.concat(runs.slice(1));
	}

	// The nearest animation target among the object and the groups above it, since an effect
	// on a group moves every member.
	private static ownerOf(
		data: cool.VectorPrimitivesData,
		objectId: number,
		targets: Set<number>,
	): number | undefined {
		for (
			let at: number | undefined = objectId, guard = 0;
			at !== undefined && guard < 100;
			at = data.objects.get(at)?.parent, ++guard
		) {
			if (targets.has(at)) return at;
		}
		return undefined;
	}

	/// True when the layers hold the slide drawn from the tree and the master tree at the canvas
	/// size.
	public holds(
		slideHash: string,
		data: cool.VectorPrimitivesData,
		master: cool.VectorPrimitivesData | null,
		canvasWidth: number,
		canvasHeight: number,
		pieces: SlidePiece[] | null = null,
	): boolean {
		return (
			this.slideHash === slideHash &&
			this.data === data &&
			this.master === master &&
			this.canvasWidth === canvasWidth &&
			this.canvasHeight === canvasHeight &&
			this.pieces === pieces
		);
	}

	/// Make the layers anew. targets maps each animation target's object id to the paragraphs
	/// that an animation moves on their own. master is the tree of the master of the slide and
	/// masterVideoIds the ids of its objects that hold a video. The textures are painted by
	/// paintNext, one layer at a time, and the next draw paints the ones still left.
	/// Each owner of pieces, which must be a target without paragraphs of its own, also gets a
	/// layer of itself without its text and a layer for each of its pieces, right after its
	/// own layer. drawLayers draws them in place of that layer for the owners it is given.
	public build(
		slideHash: string,
		data: cool.VectorPrimitivesData,
		targets: Map<number, number[]>,
		canvasWidth: number,
		canvasHeight: number,
		master: cool.VectorPrimitivesData | null = null,
		masterVideoIds: Set<number> = new Set(),
		pieces: SlidePiece[] | null = null,
	): void {
		this.clear();
		this.slideHash = slideHash;
		this.data = data;
		this.master = master;
		this.pieces = pieces;
		this.canvasWidth = canvasWidth;
		this.canvasHeight = canvasHeight;
		this.scaleX = canvasWidth / data.slideWidth;
		this.scaleY = canvasHeight / data.slideHeight;

		const runs = VectorSlideLayers.splitAtMasterVideos(
			VectorSlideLayers.paintRuns(data, new Set(targets.keys())),
			master,
			masterVideoIds,
		);
		const piecesByOwner = new Map<number, SlidePiece[]>();
		for (const piece of pieces ?? []) {
			const owned = piecesByOwner.get(piece.owner);
			if (owned) owned.push(piece);
			else piecesByOwner.set(piece.owner, [piece]);
		}
		for (const run of runs) {
			const pixels = this._runPixels(data, run);
			if (!pixels) continue;
			this.layers.push(this._makeLayer(run, undefined, pixels));
			if (run.owner === undefined) continue;
			for (const paragraph of targets.get(run.owner) ?? []) {
				this.layers.push(
					this._makeLayer(
						run,
						paragraph,
						this._paragraphPixels(data, run.owner, paragraph, pixels),
					),
				);
			}
			const owned = piecesByOwner.get(run.owner);
			if (owned) this._addSplitLayers(run.owner, run, pixels, owned);
		}
		// A run leaves out the paragraphs with layers of their own, so every layer is made
		// before any is painted.
		this.unpainted = this.layers.slice();
	}

	/// Paint the texture of the next layer that is not painted yet. True while layers are left
	/// to paint.
	public paintNext(): boolean {
		const layer = this.unpainted.shift();
		if (layer && this.data && !this.context.isDisposed()) {
			this._paint(layer, this.data, undefined);
			if (this.unpainted.length === 0) this._logMemory('build');
		}
		return this.unpainted.length > 0;
	}

	/// Draw a frame, each layer in the state states gives its target. The frame goes into
	/// texture when given, at the canvas size, else into the canvas. False when nothing was
	/// drawn.
	public draw(
		states: Map<number, cool.ObjectRenderState> | undefined,
		texture?: WebGLTexture | null,
		videos?: Map<number, ContentFrame>,
	): boolean {
		if (!this.begin(texture)) return false;
		this.drawLayers(states, videos);
		this.end();
		return true;
	}

	/// Start a frame at the canvas size, into texture when given, else into the canvas. The
	/// frame is cleared to black, or drawn over what the canvas holds when clear is false. False
	/// when nothing can be drawn.
	public begin(texture?: WebGLTexture | null, clear: boolean = true): boolean {
		if (this.context.isDisposed() || !this.data) return false;
		while (this.unpainted.length > 0) this.paintNext();
		return this.quads.begin(
			this.canvasWidth,
			this.canvasHeight,
			texture,
			clear,
		);
	}

	/// Draw layers into the frame begin started: all of them, or those whose run owners holds,
	/// where undefined stands for the runs that no target owns. A layer of a target is drawn in
	/// the state states gives it, a piece in the state of its key, and any other layer in
	/// restState. An owner of pieces in split is drawn without its text and with its pieces,
	/// any other owner with its text.
	public drawLayers(
		states: Map<number, cool.ObjectRenderState> | undefined,
		videos?: Map<number, ContentFrame>,
		owners?: ReadonlySet<number | undefined>,
		restState?: cool.DrawState,
		split?: ReadonlySet<number>,
	): void {
		const data = this.data;
		if (this.context.isDisposed() || !data) return;

		for (const layer of this.layers) {
			const owner = layer.run.owner;
			if (owners && !owners.has(owner)) continue;
			if (
				owner !== undefined &&
				!!layer.split !== !!split?.has(owner) &&
				(layer.split || this.splitOwners.has(owner))
			)
				continue;
			const state =
				owner === undefined ? restState : this._stateOf(layer, states);
			if (state?.hidden) continue;
			if (
				layer.paragraph !== undefined &&
				owner !== undefined &&
				states?.get(owner)?.hidden
			)
				continue;

			const look = VectorSlideLayers._lookOf(state);
			if (look !== layer.look && layer.piece === undefined) {
				layer.look = look;
				layer.pixels = this._grownPixels(
					layer.ownPixels,
					state?.textOverride?.sizeScale,
				);
				this._paint(layer, data, state);
			}
			const frames =
				videos?.size &&
				layer.paragraph === undefined &&
				layer.piece === undefined
					? this._frameQuads(layer, data, videos)
					: [];
			// The layer holds the still image of each video under its frame. The frame covers
			// it fully only while drawn opaque, so a faded layer is drawn around the frames.
			this._drawQuad(
				layer.texture,
				layer.pixels,
				state,
				frames.map((frame) => frame.rectangle),
			);
			for (const frame of frames)
				this._drawQuad(frame.texture, frame.rectangle, state);
		}
	}

	/// Finish the frame begin started.
	public end(): void {
		this.quads.end();
	}

	/// Forget the layers, so the next frame draws them anew.
	public invalidate(): void {
		this.clear();
	}

	/// Free the textures and the programs.
	public dispose(): void {
		this.clear();
		this._logMemory('dispose');
		if (this.context.isDisposed()) return;
		const gl = this.context.getGl();
		this.quads.dispose();
		gl.deleteProgram(this.stencilProgram);
		gl.deleteVertexArray(this.stencilArray);
		gl.deleteBuffer(this.stencilBuffer);
	}

	private clear(): void {
		if (!this.context.isDisposed()) {
			const gl = this.context.getGl();
			for (const layer of this.layers) gl.deleteTexture(layer.texture);
		}
		this.quads.clearFrames();
		this._count(-this.textureBytes);
		this.videoBytes.clear();
		this.layers = [];
		this.unpainted = [];
		this.pieces = null;
		this.splitOwners.clear();
		this.slideHash = null;
		this.data = null;
		this.master = null;
	}

	// A paragraph layer takes its paragraph's state. A run takes its owner's, which the
	// members of a group share.
	private _stateOf(
		layer: SlideLayer,
		states: Map<number, cool.ObjectRenderState> | undefined,
	): cool.DrawState | undefined {
		if (layer.run.owner === undefined || !states) return undefined;
		if (layer.piece !== undefined) return states.get(layer.piece);
		const ownerState = states.get(layer.run.owner);
		if (layer.paragraph === undefined) return ownerState;
		return ownerState?.paragraphs?.get(layer.paragraph);
	}

	// The canvas rectangle of a run: all of it for a run that starts with the page, else the
	// rectangles its objects paint, line width and shadow included. A run that never moves is
	// cut to the canvas. Null for a run that paints nothing.
	private _runPixels(
		data: cool.VectorPrimitivesData,
		run: PaintRun,
	): number[] | null {
		if (run.start === 0 && run.owner === undefined)
			return [0, 0, this.canvasWidth, this.canvasHeight];

		let left = Infinity;
		let top = Infinity;
		let right = -Infinity;
		let bottom = -Infinity;
		for (let position = run.start; position < run.end; ++position) {
			const object = data.objects.get(data.order[position]);
			if (
				!object ||
				object.x === undefined ||
				object.y === undefined ||
				object.width === undefined ||
				object.height === undefined
			)
				continue;
			left = Math.min(left, object.x);
			top = Math.min(top, object.y);
			right = Math.max(right, object.x + object.width);
			bottom = Math.max(bottom, object.y + object.height);
		}
		if (left > right || top > bottom) return null;

		const pixels = this._toPixels(left, top, right, bottom);
		// An animation can move a target in from outside the slide, so its layer keeps one
		// canvas size around it.
		return run.owner === undefined
			? this._cut(pixels, 0, 0, this.canvasWidth, this.canvasHeight)
			: this._cut(
					pixels,
					-this.canvasWidth,
					-this.canvasHeight,
					2 * this.canvasWidth,
					2 * this.canvasHeight,
				);
	}

	// The rectangle of a paragraph: its object's width, since a bullet or indent can reach out
	// of the paragraph's bounds, by its height plus a share above and below. The whole object
	// where the paragraph has no bounds.
	private _paragraphPixels(
		data: cool.VectorPrimitivesData,
		owner: number,
		paragraph: number,
		objectPixels: number[],
	): number[] {
		const object = data.objects.get(owner);
		const node = object
			? cool.TextHierarchyParagraphPrimitive.collect(object.primitives)[
					paragraph
				]
			: undefined;
		const bounds = node?.bounds;
		if (!bounds) return objectPixels;

		const overhang = (bounds[3] - bounds[1]) * paragraphOverhang;
		const top = (bounds[1] - overhang) * this.scaleY;
		const bottom = (bounds[3] + overhang) * this.scaleY;
		const objectBottom = objectPixels[1] + objectPixels[3];
		const cutTop = Math.max(objectPixels[1], Math.floor(top) - layerMargin);
		const cutBottom = Math.min(objectBottom, Math.ceil(bottom) + layerMargin);
		if (cutBottom <= cutTop) return objectPixels;
		return [objectPixels[0], cutTop, objectPixels[2], cutBottom - cutTop];
	}

	// Twips to whole canvas pixels, with the margin around them.
	private _toPixels(
		left: number,
		top: number,
		right: number,
		bottom: number,
	): number[] {
		const x = Math.floor(left * this.scaleX) - layerMargin;
		const y = Math.floor(top * this.scaleY) - layerMargin;
		return [
			x,
			y,
			Math.ceil(right * this.scaleX) + layerMargin - x,
			Math.ceil(bottom * this.scaleY) + layerMargin - y,
		];
	}

	// The stand-in colors and text look of a state, as JSON.
	private static _lookOf(state: cool.DrawState | undefined): string {
		return JSON.stringify([
			state?.colorOverride ?? null,
			state?.textOverride ?? null,
		]);
	}

	// The rectangle of a layer with its text at sizeScale times its size. The text grows about
	// a point inside, so the rectangle grows about its middle by twice the growth of the text,
	// within one canvas size around the canvas.
	private _grownPixels(
		pixels: number[],
		sizeScale: number | undefined,
	): number[] {
		if (sizeScale === undefined || sizeScale <= 1) return pixels;
		const [x, y, width, height] = pixels;
		const factor = 2 * sizeScale - 1;
		const grownWidth = Math.ceil(width * factor);
		const grownHeight = Math.ceil(height * factor);
		const grown = [
			Math.floor(x - (grownWidth - width) / 2),
			Math.floor(y - (grownHeight - height) / 2),
			grownWidth,
			grownHeight,
		];
		return (
			this._cut(
				grown,
				-this.canvasWidth,
				-this.canvasHeight,
				2 * this.canvasWidth,
				2 * this.canvasHeight,
			) ?? pixels
		);
	}

	private _cut(
		pixels: number[],
		left: number,
		top: number,
		right: number,
		bottom: number,
	): number[] | null {
		const x = Math.max(pixels[0], left);
		const y = Math.max(pixels[1], top);
		const width = Math.min(pixels[0] + pixels[2], right) - x;
		const height = Math.min(pixels[1] + pixels[3], bottom) - y;
		return width > 0 && height > 0 ? [x, y, width, height] : null;
	}

	private _makeLayer(
		run: PaintRun,
		paragraph: number | undefined,
		pixels: number[],
	): SlideLayer {
		const gl = this.context.getGl();
		const layer: SlideLayer = {
			run,
			paragraph,
			pixels,
			ownPixels: pixels,
			texture: gl.createTexture(),
			look: VectorSlideLayers._lookOf(undefined),
			bytes: 0,
		};
		gl.bindTexture(gl.TEXTURE_2D, layer.texture);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		return layer;
	}

	// Draw the layer with the primitive renderer and upload it into its texture. A run is drawn
	// with the colors and text look of the state given, without the paragraphs that have layers
	// of their own. The run that starts with the page and the bottom of its master is drawn over
	// white.
	private _paint(
		layer: SlideLayer,
		data: cool.VectorPrimitivesData,
		state: cool.DrawState | undefined,
	): void {
		if (layer.cut) {
			this._paintPieces(layer.cut, data);
			return;
		}
		const colorOverride = state?.colorOverride;
		const textOverride = state?.textOverride;
		const [x, y, width, height] = layer.pixels;
		const canvas = new OffscreenCanvas(width, height);
		const context = canvas.getContext(
			'2d',
		) as OffscreenCanvasRenderingContext2D | null;
		if (!context) return;
		if (
			layer.run.start === 0 &&
			layer.run.owner === undefined &&
			(layer.run.master?.start ?? 0) === 0
		) {
			context.fillStyle = '#FFFFFF';
			context.fillRect(0, 0, width, height);
		}
		context.setTransform(this.scaleX, 0, 0, this.scaleY, -x, -y);
		const target = context as unknown as CanvasRenderingContext2D;

		// A slideshow shows the document in its own colors.
		const owner = layer.run.owner;
		if (owner === undefined) {
			RenderManager.renderInto(target, data, {
				authoredColors: true,
				paintRange: { start: layer.run.start, end: layer.run.end },
				masterRange: layer.run.master,
			});
		} else if (layer.paragraph !== undefined) {
			RenderManager.renderObjectParagraph(
				target,
				data,
				owner,
				layer.paragraph,
				{
					colorOverride,
					textOverride,
					authoredColors: true,
				},
			);
		} else {
			const paragraphs = new Map<number, cool.DrawState>();
			for (const other of this.layers)
				if (other.run === layer.run && other.paragraph !== undefined)
					paragraphs.set(other.paragraph, { hidden: true });
			if (layer.split) {
				const count = cool.TextHierarchyParagraphPrimitive.collect(
					data.objects.get(owner)?.primitives,
				).length;
				for (let paragraph = 0; paragraph < count; ++paragraph)
					paragraphs.set(paragraph, { hidden: true });
			}
			RenderManager.renderInto(target, data, {
				authoredColors: true,
				paintRange: { start: layer.run.start, end: layer.run.end },
				states: new Map([[owner, { colorOverride, textOverride, paragraphs }]]),
			});
		}

		this._upload(layer, canvas);
	}

	// Upload what the canvas holds into the texture of the layer.
	private _upload(layer: SlideLayer, canvas: OffscreenCanvas): void {
		const bytes = canvas.width * canvas.height * 4;
		this._count(bytes - layer.bytes);
		layer.bytes = bytes;
		const gl = this.context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, layer.texture);
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
	}

	// Add the layer of the run without its text, and a layer for each of its pieces. The pieces
	// of one paragraph share a cut, which holds the rectangle around all of them.
	private _addSplitLayers(
		owner: number,
		run: PaintRun,
		pixels: number[],
		pieces: SlidePiece[],
	): void {
		this.splitOwners.add(owner);
		const body = this._makeLayer(run, undefined, pixels);
		body.split = true;
		this.layers.push(body);

		const cuts = new Map<number, PieceCut>();
		for (const piece of pieces) {
			let cut = cuts.get(piece.paragraph);
			if (!cut) {
				cut = {
					owner: piece.owner,
					paragraph: piece.paragraph,
					area: [],
					layers: [],
					corners: [],
					painted: false,
				};
				cuts.set(piece.paragraph, cut);
			}
			const corners = this._cornerPixels(piece.frame);
			const layer = this._makeLayer(
				run,
				undefined,
				VectorSlideLayers._boundsOf(corners),
			);
			layer.split = true;
			layer.piece = piece.key;
			layer.cut = cut;
			cut.layers.push(layer);
			cut.corners.push(corners);
			this.layers.push(layer);
		}
		cuts.forEach((cut) => {
			cut.area =
				this._cut(
					VectorSlideLayers._boundsOf(
						([] as cool.Point[]).concat(...cut.corners),
					),
					-this.canvasWidth,
					-this.canvasHeight,
					2 * this.canvasWidth,
					2 * this.canvasHeight,
				) ?? [];
		});
	}

	// Draw the paragraph of the cut once, and cut every piece out of that drawing along its
	// frame into the texture of its layer.
	private _paintPieces(cut: PieceCut, data: cool.VectorPrimitivesData): void {
		if (cut.painted) return;
		cut.painted = true;
		const area = cut.area;
		if (area.length !== 4) return;
		const drawing = new OffscreenCanvas(area[2], area[3]);
		const drawingContext = drawing.getContext(
			'2d',
		) as OffscreenCanvasRenderingContext2D | null;
		if (!drawingContext) return;
		drawingContext.setTransform(
			this.scaleX,
			0,
			0,
			this.scaleY,
			-area[0],
			-area[1],
		);
		RenderManager.renderObjectParagraph(
			drawingContext as unknown as CanvasRenderingContext2D,
			data,
			cut.owner,
			cut.paragraph,
			{ authoredColors: true },
		);

		cut.layers.forEach((layer, index) => {
			const pixels = this._cut(
				layer.pixels,
				area[0],
				area[1],
				area[0] + area[2],
				area[1] + area[3],
			);
			if (!pixels) return;
			layer.pixels = pixels;
			layer.ownPixels = pixels;
			const [x, y, width, height] = pixels;
			const canvas = new OffscreenCanvas(width, height);
			const context = canvas.getContext(
				'2d',
			) as OffscreenCanvasRenderingContext2D | null;
			if (!context) return;
			context.beginPath();
			cut.corners[index].forEach((point, at) => {
				if (at === 0) context.moveTo(point.x - x, point.y - y);
				else context.lineTo(point.x - x, point.y - y);
			});
			context.closePath();
			context.clip();
			context.drawImage(drawing, area[0] - x, area[1] - y);
			this._upload(layer, canvas);
		});
	}

	// The corners of the frame in canvas pixels, going round.
	private _cornerPixels(frame: cool.Matrix2D): cool.Point[] {
		const toPixels = frame.scale(this.scaleX, this.scaleY);
		return [
			toPixels.apply(0, 0),
			toPixels.apply(1, 0),
			toPixels.apply(1, 1),
			toPixels.apply(0, 1),
		];
	}

	// Whole canvas pixels around the points, with the margin around them.
	private static _boundsOf(points: cool.Point[]): number[] {
		const range = cool.Range2D.fromPoints(points);
		const x = Math.floor(range.minX) - layerMargin;
		const y = Math.floor(range.minY) - layerMargin;
		return [
			x,
			y,
			Math.ceil(range.maxX) + layerMargin - x,
			Math.ceil(range.maxY) + layerMargin - y,
		];
	}

	// The texture and the rectangle of the current frame of each video whose shape is in the
	// layer's run, or in the part of the master the run draws, with the frame uploaded.
	private _frameQuads(
		layer: SlideLayer,
		data: cool.VectorPrimitivesData,
		videos: Map<number, ContentFrame>,
	): { texture: WebGLTexture; rectangle: number[] }[] {
		const quads: { texture: WebGLTexture; rectangle: number[] }[] = [];
		const add = (objectId: number) => {
			const frame = videos.get(objectId);
			if (!frame) return;
			const texture = this.quads.frameTexture(objectId, frame.source);
			if (!texture) return;
			this._countVideo(objectId, frame.source);
			quads.push({ texture, rectangle: this.quads.frameRectangle(frame) });
		};
		const masterRange = layer.run.master;
		if (masterRange && this.master)
			for (
				let position = masterRange.start;
				position < masterRange.end;
				++position
			)
				add(this.master.order[position]);
		for (let position = layer.run.start; position < layer.run.end; ++position)
			add(data.order[position]);
		return quads;
	}

	// Count the bytes of the video texture of the object, which holds a picture of source.
	private _countVideo(objectId: number, source: TexImageSource): void {
		const size = source as {
			videoWidth?: number;
			videoHeight?: number;
			displayWidth?: number;
			displayHeight?: number;
			width?: number;
			height?: number;
		};
		const bytes =
			(size.videoWidth || size.displayWidth || size.width || 0) *
			(size.videoHeight || size.displayHeight || size.height || 0) *
			4;
		this._count(bytes - (this.videoBytes.get(objectId) ?? 0));
		this.videoBytes.set(objectId, bytes);
	}

	// Add the change to the bytes that the textures hold.
	private _count(change: number): void {
		this.textureBytes += change;
		VectorSlideLayers.allTextureBytes += change;
	}

	/// The bytes that the textures of these layers and of their videos hold.
	public get textureMemory(): number {
		return this.textureBytes;
	}

	// Log the memory of the textures, after what was done to the layers.
	private _logMemory(done: string): void {
		const megabytes = (bytes: number) => (bytes / (1024 * 1024)).toFixed(1);
		app.console.debug(
			`VectorSlideLayers.${done}: ${this.layers.length} layers, ` +
				`${megabytes(this.textureBytes)} MB of textures, ` +
				`${megabytes(VectorSlideLayers.allTextureBytes)} MB in all layers`,
		);
	}

	// Draw the texture over the rectangle, in canvas pixels, in the state given. Under an alpha
	// below 1 the draw leaves out the holes, rectangles in canvas pixels that the state moves
	// along with the texture.
	private _drawQuad(
		texture: WebGLTexture | null,
		rectangle: number[],
		state: cool.DrawState | undefined,
		holes: number[][] = [],
	): void {
		const gl = this.context.getGl();
		const transform = this._pixelTransform(state?.transform);
		const alpha = state?.alpha ?? 1;

		const clip = state?.clip;
		const drawnHoles = alpha < 1 ? holes : [];
		const isStenciled = !!clip || drawnHoles.length > 0;
		if (isStenciled) this._stencil(clip, drawnHoles, transform);

		this.quads.drawQuad(texture, rectangle, alpha, transform);

		if (isStenciled) {
			gl.disable(gl.STENCIL_TEST);
			gl.stencilMask(0xff);
		}
	}

	// The twips transform of a draw state as a canvas pixel matrix, in GLSL column order.
	private _pixelTransform(transform: number[] | undefined): Float32Array {
		if (!transform) return new Float32Array([1, 0, 0, 0, 1, 0, 0, 0, 1]);
		const [a, b, c, d, e, f] = transform;
		const scaleX = this.scaleX;
		const scaleY = this.scaleY;
		return new Float32Array([
			a,
			(b * scaleY) / scaleX,
			0,
			(c * scaleX) / scaleY,
			d,
			0,
			e * scaleX,
			f * scaleY,
			1,
		]);
	}

	// Mark the part that the clip leaves drawn and that the holes leave out in the stencil
	// buffer, and let the next draw reach only that part. Bit 0 holds the frame of the clip.
	// Bit 1 holds its pieces, each a fan that inverts the bit, which fills any polygon by the
	// even-odd rule since no two pieces overlap. Bit 2 holds the holes, rectangles in canvas
	// pixels.
	private _stencil(
		clip: cool.DrawClip | undefined,
		holes: number[][],
		stateTransform: Float32Array,
	): void {
		const gl = this.context.getGl();

		gl.enable(gl.STENCIL_TEST);
		gl.stencilMask(0xff);
		gl.clearStencil(0);
		gl.clear(gl.STENCIL_BUFFER_BIT);
		gl.colorMask(false, false, false, false);
		gl.useProgram(this.stencilProgram);
		gl.bindVertexArray(this.stencilArray);
		gl.uniform2f(
			gl.getUniformLocation(this.stencilProgram, 'u_canvas'),
			this.canvasWidth,
			this.canvasHeight,
		);
		gl.uniform1f(
			gl.getUniformLocation(this.stencilProgram, 'u_yDirection'),
			this.quads.yDirection,
		);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.stencilBuffer);

		// Outside a clip every pixel passes that no hole covers.
		let reference = 0x00;
		if (clip) {
			const toPixels = this._clipToPixels(clip.transform, stateTransform);
			const [x, y, width, height] = clip.frame;
			const frame = VectorSlideLayers._fan(
				[x, y, x + width, y, x + width, y + height, x, y + height],
				toPixels,
			);
			const pieces: number[] = [];
			for (const piece of clip.pieces)
				pieces.push(...VectorSlideLayers._pieceFan(piece, toPixels));

			gl.stencilFunc(gl.ALWAYS, 0x01, 0xff);
			gl.stencilMask(0x01);
			gl.stencilOp(gl.KEEP, gl.KEEP, gl.REPLACE);
			gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(frame), gl.STREAM_DRAW);
			gl.drawArrays(gl.TRIANGLES, 0, frame.length / 2);

			if (pieces.length) {
				gl.stencilMask(0x02);
				gl.stencilOp(gl.KEEP, gl.KEEP, gl.INVERT);
				gl.bufferData(
					gl.ARRAY_BUFFER,
					new Float32Array(pieces),
					gl.STREAM_DRAW,
				);
				gl.drawArrays(gl.TRIANGLES, 0, pieces.length / 2);
			}
			reference = clip.inverted ? 0x01 : 0x03;
		}

		if (holes.length) {
			const toPixels = (x: number, y: number): [number, number] => [
				stateTransform[0] * x + stateTransform[3] * y + stateTransform[6],
				stateTransform[1] * x + stateTransform[4] * y + stateTransform[7],
			];
			const triangles: number[] = [];
			for (const [x, y, width, height] of holes)
				triangles.push(
					...VectorSlideLayers._fan(
						[x, y, x + width, y, x + width, y + height, x, y + height],
						toPixels,
					),
				);
			gl.stencilFunc(gl.ALWAYS, 0x04, 0xff);
			gl.stencilMask(0x04);
			gl.stencilOp(gl.KEEP, gl.KEEP, gl.REPLACE);
			gl.bufferData(
				gl.ARRAY_BUFFER,
				new Float32Array(triangles),
				gl.STREAM_DRAW,
			);
			gl.drawArrays(gl.TRIANGLES, 0, triangles.length / 2);
		}

		gl.colorMask(true, true, true, true);
		gl.stencilMask(0x00);
		gl.stencilOp(gl.KEEP, gl.KEEP, gl.KEEP);
		gl.stencilFunc(gl.EQUAL, reference, clip ? 0x07 : 0x04);
	}

	// Clip coordinates to canvas pixels: the clip's transform to twips, twips to pixels, then
	// the state's transform in pixels.
	private _clipToPixels(
		clipTransform: number[],
		stateTransform: Float32Array,
	): (x: number, y: number) => [number, number] {
		const [a, b, c, d, e, f] = clipTransform;
		const scaleX = this.scaleX;
		const scaleY = this.scaleY;
		const matrix = stateTransform;
		return (x: number, y: number) => {
			const pixelX = (a * x + c * y + e) * scaleX;
			const pixelY = (b * x + d * y + f) * scaleY;
			return [
				matrix[0] * pixelX + matrix[3] * pixelY + matrix[6],
				matrix[1] * pixelX + matrix[4] * pixelY + matrix[7],
			];
		};
	}

	// A fan of triangles over the corners, given as x and y in turn, mapped to pixels.
	private static _fan(
		corners: number[],
		toPixels: (x: number, y: number) => [number, number],
	): number[] {
		const points: [number, number][] = [];
		for (let at = 0; at + 1 < corners.length; at += 2)
			points.push(toPixels(corners[at], corners[at + 1]));
		const triangles: number[] = [];
		for (let at = 1; at + 1 < points.length; ++at)
			triangles.push(...points[0], ...points[at], ...points[at + 1]);
		return triangles;
	}

	private static _pieceFan(
		piece: cool.AreaPiece,
		toPixels: (x: number, y: number) => [number, number],
	): number[] {
		if ('rectangle' in piece) {
			const [x, y, width, height] = piece.rectangle;
			return VectorSlideLayers._fan(
				[x, y, x + width, y, x + width, y + height, x, y + height],
				toPixels,
			);
		}
		if ('polygon' in piece)
			return VectorSlideLayers._fan(piece.polygon, toPixels);
		if ('ellipse' in piece) {
			const [centerX, centerY, radiusX, radiusY] = piece.ellipse;
			const corners: number[] = [];
			for (let at = 0; at < circleSegments; ++at) {
				const angle = (2 * Math.PI * at) / circleSegments;
				corners.push(
					centerX + radiusX * Math.cos(angle),
					centerY + radiusY * Math.sin(angle),
				);
			}
			return VectorSlideLayers._fan(corners, toPixels);
		}
		const [centerX, centerY, radius, start, end] = piece.slice;
		const steps = Math.max(
			1,
			Math.ceil((Math.abs(end - start) / (2 * Math.PI)) * circleSegments),
		);
		const corners: number[] = [centerX, centerY];
		for (let at = 0; at <= steps; ++at) {
			const angle = start + ((end - start) * at) / steps;
			corners.push(
				centerX + radius * Math.cos(angle),
				centerY + radius * Math.sin(angle),
			);
		}
		return VectorSlideLayers._fan(corners, toPixels);
	}
}
