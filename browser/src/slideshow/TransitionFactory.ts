/* -*- js-indent-level: 8 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

declare var SlideShow: any;

enum TransitionType {
	INVALID,
	BARWIPE,
	PINWHEELWIPE,
	SLIDEWIPE,
	RANDOMBARWIPE,
	CHECKERBOARDWIPE,
	FOURBOXWIPE,
	IRISWIPE,
	FANWIPE,
	BLINDSWIPE,
	FADE,
	DISSOLVE,
	PUSHWIPE,
	ELLIPSEWIPE,
	BARNDOORWIPE,
	WATERFALLWIPE,
	MISCSHAPEWIPE,
	ZOOM,
}

enum TransitionSubType {
	DEFAULT,
	LEFTTORIGHT,
	TOPTOBOTTOM,
	EIGHTBLADE,
	FOURBLADE,
	THREEBLADE,
	TWOBLADEVERTICAL,
	ONEBLADE,
	FROMTOPLEFT,
	FROMTOPRIGHT,
	FROMBOTTOMLEFT,
	FROMBOTTOMRIGHT,
	VERTICAL,
	HORIZONTAL,
	DOWN,
	ACROSS,
	CORNERSOUT,
	DIAMOND,
	CIRCLE,
	RECTANGLE,
	CENTERTOP,
	CROSSFADE,
	FADEOVERCOLOR,
	FROMLEFT,
	FROMRIGHT,
	FROMTOP,
	FROMBOTTOM,
	HORIZONTALLEFT,
	HORIZONTALRIGHT,
	COMBVERTICAL,
	COMBHORIZONTAL,
	TOPLEFT,
	TOPRIGHT,
	BOTTOMRIGHT,
	BOTTOMLEFT,
	TOPCENTER,
	RIGHTCENTER,
	BOTTOMCENTER,
	HEARTCORNERSIN,
	FANOUTHORIZONTAL,
	CORNERSIN,
	HEART,
	ROTATEIN,
	AIRPLANE,
	MORPH,
	MORPHBYWORD,
	MORPHBYCHARACTER,
}

const stringToTransitionTypeMap: Record<string, TransitionType> = {
	Invalid: TransitionType.INVALID,
	BarWipe: TransitionType.BARWIPE,
	PineWheelWipe: TransitionType.PINWHEELWIPE,
	SlideWipe: TransitionType.SLIDEWIPE,
	RandomBarWipe: TransitionType.RANDOMBARWIPE,
	CheckerBoardWipe: TransitionType.CHECKERBOARDWIPE,
	FourBoxWipe: TransitionType.FOURBOXWIPE,
	IrisWipe: TransitionType.IRISWIPE,
	FanWipe: TransitionType.FANWIPE,
	BlindWipe: TransitionType.BLINDSWIPE,
	Fade: TransitionType.FADE,
	Dissolve: TransitionType.DISSOLVE,
	PushWipe: TransitionType.PUSHWIPE,
	EllipseWipe: TransitionType.ELLIPSEWIPE,
	BarnDoorWipe: TransitionType.BARNDOORWIPE,
	WaterfallWipe: TransitionType.WATERFALLWIPE,
	MiscShapeWipe: TransitionType.MISCSHAPEWIPE,
	Zoom: TransitionType.ZOOM,
};

const stringToTransitionSubTypeMap: Record<string, TransitionSubType> = {
	Default: TransitionSubType.DEFAULT,
	LeftToRight: TransitionSubType.LEFTTORIGHT,
	TopToBottom: TransitionSubType.TOPTOBOTTOM,
	'8Blade': TransitionSubType.EIGHTBLADE,
	'4Blade': TransitionSubType.FOURBLADE,
	'3Blade': TransitionSubType.THREEBLADE,
	'2BladeVertical': TransitionSubType.TWOBLADEVERTICAL,
	'1Blade': TransitionSubType.ONEBLADE,
	FromTopLeft: TransitionSubType.FROMTOPLEFT,
	FromTopRight: TransitionSubType.FROMTOPRIGHT,
	FromBottomLeft: TransitionSubType.FROMBOTTOMLEFT,
	FromBottomRight: TransitionSubType.FROMBOTTOMRIGHT,
	Vertical: TransitionSubType.VERTICAL,
	Horizontal: TransitionSubType.HORIZONTAL,
	Down: TransitionSubType.DOWN,
	Across: TransitionSubType.ACROSS,
	CornersOut: TransitionSubType.CORNERSOUT,
	Diamond: TransitionSubType.DIAMOND,
	Circle: TransitionSubType.CIRCLE,
	Rectangle: TransitionSubType.RECTANGLE,
	CenterTop: TransitionSubType.CENTERTOP,
	CrossFade: TransitionSubType.CROSSFADE,
	FadeOverColor: TransitionSubType.FADEOVERCOLOR,
	FromLeft: TransitionSubType.FROMLEFT,
	FromRight: TransitionSubType.FROMRIGHT,
	FromTop: TransitionSubType.FROMTOP,
	FromBottom: TransitionSubType.FROMBOTTOM,
	HorizontalLeft: TransitionSubType.HORIZONTALLEFT,
	HorizontalRight: TransitionSubType.HORIZONTALRIGHT,
	CombVertical: TransitionSubType.COMBVERTICAL,
	CombHorizontal: TransitionSubType.COMBHORIZONTAL,
	TopLeft: TransitionSubType.TOPLEFT,
	TopRight: TransitionSubType.TOPRIGHT,
	BottomRight: TransitionSubType.BOTTOMRIGHT,
	BottomLeft: TransitionSubType.BOTTOMLEFT,
	TopCenter: TransitionSubType.TOPCENTER,
	RightCenter: TransitionSubType.RIGHTCENTER,
	BottomCenter: TransitionSubType.BOTTOMCENTER,
	CornersIn: TransitionSubType.CORNERSIN,
	FanOutHorizontal: TransitionSubType.FANOUTHORIZONTAL,
	Heart: TransitionSubType.HEART,
	RotateIn: TransitionSubType.ROTATEIN,
	Airplane: TransitionSubType.AIRPLANE,
	Morph: TransitionSubType.MORPH,
	MorphByWord: TransitionSubType.MORPHBYWORD,
	MorphByCharacter: TransitionSubType.MORPHBYCHARACTER,
};

function createTransition(
	transitionParameters: TransitionParameters,
	isSlideTransition: boolean,
) {
	const type = transitionParameters.transitionFilterInfo.transitionType;
	switch (type) {
		case TransitionType.BARWIPE:
			return BarWipeTransition(transitionParameters);

		case TransitionType.PINWHEELWIPE:
			return new SlideShow.WheelTransition(transitionParameters);

		case TransitionType.RANDOMBARWIPE:
			return new SlideShow.BarsTransition(transitionParameters);

		case TransitionType.CHECKERBOARDWIPE:
			return new SlideShow.CheckersTransition(transitionParameters);

		case TransitionType.FOURBOXWIPE:
			return new SlideShow.PlusTransition(transitionParameters);

		case TransitionType.IRISWIPE:
			return SlideShow.IrisWipeTransition(transitionParameters);

		case TransitionType.ELLIPSEWIPE:
			return SlideShow.EllipseWipeTransition(transitionParameters);

		case TransitionType.FANWIPE:
			return new SlideShow.WedgeTransition(transitionParameters);

		case TransitionType.BLINDSWIPE:
			return new SlideShow.VenetianTransition(transitionParameters);

		case TransitionType.DISSOLVE:
			return new SlideShow.SimpleDissolveTransition(transitionParameters);

		case TransitionType.BARNDOORWIPE:
			return new SlideShow.SplitTransition(transitionParameters);

		case TransitionType.WATERFALLWIPE:
			return new SlideShow.DiagonalTransition(transitionParameters);
	}

	if (isSlideTransition) {
		switch (type) {
			case TransitionType.FADE:
				return new SlideShow.FadeTransition(transitionParameters);

			case TransitionType.SLIDEWIPE:
				return SlideWipeTransition(transitionParameters);

			case TransitionType.PUSHWIPE:
				return SlideShow.PushWipeTransition(transitionParameters);

			case TransitionType.MISCSHAPEWIPE:
				return SlideShow.MicsShapeWipeTransition(transitionParameters);

			case TransitionType.ZOOM:
				return SlideShow.NewsFlashTransition(transitionParameters);
		}
	}

	app.console.log(
		'Unknown transition type',
		transitionParameters.transitionFilterInfo.transitionType,
	);
	return new SlideShow.NoTransition(transitionParameters);
}
