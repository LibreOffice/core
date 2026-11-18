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
 * Airplane transition: the leaving slide is folded into a paper dart in the lower left corner and
 * then flies away to the top right, uncovering the static entering slide.
 */

declare var SlideShow: any;

// Pieces of the folded leaving slide, in slide coordinates.
enum AirplanePiece {
	Keel,
	Wing,
	Flap,
}

// A state of the whole dart: where the center of the central triangle is on the screen (0..1), its
// depth and its orientation in degrees.
interface AirplaneKeyframe {
	time: number;
	screenX: number;
	screenY: number;
	depth: number;
	yaw: number;
	pitch: number;
	roll: number;
	smooth: boolean;
}

class AirplaneTransitionImp extends SimpleTransition {
	// Distance of the eye from the slide, in units of half slide height.
	private static readonly eyeDistance = 4.0;
	// Bottom of the keel creases, relative to the bottom center, in slide widths.
	private static readonly keelWidth = 0.14;
	private static readonly apex: vec2 = [0.5, 0.0];

	// Fold (0..0.32), glide (0.32..0.8), launch and exit (0.8..0.935), then nothing is visible.
	// prettier-ignore
	private static readonly keyframes: AirplaneKeyframe[] = [
		{ time: 0.0, screenX: 0.5, screenY: 0.667, depth: 0.0, yaw: 0, pitch: 0, roll: 0, smooth: true },
		{ time: 0.12, screenX: 0.5, screenY: 0.667, depth: 0.0, yaw: 0, pitch: 0, roll: 0, smooth: true },
		{ time: 0.24, screenX: 0.38, screenY: 0.74, depth: 0.0, yaw: -30, pitch: -15, roll: 0, smooth: true },
		{ time: 0.34, screenX: 0.22, screenY: 0.84, depth: 0.3, yaw: -58, pitch: -40, roll: 0, smooth: false },
		{ time: 0.8, screenX: 0.22, screenY: 0.82, depth: -0.1, yaw: -60, pitch: -40, roll: 0, smooth: false },
		{ time: 0.84, screenX: 0.32, screenY: 0.64, depth: -0.6, yaw: -48, pitch: -40, roll: -25, smooth: false },
		{ time: 0.88, screenX: 0.45, screenY: 0.48, depth: -1.5, yaw: -35, pitch: -45, roll: -45, smooth: false },
		{ time: 0.91, screenX: 0.56, screenY: 0.36, depth: -2.5, yaw: -30, pitch: -45, roll: -39, smooth: false },
		{ time: 0.935, screenX: 0.84, screenY: -0.15, depth: -4.0, yaw: -30, pitch: -45, roll: -39, smooth: false },
		{ time: 1.0, screenX: 0.95, screenY: -0.6, depth: -5.0, yaw: -30, pitch: -45, roll: -39, smooth: false },
	];

	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public override initWebglFlags(): void {
		if (this.context.isDisposed()) return;

		this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.depthFunc(this.gl.LEQUAL);
		this.gl.disable(this.gl.CULL_FACE);
		this.gl.disable(this.gl.BLEND);
	}

	protected getVertexShader(): string {
		return `#version 300 es
				precision mediump float;

				in vec3 a_position;
				in vec3 a_normal;
				in vec2 a_texCoord;

				uniform float u_aspect;
				uniform float u_eyeDistance;

				out vec2 v_texturePosition;
				out vec3 v_normal;

				void main(void) {
					// Perspective projection, where the z = 0 plane fills the viewport. The
					// x axis is in units of half slide height, so rotations keep the shape.
					float w = (u_eyeDistance - a_position.z) / u_eyeDistance;
					float depth = clamp(-a_position.z / 20.0, -1.0, 1.0);
					gl_Position = vec4(a_position.x / u_aspect, a_position.y, depth * w, w);
					v_texturePosition = a_texCoord;
					v_normal = a_normal;
				}
				`;
	}

	protected getFragmentShader(): string {
		return `#version 300 es
				precision mediump float;

				uniform sampler2D slideTexture;
				uniform bool u_lit;

				in vec2 v_texturePosition;
				in vec3 v_normal;

				out vec4 outColor;

				void main() {
					vec4 fragment = texture(slideTexture, v_texturePosition);
					if (u_lit) {
						const float ambient = 0.55;
						vec3 lightVector = normalize(vec3(-0.3, 0.5, 1.0));
						float light = ambient + (1.0 - ambient) * max(dot(lightVector, normalize(v_normal)), 0.0);
						fragment.rgb *= light;
					}
					outColor = vec4(fragment.rgb, 1.0);
				}
				`;
	}

	private getAspect(): number {
		return this.context.canvas.width / this.context.canvas.height;
	}

	// Slide coordinates to world coordinates of the flat slide.
	private toWorld(slidePos: vec2): vec3 {
		return [(2 * slidePos[0] - 1) * this.getAspect(), 1 - 2 * slidePos[1], 0.0];
	}

	private static smoothStep(x: number): number {
		x = Math.max(0.0, Math.min(1.0, x));
		return x * x * (3 - 2 * x);
	}

	private static phase(t: number, start: number, end: number): number {
		return AirplaneTransitionImp.smoothStep((t - start) / (end - start));
	}

	// Rotation around the line going through a and b.
	private static rotateAroundLine(a: vec3, b: vec3, angle: number): any {
		const matrix = mat4.create();
		mat4.translate(matrix, matrix, a);
		mat4.rotate(matrix, matrix, angle, [b[0] - a[0], b[1] - a[1], b[2] - a[2]]);
		mat4.translate(matrix, matrix, [-a[0], -a[1], -a[2]]);
		return matrix;
	}

	// The triangles of one side (-1: left, 1: right) of the slide, in slide coordinates.
	private static getPieceTriangle(piece: AirplanePiece, side: number): vec2[] {
		const apex = AirplaneTransitionImp.apex;
		const keelBottom: vec2 = [
			0.5 + side * AirplaneTransitionImp.keelWidth,
			1.0,
		];
		const bottomCorner: vec2 = [0.5 + side * 0.5, 1.0];
		const topCorner: vec2 = [0.5 + side * 0.5, 0.0];
		switch (piece) {
			case AirplanePiece.Keel:
				return [apex, keelBottom, [0.5, 1.0]];
			case AirplanePiece.Wing:
				return [apex, bottomCorner, keelBottom];
			case AirplanePiece.Flap:
				return [apex, topCorner, bottomCorner];
		}
	}

	// Transformation of a piece of the slide, relative to the dart; null if it's not visible.
	private getPieceMatrix(t: number, piece: AirplanePiece, side: number): any {
		const apex = this.toWorld(AirplaneTransitionImp.apex);
		const fold = AirplaneTransitionImp.phase(t, 0.18, 0.4);
		const keelMatrix = AirplaneTransitionImp.rotateAroundLine(
			apex,
			this.toWorld([0.5, 1.0]),
			side * OpsHelper.deg2rad(88) * fold,
		);
		if (piece === AirplanePiece.Keel) return keelMatrix;

		if (piece === AirplanePiece.Wing) {
			const keelBottom = this.toWorld([
				0.5 + side * AirplaneTransitionImp.keelWidth,
				1.0,
			]);
			const wingMatrix = AirplaneTransitionImp.rotateAroundLine(
				apex,
				keelBottom,
				-side * OpsHelper.deg2rad(75) * fold,
			);
			return mat4.multiply(mat4.create(), keelMatrix, wingMatrix);
		}

		const flap = AirplaneTransitionImp.phase(t, 0.0, 0.26);
		if (flap >= 1.0) return null;
		// The flap moves away from its crease, outwards, while it swings towards the viewer.
		const corner = this.toWorld([0.5 + side * 0.5, 1.0]);
		const crease = [corner[0] - apex[0], corner[1] - apex[1]];
		const length = Math.hypot(crease[0], crease[1]);
		let normal = [crease[1] / length, -crease[0] / length];
		if (normal[0] * side < 0) normal = [-normal[0], -normal[1]];
		const outwards = 1.5 * flap * flap;
		const matrix = mat4.create();
		mat4.translate(matrix, matrix, [
			normal[0] * outwards,
			normal[1] * outwards,
			1.5 * flap,
		]);
		return mat4.multiply(
			matrix,
			matrix,
			AirplaneTransitionImp.rotateAroundLine(
				apex,
				corner,
				-side * OpsHelper.deg2rad(70) * flap,
			),
		);
	}

	private static getKeyframe(t: number): AirplaneKeyframe {
		const keyframes = AirplaneTransitionImp.keyframes;
		for (let i = 0; i + 1 < keyframes.length; ++i) {
			const from = keyframes[i];
			const to = keyframes[i + 1];
			if (t > to.time) continue;

			let x = (t - from.time) / (to.time - from.time);
			if (from.smooth) x = AirplaneTransitionImp.smoothStep(x);
			const mix = (a: number, b: number) => a + (b - a) * x;
			return {
				time: t,
				screenX: mix(from.screenX, to.screenX),
				screenY: mix(from.screenY, to.screenY),
				depth: mix(from.depth, to.depth),
				yaw: mix(from.yaw, to.yaw),
				pitch: mix(from.pitch, to.pitch),
				roll: mix(from.roll, to.roll),
				smooth: from.smooth,
			};
		}
		return keyframes[keyframes.length - 1];
	}

	// Transformation of the whole dart: puts the center of its central triangle at the position
	// of the current keyframe, with the orientation of the current keyframe.
	private getDartMatrix(t: number): any {
		const keyframe = AirplaneTransitionImp.getKeyframe(t);
		const eyeDistance = AirplaneTransitionImp.eyeDistance;
		const perspective = (eyeDistance - keyframe.depth) / eyeDistance;
		const matrix = mat4.create();
		mat4.translate(matrix, matrix, [
			(2 * keyframe.screenX - 1) * this.getAspect() * perspective,
			(1 - 2 * keyframe.screenY) * perspective,
			keyframe.depth,
		]);
		mat4.rotateZ(matrix, matrix, OpsHelper.deg2rad(keyframe.yaw));
		mat4.rotateX(matrix, matrix, OpsHelper.deg2rad(keyframe.pitch));
		mat4.rotateY(matrix, matrix, OpsHelper.deg2rad(keyframe.roll));
		mat4.translate(matrix, matrix, [0, 1 / 3, 0]);
		return matrix;
	}

	private getDartVertices(t: number): Vertex[] {
		const vertices: Vertex[] = [];
		const dartMatrix = this.getDartMatrix(t);
		const eye = [0, 0, AirplaneTransitionImp.eyeDistance];
		for (const piece of [
			AirplanePiece.Keel,
			AirplanePiece.Wing,
			AirplanePiece.Flap,
		]) {
			for (const side of [-1, 1]) {
				const pieceMatrix = this.getPieceMatrix(t, piece, side);
				if (!pieceMatrix) continue;

				const matrix = mat4.multiply(mat4.create(), dartMatrix, pieceMatrix);
				const triangle = AirplaneTransitionImp.getPieceTriangle(piece, side);
				const positions: vec3[] = triangle.map((slidePos: vec2) => {
					const position = vec3.create();
					vec3.transformMat4(position, this.toWorld(slidePos), matrix);
					return position;
				});

				// Flat shading; both sides of the paper show the slide, so the normal
				// always faces the viewer.
				const normal = vec3.create();
				vec3.cross(
					normal,
					vec3.sub(vec3.create(), positions[1], positions[0]),
					vec3.sub(vec3.create(), positions[2], positions[0]),
				);
				vec3.normalize(normal, normal);
				const toEye = vec3.sub(vec3.create(), eye, positions[0]);
				if (vec3.dot(normal, toEye) < 0) vec3.negate(normal, normal);

				for (let i = 0; i < 3; ++i) {
					vertices.push({
						position: positions[i],
						normal: normal,
						texCoord: triangle[i],
					});
				}
			}
		}
		return vertices;
	}

	private getEnteringVertices(): Vertex[] {
		const vertices: Vertex[] = [];
		const corners: vec2[] = [
			[0, 0],
			[1, 0],
			[0, 1],
			[1, 0],
			[1, 1],
			[0, 1],
		];
		for (const corner of corners) {
			vertices.push({
				position: this.toWorld(corner),
				normal: [0, 0, 1],
				texCoord: corner,
			});
		}
		return vertices;
	}

	private drawTriangles(vertices: Vertex[], textureNum: number): void {
		this.gl.activeTexture(this.gl.TEXTURE0);
		this.gl.bindTexture(this.gl.TEXTURE_2D, this.textures[textureNum]);
		this.gl.uniform1i(
			this.gl.getUniformLocation(this.program, 'slideTexture'),
			0,
		);
		this.setBufferData(vertices);
		this.gl.drawArrays(this.gl.TRIANGLES, 0, vertices.length);
	}

	// Draws one frame, called from render() on each requestAnimationFrame() tick. t is 0..1
	public override displaySlides_(t: number): void {
		this.gl.uniform1f(
			this.gl.getUniformLocation(this.program, 'u_aspect'),
			this.getAspect(),
		);
		this.gl.uniform1f(
			this.gl.getUniformLocation(this.program, 'u_eyeDistance'),
			AirplaneTransitionImp.eyeDistance,
		);

		// The entering slide is texture 1, in the background:
		const litLocation = this.gl.getUniformLocation(this.program, 'u_lit');
		this.gl.depthMask(false);
		this.gl.uniform1i(litLocation, 0);
		this.drawTriangles(this.getEnteringVertices(), 1);

		// The leaving slide / dart is texture 0, always in front:
		this.gl.depthMask(true);
		this.gl.uniform1i(litLocation, 1);
		this.drawTriangles(this.getDartVertices(t), 0);
	}
}

function AirplaneTransition(transitionParameters: TransitionParameters) {
	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: [],
		enteringPrimitives: [],
		allOperations: [],
	};

	return new AirplaneTransitionImp(newTransitionParameters);
}

SlideShow.AirplaneTransition = AirplaneTransition;
