/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
	/// One stop of a gradient ramp.
	export interface GradientColorStop {
		offset: number; // 0 at the start of the ramp, 1 at its end
		color: string;
		opacity?: number; // 0 clear to 1 opaque, SVG gradients only
	}

	/// A gradient ramp and where it sits across the primitive's
	/// definition range. When the ramp carries transparency instead of
	/// color, each stop's luminance is the transparency: black opaque,
	/// white clear.
	export interface GradientAttribute {
		style?:
			| 'linear'
			| 'axial'
			| 'radial'
			| 'elliptical'
			| 'square'
			| 'rect'
			| 'unknown';
		angle?: number; // degrees
		border?: number; // 0 to 1 of the range held at the first color
		offsetX?: number; // 0 to 1, center of the round styles
		offsetY?: number; // 0 to 1, center of the round styles
		steps?: number; // flat bands the ramp is broken into
		colorStops?: GradientColorStop[];
	}
}
