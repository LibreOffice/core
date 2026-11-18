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

namespace cool {
	/**
	 * Represents a rectangle object which works with core pixels.
	 * x1 and y1 should always <= x2 and y2. In other words width >= 0 && height >= 0 is a precondition.
	 * This class doesn't check for above conditions. There is a isValid function for use when needed.
	 */
	export class Rectangle {
		private x1: number;
		private y1: number;
		private width: number;
		private height: number;

		constructor(x: number, y: number, width: number, height: number) {
			this.x1 = x;
			this.y1 = y;
			this.width = width;
			this.height = height;
		}

		// convenience private getters

		private get x2(): number {
			return this.x1 + this.width;
		}

		private get y2(): number {
			return this.y1 + this.height;
		}

		private get area(): number {
			return this.width * this.height;
		}

		// Rounded coordinates private getters

		private get rx1(): number {
			return Math.round(this.x1);
		}

		private get ry1(): number {
			return Math.round(this.y1);
		}

		private get rx2(): number {
			return Math.round(this.x2);
		}

		private get ry2(): number {
			return Math.round(this.y2);
		}

		private get rwidth(): number {
			return this.rx2 - this.rx1;
		}

		private get rheight(): number {
			return this.ry2 - this.ry1;
		}

		private get rarea(): number {
			return this.rwidth * this.rheight;
		}

		public isValid(): boolean {
			if (this.x1 <= this.x2 && this.y1 <= this.y2) return true;
			return false;
		}

		public clone(): Rectangle {
			return new Rectangle(this.x1, this.x2, this.width, this.height);
		}

		public containsPoint(x: number, y: number): boolean {
			if (x >= this.x1 && x <= this.x2 && y >= this.y1 && y <= this.y2)
				return true;

			return false;
		}

		public containsPixel(px: number, py: number): boolean {
			if (px >= this.rx1 && px <= this.rx2 && py >= this.ry1 && py <= this.ry2)
				return true;

			return false;
		}

		public containsXOrdinate(ox: number): boolean {
			if (ox >= this.x1 && ox <= this.x2) return true;

			return false;
		}

		public containsYOrdinate(oy: number): boolean {
			if (oy >= this.y1 && oy <= this.y2) return true;

			return false;
		}

		public containsPixelOrdinateX(ox: number): boolean {
			if (ox >= this.rx1 && ox <= this.rx2) return true;

			return false;
		}

		public containsPixelOrdinateY(oy: number): boolean {
			if (oy >= this.ry1 && oy <= this.ry2) return true;

			return false;
		}

		/// Sets x1 of the rectangle without changing x2.
		public setX1(x1: number): void {
			this.width += this.x1 - x1;
			this.x1 = x1;
		}

		/// Sets x2 of the rectangle without changing x1.
		public setX2(x2: number): void {
			this.width = x2 - this.x1;
		}

		/// Sets y1 of the rectangle without changing y2.
		public setY1(y1: number): void {
			this.height += this.y1 - y1;
			this.y1 = y1;
		}

		/// Sets y2 of the rectangle without changing y1.
		public setY2(y2: number): void {
			this.height = y2 - this.y1;
		}

		/// Sets width keeping x1 constant.
		public setWidth(width: number): void {
			this.width = width;
		}

		/// Sets height keeping y1 constant.
		public setHeight(height: number): void {
			this.height = height;
		}

		/// Sets area by either keeping height or width as constant.
		public setArea(area: number, preserveHeight: boolean): void {
			if (!preserveHeight) {
				//preserve width
				const height = area / this.width;
				this.setHeight(height);
			} else {
				// preserve height
				const width = area / this.height;
				this.setWidth(width);
			}
		}

		/// Moves the whole rectangle by (dx, dy) by preserving width and height.
		public moveBy(dx: number, dy: number): void {
			this.x1 += dx;
			this.y1 += dy;
		}

		/**
		 * Moves the rectangle to (x, y) as its new x1, y1 without changing
		 * its size.
		 */
		public moveTo(x: number, y: number): void {
			this.x1 = x;
			this.y1 = y;
		}

		// TODO: Following methods could be replaced with js getters
		// but we need to change their users to access getters.

		public getX1(): number {
			return this.x1;
		}

		public getX2(): number {
			return this.x2;
		}

		public getY1(): number {
			return this.y1;
		}

		public getY2(): number {
			return this.y2;
		}

		public getWidth(): number {
			return this.width;
		}

		public getHeight(): number {
			return this.height;
		}

		public getArea(): number {
			return this.area;
		}

		public getCenter(): number[] {
			return [(this.x2 + this.x1) / 2, (this.y2 + this.y1) / 2];
		}

		public getPxX1(): number {
			return this.rx1;
		}

		public getPxX2(): number {
			return this.rx2;
		}

		public getPxY1(): number {
			return this.ry1;
		}

		public getPxY2(): number {
			return this.ry2;
		}

		public getPxWidth(): number {
			return this.rwidth;
		}

		public getPxHeight(): number {
			return this.rheight;
		}

		public getPxArea(): number {
			return this.rarea;
		}

		public getPxCenter(): number[] {
			return [
				Math.round((this.rx2 + this.rx1) / 2),
				Math.round((this.ry2 + this.ry1) / 2),
			];
		}
	}

	export function createRectangle(
		x: number,
		y: number,
		width: number,
		height: number,
	): Rectangle {
		return new Rectangle(x, y, width, height);
	}
}
