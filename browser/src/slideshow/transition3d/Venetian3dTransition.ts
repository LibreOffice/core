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

declare var SlideShow: any;

function Venetian3dTransition(
	transitionParameters: TransitionParameters,
	vertical: boolean,
	parts: number,
) {
	const t30: number = Math.tan(Math.PI / 6.0);
	let ln = 0;
	const p = 1.0 / parts;

	const aLeavingPrimitives: Primitive[] = [];
	const aEnteringPrimitives: Primitive[] = [];
	for (let i = 0; i < parts; i++) {
		const Slide: Primitive = new Primitive();
		const n: number = (i + 1.0) / parts;
		if (!vertical) {
			Slide.pushTriangle([0, ln], [1, ln], [0, n]);
			Slide.pushTriangle([1, ln], [0, n], [1, n]);
			Slide.operations.push(
				makeRotateAndScaleDepthByHeight(
					vec3.fromValues(1, 0, 0),
					vec3.fromValues(0, 1 - n - ln, -t30 * p),
					-120,
					true,
					true,
					0.0,
					1.0,
				),
			);
		} else {
			Slide.pushTriangle([ln, 0], [n, 0], [ln, 1]);
			Slide.pushTriangle([n, 0], [ln, 1], [n, 1]);
			Slide.operations.push(
				makeRotateAndScaleDepthByWidth(
					vec3.fromValues(0, 1, 0),
					vec3.fromValues(n + ln - 1, 0, -t30 * p),
					-120,
					true,
					true,
					0.0,
					1.0,
				),
			);
		}
		aLeavingPrimitives.push(Primitive.cloneDeep(Slide));

		if (!vertical) {
			Slide.operations.push(
				makeSRotate(
					vec3.fromValues(1, 0, 0),
					vec3.fromValues(0, 1 - 2 * n, 0),
					-60,
					false,
					-1,
					0,
				),
			);
			Slide.operations.push(
				makeSRotate(
					vec3.fromValues(1, 0, 0),
					vec3.fromValues(0, 1 - n - ln, 0),
					180,
					false,
					-1,
					0,
				),
			);
		} else {
			Slide.operations.push(
				makeSRotate(
					vec3.fromValues(0, 1, 0),
					vec3.fromValues(2 * n - 1, 0, 0),
					-60,
					false,
					-1,
					0,
				),
			);
			Slide.operations.push(
				makeSRotate(
					vec3.fromValues(0, 1, 0),
					vec3.fromValues(n + ln - 1, 0, 0),
					180,
					false,
					-1,
					0,
				),
			);
		}
		aEnteringPrimitives.push(Slide);
		ln = n;
	}

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new SimpleTransition(newTransitionParameters);
}

SlideShow.Venetian3dTransition = Venetian3dTransition;
