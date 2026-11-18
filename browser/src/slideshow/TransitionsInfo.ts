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

enum TransitionClass {
	Invalid,
	ClipPoligon,
	Special,
}

enum TransitionReverseMethod {
	Ignore,
	SubtractAndInvert,
	Rotate180,
	FlipX,
	FlipY,
}

// keep in alphabetic order
const aTransitionInfoTable: any = {};
// type: fake transition
aTransitionInfoTable[TransitionType.INVALID] = {};
// subtype: default
aTransitionInfoTable[TransitionType.INVALID][TransitionSubType.DEFAULT] = {
	class: TransitionClass.Invalid,
	rotationAngle: 0.0,
	scaleX: 0.0,
	scaleY: 0.0,
	reverseMethod: TransitionReverseMethod.Ignore,
	outInvertsSweep: false,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.BARNDOORWIPE] = {};
aTransitionInfoTable[TransitionType.BARNDOORWIPE][TransitionSubType.VERTICAL] =
	{
		class: TransitionClass.ClipPoligon,
		rotationAngle: 0.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.SubtractAndInvert,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};
aTransitionInfoTable[TransitionType.BARNDOORWIPE][
	TransitionSubType.HORIZONTAL
] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.SubtractAndInvert,
	outInvertsSweep: true,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.BARWIPE] = {};
aTransitionInfoTable[TransitionType.BARWIPE][TransitionSubType.LEFTTORIGHT] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.FlipX,
	outInvertsSweep: false,
	scaleIsotropically: false,
};
aTransitionInfoTable[TransitionType.BARWIPE][TransitionSubType.TOPTOBOTTOM] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.FlipY,
	outInvertsSweep: false,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.BLINDSWIPE] = {};
aTransitionInfoTable[TransitionType.BLINDSWIPE][TransitionSubType.HORIZONTAL] =
	{
		class: TransitionClass.ClipPoligon,
		rotationAngle: 90.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.FlipX,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};
aTransitionInfoTable[TransitionType.BLINDSWIPE][TransitionSubType.VERTICAL] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.FlipY,
	outInvertsSweep: true,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.CHECKERBOARDWIPE] = {};
aTransitionInfoTable[TransitionType.CHECKERBOARDWIPE][TransitionSubType.DOWN] =
	{
		class: TransitionClass.ClipPoligon,
		rotationAngle: 90.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.FlipY,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};
aTransitionInfoTable[TransitionType.CHECKERBOARDWIPE][
	TransitionSubType.ACROSS
] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.FlipX,
	outInvertsSweep: true,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.DISSOLVE] = {};
aTransitionInfoTable[TransitionType.DISSOLVE][TransitionSubType.DEFAULT] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.Ignore,
	outInvertsSweep: true,
	scaleIsotropically: true,
};

aTransitionInfoTable[TransitionType.ELLIPSEWIPE] = {};
aTransitionInfoTable[TransitionType.ELLIPSEWIPE][TransitionSubType.CIRCLE] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.SubtractAndInvert,
	outInvertsSweep: true,
	scaleIsotropically: true,
};
aTransitionInfoTable[TransitionType.ELLIPSEWIPE][TransitionSubType.HORIZONTAL] =
	{
		class: TransitionClass.ClipPoligon,
		rotationAngle: 0.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.SubtractAndInvert,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};
aTransitionInfoTable[TransitionType.ELLIPSEWIPE][TransitionSubType.VERTICAL] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.SubtractAndInvert,
	outInvertsSweep: true,
	scaleIsotropically: true,
};

aTransitionInfoTable[TransitionType.FADE] = {};
aTransitionInfoTable[TransitionType.FADE][TransitionSubType.CROSSFADE] =
	aTransitionInfoTable[TransitionType.FADE][TransitionSubType.FADEOVERCOLOR] = {
		class: TransitionClass.Special,
		rotationAngle: 0.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.Ignore,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};

aTransitionInfoTable[TransitionType.FANWIPE] = {};
aTransitionInfoTable[TransitionType.FANWIPE][TransitionSubType.CENTERTOP] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.FlipY,
	outInvertsSweep: true,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.FOURBOXWIPE] = {};
aTransitionInfoTable[TransitionType.FOURBOXWIPE][TransitionSubType.CORNERSIN] =
	aTransitionInfoTable[TransitionType.FOURBOXWIPE][
		TransitionSubType.CORNERSOUT
	] = {
		class: TransitionClass.ClipPoligon,
		rotationAngle: 0.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.SubtractAndInvert,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};

aTransitionInfoTable[TransitionType.IRISWIPE] = {};
aTransitionInfoTable[TransitionType.IRISWIPE][TransitionSubType.RECTANGLE] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 0.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.SubtractAndInvert,
	outInvertsSweep: true,
	scaleIsotropically: false,
};
aTransitionInfoTable[TransitionType.IRISWIPE][TransitionSubType.DIAMOND] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 45.0,
	scaleX: Math.SQRT2,
	scaleY: Math.SQRT2,
	reverseMethod: TransitionReverseMethod.SubtractAndInvert,
	outInvertsSweep: true,
	scaleIsotropically: false,
};

aTransitionInfoTable[TransitionType.PINWHEELWIPE] = {};
aTransitionInfoTable[TransitionType.PINWHEELWIPE][TransitionSubType.ONEBLADE] =
	aTransitionInfoTable[TransitionType.PINWHEELWIPE][
		TransitionSubType.TWOBLADEVERTICAL
	] =
	aTransitionInfoTable[TransitionType.PINWHEELWIPE][
		TransitionSubType.THREEBLADE
	] =
	aTransitionInfoTable[TransitionType.PINWHEELWIPE][
		TransitionSubType.FOURBLADE
	] =
	aTransitionInfoTable[TransitionType.PINWHEELWIPE][
		TransitionSubType.EIGHTBLADE
	] =
		{
			class: TransitionClass.ClipPoligon,
			rotationAngle: 0.0,
			scaleX: 1.0,
			scaleY: 1.0,
			reverseMethod: TransitionReverseMethod.FlipX,
			outInvertsSweep: true,
			scaleIsotropically: true,
		};

aTransitionInfoTable[TransitionType.RANDOMBARWIPE] = {};
aTransitionInfoTable[TransitionType.RANDOMBARWIPE][
	TransitionSubType.HORIZONTAL
] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.Ignore,
	outInvertsSweep: true,
	scaleIsotropically: false,
};
aTransitionInfoTable[TransitionType.RANDOMBARWIPE][TransitionSubType.VERTICAL] =
	{
		class: TransitionClass.ClipPoligon,
		rotationAngle: 0.0,
		scaleX: 1.0,
		scaleY: 1.0,
		reverseMethod: TransitionReverseMethod.Ignore,
		outInvertsSweep: true,
		scaleIsotropically: false,
	};

aTransitionInfoTable[TransitionType.SLIDEWIPE] = {};
aTransitionInfoTable[TransitionType.SLIDEWIPE][TransitionSubType.FROMLEFT] =
	aTransitionInfoTable[TransitionType.SLIDEWIPE][TransitionSubType.FROMTOP] =
	aTransitionInfoTable[TransitionType.SLIDEWIPE][TransitionSubType.FROMRIGHT] =
	aTransitionInfoTable[TransitionType.SLIDEWIPE][TransitionSubType.FROMBOTTOM] =
		{
			class: TransitionClass.Special,
			rotationAngle: 0.0,
			scaleX: 1.0,
			scaleY: 1.0,
			reverseMethod: TransitionReverseMethod.Ignore,
			outInvertsSweep: true,
			scaleIsotropically: false,
		};

aTransitionInfoTable[TransitionType.WATERFALLWIPE] = {};
aTransitionInfoTable[TransitionType.WATERFALLWIPE][
	TransitionSubType.HORIZONTALLEFT
] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: -90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.Rotate180,
	outInvertsSweep: true,
	scaleIsotropically: false,
};
aTransitionInfoTable[TransitionType.WATERFALLWIPE][
	TransitionSubType.HORIZONTALRIGHT
] = {
	class: TransitionClass.ClipPoligon,
	rotationAngle: 90.0,
	scaleX: 1.0,
	scaleY: 1.0,
	reverseMethod: TransitionReverseMethod.Rotate180,
	outInvertsSweep: true,
	scaleIsotropically: false,
};
