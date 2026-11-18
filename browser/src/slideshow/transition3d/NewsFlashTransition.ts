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

function NewsFlashTransition(transitionParameters: TransitionParameters) {
	const lSlide = new Primitive();
	const eSlide = new Primitive();

	lSlide.pushTriangle([0, 0], [1, 0], [0, 1]);
	lSlide.pushTriangle([1, 0], [0, 1], [1, 1]);

	eSlide.pushTriangle([0, 0], [1, 0], [0, 1]);
	eSlide.pushTriangle([1, 0], [0, 1], [1, 1]);

	lSlide.operations.push(
		makeSRotate(
			vec3.fromValues(0, 0, 1),
			vec3.fromValues(0, 0, 0),
			3000,
			true,
			0,
			0.5,
		),
	);
	lSlide.operations.push(
		makeSScale(
			vec3.fromValues(0.01, 0.01, 0.01),
			vec3.fromValues(0, 0, 0),
			true,
			0,
			0.5,
		),
	);
	lSlide.operations.push(
		makeSTranslate(vec3.fromValues(-10000, 0, 0), false, 0.5, 2),
	);
	const aLeavingPrimitives: Primitive[] = [lSlide];

	eSlide.operations.push(
		makeSRotate(
			vec3.fromValues(0, 0, 1),
			vec3.fromValues(0, 0, 0),
			-3000,
			true,
			0.5,
			1,
		),
	);
	eSlide.operations.push(
		makeSTranslate(vec3.fromValues(-100, 0, 0), false, -1, 1),
	);
	eSlide.operations.push(
		makeSTranslate(vec3.fromValues(100, 0, 0), false, 0.5, 1),
	);
	eSlide.operations.push(
		makeSScale(
			vec3.fromValues(0.01, 0.01, 0.01),
			vec3.fromValues(0, 0, 0),
			false,
			-1,
			1,
		),
	);
	eSlide.operations.push(
		makeSScale(
			vec3.fromValues(100, 100, 100),
			vec3.fromValues(0, 0, 0),
			true,
			0.5,
			1,
		),
	);
	const aEnteringPrimitives: Primitive[] = [eSlide];

	const aOperations: Operation[] = [];
	aOperations.push(
		makeSRotate(
			vec3.fromValues(0, 0, 1),
			vec3.fromValues(0.2, 0.2, 0),
			1080,
			true,
			0,
			1,
		),
	);

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: aOperations,
	};

	return new SimpleTransition(newTransitionParameters);
}

SlideShow.NewsFlashTransition = NewsFlashTransition;
