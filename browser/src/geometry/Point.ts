// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */

/* global L */

namespace cool {

export interface PointLike {
	x: number;
	y: number;
}

export type PointConvertable = Point | Array<number> | PointLike;

/// Point represents a point with x and y coordinates.
export class Point {

	public x: number;
	public y: number;

	constructor(x: number, y: number, round: boolean = false) {
		this.x = (round ? Math.round(x) : x);
		this.y = (round ? Math.round(y) : y);
	}

	public static parse(pointString: string): Point { // (string) -> Point
		if (typeof pointString !== 'string') {
			console.error('invalid point string');
			return undefined;
		}

		var pointParts = pointString.match(/-?\d+/g);
		if (pointParts === null || pointParts.length < 2) {
			console.error('incomplete point');
			return undefined;
		}

		return new Point(parseInt(pointParts[0]), parseInt(pointParts[1]));
	}

	public clone(): Point {
		return new Point(this.x, this.y);
	}

	/// non-destructive, returns a new point
	public add(point: Point): Point {
		return this.clone()._add(Point.toPoint(point));
	}

	// destructive, used directly for performance in situations where it's safe to modify existing point
	public _add(point: Point): Point {
		this.x += point.x;
		this.y += point.y;
		return this;
	}

	public subtract(point: Point): Point {
		return this.clone()._subtract(Point.toPoint(point));
	}

	public _subtract(point: Point): Point {
		this.x -= point.x;
		this.y -= point.y;
		return this;
	}

	public divideBy(num: number): Point {
		return this.clone()._divideBy(num);
	}

	public _divideBy(num: number): Point {
		this.x /= num;
		this.y /= num;
		return this;
	}

	public multiplyBy(num: number): Point {
		return this.clone()._multiplyBy(num);
	}

	public _multiplyBy(num: number): Point {
		this.x *= num;
		this.y *= num;
		return this;
	}

	public round(): Point {
		return this.clone()._round();
	}

	public _round(): Point {
		this.x = Math.round(this.x);
		this.y = Math.round(this.y);
		return this;
	}

	public floor(): Point {
		return this.clone()._floor();
	}

	public _floor(): Point {
		this.x = Math.floor(this.x);
		this.y = Math.floor(this.y);
		return this;
	}

	public ceil(): Point {
		return this.clone()._ceil();
	}

	public _ceil(): Point {
		this.x = Math.ceil(this.x);
		this.y = Math.ceil(this.y);
		return this;
	}

	/// How far the point lies from the origin, which for a direction is its length.
	public length(): number {
		return Math.sqrt(this.x * this.x + this.y * this.y);
	}

	/// The cross product with the other one: positive where the other lies on the side the y axis
	/// turns to from this one, negative on the other side, zero where both lie on one line.
	public cross(point: Point): number {
		return this.x * point.y - this.y * point.x;
	}

	/// The dot product with the other one: how far this one reaches along the other, times the
	/// length of the other.
	public dot(point: Point): number {
		return this.x * point.x + this.y * point.y;
	}

	public distanceTo(point: Point): number {
		point = Point.toPoint(point);

		var x = point.x - this.x;
		var y = point.y - this.y;

		return Math.sqrt(x * x + y * y);
	}

	public equals(point: Point): boolean {
		point = Point.toPoint(point);

		// Proper ieee 754 equality comparison.
		return Math.abs(point.x - this.x) < Number.EPSILON &&
			Math.abs(point.y - this.y) < Number.EPSILON;
	}

	public contains(point: Point): boolean {
		point = Point.toPoint(point);

		return Math.abs(point.x) <= Math.abs(this.x) &&
			Math.abs(point.y) <= Math.abs(this.y);
	}

	public assign(point: Point): boolean {
		var xChanged = this.setX(point.x);
		var yChanged = this.setY(point.y);
		return xChanged || yChanged;
	}

	public setX(x: number): boolean {
		if (x === this.x) {
			return false;
		}

		this.x = x;
		return true;
	}

	public setY(y: number): boolean {
		if (y === this.y) {
			return false;
		}

		this.y = y;
		return true;
	}

	public toString(): string {
		return 'Point(' +
			Point.formatNum(this.x) + ', ' +
			Point.formatNum(this.y) + ')';
	}

	public static toPoint(x: PointConvertable | number, y?: number, round?: boolean): Point {
		if (x instanceof Point) {
			return x;
		}

		if (x instanceof SimplePoint) {
			return new Point(x.x, x.y);
		}

		if (Array.isArray(x)) {
			var arr = x as Array<number>;
			return new Point(arr[0], arr[1]);
		}

		if (x === undefined || x === null) {
			return undefined;
		}

		// Detect cool.Point like objects such as CPoint.
		if (Object.prototype.hasOwnProperty.call(x, 'x')
			&& Object.prototype.hasOwnProperty.call(x, 'y')) {
			x = <PointLike>x;
			return new Point(x.x, x.y);
		}

		x = <number>x;
		return new Point(x, y, round);
	}

	private static formatNum(num: number): number {
		var pow = Math.pow(10, 5);
		return Math.round(num * pow) / pow;
	}
}

}
