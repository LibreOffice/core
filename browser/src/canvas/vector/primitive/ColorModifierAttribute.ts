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
	/// A change to the red, green and blue of every color it covers, each
	/// channel from 0 to 1. Alpha never changes. modifier names the change,
	/// which decides the other fields:
	/// - "gray": the luminance, weighted 0.30, 0.59 and 0.11.
	/// - "invert": one minus each channel.
	/// - "luminance_to_alpha": a gray of one minus the luminance, weighted
	///   0.2125, 0.7154 and 0.0721.
	/// - "replace": color, for every pixel.
	/// - "interpolate": color, mixed with the original by value, where 0 is
	///   all color and 1 all original.
	/// - "saturate" and "hueRotate": matrix, nine weights, a row per channel
	///   over red, green and blue.
	/// - "matrix": matrix, twenty weights, a row for each of red, green, blue
	///   and alpha over those and one. The alpha row has no effect.
	/// - "black_and_white": value, a luminance weighted as for gray. Below it
	///   is black, the rest white.
	/// - "gamma": value, above 0 and up to 10. Channels go to one over it.
	/// - "RGBLuminanceContrast": red, green, blue, luminance and contrast, each
	///   from -1 to 1, where 0 changes nothing.
	/// - "randomize": no other field. Mixes in a random color.
	export interface ColorModifierAttribute {
		modifier?: string;
		color?: string;
		value?: number;
		matrix?: number[];
		red?: number;
		green?: number;
		blue?: number;
		luminance?: number;
		contrast?: number;
	}
}
