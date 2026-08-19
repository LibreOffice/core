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
	/// An image fill placed by a matrix that maps the unit square onto
	/// the area to cover.
	export interface FillGraphicPrimitive extends Primitive {
		type: typeof FillGraphicPrimitive.type;
		matrix?: number[];
		fillGraphic?: FillGraphicAttribute;
		transparency?: number;
	}

	export namespace FillGraphicPrimitive {
		export const type = 'fillGraphic';
	}
}
