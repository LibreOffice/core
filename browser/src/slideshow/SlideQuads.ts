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
 * Draws a slide frame as textured quads in the WebGL context of a slideshow canvas, into the
 * canvas or into a texture. The textures hold premultiplied colors and the quads are blended in
 * the order they are drawn. The current frame of a video is uploaded into a texture of its own.
 *
 * Rectangles are in canvas pixels as [x, y, width, height], with the slide filling the canvas
 * and y growing downwards.
 */

/// A framebuffer with the texture as its color and a stencil buffer of the same size.
interface TextureTarget {
	framebuffer: WebGLFramebuffer | null;
	stencil: WebGLRenderbuffer | null;
}

class SlideQuadsGl {
	public readonly context: RenderContextGl;
	private readonly quadProgram: WebGLProgram;
	private readonly quadArray: WebGLVertexArrayObject | null;
	// The size of the frame being drawn, in pixels.
	private width = 0;
	private height = 0;
	// 1 while a frame goes into the canvas, -1 while it goes into a texture.
	private direction = 1;
	// The texture the frame goes into, and its target, while one is drawn into.
	private texture: WebGLTexture | null = null;
	private target: TextureTarget | null = null;
	// Whether this context can draw into a texture with a stencil buffer, null until the first
	// texture target is made.
	private targetsComplete: boolean | null = null;
	// The texture of the current frame of each video, by a key its owner gives.
	private frameTextures = new Map<number, WebGLTexture | null>();
	// Sources whose pictures the browser refused to upload.
	private refusedSources = new WeakSet<TexImageSource>();

	constructor(context: RenderContextGl) {
		this.context = context;
		const gl = context.getGl();

		this.quadProgram = context.createProgram(
			context.createVertexShader(SlideshowShaders['SlideQuad.vert.glsl']),
			context.createFragmentShader(SlideshowShaders['SlideQuad.frag.glsl']),
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
		gl.bindVertexArray(null);
	}

	public get frameWidth(): number {
		return this.width;
	}

	public get frameHeight(): number {
		return this.height;
	}

	/// 1 while the frame goes into the canvas, -1 while it goes into a texture, which holds its
	/// top row first.
	public get yDirection(): number {
		return this.direction;
	}

	/// Start a frame of width by height pixels, cleared to black. It goes into texture when
	/// given, resized to the frame, else into the canvas, which then has the frame's size. False
	/// when the texture cannot be drawn into.
	public begin(
		width: number,
		height: number,
		texture?: WebGLTexture | null,
	): boolean {
		if (this.context.isDisposed()) return false;
		const gl = this.context.getGl();
		this.width = width;
		this.height = height;
		this.texture = texture ?? null;
		this.target = texture ? this._targetTexture(texture) : null;
		if (texture && !this.target) return false;
		// A texture holds the top row first, like an uploaded image. The canvas shows its first
		// row at the bottom.
		this.direction = this.target ? -1 : 1;
		gl.viewport(0, 0, width, height);
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
		gl.clear(gl.COLOR_BUFFER_BIT);
		// The quads are flat and drawn in order, and a flipped one faces away, so every quad
		// is drawn whole. A 3D transition turns depth testing and face culling on in the same
		// context.
		gl.disable(gl.DEPTH_TEST);
		gl.disable(gl.CULL_FACE);
		// The textures hold premultiplied colors.
		gl.enable(gl.BLEND);
		gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
		return true;
	}

	/// Finish the frame and draw into the canvas again. A texture that samples between mipmap
	/// levels gets them made anew from the frame.
	public end(): void {
		const gl = this.context.getGl();
		gl.disable(gl.BLEND);
		gl.bindVertexArray(null);
		if (this.target) this._releaseTarget(this.texture, this.target);
		this.texture = null;
		this.target = null;
	}

	/// Draw texture over rectangle. transform maps the rectangle in canvas pixels, and alpha
	/// scales the colors.
	public drawQuad(
		texture: WebGLTexture | null,
		rectangle: number[],
		alpha: number = 1,
		transform: Float32Array = SlideQuadsGl.identity,
	): void {
		const gl = this.context.getGl();
		gl.useProgram(this.quadProgram);
		gl.bindVertexArray(this.quadArray);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.uniform1i(gl.getUniformLocation(this.quadProgram, 'u_texture'), 0);
		gl.uniform1f(gl.getUniformLocation(this.quadProgram, 'u_alpha'), alpha);
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
			this.width,
			this.height,
		);
		gl.uniform1f(
			gl.getUniformLocation(this.quadProgram, 'u_yDirection'),
			this.direction,
		);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	/// The rectangle of the frame in the frame being drawn. A flipped frame covers the same
	/// rectangle from its other edge.
	public frameRectangle(frame: ContentFrame): number[] {
		const [left, right, top, bottom] = frame.edges;
		let x = left * this.width;
		let y = top * this.height;
		let width = (right - left) * this.width;
		let height = (bottom - top) * this.height;
		if (frame.mirror & 1) {
			x += width;
			width = -width;
		}
		if (frame.mirror & 2) {
			y += height;
			height = -height;
		}
		return [x, y, width, height];
	}

	/// The texture of key with the picture of source uploaded into it, or null when the browser
	/// refuses to hand over the pictures of source.
	public frameTexture(
		key: number,
		source: TexImageSource,
	): WebGLTexture | null {
		if (this.refusedSources.has(source)) return null;
		const gl = this.context.getGl();
		let texture = this.frameTextures.get(key);
		if (!texture) {
			texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, texture);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			this.frameTextures.set(key, texture);
		}
		gl.bindTexture(gl.TEXTURE_2D, texture);
		if (!RenderContextGl.uploadPicture(gl, source)) {
			this.refusedSources.add(source);
			return null;
		}
		return texture;
	}

	/// Free the frame textures.
	public clearFrames(): void {
		if (!this.context.isDisposed()) {
			const gl = this.context.getGl();
			this.frameTextures.forEach((texture) => gl.deleteTexture(texture));
		}
		this.frameTextures.clear();
	}

	/// Free the frame textures and the program.
	public dispose(): void {
		this.clearFrames();
		if (this.context.isDisposed()) return;
		const gl = this.context.getGl();
		gl.deleteProgram(this.quadProgram);
		gl.deleteVertexArray(this.quadArray);
	}

	private static readonly identity = new Float32Array([
		1, 0, 0, 0, 1, 0, 0, 0, 1,
	]);

	// Size the texture to the frame and make it the draw target, with a stencil buffer for
	// clips. Null when the texture cannot be drawn into.
	private _targetTexture(texture: WebGLTexture): TextureTarget | null {
		if (this.targetsComplete === false) return null;
		const gl = this.context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			this.width,
			this.height,
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
			this.width,
			this.height,
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

	private _releaseTarget(
		texture: WebGLTexture | null,
		target: TextureTarget,
	): void {
		const gl = this.context.getGl();
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.deleteFramebuffer(target.framebuffer);
		gl.deleteRenderbuffer(target.stencil);
		gl.viewport(0, 0, this.context.canvas.width, this.context.canvas.height);
		this.direction = 1;
		if (!texture) return;

		if (!this.context.isMipMapped(texture)) return;
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.generateMipmap(gl.TEXTURE_2D);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}
}
