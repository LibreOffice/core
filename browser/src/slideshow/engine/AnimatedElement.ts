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

type BoundingBoxType = DOMRect;

type AnimatedObjectType = ImageBitmap;

const NSS_SVG = 'http://www.w3.org/2000/svg';

enum PropertyValueType {
	Unknown,
	Number,
	Enum,
	Color,
	String,
	Bool,
	TupleNumber,
}

namespace PropertyOperators {
	const GenericPropOpSet = {
		equal: (a: any, b: any) => {
			return a === b;
		},
		add: (a: any, b: any) => {
			return a;
		},
		scale: (k: any, v: any) => {
			return v;
		},
	};
	const NumberPropOpSet = {
		equal: (a: number, b: number) => {
			return a === b;
		},
		add: (a: number, b: number) => {
			return a + b;
		},
		scale: (k: number, v: number) => {
			return k * v;
		},
	};
	const TupleNumberPropOpSet = {
		equal: (a: number[], b: number[]) => {
			assert(a.length === b.length, 'Tuples length mismatch.');
			return a.toString() === b.toString();
		},
		add: (a: number[], b: number[]) => {
			assert(a.length === b.length, 'Tuples length mismatch.');
			const r: number[] = [];
			for (let i = 0; i < a.length; ++i) {
				r.push(a[i] + b[i]);
			}
			return r;
		},
		scale: (k: number, v: number[]) => {
			const r: number[] = [];
			for (let i = 0; i < v.length; ++i) {
				r.push(k * v[i]);
			}
			return r;
		},
	};
	const ColorPropOpSet = {
		equal: (a: any, b: any): boolean => {
			return a.equal(b);
		},
		add: (a: any, b: any): any => {
			const c = a.clone();
			c.add(b);
			return c;
		},
		scale: (k: number, v: any): any => {
			const c = v.clone();
			c.scale(k);
			return c;
		},
	};

	export type PropOpSet = typeof GenericPropOpSet;
	export type PropertyOperatorsMap = Map<PropertyValueType, PropOpSet>;
	export const aOperatorSetMap: PropertyOperatorsMap = new Map();
	aOperatorSetMap.set(PropertyValueType.Number, NumberPropOpSet);
	aOperatorSetMap.set(PropertyValueType.Enum, GenericPropOpSet);
	aOperatorSetMap.set(PropertyValueType.String, GenericPropOpSet);
	aOperatorSetMap.set(PropertyValueType.Bool, GenericPropOpSet);
	aOperatorSetMap.set(PropertyValueType.TupleNumber, TupleNumberPropOpSet);
	aOperatorSetMap.set(PropertyValueType.Color, ColorPropOpSet);
}

type PropertyOperatorSet = PropertyOperators.PropOpSet;
type PropertyOperatorsMap = PropertyOperators.PropertyOperatorsMap;
const aOperatorSetMap = PropertyOperators.aOperatorSetMap;

type PropertyInterpolatorType = (nFrom: any, nTo: any, nT: number) => any;

namespace PropertyInterpolator {
	export function getInterpolator(
		eValueType: PropertyValueType,
		eValueSubType?: ColorSpace,
		eClockDirection?: ClockDirection,
	): PropertyInterpolatorType {
		switch (eValueType) {
			case PropertyValueType.Number:
				return (nFrom: any, nTo: any, nT: number): any => {
					return (1.0 - nT) * nFrom + nT * nTo;
				};
			case PropertyValueType.TupleNumber:
				return (aFrom: any, aTo: any, nT: number): any => {
					const aRes = [];
					for (let i = 0; i < aFrom.length; ++i) {
						aRes.push((1.0 - nT) * aFrom[i] + nT * aTo[i]);
					}
					return aRes as any;
				};
			case PropertyValueType.Color:
				if (eValueSubType !== undefined) {
					switch (eValueSubType) {
						case ColorSpace.rgb:
							return (nFrom: any, nTo: any, nT: number): any => {
								return RGBColor.interpolate(nFrom, nTo, nT);
							};
						case ColorSpace.hsl: {
							const bCCW = eClockDirection == ClockDirection.counterClockwise;
							return (nFrom: any, nTo: any, nT: number): any => {
								return HSLColor.interpolate(nFrom, nTo, nT, bCCW);
							};
						}
					}
				} else {
					window.app.console.log(
						'PropertyInterpolator.getInterpolator: color space not defined.',
					);
				}
				break;
			default:
				window.app.console.log(
					'PropertyInterpolator.getInterpolator: not found any valid interpolator for value type ' +
						PropertyValueType[eValueType],
				);
		}
		return null;
	}
}

interface PropertyGetterSetter {
	type: PropertyValueType;
	get: string;
	set: string;
	getmod?: (nWidth: number, nHeight: number) => (v: number) => number;
	setmod?: (nWidth: number, nHeight: number) => (v: number) => number;
}

const aPropertyGetterSetterMap = {
	height: {
		type: PropertyValueType.Number,
		get: 'getHeight',
		set: 'setHeight',
		getmod: (nWidth: number, nHeight: number) => makeScaler(1 / nHeight),
		setmod: (nWidth: number, nHeight: number) => makeScaler(nHeight),
	},

	opacity: {
		type: PropertyValueType.Number,
		get: 'getOpacity',
		set: 'setOpacity',
	},

	scale: {
		type: PropertyValueType.TupleNumber,
		get: 'getSize',
		set: 'setSize',
	},

	translate: {
		type: PropertyValueType.TupleNumber,
		get: 'getPos',
		set: 'setPos',
	},

	rotate: {
		type: PropertyValueType.Number,
		get: 'getRotationAngle',
		set: 'setRotationAngle',
	},

	skewx: {
		type: PropertyValueType.Number,
		get: 'getSkewX',
		set: 'setSkewX',
	},

	skewy: {
		type: PropertyValueType.Number,
		get: 'getSkewY',
		set: 'setSkewY',
	},

	width: {
		type: PropertyValueType.Number,
		get: 'getWidth',
		set: 'setWidth',
		getmod: (nWidth: number, nHeight: number) => makeScaler(1 / nWidth),
		setmod: (nWidth: number, nHeight: number) => makeScaler(nWidth),
	},

	x: {
		type: PropertyValueType.Number,
		get: 'getX',
		set: 'setX',
		getmod: (nWidth: number, nHeight: number) => makeScaler(1 / nWidth),
		setmod: (nWidth: number, nHeight: number) => makeScaler(nWidth),
	},

	y: {
		type: PropertyValueType.Number,
		get: 'getY',
		set: 'setY',
		getmod: (nWidth: number, nHeight: number) => makeScaler(1 / nHeight),
		setmod: (nWidth: number, nHeight: number) => makeScaler(nHeight),
	},

	visibility: {
		type: PropertyValueType.Enum,
		get: 'getVisibility',
		set: 'setVisibility',
	},

	fillstyle: {
		type: PropertyValueType.String,
		get: 'getFillStyle',
		set: 'setFillStyle',
	},

	linestyle: {
		type: PropertyValueType.String,
		get: 'getLineStyle',
		set: 'setLineStyle',
	},

	fillcolor: {
		type: PropertyValueType.Color,
		get: 'getFillColor',
		set: 'setFillColor',
	},

	linecolor: {
		type: PropertyValueType.Color,
		get: 'getStrokeColor',
		set: 'setStrokeColor',
	},

	charcolor: {
		type: PropertyValueType.Color,
		get: 'getFontColor',
		set: 'setFontColor',
	},

	dimcolor: {
		type: PropertyValueType.Color,
		get: 'getDimColor',
		set: 'setDimColor',
	},
};

type PropertyGetterSetterMapKeyType = keyof typeof aPropertyGetterSetterMap;

interface TransitionFilterFrameInfo {
	animation: TransitionFilterAnimation;
	time: number;
}

interface TransitionFiltersState {
	filterQueue: Array<number>;
	frameInfoMap: Map<number, TransitionFilterFrameInfo>;
}

class TransitionFiltersManager {
	private filterQueue = new Array<number>();
	private frameInfoMap = new Map<number, TransitionFilterFrameInfo>();

	public isEmpty() {
		return this.filterQueue.length === 0;
	}

	public add(animation: TransitionFilterAnimation, time: number) {
		this.frameInfoMap.set(animation.getNodeId(), {
			animation: animation,
			time: time,
		});
		this.updateQueue(animation.getNodeId());
	}

	private updateQueue(nodeId: number) {
		const length = this.filterQueue.length;
		const last = this.filterQueue[length];
		if (nodeId === last) return;
		const index = this.filterQueue.indexOf(nodeId);
		if (index > -1) {
			// remove it
			this.filterQueue.splice(index, 1);
		}
		this.filterQueue.push(nodeId);
	}

	public apply(properties?: AnimatedElementRenderProperties): boolean {
		let applied = false;
		this.filterQueue.forEach((nodeId) => {
			const frameInfo = this.frameInfoMap.get(nodeId);
			if (frameInfo) {
				applied = true;
				ANIMDBG.print(`TransitionFiltersManager.apply: node: ${nodeId}`);
				frameInfo.animation.renderFrame(frameInfo.time, properties);
			}
		});
		return applied;
	}

	/// The last performed frame of a transition filter, or undefined.
	public getLastFrame(): TransitionFilterFrameInfo | undefined {
		const nodeId = this.filterQueue[this.filterQueue.length - 1];
		return nodeId === undefined ? undefined : this.frameInfoMap.get(nodeId);
	}

	public clear() {
		this.frameInfoMap.forEach((frameInfo) => {
			frameInfo.animation.notifySlideEnd();
		});
		this.filterQueue = [];
		this.frameInfoMap.clear();
	}

	public getState(): TransitionFiltersState {
		return {
			filterQueue: structuredClone(this.filterQueue),
			frameInfoMap: new Map(this.frameInfoMap),
		};
	}

	public setState(state: TransitionFiltersState) {
		this.filterQueue = structuredClone(state.filterQueue);
		this.frameInfoMap = new Map(state.frameInfoMap);
	}
}

interface AnimatedElementState {
	nCenterX: number;
	nCenterY: number;
	nScaleFactorX: number;
	nScaleFactorY: number;
	nRotationAngle: number;
	nSkewX: number;
	nSkewY: number;
	aTMatrix: DOMMatrix;
	nOpacity: number;
	aFillColor: RGBColor;
	aLineColor: RGBColor;
	aFontColor: RGBColor;
	bVisible: boolean;
	transitionFiltersState: TransitionFiltersState;
}

type BoundsType = [DOMPoint, DOMPoint, DOMPoint, DOMPoint];

interface AnimatedElementColorMap {
	fromFillColor?: RGBColor;
	toFillColor?: RGBColor;
	fromLineColor?: RGBColor;
	toLineColor?: RGBColor;
}
interface AnimatedElementRenderProperties {
	bounds: BoundsType;
	alpha: number;
	colorMap?: AnimatedElementColorMap;
}

function BBoxToString(aBB: BoundingBoxType) {
	return `{ x: ${aBB.x}, y: ${aBB.y}, width: ${aBB.width}, height: ${aBB.height} }`;
}

class AnimatedElement {
	public static readonly DefaultColor = new RGBColor(0, 0, 0);

	public static readonly SupportedProperties = new Set<string>([
		'x',
		'y',
		'width',
		'height',
		'opacity',
		'visibility',
		'rotate',
		'skewx',
		'skewy',
		'fillstyle',
		'linestyle',
		'fillcolor',
		'linecolor',
		'charcolor',
		'dimcolor',
	]);

	public static readonly SupportedTransformations = new Set<string>([
		'translate',
		'scale',
		'rotate',
	]);

	protected sId: string;
	protected slideHash: string;
	private _title: string;
	protected slideWidth: number;
	protected slideHeight: number;
	private canvasWidth: number;
	private canvasHeight: number;
	protected bIsValid: boolean;
	private slideRenderer: SlideRenderer;
	protected canvasScaleFactor: { x: number; y: number };
	protected aBaseBBox: BoundingBoxType = null;
	protected nBaseCenterX: number = 0;
	protected nBaseCenterY: number = 0;
	private aActiveBBox: BoundingBoxType;
	private eAdditiveMode = AdditiveMode.Replace;
	private aStateSet = new Map<number, AnimatedElementState>();
	private aStateOnNextEffectSet = new Map<number, AnimatedElementState>();
	private nCenterX: number;
	private nCenterY: number;
	private nScaleFactorX: number = 1.0;
	private nScaleFactorY: number = 1.0;
	private nRotationAngle: number = 0;
	private nSkewX: number = 0;
	private nSkewY: number = 0;
	private aTMatrix: DOMMatrix;
	private nOpacity: number; // [0, 1]
	protected aBaseFillColor = AnimatedElement.DefaultColor;
	private aFillColor = AnimatedElement.DefaultColor;
	protected aBaseLineColor = AnimatedElement.DefaultColor;
	private aLineColor = AnimatedElement.DefaultColor;
	protected aBaseFontColor = AnimatedElement.DefaultColor;
	private aFontColor = AnimatedElement.DefaultColor;
	private aDimColor = AnimatedElement.DefaultColor;
	private bVisible: boolean;
	// Hidden at slide start where the first effect brings the target in.
	private bInitiallyVisible: boolean = true;
	private aClipPath: SVGPathElement = null;
	private runningAnimations: number = 0;

	private nCurrentEffect: number;
	// effect -> animation nodes
	private activeAnimationNodes = new Map<number, Set<number>>();

	protected transitionFiltersManager: TransitionFiltersManager;
	protected aSlideShowContext: SlideShowContext;

	constructor(
		sId: string,
		slideHash: string,
		title: string,
		slideWidth: number,
		slideHeight: number,
	) {
		ANIMDBG.print(`AnimatedElement(${sId}, ${slideHash})`);
		this.sId = sId;
		this.slideHash = slideHash;
		this._title = title;
		this.slideWidth = slideWidth;
		this.slideHeight = slideHeight;
		this.bIsValid = false;

		this.transitionFiltersManager = new TransitionFiltersManager();

		const presenter: SlideShowPresenter = app.map.slideShowPresenter;
		this.slideRenderer = presenter._slideRenderer;
	}

	getId() {
		return this.sId;
	}

	public isValid() {
		return this.bIsValid;
	}

	public getTitle() {
		return this._title;
	}

	public isTextElement() {
		return false;
	}

	protected cloneBBox(aBBox: BoundingBoxType): BoundingBoxType {
		if (!aBBox) return null;
		return new DOMRect(aBBox.x, aBBox.y, aBBox.width, aBBox.height);
	}

	// The rectangle of the target on the slide, in twips. Every transform is
	// relative to it.
	protected setBaseBBox(aBounds: BoundingBoxType) {
		this.aBaseBBox = this.cloneBBox(aBounds);
		({ x: this.nBaseCenterX, y: this.nBaseCenterY } = getRectCenter(
			this.aBaseBBox,
		));
	}

	public updateCanvasSize(size: [number, number]) {
		this.canvasWidth = size[0];
		this.canvasHeight = size[1];
		this.canvasScaleFactor = {
			x: this.canvasWidth / this.slideWidth,
			y: this.canvasHeight / this.slideHeight,
		};
		app.console.debug(
			`AnimatedElement.updateCanvasSize: (${this.canvasWidth}x${this.canvasHeight}), scale factor: ${this.canvasScaleFactor}`,
		);
	}

	// Fetch the base rectangle, base colors and content at slide start. The
	// base has no content and keeps a placeholder rectangle.
	protected updateContentInfo() {
		this.aBaseBBox = new DOMRect(0, 0, 1920, 1080);
		window.app.console.log(
			'AnimatedElement.updateContentInfo: no content available',
		);
	}

	// Fetch the content again for an element that had none at slide start.
	// The state starts over from the new base. True when there is content.
	public refreshContent(): boolean {
		if (this.bIsValid) return true;
		this.updateContentInfo();
		if (!this.bIsValid) return false;
		this.resetProperties();
		return true;
	}

	public setInitialVisibility(bVisible: boolean) {
		this.bInitiallyVisible = bVisible;
	}

	// The visibility at slide start.
	protected getInitialVisibility(): boolean {
		return this.bInitiallyVisible;
	}

	// Take the content from a newly delivered shape layer. The base has none.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public updateAnimationInfo(_animatedLayerInfo: AnimatedShapeInfo) {}

	// Draw the current state into the layer compose. The base draws nothing.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public renderLayer(_renderer: LayerRenderer) {}

	// Give a shape transition filter its rendering context and texture. The
	// base has neither.
	// eslint-disable-next-line @typescript-eslint/no-empty-function
	public setTransitionParameters(_transitionParameters: TransitionParameters) {}

	private resetProperties() {
		this.nCenterX = this.nBaseCenterX;
		this.nCenterY = this.nBaseCenterY;
		this.nScaleFactorX = 1.0;
		this.nScaleFactorY = 1.0;
		this.nRotationAngle = 0.0;
		this.nSkewX = 0.0;
		this.nSkewY = 0.0;
		this.aTMatrix = new DOMMatrix([1, 0, 0, 1, 0, 0]);
		this.nOpacity = 1;
		this.aFillColor = this.aBaseFillColor;
		this.aLineColor = this.aBaseLineColor;
		this.aFontColor = this.aBaseFontColor;
		this.bVisible = this.getInitialVisibility();
		this.transitionFiltersManager.clear();
		// this.aActiveBBox = this.cloneBBox(this.aBaseBBox);
	}

	initClipPath() {
		this.aClipPath = document.createElementNS(NSS_SVG, 'path');

		const nWidth = this.aActiveBBox.width;
		const nHeight = this.aActiveBBox.height;
		const sPathData =
			'M ' +
			this.aActiveBBox.x +
			' ' +
			this.aActiveBBox.y +
			' h ' +
			nWidth +
			' v ' +
			nHeight +
			' h -' +
			nWidth +
			' z';
		this.aClipPath.setAttribute('d', sPathData);
	}

	cleanClipPath() {
		this.aClipPath = null;
	}

	// The current state for a renderer: the transformed base rectangle in
	// slide-relative unit coordinates, the opacity and the color map.
	public getRenderProperties(): AnimatedElementRenderProperties {
		const T = this.aTMatrix;

		const tl = new DOMPoint(this.aBaseBBox.x, this.aBaseBBox.y);
		const br = new DOMPoint(
			tl.x + this.aBaseBBox.width,
			tl.y + this.aBaseBBox.height,
		);
		const bl = new DOMPoint(tl.x, br.y);
		const tr = new DOMPoint(br.x, tl.y);

		const unitBounds = [];
		for (const v of [bl, br, tl, tr]) {
			const u = v.matrixTransform(T);
			u.x = u.x / this.slideWidth;
			u.y = u.y / this.slideHeight;
			unitBounds.push(u);
		}

		return {
			bounds: unitBounds as BoundsType,
			alpha: this.nOpacity,
			colorMap: this.getColorMap(),
		};
	}

	// The colors a color animation changed, as pairs of base color and
	// current color. Empty when none changed.
	public getColorMap(): AnimatedElementColorMap {
		// This color mapping solution doesn't work if fill and line colors have
		// the same initial value. For a proper solution we would need separated
		// layers for stroke and fill, at least when a color animation is involved.
		const colorMap: AnimatedElementColorMap = {};
		if (this.isTextElement()) {
			// On desktop, fill and line color animations seem to have no effect
			// when applied to a paragraph, so here we take into account font color
			// changes only, and we forward them to the renderer as a fill color map,
			// so that we can avoid to perform a further color comparison
			// in the fragment shader.
			if (!this.aFontColor.equal(this.aBaseFontColor)) {
				colorMap.fromFillColor = this.aBaseFontColor;
				colorMap.toFillColor = this.aFontColor;
			}
		} else if (
			!this.aFillColor.equal(this.aBaseFillColor) ||
			!this.aLineColor.equal(this.aBaseLineColor)
		) {
			colorMap.fromFillColor = this.aBaseFillColor;
			colorMap.toFillColor = this.aFillColor;
			colorMap.fromLineColor = this.aBaseLineColor;
			colorMap.toLineColor = this.aLineColor;
		}
		return colorMap;
	}

	notifySlideStart(aSlideShowContext: SlideShowContext) {
		if (!aSlideShowContext) {
			window.app.console.log(
				'AnimatedElement.notifySlideStart: slideshow context is not valid',
			);
		}
		this.aSlideShowContext = aSlideShowContext;

		this.runningAnimations = 0;
		this.activeAnimationNodes.clear();
		this.nCurrentEffect = undefined;
		this.aStateOnNextEffectSet.clear();

		this.updateContentInfo();

		this.resetProperties();
		this.DBG('.notifySlideStart invoked');
	}

	notifySlideEnd() {
		this.transitionFiltersManager.clear();
	}

	notifyAnimationStart() {
		if (!this.isActive()) {
			this.slideRenderer.notifyAnimationStarted(this.sId);
		}
		++this.runningAnimations;
	}

	isActive(): boolean {
		return this.runningAnimations > 0;
	}

	notifyAnimationEnd() {
		if (this.runningAnimations <= 0) {
			window.app.console.log(
				'AnimatedElement.notifyAnimationEnd: running animations <= 0',
			);
			return;
		}
		--this.runningAnimations;
		if (!this.isActive()) {
			this.slideRenderer.notifyAnimationEnded(this.sId);
		}
	}

	notifyNextEffectStart(nEffectIndex: number) {
		this.nCurrentEffect = nEffectIndex;
		this.activeAnimationNodes.set(this.nCurrentEffect, new Set());
	}

	/** saveState
	 *  Save the state of the managed animated element and append it to aStateSet
	 *  using the passed animation node id as key.
	 *
	 *  @param nAnimationNodeId
	 *      A non negative integer representing the unique id of an animation node.
	 */
	saveState(nAnimationNodeId: number) {
		ANIMDBG.print(
			'AnimatedElement(' +
				this.getId() +
				').saveState(' +
				nAnimationNodeId +
				')',
		);
		const aState: AnimatedElementState = {
			nCenterX: this.nCenterX,
			nCenterY: this.nCenterY,
			nScaleFactorX: this.nScaleFactorX,
			nScaleFactorY: this.nScaleFactorY,
			nRotationAngle: this.nRotationAngle,
			nSkewX: this.nSkewX,
			nSkewY: this.nSkewY,
			aTMatrix: DOMMatrix.fromMatrix(this.aTMatrix),
			nOpacity: this.nOpacity,
			aFillColor: this.aFillColor,
			aLineColor: this.aLineColor,
			aFontColor: this.aFontColor,
			bVisible: this.bVisible,
			transitionFiltersState: this.transitionFiltersManager.getState(),
		};
		this.aStateSet.set(nAnimationNodeId, aState);

		// if it's the first animation for current effect set as the state
		// on effect started
		const currentEffectNodes = this.activeAnimationNodes.get(
			this.nCurrentEffect,
		);
		if (currentEffectNodes) {
			currentEffectNodes.add(nAnimationNodeId);
			if (currentEffectNodes.size === 1)
				this.aStateOnNextEffectSet.set(this.nCurrentEffect, aState);
		} else {
			window.app.console.log(
				`AnimatedElement(${this.getId()}).saveState(${nAnimationNodeId}): ` +
					`current effect not found: ${this.nCurrentEffect}`,
			);
		}
	}

	/** restoreState
	 *  Restore the state of the managed animated element to the state with key
	 *  the passed animation node id.
	 *
	 *  @param nAnimationNodeId
	 *      A non negative integer representing the unique id of an animation node.
	 *
	 *  @return
	 *      True if the restoring operation is successful, false otherwise.
	 */
	restoreState(nAnimationNodeId: number) {
		if (!this.aStateSet.has(nAnimationNodeId)) {
			window.app.console.log(
				'AnimatedElement(' +
					this.getId() +
					').restoreState: state ' +
					nAnimationNodeId +
					' is not valid',
			);
			return false;
		}

		ANIMDBG.print(
			'AnimatedElement(' +
				this.getId() +
				').restoreState(' +
				nAnimationNodeId +
				')',
		);

		let aState = this.aStateSet.get(nAnimationNodeId);

		this.activeAnimationNodes.forEach((nodes, effect, map) => {
			if (nodes.has(nAnimationNodeId)) {
				nodes.delete(nAnimationNodeId);
				// if an effect has been completely removed restore the state saved
				// when effect started
				if (nodes.size === 0) {
					map.delete(effect);
					const state = this.aStateOnNextEffectSet.get(effect);
					if (state) aState = state;
					this.aStateOnNextEffectSet.delete(effect);
				}
				return;
			}
		});

		const bRet = true;
		if (bRet) {
			this.nCenterX = aState.nCenterX;
			this.nCenterY = aState.nCenterY;
			this.nScaleFactorX = aState.nScaleFactorX;
			this.nScaleFactorY = aState.nScaleFactorY;
			this.nRotationAngle = aState.nRotationAngle;
			this.nSkewX = aState.nSkewX;
			this.nSkewY = aState.nSkewY;
			this.aTMatrix = DOMMatrix.fromMatrix(aState.aTMatrix);
			this.nOpacity = aState.nOpacity;
			this.aFillColor = aState.aFillColor;
			this.aLineColor = aState.aLineColor;
			this.aFontColor = aState.aFontColor;
			this.bVisible = aState.bVisible;
			this.transitionFiltersManager.setState(aState.transitionFiltersState);
		}
		this.aStateSet.delete(nAnimationNodeId);

		// we need to trigger at least one request animation frame for SlideRenderer.render method
		// maybe we could implement a SlideRenderer.oneShot method.
		this.notifyAnimationStart();
		this.notifyAnimationEnd();

		return bRet;
	}

	getBaseBBox() {
		return this.aBaseBBox;
	}

	// The current transform, in slide coordinates, about the base center.
	getTransformMatrix(): DOMMatrix {
		return this.aTMatrix;
	}

	getBaseCenterX() {
		return this.nBaseCenterX;
	}

	getBaseCenterY() {
		return this.nBaseCenterY;
	}

	applyTransform() {
		// empty body
	}

	getAdditiveMode() {
		return this.eAdditiveMode;
	}

	setAdditiveMode(eAdditiveMode: AdditiveMode) {
		this.eAdditiveMode = eAdditiveMode;
	}

	private updateTransformationMatrix() {
		const aTMatrix = new DOMMatrix([1, 0, 0, 1, 0, 0]);
		const skewXAngle = (180 * Math.atan(this.nSkewX)) / Math.PI;
		const skewYAngle = (180 * Math.atan(this.nSkewY)) / Math.PI;
		aTMatrix
			.translateSelf(this.nCenterX, this.nCenterY)
			.rotateSelf(this.nRotationAngle)
			.scaleSelf(this.nScaleFactorX, this.nScaleFactorY)
			.skewXSelf(skewYAngle)
			.skewXSelf(skewXAngle)
			.translateSelf(-this.nBaseCenterX, -this.nBaseCenterY);
		this.aTMatrix = aTMatrix;
	}

	// Get/Set Properties
	getX() {
		return this.nCenterX;
	}

	getY() {
		return this.nCenterY;
	}

	getPos() {
		return [this.getX(), this.getY()];
	}

	getWidth() {
		return this.nScaleFactorX * this.getBaseBBox().width;
	}

	getHeight() {
		return this.nScaleFactorY * this.getBaseBBox().height;
	}

	getSize() {
		return [this.getWidth(), this.getHeight()];
	}

	setX(nNewCenterX: number) {
		ANIMDBG.print('AnimatedElement.setX(' + nNewCenterX + ')');
		if (nNewCenterX === this.nCenterX) return;

		this.nCenterX = nNewCenterX;
		this.updateTransformationMatrix();
	}

	setY(nNewCenterY: number) {
		ANIMDBG.print('AnimatedElement.setY(' + nNewCenterY + ')');
		if (nNewCenterY === this.nCenterY) return;

		this.nCenterY = nNewCenterY;
		this.updateTransformationMatrix();
	}

	setPos(aNewPos: [number, number]) {
		const nNewCenterX = aNewPos[0];
		const nNewCenterY = aNewPos[1];
		ANIMDBG.print(
			'AnimatedElement.setPos(' + nNewCenterX + ', ' + nNewCenterY + ')',
		);

		if (nNewCenterX === this.nCenterX && nNewCenterY === this.nCenterY) return;

		this.nCenterX = nNewCenterX;
		this.nCenterY = nNewCenterY;
		this.updateTransformationMatrix();
	}

	setWidth(nNewWidth: number) {
		ANIMDBG.print('AnimatedElement.setWidth(' + nNewWidth + ')');

		const nBaseWidth = this.getBaseBBox().width;
		let nScaleFactorX = nNewWidth / nBaseWidth;

		if (Math.abs(nScaleFactorX) < 1e-5)
			nScaleFactorX = Math.sign(nScaleFactorX) * 1e-5;
		if (nScaleFactorX == this.nScaleFactorX) return;

		this.nScaleFactorX = nScaleFactorX;
		this.updateTransformationMatrix();
	}

	setHeight(nNewHeight: number) {
		ANIMDBG.print('AnimatedElement.setHeight(' + nNewHeight + ')');

		const nBaseHeight = this.getBaseBBox().height;
		let nScaleFactorY = nNewHeight / nBaseHeight;

		if (Math.abs(nScaleFactorY) < 1e-5)
			nScaleFactorY = Math.sign(nScaleFactorY) * 1e-5;
		if (nScaleFactorY == this.nScaleFactorY) return;

		this.nScaleFactorY = nScaleFactorY;
		this.updateTransformationMatrix();
	}

	setSize(aNewSize: [number, number]) {
		const nNewWidth = aNewSize[0];
		const nNewHeight = aNewSize[1];
		ANIMDBG.print(
			'AnimatedElement.setSize(' + nNewWidth + ', ' + nNewHeight + ')',
		);

		const nBaseWidth = this.getBaseBBox().width;
		let nScaleFactorX = nNewWidth / nBaseWidth;
		if (Math.abs(nScaleFactorX) < 1e-5)
			nScaleFactorX = Math.sign(nScaleFactorX) * 1e-5;

		const nBaseHeight = this.getBaseBBox().height;
		let nScaleFactorY = nNewHeight / nBaseHeight;
		if (Math.abs(nScaleFactorY) < 1e-5)
			nScaleFactorY = Math.sign(nScaleFactorY) * 1e-5;

		if (
			nScaleFactorX == this.nScaleFactorX &&
			nScaleFactorY == this.nScaleFactorY
		)
			return;

		this.nScaleFactorX = nScaleFactorX;
		this.nScaleFactorY = nScaleFactorY;
		this.updateTransformationMatrix();
	}

	getOpacity() {
		return this.nOpacity;
	}

	setOpacity(nValue: number) {
		this.nOpacity = clampN(nValue, 0, 1);
		ANIMDBG.print('AnimatedElement.setOpacity(' + nValue + ')');
	}

	getRotationAngle() {
		return this.nRotationAngle;
	}

	setRotationAngle(nNewRotAngle: number) {
		this.nRotationAngle = nNewRotAngle;
		this.updateTransformationMatrix();
		ANIMDBG.print('AnimatedElement.setRotationAngle(' + nNewRotAngle + ')');
	}

	getSkewX() {
		return this.nSkewX;
	}

	setSkewX(nSkewValue: number) {
		this.nSkewX = nSkewValue;
		this.updateTransformationMatrix();
		ANIMDBG.print('AnimatedElement.setSkewX(' + nSkewValue + ')');
	}

	getSkewY() {
		return this.nSkewY;
	}

	setSkewY(nSkewValue: number) {
		this.nSkewY = nSkewValue;
		this.updateTransformationMatrix();
		ANIMDBG.print('AnimatedElement.setSkewY(' + nSkewValue + ')');
	}

	getVisibility(): boolean {
		return this.bVisible;
	}

	setVisibility(sValue: string) {
		this.bVisible = sValue === 'visible';
		app.console.debug('AnimatedElement.setVisibility(' + sValue + ')');
	}

	getFillColor(): RGBColor {
		return this.aFillColor;
	}

	setFillColor(aRGBValue: RGBColor) {
		window.app.console.log(
			'AnimatedElement.setFillColor(' + aRGBValue.toString() + ')',
		);
		this.aFillColor = aRGBValue;
	}

	getStrokeColor(): RGBColor {
		return this.aLineColor;
	}

	setStrokeColor(aRGBValue: RGBColor) {
		window.app.console.log(
			'AnimatedElement.setStrokeColor(' + aRGBValue.toString() + ')',
		);
		this.aLineColor = aRGBValue;
	}

	getFontColor(): RGBColor {
		return this.aFontColor;
	}

	setFontColor(aRGBValue: RGBColor) {
		window.app.console.log(
			'AnimatedElement.setFontColor(' + aRGBValue.toString() + ')',
		);
		this.aFontColor = aRGBValue;
	}

	setDefaultDimColor(aRGBValue: RGBColor) {
		window.app.console.log(
			'AnimatedElement.setDefaultDimColor(' + aRGBValue.toString() + ')',
		);

		this.aDimColor = aRGBValue;
	}

	getDimColor(): RGBColor {
		return this.aDimColor;
	}

	setDimColor(aRGBValue: RGBColor) {
		window.app.console.log(
			'AnimatedElement.setDimColor(' + aRGBValue.toString() + ')',
		);
		this.aDimColor = aRGBValue;
		this.aLineColor = aRGBValue;
		this.aFillColor = aRGBValue;
		this.aFontColor = aRGBValue;
	}

	getFillStyle(): string {
		return 'solid';
	}

	setFillStyle(sValue: string) {
		window.app.console.log(
			'AnimatedElement.setFillStyle(' + sValue + ') not implemented',
		);
	}

	getLineStyle(): string {
		return 'solid';
	}

	setLineStyle(sValue: string) {
		window.app.console.log(
			'AnimatedElement.setLineStyle(' + sValue + ') not implemented',
		);
	}

	setClipPath(aClipPath: SVGPathElement) {
		// TODO: maybe we can reuse the path for creating a texture mask
		if (this.aClipPath) {
			// We need to translate the clip path to the top left corner of
			// the element bounding box.
			// var aTranslation = SVGIdentityMatrix.translate( this.aActiveBBox.x,
			// 	this.aActiveBBox.y);
			// aClipPath.matrixTransform( aTranslation );
			const sPathData = aClipPath.getAttribute('d');
			this.aClipPath.setAttribute('d', sPathData);
		}
	}

	setTransitionFilterFrame(
		aTransitionFilterAnimation: TransitionFilterAnimation,
		nT: number,
	) {
		this.transitionFiltersManager.add(aTransitionFilterAnimation, nT);
	}

	/// The last transition filter on the element and the time it reached,
	/// 0 to 1, or undefined.
	getTransitionFilterFrame():
		| { info: TransitionFilterInfo; time: number }
		| undefined {
		const frame = this.transitionFiltersManager.getLastFrame();
		if (!frame) return undefined;
		return {
			info: frame.animation.getTransitionFilterInfo(),
			time: frame.time,
		};
	}

	DBG(sMessage: string, nTime?: number) {
		aAnimatedElementDebugPrinter.print(
			'AnimatedElement(' + this.getId() + ')' + sMessage,
			nTime,
		);
	}
}
