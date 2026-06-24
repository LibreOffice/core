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

declare var SlideShow: any;

abstract class SlideRenderer {
	public _context: RenderContext = null;
	protected _slideTexture: WebGLTexture | ImageBitmap;
	protected _canvas: HTMLCanvasElement;
	protected _renderedSlideIndex: number = undefined;
	protected _requestAnimationFrameId: number = null;
	private _activeLayers: Set<string> = new Set();
	private _playingVideos: Set<string> = new Set();

	protected constructor(canvas: HTMLCanvasElement) {
		this._canvas = canvas;
	}

	// Schedule on the window that owns the canvas. The slideshow can run in its
	// own window while the document window is hidden behind it, and a hidden
	// window's animation frames are paused, which would stop the slide drawing.
	protected requestFrame(callback: FrameRequestCallback): number {
		const canvasWindow = this._canvas?.ownerDocument?.defaultView;
		return (canvasWindow || window).requestAnimationFrame(callback);
	}

	public isDisposed() {
		return this._context && this._context.isDisposed();
	}

	public get lastRenderedSlideIndex() {
		return this._renderedSlideIndex;
	}

	public getSlideTexture(): WebGLTexture {
		return this._slideTexture;
	}

	public getAnimatedSlideImage(): ImageBitmap {
		const presenter: SlideShowPresenter = app.map.slideShowPresenter;
		return presenter._slideCompositor.getAnimatedSlide(
			this._renderedSlideIndex,
		);
	}

	public abstract deleteResources(): void;

	public renderSlide(
		currentSlideTexture: WebGLTexture | ImageBitmap,
		slideInfo: SlideInfo,
	) {
		this.deleteCurrentSlideTexture();
		this._activeLayers.clear();
		this._renderedSlideIndex = slideInfo.indexInSlideShow;
		this._slideTexture = currentSlideTexture;
		this.requestFrame(this.render.bind(this));
	}

	public abstract createTexture(
		image: ImageBitmap,
		isMipMapEnable?: boolean,
	): WebGLTexture | ImageBitmap;

	public abstract deleteCurrentSlideTexture(): void;

	protected abstract render(): void;

	public createEmptyTexture(): WebGLTexture | ImageBitmap {
		return null;
	}

	public notifyAnimationStarted(sId: string) {
		const isAnyLayerActive = this.isAnyLayerActive();
		this._activeLayers.add(sId);
		if (!isAnyLayerActive) {
			this._requestAnimationFrameId = this.requestFrame(this.render.bind(this));
		}
	}

	public notifyAnimationEnded(sId: string) {
		this._activeLayers.delete(sId);
	}

	public isAnyLayerActive(): boolean {
		return this._activeLayers.size > 0;
	}

	public notifyVideoStarted(sId: string) {
		const isAnyVideoAlreadyPlaying = this.isAnyVideoPlaying;
		this._playingVideos.add(sId);
		if (!isAnyVideoAlreadyPlaying) {
			this._requestAnimationFrameId = this.requestFrame(this.render.bind(this));
		}
	}

	public notifyVideoEnded(sId: string) {
		this._playingVideos.delete(sId);
	}

	public get isAnyVideoPlaying(): boolean {
		return this._playingVideos.size > 0;
	}
}

class SlideRenderer2d extends SlideRenderer {
	constructor(canvas: HTMLCanvasElement) {
		super(canvas);
		this._context = new RenderContext2d(canvas);
	}

	public createTexture(image: ImageBitmap, _isMipMapsEnable?: boolean) {
		return image;
	}

	public deleteCurrentSlideTexture(): void {
		return;
	}

	public deleteResources(): void {
		return;
	}

	protected render() {
		if (this.isDisposed()) return;

		const canvas2dCtx = this._context.get2dGl();
		if (!canvas2dCtx) {
			console.error('Canvas 2D context not available');
			return;
		}
		canvas2dCtx.clearRect(
			0,
			0,
			canvas2dCtx.canvas.width,
			canvas2dCtx.canvas.height,
		);

		const slideImage = this.getAnimatedSlideImage();
		app.map.fire('newslideshowframe', {
			frame: slideImage,
		});

		canvas2dCtx.drawImage(slideImage, 0, 0);
		slideImage.close();

		canvas2dCtx.setTransform(1, 0, 0, 1, 0, 0);

		if (this.isAnyLayerActive() || this.isAnyVideoPlaying) {
			this._requestAnimationFrameId = this.requestFrame(this.render.bind(this));
		}
	}
}

class SlideRendererGl extends SlideRenderer {
	private readonly _program: WebGLProgram = null;
	private readonly _vao: WebGLVertexArrayObject = null;

	constructor(canvas: HTMLCanvasElement) {
		super(canvas);
		this._context = new RenderContextGl(canvas);

		const vertexShader = this._context.createVertexShader(
			this.getVertexShader(),
		);
		const fragmentShader = this._context.createFragmentShader(
			this.getFragmentShader(),
		);

		this._program = this._context.createProgram(vertexShader, fragmentShader);

		this._vao = this.setupPositions(-1.0, 1.0, 1.0, -1.0);
		this._context.getGl().useProgram(this._program);
	}

	public getVertexShader(): string {
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

	public getFragmentShader(): string {
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

	private updateTexture(
		texture: WebGLTexture,
		video: HTMLVideoElement | ImageBitmap,
	) {
		const gl = this._context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video);
	}

	public setupPositions(
		xMin: number,
		xMax: number,
		yMin: number,
		yMax: number,
	): WebGLVertexArrayObject {
		if (this.isDisposed()) return null;

		if (this._context.is2dGl()) return;

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

		const positionLocation = gl.getAttribLocation(this._program, 'a_position');

		gl.enableVertexAttribArray(positionLocation);
		gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 5 * 4, 0);

		const texCoordLocation = gl.getAttribLocation(this._program, 'a_texCoord');

		gl.enableVertexAttribArray(texCoordLocation);
		gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 5 * 4, 3 * 4);

		return vao;
	}

	private getNextTexture(): WebGLTexture {
		const slideImage = this.getAnimatedSlideImage();
		app.map.fire('newslideshowframe', {
			frame: slideImage,
		});
		this.updateTexture(this._slideTexture, slideImage);
		slideImage.close();
		return this._slideTexture;
	}

	public createTexture(image: ImageBitmap, isMipMapsEnable: boolean = false) {
		return this._context.loadTexture(<any>image, isMipMapsEnable);
	}

	public createEmptyTexture(): WebGLTexture | ImageBitmap {
		return this._context.createEmptySlide();
	}

	public deleteCurrentSlideTexture(): void {
		this._context.deleteTexture(this._slideTexture);
		this._slideTexture = null;
	}

	public deleteResources(): void {
		if (this.isDisposed()) return;

		this.deleteCurrentSlideTexture();
		if (this._context) this._context.clear();
	}

	protected render() {
		if (this.isDisposed()) return;

		const gl = this._context.getGl();
		gl.viewport(0, 0, this._canvas.width, this._canvas.height);
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
		gl.clear(gl.COLOR_BUFFER_BIT);

		gl.useProgram(this._program);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, this.getNextTexture());
		gl.uniform1i(gl.getUniformLocation(this._program, 'slideTexture'), 0);

		gl.bindVertexArray(this._vao);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

		if (this.isAnyLayerActive() || this.isAnyVideoPlaying)
			this._requestAnimationFrameId = this.requestFrame(this.render.bind(this));
	}
}
