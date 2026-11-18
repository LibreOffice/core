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

// TODO - remove code duplication
/* jscpd:ignore-start */
function TurnAroundTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();
	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [];
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	slide.operations.push(
		makeSScale(
			vec3.fromValues(1.0, -1.0, 1.0),
			vec3.fromValues(0, -1.02, 0),
			false,
			-1,
			0,
		),
	);
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	slide.clear();

	slide.operations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, 0),
			-180,
			true,
			false,
			0.0,
			1.0,
		),
	);
	const aEnteringPrimitives: Primitive[] = [Primitive.cloneDeep(slide)];
	slide.operations.push(
		makeSScale(
			vec3.fromValues(1.0, -1.0, 1.0),
			vec3.fromValues(0, -1.02, 0),
			false,
			-1,
			0,
		),
	);
	aEnteringPrimitives.push(Primitive.cloneDeep(slide));

	const aOperations: Operation[] = [];
	aOperations.push(makeSTranslate(vec3.fromValues(0, 0, -1.5), true, 0, 0.5));
	aOperations.push(makeSTranslate(vec3.fromValues(0, 0, 1.5), true, 0.5, 1));
	aOperations.push(
		makeRotateAndScaleDepthByWidth(
			vec3.fromValues(0, 1, 0),
			vec3.fromValues(0, 0, 0),
			-180,
			true,
			true,
			0.0,
			1.0,
		),
	);

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: aOperations,
	};

	return new ReflectionTransition(newTransitionParameters);
}
/* jscpd:ignore-end */

SlideShow.TurnAroundTransition = TurnAroundTransition;
