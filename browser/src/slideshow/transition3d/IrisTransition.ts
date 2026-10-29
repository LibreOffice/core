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
 * Iris transition: seven gray blades close over the leaving slide like a camera iris and open
 * again on the entering slide.
 */

declare var SlideShow: any;

class IrisTransitionImp extends SimpleTransition {
	private blades: Primitive[];
	private bladeTexture: WebGLTexture;

	constructor(
		transitionParameters: TransitionParameters3D,
		blades: Primitive[],
	) {
		super(transitionParameters);
		this.blades = blades;
		this.bladeTexture = this.createBladeTexture();
	}

	private createBladeTexture(): WebGLTexture {
		const gl = this.gl;
		const texture = gl.createTexture();
		if (!texture) throw new Error('Failed to create the iris blade texture');
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(
			gl.TEXTURE_2D,
			0,
			gl.RGBA,
			1,
			1,
			0,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			new Uint8Array([80, 80, 80, 255]),
		);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
		return texture;
	}

	// The blades are circular on the screen, so the scene is stretched along the shorter side of
	// the canvas to undo the slide's aspect ratio.
	private bladeSceneMatrix(): mat4 {
		const width = this.context.canvas.width;
		const height = this.context.canvas.height;
		const matrix = mat4.create();
		if (height > width)
			mat4.scale(matrix, matrix, vec3.fromValues(height / width, 1, 1));
		else mat4.scale(matrix, matrix, vec3.fromValues(1, width / height, 1));
		return matrix;
	}

	public displaySlides_(t: number): void {
		this.applyAllOperation(t);

		// The blades are fully closed half way, which is when the slides swap.
		if (t < 0.5)
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				0,
				this.leavingPrimitives,
				'slideTexture',
			);
		else
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				1,
				this.enteringPrimitives,
				'slideTexture',
			);

		const sceneLocation = this.gl.getUniformLocation(
			this.program,
			'u_sceneTransformMatrix',
		);
		this.gl.uniformMatrix4fv(sceneLocation, false, this.bladeSceneMatrix());

		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.bladeTexture);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'slideTexture'),
			0,
		);
		for (const blade of this.blades) {
			this.setBufferData(blade.vertices);
			this.applyLeavingOperations(t, blade.operations);
			this.gl.drawArrays(this.gl.TRIANGLES, 0, blade.vertices.length);
		}

		this.gl.uniformMatrix4fv(sceneLocation, false, mat4.create());
	}
}

function IrisTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();
	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [Primitive.cloneDeep(slide)];
	const aEnteringPrimitives: Primitive[] = [Primitive.cloneDeep(slide)];

	// One blade is a band of an arc, wider than the slide so that the closed iris covers it.
	const nSteps = 24;
	const nParts = 7;
	const innerFactor = 1.42;
	const outerFactor = 2.2;
	const bladeShape = new Primitive();
	let lastX = 1;
	let lastY = 0;
	for (let step = 1; step <= nSteps; step++) {
		const angle = (3 * 2 * Math.PI * step) / nSteps / nParts;
		const x = Math.cos(angle);
		const y = -Math.sin(angle);
		const inner: vec2 = [(innerFactor * x + 1) / 2, (innerFactor * y + 1) / 2];
		const lastInner: vec2 = [
			(innerFactor * lastX + 1) / 2,
			(innerFactor * lastY + 1) / 2,
		];
		const outer: vec2 = [(outerFactor * x + 1) / 2, (outerFactor * y + 1) / 2];
		const lastOuter: vec2 = [
			(outerFactor * lastX + 1) / 2,
			(outerFactor * lastY + 1) / 2,
		];
		bladeShape.pushTriangle(lastInner, lastOuter, inner);
		bladeShape.pushTriangle(inner, lastOuter, outer);
		lastX = x;
		lastY = y;
	}

	// Each blade pivots on its own point of the unit circle. It swings in during the first half and
	// back out during the second half.
	const blades: Primitive[] = [];
	const swingAngle = 87;
	const zAxis = vec3.fromValues(0, 0, 1);
	for (let i = 0; i < nParts; i++) {
		const blade = Primitive.cloneDeep(bladeShape);
		const pivotX = Math.cos((2 * Math.PI * i) / nParts);
		const pivotY = Math.sin((2 * Math.PI * i) / nParts);
		const pivot = vec3.fromValues(pivotX, pivotY, 0);
		blade.operations.push(
			makeSRotate(zAxis, pivot, swingAngle, true, 0.0, 0.5),
		);
		blade.operations.push(
			makeSRotate(zAxis, pivot, -swingAngle, true, 0.5, 1.0),
		);
		if (i > 0) {
			blade.operations.push(makeSTranslate(pivot, false, -1, 0));
			blade.operations.push(
				makeSRotate(
					zAxis,
					vec3.fromValues(0, 0, 0),
					(i * 360.0) / nParts,
					false,
					-1,
					0,
				),
			);
			blade.operations.push(
				makeSTranslate(vec3.fromValues(-1, 0, 0), false, -1, 0),
			);
		}
		blade.operations.push(
			makeSTranslate(vec3.fromValues(0, 0, 1), false, -2, 0.0),
		);
		blade.operations.push(
			makeSRotate(
				vec3.fromValues(1, 0.5, 0),
				vec3.fromValues(1, 0, 0),
				-30,
				false,
				-1,
				0,
			),
		);
		blades.push(blade);
	}

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new IrisTransitionImp(newTransitionParameters, blades);
}

SlideShow.IrisTransition = IrisTransition;
