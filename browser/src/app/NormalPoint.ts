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
