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
 * Draws the frames of a vector slide in the WebGL context of the slideshow canvas. The slide is
 * split in paint order into layers: runs of untargeted objects, runs of one animation target,
 * and a layer per paragraph that an animation moves on its own. Each layer is drawn once into a
 * texture over the rectangle it covers, and again only when its stand-in colors or text look
 * change. A frame draws the textures as quads in the state the animation gives them. The current
 * frame of a video is uploaded on every frame and drawn over the layer that holds its shape.
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
}

/// A framebuffer with the texture as its color and a stencil buffer of the same size.
interface TextureTarget {
	framebuffer: WebGLFramebuffer | null;
	stencil: WebGLRenderbuffer | null;
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
	private readonly quadProgram: WebGLProgram;
	private readonly stencilProgram: WebGLProgram;
	private readonly quadArray: WebGLVertexArrayObject | null;
	private readonly stencilArray: WebGLVertexArrayObject | null;
	private readonly stencilBuffer: WebGLBuffer | null;

	private layers: SlideLayer[] = [];
	// The slide and tree the layers are drawn from, null while there are none.
	private slideHash: string | null = null;
	private data: cool.VectorPrimitivesData | null = null;
	private canvasWidth = 0;
	private canvasHeight = 0;
	// Canvas pixels per twip.
	private scaleX = 1;
	private scaleY = 1;
	// 1 while a frame goes into the canvas, -1 while it goes into a texture.
	private yDirection = 1;
	// Whether this context can draw into a texture with a stencil buffer, null until the first
	// texture target is made.
	private targetsComplete: boolean | null = null;
	// The texture of the current frame of each video, by the object id of its shape.
	private videoTextures = new Map<number, WebGLTexture | null>();

	constructor(context: RenderContextGl) {
		this.context = context;
		const gl = context.getGl();

		this.quadProgram = context.createProgram(
			context.createVertexShader(
				SlideshowShaders['VectorSlideLayersQuad.vert.glsl'],
			),
			context.createFragmentShader(
				SlideshowShaders['VectorSlideLayersQuad.frag.glsl'],
			),
		);
		this.stencilProgram = context.createProgram(
			context.createVertexShader(
				SlideshowShaders['VectorSlideLayersStencil.vert.glsl'],
			),
			context.createFragmentShader(
				SlideshowShaders['VectorSlideLayersStencil.frag.glsl'],
			),
		);

		this.quadArray = gl.createVertexArray();
		gl.bindVertexArray(this.quadArray);
		const quadBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, quadBuffer);
		gl.bufferData(
			gl.ARRAY_BUFFER,
			new Float32Array([0, 0, 1, 0, 0, 1, 1, 1]),
			gl.STATIC_DRAW,
		);
		const unitLocation = gl.getAttribLocation(this.quadProgram, 'a_unit');
		gl.enableVertexAttribArray(unitLocation);
		gl.vertexAttribPointer(unitLocation, 2, gl.FLOAT, false, 0, 0);

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

	/// True when the layers hold the slide drawn from the tree at the canvas size.
	public holds(
		slideHash: string,
		data: cool.VectorPrimitivesData,
		canvasWidth: number,
		canvasHeight: number,
	): boolean {
		return (
			this.slideHash === slideHash &&
			this.data === data &&
			this.canvasWidth === canvasWidth &&
			this.canvasHeight === canvasHeight
		);
	}

	/// Draw the layers anew. targets maps each animation target's object id to the paragraphs
	/// that an animation moves on their own.
	public build(
		slideHash: string,
		data: cool.VectorPrimitivesData,
		targets: Map<number, number[]>,
		canvasWidth: number,
		canvasHeight: number,
	): void {
		this.clear();
		this.slideHash = slideHash;
		this.data = data;
		this.canvasWidth = canvasWidth;
		this.canvasHeight = canvasHeight;
		this.scaleX = canvasWidth / data.slideWidth;
		this.scaleY = canvasHeight / data.slideHeight;

		const runs = VectorSlideLayers.paintRuns(data, new Set(targets.keys()));
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
		}
		// A run leaves out the paragraphs with layers of their own, so every layer is made
		// before any is painted.
		for (const layer of this.layers) this._paint(layer, data, undefined);
	}

	/// Draw a frame, each layer in the state states gives its target. The frame goes into
	/// texture when given, at the canvas size, else into the canvas. False when nothing was
	/// drawn.
	public draw(
		states: Map<number, cool.ObjectRenderState> | undefined,
		texture?: WebGLTexture | null,
		videos?: Map<number, ContentFrame>,
	): boolean {
		const data = this.data;
		if (this.context.isDisposed() || !data) return false;
		const gl = this.context.getGl();

		const target = texture ? this._targetTexture(texture) : null;
		if (texture && !target) return false;
		// A texture holds the top row first, like an uploaded image. The canvas shows its first
		// row at the bottom.
		this.yDirection = target ? -1 : 1;
		if (target) gl.viewport(0, 0, this.canvasWidth, this.canvasHeight);
		else
			gl.viewport(0, 0, this.context.canvas.width, this.context.canvas.height);
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
		gl.clear(gl.COLOR_BUFFER_BIT);
		// The layers are flat quads in paint order, and a flipped one faces away, so every quad
		// is drawn whole. A 3D transition turns depth testing and face culling on in the same
		// context.
		gl.disable(gl.DEPTH_TEST);
		gl.disable(gl.CULL_FACE);
		// The textures hold premultiplied colors.
		gl.enable(gl.BLEND);
		gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

		for (const layer of this.layers) {
			const state = this._stateOf(layer, states);
			if (state?.hidden) continue;
			const owner = layer.run.owner;
			if (
				layer.paragraph !== undefined &&
				owner !== undefined &&
				states?.get(owner)?.hidden
			)
				continue;

			const look = VectorSlideLayers._lookOf(state);
			if (look !== layer.look) {
				layer.look = look;
				layer.pixels = this._grownPixels(
					layer.ownPixels,
					state?.textOverride?.sizeScale,
				);
				this._paint(layer, data, state);
			}
			this._drawQuad(layer.texture, layer.pixels, state);
			if (videos?.size && layer.paragraph === undefined)
				this._drawVideos(layer, data, videos, state);
		}

		gl.disable(gl.BLEND);
		gl.bindVertexArray(null);
		if (texture && target) this._releaseTarget(texture, target);
		return true;
	}

	// Size the texture to the canvas and make it the draw target, with a stencil buffer for the
	// clips. Null when the texture cannot be drawn into.
	private _targetTexture(texture: WebGLTexture): TextureTarget | null {
		if (this.targetsComplete === false) return null;
		const gl = this.context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			this.canvasWidth,
			this.canvasHeight,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			null,
		);
		gl.bindTexture(gl.TEXTURE_2D, null);

		const stencil = gl.createRenderbuffer();
		gl.bindRenderbuffer(gl.RENDERBUFFER, stencil);
		gl.renderbufferStorage(
			gl.RENDERBUFFER,
			gl.STENCIL_INDEX8,
			this.canvasWidth,
			this.canvasHeight,
		);
		gl.bindRenderbuffer(gl.RENDERBUFFER, null);

		const framebuffer = gl.createFramebuffer();
		gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
		gl.framebufferTexture2D(
			gl.FRAMEBUFFER,
			gl.COLOR_ATTACHMENT0,
			gl.TEXTURE_2D,
			texture,
			0,
		);
		gl.framebufferRenderbuffer(
			gl.FRAMEBUFFER,
			gl.STENCIL_ATTACHMENT,
			gl.RENDERBUFFER,
			stencil,
		);
		const target = { framebuffer, stencil };
		// The status check waits for the GPU process. Every target has the same formats, so only
		// the first is checked.
		if (this.targetsComplete === null)
			this.targetsComplete =
				gl.checkFramebufferStatus(gl.FRAMEBUFFER) === gl.FRAMEBUFFER_COMPLETE;
		if (!this.targetsComplete) {
			this._releaseTarget(null, target);
			return null;
		}
		return target;
	}

	// Draw into the canvas again. A texture that samples between mipmap levels gets them made
	// anew from the frame.
	private _releaseTarget(
		texture: WebGLTexture | null,
		target: TextureTarget,
	): void {
		const gl = this.context.getGl();
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.deleteFramebuffer(target.framebuffer);
		gl.deleteRenderbuffer(target.stencil);
		gl.viewport(0, 0, this.context.canvas.width, this.context.canvas.height);
		this.yDirection = 1;
		if (!texture) return;

		if (!this.context.isMipMapped(texture)) return;
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.generateMipmap(gl.TEXTURE_2D);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}

	/// Forget the layers, so the next frame draws them anew.
	public invalidate(): void {
		this.clear();
	}

	/// Free the textures and the programs.
	public dispose(): void {
		this.clear();
		if (this.context.isDisposed()) return;
		const gl = this.context.getGl();
		gl.deleteProgram(this.quadProgram);
		gl.deleteProgram(this.stencilProgram);
		gl.deleteVertexArray(this.quadArray);
		gl.deleteVertexArray(this.stencilArray);
		gl.deleteBuffer(this.stencilBuffer);
	}

	private clear(): void {
		if (!this.context.isDisposed()) {
			const gl = this.context.getGl();
			for (const layer of this.layers) gl.deleteTexture(layer.texture);
			this.videoTextures.forEach((texture) => gl.deleteTexture(texture));
		}
		this.videoTextures.clear();
		this.layers = [];
		this.slideHash = null;
		this.data = null;
	}

	// A paragraph layer takes its paragraph's state. A run takes its owner's, which the
	// members of a group share.
	private _stateOf(
		layer: SlideLayer,
		states: Map<number, cool.ObjectRenderState> | undefined,
	): cool.DrawState | undefined {
		if (layer.run.owner === undefined || !states) return undefined;
		const ownerState = states.get(layer.run.owner);
		if (layer.paragraph === undefined) return ownerState;
		return ownerState?.paragraphs?.get(layer.paragraph);
	}

	// The canvas rectangle of a run: all of it for the run that starts with the page, else the
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
	// of their own. The run that starts with the page is drawn over white.
	private _paint(
		layer: SlideLayer,
		data: cool.VectorPrimitivesData,
		state: cool.DrawState | undefined,
	): void {
		const colorOverride = state?.colorOverride;
		const textOverride = state?.textOverride;
		const [x, y, width, height] = layer.pixels;
		const canvas = new OffscreenCanvas(width, height);
		const context = canvas.getContext(
			'2d',
		) as OffscreenCanvasRenderingContext2D | null;
		if (!context) return;
		if (layer.run.start === 0 && layer.run.owner === undefined) {
			context.fillStyle = '#FFFFFF';
			context.fillRect(0, 0, width, height);
		}
		context.setTransform(this.scaleX, 0, 0, this.scaleY, -x, -y);
		const target = context as unknown as CanvasRenderingContext2D;

		const owner = layer.run.owner;
		if (owner === undefined) {
			RenderManager.renderInto(target, data, {
				paintRange: { start: layer.run.start, end: layer.run.end },
			});
		} else if (layer.paragraph !== undefined) {
			RenderManager.renderObjectParagraph(
				target,
				data,
				owner,
				layer.paragraph,
				colorOverride,
				textOverride,
			);
		} else {
			const paragraphs = new Map<number, cool.DrawState>();
			for (const other of this.layers)
				if (other.run === layer.run && other.paragraph !== undefined)
					paragraphs.set(other.paragraph, { hidden: true });
			RenderManager.renderInto(target, data, {
				paintRange: { start: layer.run.start, end: layer.run.end },
				states: new Map([[owner, { colorOverride, textOverride, paragraphs }]]),
			});
		}

		const gl = this.context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, layer.texture);
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
	}

	// Draw the current frame of each video whose shape is in the layer's run, over the layer in
	// its state.
	private _drawVideos(
		layer: SlideLayer,
		data: cool.VectorPrimitivesData,
		videos: Map<number, ContentFrame>,
		state: cool.DrawState | undefined,
	): void {
		for (let position = layer.run.start; position < layer.run.end; ++position) {
			const objectId = data.order[position];
			const frame = videos.get(objectId);
			if (!frame) continue;

			const [left, right, top, bottom] = frame.edges;
			let x = left * this.canvasWidth;
			let y = top * this.canvasHeight;
			let width = (right - left) * this.canvasWidth;
			let height = (bottom - top) * this.canvasHeight;
			// A flipped frame covers the same rectangle from its other edge.
			if (frame.mirror & 1) {
				x += width;
				width = -width;
			}
			if (frame.mirror & 2) {
				y += height;
				height = -height;
			}
			this._drawQuad(
				this._videoTexture(objectId, frame.source),
				[x, y, width, height],
				state,
			);
		}
	}

	// The video texture of the object, holding the frame given.
	private _videoTexture(
		objectId: number,
		source: TexImageSource,
	): WebGLTexture | null {
		const gl = this.context.getGl();
		let texture = this.videoTextures.get(objectId);
		if (!texture) {
			texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, texture);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			this.videoTextures.set(objectId, texture);
		}
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, source);
		return texture;
	}

	// Draw the texture over the rectangle, in canvas pixels, in the state given.
	private _drawQuad(
		texture: WebGLTexture | null,
		rectangle: number[],
		state: cool.DrawState | undefined,
	): void {
		const gl = this.context.getGl();
		const transform = this._pixelTransform(state?.transform);

		const clip = state?.clip;
		if (clip) this._stencilClip(clip, transform);

		gl.useProgram(this.quadProgram);
		gl.bindVertexArray(this.quadArray);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.uniform1i(gl.getUniformLocation(this.quadProgram, 'u_texture'), 0);
		gl.uniform1f(
			gl.getUniformLocation(this.quadProgram, 'u_alpha'),
			state?.alpha ?? 1,
		);
		gl.uniform4f(
			gl.getUniformLocation(this.quadProgram, 'u_rect'),
			rectangle[0],
			rectangle[1],
			rectangle[2],
			rectangle[3],
		);
		gl.uniformMatrix3fv(
			gl.getUniformLocation(this.quadProgram, 'u_transform'),
			false,
			transform,
		);
		gl.uniform2f(
			gl.getUniformLocation(this.quadProgram, 'u_canvas'),
			this.canvasWidth,
			this.canvasHeight,
		);
		gl.uniform1f(
			gl.getUniformLocation(this.quadProgram, 'u_yDirection'),
			this.yDirection,
		);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

		if (clip) {
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

	// Mark the part the clip leaves drawn in the stencil buffer, and let the next draw reach
	// only that part. Bit 0 holds the frame. Bit 1 holds the pieces, each a fan that inverts
	// the bit, which fills any polygon by the even-odd rule since no two pieces overlap.
	private _stencilClip(
		clip: cool.DrawClip,
		stateTransform: Float32Array,
	): void {
		const gl = this.context.getGl();
		const toPixels = this._clipToPixels(clip.transform, stateTransform);

		const [x, y, width, height] = clip.frame;
		const frame = VectorSlideLayers._fan(
			[x, y, x + width, y, x + width, y + height, x, y + height],
			toPixels,
		);
		const pieces: number[] = [];
		for (const piece of clip.pieces)
			pieces.push(...VectorSlideLayers._pieceFan(piece, toPixels));

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
			this.yDirection,
		);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.stencilBuffer);

		gl.stencilFunc(gl.ALWAYS, 0x01, 0xff);
		gl.stencilMask(0x01);
		gl.stencilOp(gl.KEEP, gl.KEEP, gl.REPLACE);
		gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(frame), gl.STREAM_DRAW);
		gl.drawArrays(gl.TRIANGLES, 0, frame.length / 2);

		if (pieces.length) {
			gl.stencilMask(0x02);
			gl.stencilOp(gl.KEEP, gl.KEEP, gl.INVERT);
			gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(pieces), gl.STREAM_DRAW);
			gl.drawArrays(gl.TRIANGLES, 0, pieces.length / 2);
		}

		gl.colorMask(true, true, true, true);
		gl.stencilMask(0x00);
		gl.stencilOp(gl.KEEP, gl.KEEP, gl.KEEP);
		gl.stencilFunc(gl.EQUAL, clip.inverted ? 0x01 : 0x03, 0x03);
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
