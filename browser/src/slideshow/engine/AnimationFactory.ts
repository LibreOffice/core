// @ts-strict-ignore
/* -*- tab-width: 4 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

type GetterType = () => any;
type SetterType = (x: any) => any;

abstract class AnimationBase {
	public abstract start(aAnimatableElement: AnimatedElement): void;

	public abstract perform(aValue: any, last?: boolean): void;

	public abstract end(): void;

	public abstract getUnderlyingValue(): any;
}

class GenericAnimation extends AnimationBase {
	protected readonly aGetValueFunc: GetterType;
	protected readonly aSetValueFunc: SetterType;
	private readonly aGetModifier: SetterType;
	private readonly aSetModifier: SetterType;
	private aAnimatableElement: AnimatedElement;
	private bAnimationStarted: boolean;

	constructor(
		aGetValueFunc: GetterType,
		aSetValueFunc: SetterType,
		aGetModifier?: SetterType,
		aSetModifier?: SetterType,
	) {
		super();

		assert(
			aGetValueFunc && aSetValueFunc,
			'GenericAnimation constructor: get value functor and/or set value functor are not valid',
		);

		this.aGetValueFunc = aGetValueFunc;
		this.aSetValueFunc = aSetValueFunc;
		this.aGetModifier = aGetModifier;
		this.aSetModifier = aSetModifier;
		this.aAnimatableElement = null;
		this.bAnimationStarted = false;
	}

	start(aAnimatableElement: any): void {
		assert(
			aAnimatableElement,
			'GenericAnimation.start: animatable element is not valid',
		);

		this.aAnimatableElement = aAnimatableElement;
		this.aAnimatableElement.notifyAnimationStart();

		if (!this.bAnimationStarted) this.bAnimationStarted = true;
	}

	end(): void {
		if (this.bAnimationStarted) {
			this.bAnimationStarted = false;
			this.aAnimatableElement.notifyAnimationEnd();
		}
	}

	perform(aValue: any): void {
		if (this.aSetModifier) aValue = this.aSetModifier(aValue);

		this.aSetValueFunc(aValue);
	}

	getUnderlyingValue(): any {
		let aValue = this.aGetValueFunc();
		if (this.aGetModifier) aValue = this.aGetModifier(aValue);
		return aValue;
	}
}

class TupleAnimation extends GenericAnimation {
	private aDefaultValue: any[];
	private aReferenceSize: any[];

	constructor(
		aGetValueFunc: GetterType,
		aSetValueFunc: SetterType,
		aDefaultValue: any[],
		aReferenceSize: any[],
	) {
		super(aGetValueFunc, aSetValueFunc);

		assert(
			aDefaultValue && aReferenceSize,
			'TupleAnimation constructor: default value functor and/or reference size are not valid',
		);

		this.aDefaultValue = aDefaultValue;
		this.aReferenceSize = aReferenceSize;
	}

	perform(aNormValue: any[]): void {
		assert(
			aNormValue.length === this.aReferenceSize.length,
			'TupleAnimation.perform: aNormValue array param has wrong length',
		);

		const aValue: any[] = [];
		for (let i = 0; i < aNormValue.length; ++i) {
			aValue.push(aNormValue[i] * this.aReferenceSize[i]);
		}

		this.aSetValueFunc(aValue);
	}

	getUnderlyingValue(): any {
		const aValue: any[] = this.aGetValueFunc();
		assert(
			aValue.length === this.aReferenceSize.length,
			'TupleAnimation.perform: array param has wrong length',
		);

		const aNormValue: any[] = [];
		for (let i = 0; i < aValue.length; ++i) {
			aNormValue.push(aValue[i] / this.aReferenceSize[i]);
		}
		return aNormValue;
	}
}

class HSLAnimationWrapper implements AnimationBase {
	private aAnimation: AnimationBase;

	constructor(aColorAnimation: AnimationBase) {
		this.aAnimation = aColorAnimation;
	}

	start(aAnimatableElement: AnimatedElement) {
		this.aAnimation.start(aAnimatableElement);
	}

	end() {
		this.aAnimation.end();
	}

	perform(aHSLValue: HSLColor) {
		this.aAnimation.perform(aHSLValue.convertToRGB());
	}

	getUnderlyingValue() {
		return this.aAnimation.getUnderlyingValue().convertToHSL();
	}
}

function createPropertyAnimation(
	sAttrName: string,
	aAnimatedElement: AnimatedElement,
	nWidth: number,
	nHeight: number,
): AnimationBase {
	const sPropNameAsKey = sAttrName as PropertyGetterSetterMapKeyType;
	if (!aPropertyGetterSetterMap[sPropNameAsKey]) {
		window.app.console.log(
			'createPropertyAnimation: attribute is unknown: ' + sAttrName,
		);
		return null;
	}

	const aFunctorSet: PropertyGetterSetter =
		aPropertyGetterSetterMap[sPropNameAsKey];

	const sGetValueMethod = aFunctorSet.get;
	const sSetValueMethod = aFunctorSet.set;

	if (!sGetValueMethod || !sSetValueMethod) {
		window.app.console.log('createPropertyAnimation: attribute is not handled');
		return null;
	}

	const aGetModifier = aFunctorSet.getmod
		? aFunctorSet.getmod(nWidth, nHeight)
		: null;
	const aSetModifier = aFunctorSet.setmod
		? aFunctorSet.setmod(nWidth, nHeight)
		: null;

	const aGetValueMethod =
		aAnimatedElement[sGetValueMethod as keyof typeof aAnimatedElement];
	const aSetValueMethod =
		aAnimatedElement[sSetValueMethod as keyof typeof aAnimatedElement];
	return new GenericAnimation(
		aGetValueMethod.bind(aAnimatedElement),
		aSetValueMethod.bind(aAnimatedElement),
		aGetModifier,
		aSetModifier,
	);
}

function createPairPropertyAnimation(
	sTransformType: string,
	aAnimatedElement: AnimatedElement,
	nWidth: number,
	nHeight: number,
): AnimationBase {
	const sTransformTypeAsKey = sTransformType as PropertyGetterSetterMapKeyType;
	const aFunctorSet: PropertyGetterSetter =
		aPropertyGetterSetterMap[sTransformTypeAsKey];
	const sGetValueMethod = aFunctorSet.get;
	const sSetValueMethod = aFunctorSet.set;

	const aDefaultValue: any[] = [];
	const aSizeReference: any[] = [];
	if (sTransformType === 'scale') {
		aDefaultValue[0] = aSizeReference[0] = aAnimatedElement.getBaseBBox().width;
		aDefaultValue[1] = aSizeReference[1] =
			aAnimatedElement.getBaseBBox().height;
	} else if (sTransformType === 'translate') {
		aDefaultValue[0] = aAnimatedElement.getBaseCenterX();
		aDefaultValue[1] = aAnimatedElement.getBaseCenterY();
		aSizeReference[0] = nWidth;
		aSizeReference[1] = nHeight;
	} else {
		window.app.console.log(
			'createPairPropertyAnimation: transform type is not handled',
		);
		return null;
	}

	const aGetValueMethod =
		aAnimatedElement[sGetValueMethod as keyof typeof aAnimatedElement];
	const aSetValueMethod =
		aAnimatedElement[sSetValueMethod as keyof typeof aAnimatedElement];
	return new TupleAnimation(
		aGetValueMethod.bind(aAnimatedElement),
		aSetValueMethod.bind(aAnimatedElement),
		aDefaultValue,
		aSizeReference,
	);
}

class TransitionFilterAnimation extends AnimationBase {
	private nNodeId: number;
	private aTransitionFilterInfo: TransitionFilterInfo;
	private aAnimatableElement: AnimatedElement;
	private bAnimationStarted: boolean;
	private aTransition: TransitionBase;

	constructor(
		nNodeId: number,
		transitionFilterInfo: TransitionFilterInfo,
		aAnimatableElement: AnimatedElement,
	) {
		assert(
			aAnimatableElement,
			'TransitionFilterAnimation: animatable element is not valid',
		);
		ANIMDBG.print(
			'TransitionFilterAnimation: Animated Element Id: ' +
				aAnimatableElement.getId(),
		);

		super();

		this.nNodeId = nNodeId;
		this.aTransitionFilterInfo = transitionFilterInfo;
		this.aAnimatableElement = aAnimatableElement;
		this.bAnimationStarted = false;
		this.aTransition = null;
	}

	getNodeId() {
		return this.nNodeId;
	}

	getTransitionFilterInfo(): TransitionFilterInfo {
		return this.aTransitionFilterInfo;
	}

	start(aAnimatableElement: AnimatedElement): void {
		assert(
			this.aAnimatableElement.getId() === aAnimatableElement.getId(),
			'TransitionFilterAnimation: animatable element mismatch',
		);

		if (!this.aTransition) {
			const transitionParameters = this.createTransitionParameters(
				this.aAnimatableElement,
				this.aTransitionFilterInfo,
			);
			// Without a rendering context there is no WebGL transition object.
			// The frames are still recorded on the element.
			if (transitionParameters.context)
				this.aTransition = this.createShapeTransition(transitionParameters);
		}

		this.aAnimatableElement.notifyAnimationStart();
		if (!this.bAnimationStarted) this.bAnimationStarted = true;
	}

	end(): void {
		if (this.bAnimationStarted) {
			this.bAnimationStarted = false;
			this.aAnimatableElement.notifyAnimationEnd();
		}
	}

	perform(nT: number): void {
		this.aAnimatableElement.setTransitionFilterFrame(this, nT);
	}

	renderFrame(nT: number, properties?: AnimatedElementRenderProperties): void {
		if (this.aTransition) {
			this.aTransition.renderFrame(nT, properties);
		}
	}

	notifySlideEnd(): void {
		// clean up resources
		if (this.aTransition) {
			this.aTransition.end();
			this.aTransition = null;
		}
	}

	getUnderlyingValue(): any {
		return 0.0;
	}

	private createTransitionParameters(
		aAnimatedElement: AnimatedElement,
		transitionFilterInfo: TransitionFilterInfo,
	) {
		const transitionParameters = new TransitionParameters();
		aAnimatedElement.setTransitionParameters(transitionParameters);
		transitionParameters.transitionFilterInfo = transitionFilterInfo;
		return transitionParameters;
	}

	private createShapeTransition(
		transitionParameters: TransitionParameters,
	): TransitionBase {
		return createTransition(transitionParameters, /*isSlideTransition*/ false);
	}
}

class PathAnimation extends AnimationBase {
	private path: string;
	private eAdditive: AdditiveMode;
	private slideWidth: number;
	private slideHeight: number;
	private aAnimatableElement: AnimatedElement;
	private bAnimationStarted: boolean;
	private centerX: number;
	private centerY: number;
	private svgPath: SVGPathElement;
	private pathLength: number;

	constructor(
		path: string,
		eAdditive: AdditiveMode,
		slideWidth: number,
		slideHeight: number,
	) {
		super();
		this.path = path;
		this.eAdditive = eAdditive;
		this.slideWidth = slideWidth;
		this.slideHeight = slideHeight;

		this.svgPath = this.createSvgPath(path);
		this.pathLength = this.svgPath.getTotalLength();
	}

	start(aAnimatableElement: AnimatedElement): void {
		assert(
			aAnimatableElement,
			'GenericAnimation.start: animatable element is not valid',
		);

		this.aAnimatableElement = aAnimatableElement;
		this.centerX = this.aAnimatableElement.getBaseCenterX();
		this.centerY = this.aAnimatableElement.getBaseCenterY();

		this.aAnimatableElement.notifyAnimationStart();

		if (!this.bAnimationStarted) this.bAnimationStarted = true;
	}

	end(): void {
		if (this.bAnimationStarted) {
			this.bAnimationStarted = false;
			this.aAnimatableElement.notifyAnimationEnd();
		}
	}

	perform(nT: number): void {
		let aOutPos = this.parametricPath(nT);
		aOutPos = [aOutPos[0] * this.slideWidth, aOutPos[1] * this.slideHeight];
		aOutPos = [aOutPos[0] + this.centerX, aOutPos[1] + this.centerY];

		this.aAnimatableElement.setPos(aOutPos);
	}

	getUnderlyingValue(): any {
		return 0.0;
	}

	private createSvgPath(path: string): SVGPathElement {
		const svgPath = document.createElementNS(
			'http://www.w3.org/2000/svg',
			'path',
		);
		svgPath.setAttribute('d', path);
		return svgPath;
	}

	private parametricPath(nT: number): [number, number] {
		const distance = this.pathLength * nT;
		const point = this.svgPath.getPointAtLength(distance);
		return [point.x, point.y];
	}
}

function createShapeTransition(
	aActivityParamSet: ActivityParamSet,
	aAnimatedElement: AnimatedElement,
	nSlideWidth: number,
	nSlideHeight: number,
	aAnimatedTransitionFilterNode: AnimationTransitionFilterNode,
): AnimationActivity {
	if (!aAnimatedTransitionFilterNode) {
		window.app.console.log(
			'createShapeTransition: the animated transition filter node is not valid.',
		);
		return null;
	}
	const eTransitionType = aAnimatedTransitionFilterNode.getTransitionType();
	const eTransitionSubType =
		aAnimatedTransitionFilterNode.getTransitionSubtype();
	const bDirectionForward =
		!aAnimatedTransitionFilterNode.getReverseDirection();
	const bModeIn =
		aAnimatedTransitionFilterNode.getTransitionMode() == TransitionMode.in;

	const transitionFilterInfo = new TransitionFilterInfo(
		eTransitionType,
		eTransitionSubType,
		bDirectionForward,
		bModeIn,
	);

	let aTransitionInfo = null;
	if (aTransitionInfoTable[eTransitionType])
		aTransitionInfo = aTransitionInfoTable[eTransitionType][eTransitionSubType];

	const eTransitionClass: TransitionClass = aTransitionInfo
		? aTransitionInfo['class']
		: TransitionClass.Invalid;

	switch (eTransitionClass) {
		default:
		case TransitionClass.Invalid:
			window.app.console.log(
				'createShapeTransition: transition class: TRANSITION_INVALID',
			);
			return null;

		case TransitionClass.ClipPoligon: {
			const aClippingAnimation = new TransitionFilterAnimation(
				aAnimatedTransitionFilterNode.getId(),
				transitionFilterInfo,
				aAnimatedElement,
			);

			return new SimpleActivity(
				aActivityParamSet,
				aClippingAnimation,
				DirectionType.Forward,
			);
		}

		case TransitionClass.Special:
			switch (eTransitionType) {
				// map SLIDEWIPE to BARWIPE, for now
				case TransitionType.SLIDEWIPE: {
					let subtype = TransitionSubType.DEFAULT;
					let isForwardDirection = true;
					switch (eTransitionSubType) {
						case TransitionSubType.FROMLEFT:
							subtype = TransitionSubType.LEFTTORIGHT;
							isForwardDirection = true;
							break;
						case TransitionSubType.FROMRIGHT:
							subtype = TransitionSubType.LEFTTORIGHT;
							isForwardDirection = false;
							break;
						case TransitionSubType.FROMTOP:
							subtype = TransitionSubType.TOPTOBOTTOM;
							isForwardDirection = true;
							break;
						case TransitionSubType.FROMBOTTOM:
							subtype = TransitionSubType.TOPTOBOTTOM;
							isForwardDirection = false;
							break;
						default:
							window.app.console.log(
								'createShapeTransition: unexpected subtype for SLIDEWIPE',
							);
							break;
					}

					transitionFilterInfo.transitionType = TransitionType.BARWIPE;
					transitionFilterInfo.transitionSubtype = subtype;
					transitionFilterInfo.isDirectionForward = isForwardDirection;

					const aClippingAnimation = new TransitionFilterAnimation(
						aAnimatedTransitionFilterNode.getId(),
						transitionFilterInfo,
						aAnimatedElement,
					);

					return new SimpleActivity(
						aActivityParamSet,
						aClippingAnimation,
						DirectionType.Forward,
					);
				}
				// we map everything else to crossfade
				default: {
					return createCrossFadeTransition(
						aActivityParamSet,
						aAnimatedElement,
						nSlideWidth,
						nSlideHeight,
						bModeIn,
					);
				}
			}
	}
}

function createCrossFadeTransition(
	aActivityParamSet: ActivityParamSet,
	aAnimatedElement: AnimatedElement,
	nSlideWidth: number,
	nSlideHeight: number,
	bModeIn: boolean,
): AnimationActivity {
	const aAnimation = createPropertyAnimation(
		'opacity',
		aAnimatedElement,
		nSlideWidth,
		nSlideHeight,
	);
	const eDirection = bModeIn ? DirectionType.Forward : DirectionType.Backward;
	return new SimpleActivity(aActivityParamSet, aAnimation, eDirection);
}
