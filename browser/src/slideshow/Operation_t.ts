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

/* This file for Set Primitive Operations */

declare var SlideShow: any;

// prettier-ignore
declare type mat4 = [
	number, number, number, number,
	number, number, number, number,
	number, number, number, number,
	number, number, number, number,
];

declare type vec2 = [number, number];
declare type vec3 = [number, number, number];
declare type vec4 = [number, number, number, number];

class Operation_t {
	OperationCB: any;

	constructor(OperationCB: any) {
		this.OperationCB = OperationCB;
	}

	applyOperations(
		matrix: any,
		t: number,
		SlideWidthScale: number,
		SlideHeightScale: number,
	) {
		return this.OperationCB().interpolate(
			matrix,
			t,
			SlideWidthScale,
			SlideHeightScale,
		);
	}
}
SlideShow.Operation_t = Operation_t;
