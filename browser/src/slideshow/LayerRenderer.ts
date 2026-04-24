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
 *
 */

declare var SlideShow: any;

interface LayerRenderer {
	initialize(): void;
	clearCanvas(): void;
	drawBitmap(
		imageInfo: ImageInfo | ImageBitmap,
		properties?: AnimatedElementRenderProperties,
	): void;
	dispose(): void;
	isDisposed(): boolean;
	fillColor(slideInfo: SlideInfo): boolean;
	isGlRenderer(): boolean;
	getRenderContext(): RenderContext;
}

interface RenderUniforms {
	bounds: BoundsType | null;
	alpha: number;
	fromFillColor: Float32Array;
	toFillColor: Float32Array;
	fromLineColor: Float32Array;
	toLineColor: Float32Array;
}

class LayerRendererGl implements LayerRenderer {
	private static readonly DefaultVertices = [-1, -1, 1, -1, -1, 1, 1, 1];
	private static readonly DefaultFromColor = new Float32Array([0, 0, 0, 0]);
	private static readonly DefaultToColor = new Float32Array([0, 0, 0, 0]);
	private offscreenCanvas: OffscreenCanvas;
	private glContext: RenderContextGl;
	private gl: WebGL2RenderingContext;
	private program: WebGLProgram;
	private vao: WebGLVertexArrayObject;
	private positionBuffer: WebGLBuffer;
	private texCoordBuffer: WebGLBuffer;
	private positionLocation: number;
	private texCoordLocation: number;
	private samplerLocation: WebGLUniformLocation;
	private imageBitmapIdCounter = 0;
	private textureCache: Map<string, WebGLTexture> = new Map();
	private imageBitmapIdMap = new WeakMap<ImageBitmap, number>();
	private disposed: boolean;

	constructor(offscreenCanvas: OffscreenCanvas) {
		this.offscreenCanvas = offscreenCanvas;
		this.glContext = new RenderContextGl(this.offscreenCanvas);
		this.gl = this.glContext.getGl();
		this.initializeWebGL();
		this.disposed = false;
	}

	initialize(): void {
		// do nothing!
	}

	isGlRenderer(): boolean {
		return true;
	}

	isDisposed(): boolean {
		return this.disposed;
	}

	public getRenderContext(): RenderContextGl {
		return this.glContext;
	}

	private vertexShaderSource = `
			attribute vec2 a_position;
			attribute vec2 a_texCoord;
			varying vec2 v_texCoord;
			void main() {
				gl_Position = vec4(a_position, 0, 1);
				v_texCoord = a_texCoord;
			}
		`;

	private fragmentShaderSource = `
		precision mediump float;
		uniform vec4 fromFillColor;
		uniform vec4 toFillColor;
		uniform vec4 fromLineColor;
		uniform vec4 toLineColor;
		uniform float alpha;
		varying vec2 v_texCoord;
		uniform sampler2D u_sampler;

		${GlHelpers.nearestPointOnSegment}
		${GlHelpers.computeColor}

		void main() {
			vec4 color = texture2D(u_sampler, v_texCoord);
			color = computeColor(color);
			color = color * alpha;
			gl_FragColor = color;
		}
		`;

	private initializeWebGL() {
		const gl = this.gl;

		// Compile shaders using RenderContextGl
		const vertexShader = this.glContext.createVertexShader(
			this.vertexShaderSource,
		);
		const fragmentShader = this.glContext.createFragmentShader(
			this.fragmentShaderSource,
		);

		// Link program using RenderContextGl
		this.program = this.glContext.createProgram(vertexShader, fragmentShader);

		// Get attribute and uniform locations
		this.positionLocation = gl.getAttribLocation(this.program, 'a_position');
		this.texCoordLocation = gl.getAttribLocation(this.program, 'a_texCoord');
		this.samplerLocation = gl.getUniformLocation(this.program, 'u_sampler');

		// Create buffers
		this.positionBuffer = gl.createBuffer();

		this.texCoordBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
		const texCoords = new Float32Array([0, 1, 1, 1, 0, 0, 1, 0]);
		gl.bufferData(gl.ARRAY_BUFFER, texCoords, gl.STATIC_DRAW);

		this.vao = gl.createVertexArray();
		gl.bindVertexArray(this.vao);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
		gl.enableVertexAttribArray(this.positionLocation);
		gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, 0, 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
		gl.enableVertexAttribArray(this.texCoordLocation);
		gl.vertexAttribPointer(this.texCoordLocation, 2, gl.FLOAT, false, 0, 0);

		gl.enable(gl.BLEND);
		gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
		gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, true);
		this.gl.disable(this.gl.DEPTH_TEST);
	}

	private initPositionBuffer(bounds: BoundsType) {
		const gl = this.gl;

		let vertices = LayerRendererGl.DefaultVertices;
		if (bounds) {
			vertices = [];
			// convert [0,1] => [-1,1]
			for (let i = 0; i < bounds.length; ++i) {
				const x = 2 * bounds[i].x - 1;
				vertices.push(x);
				// flip y coordinates
				const y = -(2 * bounds[i].y - 1);
				vertices.push(y);
			}
		}

		gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
		const positions = new Float32Array(vertices);
		gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);
	}

	clearCanvas(): void {
		if (!this.disposed) {
			const gl = this.gl;
			gl.clearColor(1.0, 1.0, 1.0, 1.0); // Clear to white
			gl.clear(gl.COLOR_BUFFER_BIT);
		}
	}

	public static computeColor(
		properties?: AnimatedElementRenderProperties,
	): RenderUniforms {
		let bounds: BoundsType = null;
		let alpha = 1.0;
		let fromFillColor = LayerRendererGl.DefaultFromColor;
		let toFillColor = LayerRendererGl.DefaultToColor;
		let fromLineColor = LayerRendererGl.DefaultFromColor;
		let toLineColor = LayerRendererGl.DefaultToColor;
		if (properties) {
			bounds = properties.bounds;
			alpha = properties.alpha;
			const colorMap = properties.colorMap;
			if (colorMap) {
				if (colorMap.fromFillColor && colorMap.toFillColor) {
					fromFillColor = colorMap.fromFillColor.toFloat32Array();
					toFillColor = colorMap.toFillColor.toFloat32Array();
				}
				if (colorMap.fromLineColor && colorMap.toLineColor) {
					fromLineColor = colorMap.fromLineColor.toFloat32Array();
					toLineColor = colorMap.toLineColor.toFloat32Array();
				}
			}
		}
		return {
			bounds,
			alpha,
			fromFillColor,
			toFillColor,
			fromLineColor,
			toLineColor,
		};
	}

	drawBitmap(
		imageInfo: ImageInfo | ImageBitmap,
		properties?: AnimatedElementRenderProperties,
	): void {
		if (this.disposed) {
			app.console.log('LayerRenderer is disposed');
			return;
		}
		if (!imageInfo) {
			app.console.log('LayerRenderer.drawBitmap: no image');
			return;
		}

		const {
			bounds,
			alpha,
			fromFillColor,
			toFillColor,
			fromLineColor,
			toLineColor,
		} = LayerRendererGl.computeColor(properties);

		let texture: WebGLTexture;
		let textureKey: string;

		if (imageInfo instanceof ImageBitmap) {
			if (!this.imageBitmapIdMap.has(imageInfo)) {
				this.imageBitmapIdMap.set(imageInfo, this.imageBitmapIdCounter++);
			}
			textureKey = `imageBitmap_${this.imageBitmapIdMap.get(imageInfo)}`;
		} else {
			textureKey = imageInfo.checksum;
		}

		if (this.textureCache.has(textureKey)) {
			texture = this.textureCache.get(textureKey);
			// app.console.debug(`LayerDrawing.drawBitmap: cache hit: key: ${textureKey}`);
		} else {
			if (imageInfo instanceof ImageBitmap) {
				texture = this.glContext.loadTexture(imageInfo, false, true);
			} else {
				texture = this.glContext.loadTexture(
					imageInfo.data as HTMLImageElement,
					false,
					true,
				);
			}
			this.textureCache.set(textureKey, texture);
		}

		this.gl.useProgram(this.program);
		this.gl.bindVertexArray(this.vao);

		this.initPositionBuffer(bounds);

		this.gl.uniform1f(this.gl.getUniformLocation(this.program, 'alpha'), alpha);
		this.gl.uniform4fv(
			this.gl.getUniformLocation(this.program, 'fromFillColor'),
			fromFillColor,
		);
		this.gl.uniform4fv(
			this.gl.getUniformLocation(this.program, 'toFillColor'),
			toFillColor,
		);
		this.gl.uniform4fv(
			this.gl.getUniformLocation(this.program, 'fromLineColor'),
			fromLineColor,
		);
		this.gl.uniform4fv(
			this.gl.getUniformLocation(this.program, 'toLineColor'),
			toLineColor,
		);

		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, texture);
		this.gl.uniform1i(this.samplerLocation, 0);

		this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, 4);
	}

	dispose(): void {
		this.gl = null;
		this.offscreenCanvas = null;
		this.disposed = true;
	}

	hexToRgba(
		hex: string,
	): { r: number; g: number; b: number; a: number } | null {
		hex = hex.replace(/^#/, '');
		let bigint: number;
		if (hex.length === 3) {
			const r = parseInt(hex.charAt(0) + hex.charAt(0), 16);
			const g = parseInt(hex.charAt(1) + hex.charAt(1), 16);
			const b = parseInt(hex.charAt(2) + hex.charAt(2), 16);
			return { r, g, b, a: 0 };
		} else if (hex.length === 6) {
			bigint = parseInt(hex, 16);
			const r = (bigint >> 16) & 255;
			const g = (bigint >> 8) & 255;
			const b = bigint & 255;
			return { r, g, b, a: 0 };
		} else if (hex.length === 8) {
			bigint = parseInt(hex, 16);
			const r = (bigint >> 24) & 255;
			const g = (bigint >> 16) & 255;
			const b = (bigint >> 8) & 255;
			const a = bigint & 255;
			return { r, g, b, a };
		} else if (hex.length === 4) {
			const r = parseInt(hex.charAt(0) + hex.charAt(0), 16);
			const g = parseInt(hex.charAt(1) + hex.charAt(1), 16);
			const b = parseInt(hex.charAt(2) + hex.charAt(2), 16);
			const a = parseInt(hex.charAt(3) + hex.charAt(2), 16);
			return { r, g, b, a };
		} else {
			return null;
		}
	}

	fillColor(slideInfo: SlideInfo): boolean {
		if (this.disposed) return true; // done

		if (slideInfo.background && slideInfo.background.fillColor) {
			const fillColor = slideInfo.background.fillColor;
			const rgba = this.hexToRgba(fillColor);
			if (rgba) {
				this.gl.clearColor(
					rgba.r / 255,
					rgba.g / 255,
					rgba.b / 255,
					rgba.a / 255,
				);
			} else {
				this.gl.clearColor(1.0, 1.0, 1.0, 1.0);
			}
			this.gl.clear(this.gl.COLOR_BUFFER_BIT);
		} else {
			this.gl.clearColor(1.0, 1.0, 1.0, 1.0);
			this.gl.clear(this.gl.COLOR_BUFFER_BIT);
		}

		if (!slideInfo.background) return true;
		return false;
	}
}

class LayerRenderer2d implements LayerRenderer {
	private offscreenCanvas: OffscreenCanvas;
	private offscreenContext: OffscreenCanvasRenderingContext2D;
	private context2d: RenderContext2d;
	private disposed: boolean;

	constructor(offscreenCanvas: OffscreenCanvas) {
		this.offscreenCanvas = offscreenCanvas;
		this.context2d = new RenderContext2d(this.offscreenCanvas);
		this.offscreenContext = this.context2d.get2dOffscreen();
		if (!this.offscreenContext) {
			throw new Error('2D Canvas context not available');
		}
		this.disposed = false;
	}

	initialize(): void {
		// Initialization is handled in the constructor
	}

	isGlRenderer(): boolean {
		return false;
	}

	isDisposed(): boolean {
		return this.disposed;
	}

	getRenderContext(): RenderContext {
		return this.context2d;
	}

	clearCanvas(): void {
		if (this.disposed) return;
		this.offscreenContext.clearRect(
			0,
			0,
			this.offscreenCanvas.width,
			this.offscreenCanvas.height,
		);
		this.offscreenContext.fillStyle = '#FFFFFF';
		this.offscreenContext.fillRect(
			0,
			0,
			this.offscreenCanvas.width,
			this.offscreenCanvas.height,
		);
	}

	drawBitmap(
		imageInfo: ImageInfo | ImageBitmap,
		properties?: AnimatedElementRenderProperties,
	): void {
		if (this.disposed) return;
		if (!imageInfo) {
			app.console.log('Canvas2DRenderer.drawBitmap: no image');
			return;
		}
		if (imageInfo instanceof ImageBitmap) {
			this.offscreenContext.drawImage(imageInfo, 0, 0);
		} else if (
			imageInfo.type === 'png' ||
			imageInfo.data instanceof ImageBitmap
		) {
			this.offscreenContext.drawImage(imageInfo.data as HTMLImageElement, 0, 0);
		} else {
			throw 'No supported bitmap to draw found';
		}
	}

	dispose(): void {
		// Cleanup references
		this.disposed = true;
		this.offscreenContext = null;
		this.offscreenCanvas = null;
	}

	fillColor(slideInfo: SlideInfo): boolean {
		if (this.disposed) return;

		// always draw a solid white rectangle behind the background
		this.offscreenContext.fillStyle = '#FFFFFF';
		this.offscreenContext.fillRect(
			0,
			0,
			this.offscreenCanvas.width,
			this.offscreenCanvas.height,
		);

		if (!slideInfo.background) return true;
		if (slideInfo.background.fillColor) {
			this.offscreenContext.fillStyle = '#' + slideInfo.background.fillColor;
			window.app.console.log(
				'LayerDrawing.drawBackground: ' + this.offscreenContext.fillStyle,
			);
			this.offscreenContext.fillRect(
				0,
				0,
				this.offscreenCanvas.width,
				this.offscreenCanvas.height,
			);
			return true;
		}

		return false;
	}
}

SlideShow.LayerRendererGl = LayerRendererGl;
SlideShow.LayerRenderer2d = LayerRenderer2d;
