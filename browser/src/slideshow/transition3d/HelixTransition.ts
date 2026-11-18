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

class HelixTransition extends SimpleTransition {
	constructor(transitionParameters: TransitionParameters3D) {
		super(transitionParameters);
	}

	public displaySlides_(t: number): void {
		this.applyAllOperation(t);
		if (t < 0.5)
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				0,
				this.leavingPrimitives,
				'slideTexture',
			);
		else
			this.displayPrimitive(
				t,
				this.gl.TEXTURE0,
				1,
				this.enteringPrimitives,
				'slideTexture',
			);
	}
}

function makeHelixTransition(
	transitionParameters: TransitionParameters,
	nRows: number = 20,
) {
	const invN = 1.0 / nRows;
	const delayFactor = 0.2; // Adjust this to control the delay between the leaving and entering operations
	let iDn = 0.0;
	let iPDn = invN;
	const aLeavingPrimitives: Primitive[] = [];
	const aEnteringPrimitives: Primitive[] = [];
	const aOperations: Operation[] = [];
	for (let i = 0; i < nRows; i++) {
		const Tile = new Primitive();

		Tile.pushTriangle([1.0, iDn], [0.0, iDn], [0.0, iPDn]);
		Tile.pushTriangle([1.0, iPDn], [1.0, iDn], [0.0, iPDn]);

		const leaveStartTime = Math.min(
			Math.max(((i - nRows / 2.0) * invN) / 2.0, 0.0),
			1.0,
		);
		const leaveEndTime = Math.min(
			Math.max(((i + nRows / 2.0) * invN) / 2.0, 0.0),
			1.0,
		);

		Tile.operations.push(
			makeSRotate(
				vec3.fromValues(0, 1, 0),
				vec3.fromValues(0, 1, 0),
				180,
				true,
				leaveStartTime,
				leaveEndTime,
			),
		);
		aLeavingPrimitives.push(Primitive.cloneDeep(Tile));

		const enterStartTime = leaveEndTime + delayFactor * invN;
		const enterEndTime = enterStartTime + (1.0 - leaveEndTime);

		Tile.operations.push(
			makeSRotate(
				vec3.fromValues(0, 1, 0),
				vec3.fromValues(0, 1, 0),
				-180,
				true,
				enterStartTime,
				enterEndTime,
			),
		);
		aEnteringPrimitives.push(Tile);
		iDn += invN;
		iPDn += invN;
	}

	const newTransitionParameters: TransitionParameters3D = {
		...transitionParameters,
		leavingPrimitives: aLeavingPrimitives,
		enteringPrimitives: aEnteringPrimitives,
		allOperations: aOperations,
	};

	return new HelixTransition(newTransitionParameters);
}

SlideShow.makeHelixTransition = makeHelixTransition;
