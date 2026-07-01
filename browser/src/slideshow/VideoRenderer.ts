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

class VideoRenderInfo {
	private texture: WebGLTexture | ImageBitmap = null;
	public videoElement: HTMLVideoElement;
	private vao: WebGLVertexArrayObject = null;
	public pos2d: number[];
	public playing: boolean;
	public ended: boolean;

	public getTexture(): WebGLTexture {
		return this.texture;
	}

	public replaceTexture(context: RenderContext, newtexture: WebGLTexture) {
		context.deleteTexture(this.texture);
		this.texture = newtexture;
	}

	public getVao(): WebGLVertexArrayObject {
		return this.vao;
	}

	public replaceVao(context: RenderContext, newVao: WebGLVertexArrayObject) {
		context.deleteVertexArray(this.vao);
		this.vao = newVao;
	}

	public deleteResources(context: RenderContext) {
		this.replaceTexture(context, null);
		this.replaceVao(context, null);
	}
}

abstract class VideoRenderer {
	protected sId: string;
	protected _context: RenderContext;
	protected _slideRenderer: SlideRenderer;
	protected videoRenderInfo: VideoRenderInfo;
	public videoInfoId: number;

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

	public abstract prepareVideo(
		videoInfo: VideoInfo,
		docWidth: number,
		docHeight: number,
	): void;

	public loadVideo() {
		if (!this.videoRenderInfo) return;
		this.videoRenderInfo.videoElement.load();
	}

	public playVideo(reset: boolean = true) {
		if (!this.videoRenderInfo) return;
		if (reset) this.videoRenderInfo.videoElement.currentTime = 0;
		this.videoRenderInfo.videoElement.play();
	}

	public pauseVideo() {
		if (!this.videoRenderInfo) return;
		app.console.debug('VideoRenderer.pauseVideo');
		this.videoRenderInfo.videoElement.pause();
	}

	public deleteResources(): void {
		if (this.isDisposed()) return;
		this.pauseVideo();
		this.videoRenderInfo.deleteResources(this._context);
	}

	public handleClick(): void {
		if (this.videoRenderInfo.playing) {
			this.pauseVideo();
		} else if (this.videoRenderInfo.ended) {
			this.playVideo(true);
		} else {
			this.playVideo(false);
		}
	}

	protected setupVideo(
		videoRenderInfo: VideoRenderInfo,
		url: string,
	): HTMLVideoElement {
		const video = document.createElement('video');

		video.playsInline = true;
		video.crossOrigin = 'anonymous';

		video.addEventListener(
			'playing',
			() => {
				videoRenderInfo.playing = true;
				videoRenderInfo.ended = false;
				this._slideRenderer.notifyVideoStarted(this.sId);
			},
			true,
		);

		video.addEventListener(
			'pause',
			() => {
				videoRenderInfo.playing = false;
				this._slideRenderer.notifyVideoEnded(this.sId);
			},
			true,
		);

		video.addEventListener(
			'ended',
			() => {
				videoRenderInfo.playing = false;
				videoRenderInfo.ended = true;
				this._slideRenderer.notifyVideoEnded(this.sId);
			},
			true,
		);

		video.src = url;
		return video;
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

	public abstract render(): void;
}

class VideoRenderer2d extends VideoRenderer {
	public prepareVideo(
		videoInfo: VideoInfo,
		docWidth: number,
		docHeight: number,
	): void {
		this.pauseVideo();

		const video = new VideoRenderInfo();
		video.videoElement = this.setupVideo(video, videoInfo.url);
		video.pos2d = this.getDocumentPositions(
			videoInfo.x,
			videoInfo.y,
			videoInfo.width,
			videoInfo.height,
			docWidth,
			docHeight,
		);
		this.videoRenderInfo = video;
		this.videoInfoId = videoInfo.id;
	}

	public render(): void {
		if (this.isDisposed()) return;
		const ctx = this._context.get2dGl();
		if (!ctx) {
			app.console.error('Canvas 2D context not available');
			return;
		}
		const video = this.videoRenderInfo;

		const width = ctx.canvas.width;
		const height = ctx.canvas.height;

		ctx.drawImage(
			video.videoElement,
			video.pos2d[0] * width,
			video.pos2d[2] * height,
			video.pos2d[1] * width - video.pos2d[0] * width,
			video.pos2d[3] * height - video.pos2d[2] * height,
		);
	}
}

class VideoRendererGl extends VideoRenderer {
	private static _program: WebGLProgram;
	public static videoProgramInitialized: boolean = false;

	// WebGL reads the raw decoded video frame and ignores any rotation
	// recorded in the container (for example the display matrix that
	// phone-recorded videos carry), so a video whose frames are stored
	// sideways would end up sideways in the texture too. A 2D canvas
	// context applies that rotation when it draws a video frame, so the
	// frame is normalized through one before it is uploaded as a texture.
	private frameCanvas: HTMLCanvasElement;
	private frameContext: CanvasRenderingContext2D;

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

	public static createProgram(context: RenderContextGl) {
		if (context.is2dGl() || context.isDisposed()) return;

		const vertexShader = context.createVertexShader(
			VideoRendererGl.getVertexShader(),
		);
		const fragmentShader = context.createFragmentShader(
			VideoRendererGl.getFragmentShader(),
		);

		VideoRendererGl._program = context.createProgram(
			vertexShader,
			fragmentShader,
		);

		VideoRendererGl.videoProgramInitialized = true;
	}

	public static deleteProgram(context: RenderContextGl) {
		if (context.is2dGl() || context.isDisposed()) return;
		const gl = context.getGl();
		gl.deleteProgram(VideoRendererGl._program);
		VideoRendererGl._program = null;
		VideoRendererGl.videoProgramInitialized = false;
	}

	public prepareVideo(
		videoInfo: VideoInfo,
		docWidth: number,
		docHeight: number,
	) {
		this.pauseVideo();

		const video = new VideoRenderInfo();
		video.videoElement = this.setupVideo(video, videoInfo.url);
		video.replaceTexture(this._context, this.initTexture());
		video.replaceVao(
			this._context,
			this.setupRectangleInDocumentPositions(
				videoInfo.x,
				videoInfo.y,
				videoInfo.width,
				videoInfo.height,
				docWidth,
				docHeight,
			),
		);
		this.videoRenderInfo = video;
		this.videoInfoId = videoInfo.id;
	}

	private initTexture() {
		const gl = this._context.getGl();
		const texture = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, texture);

		const pixel = new Uint8Array([0, 0, 255, 255]); // opaque blue
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			1,
			1,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			pixel,
		);

		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

		return texture;
	}

	// jscpd:ignore-start
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

		const program = VideoRendererGl._program;
		if (!program) {
			app.console.log('VideoRenderer: program is not valid');
			return;
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

	private setupRectangleInDocumentPositions(
		x: number,
		y: number,
		width: number,
		height: number,
		docWidth: number,
		docHeight: number,
	): WebGLVertexArrayObject {
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

	private getOrientedFrame(video: HTMLVideoElement): HTMLCanvasElement {
		if (!this.frameCanvas) {
			this.frameCanvas = document.createElement('canvas');
			this.frameContext = this.frameCanvas.getContext('2d');
		}

		if (
			this.frameCanvas.width !== video.videoWidth ||
			this.frameCanvas.height !== video.videoHeight
		) {
			this.frameCanvas.width = video.videoWidth;
			this.frameCanvas.height = video.videoHeight;
		}

		this.frameContext.drawImage(video, 0, 0);
		return this.frameCanvas;
	}

	private updateTexture(texture: WebGLTexture, video: HTMLVideoElement) {
		const gl = this._context.getGl();
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			this.getOrientedFrame(video),
		);
	}

	public render() {
		if (this.isDisposed()) return;

		app.console.debug('SlideRendererGl.render');
		const gl = this._context.getGl();

		if (!VideoRendererGl._program) {
			app.console.log('VideoRendererGl: program is not valid');
			return;
		}

		gl.useProgram(VideoRendererGl._program);
		gl.activeTexture(gl.TEXTURE0);

		const video = this.videoRenderInfo;
		if (!video.ended && video.videoElement.currentTime > 0) {
			gl.bindVertexArray(video.getVao());
			gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
			this.updateTexture(video.getTexture(), video.videoElement);
			gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
			gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
		}
	}
}

function makeVideoRenderer(
	sId: string,
	context: RenderContext,
	slideRenderer: SlideRenderer,
): VideoRenderer {
	return context.is2dGl()
		? new VideoRenderer2d(sId, context, slideRenderer)
		: new VideoRendererGl(sId, context, slideRenderer);
}
