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

abstract class SimpleTextureRenderer {
	protected gl: WebGL2RenderingContext;
	protected program: WebGLProgram;
	protected vao!: WebGLVertexArrayObject | null;
	protected context: RenderContextGl;
	protected positionBuffer: WebGLBuffer;

	constructor(canvasContext: RenderContextGl, createProgram: boolean = true) {
		this.context = canvasContext;
		this.gl = this.context.getGl();

		if (createProgram) this.createProgram();
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	protected initProgramTemplateParams(): void {}

	protected createProgram() {
		if (this.context.isDisposed()) return;

		this.initProgramTemplateParams();

		const vertexShaderSource = this.getVertexShader();
		const fragmentShaderSource = this.getFragmentShader();

		const vertexShader = this.context.createVertexShader(vertexShaderSource);
		const fragmentShader =
			this.context.createFragmentShader(fragmentShaderSource);

		this.program = this.context.createProgram(vertexShader, fragmentShader);
	}

	protected getVertexShader(): string {
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

	protected abstract getFragmentShader(): string;

	protected prepareTransition(): void {
		if (this.context.isDisposed()) return;

		this.initBuffers();
		this.initUniforms();
	}

	protected initUniforms(): void {
		if (this.context.isDisposed()) return;

		this.gl.useProgram(this.program);
		// Add more uniform here if needed.
	}

	private initBuffers(): void {
		const positions = new Float32Array([
			...[-1.0, -1.0, 0, 0, 1],
			...[1.0, -1.0, 0, 1, 1],
			...[-1.0, 1.0, 0, 0, 0],
			...[1.0, 1.0, 0, 1, 0],
		]);

		this.positionBuffer = this.gl.createBuffer();
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.positionBuffer);
		this.gl.bufferData(this.gl.ARRAY_BUFFER, positions, this.gl.STATIC_DRAW);

		this.vao = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vao);

		const positionLocation = this.gl.getAttribLocation(
			this.program,
			'a_position',
		);
		const texCoordLocation = this.gl.getAttribLocation(
			this.program,
			'a_texCoord',
		);

		if (positionLocation !== -1) {
			this.gl.enableVertexAttribArray(positionLocation);
			this.gl.vertexAttribPointer(
				positionLocation,
				3,
				this.gl.FLOAT,
				false,
				5 * 4,
				0,
			);
		}
		if (texCoordLocation !== -1) {
			this.gl.enableVertexAttribArray(texCoordLocation);
			this.gl.vertexAttribPointer(
				texCoordLocation,
				2,
				this.gl.FLOAT,
				false,
				5 * 4,
				3 * 4,
			);
		}
	}

	/// Draw the texture over the whole canvas, through the sampler uniform of the given name.
	protected drawTextureOverCanvas(
		texture: WebGLTexture,
		samplerName: string,
	): void {
		if (this.context.isDisposed()) return;
		const gl = this.gl;
		gl.viewport(0, 0, this.context.canvas.width, this.context.canvas.height);
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
		gl.clear(gl.COLOR_BUFFER_BIT);
		gl.useProgram(this.program);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.uniform1i(gl.getUniformLocation(this.program, samplerName), 0);
		gl.bindVertexArray(this.vao);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	public abstract render(nT: number): void;
}

// handle animation timing too
abstract class TextureAnimationBase extends SimpleTextureRenderer {
	protected time: number = 0;
	protected startTime: number | null = null;

	constructor(canvasContext: RenderContextGl) {
		super(canvasContext);
	}

	public abstract render(): void;
}
