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
 * AnimatedContentRenderer draws moving content (an animated image or a video)
 * over the slide, through either a 2D canvas context or a WebGL texture.
 */

// The position and size of one piece of content inside the document, in the
// same coordinate space the slide layers use.
interface AnimatedContentInfo {
	id: number;
	url: string;
	x: number;
	y: number;
	width: number;
	height: number;
	mirror?: number;
}

abstract class AnimatedContentRenderInfo {
	private texture: WebGLTexture | ImageBitmap | null = null;
	private vao: WebGLVertexArrayObject | null = null;
	public pos2d!: number[];
	public mirror: number = 0;

	public getTexture(): WebGLTexture {
		return this.texture as WebGLTexture;
	}

	public replaceTexture(
		context: RenderContext,
		newtexture: WebGLTexture | null,
	) {
		if (this.texture) context.deleteTexture(this.texture);
		this.texture = newtexture;
	}

	public getVao(): WebGLVertexArrayObject {
		return this.vao as WebGLVertexArrayObject;
	}

	public replaceVao(
		context: RenderContext,
		newVao: WebGLVertexArrayObject | null,
	) {
		if (this.vao) context.deleteVertexArray(this.vao);
		this.vao = newVao;
	}

	public abstract load(): void;

	public play(): void {
		// Nothing to start.
	}

	public pause(): void {
		// Nothing to pause.
	}

	public stop(): void {
		this.pause();
	}

	/// Play from the given second. Content without a timeline just plays.
	public playFrom(_seconds: number): void {
		this.play();
	}

	public handleClick(): void {
		// Content ignores clicks.
	}

	public deleteResources(context: RenderContext) {
		this.replaceTexture(context, null);
		this.replaceVao(context, null);
	}
}

/// How an animation draws content over the slide: the draw state of its shape, in slide units,
/// and the size of the slide in those units.
interface AnimatedContentLook {
	state: cool.DrawState;
	slideWidth: number;
	slideHeight: number;
}

/// The frame content shows now, and where: edges are its left, right, top and bottom as shares
/// of the slide's width and height, and mirror flips it across its width with bit 1 and across
/// its height with bit 2.
interface ContentFrame {
	source: TexImageSource;
	edges: number[];
	mirror: number;
}

abstract class AnimatedContentRenderer {
	protected sId: string;
	protected _context: RenderContext;
	protected _slideRenderer: SlideRenderer;
	protected info: AnimatedContentRenderInfo | null = null;
	// The id of the shape that shows the content.
	public infoId!: number;

	constructor(
		sId: string,
		context: RenderContext,
		slideRenderer: SlideRenderer,
	) {
		this.sId = sId;
		this._context = context;
		this._slideRenderer = slideRenderer;
	}

	public isDisposed() {
		return this._context && this._context.isDisposed();
	}

	protected getDocumentPositions(
		x: number,
		y: number,
		width: number,
		height: number,
		docWidth: number,
		docHeight: number,
	): number[] {
		const xMin = x / docWidth;
		const xMax = (x + width) / docWidth;
		const yMin = y / docHeight;
		const yMax = (y + height) / docHeight;
		return [xMin, xMax, yMin, yMax];
	}

	public abstract prepare(
		info: AnimatedContentInfo,
		docWidth: number,
		docHeight: number,
	): void;

	public load(): void {
		if (this.info) this.info.load();
	}

	public play(): void {
		if (this.info) this.info.play();
	}

	public pause(): void {
		if (this.info) this.info.pause();
	}

	public stop(): void {
		if (this.info) this.info.stop();
	}

	public playFrom(seconds: number): void {
		if (this.info) this.info.playFrom(seconds);
	}

	public handleClick(): void {
		if (this.info) this.info.handleClick();
	}

	public deleteResources(): void {
		if (this.info) this.info.deleteResources(this._context);
	}

	public abstract render(look?: AnimatedContentLook): void;

	/// The frame shown now, null while there is none.
	public currentFrame(): ContentFrame | null {
		return null;
	}
}

abstract class AnimatedContentRenderer2d extends AnimatedContentRenderer {
	protected abstract getDrawable(): CanvasImageSource | null;

	// Apply the shape's draw state to the canvas as the shape itself is drawn: opacity,
	// transform, and the part a clip shows. The state is in slide units, the content in pixels.
	private static applyLook(
		context: CanvasRenderingContext2D,
		look: AnimatedContentLook,
		width: number,
		height: number,
	): void {
		const state = look.state;
		if (state.alpha !== undefined) context.globalAlpha = state.alpha;
		const scaleX = width / look.slideWidth;
		const scaleY = height / look.slideHeight;
		context.scale(scaleX, scaleY);
		const matrix = cool.Matrix2D.fromArray(state.transform);
		if (matrix) matrix.applyTo(context);
		if (state.clip) VectorManager.clipTo(context, state.clip);
		context.scale(1 / scaleX, 1 / scaleY);
	}

	public render(look?: AnimatedContentLook): void {
		if (this.isDisposed()) return;
		if (!this.info) return;
		if (look?.state.hidden) return;

		// The content is drawn into the slide layer canvas, which is an OffscreenCanvas.
		const ctx = this._context.get2dGl() ?? this._context.get2dOffscreen();
		if (!ctx) {
			app.console.error('Canvas 2D context not available');
			return;
		}

		const drawable = this.getDrawable();
		if (!drawable) return;

		const width = ctx.canvas.width;
		const height = ctx.canvas.height;
		const pos2d = this.info.pos2d;

		const left = pos2d[0] * width;
		const top = pos2d[2] * height;
		const boxWidth = pos2d[1] * width - left;
		const boxHeight = pos2d[3] * height - top;
		const mirror = this.info.mirror;

		// Drawing about the centre keeps the flipped frame on the same area.
		ctx.save();
		if (look)
			AnimatedContentRenderer2d.applyLook(
				ctx as unknown as CanvasRenderingContext2D,
				look,
				width,
				height,
			);
		ctx.translate(left + boxWidth / 2, top + boxHeight / 2);
		ctx.scale(mirror & 1 ? -1 : 1, mirror & 2 ? -1 : 1);
		ctx.drawImage(drawable, -boxWidth / 2, -boxHeight / 2, boxWidth, boxHeight);
		ctx.restore();
	}
}

// The WebGL backend uploads whichever image getDrawable returns into a texture
// and draws it on a quad. Each renderer owns its own program, so when one
// renderer frees its resources it never destroys a program another renderer is
// still drawing with. The vertex and fragment shaders are the same for a video
// and an animated image, so every renderer builds its program from the same
// shader source.
abstract class AnimatedContentRendererGl extends AnimatedContentRenderer {
	private _program: WebGLProgram | null = null;

	protected abstract getDrawable(): TexImageSource | null;

	protected abstract placeholderPixel(): Uint8Array;

	private static getVertexShader(): string {
		return `#version 300 es
				in vec4 a_position;
				in vec2 a_texCoord;
				uniform mat3 u_transform;
				out vec2 v_texCoord;

				void main() {
					vec3 at = u_transform * vec3(a_position.xy, 1.0);
					gl_Position = vec4(at.xy, a_position.z, 1.0);
					v_texCoord = a_texCoord;
				}
				`;
	}

	private static getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D slideTexture;
				uniform float u_alpha;

				in vec2 v_texCoord;
				out vec4 outColor;

				void main() {
					outColor = texture(slideTexture, v_texCoord) * u_alpha;
				}
				`;
	}

	private ensureProgram(): void {
		if (this._program) return;
		if (this._context.is2dGl() || this._context.isDisposed()) return;

		const vertexShader = this._context.createVertexShader(
			AnimatedContentRendererGl.getVertexShader(),
		);
		const fragmentShader = this._context.createFragmentShader(
			AnimatedContentRendererGl.getFragmentShader(),
		);

		this._program = this._context.createProgram(vertexShader, fragmentShader);
	}

	private deleteProgram(): void {
		if (!this._program) return;
		if (!this._context.is2dGl() && !this._context.isDisposed())
			this._context.getGl().deleteProgram(this._program);
		this._program = null;
	}

	public deleteResources(): void {
		super.deleteResources();
		this.deleteProgram();
	}

	protected initTexture(): WebGLTexture | null {
		const gl = this._context.getGl();
		const texture = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, texture);

		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			1,
			1,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			this.placeholderPixel(),
		);

		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

		return texture;
	}

	// jscpd:ignore-start
	protected setupPositions(
		xMin: number,
		xMax: number,
		yMin: number,
		yMax: number,
		mirror: number = 0,
	): WebGLVertexArrayObject | null {
		if (this.isDisposed()) return null;
		if (this._context.is2dGl()) return null;

		const gl = this._context.getGl();

		// A flip swaps which edge of the image lands on which edge of the quad.
		const [texLeft, texRight] = mirror & 1 ? [1.0, 0.0] : [0.0, 1.0];
		const [texTop, texBottom] = mirror & 2 ? [0.0, 1.0] : [1.0, 0.0];

		// 5 numbers -> 3 x vertex X,Y,Z and 2x texture X,Y
		const positions = new Float32Array([
			//    vX    vY   vZ   tX       tY
			...[xMin, -yMin, 0.0, texLeft, texTop],
			...[xMax, -yMin, 0.0, texRight, texTop],
			...[xMin, -yMax, 0.0, texLeft, texBottom],
			...[xMax, -yMax, 0.0, texRight, texBottom],
		]);

		const buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
		gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);

		const vao = gl.createVertexArray();
		gl.bindVertexArray(vao);

		this.ensureProgram();
		const program = this._program;
		if (!program) {
			app.console.log('AnimatedContentRenderer: program is not valid');
			return null;
		}

		const positionLocation = gl.getAttribLocation(program, 'a_position');
		gl.enableVertexAttribArray(positionLocation);
		gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 5 * 4, 0);

		const texCoordLocation = gl.getAttribLocation(program, 'a_texCoord');
		gl.enableVertexAttribArray(texCoordLocation);
		gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 5 * 4, 3 * 4);

		return vao;
	}
	// jscpd:ignore-end

	protected setupRectangleInDocumentPositions(
		x: number,
		y: number,
		width: number,
		height: number,
		docWidth: number,
		docHeight: number,
		mirror: number = 0,
	): WebGLVertexArrayObject | null {
		const positions = this.getDocumentPositions(
			x,
			y,
			width,
			height,
			docWidth,
			docHeight,
		);
		return this.setupPositions(
			positions[0] * 2.0 - 1.0,
			positions[1] * 2.0 - 1.0,
			positions[2] * 2.0 - 1.0,
			positions[3] * 2.0 - 1.0,
			mirror,
		);
	}

	private updateTexture(texture: WebGLTexture, drawable: TexImageSource) {
		const gl = this._context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			drawable,
		);
	}

	// The shape's state gives the frame its opacity and transform. The context has no stencil
	// buffer, so a clip leaves the frame whole.
	public render(look?: AnimatedContentLook) {
		if (this.isDisposed()) return;
		if (!this.info) return;
		if (look?.state.hidden) return;

		if (!this._program) {
			app.console.log('AnimatedContentRenderer: program is not valid');
			return;
		}

		const drawable = this.getDrawable();
		if (!drawable) return;

		const gl = this._context.getGl();
		gl.useProgram(this._program);
		gl.activeTexture(gl.TEXTURE0);

		gl.bindVertexArray(this.info.getVao());
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
		this.updateTexture(this.info.getTexture(), drawable);

		const alpha = look?.state.alpha ?? 1;
		gl.uniform1f(gl.getUniformLocation(this._program, 'u_alpha'), alpha);
		gl.uniformMatrix3fv(
			gl.getUniformLocation(this._program, 'u_transform'),
			false,
			AnimatedContentRendererGl.clipSpaceTransform(look),
		);
		// The frame is multiplied by its opacity, so it blends premultiplied, like the layers of
		// the slide. The blend state stays set.
		gl.enable(gl.BLEND);
		gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
	}

	// The slide unit transform of the state as a clip space matrix in GLSL column order. Clip
	// space runs from -1 to 1 across the slide, with y upwards.
	private static clipSpaceTransform(
		look: AnimatedContentLook | undefined,
	): Float32Array {
		const transform = look?.state.transform;
		if (!look || !transform)
			return new Float32Array([1, 0, 0, 0, 1, 0, 0, 0, 1]);
		const toClipSpace = new DOMMatrix()
			.translate(-1, 1)
			.scale(2 / look.slideWidth, -2 / look.slideHeight);
		const matrix = toClipSpace
			.multiply(new DOMMatrix(transform))
			.multiply(toClipSpace.inverse());
		return new Float32Array([
			matrix.a,
			matrix.b,
			0,
			matrix.c,
			matrix.d,
			0,
			matrix.e,
			matrix.f,
			1,
		]);
	}
}
