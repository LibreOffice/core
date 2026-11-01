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
 * SlideMirror shows the frames of the slideshow on a canvas in another window, such as the current
 * slide of the presenter console. A WebGL context draws only into its own canvas, and its textures
 * cannot be used by another context, so the mirror has a context of its own. A frame that the
 * compositor draws straight into the slideshow canvas is drawn into the mirror as well, from
 * layers that the compositor keeps for the mirror's context at the size of its canvas. No pixels
 * are read back from the GPU for such a frame. A frame that comes as an image, such as a frame
 * of a slide transition, is uploaded and drawn over the whole canvas.
 */

class SlideMirror extends SimpleTextureRenderer {
	// The texture that holds the last image drawn, null until the first one.
	private imageTexture: WebGLTexture | null = null;

	/// Throws when the canvas has no WebGL2 context.
	constructor(canvas: HTMLCanvasElement) {
		// The compositor clips with the stencil buffer.
		super(new RenderContextGl(canvas, { stencil: true }));
		this.prepareTransition();
	}

	protected getFragmentShader(): string {
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

	/// Draw the current frame of the slide, in the state of its animations. False when the
	/// compositor does not draw the slide this way.
	public drawSlide(slideIndex: number): boolean {
		if (this.context.isDisposed()) return false;
		const compositor = app.map.slideShowPresenter?._slideCompositor;
		return !!compositor && compositor.drawFrameInto(this.context, slideIndex);
	}

	/// Draw the image over the whole canvas.
	public drawImage(image: TexImageSource): void {
		if (this.context.isDisposed()) return;
		const gl = this.gl;
		if (!this.imageTexture) {
			this.imageTexture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, this.imageTexture);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		}
		gl.bindTexture(gl.TEXTURE_2D, this.imageTexture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
		this.render();
	}

	/// Draw the last image again.
	public render(): void {
		if (this.imageTexture)
			this.drawTextureOverCanvas(this.imageTexture, 'slideTexture');
	}

	/// Free the objects on the GPU.
	public dispose(): void {
		if (this.context.isDisposed()) return;
		const gl = this.gl;
		gl.deleteTexture(this.imageTexture);
		gl.deleteProgram(this.program);
		gl.deleteVertexArray(this.vao);
		gl.deleteBuffer(this.positionBuffer);
		this.imageTexture = null;
		this.context.clear();
	}
}

SlideShow.SlideMirror = SlideMirror;
