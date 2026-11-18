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

class STranslate extends Operation {
	private vector: vec3;
	private mbInterpolate: boolean = false;
	private mnT0: number = 0.0;
	private mnT1: number = 0.0;

	// TODO - remove code duplication
	/* jscpd:ignore-start */
	constructor(
		vector: vec3,
		mbInterpolate: boolean,
		mnT0: number,
		mnT1: number,
	) {
		super();
		this.vector = vector;
		this.mbInterpolate = mbInterpolate;
		this.mnT0 = mnT0;
		this.mnT1 = mnT1;
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
		mat4.translate(
			matrix,
			matrix,
			vec3.fromValues(
				SlideWidthScale * t * this.vector[0],
				SlideHeightScale * t * this.vector[1],
				t * this.vector[2],
			),
		);
		return matrix;
	}
	/* jscpd:ignore-end */
}

function makeSTranslate(
	vector: vec3,
	mbInterpolate: boolean,
	mnT0: number,
	mnT1: number,
) {
	return new STranslate(vector, mbInterpolate, mnT0, mnT1);
}

SlideShow.makeSTranslate = makeSTranslate;
