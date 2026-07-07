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
}

abstract class AnimatedContentRenderInfo {
	private texture: WebGLTexture | ImageBitmap | null = null;
	private vao: WebGLVertexArrayObject | null = null;
	public pos2d!: number[];

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

	public handleClick(): void {
		// Content ignores clicks.
	}

	public deleteResources(context: RenderContext) {
		this.replaceTexture(context, null);
		this.replaceVao(context, null);
	}
}

abstract class AnimatedContentRenderer {
	protected sId: string;
	protected _context: RenderContext;
	protected _slideRenderer: SlideRenderer;
	protected info: AnimatedContentRenderInfo | null = null;
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

	public handleClick(): void {
		if (this.info) this.info.handleClick();
	}

	public deleteResources(): void {
		if (this.info) this.info.deleteResources(this._context);
	}

	public abstract render(): void;
}

abstract class AnimatedContentRenderer2d extends AnimatedContentRenderer {
	protected abstract getDrawable(): CanvasImageSource | null;

	public render(): void {
		if (this.isDisposed()) return;
		if (!this.info) return;

		const ctx = this._context.get2dGl();
		if (!ctx) {
			app.console.error('Canvas 2D context not available');
			return;
		}

		const drawable = this.getDrawable();
		if (!drawable) return;

		const width = ctx.canvas.width;
		const height = ctx.canvas.height;
		const pos2d = this.info.pos2d;

		ctx.drawImage(
			drawable,
			pos2d[0] * width,
			pos2d[2] * height,
			pos2d[1] * width - pos2d[0] * width,
			pos2d[3] * height - pos2d[2] * height,
		);
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
				out vec2 v_texCoord;

				void main() {
					gl_Position = a_position;
					v_texCoord = a_texCoord;
				}
				`;
	}

	private static getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D slideTexture;

				in vec2 v_texCoord;
				out vec4 outColor;

				void main() {
					outColor = texture(slideTexture, v_texCoord);
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
	): WebGLVertexArrayObject | null {
		if (this.isDisposed()) return null;
		if (this._context.is2dGl()) return null;

		const gl = this._context.getGl();

		// 5 numbers -> 3 x vertex X,Y,Z and 2x texture X,Y
		const positions = new Float32Array([
			//    vX    vY   vZ   tX   tY
			...[xMin, -yMin, 0.0, 0.0, 1.0],
			...[xMax, -yMin, 0.0, 1.0, 1.0],
			...[xMin, -yMax, 0.0, 0.0, 0.0],
			...[xMax, -yMax, 0.0, 1.0, 0.0],
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

	public render() {
		if (this.isDisposed()) return;
		if (!this.info) return;

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
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
		gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
	}
}
