/* -*- js-indent-level: 8; fill-column: 100 -*- */

/// Intermediate representation of a point position.
class NormalPoint {
	public static SCALE = 1.2;

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

	private static scale(zoom: number): number {
		return zoom ? Math.pow(NormalPoint.SCALE, zoom) : 1;
	}

	// Needed in Map.prototype.unproject()
	// constructs NormalPoint from css pixel point at a given zoom.
	public static pointToLatLng(point: cool.Point, zoom: number): NormalPoint {
		const scale = NormalPoint.scale(zoom);
		return new NormalPoint(-point.y / scale, point.x / scale);
	}

	// Needed in Map.prototype.project()
	// constructs css pixel point from LatLng at a given zoom.
	public static latLngToPoint(
		normPoint: NormalPoint,
		zoom: number,
	): cool.Point {
		const scale = NormalPoint.scale(zoom);
		return new cool.Point(normPoint.lng * scale, -normPoint.lat * scale);
	}

	// used in Map.prototype.rescale(), only makes sense for pixel points.
	// (cool.Point, number, number) -> cool.Point
	public static rescale(
		point: cool.Point,
		oldZoom: number,
		newZoom: number,
	): cool.Point {
		const scale = NormalPoint.scale(newZoom - oldZoom);
		return cool.Point.toPoint(point.x * scale, point.y * scale);
	}

	// used in Map.prototype.distance()
	public static distance(
		normPoint1: NormalPoint,
		normPoint2: NormalPoint,
	): number {
		const dx = normPoint2.lng - normPoint1.lng;
		const dy = normPoint2.lat - normPoint1.lat;
		return Math.sqrt(dx * dx + dy * dy);
	}

	public equals(obj: any, maxMargin?: number): boolean {
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

	public toString(precision?: number): string {
		return (
			'LatLng(' +
			app.util.formatNum(this.lat, precision ? precision : 3) +
			', ' +
			app.util.formatNum(this.lng, precision ? precision : 3) +
			')'
		);
	}

	public distanceTo(): number {
		return 0;
	}

	public wrap(): NormalPoint | null {
		return null;
	}

	public isNaN(): boolean {
		return isNaN(this.lat) || isNaN(this.lng);
	}
}

window.L.LatLng = NormalPoint;
window.L.latLng = NormalPoint.flexConstruct;
