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

/*
 * SlideshowShaders maps a file name under src/slideshow/shaders to the source of that shader.
 * The build generates it as plain JS from the .glsl files and bundles it, the same way as the
 * backstage icons, because combining compiled TS with generated TS does not work in
 * out-of-tree builds.
 */
declare const SlideshowShaders: Record<string, string>;

class TransitionParameters3D extends TransitionParameters {
	public leavingPrimitives: Primitive[] = [];
	public enteringPrimitives: Primitive[] = [];
	public allOperations: Operation[] = [];
}

class Transition3d extends TransitionBase {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
		if (this.context.isDisposed()) return;
		this.gl.enable(this.gl.BLEND);
		this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
	}

	protected getVertexShader(): string {
		return `#version 300 es
				precision mediump float;

				in vec4 a_position;
				in vec3 a_normal;
				in vec2 a_texCoord;

				uniform mat4 u_projectionMatrix;
				uniform mat4 u_modelViewMatrix;
				uniform mat4 u_sceneTransformMatrix;
				uniform mat4 u_primitiveTransformMatrix;
				uniform mat4 u_operationsTransformMatrix;

				out vec2 v_texturePosition;
				out vec3 v_normal;

				mat4 customTranspose(mat4 m) {
					return mat4(
						m[0][0], m[1][0], m[2][0], m[3][0],
						m[0][1], m[1][1], m[2][1], m[3][1],
						m[0][2], m[1][2], m[2][2], m[3][2],
						m[0][3], m[1][3], m[2][3], m[3][3]
					);
				}

				mat4 customInverse(mat4 m) {
					float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];
					float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];
					float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];
					float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];

					float b00 = a00 * a11 - a01 * a10;
					float b01 = a00 * a12 - a02 * a10;
					float b02 = a00 * a13 - a03 * a10;
					float b03 = a01 * a12 - a02 * a11;
					float b04 = a01 * a13 - a03 * a11;
					float b05 = a02 * a13 - a03 * a12;
					float b06 = a20 * a31 - a21 * a30;
					float b07 = a20 * a32 - a22 * a30;
					float b08 = a20 * a33 - a23 * a30;
					float b09 = a21 * a32 - a22 * a31;
					float b10 = a21 * a33 - a23 * a31;
					float b11 = a22 * a33 - a23 * a32;

					float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

					return mat4(
						a11 * b11 - a12 * b10 + a13 * b09,
						a02 * b10 - a01 * b11 - a03 * b09,
						a31 * b05 - a32 * b04 + a33 * b03,
						a22 * b04 - a21 * b05 - a23 * b03,
						a12 * b08 - a10 * b11 - a13 * b07,
						a00 * b11 - a02 * b08 + a03 * b07,
						a32 * b02 - a30 * b05 - a33 * b01,
						a20 * b05 - a22 * b02 + a23 * b01,
						a10 * b10 - a11 * b08 + a13 * b06,
						a01 * b08 - a00 * b10 - a03 * b06,
						a30 * b04 - a31 * b02 + a33 * b00,
						a21 * b02 - a20 * b04 - a23 * b00,
						a11 * b07 - a10 * b09 - a12 * b06,
						a00 * b09 - a01 * b07 + a02 * b06,
						a31 * b01 - a30 * b03 - a32 * b00,
						a20 * b03 - a21 * b01 + a22 * b00) / det;
				}

				void main(void) {
					mat4 modelViewMatrix = u_modelViewMatrix * u_operationsTransformMatrix * u_sceneTransformMatrix * u_primitiveTransformMatrix;
					mat3 normalMatrix = mat3(customTranspose(customInverse(modelViewMatrix)));
					gl_Position = u_projectionMatrix * modelViewMatrix * a_position;
					v_texturePosition = a_texCoord;
					v_normal = normalize(normalMatrix * a_normal);
				}
				`;
	}

	protected getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D slideTexture;
				in vec2 v_texturePosition;
				in vec3 v_normal;

				out vec4 outColor;

				void main() {
					vec3 lightVector = vec3(0.0, 0.0, 1.0);
					float light = max(dot(lightVector, v_normal), 0.0);
					vec4 fragment = texture(slideTexture, v_texturePosition);
					vec4 black = vec4(0.0, 0.0, 0.0, fragment.a);
					outColor = mix(black, fragment, light);
				}
				`;
	}

	private calculateModelViewMatrix() {
		const EyePos = 10.0;
		const modelView = glMatrix.mat4.create();

		glMatrix.mat4.translate(
			modelView,
			modelView,
			new Float32Array([0, 0, -EyePos]),
		);

		return modelView;
	}

	private calculateProjectionMatrix() {
		const EyePos = 10.0;
		const RealN = -1.0,
			RealF = 1.0;
		const RealL = -1.0,
			RealR = 1.0,
			RealB = -1.0,
			RealT = 1.0;
		const ClipN = EyePos + 5.0 * RealN;
		const ClipF = EyePos + 15.0 * RealF;
		const ClipL = RealL * 8.0;
		const ClipR = RealR * 8.0;
		const ClipB = RealB * 8.0;
		const ClipT = RealT * 8.0;

		const projection = glMatrix.mat4.create();
		glMatrix.mat4.frustum(projection, ClipL, ClipR, ClipB, ClipT, ClipN, ClipF);
		const scale = new Float32Array([
			1.0 /
				((RealR * 2.0 * ClipN) / (EyePos * (ClipR - ClipL)) -
					(ClipR + ClipL) / (ClipR - ClipL)),
			1.0 /
				((RealT * 2.0 * ClipN) / (EyePos * (ClipT - ClipB)) -
					(ClipT + ClipB) / (ClipT - ClipB)),
			1.0,
		]);
		glMatrix.mat4.scale(projection, projection, scale);
		return projection;
	}

	public initUniforms(): void {
		if (this.context.isDisposed()) return;

		this.gl.useProgram(this.program);

		const modelViewMatrix = this.calculateModelViewMatrix();
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_modelViewMatrix'),
			false,
			modelViewMatrix,
		);

		const projectionMatrix = this.calculateProjectionMatrix();
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_projectionMatrix'),
			false,
			projectionMatrix,
		);

		// prettier-ignore
		const sceneTransformMatrix = new Float32Array([
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ]);
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_sceneTransformMatrix'),
			false,
			sceneTransformMatrix,
		);

		// prettier-ignore
		const primitiveTransformMatrix = new Float32Array([
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ]);
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_primitiveTransformMatrix'),
			false,
			primitiveTransformMatrix,
		);

		// prettier-ignore
		const operationsTransformMatrix = new Float32Array([
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        ]);
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_operationsTransformMatrix'),
			false,
			operationsTransformMatrix,
		);

		this.otherUniformsInitialization();

		app.console.log('Uniforms initialized');
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public otherUniformsInitialization(): void {}

	public render(nT: number): void {
		if (this.context.isDisposed()) return;

		this.gl.viewport(
			0,
			0,
			this.context.canvas.width,
			this.context.canvas.height,
		);
		this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
		this.gl.clear(this.gl.COLOR_BUFFER_BIT);

		this.gl.useProgram(this.program);
		this.gl.uniform1f(this.gl.getUniformLocation(this.program, 'time'), nT);

		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.leavingSlide);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'leavingSlideTexture'),
			0,
		);

		this.gl.activeTexture(this.gl.TEXTURE1);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.enteringSlide);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'enteringSlideTexture'),
			1,
		);

		this.renderUniformValue();

		this.gl.bindVertexArray(this.vao);
		this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, 4);

		app.map.fire('newslideshowframe', {
			frame: this.gl.canvas,
		});
	}

	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public renderUniformValue(): void {}
}

SlideShow.Transition3d = Transition3d;
