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
 * Honeycomb transition: both slides are built from hexagonal cells. The leaving slide grows, turns
 * and flies toward the viewer while its cells fade out one by one, and the entering slide comes
 * from far behind, turning into place as its cells light up.
 */

declare var SlideShow: any;

class HoneycombTransitionImp extends PermTextureTransition {
	private static readonly columns = 21;
	private static readonly rows = 21;
	// The gap between the cells, as a fraction of the cell size. The cells are drawn once at
	// 1 - border and once at 1 + border, and the larger pass fills the gaps.
	private static readonly borderSize = 0.15;
	// How far behind a cell its walls reach, in slide units.
	private static readonly wallDepth = 0.3;
	private static readonly wallLight = 0.3;
	// Floats per vertex: cell center with the wall depth as z, corner offset and normal.
	private static readonly stride = 3 + 2 + 3;
	// The six corners of a cell, in units that the shader scales to the cell size.
	private static readonly corners: vec2[] = [
		[-3, -2],
		[0, -4],
		[3, -2],
		[3, 2],
		[0, 4],
		[-3, 2],
	];

	// Set once the mesh is uploaded, which happens inside the base class constructor.
	private meshVertexCount!: number;

	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public initWebglFlags(): void {
		if (this.context.isDisposed()) return;

		// The entering slide shows through the fading cells of the leaving one.
		this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.depthFunc(this.gl.LEQUAL);
		this.gl.disable(this.gl.CULL_FACE);
		this.gl.enable(this.gl.BLEND);
		this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
	}

	public initBuffers(): void {
		if (this.meshVertexCount) return;

		this.meshVertexCount = this.uploadMesh(
			HoneycombTransitionImp.buildMesh(),
			HoneycombTransitionImp.stride,
			[
				['a_center', 3, 0],
				['a_offset', 2, 3],
				['a_normal', 3, 5],
			],
		);
	}

	// A cell is a fan of six triangles around its center plus six walls going back from its edges.
	// The vertex positions are worked out in the shader from the center and the corner offset, so
	// the same mesh serves the cells and the gap filler between them.
	private static buildMesh(): Float32Array {
		const columns = HoneycombTransitionImp.columns;
		const rows = HoneycombTransitionImp.rows;
		const corners = HoneycombTransitionImp.corners;
		const wallDepth = HoneycombTransitionImp.wallDepth;
		const wallLight = HoneycombTransitionImp.wallLight;
		const data: number[] = [];

		const push = (center: vec2, depth: number, offset: vec2, light: number) => {
			data.push(center[0], center[1], depth, offset[0], offset[1], 0, 0, light);
		};

		for (let y = 0; y < rows + 2; y += 2) {
			for (let x = 0; x < columns + 2; x += 2) {
				const slideX = (y % 4 ? x : x + 1) / columns;
				const slideY = y / rows;
				const center: vec2 = [2 * slideX - 1, -2 * slideY + 1];

				for (let i = 0; i < 6; i++) {
					const a = corners[i];
					const b = corners[(i + 1) % 6];

					push(center, 0, a, 1);
					push(center, 0, b, 1);
					push(center, 0, [0, 0], 1);

					push(center, 0, a, wallLight);
					push(center, -wallDepth, a, wallLight);
					push(center, 0, b, wallLight);
					push(center, -wallDepth, a, wallLight);
					push(center, -wallDepth, b, wallLight);
					push(center, 0, b, wallLight);
				}
			}
		}
		return new Float32Array(data);
	}

	protected getVertexShader(): string {
		return SlideshowShaders['HoneycombTransition.vert.glsl'];
	}

	protected getFragmentShader(): string {
		return SlideshowShaders['HoneycombTransition.frag.glsl'];
	}

	public displaySlides_(t: number): void {
		const gl = this.gl;
		this.applyAllOperation(t);

		gl.uniform1f(
			gl.getUniformLocation(this.program, 'slideRatio'),
			this.context.canvas.height / this.context.canvas.width,
		);
		this.displayPermSlide_();

		const selectedTextureLocation = gl.getUniformLocation(
			this.program,
			'selectedTexture',
		);
		const hexagonSizeLocation = gl.getUniformLocation(
			this.program,
			'hexagonSize',
		);
		const borderSize = HoneycombTransitionImp.borderSize;

		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(gl.getUniformLocation(this.program, 'slideTexture'), 0);

		// The entering slide is behind the leaving one, so it is drawn first for the blending to
		// work. Each slide is drawn as its cells and then as the filler between them.
		gl.bindTexture(gl.TEXTURE_2D, this.textures[1]);
		gl.uniform1f(selectedTextureLocation, 0.0);
		gl.uniform1f(hexagonSizeLocation, 1.0 - borderSize);
		gl.drawArrays(gl.TRIANGLES, 0, this.meshVertexCount);
		gl.uniform1f(hexagonSizeLocation, 1.0 + borderSize);
		gl.drawArrays(gl.TRIANGLES, 0, this.meshVertexCount);

		gl.bindTexture(gl.TEXTURE_2D, this.textures[0]);
		gl.uniform1f(selectedTextureLocation, 1.0);
		gl.uniform1f(hexagonSizeLocation, 1.0 - borderSize);
		gl.drawArrays(gl.TRIANGLES, 0, this.meshVertexCount);
		gl.uniform1f(hexagonSizeLocation, 1.0 + borderSize);
		gl.drawArrays(gl.TRIANGLES, 0, this.meshVertexCount);
	}
}

function HoneycombTransition(transitionParameters: TransitionParameters) {
	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: [],
		enteringPrimitives: [],
		allOperations: [],
	};

	return new HoneycombTransitionImp(newTransitionParameters);
}

SlideShow.HoneycombTransition = HoneycombTransition;
