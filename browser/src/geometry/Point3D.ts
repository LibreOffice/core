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
	/// A point or a direction in 3D. A point never changes once made, each operation returns a
	/// new one.
	export class Point3D {
		constructor(
			public readonly x: number,
			public readonly y: number,
			public readonly z: number,
		) {}

		plus(other: Point3D): Point3D {
			return new Point3D(this.x + other.x, this.y + other.y, this.z + other.z);
		}

		minus(other: Point3D): Point3D {
			return new Point3D(this.x - other.x, this.y - other.y, this.z - other.z);
		}

		times(factor: number): Point3D {
			return new Point3D(this.x * factor, this.y * factor, this.z * factor);
		}

		dot(other: Point3D): number {
			return this.x * other.x + this.y * other.y + this.z * other.z;
		}

		cross(other: Point3D): Point3D {
			return new Point3D(
				this.y * other.z - this.z * other.y,
				this.z * other.x - this.x * other.z,
				this.x * other.y - this.y * other.x,
			);
		}

		length(): number {
			return Math.hypot(this.x, this.y, this.z);
		}

		/// The point that far on the way to the other one: this one at 0, the other at 1.
		towards(other: Point3D, share: number): Point3D {
			return this.plus(other.minus(this).times(share));
		}
	}
}
