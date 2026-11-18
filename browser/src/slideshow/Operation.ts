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

var { vec2, vec3, mat4 } = glMatrix;

class Operation {
	public interpolate(
		matrix: mat4,
		t: number,
		SlideWidthScale: number,
		SlideHeightScale: number,
	): mat4 {
		return matrix;
	}
}

SlideShow.Operation = Operation;

class OpsHelper {
	static intervalInter(t: number, T0: number, T1: number): number {
		return (t - T0) / (T1 - T0);
	}

	static deg2rad(v: number, degMultiple = 1): number {
		// Divide first for exact values at multiples of 90 degrees
		return ((v / (90.0 * degMultiple)) * Math.PI) / 2;
	}
}

SlideShow.OpsHelper = OpsHelper;
