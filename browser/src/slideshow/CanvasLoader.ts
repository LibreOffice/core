// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Loader for slideshow - it shows animation for the user to indicate loading
 */

declare var SlideShow: any;

abstract class CanvasLoader {
	public abstract startLoader(): void;
	public abstract stopLoader(): void;
}

class CanvasLoader2d implements CanvasLoader {
	constructor(canvasContext: RenderContext2d) {} // eslint-disable-line

	public startLoader(): void {} // eslint-disable-line

	public stopLoader(): void {} // eslint-disable-line
}

class CanvasLoaderGl extends TextureAnimationBase implements CanvasLoader {
	private animationId: number | null = null;
	private uResolutionLoc: WebGLUniformLocation;
	private uTimeLoc: WebGLUniformLocation;

	constructor(canvasContext: RenderContextGl) {
		super(canvasContext);
		this.prepareTransition();

		this.uResolutionLoc = this.gl.getUniformLocation(
			this.program,
			'u_resolution',
		);
		this.uTimeLoc = this.gl.getUniformLocation(this.program, 'u_time');
	}

	public renderUniformValue(): void {
		if (this.context.isDisposed()) return;

		this.gl.uniform2f(
			this.uResolutionLoc,
			this.context.canvas.width,
			this.context.canvas.height,
		);
		this.gl.uniform1f(this.uTimeLoc, this.time);
	}

	public startLoader(): void {
		if (this.context.isDisposed()) return;

		if (this.animationId === null) {
			this.startTime = performance.now();
			this.animate();
		}
	}

	public stopLoader(): void {
		if (this.context.isDisposed()) return;

		if (this.animationId !== null) {
			cancelAnimationFrame(this.animationId);
			this.animationId = null;
			this.startTime = null;

			// Clear the canvas
			this.gl.clear(this.gl.COLOR_BUFFER_BIT);

			// Delete WebGL resources
			if (this.vao) {
				this.gl.deleteVertexArray(this.vao);
				this.vao = null;
			}

			if (this.program) {
				this.gl.deleteProgram(this.program);
				this.program = null;
			}

			// Unbind any bound buffers or textures
			this.gl.bindBuffer(this.gl.ARRAY_BUFFER, null);
			this.gl.bindVertexArray(null);

			// Reset WebGL state
			this.gl.useProgram(null);

			// Flush any pending WebGL commands
			this.gl.flush();
		}
	}

	private animate = (): void => {
		if (!this.startTime) this.startTime = performance.now();
		this.time = (performance.now() - this.startTime) / 1000; // Convert to seconds

		this.render();

		this.animationId = requestAnimationFrame(this.animate);
	};

	public render(): void {
		if (this.context.isDisposed()) return;

		this.gl.viewport(
			0,
			0,
			this.context.canvas.width,
			this.context.canvas.height,
		);
		this.gl.clearColor(0.0, 0.0, 0.0, 1.0); // Black background
		this.gl.clear(this.gl.COLOR_BUFFER_BIT);

		this.gl.useProgram(this.program);
		this.renderUniformValue();

		this.gl.bindVertexArray(this.vao);
		this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, 4);
	}

	public getVertexShader(): string {
		return `#version 300 es
                in vec4 a_position;
                void main() {
                    gl_Position = a_position;
                }`;
	}

	public getFragmentShader(): string {
		return `#version 300 es
                precision highp float;
                uniform vec2 u_resolution;
                uniform float u_time;
                out vec4 outColor;

                // Directly defined constants
                const float LOADER_SIZE = 0.04;
                const float LOADER_THICKNESS = 0.2;
                const vec3 LOADER_COLOR = vec3(0.8, 0.8, 0.8);
                const float ROTATION_SPEED = 5.0;

                void main() {
                    vec2 center = vec2(0.5, 0.5);
                    vec2 aspect = vec2(u_resolution.x / u_resolution.y, 1.0);
                    vec2 position = (gl_FragCoord.xy / u_resolution - center) * aspect;
                    float radius = length(position);
                    float angle = atan(position.y, position.x) + u_time * ROTATION_SPEED;

                    float outerRadius = LOADER_SIZE;
                    float innerRadius = outerRadius - LOADER_SIZE * LOADER_THICKNESS;

                    if (radius > outerRadius || radius < innerRadius) {
                        discard;
                    }

                    float segmentAngle = 3.14159 * 0.5; // Quarter circle

                    if (mod(angle, 6.28318) > segmentAngle) {
                        discard;
                    }

                    outColor = vec4(LOADER_COLOR, 1.0);
                }`;
	}
}

SlideShow.CanvasLoaderGl = CanvasLoaderGl;
SlideShow.CanvasLoader2d = CanvasLoader2d;
