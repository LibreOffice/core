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

declare var SlideShow: any;

class FlipTilesTransition extends SimpleTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public initWebglFlags(): void {
		this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.depthFunc(this.gl.LEQUAL);

		// Enable face culling
		this.gl.enable(this.gl.CULL_FACE);
		this.gl.cullFace(this.gl.BACK);

		// Disable blending
		this.gl.disable(this.gl.BLEND);
	}

	public processPrimitives(
		t: number,
		texturePrimitive: Primitive[],
		applyOperations: (t: number, operations: Operation[]) => void,
	): void {
		for (const primitive of texturePrimitive) {
			this.setBufferData(primitive.vertices);
			applyOperations.call(this, t, primitive.operations);
			this.gl.drawArrays(this.gl.TRIANGLES, 0, primitive.vertices.length);
		}
	}

	public displaySlides_(t: number): void {
		this.applyAllOperation(t);

		// Render the entering slide primitives
		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[1]);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'slideTexture'),
			0,
		);
		this.processPrimitives(
			t,
			this.enteringPrimitives,
			this.applyEnteringOperations,
		);

		// Render the leaving slide primitives
		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[0]);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'slideTexture'),
			0,
		);
		this.processPrimitives(
			t,
			this.leavingPrimitives,
			this.applyLeavingOperations,
		);
	}
}

function vec(x: number, y: number, nx: number, ny: number): vec2 {
	x = x < 0.0 ? 0.0 : x;
	x = Math.min(x, nx);
	y = y < 0.0 ? 0.0 : y;
	y = Math.min(y, ny);
	return [x / nx, y / ny];
}

function calculateMidpoint(vec1: vec3, vec2: vec3): vec3 {
	return [
		(vec1[0] + vec2[0]) / 2.0, // X coordinate
		(vec1[1] + vec2[1]) / 2.0, // Y coordinate
		(vec1[2] + vec2[2]) / 2.0, // Z coordinate
	];
}

function makeFlipTilesTransition(
	transitionParameters: TransitionParameters,
	n: number = 8,
	m: number = 6,
) {
	const aLeavingPrimitives: Primitive[] = [];
	const aEnteringPrimitives: Primitive[] = [];

	for (let x = 0; x < n; x++) {
		for (let y = 0; y < n; y++) {
			const aTile = new Primitive();
			const x11 = vec(x, y, n, m);
			const x12 = vec(x, y + 1, n, m);
			const x21 = vec(x + 1, y, n, m);
			const x22 = vec(x + 1, y + 1, n, m);

			aTile.pushTriangle(x21, x11, x12);
			aTile.pushTriangle(x22, x21, x12);

			aTile.operations.push(
				makeSRotate(
					vec3.fromValues(0, 1, 0),
					calculateMidpoint(aTile.getVertex(1), aTile.getVertex(3)),
					180,
					true,
					(x11[0] * x11[1]) / 2.0,
					(x22[0] * x22[1] + 1.0) / 2.0,
				),
			);
			aLeavingPrimitives.push(Primitive.cloneDeep(aTile));

			aTile.operations.push(
				makeSRotate(
					vec3.fromValues(0, 1, 0),
					calculateMidpoint(aTile.getVertex(1), aTile.getVertex(3)),
					-180,
					false,
					(x11[0] * x11[1]) / 2.0,
					(x22[0] * x22[1] + 1.0) / 2.0,
				),
			);
			aEnteringPrimitives.push(aTile);
		}
	}

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new FlipTilesTransition(newTransitionParameters);
}

SlideShow.makeFlipTilesTransition = makeFlipTilesTransition;
