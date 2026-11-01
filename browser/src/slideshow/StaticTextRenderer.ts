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
 * StaticTextRenderer is an empty canvas used to draw text, which we then load onto the WebGL canvas....
 * We are not rendering the text directly on the WebGL canvas. instead, we draw the text on a separate off-screen 2D canvas and use it as a texture in WebGL....
 */

declare var SlideShow: any;

class StaticTextRenderer extends TextureAnimationBase {
	public textTexture: WebGLTexture;

	constructor(canvasContext: RenderContextGl) {
		super(canvasContext);
	}

	public display(displayText: string) {
		if (this.context.isDisposed()) return;

		this.textTexture = this.createTextTexture(displayText);
		this.prepareTransition();
		requestAnimationFrame(this.animate.bind(this));
	}

	public animate(): void {
		requestAnimationFrame(this.render.bind(this));
	}

	public createTextTexture(displayText: string): WebGLTexture {
		const textCanvas = this.create2DCanvasWithText(displayText);
		return this.load2dCanvasToGlCanvas(textCanvas);
	}

	// Create an off-screen 2D canvas with text centered
	public create2DCanvasWithText(displayText: string): HTMLCanvasElement {
		if (this.context.isDisposed()) return null;

		const canvas = document.createElement('canvas');
		canvas.width = this.context.canvas.width;
		canvas.height = this.context.canvas.height;
		const ctx = canvas.getContext('2d');

		// Set background color
		ctx.fillStyle = 'black';
		ctx.fillRect(0, 0, canvas.width, canvas.height);

		// Set text attributes
		ctx.fillStyle = 'white';
		ctx.font = '20px sans-serif';
		ctx.textAlign = 'center';
		ctx.textBaseline = 'middle';

		ctx.fillText(displayText, canvas.width / 2, canvas.height / 2);

		return canvas;
	}

	public load2dCanvasToGlCanvas(canvas: HTMLCanvasElement): WebGLTexture {
		if (this.context.isDisposed()) return null;

		const texture = this.gl.createTexture();
		if (!texture) {
			throw new Error('Failed to create texture');
		}
		this.gl.bindTexture(this.gl.TEXTURE_2D, texture);
		this.gl.texImage2D(
			this.gl.TEXTURE_2D,
			0,
			this.gl.RGBA,
			this.gl.RGBA,
			this.gl.UNSIGNED_BYTE,
			canvas,
		);
		this.gl.texParameteri(
			this.gl.TEXTURE_2D,
			this.gl.TEXTURE_WRAP_S,
			this.gl.CLAMP_TO_EDGE,
		);
		this.gl.texParameteri(
			this.gl.TEXTURE_2D,
			this.gl.TEXTURE_WRAP_T,
			this.gl.CLAMP_TO_EDGE,
		);
		this.gl.texParameteri(
			this.gl.TEXTURE_2D,
			this.gl.TEXTURE_MIN_FILTER,
			this.gl.LINEAR,
		);
		this.gl.texParameteri(
			this.gl.TEXTURE_2D,
			this.gl.TEXTURE_MAG_FILTER,
			this.gl.LINEAR,
		);

		return texture;
	}

	public render(): void {
		this.drawTextureOverCanvas(this.textTexture, 'u_texture');
	}

	public getFragmentShader(): string {
		return `#version 300 es
			precision highp float;
			in vec2 v_texCoord;
			uniform sampler2D u_texture;
			out vec4 outColor;

			void main() {
				outColor = texture(u_texture, v_texCoord);
			}`;
	}
}

SlideShow.StaticTextRenderer = StaticTextRenderer;
