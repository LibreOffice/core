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
 * Revolving circles transition: the slide is cut into a central disk and concentric rings. Each
 * ring turns over around its own random axis in the slide plane, the inner ones first, and shows
 * the entering slide on its back.
 */

declare var SlideShow: any;

class RevolvingCirclesTransitionImp extends SimpleTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public initWebglFlags(): void {
		if (this.context.isDisposed()) return;

		// A ring shows its front while it leaves and its back once it has turned over, so culling
		// the back faces picks the visible slide for every ring.
		this.gl.enable(this.gl.DEPTH_TEST);
		this.gl.depthFunc(this.gl.LEQUAL);
		this.gl.enable(this.gl.CULL_FACE);
		this.gl.cullFace(this.gl.BACK);
		this.gl.disable(this.gl.BLEND);
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
			this.gl.drawArrays(this.gl.TRIANGLES, 0, primitive.vertices.length);
		}
	}
}

function randomAxisInSlidePlane(): vec3 {
	const axis = vec3.fromValues(Math.random() * 2 - 1, Math.random() * 2 - 1, 0);
	if (vec3.length(axis) === 0) return vec3.fromValues(1, 0, 0);
	return vec3.normalize(axis, axis);
}

function RevolvingCirclesTransition(
	transitionParameters: TransitionParameters,
	nCircles: number = 8,
	nPointsOnCircles: number = 128,
) {
	if (nCircles < 2 || nPointsOnCircles < 4)
		return makeFlipTilesTransition(transitionParameters, 1, 1);

	const dRadius = 1.0 / nCircles;

	// Unit circle points, starting at the top and going clockwise on the screen.
	const unitCircle: vec2[] = [];
	for (let point = 0; point < nPointsOnCircles; point++) {
		const angle = (2 * Math.PI * point) / nPointsOnCircles - Math.PI / 2;
		unitCircle.push([Math.cos(angle), Math.sin(angle)]);
	}

	// A circle point scaled to the given radius, in slide coordinates. The outermost ring reaches
	// out to the slide corners, so its points are held inside the slide.
	const circlePoint = (
		radius: number,
		point: number,
		clamped: boolean,
	): vec2 => {
		let x = radius * unitCircle[point][0];
		let y = radius * unitCircle[point][1];
		if (clamped) {
			x = Math.min(Math.max(x, -1), 1);
			y = Math.min(Math.max(y, -1), 1);
		}
		return [x / 2 + 0.5, y / 2 + 0.5];
	};

	const aLeavingPrimitives: Primitive[] = [];
	const aEnteringPrimitives: Primitive[] = [];

	for (let i = 0; i < nCircles; i++) {
		const isOuterRing = i === nCircles - 1;
		const innerRadius = i * dRadius;
		const nominalOuterRadius = (i + 1) * dRadius;
		const outerRadius = isOuterRing ? Math.sqrt(2) : nominalOuterRadius;

		const ring = new Primitive();
		for (let side = 0; side < nPointsOnCircles; side++) {
			const next = (side + 1) % nPointsOnCircles;
			if (i === 0) {
				ring.pushTriangle(
					[0.5, 0.5],
					circlePoint(outerRadius, next, false),
					circlePoint(outerRadius, side, false),
				);
			} else {
				ring.pushTriangle(
					circlePoint(outerRadius, side, isOuterRing),
					circlePoint(innerRadius, side, false),
					circlePoint(innerRadius, next, false),
				);
				ring.pushTriangle(
					circlePoint(outerRadius, side, isOuterRing),
					circlePoint(innerRadius, next, false),
					circlePoint(outerRadius, next, isOuterRing),
				);
			}
		}

		// Each ring turns during its own window, which starts later the further out it is.
		const startTime = nominalOuterRadius / 2.0;
		const endTime = Math.min((nominalOuterRadius + dRadius + 1.0) / 2.0, 1.0);
		const axis = randomAxisInSlidePlane();
		const origin = vec3.fromValues(0, 0, 0);

		const leavingRing = Primitive.cloneDeep(ring);
		leavingRing.operations.push(
			makeSRotate(axis, origin, 180, true, startTime, endTime),
		);
		aLeavingPrimitives.push(leavingRing);

		// The entering ring starts turned away and comes around to face the viewer.
		ring.operations.push(
			makeSRotate(axis, origin, 180, true, startTime, endTime),
		);
		ring.operations.push(makeSRotate(axis, origin, -180, false, -1, 0));
		aEnteringPrimitives.push(ring);
	}

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new RevolvingCirclesTransitionImp(newTransitionParameters);
}

SlideShow.RevolvingCirclesTransition = RevolvingCirclesTransition;
