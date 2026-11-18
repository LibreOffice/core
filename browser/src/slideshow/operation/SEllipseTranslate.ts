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

class SEllipseTranslate extends Operation {
	private width: number = 0.0;
	private height: number = 0.0;
	private startPosition: number = 0.0;
	private endPosition: number = 0.0;
	private mbInterpolate: boolean = false;
	private mnT0: number = 0.0;
	private mnT1: number = 0.0;

	constructor(
		dWidth: number,
		dHeight: number,
		dStartPosition: number,
		dEndPosition: number,
		bInter: boolean,
		T0: number,
		T1: number,
	) {
		super();
		this.width = dWidth;
		this.height = dHeight;
		this.startPosition = dStartPosition;
		this.endPosition = dEndPosition;
		this.mbInterpolate = bInter;
		this.mnT0 = T0;
		this.mnT1 = T1;
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

		// let a1:number ,a2: number ,x :number, y: number ;
		const a1 = this.startPosition * 2 * Math.PI;
		const a2 =
			(this.startPosition + t * (this.endPosition - this.startPosition)) *
			2 *
			Math.PI;
		const x = (this.width * (Math.cos(a2) - Math.cos(a1))) / 2;
		const y = (this.height * (Math.sin(a2) - Math.sin(a1))) / 2;

		mat4.translate(matrix, matrix, vec3.fromValues(x, 0, y));
		return matrix;
	}
}

function makeSEllipseTranslate(
	dWidth: number,
	dHeight: number,
	dStartPosition: number,
	dEndPosition: number,
	bInter: boolean,
	T0: number,
	T1: number,
) {
	return new SEllipseTranslate(
		dWidth,
		dHeight,
		dStartPosition,
		dEndPosition,
		bInter,
		T0,
		T1,
	);
}

SlideShow.makeSEllipseTranslate = makeSEllipseTranslate;
