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

class PermTextureTransition extends SimpleTransition {
	public permTexture: any;
	public m_nHelperTexture: WebGLTexture | undefined;

	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
		this.permTexture = this.createPermTexture();
	}

	public createPermTexture(): WebGLTexture {
		const gl = this.gl;
		const permTex = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, permTex);

		gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA8, 256, 256);

		const data = this.generatePermutationData();
		gl.texSubImage2D(
			gl.TEXTURE_2D,
			0,
			0,
			0,
			256,
			256,
			gl.RGBA,
			gl.UNSIGNED_BYTE,
			data,
		);

		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

		if (!permTex) {
			throw new Error('Invalid extra texture object');
		}
		return permTex;
	}

	// prettier-ignore
	private permutation256: number[] = [
        215, 100, 200, 204, 233,  50,  85, 196,
         71, 141, 122, 160,  93, 131, 243, 234,
        162, 183,  36, 155,   4,  62,  35, 205,
         40, 102,  33,  27, 255,  55, 214, 156,
         75, 163, 134, 126, 249,  74, 197, 228,
         72,  90, 206, 235,  17,  22,  49, 169,
        227,  89,  16,   5, 117,  60, 248, 230,
        217,  68, 138,  96, 194, 170, 136,  10,
        112, 238, 184, 189, 176,  42, 225, 212,
         84,  58, 175, 244, 150, 168, 219, 236,
        101, 208, 123,  37, 164, 110, 158, 201,
         78, 114,  57,  48,  70, 142, 106,  43,
        232,  26,  32, 252, 239,  98, 191,  94,
         59, 149,  39, 187, 203, 190,  19,  13,
        133,  45,  61, 247,  23,  34,  20,  52,
        118, 209, 146, 193, 222,  18,   1, 152,
         46,  41,  91, 148, 115,  25, 135,  77,
        254, 147, 224, 161,   9, 213, 223, 250,
        231, 251, 127, 166,  63, 179,  81, 130,
        139,  28, 120, 151, 241,  86, 111,   0,
         88, 153, 172, 182, 159, 105, 178,  47,
         51, 167,  65,  66,  92,  73, 198, 211,
        245, 195,  31, 220, 140,  76, 221, 186,
        154, 185,  56,  83,  38, 165, 109,  67,
        124, 226, 132,  53, 229,  29,  12, 181,
        121,  24, 207, 199, 177, 113,  30,  80,
          3,  97, 188,  79, 216, 173,   8, 145,
         87, 128, 180, 237, 240, 137, 125, 104,
         15, 242, 119, 246, 103, 143,  95, 144,
          2,  44,  69, 157, 192, 174,  14,  54,
        218,  82,  64, 210,  11,   6, 129,  21,
        116, 171,  99, 202,   7, 107, 253, 108
    ];

	public generatePermutationData(): Uint8Array {
		const a = new Uint8Array(256 * 256 * 4); // Multiplied by 4 for RGBA
		for (let y = 0; y < 256; y++) {
			for (let x = 0; x < 256; x++) {
				const base = (x + y * 256) * 4;
				const value = this.permutation256[(y + this.permutation256[x]) % 256];
				a[base] = value; // R
				a[base + 1] = value; // G (could be different for variety)
				a[base + 2] = value; // B (could be different for variety)
				a[base + 3] = 255; // A (alpha channel)
			}
		}
		return a;
	}

	// Uploads a prebuilt mesh of interleaved float vertices into a fresh vertex array and buffer,
	// and points the given attributes at it. Each attribute is its shader name, its size in floats
	// and its offset in floats within a vertex. Returns the number of vertices in the mesh.
	protected uploadMesh(
		mesh: Float32Array,
		stride: number,
		attributes: [string, number, number][],
	): number {
		const gl = this.gl;
		this.vao = gl.createVertexArray();
		gl.bindVertexArray(this.vao);

		this.buffer = gl.createBuffer();
		if (!this.buffer) throw new Error('Failed to create buffer');
		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.bufferData(gl.ARRAY_BUFFER, mesh, gl.STATIC_DRAW);

		const strideBytes = stride * 4;
		for (const [name, size, offset] of attributes) {
			const location = gl.getAttribLocation(this.program, name);
			if (location === -1) continue;
			gl.enableVertexAttribArray(location);
			gl.vertexAttribPointer(
				location,
				size,
				gl.FLOAT,
				false,
				strideBytes,
				offset * 4,
			);
		}

		gl.bindBuffer(gl.ARRAY_BUFFER, null);
		gl.bindVertexArray(null);
		return mesh.length / stride;
	}

	public displayPermSlide_(): void {
		this.gl.activeTexture(this.gl.TEXTURE2);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.permTexture);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'permTexture'),
			2,
		);
	}

	public displaySlides_(t: number): void {
		this.gl.uniform1f(this.gl.getUniformLocation(this.program, 'time'), t);

		this.setBufferData(this.leavingPrimitives[0].vertices);
		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[0]);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'leavingSlideTexture'),
			0,
		);

		this.gl.activeTexture(this.gl.TEXTURE1);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[1]);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'enteringSlideTexture'),
			1,
		);
		this.gl.drawArrays(
			this.gl.TRIANGLE_STRIP,
			0,
			this.leavingPrimitives[0].vertices.length,
		);

		this.displayPermSlide_();
	}
}

SlideShow.PermTextureTransition = PermTextureTransition;
