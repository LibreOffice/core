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
	/// A hatch filling a rectangle. The lines are laid out across
	/// definitionRange and painted inside outputRange.
	export interface FillHatchPrimitive extends Primitive {
		type: typeof FillHatchPrimitive.type;
		outputRange?: [number, number, number, number];
		definitionRange?: [number, number, number, number];
		backgroundColor?: string;
		hatch?: HatchAttribute;
	}

	export namespace FillHatchPrimitive {
		export const type = 'fillHatch';
	}
}
