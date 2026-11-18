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

class KeyStopLerp {
	private aKeyStopList: Array<number>;
	private nLastIndex: number;
	private nKeyStopDistance: number;
	private nUpperBoundIndex: number;

	constructor(aValueList: Array<number>) {
		KeyStopLerp.validateInput(aValueList);

		this.aKeyStopList = [];
		this.nLastIndex = 0;
		this.nKeyStopDistance = aValueList[1] - aValueList[0];
		if (this.nKeyStopDistance <= 0) this.nKeyStopDistance = 0.001;

		for (let i = 0; i < aValueList.length; ++i)
			this.aKeyStopList.push(aValueList[i]);

		this.nUpperBoundIndex = this.aKeyStopList.length - 2;
	}

	public static validateInput(aValueList: Array<number>) {
		const nSize = aValueList.length;
		assert(
			nSize > 1,
			'KeyStopLerp.validateInput: key stop vector must have two entries or more',
		);

		for (let i = 1; i < nSize; ++i) {
			if (aValueList[i - 1] > aValueList[i])
				window.app.console.log(
					'KeyStopLerp.validateInput: time vector is not sorted in ascending order!',
				);
		}
	}

	public reset() {
		KeyStopLerp.validateInput(this.aKeyStopList);
		this.nLastIndex = 0;
		this.nKeyStopDistance = this.aKeyStopList[1] - this.aKeyStopList[0];
		if (this.nKeyStopDistance <= 0) this.nKeyStopDistance = 0.001;
	}

	public lerp(nAlpha: number) {
		if (nAlpha > this.aKeyStopList[this.nLastIndex + 1]) {
			do {
				const nIndex = this.nLastIndex + 1;
				this.nLastIndex = clampN(nIndex, 0, this.nUpperBoundIndex);
				this.nKeyStopDistance =
					this.aKeyStopList[this.nLastIndex + 1] -
					this.aKeyStopList[this.nLastIndex];
			} while (
				this.nKeyStopDistance <= 0 &&
				this.nLastIndex < this.nUpperBoundIndex
			);
		}

		let nRawLerp =
			(nAlpha - this.aKeyStopList[this.nLastIndex]) / this.nKeyStopDistance;

		nRawLerp = clampN(nRawLerp, 0.0, 1.0);

		return {
			nIndex: this.nLastIndex,
			nLerp: nRawLerp,
		};
	}
}

abstract class ContinuousKeyTimeActivityBase extends SimpleContinuousActivityBase {
	private aLerper: KeyStopLerp;

	protected constructor(aCommonParamSet: ActivityParamSet) {
		super(aCommonParamSet);

		const nSize = aCommonParamSet.aDiscreteTimes.length;
		assert(
			nSize > 1,
			'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.length > 1) failed',
		);

		assert(
			aCommonParamSet.aDiscreteTimes[0] == 0.0,
			'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.front() == 0.0) failed',
		);

		assert(
			aCommonParamSet.aDiscreteTimes[nSize - 1] <= 1.0,
			'ContinuousKeyTimeActivityBase constructor: assertion (aDiscreteTimes.back() <= 1.0) failed',
		);

		this.aLerper = new KeyStopLerp(aCommonParamSet.aDiscreteTimes);
	}

	public activate(aEndElement: any) {
		super.activate(aEndElement);

		this.aLerper.reset();
	}

	public abstract performContinuousHook(
		nIndex: number,
		nFractionalIndex: number,
		nRepeatCount: number,
	): void;

	public simplePerform(nSimpleTime: number, nRepeatCount: number) {
		const nAlpha = this.calcAcceleratedTime(nSimpleTime);

		const aLerpResult = this.aLerper.lerp(nAlpha);

		this.performContinuousHook(
			aLerpResult.nIndex,
			aLerpResult.nLerp,
			nRepeatCount,
		);
	}
}
