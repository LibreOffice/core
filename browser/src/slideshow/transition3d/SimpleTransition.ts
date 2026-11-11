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

class SimpleTransition extends SlideShow.Transition3d {
	public leavingPrimitives: Primitive[] = [];
	public enteringPrimitives: Primitive[] = [];
	public allOperations: Operation[] = [];
	public buffer: any;
	public textures: any[] = [];

	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
		this.leavingPrimitives = transitionParameters.leavingPrimitives;
		this.enteringPrimitives = transitionParameters.enteringPrimitives;
		this.allOperations = transitionParameters.allOperations;

		this.textures = [transitionParameters.current, transitionParameters.next];

		this.initWebglFlags();
		this.prepareTransition();
	}

	public initWebglFlags(): void {
		if (this.context.isDisposed()) return;

		this.gl.disable(this.gl.DEPTH_TEST);
		this.gl.disable(this.gl.CULL_FACE);

		// Enable alpha blending
		this.gl.enable(this.gl.BLEND);
		this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
	}

	public initBuffers(): void {
		this.vao = this.gl.createVertexArray();
		this.gl.bindVertexArray(this.vao);

		this.buffer = this.gl.createBuffer();
		if (!this.buffer) {
			throw new Error('Failed to create buffer');
		}
		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffer);

		// prettier-ignore
		const initialPositions = new Float32Array([
            -1, -1, 0, 0, 1,
             1, -1, 0, 1, 1,
            -1,  1, 0, 0, 0,
             1,  1, 0, 1, 0,
        ]);

		this.gl.bufferData(
			this.gl.ARRAY_BUFFER,
			initialPositions,
			this.gl.STATIC_DRAW,
		);

		const positionLocation = this.gl.getAttribLocation(
			this.program,
			'a_position',
		);
		const normalLocation = this.gl.getAttribLocation(this.program, 'a_normal');
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
				8 * 4,
				0,
			);
		}
		if (normalLocation !== -1) {
			this.gl.enableVertexAttribArray(normalLocation);
			this.gl.vertexAttribPointer(
				normalLocation,
				3,
				this.gl.FLOAT,
				false,
				8 * 4,
				3 * 4,
			);
		}
		if (texCoordLocation !== -1) {
			this.gl.enableVertexAttribArray(texCoordLocation);
			this.gl.vertexAttribPointer(
				texCoordLocation,
				2,
				this.gl.FLOAT,
				false,
				8 * 4,
				6 * 4,
			);
		}

		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, null);
		this.gl.bindVertexArray(null);

		console.debug('Simple Transition buffer initialized.');
	}

	public applyAllOperation(t: number): void {
		let matrix = mat4.create();

		for (const operation of this.allOperations) {
			matrix = operation.interpolate(matrix, t, 1.0, 1.0);
		}
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_operationsTransformMatrix'),
			false,
			matrix,
		);
	}

	public applyLeavingOperations(t: number, operations: Operation[]): void {
		let matrix = mat4.create();
		for (const operation of operations) {
			matrix = operation.interpolate(matrix, t, 1.0, 1.0);
		}
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_primitiveTransformMatrix'),
			false,
			matrix,
		);
	}

	public applyEnteringOperations(t: number, operations: Operation[]): void {
		let matrix = mat4.create();
		for (const operation of operations) {
			matrix = operation.interpolate(matrix, t, 1.0, 1.0);
		}
		this.gl.uniformMatrix4fv(
			this.gl.getUniformLocation(this.program, 'u_primitiveTransformMatrix'),
			false,
			matrix,
		);
	}

	public displayPrimitive(
		t: number,
		textureType: any,
		textureNum: number,
		texturePrimitive: Primitive[],
		textureName: string,
	): void {
		this.gl.activeTexture(textureType);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[textureNum]);
		this.gl.uniform1i(this.gl.getUniformLocation(this.program, textureName), 0);
		for (const primitive of texturePrimitive) {
			this.setBufferData(primitive.vertices);
			this.applyLeavingOperations(t, primitive.operations);
			this.gl.drawArrays(this.gl.TRIANGLE_STRIP, 0, primitive.vertices.length);
		}
	}

	public displaySlides_(t: number): void {
		this.applyAllOperation(t);
		this.displayPrimitive(
			t,
			this.gl.TEXTURE0,
			0,
			this.leavingPrimitives,
			'slideTexture',
		);
		this.displayPrimitive(
			t,
			this.gl.TEXTURE0,
			1,
			this.enteringPrimitives,
			'slideTexture',
		);
	}

	public render(nT: number): void {
		if (this.context.isDisposed()) return;

		// The slide textures are drawn anew in the same context between two frames, with depth
		// testing, face culling and blending set for drawing them, so each frame sets the flags
		// of the transition again.
		this.initWebglFlags();
		this.gl.viewport(
			0,
			0,
			this.context.canvas.width,
			this.context.canvas.height,
		);
		this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
		this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);

		this.gl.useProgram(this.program);
		this.gl.uniform1f(this.gl.getUniformLocation(this.program, 'time'), nT);

		this.gl.bindVertexArray(this.vao);

		this.displaySlides_(nT);

		this.gl.bindVertexArray(null);

		app.map.fire('newslideshowframe', {
			frame: this.gl.canvas,
		});
	}

	public setBufferData(vertices: Vertex[]): void {
		const positionData: number[] = [];
		for (const vertex of vertices) {
			positionData.push(
				...vertex.position,
				...vertex.normal,
				...vertex.texCoord,
			);
		}
		const positions = new Float32Array(positionData);

		this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffer);
		this.gl.bufferData(this.gl.ARRAY_BUFFER, positions, this.gl.STATIC_DRAW);
	}
}

SlideShow.SimpleTransition = SimpleTransition;
