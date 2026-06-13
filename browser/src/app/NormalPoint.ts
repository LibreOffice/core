/* -*- js-indent-level: 8; fill-column: 100 -*- */

/// Intermediate representation of a point position.
class NormalPoint {
	public static SCALE = 1.2;

	public x: number;
	public y: number;

	constructor(x_: number, y_: number) {
		if (isNaN(x_) || isNaN(y_)) {
			throw new Error('Invalid Intern object: (' + x_ + ', ' + y_ + ')');
		}

		this.x = +x_;
		this.y = +y_;
	}

	// constructs Intern with different signatures
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

		if (typeof a === 'object' && 'x' in a && 'y' in a) {
			return new NormalPoint(a.x, a.y);
		}

		if (typeof a === 'number' && typeof b === 'number') {
			return new NormalPoint(a, b);
		}

		return null;
	}

	public static scale(zoom: number): number {
		return zoom ? Math.pow(NormalPoint.SCALE, zoom) : 1;
	}

	// Needed in Map.prototype.unproject()
	// constructs NormalPoint from css pixel point at a given zoom.
	public static pointToIntern(point: cool.Point, zoom: number): NormalPoint {
		const scale = NormalPoint.scale(zoom);
		return new NormalPoint(point.x / scale, point.y / scale);
	}

	// Needed in Map.prototype.project()
	// constructs css pixel point from Intern at a given zoom.
	public static internToPoint(
		normPoint: NormalPoint,
		zoom: number,
	): cool.Point {
		const scale = NormalPoint.scale(zoom);
		return new cool.Point(normPoint.x * scale, normPoint.y * scale);
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
		const dx = normPoint2.x - normPoint1.x;
		const dy = normPoint2.y - normPoint1.y;
		return Math.sqrt(dx * dx + dy * dy);
	}

	public equals(obj_: any, maxMargin?: number): boolean {
		if (!obj_) {
			return false;
		}

		const obj = NormalPoint.flexConstruct(obj_);
		Util.ensureValue(obj);

		const margin = Math.max(Math.abs(this.x - obj.x), Math.abs(this.y - obj.y));

		return margin <= (maxMargin === undefined ? 1.0e-9 : maxMargin);
	}

	public toString(precision?: number): string {
		return (
			'NormalPoint(' +
			app.util.formatNum(this.x, precision ? precision : 3) +
			', ' +
			app.util.formatNum(this.y, precision ? precision : 3) +
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
		return isNaN(this.x) || isNaN(this.y);
	}

	public getX(): number {
		return this.x;
	}

	public getY(): number {
		return this.y;
	}
}
