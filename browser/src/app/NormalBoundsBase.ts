/* -*- js-indent-level: 8 -*- */

/// Intermediate representation of a rectangular bound using NormalPoint.
class NormalBoundsBase {
	private _southWest?: NormalPoint = undefined;
	private _northEast?: NormalPoint = undefined;

	public static flexConstruct(
		a:
			| undefined
			| NormalBoundsBase
			| NormalPoint
			| NormalPoint[]
			| number[]
			| number[][],
		b?: NormalPoint | number[],
	): NormalBoundsBase | undefined {
		if (!a) {
			return undefined;
		}

		if (a instanceof NormalBoundsBase) {
			return a;
		}

		return new NormalBoundsBase(a, b);
	}

	constructor(
		southWest: NormalPoint | NormalPoint[] | number[] | number[][],
		northEast?: NormalPoint | number[],
	) {
		if (!southWest) {
			return;
		}

		const points: NormalPoint[] = [];
		const inputList: any[] = [southWest, northEast];

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

	public extend(obj: any): NormalBoundsBase {
		if (!obj) {
			return this;
		}
		const sw = this._southWest;
		const ne = this._northEast;
		let sw2: NormalPoint | undefined = undefined;
		let ne2: NormalPoint | undefined = undefined;

		if (obj instanceof NormalPoint) {
			sw2 = obj;
			ne2 = obj;
		} else if (obj instanceof NormalBoundsBase) {
			sw2 = obj._southWest;
			ne2 = obj._northEast;
		} else {
			this.extend(
				NormalPoint.flexConstruct(obj) || NormalBoundsBase.flexConstruct(obj),
			);
		}

		if (!sw2 || !ne2) {
			return this;
		}

		if (!sw && !ne) {
			this._southWest = new NormalPoint(sw2.lat, sw2.lng);
			this._northEast = new NormalPoint(ne2.lat, ne2.lng);
		} else {
			Util.ensureValue(sw);
			sw.lat = Math.min(sw2.lat, sw.lat);
			sw.lng = Math.min(sw2.lng, sw.lng);

			Util.ensureValue(ne);
			ne.lat = Math.max(ne2.lat, ne.lat);
			ne.lng = Math.max(ne2.lng, ne.lng);
		}
		return this;
	}

	public getCenter(): NormalPoint {
		Util.ensureValue(this._southWest);
		Util.ensureValue(this._northEast);
		return new NormalPoint(
			(this._southWest.lat + this._northEast.lat) / 2,
			(this._southWest.lng + this._northEast.lng) / 2,
		);
	}

	public getSouthWest(): NormalPoint {
		Util.ensureValue(this._southWest);
		return this._southWest;
	}

	public getNorthEast(): NormalPoint {
		Util.ensureValue(this._northEast);
		return this._northEast;
	}

	public getNorthWest(): NormalPoint {
		return new NormalPoint(this.getNorth(), this.getWest());
	}

	public getSouthEast(): NormalPoint {
		return new NormalPoint(this.getSouth(), this.getEast());
	}

	public getWest(): number {
		Util.ensureValue(this._southWest);
		return this._southWest.lng;
	}

	public getSouth(): number {
		Util.ensureValue(this._southWest);
		return this._southWest.lat;
	}

	public getEast(): number {
		Util.ensureValue(this._northEast);
		return this._northEast.lng;
	}

	public getNorth(): number {
		Util.ensureValue(this._northEast);
		return this._northEast.lat;
	}

	public getWidth(): number {
		return Math.abs(this.getEast() - this.getWest());
	}

	public getHeight(): number {
		return Math.abs(this.getNorth() - this.getSouth());
	}
}
