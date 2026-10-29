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
 * Glitter transition: the slide is tiled with small hexagons. Each one spins around its vertical
 * axis at its own time, sweeping from left to right, and comes back showing the entering slide.
 */

declare var SlideShow: any;

class GlitterTransitionImp extends PermTextureTransition {
	private static readonly columns = 80;
	private static readonly rows = Math.floor(
		(GlitterTransitionImp.columns * 4) / 3,
	);
	// Floats per vertex: position, normal, texture coordinate and hexagon center.
	private static readonly stride = 3 + 3 + 2 + 3;

	// Set once the mesh is uploaded, which happens inside the base class constructor.
	private meshVertexCount!: number;

	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public initWebglFlags(): void {
		if (this.context.isDisposed()) return;

		// A spinning hexagon overlaps its neighbors, so the depth test sorts them. Its back is lit
		// from behind and comes out black, which is the glitter.
		this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.depthFunc(this.gl.LEQUAL);
		this.gl.disable(this.gl.CULL_FACE);
		this.gl.disable(this.gl.BLEND);
	}

	public initBuffers(): void {
		if (this.meshVertexCount) return;

		this.meshVertexCount = this.uploadMesh(
			GlitterTransitionImp.buildMesh(),
			GlitterTransitionImp.stride,
			[
				['a_position', 3, 0],
				['a_normal', 3, 3],
				['a_texCoord', 2, 6],
				['a_center', 3, 8],
			],
		);
	}

	// The hexagons sit on a grid of columns by rows cells, two cells wide and four cells tall with
	// every second row shifted by one cell. Each one is a fan of six triangles around its center.
	private static buildMesh(): Float32Array {
		const columns = GlitterTransitionImp.columns;
		const rows = GlitterTransitionImp.rows;
		const data: number[] = [];

		const cell = (x: number, y: number): vec2 => {
			return [
				Math.min(Math.max(x, 0), columns) / columns,
				Math.min(Math.max(y, 0), rows) / rows,
			];
		};
		const toModel = (point: vec2): vec2 => {
			return [2 * point[0] - 1, -2 * point[1] + 1];
		};

		for (let y = 0; y < rows + 2; y += 2) {
			for (let x = 0; x < columns + 2; x += 2) {
				const corners: vec2[] =
					y % 4 === 0
						? [
								cell(x - 1, y - 1),
								cell(x, y - 2),
								cell(x + 1, y - 1),
								cell(x + 1, y),
								cell(x, y + 1),
								cell(x - 1, y),
							]
						: [
								cell(x - 2, y - 1),
								cell(x - 1, y - 2),
								cell(x, y - 1),
								cell(x, y),
								cell(x - 1, y + 1),
								cell(x - 2, y),
							];
				const center = toModel(cell(x, y + 0.5));
				for (let i = 0; i < 6; i++) {
					const triangle = [
						toModel(corners[i]),
						toModel(corners[(i + 1) % 6]),
						center,
					];
					for (const point of triangle) {
						data.push(point[0], point[1], 0, 0, 0, 1, 0, 0);
						data.push(center[0], center[1], 0);
					}
				}
			}
		}
		return new Float32Array(data);
	}

	protected getVertexShader(): string {
		return SlideshowShaders['GlitterTransition.vert.glsl'];
	}

	protected getFragmentShader(): string {
		return SlideshowShaders['GlitterTransition.frag.glsl'];
	}

	public displaySlides_(t: number): void {
		const gl = this.gl;
		this.applyAllOperation(t);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, this.textures[0]);
		gl.uniform1i(gl.getUniformLocation(this.program, 'leavingSlideTexture'), 0);
		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(gl.TEXTURE_2D, this.textures[1]);
		gl.uniform1i(
			gl.getUniformLocation(this.program, 'enteringSlideTexture'),
			1,
		);
		this.displayPermSlide_();

		gl.drawArrays(gl.TRIANGLES, 0, this.meshVertexCount);
	}
}

function GlitterTransition(transitionParameters: TransitionParameters) {
	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: [],
		enteringPrimitives: [],
		allOperations: [],
	};

	return new GlitterTransitionImp(newTransitionParameters);
}

SlideShow.GlitterTransition = GlitterTransition;
