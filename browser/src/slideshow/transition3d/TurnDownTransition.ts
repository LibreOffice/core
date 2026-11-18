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
function TurnDownTransition(transitionParameters: TransitionParameters) {
	const slide = new Primitive();
	slide.pushTriangle([0, 0], [1, 0], [0, 1]);
	slide.pushTriangle([1, 0], [0, 1], [1, 1]);

	const aLeavingPrimitives: Primitive[] = [];
	aLeavingPrimitives.push(Primitive.cloneDeep(slide));

	slide.operations.push(
		makeSTranslate(vec3.fromValues(0, 0, 0.0001), false, -1.0, 0.0),
	);
	slide.operations.push(
		makeSRotate(
			vec3.fromValues(0, 0, 1),
			vec3.fromValues(-1, 1, 0),
			-90,
			true,
			0.0,
			1.0,
		),
	);
	slide.operations.push(
		makeSRotate(
			vec3.fromValues(0, 0, 1),
			vec3.fromValues(-1, 1, 0),
			90,
			false,
			-1.0,
			0.0,
		),
	);

	const aEnteringPrimitives: Primitive[] = [];
	aEnteringPrimitives.push(slide);

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: [],
	};

	return new SimpleTransition(newTransitionParameters);
}
/* jscpd:ignore-end */

SlideShow.TurnDownTransition = TurnDownTransition;
