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

class RotateAndScaleDepth extends Operation {
	private axis: vec3 = vec3.create();
	private origin: vec3 = vec3.create();
	private angle: number = 0.0;
	private scale: boolean = false;
	private mbInterpolate: boolean = false;
	private mnT0: number = 0.0;
	private mnT1: number = 0.0;
	private isDepthByWidth: boolean = true;

	constructor(
		axis: vec3,
		origin: vec3,
		angle: number,
		scale: boolean,
		mbInterpolate: boolean,
		mnT0: number,
		mnT1: number,
		isDepthByWidth: boolean,
	) {
		super();
		this.axis = axis;
		this.origin = origin;
		this.angle = OpsHelper.deg2rad(angle);
		this.scale = scale;
		this.mbInterpolate = mbInterpolate;
		this.mnT0 = mnT0;
		this.mnT1 = mnT1;
		this.isDepthByWidth = isDepthByWidth;
	}

	public interpolate(
		matrix: mat4,
		t: number,
		SlideWidthScale: number,
		SlideHeightScale: number,
	): mat4 {
		if (t <= this.mnT0) {
			return matrix;
		}
		if (!this.mbInterpolate || t > this.mnT1) {
			t = this.mnT1;
		}
		t = OpsHelper.intervalInter(t, this.mnT0, this.mnT1);
		const translationVector = vec3.fromValues(
			SlideWidthScale * this.origin[0],
			SlideHeightScale * this.origin[1],
			(this.isDepthByWidth ? SlideWidthScale : SlideHeightScale) *
				this.origin[2],
		);
		const scaleVector = vec3.fromValues(
			SlideWidthScale * SlideWidthScale,
			SlideHeightScale * SlideHeightScale,
			1,
		);
		mat4.translate(matrix, matrix, translationVector);
		if (this.scale) {
			mat4.scale(matrix, matrix, scaleVector);
		}
		mat4.rotate(matrix, matrix, this.angle * t, this.axis);
		if (this.scale) {
			mat4.scale(
				matrix,
				matrix,
				vec3.fromValues(
					1 / scaleVector[0],
					1 / scaleVector[1],
					1 / scaleVector[2],
				),
			);
		}
		mat4.translate(
			matrix,
			matrix,
			vec3.negate(vec3.create(), translationVector),
		);
		return matrix;
	}
}

function makeRotateAndScaleDepthByWidth(
	axis: vec3,
	origin: vec3,
	angle: number,
	scale: boolean,
	mbInterpolate: boolean,
	mnT0: number,
	mnT1: number,
) {
	return new RotateAndScaleDepth(
		axis,
		origin,
		angle,
		scale,
		mbInterpolate,
		mnT0,
		mnT1,
		true,
	);
}

function makeRotateAndScaleDepthByHeight(
	axis: vec3,
	origin: vec3,
	angle: number,
	scale: boolean,
	mbInterpolate: boolean,
	mnT0: number,
	mnT1: number,
) {
	return new RotateAndScaleDepth(
		axis,
		origin,
		angle,
		scale,
		mbInterpolate,
		mnT0,
		mnT1,
		false,
	);
}

SlideShow.makeRotateAndScaleDepthByWidth = makeRotateAndScaleDepthByWidth;
SlideShow.makeRotateAndScaleDepthByHeight = makeRotateAndScaleDepthByHeight;
