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
	/// Wrapper for geometry that exists in the document model but
	/// must not paint, for example shapes kept only for hit-testing
	/// or layout.
	export interface HiddenGeometryPrimitive extends Primitive {
		type: typeof HiddenGeometryPrimitive.type;
	}

	export namespace HiddenGeometryPrimitive {
		export const type = 'hiddenGeometry';
	}
}
