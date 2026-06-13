/* -*- js-indent-level: 8 -*- */

/// Intermediate representation of a rectangular bound using NormalPoint.
class NormalBounds {
	private _min?: NormalPoint = undefined;
	private _max?: NormalPoint = undefined;

	public static flexConstruct(
		a:
			| undefined
			| NormalBounds
			| NormalPoint
			| NormalPoint[]
			| number[]
			| number[][],
		b?: NormalPoint | number[],
	): NormalBounds | undefined {
		if (!a) {
			return undefined;
		}

		if (a instanceof NormalBounds) {
			return a;
		}

		return new NormalBounds(a, b);
	}

	constructor(
		min: NormalPoint | NormalPoint[] | number[] | number[][],
		max?: NormalPoint | number[],
	) {
		if (!min) {
			return;
		}

		const points: NormalPoint[] = [];
		const inputList: any[] = [min, max];

		for (let inputIdx = 0; inputIdx < 2; ++inputIdx) {
			const input = inputList[inputIdx];

			if (input instanceof NormalPoint) {
				points.push(input);
			}

			if (Array.isArray(input)) {
				let onlyNumbers = true;
				for (let i = 0; i < input.length; ++i) {
					const element = input[i];
					if (element instanceof NormalPoint) {
						points.push(element);
						onlyNumbers = false;
					} else if (Array.isArray(element)) {
						if (
							element.length === 2 &&
							typeof element[0] === 'number' &&
							typeof element[1] === 'number'
						) {
							onlyNumbers = false;
							const np = NormalPoint.flexConstruct(element);
							if (np) {
								points.push(np);
							}
						}
					}
				}

				if (onlyNumbers && input.length === 2) {
					const np = NormalPoint.flexConstruct(input);
					if (np) {
						points.push(np);
					}
				}
			}
		}

		for (let i = 0; i < points.length; ++i) {
			this.extend(points[i]);
		}
	}

	public extend(obj: any): NormalBounds {
		if (!obj) {
			return this;
		}
		const min = this._min;
		const max = this._max;
		let min2: NormalPoint | undefined = undefined;
		let max2: NormalPoint | undefined = undefined;

		if (obj instanceof NormalPoint) {
			min2 = obj;
			max2 = obj;
		} else if (obj instanceof NormalBounds) {
			min2 = obj._min;
			max2 = obj._max;
		} else {
			this.extend(
				NormalPoint.flexConstruct(obj) || NormalBounds.flexConstruct(obj),
			);
		}

		if (!min2 || !max2) {
			return this;
		}

		if (!min && !max) {
			this._min = new NormalPoint(min2.x, min2.y);
			this._max = new NormalPoint(max2.x, max2.y);
		} else {
			Util.ensureValue(min);
			min.x = Math.min(min2.x, min.x);
			min.y = Math.min(min2.y, min.y);

			Util.ensureValue(max);
			max.x = Math.max(max2.x, max.x);
			max.y = Math.max(max2.y, max.y);
		}
		return this;
	}

	public getCenter(): NormalPoint {
		Util.ensureValue(this._min);
		Util.ensureValue(this._max);
		return new NormalPoint(
			(this._min.x + this._max.x) / 2,
			(this._min.y + this._max.y) / 2,
		);
	}

	public getTopLeft(): NormalPoint {
		Util.ensureValue(this._min);
		return this._min;
	}

	public getBottomRight(): NormalPoint {
		Util.ensureValue(this._max);
		return this._max;
	}

	public getLeft(): number {
		Util.ensureValue(this._min);
		return this._min.x;
	}

	public getBottom(): number {
		Util.ensureValue(this._max);
		return this._max.y;
	}

	public getRight(): number {
		Util.ensureValue(this._max);
		return this._max.x;
	}

	public getTop(): number {
		Util.ensureValue(this._min);
		return this._min.y;
	}

	public getWidth(): number {
		return Math.abs(this.getRight() - this.getLeft());
	}

	public getHeight(): number {
		return Math.abs(this.getBottom() - this.getTop());
	}

	private _getAsPointOrBounds(
		obj: NormalPoint | number[] | NormalBounds | NormalPoint[] | number[][],
	): NormalPoint | NormalBounds | undefined {
		let res: NormalPoint | NormalBounds | null | undefined;
		if (
			(Array.isArray(obj) && typeof obj[0] === 'number') ||
			obj instanceof NormalPoint
		) {
			res = NormalPoint.flexConstruct(obj);
		} else {
			res = NormalBounds.flexConstruct(obj);
		}
		return res ? res : undefined;
	}

	public contains(
		obj_: NormalPoint | number[] | NormalBounds | NormalPoint[] | number[][],
	): boolean {
		Util.ensureValue(this._min);
		Util.ensureValue(this._max);

		const obj = this._getAsPointOrBounds(obj_);
		Util.ensureValue(obj);

		const min = this._min;
		const max = this._max;
		let min2: NormalPoint;
		let max2: NormalPoint;

		if (obj instanceof NormalBounds) {
			min2 = obj.getTopLeft();
			max2 = obj.getBottomRight();
		} else {
			min2 = max2 = obj;
		}

		return (
			min2.x >= min.x && min2.y >= min.y && max2.x <= max.x && max2.y <= max.y
		);
	}

	public intersects(
		bounds_: NormalBounds | NormalPoint[] | number[][],
	): boolean {
		const bounds = NormalBounds.flexConstruct(bounds_);
		Util.ensureValue(bounds);
		Util.ensureValue(this._min);
		Util.ensureValue(this._max);

		const min = this._min;
		const max = this._max;
		const min2 = bounds.getTopLeft();
		const max2 = bounds.getBottomRight();

		const xIntersects = max2.x >= min.x && min2.x <= max.x;
		const yIntersects = max2.y >= min.y && min2.y <= max.y;

		return xIntersects && yIntersects;
	}

	public equals(
		bounds_: NormalBounds | NormalPoint[] | number[][] | undefined | null,
	): boolean {
		if (!bounds_) {
			return false;
		}

		const bounds = NormalBounds.flexConstruct(bounds_);
		Util.ensureValue(bounds);
		Util.ensureValue(this._min);
		Util.ensureValue(this._max);

		return (
			this._min.equals(bounds.getTopLeft()) &&
			this._max.equals(bounds.getBottomRight())
		);
	}

	public isValid(): boolean {
		return !!(this._min && this._max);
	}

	public isInAny(boundsArray: NormalBounds[]): boolean {
		for (let i = 0; i < boundsArray.length; ++i) {
			if (boundsArray[i].contains(this)) {
				return true;
			}
		}

		return false;
	}

	// Please do not remove even if unused. It can be useful in
	// temporary console.log() etc.
	public toString() {
		return (
			'NormalBounds(' +
			(this._min ? this._min.toString() : 'undefined') +
			',' +
			(this._max ? this._max.toString() : 'undefined') +
			')'
		);
	}
}
