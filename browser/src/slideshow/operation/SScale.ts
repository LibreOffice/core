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

declare var glMatrix: any;
declare var SlideShow: any;

var { vec3, mat4 } = glMatrix;

class SScale extends Operation {
	private scale: vec3 = vec3.create();
	private origin: vec3 = vec3.create();
	private mbInterpolate: boolean = false;
	private mnT0: number = 0.0;
	private mnT1: number = 0.0;

	constructor(
		scale: vec3,
		origin: vec3,
		mbInterpolate: boolean,
		mnT0: number,
		mnT1: number,
	) {
		super();
		this.scale = scale;
		this.origin = origin;
		this.mbInterpolate = mbInterpolate;
		this.mnT0 = mnT0;
		this.mnT1 = mnT1;
	}
	private scaleMatrix(matrix: mat4, t: number, scale: vec3): mat4 {
		const scaleFactor = vec3.create();
		vec3.scale(scaleFactor, scale, t); // (t * scale)
		vec3.add(scaleFactor, scaleFactor, [1 - t, 1 - t, 1 - t]); // (1 - t) + (t * scale)

		const scaledMatrix = mat4.clone(matrix);
		mat4.scale(scaledMatrix, scaledMatrix, scaleFactor);

		return scaledMatrix;
	}

	public interpolate(
		matrix: mat4,
		t: number,
		SlideWidthScale: number,
		SlideHeightScale: number,
	): mat4 {
		if (t <= this.mnT0) return matrix;
		if (!this.mbInterpolate || t > this.mnT1) t = this.mnT1;
		t = OpsHelper.intervalInter(t, this.mnT0, this.mnT1);

		const translationVector = vec3.fromValues(
			SlideWidthScale * this.origin[0],
			SlideHeightScale * this.origin[1],
			this.origin[2],
		);

		mat4.translate(matrix, matrix, translationVector);
		matrix = this.scaleMatrix(matrix, t, this.scale);
		mat4.translate(
			matrix,
			matrix,
			vec3.negate(vec3.create(), translationVector),
		);
		return matrix;
	}
}
function makeSScale(
	scale: vec3,
	origin: vec3,
	mbInterpolate: boolean,
	mnT0: number,
	mnT1: number,
) {
	return new SScale(scale, origin, mbInterpolate, mnT0, mnT1);
}

SlideShow.makeSScale = makeSScale;
