/* -*- js-indent-level: 8; fill-column: 100 -*- */

/// Intermediate representation of a point position.
class NormalPoint {
	public lat: number;
	public lng: number;

	constructor(lat: number, lng: number) {
		if (isNaN(lat) || isNaN(lng)) {
			throw new Error('Invalid LatLng object: (' + lat + ', ' + lng + ')');
		}

		this.lat = +lat;
		this.lng = +lng;
	}

	// constructs LatLng with different signatures
	// (NormalPoint) or ([Number, Number]) or (Number, Number) or (NormalPointLike)
	public static flexConstruct(a: any, b?: number): NormalPoint | null {
		if (a instanceof NormalPoint) {
			return a;
		}

		if (Array.isArray(a) && typeof a[0] === 'number') {
			if (a.length === 2) {
				return new NormalPoint(a[0], a[1]);
			}
			return null;
		}

		if (a === undefined || a === null) {
			return null;
		}

		if (typeof a === 'object' && 'lat' in a && 'lng' in a) {
			return new NormalPoint(a.lat, a.lng);
		}

		if (typeof a === 'number' && typeof b === 'number') {
			return new NormalPoint(a, b);
		}

		return null;
	}

	public equals(obj: any, maxMargin: number): boolean {
		if (!obj) {
			return false;
		}

		obj = window.L.latLng(obj);

		const margin = Math.max(
			Math.abs(this.lat - obj.lat),
			Math.abs(this.lng - obj.lng),
		);

		return margin <= (maxMargin === undefined ? 1.0e-9 : maxMargin);
	}

	public toString(precision: number): string {
		return (
			'LatLng(' +
			app.util.formatNum(this.lat, precision) +
			', ' +
			app.util.formatNum(this.lng, precision) +
			')'
		);
	}

	public distanceTo(): number {
		return 0;
	}

	public wrap(): NormalPoint | null {
		return null;
	}
}
