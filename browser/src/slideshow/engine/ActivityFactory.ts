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

// Evaluate an anim:formula expression per the grammar in ODF 1.2 section 19.6.
const animFormulaFns: Record<string, (...a: number[]) => number> =
	Object.assign(Object.create(null), {
		abs: Math.abs,
		sqrt: Math.sqrt,
		sin: Math.sin,
		cos: Math.cos,
		tan: Math.tan,
		asin: Math.asin,
		acos: Math.acos,
		atan: Math.atan,
		exp: Math.exp,
		log: Math.log,
		min: Math.min,
		max: Math.max,
	});

function evalAnimFormula(expr: string, vars: Record<string, number>): number {
	let p = 0;
	function ws() {
		while (p < expr.length && expr[p] === ' ') p++;
	}

	// number
	function number_literal(): number {
		const s = p;
		while (
			p < expr.length &&
			((expr[p] >= '0' && expr[p] <= '9') || expr[p] === '.')
		)
			p++;
		return parseFloat(expr.substring(s, p));
	}

	// identifier
	function read_identifier(): string {
		if (expr[p] === '$') {
			p++;
			return '$';
		}
		const s = p;
		while (p < expr.length && expr[p] >= 'a' && expr[p] <= 'z') p++;
		return expr.substring(s, p);
	}

	function identifier_value(id: string): number {
		if (id === '$') return vars['$'] || 0;
		if (id === 'pi') return Math.PI;
		if (id === 'e') return Math.E;
		if (id in vars) return vars[id];
		throw new Error('Unknown identifier in animation expression: ' + id);
	}

	function expect(ch: string): void {
		if (expr[p] !== ch)
			throw new Error(
				'Expected ' + ch + ' in animation expression at position ' + p,
			);
		p++;
	}

	// function '(' additive_expression ')'
	// binary_function '(' additive_expression ',' additive_expression ')'
	function function_call(name: string): number {
		expect('(');
		const a = additive_expression();
		ws();
		if (expr[p] === ',') {
			p++;
			const b = additive_expression();
			ws();
			expect(')');
			return animFormulaFns[name](a, b);
		}
		expect(')');
		return animFormulaFns[name](a);
	}

	function basic_expression(): number {
		ws();
		// '(' additive_expression ')'
		if (expr[p] === '(') {
			p++;
			const v = additive_expression();
			ws();
			expect(')');
			return v;
		}
		// number
		if ((expr[p] >= '0' && expr[p] <= '9') || expr[p] === '.')
			return number_literal();
		// function / binary_function / identifier
		const id = read_identifier();
		ws();
		if (id in animFormulaFns && expr[p] === '(') return function_call(id);
		return identifier_value(id);
	}

	// unary_expression
	function unary_expression(): number {
		ws();
		if (expr[p] === '-') {
			p++;
			return -basic_expression();
		}
		return basic_expression();
	}

	// multiplicative_expression
	function multiplicative_expression(): number {
		let v = unary_expression();
		ws();
		while (expr[p] === '*' || expr[p] === '/') {
			const op = expr[p++];
			v = op === '*' ? v * unary_expression() : v / unary_expression();
			ws();
		}
		return v;
	}

	// additive_expression
	function additive_expression(): number {
		let v = multiplicative_expression();
		ws();
		while (expr[p] === '+' || expr[p] === '-') {
			const op = expr[p++];
			v =
				op === '+'
					? v + multiplicative_expression()
					: v - multiplicative_expression();
			ws();
		}
		return v;
	}

	const result = additive_expression();
	ws();
	if (p < expr.length)
		throw new Error('Unexpected character in animation expression: ' + expr[p]);
	return result;
}

function createActivity(
	aActivityParamSet: ActivityParamSet,
	aAnimationNode: AnimationBaseNode3,
	aAnimation: AnimationBase,
	aInterpolator: PropertyInterpolatorType,
): AnimationActivity {
	const eCalcMode = aAnimationNode.getCalcMode();

	const sAttributeName = aAnimationNode.getAttributeName();
	const sKey = sAttributeName as PropertyGetterSetterMapKeyType;
	const aAttributeProp = aPropertyGetterSetterMap[sKey];

	const eValueType: PropertyValueType = aAttributeProp['type'];

	// do we need to get an interpolator ?
	if (!aInterpolator) {
		aInterpolator = PropertyInterpolator.getInterpolator(eValueType);
	}

	// is it cumulative ?
	const bAccumulate =
		aAnimationNode.getAccumulate() === AccumulateMode.Sum &&
		!(
			eValueType === PropertyValueType.Bool ||
			eValueType === PropertyValueType.String ||
			eValueType === PropertyValueType.Enum
		);

	if (aAnimationNode.getFormula()) {
		const sFormula: string = aAnimationNode.getFormula();
		const aAnimatedElement = aAnimationNode.getAnimatedElement();
		const aBBox = aAnimatedElement.getBaseBBox();
		const formulaVars: Record<string, number> = {
			width: aBBox.width / aActivityParamSet.nSlideWidth,
			height: aBBox.height / aActivityParamSet.nSlideHeight,
			x: (aBBox.x + aBBox.width / 2) / aActivityParamSet.nSlideWidth,
			y: (aBBox.y + aBBox.height / 2) / aActivityParamSet.nSlideHeight,
		};
		aActivityParamSet.aFormula = function (param: any) {
			formulaVars['$'] = param;
			return evalAnimFormula(sFormula, formulaVars);
		};
	}

	aActivityParamSet.aDiscreteTimes = aAnimationNode.getKeyTimes();

	// do we have a value list ?
	const aValueSet = aAnimationNode.getValues();
	const nValueSetSize = aValueSet.length;

	if (nValueSetSize != 0) {
		// Value list activity
		if (aActivityParamSet.aDiscreteTimes.length == 0) {
			for (let i = 0; i < nValueSetSize; ++i)
				aActivityParamSet.aDiscreteTimes.push(i / nValueSetSize);
		}

		switch (eCalcMode) {
			case CalcMode.Discrete:
				aActivityParamSet.aWakeupEvent = new WakeupEvent(
					aActivityParamSet.aTimerEventQueue.getTimer(),
					aActivityParamSet.aActivityQueue,
				);

				return createValueListActivity(
					aActivityParamSet,
					aAnimationNode,
					aAnimation,
					aInterpolator,
					DiscreteValueListActivity,
					bAccumulate,
					eValueType,
				);
			default:
				window.app.console.log(
					'createActivity: unexpected calculation mode: ' + CalcMode[eCalcMode],
				);
			// FALLTHROUGH intended
			case CalcMode.Paced:
			case CalcMode.Spline:
			case CalcMode.Linear:
				return createValueListActivity(
					aActivityParamSet,
					aAnimationNode,
					aAnimation,
					aInterpolator,
					LinearValueListActivity,
					bAccumulate,
					eValueType,
				);
		}
	} else {
		// FromToBy activity
		switch (eCalcMode) {
			case CalcMode.Discrete:
				aActivityParamSet.aWakeupEvent = new WakeupEvent(
					aActivityParamSet.aTimerEventQueue.getTimer(),
					aActivityParamSet.aActivityQueue,
				);
				return createFromToByActivity(
					aActivityParamSet,
					aAnimationNode,
					aAnimation,
					aInterpolator,
					DiscreteFromToByActivity,
					bAccumulate,
					eValueType,
				);

			default:
				window.app.console.log(
					'createActivity: unexpected calculation mode: ' + CalcMode[eCalcMode],
				);
			// FALLTHROUGH intended
			case CalcMode.Paced:
			case CalcMode.Spline:
			case CalcMode.Linear:
				return createFromToByActivity(
					aActivityParamSet,
					aAnimationNode,
					aAnimation,
					aInterpolator,
					LinearFromToByActivity,
					bAccumulate,
					eValueType,
				);
		}
	}
}

function createValueListActivity(
	aActivityParamSet: ActivityParamSet,
	aAnimationNode: AnimationBaseNode3,
	aAnimation: AnimationBase,
	aInterpolator: PropertyInterpolatorType,
	ValueListActivityCtor: ValueListActivityCtorType,
	bAccumulate: boolean,
	eValueType: PropertyValueType,
): AnimationActivity {
	const aAnimatedElement = aAnimationNode.getAnimatedElement();
	const aOperatorSet = aOperatorSetMap.get(eValueType);
	assert(aOperatorSet, 'createValueListActivity: no operator set found');

	const aValueSet = aAnimationNode.getValues();

	const aValueList: any[] = [];

	extractAttributeValues(
		eValueType,
		aValueList,
		aValueSet,
		aAnimatedElement.getBaseBBox(),
		aActivityParamSet.nSlideWidth,
		aActivityParamSet.nSlideHeight,
	);

	for (let i = 0; i < aValueList.length; ++i) {
		ANIMDBG.print(
			'createValueListActivity: value[' + i + '] = ' + aValueList[i],
		);
	}

	return new ValueListActivityCtor(
		aValueList,
		aActivityParamSet,
		aAnimation,
		aInterpolator,
		aOperatorSet,
		bAccumulate,
	);
}

function createFromToByActivity(
	aActivityParamSet: ActivityParamSet,
	aAnimationNode: AnimationBaseNode3,
	aAnimation: any,
	aInterpolator: PropertyInterpolatorType,
	ClassTemplateInstance: FromToByActivityCtorType,
	bAccumulate: boolean,
	eValueType: PropertyValueType,
) {
	const aAnimatedElement = aAnimationNode.getAnimatedElement();
	const aOperatorSet = aOperatorSetMap.get(eValueType);
	assert(aOperatorSet, 'createFromToByActivity: no operator set found');

	const aValueSet = [];
	aValueSet[0] = aAnimationNode.getFromValue();
	aValueSet[1] = aAnimationNode.getToValue();
	aValueSet[2] = aAnimationNode.getByValue();

	ANIMDBG.print(
		'createFromToByActivity: value type: ' +
			PropertyValueType[eValueType] +
			', aFrom = ' +
			aValueSet[0] +
			', aTo = ' +
			aValueSet[1] +
			', aBy = ' +
			aValueSet[2],
	);

	const aValueList: any[] = [];

	extractAttributeValues(
		eValueType,
		aValueList,
		aValueSet,
		aAnimatedElement.getBaseBBox(),
		aActivityParamSet.nSlideWidth,
		aActivityParamSet.nSlideHeight,
	);

	ANIMDBG.print(
		'createFromToByActivity: ' +
			', aFrom = ' +
			aValueList[0] +
			', aTo = ' +
			aValueList[1] +
			', aBy = ' +
			aValueList[2],
	);

	return new ClassTemplateInstance(
		aValueList[0],
		aValueList[1],
		aValueList[2],
		aActivityParamSet,
		aAnimation,
		aInterpolator,
		aOperatorSet,
		bAccumulate,
	);
}

function extractAttributeValues(
	eValueType: PropertyValueType,
	aValueList: any[],
	aValueSet: any[],
	aBBox: DOMRect,
	nSlideWidth: number,
	nSlideHeight: number,
) {
	let i: number;
	switch (eValueType) {
		case PropertyValueType.Number:
			evalValuesAttribute(
				aValueList,
				aValueSet,
				aBBox,
				nSlideWidth,
				nSlideHeight,
			);
			break;
		case PropertyValueType.Bool:
			for (i = 0; i < aValueSet.length; ++i) {
				const aValue = booleanParser(aValueSet[i]);
				aValueList.push(aValue);
			}
			break;
		case PropertyValueType.String:
			for (i = 0; i < aValueSet.length; ++i) {
				aValueList.push(aValueSet[i]);
			}
			break;
		case PropertyValueType.Enum:
			for (i = 0; i < aValueSet.length; ++i) {
				aValueList.push(aValueSet[i]);
			}
			break;
		case PropertyValueType.Color:
			for (i = 0; i < aValueSet.length; ++i) {
				const aValue = colorParser(aValueSet[i]);
				aValueList.push(aValue);
			}
			break;
		case PropertyValueType.TupleNumber:
			for (i = 0; i < aValueSet.length; ++i) {
				if (typeof aValueSet[i] === 'string') {
					const aTuple = aValueSet[i].split(',');
					const aValue: number[] = [];
					evalValuesAttribute(aValue, aTuple, aBBox, nSlideWidth, nSlideHeight);
					aValueList.push(aValue);
				} else {
					aValueList.push(undefined);
				}
			}
			break;
		default:
			window.app.console.log(
				'createValueListActivity: unexpected value type: ' + eValueType,
			);
	}
}

function evalValuesAttribute(
	aValueList: number[],
	aValueSet: any[],
	aBBox: DOMRect,
	nSlideWidth: number,
	nSlideHeight: number,
) {
	const vars: Record<string, number> = {
		width: aBBox.width / nSlideWidth,
		height: aBBox.height / nSlideHeight,
		x: (aBBox.x + aBBox.width / 2) / nSlideWidth,
		y: (aBBox.y + aBBox.height / 2) / nSlideHeight,
	};
	for (let i = 0; i < aValueSet.length; ++i) {
		const sValue: string = aValueSet[i];
		aValueList.push(sValue ? evalAnimFormula(sValue, vars) : undefined);
	}
}
